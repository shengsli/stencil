/**
 * g++ sum3D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=1 -DNXS=3 -DNYS=3 -DNZS=3 -DPADDING=0 -DNITERS=1 -DOUTPUT -o sum3D
 * ./sum3D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil3D.hpp"

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
	for (int z=0; z<NZS; ++z)
	{
		for (int y=0; y<NYS; ++y)
		{
			for (int x=0; x<NXS; ++x)
			{
				printf("%d, ", vec[x+NXS*(y+NYS*z)]);
			}
			std::cout << std::endl;
		}
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
	for (int i=0; i<(radius*2+1)*(radius*2+1)*(radius*2+1); ++i)
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
    std::cout << "sequentialSum, " << tstop-tstart << std::endl;
}

void parallelSum(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil3d = Stencil3D(stencilkernel, RADIUS, NXS, NYS, NZS, PADDING, NITERS, NTHREADS);
    stencil3d(output, input);
	
    tstop = second();
    std::cout << "parallelSum, " << tstop-tstart << ", " << RADIUS << ", " << NTHREADS  << ", " << NXS << ", " << NYS << ", " << NZS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> seqInput(NXS*NYS*NZS);
    std::vector<int> parInput(NXS*NYS*NZS);
    for(size_t i=0; i<NXS*NYS*NZS; ++i)
    {
		seqInput[i] = parInput[i] = i;
	}
    std::vector<int> seqOutput(seqInput.size());
    std::vector<int> parOutput(parInput.size());

    #ifdef OUTPUT
	FILE *outfile;
    outfile = fopen("sum3Dtest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i=0; i<NXS*NYS*NZS; i++) {
      if (i%NXS == 0) fprintf(outfile," \t");
      if (i%(NXS*NYS) == 0) fprintf(outfile," \n");
      fprintf(outfile,"%5d, ", seqInput[i]);
    }
	#endif
	
	sequentialSum(seqOutput, seqInput);
	parallelSum(parOutput, parInput);

	#ifdef OUTPUT
    fprintf(outfile,"\nSequential Output: ");
    for (size_t i=0; i<NXS*NYS*NZS; i++) {
      if (i%NXS == 0) fprintf(outfile," \t");
      if (i%(NXS*NYS) == 0) fprintf(outfile," \n");
      fprintf(outfile,"%5d, ", seqOutput[i]);
    }
	fprintf(outfile,"\n");
    fprintf(outfile,"\nParallel Output: ");
    for (size_t i=0; i<NXS*NYS*NZS; i++) {
      if (i%NXS == 0) fprintf(outfile," \t");
      if (i%(NXS*NYS) == 0) fprintf(outfile," \n");
      fprintf(outfile,"%5d, ", parOutput[i]);
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
