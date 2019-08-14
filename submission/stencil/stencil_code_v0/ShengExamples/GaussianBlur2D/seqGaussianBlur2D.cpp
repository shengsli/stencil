/**
 * Gaussian filter is hardcoded as a 5x5 filter. Radius must be 2. 
 * g++ seqGaussianBlur2D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=2 -DNROWS=6 -DNCOLS=6 -DPADDING=0 -DNITERS=4 -o seqGaussianBlur2D
 * ./seqGaussianBlur2D
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

double filter[] = {1./273, 4./273, 7./273, 4./273, 1./273, 
				   4./273,16./273,26./273,16./273, 4./273,
				   7./273,26./273,41./273,26./273, 7./273,
				   4./273,16./273,26./273,16./273, 4./273,
				   1./273, 4./273, 7./273, 4./273, 1./273};

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

void sequentialGaussianBlur(std::vector<double> &output, std::vector<double> &input)
{
	double tstart, tstop;
    tstart = second();

	// gaussian blur
	int filterSize = 2*RADIUS+1;
	double *neighbourhood = (double *) malloc(filterSize*filterSize*sizeof(double));
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

			for (int row=0; row<2*RADIUS+1; ++row) { // iterate over filter window
				for (int col=0; col<2*RADIUS+1; ++col) {
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
				
			}
			output[elIdx] = conv(neighbourhood,filter,(RADIUS*2+1)*(RADIUS*2+1));
		}
	}
	
    tstop = second();
	std::cout << tstop-tstart << ", 0, 0, " << NROWS*NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<double> seqInput(NROWS*NCOLS);
    std::vector<double> seqOutput(seqInput.size());
    for(size_t i=0; i<NROWS*NCOLS; ++i) {
	    seqInput[i] = i;
	}
    sequentialGaussianBlur(seqOutput, seqInput);
	return 0;
}
