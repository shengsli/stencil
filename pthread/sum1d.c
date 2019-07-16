/**
 * gcc sum1d.c -O2 -lpthread -DRADIUS=1 -DNTHREADS=4 -DNITEMS=100 -DOUTPUT -o sum1d 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct arg_pack_tag {
	int tid;
	int cur_chunk_size;
 	int *input;
	int *output;
} arg_pack;

typedef arg_pack *argptr;

void print_arr(char *msg, int *arr, int size)
{
	printf("%s", msg);
	int i;
	for (i=0; i<size-1; i++)
	{
		printf("%d, ", arr[i]);
	}
	printf("%d\n", arr[i]);
}

void sequential_sum (int* input, int *output)
{
	int elementIndex;
	for (elementIndex=0; elementIndex<NITEMS; elementIndex++) {
		int sum=0;
		int i;
		for (i=0; i<RADIUS*2+1; ++i) {
		    sum += input[(elementIndex+i-RADIUS+NITEMS)%NITEMS];
		}
		output[elementIndex] = sum;
	}
}

void *sum (void *args)
{
	int tid, cur_chunk_size;
	int *input, *output;
	tid=((arg_pack*)args)->tid;
	cur_chunk_size=((arg_pack*)args)->cur_chunk_size;
	input=((arg_pack*)args)->input;
	output=((arg_pack*)args)->output;

	// iterate over data chunk
	int i;
	for (i=0; i<cur_chunk_size; i++)
	{
		int chunk_size = NITEMS / NTHREADS; // chunk size of 0..n-1 chunks
		int sum=0;
		int j;
		for (j=0; j<RADIUS*2+1; j++)
		{
		    sum += input[(tid*chunk_size+i+NITEMS-RADIUS+j)%NITEMS];
		}
		output[tid*chunk_size+i] = sum;
	}
}

/**
 * @param input 
 * @param output 
 * @param size is the size of input array.
 */
void parallel_sum(int *input, int *output)
{
	pthread_t *threads;
	arg_pack *threadargs;
	threads = (pthread_t *) malloc(NTHREADS*sizeof(pthread_t));
	threadargs  = (arg_pack *) malloc(NTHREADS*sizeof(arg_pack));

	int chunk_size = NITEMS / NTHREADS;
	int i;
	for (i=0; i<NTHREADS; i++)
	{
		threadargs[i].tid = i;
		threadargs[i].cur_chunk_size = chunk_size;
		threadargs[i].input = input;
		threadargs[i].output = output;
	}
	// give rest items to the last thread
	threadargs[NTHREADS-1].cur_chunk_size += NITEMS - chunk_size * NTHREADS;

	for (i=0; i<NTHREADS; i++)
		pthread_create(&threads[i],NULL,sum,(void*)&threadargs[i]);
	for (i=0; i<NTHREADS; i++)
		pthread_join(threads[i], NULL);
}

int check_result (int *expected_arr, int *arr, int size) {
	int i;
	for (i=0; i<size; i++)
	{
		if (arr[i] != expected_arr[i]) return 0;
	}
	return 1;
}

int main (int argc, char* argv[])
{
	int *seq_input, *seq_output, *par_input, *par_output;
	seq_input = malloc(NITEMS*sizeof(int));
	seq_output = malloc(NITEMS*sizeof(int));
	par_input = malloc(NITEMS*sizeof(int));
	par_output = malloc(NITEMS*sizeof(int));

	// init arr
	int i;
	for (i=0; i<NITEMS; i++) {
		seq_input[i] = par_input[i] = i;
	}

#ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("sum1d_test.txt","w");
    fprintf(outfile,"seq_input: ");
    for (i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", seq_input[i]);
    }
#endif
	
	sequential_sum(seq_input, seq_output);
	parallel_sum(par_input, par_output);

#ifdef OUTPUT
    fprintf(outfile,"\nSequential Output: ");
    for (i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", seq_output[i]);
    }
    fprintf(outfile,"\nParallel Output: ");
    for (i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", par_output[i]);
    }
	fprintf(outfile,"\n");
	fclose(outfile);

	if (check_result (seq_output, par_output, NITEMS))
		printf("Sucess: parallel result matched sequential result.\n");
	else
		printf("FAIL: NOT MATCHED.\n");
#endif
	
	return 0;
}
