#ifndef STENCIL1D_HPP
#define STENCIL1D_HPP
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

#define WRAP_AROUND 0
#define FIXED_VALUE 1
#define REPLICATE_LAST_ELEMENT 2

class Stencil1DSkeleton
{
    private:
	Stencil1DSkeleton() {}
	template<typename EL>
	class Elemental
	{
	    public:
		Elemental(EL el) : elemental(el) {}
		EL elemental;
	};

    public:
	template<typename EL>
	class Stencil1DImplementation
	{
	    private:
		unsigned char BLOCK_FLAG_INITIAL_VALUE;
		size_t nthreads;
		size_t nDataBlocks;
		size_t radius;
		unsigned char paddingOption;

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
			ThreadArgument(std::vector<OUT> &output, std::vector<IN> &input, size_t threadInputIndex, size_t chunkSize) : threadInputIndex(threadInputIndex), chunkSize(chunkSize), input(&input), output(&output) {}
			~ThreadArgument()
			{
				delete[] dataBlockIndices;
				delete[] dataBlockFlags;
				delete dataBlockMutex;
			}
		};

		template<typename IN, typename OUT, typename ...ARGs>
		void threadStencil1D(ThreadArgument<IN,OUT> *threadArguments, size_t radius, unsigned char paddingOption, size_t threadID, ARGs... args)
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

						IN *neighbourhood = (IN *) malloc((radius*2+1)*sizeof(IN));

						for(size_t elementIndex = dataBlockIndices[dataBlock];
							elementIndex < dataBlockIndices[ dataBlock+1 ];
							++elementIndex)
						{
							for (size_t i=0; i<2*radius+1; i++)
							{
								switch (paddingOption)
								{
								case WRAP_AROUND:
									neighbourhood[i]=input->at((elementIndex-radius+i+inputSize)%inputSize);
									break;
								case FIXED_VALUE:
									{
										int idx = elementIndex-radius+i;
										if (idx >= 0 && idx < inputSize)
											neighbourhood[i]=input->at(idx);
										else
											neighbourhood[i]=0;
									}
									break;
								case REPLICATE_LAST_ELEMENT:
									{
										int idx = elementIndex-radius+i;
										if (idx >= 0 && idx < inputSize)
											neighbourhood[i]=input->at(idx);
										else if (idx<0)
											neighbourhood[i]=input->at(0);
										else if (idx>=inputSize)
											neighbourhood[i]=input->at(inputSize-1);
									}
									break;
								default:
									throw std::invalid_argument("Invalid padding option.");
									break;
								}
							}
							output->at(elementIndex)=elemental.elemental(neighbourhood,radius,args...);
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
		Stencil1DImplementation(Elemental<EL> elemental, size_t radius, unsigned char paddingOption, size_t threads) : elemental(elemental), radius(radius), paddingOption(paddingOption), nthreads(threads)
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
				//output[0] = elemental.elemental(input[0], radius, args...);
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
				THREADS[t]=new std::thread(&Stencil1DImplementation<EL>::threadStencil1D<IN,OUT,ARGs...>, this, threadArguments, radius, paddingOption, t, args...);
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
		friend Stencil1DImplementation<EL2> __Stencil1DWithAccess(EL2 el, const size_t &radius, const unsigned char &paddingOption, const size_t &threads);
	};
	template<typename EL2>
	friend Stencil1DImplementation<EL2> __Stencil1DWithAccess(EL2 el, const size_t &radius, const unsigned char &paddingOption, const size_t &threads);
};

/*
 * We cannot define a friend function with default argument
 * that needs access to inner class on latest g++ compiler versions.
 * We need a wrapper!
 */
template<typename EL>
Stencil1DSkeleton::Stencil1DImplementation<EL> __Stencil1DWithAccess(EL el, const size_t &radius, const unsigned char &paddingOption, const size_t &threads)
{
    Stencil1DSkeleton::Elemental<EL> elemental(el);
    Stencil1DSkeleton::Stencil1DImplementation<EL> stencil1D(elemental, radius, paddingOption, threads);
    return stencil1D;
}

template<typename EL>
Stencil1DSkeleton::Stencil1DImplementation<EL> Stencil1D(EL el, const size_t &radius, const unsigned char &paddingOption, const size_t &threads = 0)
{
    return __Stencil1DWithAccess(el, radius, paddingOption, threads);
}

#endif /* STENCIL1D_HPP */
