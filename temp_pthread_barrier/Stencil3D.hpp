#ifndef STENCIL3D_HPP
#define STENCIL3D_HPP
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

class Stencil3DSkeleton
{
    private:
	Stencil3DSkeleton() {}
	template<typename EL>
	class Elemental
	{
	    public:
		Elemental(EL el) : elemental(el) {}
		EL elemental;
	};

    public:
	template<typename EL>
	class Stencil3DImplementation
	{
	    private:
		unsigned char BLOCK_FLAG_INITIAL_VALUE;
		size_t nthreads;
		size_t nDataBlocks;
		size_t radius;
		size_t nxs;
		size_t nys;
		size_t nzs;
		unsigned char paddingOption;
		size_t nIters;
		pthread_barrier_t barrier;

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
		void threadStencil3D(ThreadArgument<IN,OUT> *threadArguments, size_t radius, size_t nxs, size_t nys, size_t nzs, unsigned char paddingOption, size_t nIters, size_t threadID, ARGs... args)
		{
			auto input = threadArguments[threadID].input;
			auto output = threadArguments[threadID].output;
			for (int iter=0; iter<nIters; iter++)
			{
				size_t assistedThreadID = threadID;
				// NEW
				if (iter>0)
				{
					auto temp = input;
					temp = input;
					input  = output;
					output = temp;
				}

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

							IN *neighbourhood = (IN *) malloc((radius*2+1)*(radius*2+1)*(radius*2+1)*sizeof(IN));

							for(size_t elementIndex = dataBlockIndices[dataBlock];
								elementIndex < dataBlockIndices[ dataBlock+1 ];
								++elementIndex)
							{
								int elx = elementIndex % nxs;
								int ely = (elementIndex / nxs) % nys;
								int elz = (elementIndex / nxs / nys) % nzs;
								int neighbourx, neighboury, neighbourz;

								// iterate over filter window
								for (int filterz=0; filterz<2*radius+1; ++filterz)
								{
									for (int filtery=0; filtery<2*radius+1; ++filtery)
									{
										for (int filterx=0; filterx<2*radius+1; ++filterx)
										{
											switch (paddingOption)
											{
											case WRAP_AROUND:
												{
													neighbourx = (elx+filterx-radius+nxs)%nxs;
													neighboury = (ely+filtery-radius+nys)%nys;
													neighbourz = (elx+filterz-radius+nzs)%nzs;
													neighbourhood[filterx+(2*radius+1)*(filtery+(2*radius+1)*filterz)] = input->at(neighbourx+nxs*(neighboury+nys*neighbourz));
												}
												break;
											case FIXED_VALUE:
												{
												}
												break;
											case REPLICATE_LAST_ELEMENT:
												{
												}
												break;
											default:
												throw std::invalid_argument("Invalid padding option.");
												break;
											}
										}
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

				pthread_barrier_wait(&barrier);
				std::fill_n(threadArguments[assistedThreadID].dataBlockFlags, nDataBlocks, BLOCK_FLAG_INITIAL_VALUE);
			}
		}

		Elemental<EL> elemental;
		Stencil3DImplementation(Elemental<EL> elemental, size_t radius, size_t nxs, size_t nys, size_t nzs, unsigned char paddingOption, size_t nIters, size_t threads) : elemental(elemental), radius(radius), nxs(nxs), nys(nys), nzs(nzs), paddingOption(paddingOption), nIters(nIters), nthreads(threads)
		{
			this->nDataBlocks = NDATABLOCKS; 
			// this->nDataBlocks = 1; // MIC! was 10
			this->BLOCK_FLAG_INITIAL_VALUE = 1;
			pthread_barrier_init(&barrier, NULL, nthreads);
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
				THREADS[t]=new std::thread(&Stencil3DImplementation<EL>::threadStencil3D<IN,OUT,ARGs...>, this, threadArguments, radius, nxs, nys, nzs, paddingOption, nIters, t, args...);
			}

			for(size_t t=0; t< nthreads; ++t)
			{
				THREADS[t]->join();
				delete THREADS[t];
			}
			output = tempOutput;
			delete[] threadArguments;
			
			// NEW
			if (nIters%2==0)
			{
				auto temp = input;
				input = output;
				output = temp;
			}
		}
		template<typename EL2>
		friend Stencil3DImplementation<EL2> __Stencil3DWithAccess(EL2 el, const size_t &radius, const size_t nxs, const size_t nys, const size_t nzs, const unsigned char &paddingOption, const size_t &nIters, const size_t &threads);
	};
	template<typename EL2>
	friend Stencil3DImplementation<EL2> __Stencil3DWithAccess(EL2 el, const size_t &radius, const size_t nxs, const size_t nys, const size_t nzs, const unsigned char &paddingOption, const size_t &nIters, const size_t &threads);
};

/*
 * We cannot define a friend function with default argument
 * that needs access to inner class on latest g++ compiler versions.
 * We need a wrapper!
 */
template<typename EL>
Stencil3DSkeleton::Stencil3DImplementation<EL> __Stencil3DWithAccess(EL el, const size_t &radius, const size_t nxs, const size_t nys, const size_t nzs, const unsigned char &paddingOption, const size_t &nIters, const size_t &threads)
{
    Stencil3DSkeleton::Elemental<EL> elemental(el);
    Stencil3DSkeleton::Stencil3DImplementation<EL> stencil3D(elemental, radius, nxs, nys, nzs, paddingOption, nIters, threads);
    return stencil3D;
}

template<typename EL>
Stencil3DSkeleton::Stencil3DImplementation<EL> Stencil3D(EL el, const size_t &radius, const size_t nxs, const size_t nys, const size_t nzs, const unsigned char &paddingOption, const size_t &nIters=1, const size_t &threads = 0)
{
    return __Stencil3DWithAccess(el, radius, nxs, nys, nzs, paddingOption, nIters, threads);
}

#endif /* STENCIL3D_HPP */
