/**
 * g++ seqSum3D.cpp -std=c++11 -O2 -lpthread -DRADIUS=1 -DNXS=3 -DNYS=4 -DNZS=5 -DPADDING=0 -DNITERS=4 -o seqSum3D
 * ./seqSum3D
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
	int nItems = NXS*NYS*NZS;
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
			int elx = elIdx % NXS;
			int ely = (elIdx / NXS) % NYS;
			int elz = elIdx / NXS / NYS;
			int neighbourx, neighboury, neighbourz;
			int sum=0;
		
			// iterate over filter window
			for (int filterz=0; filterz<2*RADIUS+1; ++filterz)
			{
				for (int filtery=0; filtery<2*RADIUS+1; ++filtery)
				{
					for (int filterx=0; filterx<2*RADIUS+1; ++filterx)
					{
						switch (PADDING)
						{
						case WRAP_AROUND:
							{
								neighbourx = (elx+filterx-RADIUS+NXS)%NXS;
								neighboury = (ely+filtery-RADIUS+NYS)%NYS;
								neighbourz = (elz+filterz-RADIUS+NZS)%NZS;
								sum += input[neighbourx+NXS*(neighboury+NYS*neighbourz)];
							}
							break;
						case FIXED_VALUE:
							{
							}
							break;
						case REPLICATE_LAST_ELEMENT:
							{
							}
							break;
						default:
							throw std::invalid_argument("Invalid padding option.");
							break;
						}
					}
				}
			}
			output[elIdx] = sum;
		}
	}
	
    tstop = second();
    std::cout << tstop-tstart << ", 0, 0, "<< NXS*NYS*NZS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NXS*NYS*NZS);
    std::vector<int> seqOutput(seqInput.size());
    for(size_t i=0; i<NXS*NYS*NZS; ++i)
    {
		seqInput[i] = i;
	}
	sequentialSum(seqOutput, seqInput);
	return 0;
}
