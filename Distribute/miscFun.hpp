/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   miscFun.hpp
 * Author: tonio
 *
 * Created on June 5, 2018, 6:14 PM
 */


#ifndef MISCFUN_HPP
#define MISCFUN_HPP

#include <cstdlib>
#include <string>
#include <vector>


#include "def_file.hpp"

#if defined(MAP) || defined(REDUCE) || defined(MAPREDUCE) || defined(SCAN)
    void justAMoment(size_t n = COLLATZ);
#endif

double second();

void printResults(size_t threads, size_t RUNS, double* seqTime, double seqAVG, double* skelTime, double parAVG, double ratio);

void printMessage(std::string s);


int iRand(int iMin, int iMax);

double fRand(double fMin=0.0, double fMax=1.0);


extern double tstart,tstop, skelTime[NRUNS], seqTime[NRUNS];


#endif /* MISCFUN_HPP */

