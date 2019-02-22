/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   testMap.cpp
 * Author: tonio
 *
 * Created on June 6, 2018, 3:02 PM
 */


#include <cassert>

#include "testMap.hpp"


#include "def_file.hpp"
#include "miscFun.hpp"

#include "Map.hpp"



int elemental(int a, int b) {

#ifdef UNBALANCED
    justAMoment( a < ( NITEMS / 2 ) ? 1 : COLLATZ );
#else
    justAMoment();
#endif

    return a + b;
}


int MapMain(int argc, char** argv) {

    size_t RUNS = NRUNS;
    size_t T = MTHREADS;
    size_t t = 1;
    size_t r = 0;

    int b = 2;

    long double seqProd = 0.0, skelProd = 0.0;

    std::vector<int> in(NITEMS);

    for(size_t i = 0; i < NITEMS; ++i){
        in[i] = i;
    }

    

    //
    //
    // Sequential
    //
    //
    std::vector<int> resSeq(in.size());
    
    while(r++<RUNS){

        size_t s = in.size();
        tstart = second();
            for(size_t index = 0; index < s; ++index)
                resSeq[ index ] = elemental( in[ index ], b );
        tstop = second();

        seqTime[r-1] = tstop-tstart;
        seqProd += seqTime[r-1];


    }
    
    double seqAVG = seqProd / RUNS;
    r = 0;
    
    //
    //
    // Skeleton
    //
    //
    while(t<=T) {

        while(r++<RUNS){

            std::vector<int> resSkel(in.size());

            auto map = Map(elemental,t);

            tstart = second();
                map(resSkel,in,b);
            tstop = second();

            skelTime[r-1] = tstop-tstart;
            skelProd += skelTime[r-1];

            assert(resSkel.size()==resSeq.size());
            for( size_t index = 0; index < resSkel.size(); ++index ) {
                assert(resSkel[ index ] == resSeq[ index ]);
            }
        }
    
        double skelAVG = skelProd / RUNS;
        // skelProd = 1.0; OOPS! MIC should really be called skelSum
        skelProd = 0.0;

        double gain = seqAVG / skelAVG;

        printResults(t,RUNS,seqTime,seqAVG,skelTime,skelAVG,gain);
        
        r = 0;
        t += 1;
        //t *= 2;
    }

    return 0;
}

