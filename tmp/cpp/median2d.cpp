/**
 * median2d.c
 * @brief	2D median filtering. It assumes NITEMS>=NTHREADS. NITEMS<NTHREADS is not considered.
 * Usage    g++ -o median2d median2d.cpp -std=c++11 
 */
#include <iostream>
#include <vector>

#define RADIUS 1

template<typename T>
void print2DVector(const std::vector<std::vector<T>> &vec2d)
{
	int i, j;
	for (i=0; i<vec2d.size(); i++)
	{
		for (j=0; j<vec2d[0].size()-1; j++)
		{
			std::cout << vec2d[i][j] << ", ";
		}
		std::cout << vec2d[i][j] << std::endl;
	}
}

std::vector<std::vector<double>> genVector()
{
	std::vector<std::vector<double>> vec2d(GRIDSIZE, std::vector<double>(GRIDSIZE, 0));
	std::srand(0);
	for (int i=0; i<GRIDSIZE; i++)
	{
		for (int j=0; j<GRIDSIZE; j++)
			vec2d[i][j] = (double)(std::rand()%10);
	}
	return vec2d;
}

double seqMedian(int *in, int radius)
{
	int width = radius*2 + 1;
	int *neighbour;
	neighbour = malloc(width*sizeof(double))
	auto it = vec1d.begin();
	for (int i=0; i<GRIDSIZE; i++)
	{
		std::copy(vec1d.begin(), vec1d.end(), it);
		it += in[0].size();
	}
	out = vec1d;
}

int main (int argc, char* argv[])
{
	std::vector<std::vector<double>> in = genVector();
	std::vector<std::vector<double>> out(in.size());
	print2DVector(in);
	print2DVector(out);
}
