/**
 * g++ seqSum2D.cpp -std=c++11 -O2 -lpthread -DRADIUS=1 -DNROWS=4 -DNCOLS=5 -DPADDING=0 -DNITERS=4 -o seqSum2D
 * ./seqSum2D
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

void sequentialSum(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	// sum
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
			int sum=0;
		
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
							sum += input[neighbourCol+neighbourRow*NCOLS];
						}
						break;
					case FIXED_VALUE:
						{
							neighbourCol = elCol+col-RADIUS;
							neighbourRow = elRow+row-RADIUS;
							if (neighbourCol>=0 && neighbourCol<NCOLS && neighbourRow>=0 && neighbourRow<NROWS)
								sum += input[neighbourCol+neighbourRow*NCOLS];
						}
						break;
					case REPLICATE_LAST_ELEMENT:
						{
							neighbourCol = elCol+col-RADIUS;
							neighbourRow = elRow+row-RADIUS;
							if ((neighbourCol<0 || neighbourCol>=NCOLS) && (neighbourRow<0 || neighbourRow>=NROWS))
								sum += 0;
							else if (neighbourCol<0)
								sum += input[neighbourRow*NCOLS];
							else if (neighbourCol>=NCOLS)
								sum += input[(neighbourRow+1)*NCOLS-1];
							else if (neighbourRow<0)
								sum += input[neighbourCol];
							else if (neighbourRow>=NROWS)
								sum += input[neighbourCol+(NROWS-1)*NCOLS];
							else
								sum += input[neighbourCol+neighbourRow*NCOLS];
						}
						break;
					default:
						throw std::invalid_argument("Invalid padding option.");
						break;
					}
				}
			}
			output[elIdx] = sum;
		}
	}
	
    tstop = second();
	std::cout << tstop-tstart << ", 0, 0, " << NROWS*NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NROWS*NCOLS);
    std::vector<int> seqOutput(seqInput.size());
    for(size_t i = 0; i < NROWS*NCOLS; ++i)
    {
		seqInput[i] = i;
	}	
	sequentialSum(seqOutput, seqInput);
	return 0;
}
