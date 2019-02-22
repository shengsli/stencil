//
// Created by tonio on 23/07/18.
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
#include "MapReduce.hpp"

#include "problemRadixsort.hpp"

namespace RadixSort {


    size_t getRandElement(size_t numberLen, size_t base){

        size_t exp = numberLen*base;
        size_t maxValue = pow( 10, exp );
        size_t upperLimit = maxValue - 1;

        return std::max((size_t)(upperLimit*fRand()),(size_t)1);
    }

    size_t getDigit(size_t element, size_t digitPos, size_t base ){

        size_t b = pow(10, base * (digitPos-1));

        size_t m = pow(10,base);

        return (element / b) % m;
    }


    std::list<std::pair<size_t,size_t>> countDigitOccurrences(size_t element, size_t digitPos, size_t base){

        auto digit = getDigit(element, digitPos, base );

        return {std::make_pair(digit,1)};
    };

    std::list<size_t> reduceDigitCounts(size_t key, std::list<size_t> counts, size_t digitPos, size_t base){

        return {counts.size()};
    }

    size_t successor(size_t a, size_t b){

        return a+b;
    }

    std::vector<size_t> RadixSortSkel(std::vector<size_t> elements, size_t numberLen, size_t base,
                                      std::vector<double> &mapReduceTimings, std::vector<double> &scanTimings, std::vector<double> &skelSeqTiming ) {

//        auto mapReduce = MapReduce(countDigitOccurrences, reduceDigitCounts, [](int k){ return std::hash<size_t >{}(k); });
        auto scan = Scan(successor);

        std::vector<size_t> orderedElements( elements.size() );

        for( size_t digitPos = 1; digitPos < numberLen + 1; ++digitPos ){

//            std::vector<std::pair<size_t, std::vector<size_t> >> digitCountPairs;
            std::vector<int> digitCount( pow(10,base), 0 );

            auto start = second();
//                mapReduce( digitCountPairs, elements, digitPos, base );
//
//                for( auto digitCountPair : digitCountPairs ){
//
//                    digitCount[ digitCountPair.first ] = digitCountPair.second[0];
//                }
                for( auto element : elements ){

                    auto digit = getDigit(element, digitPos, base );

                    digitCount[ digit ]++;
                }
            mapReduceTimings.push_back( second() - start );

            start = second();
                scan( digitCount, digitCount );
            scanTimings.push_back( second() - start );

            start = second();
                for( size_t elementIndex = elements.size() - 1; elementIndex >= 0 && elementIndex < elements.size(); --elementIndex ){

                    auto digit = getDigit( elements[elementIndex], digitPos, base );

                    orderedElements[ digitCount[ digit ] - 1 ] = elements[ elementIndex ];

                    digitCount[ digit ]--;
                }

                elements.clear();
                elements = orderedElements;
            skelSeqTiming.push_back( second() - start );
        }

        return elements;
    }


    std::vector<size_t> RadixSortSeq(std::vector<size_t> elements, size_t numberLen, size_t base,
                                     std::vector<double> &nonParallelTimings ) {

        std::vector<size_t> orderedElements( elements.size() );

        for( size_t digitPos = 1; digitPos < numberLen + 1; ++digitPos ){

            std::vector<int> digitCount( pow(10,base), 0 );

            for( auto element : elements ){

                auto digit = getDigit(element, digitPos, base );

                digitCount[ digit ]++;
            }

            for( size_t digitIndex = 1; digitIndex < digitCount.size(); ++digitIndex ){

                digitCount[ digitIndex ] += digitCount[ digitIndex - 1 ];
            }

            auto start = second();
                for( size_t elementIndex = elements.size() - 1; elementIndex >= 0 && elementIndex < elements.size(); --elementIndex ){

                    auto digit = getDigit( elements[elementIndex], digitPos, base );

                    orderedElements[ digitCount[ digit ] - 1 ] = elements[ elementIndex ];

                    digitCount[ digit ]--;
                }
                elements.clear();
                elements = orderedElements;
            nonParallelTimings.push_back( second() - start );
        }

        return elements;
    }


    std::vector<size_t> RadixSortPThreads(std::vector<size_t> elements, std::vector<double> &nonParallelTimings) {

        return elements;
    }

