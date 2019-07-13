/**
 * g++ seqMedian2D.cpp -std=c++11 -O2 -lpthread -DRADIUS=1 -DNROWS=4 -DNCOLS=5 -DPADDING=0 -DNITERS=4 -o seqMedian2D
 * ./seqMedian2D
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
	int *neighbourhood = (int *) malloc(filterSize*filterSize*sizeof(int));
	int nItems = NROWS*NCOLS;
	for (int iter=0; iter<NITERS; iter++) {
		if (iter>0) { // NEW
			auto temp = input;
		    input = output;
			output = temp;
		}
		
		for (int elIdx=0; elIdx<nItems; ++elIdx) {
			int elCol = elIdx % NCOLS;
			int elRow = elIdx / NCOLS;
			int neighbourCol, neighbourRow;
			int col, row;

			for (int filterIdx=0; filterIdx<filterSize*filterSize; filterIdx++) { // iterate over filter window, loop coalescing
				row = filterIdx / filterSize;
				col = filterIdx % filterSize;
				switch (PADDING) {
				    case WRAP_AROUND:
					{
						neighbourCol = (elCol+col+NCOLS-RADIUS)%NCOLS;
						neighbourRow = (elRow+row+NROWS-RADIUS)%NROWS;
						neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+neighbourRow*NCOLS];
						break;
					}
				    case FIXED_VALUE:
					{
						neighbourCol = elCol+col-RADIUS;
						neighbourRow = elRow+row-RADIUS;
						if (neighbourCol<0 || neighbourCol>=NCOLS || neighbourRow<0 || neighbourRow>=NROWS)
							neighbourhood[col+row*(2*RADIUS+1)] = 0;
						else
							neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+neighbourRow*NCOLS];
						break;
					}
				    case REPLICATE_LAST_ELEMENT:
					{
						neighbourCol = elCol+col-RADIUS;
						neighbourRow = elRow+row-RADIUS;
						if ((neighbourCol<0 || neighbourCol>=NCOLS) && (neighbourRow<0 || neighbourRow>=NROWS))
							neighbourhood[col+row*(2*RADIUS+1)] = 0;
						else if (neighbourCol<0)
							neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourRow*NCOLS];
						else if (neighbourCol>=NCOLS)
							neighbourhood[col+row*(2*RADIUS+1)] = input[(neighbourRow+1)*NCOLS-1];
						else if (neighbourRow<0)
							neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol];
						else if (neighbourRow>=NROWS)
							neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+(NROWS-1)*NCOLS];
						else
							neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+neighbourRow*NCOLS];
						break;
					}
				    default:
						throw std::invalid_argument("Invalid padding option.");
						break;
				}
			}
			output[elIdx] = find_median(neighbourhood, filterSize*filterSize);
		}
	}
	
    tstop = second();
    std::cout << tstop-tstart << ", seq, 0, " << NROWS*NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NROWS*NCOLS);
    std::vector<int> seqOutput(seqInput.size());
    for(size_t i = 0; i < NROWS*NCOLS; ++i) {
		seqInput[i] = i;
	}	
	sequentialMedian(seqOutput, seqInput);
	return 0;
}
