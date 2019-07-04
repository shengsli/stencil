/**
 * g++ median.cpp -std=c++11 -O2 -lpthread -DWIDTH=2 -DNTHREADS=4 -DNITEMS=1024 -DITERMAX=1000 -DNDATABLOCKS=100 -DOUTPUT -o median
 * ./median
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil1D.hpp"

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
    for (int j=low; j<=high-1; j++)
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
  
void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition(arr, low, high); 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
} 

int find_median (int *arr, int size)
{
	quickSort(arr, 0, size-1);
	if (size%2 != 0) return arr[size/2];
	return (arr[(size-1)/2] + arr[size/2])/2;
}

bool compareResult(const std::vector<int> &vec1, const std::vector<int> &vec2)
{
	if (vec1.size() != vec2.size())
		return false;
	auto it1 = vec1.begin();
	auto it2 = vec2.begin();
	for (; it1 != vec1.end(); ++it1)
	{
		if (*it1 != *it2)
			return false;
		++it2;
	}
	return true;
}

void printVector(const std::vector<int> &vec)
{
	auto it = vec.begin();
	for (; it != vec.end(); ++it)
	{
		std::cout << *it << std::endl;
	}
}

double second()
{
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

int stencilkernel (int neighbourhood[], int width)
{
	return find_median(neighbourhood, width*2+1);
}

void sequentialMedian(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	// find median
	int *neighbourhood = (int *) malloc((WIDTH*2+1)*sizeof(int));
	
	int inputSize = input.size();
	for (int targetIdx=0; targetIdx<NITEMS; ++targetIdx)
	{
		int median=0;
		for (int i=0; i<WIDTH*2+1; ++i)
		{
			neighbourhood[i] = input[(targetIdx+i-WIDTH+inputSize)%inputSize];
		}
		output[targetIdx] = find_median(neighbourhood, WIDTH*2+1);
	}
	
    tstop = second();
    std::cout << "sequentialMedian, " << tstop-tstart << std::endl;
}

void parallelMedian(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil = Stencil1D(stencilkernel, WIDTH, NTHREADS);
    stencil(output, input);
	
    tstop = second();
    std::cout << "parallelMedian, " << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << NITEMS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> input(NITEMS);
    for(size_t i = 0; i < NITEMS; ++i)
    {
		input[i] = i;
	}
    std::vector<int> seqOutput(input.size());
    std::vector<int> parOutput(input.size());

	sequentialMedian(seqOutput, input);
	parallelMedian(parOutput, input);
	
    #ifdef OUTPUT
    // Output results
	FILE *outfile;
    outfile = fopen("mediantest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%4d", input[i]);
    }

    fprintf(outfile,"\nSequential Output: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%4d", seqOutput[i]);
    }

    fprintf(outfile,"\nParallel Output: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%4d", parOutput[i]);
    }
	fprintf(outfile," \n");
	fclose(outfile);
    #endif

	if (compareResult(seqOutput, parOutput))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;
	return 0;
}
