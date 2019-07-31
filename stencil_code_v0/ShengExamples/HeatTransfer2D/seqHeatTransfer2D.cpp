/**
 * By 2D conduction formula, radius must be 1. 
 * g++ seqHeatTransfer2D.cpp -std=c++11 -O2 -lpthread -DRADIUS=1 -DNROWS=4 -DNCOLS=5 -DPADDING=0 -DNITERS=4 -o seqHeatTransfer2D
 * ./seqHeatTransfer2D
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

double second()
{
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

double average(double neighbourhood[])
{
	double sum = 0;
	sum = neighbourhood[1] + neighbourhood[3] + neighbourhood[5] + neighbourhood[7];
	return sum/5.; // hard coded 4
}

void sequentialMedian(std::vector<double> &output, std::vector<double> &input)
{
	double tstart, tstop;
    tstart = second();

	// computation
	double *neighbourhood = (double *) malloc((2*RADIUS+1)*(2*RADIUS+1)*sizeof(double));
	int nItems = NROWS*NCOLS;
	for (int iter=0; iter<NITERS; iter++)
	{
		// NEW
		if (iter>0)
		{
			auto temp = input;
		    input = output;
			output = temp;
		}
		
		for (int elIdx=0; elIdx<nItems; ++elIdx)
		{
			int elCol = elIdx % NCOLS;
			int elRow = elIdx / NCOLS;
			int neighbourCol, neighbourRow;

			// iterate over filter window
			for (int row=0; row<2*RADIUS+1; ++row)
			{
				for (int col=0; col<2*RADIUS+1; ++col)
				{
					switch (PADDING)
					{
					case WRAP_AROUND:
						{
							neighbourCol = (elCol+col+NCOLS-RADIUS)%NCOLS;
							neighbourRow = (elRow+row+NROWS-RADIUS)%NROWS;
							neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+neighbourRow*NCOLS];
						}
						break;
					case FIXED_VALUE:
						{
							neighbourCol = elCol+col-RADIUS;
							neighbourRow = elRow+row-RADIUS;
							if (neighbourCol<0 || neighbourCol>=NCOLS || neighbourRow<0 || neighbourRow>=NROWS)
								neighbourhood[col+row*(2*RADIUS+1)] = 0;
							else
								neighbourhood[col+row*(2*RADIUS+1)] = input[neighbourCol+neighbourRow*NCOLS];
						}
						break;
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
						}
						break;
					default:
						throw std::invalid_argument("Invalid padding option.");
						break;
					}
				}
			}
			output[elIdx] = average(neighbourhood);
		}
	}
	
    tstop = second();
    // std::cout << "sequentialMedian, " << tstop-tstart << std::endl;
}

int main(int argc, char** argv)
{
    std::vector<double> seqInput(NROWS*NCOLS);
    std::vector<double> seqOutput(seqInput.size());
    for(size_t i = 0; i < NROWS*NCOLS; ++i) {
		seqInput[i] = 0.;
	}
	sequentialMedian(seqOutput, seqInput);
	return 0;
}
