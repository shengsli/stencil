/**
 * g++ returnInt.cpp -std=c++11 -O2 -lpthread -DWIDTH=2 -DNTHREADS=4 -DHXRES=1024 -DHYRES=1024 -DITERMAX=1000 -DNDATABLOCKS=100 -DOUTPUT -o returnInt
 * ./returnInt
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil.hpp"

double second() {
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

/**
 * Take an integer and scale it to 0..255.   
 */
int stencilkernel (int neighbourhood[], int width) {
	return *neighbourhood;
}

int main(int argc, char** argv) {
    double tstart, tstop;
    tstart = second();
	
    std::vector<int> in(HXRES*HYRES);
    for(size_t i = 0; i < HXRES*HYRES; ++i)
        in[i] = i;
	
    std::vector<int> out (in.size());
    auto stencil = Stencil(stencilkernel, WIDTH, NTHREADS);
    stencil(out, in);
	
    tstop = second();
    std::cout << tstop-tstart << ", " << WIDTH << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << HXRES*HYRES <<  std::endl;
	return 0;
}
