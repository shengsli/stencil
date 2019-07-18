/**
 * g++ median3D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=1 -DNXS=3 -DNYS=4 -DNZS=5 -DPADDING=0 -DNITERS=4 -o median3D
 * ./median3D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil3D.hpp"

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

int findMedian (int *arr, int size)
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
	return findMedian(neighbourhood, (RADIUS*2+1)*(RADIUS*2+1)*(RADIUS*2+1));
}

void parallelMedian(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil3d = Stencil3D(stencilkernel, RADIUS, NXS, NYS, NZS, PADDING, NITERS, NTHREADS);
    stencil3d(output, input);
	
    tstop = second();
    std::cout << tstop-tstart << ", " << NTHREADS  << ", " << NDATABLOCKS << ", " << NXS*NYS*NZS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> parInput(NXS*NYS*NZS);
    std::vector<int> parOutput(parInput.size());
    for(size_t i=0; i<NXS*NYS*NZS; ++i) {
	    parInput[i] = i;
	}
	parallelMedian(parOutput, parInput);	
	return 0;
}
