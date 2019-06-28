#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NTHREADS 8
#define NITEMS 8

typedef struct arg_pack_tag {
	int tid;
	double *arr;
	int size;
	int arg1;
	int arg2;
	int arg3;
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
	double* data;
	data = malloc(3*sizeof(double));
	int i;
	for (i=0; i<NITEMS; i++)
	{
		data[0] = arr[(i-1+NITEMS)%NITEMS];
		data[1] = arr[i];
		data[2] = arr[(i+1+NITEMS)%NITEMS];
		arr[i] = find_median(data, 3);
	}
}

void *median (void *args)
{
	int tid, size;
	double *arr;
	double arg1, arg2, arg3;
	tid=((arg_pack*)args)->tid;
	arr=((arg_pack*)args)->arr;
	size=((arg_pack*)args)->size;
	arg1=((arg_pack*)args)->arg1;
	arg2=((arg_pack*)args)->arg2;
	arg3=((arg_pack*)args)->arg3;
	double* data;
	data = malloc(3*sizeof(double));
	data[0] = arg1;
	data[1] = arg2;
	data[2] = arg3;
	double median = find_median(data, 3);
	arr[tid]=median;
	printf("This is thread %d. Median = %.2f\n", tid, median);
}

void parallel_median(double *arr)
{
	pthread_t *threads;
	arg_pack *threadargs;
	threads = (pthread_t *) malloc (NTHREADS*sizeof(pthread_t));
	threadargs  = (arg_pack *) malloc (NTHREADS*sizeof(arg_pack));
	int i;
	for (i=0; i<NTHREADS; i++)
	{
		threadargs[i].tid = i;
		threadargs[i].arr = arr;
		threadargs[i].arg1 = arr[(i-1+NITEMS)%NITEMS];
		threadargs[i].arg2 = arr[i];
		threadargs[i].arg3 = arr[(i+1+NITEMS)%NITEMS];
	}
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
	double *arr1, *arr2;
	arr1 = malloc(NITEMS*sizeof(double));
	arr2 = malloc(NITEMS*sizeof(double));
	int i;
	srand(0);
	for (i=0; i<NITEMS; i++)
	{
		int num;
		num = rand()%10;
		arr1[i] = arr2[i] = (double) num;
	}

	// print init value of arr1 and arr2
	printf("init arr: ");
	print_arr(arr1, NITEMS);

	// sequential
	sequential_median(arr1);
	printf("sequential_median result: ");
	print_arr(arr1, NITEMS);

	// parallel
	parallel_median(arr2);
	printf("parallel_median result: ");
	print_arr(arr2, NITEMS);

	if (check_result (arr1, arr2, NITEMS))
	{
		printf("Sucess: parallel result matched sequential result.\n");
	}
	else
	{
		printf("FAIL: NOT MATCHED.\n");
	}
	
	return 0;
}
