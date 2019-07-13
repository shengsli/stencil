/**
 * g++ median2D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=1 -DNROWS=4 -DNCOLS=5 -DPADDING=0 -DNITERS=4 -DOUTPUT -o median2D
 * ./median2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

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
	for (int row=0; row<NROWS; ++row)
	{
		for (int col=0; col<NCOLS; ++col)
		{
			printf("%4d", vec[col+row*NCOLS]);
		}
		std::cout << std::endl;
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
	return find_median(neighbourhood, (2*radius+1)*(2*radius+1));
}

void sequentialMedian(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	// find median
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
    // std::cout << "sequentialMedian, " << tstop-tstart << std::endl;
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
    std::vector<int> seqInput(NROWS*NCOLS);
    std::vector<int> parInput(NROWS*NCOLS);
    for(size_t i = 0; i < NROWS*NCOLS; ++i) {
		seqInput[i] = parInput[i] = i;
	}
    std::vector<int> seqOutput(seqInput.size());
    std::vector<int> parOutput(parInput.size());

#ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("median2Dtest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
		if (i%NCOLS == 0) fprintf(outfile," \n");
		fprintf(outfile,"%d, ", seqInput[i]);
    }
#endif
	
	sequentialMedian(seqOutput, seqInput);
	parallelMedian(parOutput, parInput);
	
#ifdef OUTPUT
    fprintf(outfile,"\nSequential Output: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
		if (i%NCOLS == 0) fprintf(outfile," \n");
		fprintf(outfile,"%d, ", seqOutput[i]);
    }
	fprintf(outfile,"\n");
    fprintf(outfile,"\nParallel Output: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
		if (i%NCOLS == 0) fprintf(outfile," \n");
		fprintf(outfile,"%d, ", parOutput[i]);
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
