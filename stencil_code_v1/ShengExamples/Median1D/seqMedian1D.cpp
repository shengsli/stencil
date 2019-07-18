/**
 * g++ seqMedian1D.cpp -std=c++11 -O2 -lpthread -DRADIUS=2 -DNITEMS=1024 -DNITERS=10 -DPADDING=0 -o seqMedian1D
 * ./seqMedian1D
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

void sequentialMedian(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();
	
	int *neighbourhood = (int *) malloc((RADIUS*2+1)*sizeof(int));	
	int inputSize = input.size();
	for (int iter=0; iter<NITERS; iter++)
	{
		if (iter>0) { // swap input and output reference
			auto temp = input;
		    input = output;
			output = temp;
		}
		
		for (int elementIndex=0; elementIndex<NITEMS; ++elementIndex)
		{
			int median=0;
			for (int i=0; i<RADIUS*2+1; ++i)
			{
				switch (PADDING)
				{
    				case WRAP_AROUND:
						neighbourhood[i] = input[(elementIndex+i-RADIUS+inputSize)%inputSize];
					break;
				    case FIXED_VALUE:
					{
						int idx = elementIndex-RADIUS+i;
						if (idx >= 0 && idx < inputSize)
							neighbourhood[i]=input[idx];
						else
							neighbourhood[i]=0;
						break;
					}
    				case REPLICATE_LAST_ELEMENT:
					{
						int idx = elementIndex-RADIUS+i;
						if (idx >= 0 && idx < inputSize)
							neighbourhood[i]=input[idx];
						else if (idx<0)
							neighbourhood[i]=input[0];
						else if (idx>=inputSize)
							neighbourhood[i]=input[inputSize-1];
						break;
					}
				    default:
						throw std::invalid_argument("Invalid padding option.");
						break;
				}
			}
			output[elementIndex] = findMedian(neighbourhood, RADIUS*2+1);
		}
	}
	
    tstop = second();
	std::cout << tstop-tstart << ", 0, 0, " << NITEMS << std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NITEMS);
    std::vector<int> seqOutput(seqInput.size());
    for(size_t i = 0; i<NITEMS; ++i)
    {
		seqInput[i] = i;
	}
	sequentialMedian(seqOutput, seqInput);
	return 0;
}
