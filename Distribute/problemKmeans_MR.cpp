//
// Created by tonio on 24/06/18.
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

#include "Map.hpp"
#include "Reduce.hpp"
#include "MapReduce.hpp"

#include "problemKmeans_MR.hpp"

namespace KMeans_MapReduce {

    typedef std::pair<double, double> COORD;

    class KMPoint {
        public:
            bool hasChanged;
            COORD coord;
            COORD cluster;

            bool operator==(const KMPoint &p) const {

                bool b = (std::abs(coord.first - p.coord.first) < EPSILON &&
                          std::abs(coord.second - p.coord.second) < EPSILON)
                         && (std::abs(cluster.first - p.cluster.first) < EPSILON &&
                             std::abs(cluster.second - p.cluster.second) < EPSILON);
                return b;
            }

            void print() {

                std::cout << "Coord  : {" << coord.first << "," << coord.second << "}" << std::endl;
                std::cout << "Cluster: {" << cluster.first << "," << cluster.second << "}" << std::endl;
            }

            struct KeyHasher
            {
                std::size_t operator()(const KMPoint& centroid) const
                {
                    return ( (std::hash<double>()(centroid.coord.first) ^ (std::hash<double>()(centroid.coord.second) << 1)) >> 1);
                }
            };
    };

    int iRand(int iMin, int iMax) {
        double i = (double) rand() / RAND_MAX;
        return (int) (iMin + i * (iMax - iMin));
    }

    double fRand(double fMin = 0.0, double fMax = 1.0) {
        double f = (double) rand() / RAND_MAX;
        return fMin + f * (fMax - fMin);
    }


    double distanceOf(COORD &p1, COORD &p2) {

        double a = (p1.first - p2.first);
        double b = (p1.second - p2.second);

        return std::sqrt(a * a + b * b);

    }

    void printKMPoint(KMPoint point) {

        std::cout << "(" << point.coord.first << "," << point.coord.second << ")";
        std::cout << "->(" << point.cluster.first << "," << point.cluster.second << ")" << std::endl;
    }

    std::list<std::pair<KMPoint, KMPoint>> elementalMR_GetClosestCentroid(KMPoint point, std::vector<KMPoint> &centroids) {

        KMPoint closestCentroid = centroids[0];
        double closestDistance = distanceOf(point.coord, closestCentroid.coord);


        for (auto &centroid : centroids) {

            auto centroidCoord = centroid.coord;
            double distance = distanceOf(point.coord, centroidCoord);

            if (closestDistance > distance) {

                closestCentroid = centroid;
                closestDistance = distance;

            }
        }

        if (distanceOf(point.cluster, closestCentroid.coord)) {

            point.cluster = closestCentroid.coord;
            point.hasChanged = true;
        } else {
            point.hasChanged = false;
        }


        return std::list<std::pair<KMPoint, KMPoint>>{std::make_pair(closestCentroid, point)};
    };

    std::list<KMPoint> combinerMR_GetNewCentroid(KMPoint &centroid, std::list<KMPoint> points, std::vector<KMPoint>) {

        COORD newCentroid;

        double sumX = 0.0;
        double sumY = 0.0;

        for (auto &point : points) {

            sumX += point.coord.first;
            sumY += point.coord.second;
        }

        sumX /= points.size();
        sumY /= points.size();

        newCentroid.first = sumX;
        newCentroid.second = sumY;

        if ( distanceOf( centroid.coord, newCentroid ) ) {

            for (auto &point : points) {

                point.cluster = newCentroid;
                point.hasChanged = true;
            }
        }

        return points;
    };

    auto hashFunction =
            [](KMPoint centroid) {
                return  (std::hash<double>()(centroid.coord.first) ^ (std::hash<double>()(centroid.coord.second) << 1)) >> 1;
            };

