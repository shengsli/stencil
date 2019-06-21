#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NTHREADS 8
#define NITEMS 8

typedef struct arg_pack_tag {
	int tid;
	int chunk_size;
 	double *input; // a pointer to the first element of input array
	double *output; // a pointer to the first element of output array
} arg_pack;

typedef arg_pack *argptr;

void print_arr(double *arr, int size)
{
	int i;
	for (i=0; i<size-1; i++)
	{
		printf("%.2f, ", arr[i]);
	}
	printf("%.2f\n", arr[i]);
}

void sequential_add(double *arr, int size)
{
	double *arr2;
	arr2 = malloc(NTHREADS*sizeof(double));
	memcpy(arr2, arr, NTHREADS*sizeof(double));

	int i;
	for (i=0; i<NTHREADS; i++)
	{
		arr[i] = arr2[(i+NTHREADS-1)%NTHREADS] + arr2[i] + arr2[(i+NTHREADS+1)%NTHREADS];
	}
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
void sequential_median (double *arr)
{
	int radius = 1;
	int padding_size = 1;
	int window_size = radius*2 + 1;
	double* neighbourhood;
	neighbourhood = malloc(window_size+padding_size*2*sizeof(double));
	int i;
	for (i=0; i<NITEMS; i++)
	{
		int tmp_idx;
		tmp_idx = (i-1 < 0) ? 0 : i-1;
		neighbourhood[0] = arr[tmp_idx];
		neighbourhood[1] = arr[i];
		tmp_idx = (i+1 > NITEMS-1) ? NITEMS-1 : i+1;
		neighbourhood[2] = arr[tmp_idx];
		arr[i] = find_median(neighbourhood, window_size);
	}
}

void *median (void *args)
{
	int tid, chunk_size;
	double *input, *output;
	tid=((arg_pack*)args)->tid;
	chunk_size=((arg_pack*)args)->chunk_size;
	input=((arg_pack*)args)->input;
	output=((arg_pack*)args)->output;

	// TODO: loop over data chunk
	int i;
	for (i=0; i<chunk_size; i++)
	{
		int radius = 1;
		int padding_size = 1;
		int window_size = radius*2 + 1;
	   	double *neighbourhood;
		neighbourhood = malloc(window_size+padding_size*2*sizeof(double));
		neighbourhood[0] = *(input+i-1);
		neighbourhood[1] = *input;
		neighbourhood[2] = *(input+i+1);
		printf("%d: %.2f\n", tid, neighbourhood[0]);
		double median = find_median(neighbourhood, window_size);
		*output=median;
		// printf("%d: ", tid);
		// print_arr(neighbourhood, 3);
	}
}

/**
 * \param input points to the array to process. 
 * \param output points to the ouput array.
 * \param size is the size of input array.
 */
void parallel_median(double *input, double *output, int size)
{
	pthread_t *threads;
	arg_pack *threadargs;
	threads = (pthread_t *) malloc(NTHREADS*sizeof(pthread_t));
	threadargs  = (arg_pack *) malloc(NTHREADS*sizeof(arg_pack));

	int chunk_size = size / NTHREADS;
	int i;
	for (i=0; i<NTHREADS; i++)
	{
		threadargs[i].tid = i;
		threadargs[i].chunk_size = chunk_size;
		threadargs[i].input = input + chunk_size*i;
		threadargs[i].output = output + chunk_size*i;
	}
	// give rest items to the last thread
	threadargs[NTHREADS-1].chunk_size += size - chunk_size * NTHREADS;

	for (i=0; i<NTHREADS; i++)
	{
		pthread_create(&threads[i],NULL,median,(void*)&threadargs[i]);
	}
	for (i=0; i<NTHREADS; i++)
	{
		pthread_join(threads[i], NULL);
	}
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
	double *seq_arr, *input_par_arr, *output_par_arr;
	seq_arr = malloc(NITEMS*sizeof(double));
	input_par_arr = malloc(NITEMS*sizeof(double));
	output_par_arr = malloc(NITEMS*sizeof(double));

	// init array
	int i;
	srand(0);
	for (i=0; i<NITEMS; i++)
	{
		int num;
		num = rand()%10;
		seq_arr[i] = input_par_arr[i] = (double) num;
	}

	// print init value of seq_arr and input_par_arr
	printf("init seq_arr: ");
	print_arr(seq_arr, NITEMS);
	printf("init input_par_arr: ");
	print_arr(input_par_arr, NITEMS);
	printf("init output_par_arr: ");
	print_arr(output_par_arr, NITEMS);

	// sequential version median filtering
	sequential_median(seq_arr);
	printf("sequential_median result: ");
	print_arr(seq_arr, NITEMS);

	// parallel version median filtering
	parallel_median(input_par_arr, output_par_arr, NITEMS);
	printf("parallel_median result: ");
	print_arr(output_par_arr, NITEMS);

	if (check_result (seq_arr, output_par_arr, NITEMS))
	{
		printf("Sucess: parallel result matched sequential result.\n");
	}
	else
	{
		printf("FAIL: NOT MATCHED.\n");
	}
	return 0;
}
