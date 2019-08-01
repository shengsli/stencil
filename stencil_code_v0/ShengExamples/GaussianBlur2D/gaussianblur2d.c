/**
 * Gaussian filter is hardcoded as a 5x5 filter. Radius must be 2. 
 * gcc gaussianblur2d.c -O2 -lpthread -DRADIUS=2 -DNTHREADS=4 -DNROWS=100 -DNCOLS=100 -DNITERS=20 -o gaussianblur2d 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

typedef struct arg_pack_tag {
	int tid;
	int cur_chunk_size;
 	double *input;
	double *output;
} arg_pack;

typedef arg_pack *argptr;

pthread_barrier_t barrier;

double filter[] = {1./273, 4./273, 7./273, 4./273, 1./273, 
				   4./273,16./273,26./273,16./273, 4./273,
				   7./273,26./273,41./273,26./273, 7./273,
				   4./273,16./273,26./273,16./273, 4./273,
				   1./273, 4./273, 7./273, 4./273, 1./273};

double second()
{
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

double conv(double *lhs, double *rhs, int size)
{
	double sum=0.;
	for (int i=0; i<size; ++i) {
		sum += lhs[i]*rhs[i];
	}
	return sum;
}

void *gaussian_blur (void *args)
{
	int tid, cur_chunk_size;
	double *input, *output;
	tid=((arg_pack*)args)->tid;
	cur_chunk_size=((arg_pack*)args)->cur_chunk_size;
	input=((arg_pack*)args)->input;
	output=((arg_pack*)args)->output;

	int iter;
	for (iter=0; iter<NITERS; iter++) {
		if (iter>0) {
			double* temp = input;
		    input = output;
			output = temp;
		}

		int i;
		for (i=0; i<cur_chunk_size; i++) { // iterate over data chunk
			int chunk_size = NROWS*NCOLS / NTHREADS; // chunk size of 0..n-1 chunks
			int j;

			int elIdx = tid*chunk_size+i;
			int elCol = elIdx % NCOLS;
			int elRow = elIdx / NCOLS;
			int neighbourCol, neighbourRow;
			double* neighbourhood;
			neighbourhood = malloc((RADIUS*2+1)*(RADIUS*2+1)*sizeof(double));

			// iterate over filter window
			int row, col;
			for (row=0; row<2*RADIUS+1; ++row) {
				for (col=0; col<2*RADIUS+1; ++col) {
					neighbourCol = (elCol+col+NCOLS-RADIUS)%NCOLS;
					neighbourRow = (elRow+row+NROWS-RADIUS)%NROWS;
					neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+neighbourRow*NCOLS];
				}
			}
			output[tid*chunk_size+i] = conv(neighbourhood,filter,(RADIUS*2+1)*(RADIUS*2+1));
		}
		pthread_barrier_wait(&barrier);
	}
}

void parallel_gaussian_blur(double *input, double *output)
{
	double tstart, tstop;
    tstart = second();

	pthread_barrier_init(&barrier, NULL, NTHREADS);
	pthread_t *threads;
	arg_pack *threadargs;
	threads = (pthread_t *) malloc(NTHREADS*sizeof(pthread_t));
	threadargs  = (arg_pack *) malloc(NTHREADS*sizeof(arg_pack));

	int chunk_size = NROWS*NCOLS / NTHREADS;
	int i;
	for (i=0; i<NTHREADS; i++)
	{
		threadargs[i].tid = i;
		threadargs[i].cur_chunk_size = chunk_size;
		threadargs[i].input = input;
		threadargs[i].output = output;
	}
	// give rest items to the last thread
	threadargs[NTHREADS-1].cur_chunk_size += NROWS*NCOLS - chunk_size * NTHREADS;

	for (i=0; i<NTHREADS; i++)
		pthread_create(&threads[i],NULL,gaussian_blur,(void*)&threadargs[i]);
	for (i=0; i<NTHREADS; i++)
		pthread_join(threads[i], NULL);
	
    tstop = second();
    printf("%f, p%d, 0, %d\n", tstop-tstart, NTHREADS, NROWS*NCOLS);
}

int main (int argc, char* argv[])
{
	double *par_input, *par_output;
	par_input = malloc(NROWS*NCOLS*sizeof(double));
	par_output = malloc(NROWS*NCOLS*sizeof(double));

	int i;
	for (i=0; i<NROWS*NCOLS; i++) { // init arr
		par_input[i] = (double)i;
	}
	parallel_gaussian_blur(par_input, par_output);
	return 0;
}
