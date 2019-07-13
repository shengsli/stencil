/**
 * g++ seqMedian3D.cpp -std=c++11 -O2 -lpthread -DRADIUS=1 -DNXS=3 -DNYS=4 -DNZS=5 -DPADDING=0 -DNITERS=4 -o seqMedian3D
 * ./seqMedian3D
 */

#include <cassert>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <utility>
#include <stdexcept>

#define WRAP_AROUND 0
#define FIXED_VALUE 1
#define REPLICATE_LAST_ELEMENT 2

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


void sequentialMedian(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	int filterSize = 2*RADIUS+1;
	int *neighbourhood = (int *) malloc(filterSize*filterSize*filterSize*sizeof(int));
	int nItems = NXS*NYS*NZS;
	for (int iter=0; iter<NITERS; iter++)
	{
		if (iter>0) { // swap input and output reference
			auto temp = input;
		    input = output;
			output = temp;
		}

		for (int elIdx=0; elIdx<nItems; ++elIdx) {
			int elx = elIdx % NXS;
			int ely = (elIdx / NXS) % NYS;
			int elz = elIdx / NXS / NYS;
			int neighbourx, neighboury, neighbourz;
			int filterx, filtery, filterz;
			int sum=0;
		
			for (int filterIdx=0; filterIdx<filterSize*filterSize*filterSize; ++filterIdx) { // iterate over filter window, loop colaescing 
				filterz = filterIdx / (filterSize*filterSize);
				filtery = filterIdx % (filterSize*filterSize) / filterSize;
				filterx = filterIdx % filterSize;
				switch (PADDING)
				{
				    case WRAP_AROUND:
					{
						neighbourx = (elx+filterx-RADIUS+NXS)%NXS;
						neighboury = (ely+filtery-RADIUS+NYS)%NYS;
						neighbourz = (elz+filterz-RADIUS+NZS)%NZS;
						neighbourhood[filterx+(2*RADIUS+1)*(filtery+(2*RADIUS+1)*filterz)] = input[neighbourx+NXS*(neighboury+NYS*neighbourz)];
						break;
					}
				    case FIXED_VALUE:
						break;
				case REPLICATE_LAST_ELEMENT:
					break;
				default:
					throw std::invalid_argument("Invalid padding option.");
					break;
				}
			}
			output[elIdx] = find_median(neighbourhood, filterSize*filterSize*filterSize);
		}
	}
	
    tstop = second();
	std::cout << tstop-tstart << ", seq, 0, " << NXS*NYS*NZS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NXS*NYS*NZS);
    std::vector<int> seqOutput(seqInput.size());
    for(size_t i=0; i<NXS*NYS*NZS; ++i) {
		seqInput[i] = i;
	}
	sequentialMedian(seqOutput, seqInput);
	return 0;
}
