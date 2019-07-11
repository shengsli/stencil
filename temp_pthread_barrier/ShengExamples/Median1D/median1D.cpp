/**
 * g++ median1D.cpp -std=c++11 -O2 -lpthread -DRADIUS=2 -DNTHREADS=4 -DNITEMS=1024 -DNITERS=10 -DNDATABLOCKS=100 -DPADDING=0 -DOUTPUT -o median1D
 * ./median1D
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

int stencilkernel (int neighbourhood[], int radius)
{
	return find_median(neighbourhood, radius*2+1);
}

void sequentialMedian(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	// find median
	int *neighbourhood = (int *) malloc((RADIUS*2+1)*sizeof(int));
	
	int inputSize = input.size();
	for (int iter=0; iter<NITERS; iter++)
	{
		if (iter>0)
		{
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
					}
					break;
				case REPLICATE_LAST_ELEMENT:
					{
						int idx = elementIndex-RADIUS+i;
						if (idx >= 0 && idx < inputSize)
							neighbourhood[i]=input[idx];
						else if (idx<0)
							neighbourhood[i]=input[0];
						else if (idx>=inputSize)
							neighbourhood[i]=input[inputSize-1];
					}
					break;
				default:
					throw std::invalid_argument("Invalid padding option.");
					break;
				}
			}
			output[elementIndex] = find_median(neighbourhood, RADIUS*2+1);
		}
	}
	
    tstop = second();
    std::cout << "sequentialMedian, " << tstop-tstart << std::endl;
}

void parallelMedian(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil = Stencil1D(stencilkernel, RADIUS, PADDING, NITERS, NTHREADS);
    stencil(output, input);
	
    tstop = second();
    std::cout << "parallelMedian, " << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NITEMS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NITEMS);
    std::vector<int> parInput(NITEMS);
    for(size_t i = 0; i < NITEMS; ++i)
    {
		seqInput[i] = parInput[i] = i;
	}
    std::vector<int> seqOutput(seqInput.size());
    std::vector<int> parOutput(parInput.size());

    #ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("median1Dtest.txt","w");
    fprintf(outfile,"seqInput: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", seqInput[i]);
    }
	#endif

	sequentialMedian(seqOutput, seqInput);
	parallelMedian(parOutput, parInput);
	
    #ifdef OUTPUT
    fprintf(outfile,"\nSequential Output: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", seqOutput[i]);
    }
    fprintf(outfile,"\nParallel Output: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", parOutput[i]);
    }
	fprintf(outfile,"\n");
	fclose(outfile);
    #endif

	if (compareResult(seqOutput, parOutput))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;
	return 0;
}
