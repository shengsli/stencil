/**
 * median1d.c
 * @brief	1D median filtering. It assumes NITEMS>=NTHREADS. NITEMS<NTHREADS is not considered. 
 * Usage	gcc -o median1d median1d.c -lpthread -DWIDTH=1 -DNTHREADS=3 -DNITEMS=10 
 *			./median1d
 *			
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

void swap(int *xp, int *yp) 
{
    int temp = *xp;
    *xp = *yp; 
    *yp = temp;
} 

void bubble_sort (int *arr, int size) 
{
	int i, j;
	for (i = 0; i<size-1; i++)
		for (j = 0; j<size-i-1; j++)
			if (arr[j] > arr[j+1])
				swap(&arr[j], &arr[j+1]);
} 

int partition (int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = low-1;
	int j;
    for (j=low; j<=high-1; j++)
    {
        if (arr[j] <= pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quick_sort(int arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition(arr, low, high); 
        quick_sort(arr, low, pi - 1); 
        quick_sort(arr, pi + 1, high); 
    } 
} 

int find_median (int *arr, int size)
{
	quick_sort(arr, 0, size-1);
	if (size%2 != 0) return arr[size/2];
	return (arr[(size-1)/2] + arr[size/2])/2.0;
}

/**
 * This function hard codes filter as a 1x3 filter. 
 */
void sequential_median (int* input, int *output)
{
	int* neighbourhood;
	neighbourhood = malloc((WIDTH*2+1)*sizeof(int));
	int targetIdx;
	for (targetIdx=0; targetIdx<NITEMS; targetIdx++)
	{
		int i;
		for (i=0; i<WIDTH*2+1; ++i)
		{
			neighbourhood[i] = input[(targetIdx+i-WIDTH+NITEMS)%NITEMS];
		}
		output[targetIdx] = find_median(neighbourhood, WIDTH*2+1);
	}
}

void *median (void *args)
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
	   	int *neighbourhood;
		neighbourhood = malloc((WIDTH*2+1)*sizeof(int));
		int chunk_size = NITEMS / NTHREADS; // chunk size of 0..n-1 chunks

		// create neighbourhood
		int j;
		for (j=0; j<WIDTH*2+1; j++)
		{
			neighbourhood[j] = input[(tid*chunk_size+i+NITEMS-WIDTH+j)%NITEMS];
		}
		output[tid*chunk_size+i] = find_median(neighbourhood, (WIDTH*2+1));
	}
}

/**
 * @param input 
 * @param output 
 * @param size is the size of input array.
 */
void parallel_median(int *input, int *output)
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

	// init array
	int i;
	for (i=0; i<NITEMS; i++)
	{
		seq_input[i] = par_input[i] = rand()%10;
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
