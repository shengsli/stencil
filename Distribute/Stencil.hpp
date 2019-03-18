/**
 * Stencil.hpp
 * \class: StencilSkeleton
 * \author: sheng
 * \version: 0.0.0
 * \date: 2019-03-14 Thu 14:14
 */

#ifndef STENCIL_HPP
#define STENCIL_HPP

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
		size_t nthreads;
		size_t nDataBlocks;

		template<typename IN, typename OUT>
		class ThreadArgument {
		public:
			size_t chunkSize;
		}
	}
};

#endif /* STENCIL_HPP */
