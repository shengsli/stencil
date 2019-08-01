/**
 * By 2D conduction formula, radius must be 1. 
 * gcc heattransfer2d.c -O2 -lpthread -DRADIUS=1 -DNTHREADS=4 -DNROWS=100 -DNCOLS=100 -DNITERS=20 -DOUTPUT -o heattransfer2d 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct arg_pack_tag {
	int tid;
	int cur_chunk_size;
 	double *input;
	double *output;
} arg_pack;

typedef arg_pack *argptr;

pthread_barrier_t barrier;

void print_arr(char *msg, double *arr, int size)
{
	printf("%s", msg);
	int i;
	for (i=0; i<size-1; i++)
	{
		printf("%.2f, ", arr[i]);
	}
	printf("%.2f\n", arr[i]);
}

double average (double *neighbourhood, int size)
{
	double sum = 0;
	sum = neighbourhood[1] + neighbourhood[3] + neighbourhood[5] + neighbourhood[7];
	return sum/4.; // hard coded 4
}

void sequential_heat_transfer (double* input, double *output)
{
	double *neighbourhood = (double *) malloc((2*RADIUS+1)*(2*RADIUS+1)*sizeof(double));
	int nItems = NROWS*NCOLS;
	
	int iter;
	for (iter=0; iter<NITERS; iter++) {
		if (iter>0) {
			double* temp = input;
		    input = output;
			output = temp;
		}

		int elIdx;
		for (elIdx=0; elIdx<nItems; ++elIdx) {
			int elCol = elIdx % NCOLS;
			int elRow = elIdx / NCOLS;
			int neighbourCol, neighbourRow;

			int row, col;
			for (row=0; row<2*RADIUS+1; ++row) { // iterate over filter window
				for (col=0; col<2*RADIUS+1; ++col)
				{
					neighbourCol = (elCol+col+NCOLS-RADIUS)%NCOLS;
					neighbourRow = (elRow+row+NROWS-RADIUS)%NROWS;
					neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+neighbourRow*NCOLS];
				}
			}
			output[elIdx] = average(neighbourhood, (2*RADIUS+1)*(2*RADIUS+1));
		}
	}
}

void *heat_transfer (void *args)
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
			output[tid*chunk_size+i] = average(neighbourhood, (2*RADIUS+1)*(2*RADIUS+1));
		}
		pthread_barrier_wait(&barrier);
	}
}

void parallel_heat_transfer(double *input, double *output)
{
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
		pthread_create(&threads[i],NULL,heat_transfer,(void*)&threadargs[i]);
	for (i=0; i<NTHREADS; i++)
		pthread_join(threads[i], NULL);
}

int check_result (double *expected_arr, double *arr, int size) {
	int i;
	for (i=0; i<size; i++)
	{
		if (arr[i] != expected_arr[i]) return 0;
	}
	return 1;
}

int main (int argc, char* argv[])
{
	double *seq_input, *seq_output, *par_input, *par_output;
	seq_input = malloc(NROWS*NCOLS*sizeof(double));
	seq_output = malloc(NROWS*NCOLS*sizeof(double));
	par_input = malloc(NROWS*NCOLS*sizeof(double));
	par_output = malloc(NROWS*NCOLS*sizeof(double));

	// init arr
	int i;
	for (i=0; i<NROWS*NCOLS; i++) {
		seq_input[i] = par_input[i] = (double)i;
	}

#ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("heattransfer2d_test.txt","w");
    fprintf(outfile,"Input: ");
    for (i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%.2f, ", seq_input[i]);
    }
#endif
	
	sequential_heat_transfer(seq_input, seq_output);
	parallel_heat_transfer(par_input, par_output);

#ifdef OUTPUT
    fprintf(outfile,"\nSequential Output: ");
    for (i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%.2f, ", seq_output[i]);
    }
	fprintf(outfile,"\n");
    fprintf(outfile,"\nParallel Output: ");
    for (i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%.2f, ", par_output[i]);
    }
	fprintf(outfile,"\n");
	fclose(outfile);

	if (check_result (seq_output, par_output, NROWS*NCOLS))
		printf("Sucess: parallel result matched sequential result.\n");
	else
		printf("FAIL: NOT MATCHED.\n");
#endif
	
	return 0;
}
