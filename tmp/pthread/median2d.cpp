/**
 * median2d.c
 * @brief	2D median filtering. It assumes NITEMS>=NTHREADS. NITEMS<NTHREADS is not considered. 
 */
#include <iostream>
#include <vector>

#define GRIDSIZE 3

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
	std::srand(0);
	std::vector<std::vector<double>> vec2d;
	for (int i=0; i<GRIDSIZE; i++)
	{
		vec2d[i].push_back((double)(std::rand()%10));
	}
	return vec2d;
}

void seqMedian (std::vector<std::vector<double>> &out, std::vector<std::vector<double>> &in)
{
	
}

int main (int argc, char* argv[])
{
	std::vector<std::vector<int>> matrix(GRIDSIZE, std::vector<int>(GRIDSIZE, 0));
	print2DVector(matrix);
	// std::vector<std::vector<double>> in = genVector();
	// print2DVector(in);
	// seqMedian();
}
