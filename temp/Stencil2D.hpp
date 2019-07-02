#ifndef STENCIL2D_HPP
#define STENCIL2D_HPP
#include <cstdlib>
#include <iostream>
#include <vector>
#include <type_traits>
#include <functional>
#include <stdarg.h>
#include <typeinfo>
#include <utility>
#include <thread>
#include <mutex>

class Stencil2DSkeleton
{
    private:
	Stencil2DSkeleton() {}
	template<typename EL>
	class Elemental
	{
	    public:
		Elemental(EL el) : elemental(el) {}
		EL elemental;
	};

    public:
	template<typename EL>
	class Stencil2DImplementation
	{
	    private:
		unsigned char BLOCK_FLAG_INITIAL_VALUE;
		size_t nthreads;
		size_t nDataBlocks;
		size_t width;
		size_t nrows;
		size_t ncols;

		template<typename IN, typename OUT>
		class ThreadArgument
		{
		    public:
			size_t threadInputIndex;
			size_t chunkSize;
			size_t nDataBlocks;
			std::mutex *dataBlockMutex;
			unsigned char *dataBlockFlags;
			size_t *dataBlockIndices;
			std::vector<IN> *input;
			std::vector<OUT> *output;

			ThreadArgument() {}
			ThreadArgument(std::vector<OUT> &output, std::vector<IN> &input,
						   size_t threadInputIndex, size_t chunkSize)
				: threadInputIndex(threadInputIndex), chunkSize(chunkSize),
				  input(&input), output(&output) {}

			~ThreadArgument()
			{
				delete[] dataBlockIndices;
				delete[] dataBlockFlags;
				delete dataBlockMutex;
			}
		};

		template<typename IN, typename OUT, typename ...ARGs>
		void threadStencil2D(ThreadArgument<IN,OUT> *threadArguments, size_t width,
							 size_t nrows, size_t ncols, size_t threadID, ARGs... args)
		{
			auto input = threadArguments[threadID].input;
			auto output = threadArguments[threadID].output;
			size_t assistedThreadID = threadID;
			do {
				std::mutex* dataBlockMutex = threadArguments[assistedThreadID].dataBlockMutex;
				unsigned char* dataBlockFlags = threadArguments[assistedThreadID].dataBlockFlags;
				std::size_t* dataBlockIndices = threadArguments[assistedThreadID].dataBlockIndices;
				size_t nDataBlocks = threadArguments[assistedThreadID].nDataBlocks;
				size_t dataBlock = 0;
				size_t inputSize = input->size();

				// why shouldn't we 'steal' even the first dataBlock? :P
				while( dataBlock < nDataBlocks )
				{
					// if the data block has been, or being processed by another thread...
					if(dataBlockFlags[ dataBlock ] == 0 || 
					   dataBlockIndices[ dataBlock ] == dataBlockIndices[ dataBlock+1 ])
					{
						++dataBlock;
						continue;
						//as we iterate in reverse -> continue.
						// In case there are no more dataBlocks,
						// as chunkSize might be less than NDATABLOCKS
					}

					dataBlockMutex->lock();
					if(dataBlockFlags[dataBlock] == 1)
					{ 
						// were the flag is zero, the following flags are zero too.
						dataBlockFlags[ dataBlock ] = 0;
						dataBlockMutex->unlock();

						IN *neighbourhood = (IN *) malloc((width*2+1)*(width*2+1)*sizeof(IN));

						for(size_t elementIndex = dataBlockIndices[dataBlock];
							elementIndex < dataBlockIndices[ dataBlock+1 ];
							++elementIndex)
						{
							int elCol = elementIndex % ncols;
							int elRow = elementIndex / ncols;
							int neighbourCol, neighbourRow;

							// iterate over filter window
							for (int row=0; row<2*width+1; ++row)
							{
								for (int col=0; col<2*width+1; ++col)
								{
									neighbourCol = (elCol+col+ncols-width)%ncols;
									neighbourRow = (elRow+row+nrows-width)%nrows;
									neighbourhood[col+row*(2*width+1)] = input->at(neighbourCol+neighbourRow*ncols);
								}
							}							
							output->at(elementIndex)=elemental.elemental(neighbourhood,width,args...);
						}
						free(neighbourhood);
					}
					else
					{
						// Just in case after the first if,
						// the flag changes its value to 0 from another thread
						dataBlockMutex->unlock();
					}
					++dataBlock;
				}
				assistedThreadID = (assistedThreadID + 1) % nthreads;
			} while(assistedThreadID != threadID);
		}

