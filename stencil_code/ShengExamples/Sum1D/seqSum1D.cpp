/**
 * g++ seqSum1D.cpp -std=c++11 -O2 -lpthread -DRADIUS=2 -DNITEMS=1024 -DNITERS=10 -DPADDING=0 -o seqSum1D
 * ./seqSum1D
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

	int inputSize = input.size();
	for (int iter=0; iter<NITERS; iter++)
	{
		if (iter>0)
		{
			auto temp = input;
		    input = output;
			output = temp;
		}
		
		for (int elementIndex=0; elementIndex<NITEMS; ++elementIndex) {
			int sum=0;
			for (int i=0; i<RADIUS*2+1; ++i) {
				switch (PADDING) {
				    case WRAP_AROUND:
						sum += input[(elementIndex+i-RADIUS+inputSize)%inputSize];
						break;
				    case FIXED_VALUE:
					{
						int idx = elementIndex+i-RADIUS;
						if (idx >= 0 && idx < inputSize)
							sum += input[idx];
						break;
					}
				    case REPLICATE_LAST_ELEMENT:
					{
						int idx = elementIndex-RADIUS+i;
						if (idx >= 0 && idx < inputSize)
							sum+=input[idx];
						else if (idx<0)
							sum+=input[0];
						else if (idx>=inputSize)
							sum+=input[inputSize-1];
						break;
					}
				    default:
						throw std::invalid_argument("Invalid padding option.");
						break;
				}
			}
			output[elementIndex] = sum;
		}		
	}
	
    tstop = second();
	std::cout << tstop-tstart << ", seq, 0, " << NITEMS <<  std::endl;
}

int main (int argc, char** argv)
{
    std::vector<int> seqInput(NITEMS);
    std::vector<int> seqOutput(seqInput.size());
    for(size_t i = 0; i < NITEMS; ++i) {
		seqInput[i] = i;
	}
	sequentialSum(seqOutput, seqInput);
	return 0;
}
