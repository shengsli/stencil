//
// Created by tonio on 26/07/18.
//


#include <cstdlib>
#include <utility>
#include <cmath>
#include <map>
#include <typeinfo>

#include <cassert>
#include <algorithm>


#include "def_file.hpp"
#include "miscFun.hpp"

#include "Scan.hpp"
#include "Map.hpp"

#include "problemSummedArea.hpp"

namespace SummedArea {


    double fun(double x){

        return x/8 + std::sin(2*x)*std::log(x+1);
    }

    template<typename F>
    double Simpson(double a, double b, F f, int n = 10){

        double dx = (b-a)/(double)n;

        double dx3 = dx/3;

        double sum = f(a);

        for( int i = 1; i < n; ++i ){

            double xi = a + i*dx;

            sum += f(xi) * ( (i%2)*2 + 2);
        }

        sum += f( a+ n*dx );

        double area = sum * dx3;

        return area;
    }

    template<typename S, typename F>
    double mapper( double sample, double dx, S simpson, F fun, int n ){

        return simpson( sample, sample+dx, fun, n );
    }

    double successor( double a, double b ){

        return a + b;
    }

    std::vector<double> SummedAreaSkel(std::vector<double> samples, double dx, double a, double b, int n,
                                       double &mapTimings, double &scanTimings, double &skelSeqTiming) {

        auto start = second();
            using F = decltype(fun);
            using S = decltype(Simpson<F>);
            auto map = Map( mapper<S,F>, NTHREADS );
            auto scan = Scan( successor, NTHREADS );
            std::vector<double> sampleArea( samples.size() );
            std::vector<double> summedArea( samples.size() );
        skelSeqTiming = second() - start;

        start = second();
            map( sampleArea, samples, dx, Simpson<F>, fun, n );
        mapTimings = second() - start;

        start = second();
            scan( summedArea, sampleArea );
        scanTimings = second() - start;

        return summedArea;
    }


    std::vector<double> SummedAreaSeq(std::vector<double> samples, double dx, double a, double b, int n,
                                      double &nonParallelTimings) {

        auto start = second();
            std::vector<double> sampleArea( samples.size() );
            std::vector<double> summedArea( samples.size() );
        nonParallelTimings = second() - start;


        for( int i = 0; i < sampleArea.size(); ++i ){

            sampleArea[ i ] = Simpson( samples[ i ], samples[ i ]+dx, fun, n );
        }

        summedArea[0] = sampleArea[0];
        for( int i = 1; i < sampleArea.size(); ++i ){

            summedArea[ i ] = sampleArea[ i ] + summedArea[ i-1 ];
        }

        return summedArea;
    }


    class ThreadArgGetArea{

        public:
            size_t sampleIndexStart;
            size_t sampleIndexEnd;
            double dx;
            int n;
            std::vector<double> *samples;
            std::vector<double> *sampleArea;
    };

    void pthreadGetAreas(std::vector<ThreadArgGetArea>* threadArguments, size_t threadID){

        auto samples = threadArguments->operator[](threadID).samples;
        auto sampleArea = threadArguments->operator[](threadID).sampleArea;
        auto sampleIndexStart = threadArguments->operator[](threadID).sampleIndexStart;
        auto sampleIndexEnd = threadArguments->operator[](threadID).sampleIndexEnd;
        auto dx = threadArguments->operator[](threadID).dx;
        auto n = threadArguments->operator[](threadID).n;


        for( size_t i = sampleIndexStart; i < sampleIndexEnd; ++i ){

            sampleArea->operator[](i) = Simpson( samples->operator[](i), samples->operator[](i)+dx,fun,n );
        }
    }

    std::mutex pthreadBarrierLock;
    std::condition_variable pthreadCond_Var;
    size_t pthreadThreadsArrived;
    void pthreadBarrier(size_t nThreadsToBarrier) {

        auto lck = std::unique_lock<std::mutex>(pthreadBarrierLock);

        pthreadThreadsArrived++;
        if(pthreadThreadsArrived == nThreadsToBarrier) {

            pthreadCond_Var.notify_all();
            pthreadThreadsArrived = 0;
        }
        else {
            pthreadCond_Var.wait( lck );
        }
    }
    class ThreadArgGetSummedArea{
        public:
            size_t chunkStart;
            size_t chunkEnd;
            std::vector<double> *sampleArea;
            std::vector<double> *summedArea;
    };

