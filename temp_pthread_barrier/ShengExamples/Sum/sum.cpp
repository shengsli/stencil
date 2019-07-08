/**
 * g++ sum.cpp -std=c++11 -O2 -lpthread -DRADIUS=2 -DNTHREADS=4 -DNITEMS=1024 -DNITERS=10 -DNDATABLOCKS=100 -DPADDING=0 -DOUTPUT -o sum
 * ./sum
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil1D.hpp"

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
	int sum = 0;
	for (int i=0; i<radius*2+1; ++i)
		sum += neighbourhood[i];
	return sum;
}

void sequentialSum(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

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
			int sum=0;
			for (int i=0; i<RADIUS*2+1; ++i)
			{
				switch (PADDING)
				{
				case WRAP_AROUND:
					sum += input[(elementIndex+i-RADIUS+inputSize)%inputSize];
					break;
				case FIXED_VALUE:
					{
						int idx = elementIndex+i-RADIUS;
						if (idx >= 0 && idx < inputSize)
							sum += input[idx];
					}
					break;
				case REPLICATE_LAST_ELEMENT:
					{
						int idx = elementIndex-RADIUS+i;
						if (idx >= 0 && idx < inputSize)
							sum+=input[idx];
						else if (idx<0)
							sum+=input[0];
						else if (idx>=inputSize)
							sum+=input[inputSize-1];
					}
					break;
				default:
					throw std::invalid_argument("Invalid padding option.");
					break;
				}
			}
			output[elementIndex] = sum;
		}		
	}	
    tstop = second();
    std::cout << "sequentialSum, " << tstop-tstart << std::endl;
}

void parallelSum(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil = Stencil1D(stencilkernel, RADIUS, PADDING, NITERS, NTHREADS);
    stencil(output, input);
	
    tstop = second();
    std::cout << "parallelSum, " << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NITEMS <<  std::endl;
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
    outfile = fopen("sumtest.txt","w");
    fprintf(outfile,"seqInput: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", seqInput[i]);
    }
	#endif

	sequentialSum(seqOutput, seqInput);
	parallelSum(parOutput, parInput);
	
    #ifdef OUTPUT
    fprintf(outfile,"\nseqOutput: ");
    for (size_t i = 0; i<NITEMS; i++) {
      fprintf(outfile,"%d, ", seqOutput[i]);
    }
    fprintf(outfile,"\nparOutput: ");
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
