// Would be simpler if we had "myindex" and also MapThenReduce Skepu2 style

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>

#include <cassert>
#include <sys/time.h>
#include "../../Map.hpp"
#include "../../Reduce.hpp"

double second() {
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}


float nth_term(size_t index, float x)
{
	int k = index + 1;
	float temp_x = pow(x, k);
	int sign = (k % 2 == 0) ? -1 : 1;
	return sign * temp_x / k;
}

float plus(float a, float b)
{
	return a + b;
}


int main(int argc, char *argv[])
{
    const size_t threads = NTHREADS;
    const size_t nterms  = NTERMS;
    const float x        = XVALUE - 1;

    double tstart, tstop;

    tstart = second();

    // create and initialize indices vector
    std::vector<size_t> indices(nterms);
    for (size_t i = 0; i<nterms; i++) {
      indices[i] = i;
    }

    // create terms vector
    std::vector<float> terms(nterms);

    auto taylor_map = Map(nth_term, threads);
    taylor_map(terms, indices, x);


    float result;
    auto taylor_reduce = Reduce (plus, threads);
    taylor_reduce(result, terms);

    tstop = second();

    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NTERMS << ", "  << XVALUE <<  std::endl;

    // Remove this line during timing tests
    //std::cout << "the result is  " << result << std::endl;

    return 0;
}

