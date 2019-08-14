/**
 * gcc sum3d.c -O2 -lpthread -DRADIUS=1 -DNTHREADS=4 -DNXS=10 -DNYS=10 -DNZS=10 -DNITERS=3 -o sum3d 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

typedef struct arg_pack_tag {
	int tid;
	int cur_chunk_size;
 	int *input;
	int *output;
} arg_pack;

typedef arg_pack *argptr;

pthread_barrier_t barrier;

double second()
{
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

void *sum (void *args)
{
	int tid, cur_chunk_size;
	int *input, *output;
	tid=((arg_pack*)args)->tid;
	cur_chunk_size=((arg_pack*)args)->cur_chunk_size;
	input=((arg_pack*)args)->input;
	output=((arg_pack*)args)->output;

	int iter;
	for (iter=0; iter<NITERS; iter++) {
		if (iter>0) {
			int* temp = input;
		    input = output;
			output = temp;
		}
	
		int i;
		for (i=0; i<cur_chunk_size; i++) { // iterate over data chunk
			int chunk_size = NXS*NYS*NZS / NTHREADS; // chunk size of 0..n-1 chunks
			int sum=0;
			int j;

			int elIdx = tid*chunk_size+i;
			int elx = elIdx % NXS;
			int ely = (elIdx / NXS) % NYS;
			int elz = elIdx / NXS / NYS;
			int neighbourx, neighboury, neighbourz;

			// iterate over filter window
			int filterx, filtery, filterz;
			for (filterz=0; filterz<2*RADIUS+1; ++filterz) {
				for (filtery=0; filtery<2*RADIUS+1; ++filtery) {
					for (filterx=0; filterx<2*RADIUS+1; ++filterx) {
						neighbourx = (elx+filterx-RADIUS+NXS)%NXS;
						neighboury = (ely+filtery-RADIUS+NYS)%NYS;
						neighbourz = (elz+filterz-RADIUS+NZS)%NZS;
					    sum += input[neighbourx+NXS*(neighboury+NYS*neighbourz)];
					}
				}
			}
			output[tid*chunk_size+i] = sum;
		}
		pthread_barrier_wait(&barrier);
	}
}

void parallel_sum(int *input, int *output)
{
    double tstart, tstop;
    tstart = second();

	pthread_barrier_init(&barrier, NULL, NTHREADS);
	pthread_t *threads;
	arg_pack *threadargs;
	threads = (pthread_t *) malloc(NTHREADS*sizeof(pthread_t));
	threadargs  = (arg_pack *) malloc(NTHREADS*sizeof(arg_pack));

	int chunk_size = NXS*NYS*NZS / NTHREADS;
	int i;
	for (i=0; i<NTHREADS; i++)
	{
		threadargs[i].tid = i;
		threadargs[i].cur_chunk_size = chunk_size;
		threadargs[i].input = input;
		threadargs[i].output = output;
	}
	// give rest items to the last thread
	threadargs[NTHREADS-1].cur_chunk_size += NXS*NYS*NZS - chunk_size * NTHREADS;

	for (i=0; i<NTHREADS; i++)
		pthread_create(&threads[i],NULL,sum,(void*)&threadargs[i]);
	for (i=0; i<NTHREADS; i++)
		pthread_join(threads[i], NULL);

	tstop = second();
    printf("%f, p%d, 0, %d\n", tstop-tstart, NTHREADS, NXS*NYS*NZS);
}

int main (int argc, char* argv[])
{
	int *par_input, *par_output;
	par_input = malloc(NXS*NYS*NZS*sizeof(int));
	par_output = malloc(NXS*NYS*NZS*sizeof(int));

	int i;
	for (i=0; i<NXS*NYS*NZS; i++) { // init arr
		par_input[i] = i;
	}
	parallel_sum(par_input, par_output);	
	return 0;
}
