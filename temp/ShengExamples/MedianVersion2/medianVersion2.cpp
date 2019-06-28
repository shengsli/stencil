/**
 * g++ medianVersion2.cpp -std=c++11 -O2 -lpthread -DWIDTH=2 -DNTHREADS=4 -DSIZE=1024 -DITERMAX=1000 -DNDATABLOCKS=100 -DOUTPUT -o medianVersion2
 * ./medianVersion2
 */

#include <cassert>
#include <sys/time.h>
#include <algorithm>

#include "../../Stencil.hpp"

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

/**
 * size is an odd number in this example. 
 */
int find_median (int *arr, int size)
{
	bubble_sort(arr, size);
	if (size%2 != 0) return arr[size/2];
	return (arr[(size-1)/2] + arr[size/2])/2; // size is odd, this line will not be executed. 
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

/**
 * size is an odd number in this example. 
 */
int findMedian (std::vector<int> &neighbourhood, int size)
{
	std::sort(neighbourhood.begin(), neighbourhood.end());
	if (size%2 != 0) return neighbourhood[size/2];
	return (neighbourhood[(size-1)/2] + neighbourhood[size/2])/2; // size is odd, this line will not be executed. 
}

int stencilkernel (std::vector<int> &neighbourhood, int width)
{
	return findMedian(neighbourhood, width*2+1);
}

void sequentialMedian(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	// TODO
	int *neighbourhood = (int *) malloc((WIDTH*2+1)*sizeof(int));
	
	int inputSize = input.size();
	for (int targetIdx=0; targetIdx<SIZE; ++targetIdx)
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
	
    auto stencil = Stencil(stencilkernel, WIDTH, NTHREADS);
    stencil(output, input);
	
    tstop = second();
    std::cout << "parallelMedian, " << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << SIZE <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> input(SIZE);
    for(size_t i = 0; i < SIZE; ++i)
    {
		input[i] = i;
	}
    std::vector<int> seqOutput(input.size());
    std::vector<int> parOutput(input.size());

	sequentialMedian(seqOutput, input);
	parallelMedian(parOutput, input);
	
	if (compareResult(seqOutput, parOutput))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;
	return 0;
}
