


#include <cstdlib>
#include <iostream>
#include <thread>
#include <string>


#include "testMap.hpp"
#include "testReduce.hpp"
#include "testMapReduce.hpp"
#include "testScan.hpp"

#include "problemKmeans.hpp"
#include "problemKmeans_MR.hpp"

#include "problemRadixsort.hpp"

#include "problemSummedArea.hpp"

#include "def_file.hpp"
#include "miscFun.hpp"

#include "MapReduce.hpp"

#if defined(MAP) || defined(REDUCE) || defined(MAPREDUCE) || defined(SCAN)

    void printHeaderForBenchmark(std::string skeleton) {

        std::cout << "Benchmarking " << skeleton << " Skeleton" << std::endl;
        std::cout << "########################" << std::endl;

        std::cout << "Number of Runs (for AVG):\t" << NRUNS << std::endl;
        std::cout << "Hardware Concurrency:\t\t" << std::thread::hardware_concurrency() << std::endl;
        std::cout << "Max Threads (last Run):\t\t" << MTHREADS << std::endl;
        std::cout << "Number of Items per Run:\t" << NITEMS << std::endl;
        std::cout << "Collatz seed:\t\t\t" << COLLATZ << std::endl;
#if defined(UNBALANCED)
        std::cout << "Balanced :\t\t\tNO"<< std::endl;
#else
        std::cout << "Balanced :\t\t\tYES"<< std::endl;
#endif
        std::cout << "########################" << std::endl;
    }

#endif


int main(int argc, char** argv) {

#ifdef MAP
    printHeaderForBenchmark("MAP");
    MapMain( argc, argv );
#endif

#ifdef REDUCE
    printHeaderForBenchmark("REDUCE");
    ReduceMain( argc, argv );
#endif

#ifdef SCAN
    printHeaderForBenchmark("SCAN");
    ScanMain( argc, argv );
#endif

#ifdef MAPREDUCE
    printHeaderForBenchmark("MAPREDUCE");
    MapReduceMain( argc, argv );
#endif

#ifdef KMEANS
    KMeans::KMeansMain( argc, argv );
#endif

#ifdef KMEANS_MR
    std::cout << "K-Means problem with MapReduce Skeleton" << std::endl;
    KMeans_MapReduce::KMeansMapReduceMain( argc, argv );
#endif

#ifdef RADIX_SORT
    std::cout << "Radix-Sort problem with MapReduce and Scan Skeletons" << std::endl;
    RadixSort::RadixSortMain( argc, argv );
#endif

#ifdef SUMMEDAREA
    std::cout << "Summed-Area problem with Map and Scan Skeletons" << std::endl;
    SummedArea::SummedAreaMain( argc, argv );
#endif

    return 0;
}

