// DANGER DANGER this is an evolving 2D Stencil skeleton
// could be chaotic

// This version assumes wrap-around boundaries
// and a square neighbourhood, extending "width" items from the central
// point in each direction. (Eg width == 1 gives a 9 point neighbourhood,
// width == 2 gives a 25 point neighbourhood and so on)

// Because of issues with C multi-dimensional arrays, we make the
// neighbourhood a 1D array, which we treat as though it is 2D
// by making careful use of index calculations.

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Stencil2d.hpp
 * Author: tonio / murray
 *
 */



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

class Stencil2DSkeleton {

    private:
        Stencil2DSkeleton(){}

        /*
         *
         * Elemental
         *
         *
         */
        template<typename EL>
        class Elemental {
            public:
                Elemental(EL el) : elemental(el) {}
                EL elemental;
        };

    public:

        /**
         *
         * Stencil2DImplementation
         *
         *
         */
        template<typename EL>
        class Stencil2DImplementation {

             private:
                unsigned char BLOCK_FLAG_INITIAL_VALUE;
                size_t nthreads;
	        size_t width;      // NEW
	        size_t nrows;      // NEW
	        size_t ncols;      // NEW
                size_t nDataBlocks;

                /*
                 *
                 *ThreadArgument
                 *
                 */
                template<typename IN, typename OUT>
                class ThreadArgument {

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

                        ThreadArgument(std::vector<OUT> &output, std::vector<IN> &input, size_t threadInputIndex, size_t chunkSize)
                                : threadInputIndex(threadInputIndex), chunkSize(chunkSize), input(&input), output(&output) {
                        }

                        ~ThreadArgument() {

                            delete[] dataBlockIndices;
                            delete[] dataBlockFlags;
                            delete dataBlockMutex;
                        }

                };


                template<typename IN, typename OUT, typename ...ARGs>
                void threadStencil2D(ThreadArgument<IN,OUT> *threadArguments, size_t width, size_t nrows, size_t ncols, size_t threadID, ARGs... args) {  //NEW

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
                        while( dataBlock < nDataBlocks ) {

                            if( dataBlockFlags[ dataBlock ] == 0 // if the data block has been, or being processed by another thread...
                                || dataBlockIndices[ dataBlock ] == dataBlockIndices[ dataBlock+1 ] ){
                                ++dataBlock;
                                continue; //as we iterate in reverse -> continue. In case there are no more dataBlocks, as chunkSize might be less than NDATABLOCKS
                            }

                            dataBlockMutex->lock();
                            if( dataBlockFlags[ dataBlock ] == 1 ){ // were the flag is zero, the following flags are zero too.
                                dataBlockFlags[ dataBlock ] = 0;
                                dataBlockMutex->unlock();

                                
                                // We would ideally want neighbourhood to be a genuine 2D array with flexible
				// row and column sizes, but the C++ compiler can't handle that. It needs
				// the everything but the first dimension size to be a comnpile-time constant.
				// So, we fake 2D arrays  with a 1D array and careful use of array indexing
				// This also affect the user code for the elemental function - (see examples)

 		                IN *neighbourhood = (IN *) malloc (sizeof(IN) * (2*width +1) * (2*width +1));

				// We assume wrap-around boundaries in this version
                                for(size_t elementIndex = dataBlockIndices[ dataBlock ]; elementIndex < dataBlockIndices[ dataBlock+1 ]; ++elementIndex) {
                                    for(size_t i=0; i<=2*width; i++) {
                                       for(size_t j=0; j<=2*width; j++) {
					 neighbourhood[i+(2*width +1) +j] = input->at(elementIndex);  // this needs be calculated correctly!!
					                                                              // ie to take the correct elements from input
					                                                              // into the neighbourhood. This version
					                                                              // just copies the central point, which is wrong
                                       }
                                    }
                                    output->at(elementIndex) = elemental.elemental( neighbourhood, width, args... );
                                }
                            }
                            else { // Just in case after the first if, the flag changes its value to 0 from another thread
                                dataBlockMutex->unlock();
                            }
                            ++dataBlock;
                        }

                        assistedThreadID = (assistedThreadID + 1) % nthreads;
                    }while(  assistedThreadID != threadID  );
                }

                Elemental<EL> elemental;

	  Stencil2DImplementation(Elemental<EL> elemental, size_t width, size_t nrows, size_t ncols, size_t threads) : elemental(elemental), width(width), nrows(nrows), ncols(ncols), nthreads(threads){ 
		  this->nDataBlocks = NDATABLOCKS; 
                    // this->nDataBlocks = 1; MIC! was 10
                    this->BLOCK_FLAG_INITIAL_VALUE = 1;
                }

