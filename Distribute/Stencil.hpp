/**
 * Stencil.hpp
 * @class: StencilSkeleton
 * @author: sheng
 * @version: 0.0.0
 * @date: 2019-03-14 Thu 14:14
 */

#ifndef STENCIL_HPP
#define STENCIL_HPP

#include <iostream>
#include <vector>

class StencilSkeleton {
private:
	StencilSkeleton(){}

	template<typename EL>
	class Elemental {
	public:
		Elemental(EL el) : elemental(el) {}
		EL elemental;
	};
public:
	template<template EL>
	class StencilImplementation {
	private:
		unsigned char BLOCK_FLAG_INITIAL_VALUE;/*???*/
		size_t nthreads;
		size_t nDataBlocks;

		template<typename IN, typename OUT>
		class ThreadArgument {
		public:
			size_t threadInputIndex;/*???*/
			size_t chunkSize;
			size_t nDataBlocks;
			std::mutex *dataBlockMutex;
			unsigned char *dataBlockFlags;/*???*/
			size_t *dataBlockIndices;/*???*/
			std::vector<IN> *input;
			std::vector<OUT> *output;
			ThreadArgument() {}
			ThreadArgument(std::vector<OUT> &output,
						   std::vector<IN> &input,
						   size_t threadInputIndex,
						   size_t chunkSize)
				: threadInputIndex(threadInputIndex),
				  chunkSize(chunkSize),
				  input(&input),
				  output(&output) {}
			~ThreadArgument() {
				delete[] dataBlockIndices;
				delete[] dataBlockFlags;
				delete dataBlockMutex;
			}
		};

		template<typename IN, typename OUT, typename ...ARGs>
		void threadStencil(ThreadArgument<IN, OUT> *threadArguments, size_t threadID, ARGs... args) {
			auto input = threadArguments[threadID].input;
			auto output = threadArguments[threadID].output;
			size_t assistedThreadID = threadID;
			do {
				std::mutex *dataBlockMutex = threadArguments[assistedThreadID].dataBlockMutex;
				unsigned char *dataBlockFlags = threadArguments[assistedThreadID].dataBlockFlags;
				std::size_t *dataBlockIndices = threadArguments[assistedThreadID].dataBlockIndices;
				size_t nDataBlock = threadArguments[assistedThreadID].nDataBlocks;
				size_t dataBlock = 0;				
			} while (assistedThreadID != threadID);
		}

		Elemental<EL> elemental;

		StencilImplementation(Elemental<EL> elemental, size_t threads) : Elemental(elemental), nthreads(threads) {
			this->nDataBlocks = NDATABLOCKS;
			this->BLOCK_FLAG_INITIAL_VALUE = 1;
		}

	public:
		template<typename IN, typename OUT, typename ...ARGs>
		void operator()(std::vector<OUT> &output, std:vector<IN> &input, ARGs... args) {
			
		}

		template<typename EL2>
		friend StencilImplementation<EL2> __StencilWithAccess(EL2 el, const size_t &threads);
	};

	template<typename EL2>
	friend StencilImplementation<EL2> __StencilWithAccess(EL2 el, const size_t &thhreads);
};

template<typename EL>
StencilSkeleton::StencilImplementation<EL> __StencilWithAccess(EL el, const size_t &threads) {
    StencilSkeleton::Elemental<EL> elemental(el);
    StencilSkeleton::StencilolImplementation<EL> stencil(elemental, threads);
    return stencil;
}

template<typename EL>
StencilSkeleton::StencilImplementation<EL> Stencil(EL el, const size_t &threads = 0) {
    return __StencilWithAccess(el, threads);
}


#endif /* STENCIL_HPP */