		Elemental<EL> elemental;
		Stencil2DImplementation(Elemental<EL> elemental, size_t width,
								size_t nrows, size_t ncols, size_t threads)
			: elemental(elemental), width(width), nrows(nrows), ncols(ncols), nthreads(threads)
		{
			this->nDataBlocks = NDATABLOCKS; 
			// this->nDataBlocks = 1; // MIC! was 10
			this->BLOCK_FLAG_INITIAL_VALUE = 1;
		}

    	public:
		template<typename IN, typename OUT, typename ...ARGs>
		void operator()(std::vector<OUT> &output, std::vector<IN> &input, ARGs... args)
		{
			nthreads = nthreads ? nthreads : std::thread::hardware_concurrency();
			nthreads = nthreads >= input.size() ? input.size() / 2 : nthreads;

			/*
			 * TODO: Handle input.size == 0 or 1
			 * Hardcoded for now...
			 */
			if( input.size() == 0 ) 
			{
				return;
			}
			if( input.size() == 1 )
			{
				//output[0] = elemental.elemental(input[0], width, args...);
				return;
			}
			std::thread *THREADS[nthreads];
			ThreadArgument<IN,OUT> *threadArguments = new ThreadArgument<IN,OUT>[nthreads];
			std::vector<OUT> tempOutput( input.size() );
			size_t chunkIndex = 0;

			for(size_t t=0; t< nthreads; ++t )
			{
 				if( t < (input.size() % nthreads) )
					threadArguments[t].chunkSize = 1 + input.size() / nthreads;
				else
					threadArguments[t].chunkSize = input.size() / nthreads;

				threadArguments[t].input = &input;
				threadArguments[t].output = &tempOutput;
				threadArguments[t].threadInputIndex = chunkIndex;
				chunkIndex += threadArguments[t].chunkSize;

				/*
				 * Data Blocks
				 */
				nDataBlocks = nDataBlocks > threadArguments[t].chunkSize
					? threadArguments[t].chunkSize / 2 : nDataBlocks;
				threadArguments[t].nDataBlocks = nDataBlocks;
				threadArguments[t].dataBlockFlags = new unsigned char[nDataBlocks]();
				std::fill_n(threadArguments[t].dataBlockFlags, nDataBlocks, BLOCK_FLAG_INITIAL_VALUE);
				threadArguments[t].dataBlockMutex = new std::mutex;
				threadArguments[t].dataBlockIndices = new size_t[nDataBlocks+1]();
				size_t blockSize, blockStart = threadArguments[t].threadInputIndex, blockEnd;
				for(size_t block = 0; block < nDataBlocks; ++block)
				{
					if( block < (threadArguments[t].chunkSize % nDataBlocks) )
						blockSize = 1 + threadArguments[t].chunkSize / nDataBlocks;
					else
						blockSize = threadArguments[t].chunkSize / nDataBlocks;
					blockEnd = blockStart + blockSize;
					threadArguments[t].dataBlockIndices[ block ] = blockStart;
					blockStart = blockEnd;
				}
				threadArguments[t].dataBlockIndices[ nDataBlocks ] = blockEnd;
			}

			for(size_t t=0; t< nthreads; ++t )
			{
				THREADS[t]=new std::thread(&Stencil2DImplementation<EL>::threadStencil2D<IN,OUT,ARGs...>, this, threadArguments, width, nrows, ncols, t, args...);
			}

			for(size_t t=0; t< nthreads; ++t)
			{
				THREADS[t]->join();
				delete THREADS[t];
			}
			output = tempOutput;
			delete[] threadArguments;
		}
		template<typename EL2>
		friend Stencil2DImplementation<EL2> __Stencil2DWithAccess(EL2 el, const size_t &width, const size_t nrows, const size_t ncols, const size_t &threads);
	};
	template<typename EL2>
	friend Stencil2DImplementation<EL2> __Stencil2DWithAccess(EL2 el, const size_t &width, const size_t nrows, const size_t ncols, const size_t &threads);
};

/*
 * We cannot define a friend function with default argument
 * that needs access to inner class on latest g++ compiler versions.
 * We need a wrapper!
 */
template<typename EL>
Stencil2DSkeleton::Stencil2DImplementation<EL> __Stencil2DWithAccess(EL el, const size_t &width, const size_t nrows, const size_t ncols, const size_t &threads)
{
    Stencil2DSkeleton::Elemental<EL> elemental(el);
    Stencil2DSkeleton::Stencil2DImplementation<EL> stencil2D(elemental, width, nrows, ncols, threads);
    return stencil2D;
}

template<typename EL>
Stencil2DSkeleton::Stencil2DImplementation<EL> Stencil2D(EL el, const size_t &width, const size_t nrows, const size_t ncols, const size_t &threads = 0)
{
    return __Stencil2DWithAccess(el, width, nrows, ncols, threads);
}

#endif /* STENCIL2D_HPP */