            public:

                template<typename IN, typename OUT, typename ...ARGs>
                void operator()(std::vector<OUT> &output, std::vector<IN> &input, ARGs... args) {

                    nthreads = nthreads ? nthreads : std::thread::hardware_concurrency();

                    nthreads = nthreads >= input.size() ? input.size() / 2 : nthreads;

                    /*
                     * TODO: Handle input.size == 0 or 1
                     * Hardcoded for now...
                     */
                    if( input.size() == 0 ) {
                        return;
                    }
                    if( input.size() == 1 ) {

		      //output[0] = elemental.elemental( input[0], args...);
                        return;
                    }

                    std::thread *THREADS[nthreads];
                    ThreadArgument<IN,OUT> *threadArguments = new ThreadArgument<IN,OUT>[nthreads];

                    std::vector<OUT> tempOutput( input.size() );
                    size_t chunkIndex = 0;


                    for(size_t t=0; t< nthreads; ++t ){

                        if( t < (input.size() % nthreads) ) threadArguments[t].chunkSize = 1 + input.size() / nthreads;
                        else threadArguments[t].chunkSize = input.size() / nthreads;

                        threadArguments[t].input = &input;
                        threadArguments[t].output = &tempOutput;
                        threadArguments[t].threadInputIndex = chunkIndex;

                        chunkIndex += threadArguments[t].chunkSize;

                        /*
                         * Data Blocks
                         */
                        nDataBlocks = nDataBlocks > threadArguments[t].chunkSize ? threadArguments[t].chunkSize / 2 : nDataBlocks;

                        threadArguments[t].nDataBlocks = nDataBlocks;

                        threadArguments[t].dataBlockFlags = new unsigned char[nDataBlocks]();
                        std::fill_n( threadArguments[t].dataBlockFlags, nDataBlocks, BLOCK_FLAG_INITIAL_VALUE );

                        threadArguments[t].dataBlockMutex = new std::mutex;

                        threadArguments[t].dataBlockIndices = new size_t[nDataBlocks+1]();

                        size_t blockSize, blockStart = threadArguments[t].threadInputIndex, blockEnd;

                        for(size_t block = 0; block < nDataBlocks; ++block) {

                            if( block < (threadArguments[t].chunkSize % nDataBlocks) ) blockSize = 1 + threadArguments[t].chunkSize / nDataBlocks;
                            else blockSize = threadArguments[t].chunkSize / nDataBlocks;

                            blockEnd = blockStart + blockSize;
                            threadArguments[t].dataBlockIndices[ block ] = blockStart;
                            blockStart = blockEnd;
                        }
                        threadArguments[t].dataBlockIndices[ nDataBlocks ] = blockEnd;

                    }

                    for(size_t t=0; t< nthreads; ++t ){
		      THREADS[t] = new std::thread(&Stencil2DImplementation<EL>::threadStencil2D<IN,OUT,ARGs...>, this, threadArguments, width, nrows, ncols, t, args...); 
                    }

                    for(size_t t=0; t< nthreads; ++t){
                        THREADS[t]->join();
                        delete THREADS[t];
                    }

                    output = tempOutput;

                    delete[] threadArguments;
                }

            template<typename EL2>
            friend Stencil2DImplementation<EL2> __Stencil2DWithAccess(EL2 el, const size_t &width, const size_t &nrows ,const size_t &ncols, const size_t &threads);
        };

        template<typename EL2>
        friend Stencil2DImplementation<EL2> __Stencil2DWithAccess(EL2 el, const size_t &width, const size_t &nrows ,const size_t &ncols, const size_t &threads);
};

/*
 * We cannot define a friend function with default argument
 * that needs access to inner class on latest g++ compiler versions.
 * We need a wrapper!
 */
template<typename EL>
Stencil2DSkeleton::Stencil2DImplementation<EL> __Stencil2DWithAccess(EL el, const size_t &width, const size_t &nrows, const size_t &ncols, const size_t &threads) {

    Stencil2DSkeleton::Elemental<EL> elemental(el);
    Stencil2DSkeleton::Stencil2DImplementation<EL> stencil(elemental, width, nrows, ncols, threads);

    return stencil;
}

template<typename EL>
Stencil2DSkeleton::Stencil2DImplementation<EL> Stencil2D(EL el, const size_t &width, const size_t &nrows ,const size_t &ncols, const size_t &threads = 0) {

  return __Stencil2DWithAccess(el, width, nrows, ncols, threads);
}

#endif /* STENCIL2D_HPP */

