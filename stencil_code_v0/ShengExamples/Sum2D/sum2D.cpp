/**
 * g++ sum2D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=1 -DNROWS=4 -DNCOLS=5 -DPADDING=0 -DNITERS=4 -o sum2D
 * ./sum2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

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

void parallelSum(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil2d = Stencil2D(stencilkernel, RADIUS, NROWS, NCOLS, PADDING, NITERS, NTHREADS);
    stencil2d(output, input);
	
    tstop = second();
    std::cout << tstop-tstart << ", " << NTHREADS  << ", " << NDATABLOCKS << ", "  << NROWS*NCOLS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> parInput(NROWS*NCOLS);
    std::vector<int> parOutput(parInput.size());
    for(size_t i = 0; i < NROWS*NCOLS; ++i)
    {
	    parInput[i] = i;
	}	
	parallelSum(parOutput, parInput);	
	return 0;
}