    /*
     *
     */
    int RadixSortMain(int argc, char **argv) {

        double tstart, tstop;

        size_t numberLen = NUM_LENGTH;
        size_t base = NUM_BASE;


        std::cout << "Problem Radix-Sort" << std::endl;

#if defined(TIMING)
        for (base = MIN_NUM_BASE; base <= MAX_NUM_BASE; ++base) {
            for (numberLen = MIN_NUM_LENGTH; numberLen <= MAX_NUM_LENGTH; ++numberLen) {
#endif

                std::cout << "########################" << std::endl;
                std::cout << "Radix-Sort problem with:\tMapReduce and Scan Skeletons" << std::endl;
                std::cout << "Number of elements:\t" << NELEMENTS << std::endl;
                std::cout << "Base:\t" << base << std::endl;
                std::cout << "Length of number:\t" << numberLen << std::endl;
                std::cout << "Number of Threads:\t" << NTHREADS << std::endl;
                std::cout << "Hardware Concurrency:\t" << std::thread::hardware_concurrency() << std::endl;
                std::cout << "Number of Runs (for AVG):\t" << NRUNS << std::endl;
//#if !defined(TIMING)
                std::cout << "########################" << std::endl;
//#endif

                std::srand(SEED);

                std::vector<size_t> elements(NELEMENTS);

                for (size_t i = 0; i < NELEMENTS; ++i) {

                    elements[i] = getRandElement(numberLen,base);
                }


                double skelTime = 0.0, skelMapReduceTime = 0.0, skelScanTime = 0.0, skelSeqTime = 0.0,
                       seqTime = 0.0, seqNonParTime = 0.0,
                       pthreadTime = 0.0;

                size_t r = 0;

                decltype(elements) skelElements, seqElements, pthreadsElements;

                while (r++ < NRUNS) {

                    /*
                     * Skeleton
                     */
                    std::vector<double> skelMapReduceTiming, skelScanTiming, skelSeqTiming;
                    tstart = second();
                        skelElements = RadixSortSkel(elements, numberLen, base, skelMapReduceTiming, skelScanTiming, skelSeqTiming);
                    tstop = second();
                    skelTime += tstop - tstart;

                    for (auto &t : skelMapReduceTiming)
                        skelMapReduceTime += t;
                    skelMapReduceTiming.clear();

                    for (auto &t : skelScanTiming)
                        skelScanTime += t;
                    skelScanTiming.clear();

                    for (auto &t : skelSeqTiming)
                        skelSeqTime += t;
                    skelSeqTiming.clear();


                    /*
                     * Sequential
                     */
                    std::vector<double> nonParallelTimings;

                    tstart = second();
                        seqElements = RadixSortSeq(elements, numberLen, base, nonParallelTimings);
                    tstop = second();
                    seqTime += tstop - tstart;

                    for (auto &t : nonParallelTimings)
                        seqNonParTime += t;
                    nonParallelTimings.clear();

                    /*
                     * Pthreads
                     */
                    std::vector<double> timings;
                    tstart = second();
                        pthreadsElements = RadixSortPThreads(elements,timings);
                    tstop = second();
                    pthreadTime += tstop - tstart;

                    /*
                     * Validate results
                     */
                    assert(seqElements.size() == skelElements.size());
                    for (size_t i = 0; i < seqElements.size(); ++i) {
                        assert(seqElements[i] == skelElements[i]);
                    }
//
//                    assert(seqElements.size() == pthreadsElements.size());
//                    for (size_t i = 0; i < seqElements.size(); ++i) {
//                        assert(seqElements[i] == pthreadsElements[i]);
//                    }
                }

                printMessage("Results validated");

                std::cout << "Sequential Time:\t" << seqTime / NRUNS << std::endl;
                std::cout << "Seq. Non-Parallel Time:\t" << seqNonParTime / NRUNS << std::endl;
                std::cout << "Skeleton Time:\t" << skelTime / NRUNS << std::endl;
                std::cout << "skelMapReduce Time:\t" << skelMapReduceTime / NRUNS << std::endl;
                std::cout << "skelScan Time:\t" << skelScanTime / NRUNS << std::endl;
                std::cout << "skelSeq Time:\t" << skelSeqTime / NRUNS << std::endl;
                std::cout << "PThread Time:\t" << pthreadTime / NRUNS << std::endl;
                std::cout << "Seq Gain:\t" << seqTime / skelTime << std::endl;
                std::cout << "PThread Gain:\t" << pthreadTime / skelTime << std::endl;
#if defined(TIMING)
            }
        }
#endif

#if !defined(TIMING)
//        std::cout  << "Define timing (-DTIMING)" << std::endl;

#endif

        return 0;
    }
}