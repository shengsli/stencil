/**
 * By 2D conduction formula, radius must be 1. 
 * g++ heatTransfer2D.cpp -std=c++11 -O2 -lpthread -DRADIUS=1 -DNTHREADS=4 -DNROWS=4 -DNCOLS=5 -DNDATABLOCKS=100 -DPADDING=0 -DNITERS=4 -DOUTPUT -o heatTransfer2D
 * ./heatTransfer2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

void prdouble_arr(double *arr, double size)
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

double average(double neighbourhood[])
{
	double sum = 0;
	sum = neighbourhood[1] + neighbourhood[3] + neighbourhood[5] + neighbourhood[7];
	return sum/4.; // hard coded 4
}

double stencilkernel (double neighbourhood[], int radius)
{
	return average(neighbourhood);
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

void parallelMedian(std::vector<double> &output, std::vector<double> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil2d = Stencil2D(stencilkernel, RADIUS, NROWS, NCOLS, PADDING, NITERS, NTHREADS);
    stencil2d(output, input);
	
    tstop = second();
    std::cout << "parallelMedian, " << tstop-tstart << ", " << NTHREADS  << ", " << NDATABLOCKS << ", " << NROWS*NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<double> seqInput(NROWS*NCOLS);
    std::vector<double> parInput(NROWS*NCOLS);
    for(size_t i = 0; i < NROWS*NCOLS; ++i)
    {
		seqInput[i] = parInput[i] = i/(double)(NROWS*NCOLS-1)*255;
	}
    std::vector<double> seqOutput(seqInput.size());
    std::vector<double> parOutput(parInput.size());

    #ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("heatTransfer2Dtest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile,"%.2f, ", seqInput[i]);
    }
	fprintf(outfile,"\n");
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
	
	if (compareResult(seqOutput, parOutput))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;

	outfile = fopen("heatTransfer2Dtest.ppm","w");
    fprintf(outfile,"P6\n# Stencil testing\n");
    fprintf(outfile,"%d %d\n255\n",NCOLS,NROWS);
    for (size_t i = 0; i<NROWS*NCOLS; i++)
	{
		fputc((char)parOutput[i],outfile);
		fputc((char)parOutput[i],outfile);
		fputc((char)parOutput[i],outfile);
    }
	fclose(outfile);
    #endif

	return 0;
}
