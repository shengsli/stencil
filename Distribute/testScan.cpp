//
// Created by tonio on 16/07/18.
//

#include <cassert>

#include "testScan.hpp"

#include "def_file.hpp"
#include "miscFun.hpp"

#include "Scan.hpp"


//double* combine(double *a, double *b) {
double combine(double a, double b) {

#ifdef UNBALANCED
    justAMoment( b < 1.0 ? 1 : COLLATZ );
#else
    justAMoment();
#endif

    return a+b;
}



int ScanMain(int argc, char** argv) {


    size_t RUNS = NRUNS;
    size_t r = 0;

    size_t T = MTHREADS;
    size_t t = 1;

    double seqSum = 0.0, skelSum = 0.0;

//    std::vector<double*> in(NITEMS);
    std::vector<double> in(NITEMS,0.0);
//    std::vector<double*> seqRes(NITEMS);
    std::vector<double> seqRes(NITEMS);

    for(int i=(NITEMS/2);i<NITEMS;++i)
        in[i] = (double)i;



    //
    //
    // Sequential
    //
    //
    while(r++<RUNS){
        std::cout << "Run: " <<r << std::endl;

        tstart = second();
            seqRes[0] = in[0];
            for( size_t e = 1; e < NITEMS; ++e) {
               seqRes[e] = combine(seqRes[e-1],in[e]);
            }
        tstop = second();
        seqTime[r-1] = tstop-tstart;
        seqSum += seqTime[r-1];

//        if( r < RUNS ) {
//            for( size_t e = 0; e < NITEMS; ++e) {
//                delete seqRes[e];
//            }
//        }
//        for( size_t e = 1; e < NITEMS; ++e) {
//            delete in[e];
//        }
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

            std::cout << "Run: " <<r << std::endl;
//            std::vector<double*> skelRes(NITEMS);
            std::vector<double> skelRes(NITEMS);

            auto scan = Scan(combine,t);

            tstart = second();
                scan(skelRes,in);
            tstop = second();
            skelTime[r-1] = tstop-tstart;
            skelSum += skelTime[r-1];

//            assert( skelRes.size() == seqRes.size() );
//            for( size_t i = 0; i < skelRes.size(); ++i ) {
//                assert(*skelRes[i]==*seqRes[i]);
//            }
//
//            for( size_t e = 0; e < NITEMS; ++e) {
//                delete skelRes[e];
//                delete in[e];
//            }

            assert( skelRes.size() == seqRes.size() );
            for( size_t i = 0; i < skelRes.size(); ++i ) {
                assert(skelRes[i]==seqRes[i]);
            }

        }

        double skelAVG = skelSum / RUNS;
        skelSum = 0.0;

        double gain = seqAVG / skelAVG;

        printResults(t,RUNS,seqTime,seqAVG,skelTime,skelAVG,gain);

        r = 0;
        t += 1;
        //t *= 2;
    }

//    for(int i=0;i<NITEMS;++i)
//        delete seqRes[i];
//        delete in[i];
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

