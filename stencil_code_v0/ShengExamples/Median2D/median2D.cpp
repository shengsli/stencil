/**
 * g++ median2D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=1 -DNROWS=4 -DNCOLS=5 -DPADDING=0 -DNITERS=4 -DOUTPUT -o median2D
 * ./median2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

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

double second()
{
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

int stencilkernel (int neighbourhood[], int radius)
{
	return find_median(neighbourhood, (2*radius+1)*(2*radius+1));
}

void parallelMedian(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil2d = Stencil2D(stencilkernel, RADIUS, NROWS, NCOLS, PADDING, NITERS, NTHREADS);
    stencil2d(output, input);
	
    tstop = second();
    std::cout << tstop-tstart << ", " << NTHREADS  << ", " << NDATABLOCKS << ", " << NROWS*NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> parInput(NROWS*NCOLS); 
    std::vector<int> parOutput(parInput.size());
	for(size_t i = 0; i < NROWS*NCOLS; ++i)
    {
	    parInput[i] = i;
	}
	parallelMedian(parOutput, parInput);
	return 0;
}
