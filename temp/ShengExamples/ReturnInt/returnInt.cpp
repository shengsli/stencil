/**
 * g++ returnInt.cpp -std=c++11 -O2 -lpthread -DWIDTH=2 -DNTHREADS=4 -DHXRES=1024 -DHYRES=1024 -DITERMAX=1000 -DNDATABLOCKS=100 -DOUTPUT -o returnInt
 * ./returnInt
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

double second() {
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

int stencilkernel (int neighbourhood[], int width) {
	return neighbourhood[width];
}

int main(int argc, char** argv) {
    double tstart, tstop;
    tstart = second();
	
    std::vector<int> in(HXRES*HYRES);
    for(size_t i = 0; i < HXRES*HYRES; ++i)
        in[i] = i;
	
    std::vector<int> out (in.size());
    auto stencil = Stencil1D(stencilkernel, WIDTH, NTHREADS);
    stencil(out, in);
	
    tstop = second();
    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << HXRES*HYRES <<  std::endl;

	if (compareResult(in, out))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;
	return 0;
}
