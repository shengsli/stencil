#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NTHREADS 3
#define NITEMS 10

typedef struct arg_pack_tag {
	int tid;
	int cur_chunk_size;
 	double *input;
	double *output;
} arg_pack;

typedef arg_pack *argptr;

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

void swap(double *xp, double *yp) 
{ 
    double temp = *xp; 
    *xp = *yp; 
    *yp = temp;
} 

void bubble_sort (double *arr, int size) 
{
	int i, j;
	for (i = 0; i<size-1; i++)
		for (j = 0; j<size-i-1; j++)
			if (arr[j] > arr[j+1])
				swap(&arr[j], &arr[j+1]);
} 

double find_median (double *arr, int size)
{
	bubble_sort(arr, size);
	if (size%2 != 0) return arr[size/2];
	return (arr[(size-1)/2] + arr[size/2])/2.0;
}

/**
 * This function hard codes filter as a 1x3 filter. 
 */
void sequential_median (double* input, double *output)
{
	int radius = 1;
	int window_size = radius*2 + 1;
	double* neighbourhood;
	neighbourhood = malloc(window_size*sizeof(double));
	int i;
	for (i=0; i<NITEMS; i++)
	{
		neighbourhood[0] = input[(i-1+NITEMS)%NITEMS];
		neighbourhood[1] = input[i];
		neighbourhood[2] = input[(i+1+NITEMS)%NITEMS];
		double median = find_median(neighbourhood, window_size);
		output[i] = median;
	}
}

void *median (void *args)
{
	int tid, cur_chunk_size;
	double *input, *output;
	tid=((arg_pack*)args)->tid;
	cur_chunk_size=((arg_pack*)args)->cur_chunk_size;
	input=((arg_pack*)args)->input;
	output=((arg_pack*)args)->output;

	// TODO: loop over data chunk
	int i;
	for (i=0; i<cur_chunk_size; i++)
	{
		int radius = 1;
		int window_size = radius*2 + 1;
	   	double *neighbourhood;
		neighbourhood = malloc(window_size*sizeof(double));
		int chunk_size = NITEMS / NTHREADS; // chunk size of 0..n-1 chunks
		double *cur = input+tid*chunk_size+i;
		neighbourhood[0] = input[(tid*chunk_size+i+NITEMS-1)%NITEMS];
		neighbourhood[1] = input[tid*chunk_size+i];
		neighbourhood[2] = input[(tid*chunk_size+i+NITEMS+1)%NITEMS];
		double median = find_median(neighbourhood, window_size);
		output[tid*chunk_size+i] = median;
	}
}

/**
 * \param input 
 * \param output 
 * \param size is the size of input array.
 */
void parallel_median(double *input, double *output)
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
		pthread_create(&threads[i],NULL,median,(void*)&threadargs[i]);
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
	seq_input = malloc(NITEMS*sizeof(double));
	seq_output = malloc(NITEMS*sizeof(double));
	par_input = malloc(NITEMS*sizeof(double));
	par_output = malloc(NITEMS*sizeof(double));

	// init array
	int i;
	srand(0);
	for (i=0; i<NITEMS; i++)
	{
		int num;
		num = rand()%10;
		seq_input[i] = par_input[i] = (double) num;
	}

	// print init value of seq_input and par_input
	print_arr("init seq_input:  ", seq_input, NITEMS);
	print_arr("init par_input:  ", par_input, NITEMS);
	print_arr("init seq_output: ", seq_output, NITEMS);
	print_arr("init par_output: ", par_output, NITEMS);

	sequential_median(seq_input, seq_output); // sequential version median filtering
	print_arr("seq_output: ", seq_output, NITEMS);

	parallel_median(par_input, par_output); // parallel version median filtering
	print_arr("par_output: ", par_output, NITEMS);

	if (check_result (seq_output, par_output, NITEMS))
		printf("Sucess: parallel result matched sequential result.\n");
	else
		printf("FAIL: NOT MATCHED.\n");
	return 0;
}
