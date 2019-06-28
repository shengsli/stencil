#include <cstdlib>
#include <cassert>
#include <sys/time.h>

#include "../../Stencil.hpp"

/**
 * Finds the median values of input vector. 
 * @param neighbourhood is a 1D vector of type double.
 * @return the median of vector neighbourhood. 
 */
double findMedian(vector<double> neighbourhood)
{
	
}

bool sum(vector<double> neighbourhood)

vector<double> randVector()
{
	vector<double> vec;
	int i;
	srand(0);
	for (i=0; i<NITEMS; i++)
	{
		int num;
		num = rand()%10;
		vec[i] = (double) num;
	}
	return vec;
}

int main(int argc, char** argv) {
	std::vector<double> in = randVector();
	std::vector<double> out(in.size());

    auto stencil = Stencil(findMedian, NTHREADS); // a stencil object whose elemental is findMedian
    stencil(out, in);
   
	return 0;
}
