/**
 * g++ sum1D.cpp -std=c++11 -O2 -lpthread -DRADIUS=2 -DNTHREADS=4 -DNITEMS=1024 -DNITERS=10 -DNDATABLOCKS=100 -DPADDING=0 -o sum1D
 * ./sum1D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil1D.hpp"

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

void parallelSum(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil = Stencil1D(stencilkernel, RADIUS, PADDING, NITERS, NTHREADS);
    stencil(output, input);
	
    tstop = second();
    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NITEMS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> parInput(NITEMS);
    std::vector<int> parOutput(parInput.size());
    for(size_t i = 0; i < NITEMS; ++i)
	{
	    parInput[i] = i;
	}
	parallelSum(parOutput, parInput);
	return 0;
}
