#include <cstdlib>
#include <vector>
#include <numeric>
#include <iostream>

#include "../../Stencil.hpp"

template<typename T>
void printVector(std::vector<T> vec)
{
	typename std::vector<T>::iterator it = vec.begin();
	for (; it!=vec.end()-1; ++it)
	{
		std::cout << *it << ", ";
	}
	std::cout << *(vec.end()-1) << std::endl;
}

/**
 * @param neighbourhood is a 1D vector.
 * @return the sum of vector. 
 */
template<typename T>
T sum(std::vector<T> vec)
{
	return std::accumulate(vec.begin(), vec.end(), 0);
}

int foo(int var)
{
	return var;
}

std::vector<int> randVector()
{
	std::vector<int> vec;
	int i;
	srand(0);
	for (i=0; i<NITEMS; i++)
		vec.push_back(rand()%10);
	return vec;
}

int main(int argc, char** argv) {
	std::vector<int> in = randVector();
	std::vector<int> out(in.size());
	printVector(in);

    auto stencil = Stencil(foo, NTHREADS); // a stencil object whose elemental is sum
    stencil(out, in);
   
	return 0;
}
