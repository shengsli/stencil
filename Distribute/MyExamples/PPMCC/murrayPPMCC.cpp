// This one shows why Skepu2 multiple pointwise inputs are really useful
// and also why Skepu2 MapReduce is handy


/*!
 *  PPMCC stands for "Pearson product-moment correlation coefficient"
 *  In statistics, the Pearson coefficient of correlation is a measure by the
 *  linear dependence between two variables X and Y. The mathematical
 *  expression of the Pearson coefficient of correlation is as follows:
 *   r = ( (n*sum(X.Y)-sum(X)*sum(Y))/((n*sum(X^2)-(sum(X))^2)*(n*sum(Y^2)-(sum(Y))^2)) )
 */

#include <iostream>
#include <cmath>

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


template<typename T>
T square(T a)
{
	return a * a;
}

template<typename T>
T mult(int id, std::vector<T>a, std::vector<T> b)
{
	return a[id] * b[id];
}


template<typename T>
T plus(T a, T b)
{
	return a + b;
}



int main(int argc, char *argv[])
{
    const size_t threads = NTHREADS;
    const size_t nitems  = NITEMS;

    double tstart, tstop;

    tstart = second();

    // create and initialize indices vector
    std::vector<int> x(nitems);
    std::vector<int> xsquare(nitems);
    std::vector<int> y(nitems);
    std::vector<int> ysquare(nitems);
    std::vector<int> dpterms(nitems);

    // create and initialize indices vector
    std::vector<int> indices(nitems);
    for (size_t i = 0; i<nitems; i++) {
      indices[i] = i;
    }

    //generate random sequences as in the Skepu2 original
    for (size_t i = 0; i<nitems; i++) {
      x[i] = 1+rand()%3;
    }
    for (size_t i = 0; i<nitems; i++) {
      y[i] = 2+rand()%4;
    }

    auto sum = Reduce(plus<int>, threads);
    int sumx, sumy, sumsquarex, sumsquarey, dpxy;
    sum(sumx, x);
    sum(sumy, y);
        
    auto sumSquareMap     = Map(square<int>, threads);
    sumSquareMap (xsquare, x);
    sumSquareMap (ysquare, y);
    sum(sumsquarex, xsquare);
    sum(sumsquarey, ysquare);

    auto dotProductMap    = Map(mult<int>, threads);
    dotProductMap(dpterms, indices, x, y);

    sum(dpxy, dpterms);

    float ppmcc = (nitems * dpxy - sumx * sumy)
		/ sqrt((nitems * sumsquarex - pow(sumx, 2)) * (nitems * sumsquarey - pow(sumy, 2)));
    
    tstop = second();

    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NITEMS <<  std::endl;

    
    // Remove this line when performance testing
    //std::cout << "Result: " << ppmcc << std::endl;
    return 0;
}