    std::vector<KMPoint> KMeansMR_Skel(std::vector<KMPoint> points, std::vector<KMPoint> centroids,
                                       std::vector<double> &mapReduceTimings, std::vector<double> &skelSeqTiming) {


        auto mapReduce = MapReduce(elementalMR_GetClosestCentroid, combinerMR_GetNewCentroid, hashFunction, NTHREADS);

        std::vector<std::pair<KMPoint, std::vector<KMPoint>>> clusters;

        bool hasChanged = true;
        while (hasChanged) {
            /*
             * For each point, find closest centroid
             * and then for each cluster find its new centroid
             */
            auto start = second();
                mapReduce(clusters, points, centroids);
            auto stop = second();
            mapReduceTimings.push_back(stop - start);


            /*
             * Sequential Code
             *
             * Update centroids vector, points vector
             * and get boolean were at least on point has changed centroid
             */

            start = second();
                hasChanged = false;

                for( size_t centroidIndex = 0, pointsIndex = 0; centroidIndex < clusters.size(); ++centroidIndex ) {

                    auto aPointInCluster = clusters[ centroidIndex ].second[0]; //carries the information of the new centroid

                    KMPoint newCentroid;

                    newCentroid.coord = aPointInCluster.cluster;
                    newCentroid.cluster = newCentroid.coord;
                    newCentroid.hasChanged = aPointInCluster.hasChanged;

                    centroids[ centroidIndex ] = newCentroid;

                    hasChanged |= newCentroid.hasChanged;

                    //update the points vector
                    auto &clusterPoints = clusters[ centroidIndex ].second;
                    for( size_t index = 0; index < clusterPoints.size(); ++index ) {

                        points[ pointsIndex++ ] = clusterPoints[ index ];
                    }
                }
            stop = second();
            skelSeqTiming.push_back(stop - start);
        }

        return points;
    }

    std::vector<KMPoint> KMeansMR_Seq(std::vector<KMPoint> points, std::vector<KMPoint> centroids,
                                      std::vector<double> &nonParallelTimings) {

        std::unordered_map<KMPoint, std::list<KMPoint>, KMPoint::KeyHasher> clusters;

        bool hasChanged = true;

        while (hasChanged) {
            /*
             * For each point, find closest centroid
             */
            clusters.clear();
            for( auto &point : points ){

                auto mapOutput = elementalMR_GetClosestCentroid( point, centroids );

                /*
                 * Gather pointsPtrs per centroid
                 */
                clusters[ mapOutput.front().first ].push_back( mapOutput.front().second );
            }


            /*
             * Find new centroids
             */

            for (auto &cluster : clusters) {

                auto centroid = cluster.first;
                auto points = cluster.second;

                cluster.second = combinerMR_GetNewCentroid( centroid, points, centroids /*need to pass 3rd arg*/ );
            }

            auto tstart = second();
                hasChanged = false;
                size_t centroidIndex = 0, pointsIndex= 0;
                for( auto &cluster : clusters ) {

                    auto aPointInCluster = cluster.second.front(); //carries the information of the new centroid

                    KMPoint newCentroid;

                    newCentroid.coord = aPointInCluster.cluster;
                    newCentroid.cluster = newCentroid.coord;
                    newCentroid.hasChanged = aPointInCluster.hasChanged;

                    centroids[ centroidIndex++ ] = newCentroid;

                    hasChanged |= newCentroid.hasChanged;

                    //update the points vector
                    auto &clusterPoints = cluster.second;
                    for( auto &point : clusterPoints ) {

                        points[ pointsIndex++ ] = point;
                    }
                }
            auto tstop = second();
            nonParallelTimings.push_back(tstop - tstart);
        }

        return points;
    }
    class ThreadArgPthread {
        public:

            ThreadArgPthread(){}

            std::vector<KMPoint*> *points;
            std::vector<KMPoint>  *centroids;
            std::unordered_map<size_t,std::pair< KMPoint,std::list<KMPoint>* > > threadHashTable;
            std::vector<size_t> keys;

            size_t *chunkIndices;
    };

