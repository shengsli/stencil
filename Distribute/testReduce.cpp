/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   testReduce.cpp
 * Author: tonio
 *
 * Created on June 5, 2018, 6:13 PM
 */


#include <cassert>

#include "testReduce.hpp"


#include "def_file.hpp"
#include "miscFun.hpp"

#include "Reduce.hpp"


//double* combine(double *a, double *b) {
double combine(double a, double b) {

#ifdef UNBALANCED
    justAMoment( b < ( NITEMS / 2 ) ? 1 : COLLATZ );
//    justAMoment( *b < ( NITEMS / 2 ) ? 1 : COLLATZ );
#else
    justAMoment();
#endif

//    double *c = new double;
//    *c = *a + *b;
//    return c;
    return a+b;
}



int ReduceMain(int argc, char** argv) {

    
    size_t RUNS = NRUNS;
    size_t T = MTHREADS;
    size_t t = 1;
    size_t r = 0;

    double seqSum = 0.0, skelSum = 0.0;

//    std::vector<double*> in(NITEMS);
//    double* seqRes;
//    double* tempSeqRes;

    std::vector<double> in(NITEMS);
    double seqRes;
    double tempSeqRes;

    


//    for(int i=1;i<=NITEMS;++i)
//        in[i-1] = new double(i);
    for(int i=1;i<=NITEMS;++i)
        in[i-1] = i;

    //
    //
    // Sequential
    //
    //
    while(r++<RUNS){
        
        tstart = second();
            tempSeqRes = in[0];
            seqRes = combine(tempSeqRes,in[1]);
            size_t e;
            for( e = 2; e < NITEMS; ++e) {
                tempSeqRes = seqRes;
                seqRes = combine(tempSeqRes,in[e]);
//                delete tempSeqRes;
            }

        tstop = second();
        seqTime[r-1] = tstop-tstart;
        seqSum += seqTime[r-1];

//        if( r < RUNS ) delete seqRes;
    }
    
    double seqAVG = seqSum / RUNS;
    r = 0;
    
    //
    //
    // Skeleton
    //
    //
    while(t<=T) {
        while(r++<RUNS){

//            double *skelRes;
            double skelRes;

            auto reduce = Reduce(combine,t);

            tstart = second();
                reduce(skelRes,in);
            tstop = second();
            skelTime[r-1] = tstop-tstart;
            skelSum += skelTime[r-1];

//            if( *skelRes!=*seqRes ){
//                printMessage(std::to_string(*skelRes)+" != "+std::to_string(*seqRes));
//            }

//            assert(*skelRes==*seqRes);
            assert(skelRes==seqRes);

//            delete skelRes;

        }

        double skelAVG = skelSum / RUNS;
        skelSum = 0.0;

        double gain = seqAVG / skelAVG;

        printResults(t,RUNS,seqTime,seqAVG,skelTime,skelAVG,gain);

        r = 0;
        t += 1;
        //t *= 2;
    }

//    for(int i=1;i<=NITEMS;++i)
//        delete in[i-1];
//
//    delete seqRes;
//    
////    std::vector<int> n1(20,1);
////    int r1;
////    
////    auto reduce1 = Reduce([](int a,int b, int c){return a+b+c;});
////    reduce1(r1,n1,5);
////    
////    std::cout << r1 << endl;


    return 0;
} /**/

