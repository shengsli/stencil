/**
 * 1D median filtering. It assumes NITEMS>=NTHREADS. NITEMS<NTHREADS is not considered. 
 * gcc median1d.c -O2 -lpthread -DRADIUS=1 -DNTHREADS=4 -DNITEMS=100 -DNITERS=20 -DOUTPUT -o median1d 
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

pthread_barrier_t barrier;

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

void sequential_median (int* input, int *output)
{
	int* neighbourhood;
	neighbourhood = malloc((RADIUS*2+1)*sizeof(int));

	int iter;
	for (iter=0; iter<NITERS; iter++) {
		if (iter>0) {
			int* temp = input;
		    input = output;
			output = temp;
		}

		int elIdx;
		for (elIdx=0; elIdx<NITEMS; elIdx++) {
			int i;
			for (i=0; i<RADIUS*2+1; ++i) {
				neighbourhood[i] = input[(elIdx+i-RADIUS+NITEMS)%NITEMS];
			}
			output[elIdx] = find_median(neighbourhood, RADIUS*2+1);
		}
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

	int iter;
	for (iter=0; iter<NITERS; iter++) {
		if (iter>0) {
			int* temp = input;
		    input = output;
			output = temp;
		}
	
		int i;
		for (i=0; i<cur_chunk_size; i++) { // iterate over data chunk
			int *neighbourhood;
			neighbourhood = malloc((RADIUS*2+1)*sizeof(int));
			int chunk_size = NITEMS / NTHREADS; // chunk size of 0..n-1 chunks

			int j;
			for (j=0; j<RADIUS*2+1; j++) { // create neighbourhood
				neighbourhood[j] = input[(tid*chunk_size+i+NITEMS-RADIUS+j)%NITEMS];
			}
			output[tid*chunk_size+i] = find_median(neighbourhood, (RADIUS*2+1));
		}
		pthread_barrier_wait(&barrier);
	}
}

void parallel_median(int *input, int *output)
{
	pthread_barrier_init(&barrier, NULL, NTHREADS);
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

	// init arr
	int i;
	for (i=0; i<NITEMS; i++) {
		seq_input[i] = par_input[i] = i;
	}

#ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("median1d_test.txt","w");
    fprintf(outfile,"seq_input: ");
    for (i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", seq_input[i]);
    }
#endif
	
	sequential_median(seq_input, seq_output);
	parallel_median(par_input, par_output);

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