    void pthreadGetSummedArea(std::vector<ThreadArgGetSummedArea>* threadArguments, size_t threadID ){

        auto chunkStart = threadArguments->operator[](threadID).chunkStart;
        auto chunkEnd = threadArguments->operator[](threadID).chunkEnd;
        auto sampleArea = threadArguments->operator[](threadID).sampleArea;
        auto summedArea = threadArguments->operator[](threadID).summedArea;

        summedArea->operator[](chunkStart) = sampleArea->operator[](chunkStart);

        for(size_t i = chunkStart + 1; i < chunkEnd; ++i){
            summedArea->operator[](i) = summedArea->operator[](i-1) + sampleArea->operator[](i);
        }

        pthreadBarrier( threadArguments->size() );

        if( threadID == 0){

            for( size_t threadIndex = 1; threadIndex < threadArguments->size(); ++threadIndex){

                auto threadLastElementIndex = threadArguments->operator[](threadIndex).chunkEnd - 1;
                auto prevThreadLastElementIndex = threadArguments->operator[](threadIndex - 1).chunkEnd - 1;

                summedArea->operator[]( threadLastElementIndex ) =
                        summedArea->operator[](prevThreadLastElementIndex) + summedArea->operator[](threadLastElementIndex);
            }
        }

        pthreadBarrier( threadArguments->size() );


        if(threadID != 0) {

            auto prevThreadLastElementIndex = threadArguments->operator[](threadID - 1).chunkEnd - 1;
            auto prevThreadLastElement = summedArea->operator[]( prevThreadLastElementIndex );

            for(size_t i = chunkStart; i < chunkEnd - 1; ++i){
                summedArea->operator[](i) = summedArea->operator[](i) + prevThreadLastElement;
            }
        }
    }

    std::vector<double> SummedAreaPThreads(std::vector<double> samples, double dx, double a, double b, int n,
                                           double &nonParallelTimings) {

        std::vector<double> sampleArea( samples.size() );
        std::vector<double> summedArea( samples.size() );

        std::thread *THREADS[NTHREADS];
        size_t chunkSize, chunkStart = 0, chunkEnd;

        std::vector<ThreadArgGetArea> threadArgumentsGetArea;
        for (size_t t = 0; t < NTHREADS; ++t) {

            if (t < (sampleArea.size() % NTHREADS)) chunkSize = 1 + samples.size() / NTHREADS;
            else chunkSize = samples.size() / NTHREADS;

            chunkEnd = chunkStart + chunkSize;

            ThreadArgGetArea arg;

            arg.sampleIndexStart = chunkStart;
            arg.sampleIndexEnd = chunkEnd;
            arg.dx = dx;
            arg.n = n;
            arg.samples = &samples;
            arg.sampleArea = &sampleArea;

            threadArgumentsGetArea.push_back(arg);

            chunkStart = chunkEnd;
        }

        for (size_t t = 0; t < NTHREADS; ++t) {

            THREADS[t] = new std::thread(pthreadGetAreas, &threadArgumentsGetArea, t);
        }

        for(size_t t=0; t < NTHREADS; ++t){
            THREADS[t]->join();
            delete THREADS[t];
        }


        pthreadThreadsArrived = 0;
        chunkStart = 0;

        std::vector<ThreadArgGetSummedArea> threadArgumentsGetSummedArea;
        for (size_t t = 0; t < NTHREADS; ++t) {

            if (t < (sampleArea.size() % NTHREADS)) chunkSize = 1 + sampleArea.size() / NTHREADS;
            else chunkSize = sampleArea.size() / NTHREADS;

            chunkEnd = chunkStart + chunkSize;

            ThreadArgGetSummedArea arg;

            arg.chunkStart = chunkStart;
            arg.chunkEnd = chunkEnd;
            arg.sampleArea = &sampleArea;
            arg.summedArea = &summedArea;


            threadArgumentsGetSummedArea.push_back(arg);

            chunkStart = chunkEnd;
        }

        for (size_t t = 0; t < NTHREADS; ++t) {

            THREADS[t] = new std::thread(pthreadGetSummedArea, &threadArgumentsGetSummedArea, t);
        }

        for(size_t t=0; t < NTHREADS; ++t){
            THREADS[t]->join();
            delete THREADS[t];
        }

        return summedArea;
    }

