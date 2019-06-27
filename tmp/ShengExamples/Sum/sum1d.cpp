/**
 * Usage: g++ -o sum1d sum1d.cpp -std=c++11 
 */
#include "../../Stencil.hpp"

#define RADIUS 1
#define NTHREADS 2

int sum(int *in, int radius)
{
	int width = radius*2 + 1;
	int *neighbourhood;
	neighbourhood = malloc(width*sizeof(int));

	int sum = 0;
	for (int i=0; i<width; i++)
	{
		sum += neighbourhood[i];
	}
	return sum;
}

int main (int argc, char* argv[])
{
	auto stencil = Stencil(sum, RADIUS, NTHREADS);
	stencil( ,in);
}
