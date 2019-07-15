/**
 * gaussian filter is hardcoded as a 5x5 filter. radius must be 2. 
 * g++ gaussianBlur2D.cpp -std=c++11 -O2 -lpthread -DNTHREADS=4 -DNDATABLOCKS=100 -DRADIUS=2 -DNROWS=6 -DNCOLS=6 -DPADDING=0 -DNITERS=4 -o gaussianBlur2D
 * ./gaussianBlur2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

double filter[] = {1./273, 4./273, 7./273, 4./273, 1./273, 
				   4./273,16./273,26./273,16./273, 4./273,
				   7./273,26./273,41./273,26./273, 7./273,
				   4./273,16./273,26./273,16./273, 4./273,
				   1./273, 4./273, 7./273, 4./273, 1./273};

double second()
{
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

double conv(double *lhs, double *rhs, int size)
{
	double sum=0.;
	for (int i=0; i<size; ++i) {
		sum += lhs[i]*rhs[i];
	}
	return sum;
}

double stencilkernel (double neighbourhood[], int radius)
{
	return conv(neighbourhood, filter, (2*radius+1)*(2*radius+1));
}

void parallelGaussianBlur(std::vector<double> &output, std::vector<double> &input)
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
    for(size_t i=0; i<NROWS*NCOLS; ++i) {
	    parInput[i] = i;
	}
	parallelGaussianBlur(parOutput, parInput);
	return 0;
}
