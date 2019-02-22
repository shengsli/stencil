/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   def_file.hpp
 * Author: tonio
 *
 * Created on June 6, 2018, 4:41 PM
 */


#ifndef DEF_FILE_HPP
#define DEF_FILE_HPP

#include <cmath>

/*
 * Benchmark
 */
#if defined(MAP) || defined(REDUCE) || defined(MAPREDUCE) || defined(SCAN)

    #ifndef NRUNS
        #define NRUNS 100
    #endif

    #ifndef NITEMS
        #define NITEMS 100000
    #endif

    #ifndef MTHREADS
        #define MTHREADS 32
    #endif

    #ifdef SCOLLATZ
        #define COLLATZ 9
    #endif

    #ifdef MCOLLATZ
        #define COLLATZ 97
    #endif

    #ifdef HCOLLATZ
        #define COLLATZ 871
    #endif

    #ifdef UHCOLLATZ
        #define COLLATZ 77031
    #endif

    #ifndef COLLATZ
        #define COLLATZ 871
    #endif

#endif


/*
 * KMeans
 */
#if defined(KMEANS) || defined(KMEANS_MR)

    #ifndef NPOINTS
        #define NPOINTS 100
    #endif

    #ifndef NCLUSTERS
        #define NCLUSTERS static_cast<int>(log10(NPOINTS)+1)
    #endif

    #ifndef SEED
        #define SEED 2909
    #endif

    #ifndef EPSILON
        #define EPSILON 0.00005
    #endif

    #ifndef NTHREADS
        #define NTHREADS 8
    #endif

    #ifndef MIN_POINTS
        #define MIN_POINTS 100
    #endif

    #ifndef MAX_POINTS
        #define MAX_POINTS 100000
    #endif

    #ifndef MIN_CLUSTERS
        #define MIN_CLUSTERS static_cast<int>(log10(MIN_POINTS)+1)
    #endif

    #ifndef MAX_CLUSTERS
        #define MAX_CLUSTERS static_cast<int>(log10(MAX_POINTS)+1)
    #endif

    #ifndef NRUNS
        #define NRUNS 1
    #endif
#endif

/*
 * RadixSort
 */
#if defined(RADIX_SORT)

    #ifndef NELEMENTS
        #define NELEMENTS 100000000
    #endif

    #ifndef NUM_LENGTH
        #define NUM_LENGTH 6
    #endif

    #ifndef NUM_BASE
        #define NUM_BASE 3
    #endif

    #ifndef SEED
        #define SEED 2909
    #endif

    #ifndef EPSILON
        #define EPSILON 0.00005
    #endif

    #ifndef NTHREADS
        #define NTHREADS 8
    #endif

    #ifndef NRUNS
        #define NRUNS 10
    #endif

    #ifndef MIN_NUM_BASE
        #define MIN_NUM_BASE 1
    #endif

    #ifndef MAX_NUM_BASE
        #define MAX_NUM_BASE 6
    #endif

    #ifndef MIN_NUM_LENGTH
        #define MIN_NUM_LENGTH 1
    #endif

    #ifndef MAX_NUM_LENGTH
        #define MAX_NUM_LENGTH 3
    #endif

    #if MAX_NUM_LENGTH > 6
        #undef MAX_NUM_LENGTH
        #define MAX_NUM_LENGTH 6
    #endif

    #if NUM_LENGTH > 6
        #undef NUM_LENGTH
        #define NUM_LENGTH 6
    #endif
#endif

/*
 * SummedArea
 */
#if defined(SUMMEDAREA)



#ifndef MIN_SAMPLES
    #define MIN_SAMPLES 1e4
#endif

#ifndef MAX_SAMPLES
    #define MAX_SAMPLES 1e6
#endif

#ifndef MIN_SIMPSON_SAMPLES
    #define MIN_SIMPSON_SAMPLES 1e1
#endif

#ifndef MAX_SIMPSON_SAMPLES
    #define MAX_SIMPSON_SAMPLES 1e3
#endif

#ifndef START
    #define START 1
#endif

#ifndef END
    #define END 10
#endif

#ifndef TIMING
    #define TIMING
#endif





#ifndef SEED
    #define SEED 2909
#endif

#ifndef EPSILON
    #define EPSILON 0.00005
#endif

#ifndef NRUNS
    #define NRUNS 1
#endif

#ifndef NTHREADS
    #define NTHREADS 8
#endif

#endif

#endif /* DEF_FILE_HPP */

