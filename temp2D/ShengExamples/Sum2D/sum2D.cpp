/**
 * g++ sum2D.cpp -std=c++11 -O2 -lpthread -DWIDTH=2 -DNTHREADS=4 -DNITEMS=1024 -DITERMAX=1000 -DNDATABLOCKS=100 -DOUTPUT -o sum2D 
 * ./sum2D
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil.hpp"

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

double second()
{
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

int stencilkernel (int neighbourhood[], int width)
{
	int sum = 0;
	for (int i=0; i<width*2+1; ++i)
		sum += neighbourhood[i];
	return sum;
}

void sequentialSum(std::vector<int> &output, std::vector<int> &input)
{
	double tstart, tstop;
    tstart = second();

	// TODO
	int inputSize = input.size();
	for (int targetIdx=0; targetIdx<NITEMS; ++targetIdx)
	{
		int sum=0;
		for (int i=0; i<WIDTH*2+1; ++i)
		{
			sum += input[(targetIdx+i-WIDTH+inputSize)%inputSize];
		}
		output[targetIdx] = sum;
	}
	
    tstop = second();
    std::cout << "sequentialSum, " << tstop-tstart << std::endl;
}

void parallelSum(std::vector<int> &output, std::vector<int> &input)
{
    double tstart, tstop;
    tstart = second();
	
    auto stencil = Stencil(stencilkernel, WIDTH, NTHREADS);
    stencil(output, input);
	
    tstop = second();
    std::cout << "parallelSum, " << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << NITEMS <<  std::endl;
}

int main(int argc, char** argv)
{
    std::vector<int> input(NITEMS);
    for(size_t i = 0; i < NITEMS; ++i)
    {
		input[i] = i;
	}
    std::vector<int> seqOutput(input.size());
    std::vector<int> parOutput(input.size());

	sequentialSum(seqOutput, input);
	parallelSum(parOutput, input);
	
	if (compareResult(seqOutput, parOutput))
		std::cout << "out is the same as in" << std:: endl;
	else
		std::cout << "ERROR: out != in" << std::endl;
	return 0;
}
