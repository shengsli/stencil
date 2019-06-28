#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define P 8

typedef struct arg_pack_tag {
	int id;
	int *arr;
	int arg1;
	int arg2;
	int arg3;
} arg_pack;

typedef arg_pack *argptr;

void print_arr(int *arr)
{
	int i;
	for (i=0;i<P-1;++i)
	{
		printf("%d, ", arr[i]);
	}
	printf("%d\n", arr[P-1]);
}

void seq_add(int *arr)
{
	int *arr2;
	arr2 = malloc(P*sizeof(int));
	memcpy(arr2, arr, P*sizeof(int));

	int i;
	for (i=0;i<P;++i)
	{
		arr[i] = arr2[(i+P-1)%P] + arr2[i] + arr2[(i+P+1)%P];
	}
}

void *add (void *args)
{
	int id,*arr,arg1,arg2,arg3;
	id=((arg_pack*)args)->id;
	arr=((arg_pack*)args)->arr;
	arg1=((arg_pack*)args)->arg1;
	arg2=((arg_pack*)args)->arg2;
	arg3=((arg_pack*)args)->arg3;
	int sum = arg1+arg2+arg3;
	arr[id]=sum;
	printf("this is thread %d. args are %d, %d, %d. sum = %d\n", id, arg1, arg2, arg3, sum);
}

int main (int argc, char* argv[])
{
	// init array
	int *arr1, *arr2;
	arr1 = malloc(P*sizeof(int));
	arr2 = malloc(P*sizeof(int));
	
	int i;
	srand(0);
	for (i=0;i<P;++i)
	{
		arr1[i] = arr2[i] = rand()%10;
	}

	// init value of arr1 and arr2
	printf("init arr: ");
	print_arr(arr1);

	// seq add on arr1
	seq_add(arr1);
	printf("seq_add result: ");
	print_arr(arr1);

	// arg pack
	pthread_t *threads;
	arg_pack *threadargs;
	threads = (pthread_t *) malloc (P * sizeof(pthread_t));
	threadargs  = (arg_pack *) malloc (P * sizeof(arg_pack));
	for (i=0;i<P;++i)
	{
		threadargs[i].id = i;
		threadargs[i].arr = arr2;
		threadargs[i].arg1 = arr2[(i+P-1)%P];
		threadargs[i].arg2 = arr2[i];
		threadargs[i].arg3 = arr2[(i+P+1)%P];
	}
	for (i=0;i<P;++i)
	{
		pthread_create(&threads[i],NULL,add,(void*)&threadargs[i]);
	}
	for (i=0;i<P;++i)
	{
		pthread_join(threads[i], NULL);
	}

	return 0;
}
