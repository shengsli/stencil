/**
 * g++ median3D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=1 -DNXS=3 -DNYS=3 -DNZS=3 -DPADDING=0 -DNITERS=1 -DOUTPUT -o median3D
 * ./median3D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil3D.hpp"

void print_arr(int *arr, int size)
{
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
	for (int z=0; z<NZS; ++z)
	{
		for (int y=0; y<NYS; ++y)
		{
			for (int x=0; x<NXS; ++x)
			{
				printf("%d, ", vec[x+NXS*(y+NYS*z)]);
			}
			std::cout << std::endl;
		}
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
	return find_median(neighbourhood, (RADIUS*2+1)*(RADIUS*2+1)*(RADIUS*2+1));
}

void sequentialMedian(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	// median
	int filterSize = 2*RADIUS+1;
	int *neighbourhood = (int *) malloc(filterSize*filterSize*filterSize*sizeof(int));
	int nItems = NXS*NYS*NZS;
	for (int iter=0; iter<NITERS; iter++)
	{
		if (iter>0) { // NEW
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
    // std::cout << "sequentialSum, " << tstop-tstart << std::endl;
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
    std::vector<int> seqInput(NXS*NYS*NZS);
    std::vector<int> parInput(NXS*NYS*NZS);
    for(size_t i=0; i<NXS*NYS*NZS; ++i)
    {
		seqInput[i] = parInput[i] = i;
	}
    std::vector<int> seqOutput(seqInput.size());
    std::vector<int> parOutput(parInput.size());

    #ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("median3Dtest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i=0; i<NXS*NYS*NZS; i++) {
      if (i%NXS == 0) fprintf(outfile," \t");
      if (i%(NXS*NYS) == 0) fprintf(outfile," \n");
      fprintf(outfile,"%5d, ", seqInput[i]);
    }
	#endif
	
	sequentialMedian(seqOutput, seqInput);
	parallelMedian(parOutput, parInput);

	#ifdef OUTPUT
    fprintf(outfile,"\nSequential Output: ");
    for (size_t i=0; i<NXS*NYS*NZS; i++) {
      if (i%NXS == 0) fprintf(outfile," \t");
      if (i%(NXS*NYS) == 0) fprintf(outfile," \n");
      fprintf(outfile,"%5d, ", seqOutput[i]);
    }
	fprintf(outfile,"\n");
    fprintf(outfile,"\nParallel Output: ");
    for (size_t i=0; i<NXS*NYS*NZS; i++) {
      if (i%NXS == 0) fprintf(outfile," \t");
      if (i%(NXS*NYS) == 0) fprintf(outfile," \n");
      fprintf(outfile,"%5d, ", parOutput[i]);
    }
	fprintf(outfile,"\n");
	fclose(outfile);

	if (compareResult(seqOutput, parOutput))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;
    #endif
	
	return 0;
}