    /*
     *
     */
    int SummedAreaMain(int argc, char **argv) {

        double tstart, tstop;

        std::cout << "Problem Summed-Area" << std::endl;

        double a = START;
        double b = END;

        for (size_t nsamples = MIN_SAMPLES; nsamples <= MAX_SAMPLES; nsamples *= 10) {
            for (size_t simpsonSamples = MIN_SIMPSON_SAMPLES; simpsonSamples <= MAX_SIMPSON_SAMPLES; simpsonSamples *= 10) {

                std::cout << "########################" << std::endl;
                std::cout << "Summed-Area problem with:\tMap and Scan Skeletons" << std::endl;
                std::cout << "Size of summed-area table:\t" << nsamples << std::endl;
                std::cout << "Number of simpson samples:\t" << simpsonSamples << std::endl;
                std::cout << "Number of Threads:\t" << NTHREADS << std::endl;
                std::cout << "Hardware Concurrency:\t" << std::thread::hardware_concurrency() << std::endl;
                std::cout << "Number of Runs (for AVG):\t" << NRUNS << std::endl;

                std::srand(SEED);

                std::vector<double> samples(nsamples);

                double dx = (b-a)/nsamples;

                for (size_t i = 0; i < nsamples; ++i) {

                    samples[i] = i*dx;
                }


                double skelTime = 0.0, skelMapTime = 0.0, skelScanTime = 0.0, skelSeqTime = 0.0,
                        seqTime = 0.0, seqNonParTime = 0.0,
                        pthreadTime = 0.0;

                size_t r = 0;

                decltype(samples) skelElements, seqElements, pthreadsElements;

                while (r++ < NRUNS) {

                    /*
                     * Skeleton
                     */
                    double skelMapTiming, skelScanTiming, skelSeqTiming;
                    tstart = second();
                        skelElements = SummedAreaSkel(samples, dx, a, b, simpsonSamples, skelMapTiming, skelScanTiming, skelSeqTiming);
                    tstop = second();
                    skelTime += tstop - tstart;

                    skelMapTime += skelMapTiming;

                    skelScanTime += skelScanTiming;

                    skelSeqTime += skelSeqTiming;


                    /*
                     * Sequential
                     */
                    double nonParallelTimings;

                    tstart = second();
                        seqElements = SummedAreaSeq(samples, dx, a, b, simpsonSamples, nonParallelTimings);
                    tstop = second();
                    seqTime += tstop - tstart;

                    seqNonParTime += nonParallelTimings;


                    /*
                     * Pthreads
                     */
                    double timings;
                    tstart = second();
                        pthreadsElements = SummedAreaPThreads(samples, dx, a, b, simpsonSamples, timings);
                    tstop = second();
                    pthreadTime += tstop - tstart;

                    /*
                     * Validate results
                     */
                    assert(seqElements.size() == skelElements.size());
                    for (size_t i = 0; i < seqElements.size(); ++i) {
//                        std::cout << seqElements[i] << std::endl;
                        assert(seqElements[i] - skelElements[i] < 1e-9);
                    }

                    assert(seqElements.size() == pthreadsElements.size());
                    for (size_t i = 0; i < seqElements.size(); ++i) {
//                        std::cout << seqElements[i] << std::endl;;
//                        std::cout << pthreadsElements[i] << std::endl;;

                        assert(seqElements[i] - pthreadsElements[i] < 1e-9);
                    }
                }

                printMessage("Results validated");

                std::cout << "Sequential Time:\t" << seqTime / NRUNS << std::endl;
                std::cout << "Seq. Non-Parallel Time:\t" << seqNonParTime / NRUNS << std::endl;
                std::cout << "Skeleton Time:\t" << skelTime / NRUNS << std::endl;
                std::cout << "skelMap Time:\t" << skelMapTime / NRUNS << std::endl;
                std::cout << "skelScan Time:\t" << skelScanTime / NRUNS << std::endl;
                std::cout << "skelSeq Time:\t" << skelSeqTime / NRUNS << std::endl;
                std::cout << "PThread Time:\t" << pthreadTime / NRUNS << std::endl;
                std::cout << "Seq Gain:\t" << seqTime / skelTime << std::endl;
                std::cout << "PThread Gain:\t" << pthreadTime / skelTime << std::endl;
            }
        }

        return 0;
    }
}
