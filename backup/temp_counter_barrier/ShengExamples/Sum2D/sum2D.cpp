/**
 * g++ sum2D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=1 -DNROWS=4 -DNCOLS=5 -DPADDING=0 -DNITERS=4 -DOUTPUT -o sum2D
 * ./sum2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

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
	int sum = 0;
	for (int i=0; i<(radius*2+1)*(radius*2+1); ++i)
	{
		sum += neighbourhood[i];
	}
	return sum;
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
    std::cout << "sequentialSum, " << tstop-tstart << std::endl;
}

void parallelSum(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil2d = Stencil2D(stencilkernel, RADIUS, NROWS, NCOLS, PADDING, NITERS, NTHREADS);
    stencil2d(output, input);
	
    tstop = second();
    std::cout << "parallelSum, " << tstop-tstart << ", " << RADIUS << ", " << NTHREADS  << ", " << NROWS << ", " << NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NROWS*NCOLS);
    std::vector<int> parInput(NROWS*NCOLS);
    for(size_t i = 0; i < NROWS*NCOLS; ++i)
    {
		seqInput[i] = parInput[i] = i;
	}
    std::vector<int> seqOutput(seqInput.size());
    std::vector<int> parOutput(parInput.size());

    #ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("sum2Dtest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%d, ", seqInput[i]);
    }
	#endif
	
	sequentialSum(seqOutput, seqInput);
	parallelSum(parOutput, parInput);

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
    #endif
	
	if (compareResult(seqOutput, parOutput))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;
	return 0;
}