    std::mutex pthreadBarrierLock;
    std::condition_variable pthreadCond_Var;
    size_t pthreadThreadsArrived;
    void pthreadClosestCentroidBarrier(size_t nThreadsToBarrier) {

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

    void pthreadFunction(std::vector<ThreadArgPthread> *threadArguments, size_t threadID) {

        size_t start = threadArguments->at(threadID).chunkIndices[threadID];
        size_t end = threadArguments->at(threadID).chunkIndices[threadID + 1];

        auto points = threadArguments->at(threadID).points;
        auto centroids = threadArguments->at(threadID).centroids;
        auto &threadHashTable = threadArguments->at(threadID).threadHashTable;

        for (size_t inputIndex = start; inputIndex < end; ++inputIndex) {

            auto mapOutput = elementalMR_GetClosestCentroid( *points->at( inputIndex ), *centroids );

            auto hashValue = hashFunction( mapOutput.front().first );

            if( threadHashTable.count( hashValue ) ) {
                threadHashTable.at(hashValue).second->push_back( mapOutput.front().second );
            }
            else {
                std::list<KMPoint>* keyList = new std::list<KMPoint>{ mapOutput.front().second };

                threadHashTable.insert( std::make_pair( hashValue, std::make_pair( mapOutput.front().first, keyList  ) ) );
            }
        }

        pthreadClosestCentroidBarrier( threadArguments->size() );

        if( threadID == 0 ) {

            for( size_t t = 1; t < threadArguments->size(); ++t) {

                for( auto &hashKey_Bpair : threadArguments->at( t ).threadHashTable ){

                    if( threadHashTable.count( hashKey_Bpair.first ) ){

                        auto &Apair = threadHashTable[ hashKey_Bpair.first ];

                        Apair.second->splice( Apair.second->end(), *hashKey_Bpair.second.second  );

                        delete hashKey_Bpair.second.second;
                    }
                    else {
                        threadHashTable[ hashKey_Bpair.first ] = hashKey_Bpair.second;
                    }
                }
            }

            size_t t = 0;
            for( auto key_value : threadHashTable ){

                threadArguments->operator[]( t ).keys.push_back( key_value.first );

                t = (t + 1) % threadArguments->size();
            }
        }

        pthreadClosestCentroidBarrier( threadArguments->size() );

        auto &keys = threadArguments->operator[]( threadID ).keys;

        for( auto key : keys) {

            auto &reducePair = threadArguments->operator[]( 0 ).threadHashTable[ key ];

            auto reduceRes = combinerMR_GetNewCentroid( reducePair.first, *reducePair.second, *centroids );

            *reducePair.second = reduceRes;
        }
    }

    std::vector<KMPoint> KMeansMR_PThreads(std::vector<KMPoint> points, std::vector<KMPoint> centroids,
                                           std::vector<double> &nonParallelTimings) {

        std::map<COORD, std::vector<KMPoint *>> centroidMap;

        std::vector<KMPoint *> pointsPtrs(points.size());
        std::transform(points.begin(), points.end(), pointsPtrs.begin(), [](KMPoint &p) { return &p; });

        bool hasChanged = true;

        while (hasChanged) {

            /*
             * For each point, find closest centroid
             */

            std::vector<ThreadArgPthread> threadArguments(NTHREADS);
            std::thread *THREADS[NTHREADS];

            size_t chunkIndices[NTHREADS + 1];

            size_t chunkSize, chunkStart = 0, chunkEnd;

            for (size_t t = 0; t < NTHREADS; ++t) {

                if (t < (pointsPtrs.size() % NTHREADS)) chunkSize = 1 + pointsPtrs.size() / NTHREADS;
                else chunkSize = pointsPtrs.size() / NTHREADS;

                chunkIndices[t] = chunkStart;
                chunkEnd = chunkStart + chunkSize;
                chunkStart = chunkEnd;

                threadArguments[t].chunkIndices = chunkIndices;
                threadArguments[t].points = &pointsPtrs;
                threadArguments[t].centroids = &centroids;

            }
            chunkIndices[NTHREADS] = chunkEnd;

            for (size_t t = 0; t < NTHREADS; ++t) {

                THREADS[t] = new std::thread(pthreadFunction, &threadArguments, t);
            }

            for(size_t t=0; t < NTHREADS; ++t){
                THREADS[t]->join();
                delete THREADS[t];
            }

            auto clusters = threadArguments[0].threadHashTable;

            hasChanged = false;
            size_t centroidIndex = 0;
            size_t pointsIndex = 0;
            for( auto cluster : clusters ) {

                auto aPointInCluster = cluster.second.second->front(); //carries the information of the new centroid

                KMPoint newCentroid;

                newCentroid.coord = aPointInCluster.cluster;
                newCentroid.cluster = newCentroid.coord;
                newCentroid.hasChanged = aPointInCluster.hasChanged;

                centroids[ centroidIndex++ ] = newCentroid;

                hasChanged |= newCentroid.hasChanged;

                //update the points vector
                auto clusterPoints = cluster.second.second;
                for( auto clusterPoint : *clusterPoints ) {

                    points[ pointsIndex++ ] = clusterPoint;
                }

                delete cluster.second.second;
            }
        }

        return points;
    }

    int KMeansMapReduceMain(int argc, char **argv) {

        std::srand(SEED);

        double tstart, tstop;

        size_t npoints = NPOINTS;
        size_t nclusters = NCLUSTERS;


        std::cout << "Problem K-Means(MapReduce)" << std::endl;

#if defined(TIMING)
        for(nclusters = MIN_CLUSTERS; nclusters <= MAX_CLUSTERS; ++nclusters) {
            for(npoints = MIN_POINTS; npoints <= MAX_POINTS; npoints *= 10) {
#endif

                std::cout << "########################" << std::endl;
                std::cout << "K-Means problem with:\tMapReduce Skeleton" << std::endl;
                std::cout << "Number of 2-D Points:\t" << npoints << std::endl;
                std::cout << "Number of Clusters:\t" << nclusters << std::endl;
                std::cout << "Number of Threads:\t" << NTHREADS << std::endl;
                std::cout << "Hardware Concurrency:\t" << std::thread::hardware_concurrency() << std::endl;
                std::cout << "Number of Runs (for AVG):\t" << NRUNS << std::endl;
        #if !defined(TIMING)
                std::cout << "########################" << std::endl;
        #endif

                std::vector<KMPoint> points;
                std::vector<KMPoint> centroids;

                for (size_t i = 0; i < nclusters; ++i) {

                    KMPoint centroid;

                    centroid.coord = std::make_pair(fRand(), fRand());
                    centroid.cluster = centroid.coord;
                    centroid.hasChanged = false;
                    centroids.push_back(centroid);
                }

                for (size_t i = 0; i < npoints; ++i) {

                    KMPoint point;

                    point.coord = std::make_pair(fRand(), fRand());
                    point.cluster = centroids[iRand(0, centroids.size())].coord;
                    point.hasChanged = false;

                    points.push_back(point);
                }


                double skelTime = 0.0, skelMapReduceTime = 0.0, skelSeqTime = 0.0, seqTime = 0.0, seqNonParTime = 0.0, pthreadTime = 0.0;
                size_t r = 0;

                decltype(points) skelPoints;

                while (r++ < NRUNS) {

                    /*
                     * Skeleton
                     */
                    std::vector<double> skelMapReduceTiming, skelSeqTiming;
                    tstart = second();
                        skelPoints = KMeansMR_Skel(points, centroids, skelMapReduceTiming, skelSeqTiming);
                    tstop = second();
                    skelTime += tstop - tstart;

                    for (auto &t : skelMapReduceTiming)
                        skelMapReduceTime += t;
                    skelMapReduceTiming.clear();

                    for (auto &t : skelSeqTiming)
                        skelSeqTime += t;
                    skelSeqTiming.clear();


                    /*
                     * Sequential
                     */
                    std::vector<double> nonParallelTimings;
                    tstart = second();
                        auto seqPoints = KMeansMR_Seq(points, centroids, nonParallelTimings);
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
                        auto pthreadsPoints = KMeansMR_PThreads(points, centroids, timings);
                    tstop = second();
                    pthreadTime += tstop - tstart;



                    /*
                     * Validate results
                     */
                    auto sortPoint = [](KMPoint a, KMPoint b)
                    {
                        if( a.coord.first < b.coord.first ) return true;

                        if( (a.coord.first == b.coord.first) && (a.coord.second < b.coord.second )) return true;

                        return false;
                    };

                    std::sort( seqPoints.begin(), seqPoints.end(), sortPoint);
                    std::sort( skelPoints.begin(), skelPoints.end(), sortPoint);
                    std::sort( pthreadsPoints.begin(), pthreadsPoints.end(), sortPoint);

                    assert(seqPoints.size() == skelPoints.size());
                    for (size_t i = 0; i < seqPoints.size(); ++i) {
                        assert(seqPoints[i] == skelPoints[i]);
                    }

                    assert(seqPoints.size() == pthreadsPoints.size());
                    for (size_t i = 0; i < seqPoints.size(); ++i) {
                        assert(seqPoints[i] == pthreadsPoints[i]);
                    }
                }

                printMessage("Results validated");
#if defined(TIMING)
                std::cout << "Sequential Time:\t" << seqTime / NRUNS << std::endl;
                std::cout << "Seq. Non-Parallel Time:\t" << seqNonParTime / NRUNS << std::endl;
                std::cout << "Skeleton Time:\t" << skelTime / NRUNS << std::endl;
                std::cout << "skelSeq Time:\t" << skelSeqTime / NRUNS << std::endl;
                std::cout << "PThread Time:\t" << pthreadTime / NRUNS << std::endl;
                std::cout << "Seq Gain:\t" << seqTime / skelTime << std::endl;
                std::cout << "PThread Gain:\t" << pthreadTime / skelTime << std::endl;

            }
        }
#endif

#if !defined(TIMING)
        std::cout << "-----Result-----" << std::endl;
        for (auto p : skelPoints) printKMPoint(p);
#endif

        return 0;
    }

}