/**
 * By 2D conduction formula, radius must be 1. 
 * g++ heatTransfer2D.cpp -std=c++11 -O2 -lpthread -DRADIUS=1 -DNTHREADS=4 -DNROWS=4 -DNCOLS=5 -DNDATABLOCKS=100 -DPADDING=0 -DNITERS=4 -o heatTransfer2D
 * ./heatTransfer2D
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

void parallelHeatTransfer(std::vector<double> &output, std::vector<double> &input)
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
    std::vector<double> parInput(NROWS*NCOLS);
    std::vector<double> parOutput(parInput.size());
    for(size_t i = 0; i < NROWS*NCOLS; ++i) {
	    parInput[i] = 0.;
	}
	parallelHeatTransfer(parOutput, parInput);
	return 0;
}
