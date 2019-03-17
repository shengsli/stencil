/*
 * File:   Stencil.hpp
 * Author: sheng
 * 
 * Created on March 14, 2019, 02:00 PM
 * This skeleton follows tonio's style. 
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
		
	}
};

#endif /* STENCIL_HPP */
