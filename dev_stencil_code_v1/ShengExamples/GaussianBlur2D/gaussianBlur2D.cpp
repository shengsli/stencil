/**
 * gaussian filter is hardcoded as a 5x5 filter. radius must be 2. 
 * g++ gaussianBlur2D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=2 -DNROWS=6 -DNCOLS=6 -DPADDING=0 -DNITERS=4 -DOUTPUT -o gaussianBlur2D
 * ./gaussianBlur2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

void print_arr(double *arr, double size)
{
	int i;
	for (i=0; i<size-1; i++)
	{
		printf("%.2f, ", arr[i]);
	}
	printf("%.2f\n", arr[i]);
}


bool compareResult(const std::vector<double> &vec1, const std::vector<double> &vec2)
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

void printVector(const std::vector<double> &vec)
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

double conv(double *lhs, double *rhs, int size)
{
	double sum=0.;
	for (int i=0; i<size; ++i) {
		sum += lhs[i]*rhs[i];
	}
	return sum;
}

double filter[] = {1./273, 4./273, 7./273, 4./273, 1./273, 
				   4./273,16./273,26./273,16./273, 4./273,
				   7./273,26./273,41./273,26./273, 7./273,
				   4./273,16./273,26./273,16./273, 4./273,
				   1./273, 4./273, 7./273, 4./273, 1./273};
double stencilkernel (double neighbourhood[], int radius)
{
	return conv(neighbourhood, filter, (2*radius+1)*(2*radius+1));
}

void sequentialMedian(std::vector<double> &output, std::vector<double> &input)
{
	double tstart, tstop;
    tstart = second();

	// find median
	int filterSize = 2*RADIUS+1;
	double *neighbourhood = (double *) malloc(filterSize*filterSize*sizeof(double));
	int nItems = NROWS*NCOLS;
	for (int iter=0; iter<NITERS; iter++) {
		if (iter>0) { // NEW
			auto temp = input;
		    input = output;
			output = temp;
		}
		
		for (int elIdx=0; elIdx<nItems; ++elIdx)
		{
			int elCol = elIdx % NCOLS;
			int elRow = elIdx / NCOLS;
			int neighbourCol, neighbourRow;
			int row, col;

			for (int filterIdx=0; filterIdx<filterSize*filterSize; ++filterIdx) { // iterate over filter window, loop coalescing
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
			output[elIdx] = conv(neighbourhood,filter,(RADIUS*2+1)*(RADIUS*2+1));
		}
	}
	
    tstop = second();
    std::cout << "sequentialMedian, " << tstop-tstart << std::endl;
}

void parallelMedian(std::vector<double> &output, std::vector<double> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil2d = Stencil2D(stencilkernel, RADIUS, NROWS, NCOLS, PADDING, NITERS, NTHREADS);
    stencil2d(output, input);
	
    tstop = second();
    std::cout << "parallelMedian, " << tstop-tstart << ", " << RADIUS << ", " << NTHREADS  << ", " << NROWS << ", " << NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<double> seqInput(NROWS*NCOLS);
    std::vector<double> parInput(NROWS*NCOLS);
    for(size_t i = 0; i < NROWS*NCOLS; ++i)
    {
		seqInput[i] = parInput[i] = i;
	}
    std::vector<double> seqOutput(seqInput.size());
    std::vector<double> parOutput(parInput.size());

    #ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("gaussianBlur2Dtest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%.2f, ", seqInput[i]);
    }
	#endif
	
	sequentialMedian(seqOutput, seqInput);
	parallelMedian(parOutput, parInput);
	
    #ifdef OUTPUT
    fprintf(outfile,"\nSequential Output: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%.2f, ", seqOutput[i]);
    }
	fprintf(outfile,"\n");
    fprintf(outfile,"\nParallel Output: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%.2f, ", parOutput[i]);
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
