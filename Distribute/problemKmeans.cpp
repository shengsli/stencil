/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   problemKMeans.cpp
 * Author: tonio
 *
 * Created on June 8, 2018, 4:36 PM
 */

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

#include "problemKmeans.hpp"


namespace KMeans {

    typedef std::pair<double, double> COORD;

    struct KMPoint {

        bool hasChanged;
        COORD coord;
        COORD cluster;

        bool operator==(const KMPoint &p) {

            bool b = (std::abs(coord.first - p.coord.first) < EPSILON &&
                      std::abs(coord.second - p.coord.second) < EPSILON)
                     && (std::abs(cluster.first - p.cluster.first) < EPSILON &&
                         std::abs(cluster.second - p.cluster.second) < EPSILON)
                     && (hasChanged == p.hasChanged);
            return b;
        }

        void print() {
            std::cout.precision(17);
            std::cout << "{" << coord.first << "," << coord.second << "}" << std::endl;
        }
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

    KMPoint *elemental(KMPoint *point, std::vector<KMPoint> &centroids) {

        COORD closestCentroid = centroids[0].cluster;
        double closestDistance = distanceOf(point->coord, closestCentroid);


        for (auto c : centroids) {

            double distance = distanceOf(point->coord, c.coord);

            if (closestDistance > distance) {

                closestCentroid = c.coord;
                closestDistance = distance;

            }
        }

        if (distanceOf(point->cluster, closestCentroid)) {

            point->cluster = closestCentroid;
            point->hasChanged = true;
        } else {
            point->hasChanged = false;
        }


        return point;
    }


    template<typename R>
    KMPoint mapToNewCentroidElemental(std::vector<KMPoint *> *pointsInCentroid, R &reduce) {

        KMPoint newCentroid;

        if (!pointsInCentroid->empty()) {

            KMPoint *nCp;

            reduce(nCp, *pointsInCentroid);

            nCp->coord.first /= pointsInCentroid->size();
            nCp->coord.second /= pointsInCentroid->size();
            nCp->cluster = nCp->coord;

            newCentroid = *nCp;
        } else {

            newCentroid.coord.first = fRand();
            newCentroid.coord.second = fRand();
            newCentroid.cluster = newCentroid.coord;
        }

        return newCentroid;

    }

    KMPoint *combiner(KMPoint *p1, KMPoint *p2) {

        KMPoint *point = new KMPoint;

        point->coord = std::make_pair((p1->coord.first + p2->coord.first), (p1->coord.second + p2->coord.second));

        return point;
    }

    void printKMPoint(KMPoint point) {

        std::cout << "(" << point.coord.first << "," << point.coord.second << ")";
        std::cout << "->(" << point.cluster.first << "," << point.cluster.second << ")" << std::endl;
    }

    std::vector<KMPoint> KMeansSkel(std::vector<KMPoint> points, std::vector<KMPoint> centroids,
                                    std::vector<double> &mapTimings, std::vector<double> &reduceTimings, std::vector<double> &skelSeqTiming) {

        auto map = Map(elemental, NTHREADS);
        auto reduce = Reduce(combiner, NTHREADS);

        using R = decltype(reduce);
        auto mapToNewCentroid = Map(mapToNewCentroidElemental<R>, 1);

        std::vector<KMPoint *> pointsPtrs(points.size());
        std::transform(points.begin(), points.end(), pointsPtrs.begin(), [](KMPoint &p) { return &p; });

        std::map<COORD, std::vector<KMPoint *>> centroidMap;
        std::vector<std::vector<KMPoint *> *> centroidVectorPair;

        size_t ncentroids = centroids.size();

        bool hasChanged = true;

        while (hasChanged) {

            /*
             * For each point, find closest centroid
             */
            auto start = second();
            map(pointsPtrs, pointsPtrs, centroids);
            auto stop = second();
            mapTimings.push_back(stop - start);

            start = second();
            centroids.clear();
            centroids.resize(ncentroids);

            /*
             * Gather pointsPtrs per centroid
             */

            centroidVectorPair.clear();
            centroidMap.clear();
            for (auto &p : pointsPtrs) {

                centroidMap[p->cluster].push_back(p);
            }

            for (auto &c : centroidMap) {
                centroidVectorPair.push_back(&c.second);
            }

            /*
             * get boolean, if at least on point has changed centroid
             */
            hasChanged = false;

            for (auto &p : pointsPtrs) {

                hasChanged |= p->hasChanged;
            }
            //            mapChanged(pointsHaveChanged,pointsPtrs);        // Seq: 0.702935 sec.            | For the whole run
            //            convergeReduce(hasChanged,pointsHaveChanged);    // and Skel(2): 2.384789 sec.    | For 10^6 points
            stop = second();
            skelSeqTiming.push_back(stop - start);

            /*
             * Find new centroids
             */
            if (hasChanged) {
                auto start = second();
                    mapToNewCentroid(centroids, centroidVectorPair, reduce);
                auto stop = second();
                reduceTimings.push_back(stop - start);
            }


        }

        return points;
    }

    void seqFindClosestCentroid(std::vector<KMPoint *> &points, std::vector<KMPoint> &centroids) {

        for (auto &point : points) {

            COORD closestCentroid = centroids[0].cluster;
            double closestDistance = distanceOf(point->coord, closestCentroid);


            for (auto &c : centroids) {

                double distance = distanceOf(point->coord, c.coord);

                if (closestDistance > distance) {

                    closestCentroid = c.coord;
                    closestDistance = distance;

                }
            }

            if (distanceOf(point->cluster, closestCentroid)) {

                point->cluster = closestCentroid;
                point->hasChanged = true;
            } else {

                point->hasChanged = false;
            }
        }
    }

    void seqFindNewCentroids(KMPoint &newCentroid, std::vector<KMPoint *> &points) {

        newCentroid.coord.first = 0.0;
        newCentroid.coord.second = 0.0;

        for (auto p : points) {

            newCentroid.coord.first += p->coord.first;
            newCentroid.coord.second += p->coord.second;
        }

        newCentroid.coord.first /= points.size();
        newCentroid.coord.second /= points.size();
        newCentroid.cluster = newCentroid.coord;
    }

    std::vector<KMPoint> KMeansSeq(std::vector<KMPoint> points, std::vector<KMPoint> centroids, std::vector<double> &nonParallelTimings) {

        std::map<COORD, std::vector<KMPoint *>> centroidMap;

        std::vector<KMPoint *> pointsPtrs(points.size());
        std::transform(points.begin(), points.end(), pointsPtrs.begin(), [](KMPoint &p) { return &p; });

        bool hasChanged = true;

        while (hasChanged) {

            /*
             * For each point, find closest centroid
             */
            for (auto &p : pointsPtrs) {
                p = elemental(p, centroids);
            }

            /*
             * Gather pointsPtrs per centroid
             */
            auto tstart = second();
            centroids.clear();
            centroidMap.clear();

            for (auto &p : pointsPtrs) {

                centroidMap[p->cluster].push_back(p);
            }

            /*
             * get boolean, if at least on point has changed centroid
             */
            hasChanged = false;

            for (auto &p : pointsPtrs) {

                hasChanged |= p->hasChanged;
            }
            auto tstop = second();
            nonParallelTimings.push_back(tstop - tstart);
            /*
             * Find new centroids
             */
            if (hasChanged) {
                /*
                 * This is equivalent to mapToNewCentroid in Skeleton implementation
                 */
                for (auto &c : centroidMap) {

                    KMPoint newCentroid;

                    if (c.second.empty()) {

                        newCentroid.coord.first = fRand();
                        newCentroid.coord.second = fRand();
                        newCentroid.cluster = newCentroid.coord;

                    } else {
                        seqFindNewCentroids(newCentroid, c.second);
                    }

                    centroids.push_back(newCentroid);
                }
            }
        }

        return points;
    }

    class ThreadArgClosestCentroid {
        public:
            std::vector<KMPoint*> *input;
            std::vector<KMPoint>  *centroids;
            size_t *chunkIndices;
    };

    void pthreadClosestCentroid(ThreadArgClosestCentroid *threadArguments, size_t threadID) {

        size_t start = threadArguments[threadID].chunkIndices[threadID];
        size_t end = threadArguments[threadID].chunkIndices[threadID + 1];

        auto &input = threadArguments[threadID].input;
        auto centroids = threadArguments[threadID].centroids;

        for (size_t inputIndex = start; inputIndex < end; ++inputIndex) {

            (*input)[ inputIndex ] = elemental( (*input)[ inputIndex ], *centroids );
        }
    }

    class ThreadArgNewCentroid {
        public:
            size_t *chunkIndices;
            std::vector<KMPoint *> *input;
            KMPoint centroid;
    };

    std::mutex pthreadBarrierLock;
    std::condition_variable pthreadCond_Var;
    size_t pthreadThreadsArrived;
    void pthreadNewCentroidBarrier(size_t nThreadsToBarrier) {

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

    void pthreadNewCentroid( std::vector<ThreadArgNewCentroid> *threadArguments, size_t threadID ) {

        size_t start = threadArguments->operator[](threadID).chunkIndices[threadID];
        size_t end = threadArguments->operator[](threadID).chunkIndices[threadID + 1];

        auto input = threadArguments->operator[](threadID).input;

        KMPoint chunkSum;
        chunkSum.coord.first = 0;
        chunkSum.coord.second = 0;

        for (size_t inputIndex = start; inputIndex < end; ++inputIndex) {

            chunkSum.coord.first += (*input)[ inputIndex ]->coord.first;
            chunkSum.coord.second += (*input)[ inputIndex ]->coord.second;
        }

        threadArguments->operator[](threadID).centroid = chunkSum;

        pthreadNewCentroidBarrier( threadArguments->size() );

        if( threadID == 0 ) {

            auto &newCentroid = threadArguments->operator[](threadID).centroid;

            for( size_t t = 1; t < threadArguments->size(); ++t ) {
                newCentroid.coord.first += threadArguments->operator[](t).centroid.coord.first;
                newCentroid.coord.second += threadArguments->operator[](t).centroid.coord.second;
            }

            newCentroid.coord.first /= input->size();
            newCentroid.coord.second /= input->size();
        }
    }

    void pThreadsFindNewCentroid(KMPoint &newCentroid, std::vector<KMPoint *> &points) {

        size_t chunkSize, chunkStart = 0, chunkEnd;

        size_t nthreads = NTHREADS*2 > points.size() ? points.size() / 2 : NTHREADS;

        std::vector<ThreadArgNewCentroid> threadArguments(nthreads);
        std::thread *THREADS[nthreads];
        size_t chunkIndices[nthreads + 1];

        for (size_t t = 0; t < nthreads; ++t) {

            if (t < (points.size() % nthreads)) chunkSize = 1 + points.size() / nthreads;
            else chunkSize = points.size() / nthreads;

            chunkIndices[t] = chunkStart;
            chunkEnd = chunkStart + chunkSize;
            chunkStart = chunkEnd;

            threadArguments[t].chunkIndices = chunkIndices;
            threadArguments[t].input = &points;
        }
        chunkIndices[nthreads] = chunkEnd;

        pthreadThreadsArrived = 0;

        for (size_t t = 0; t < nthreads; ++t) {
            THREADS[t] = new std::thread(pthreadNewCentroid, &threadArguments, t);
        }

        for(size_t t=0; t < nthreads; ++t){
            THREADS[t]->join();
            delete THREADS[t];
        }

        newCentroid = threadArguments[0].centroid;
        newCentroid.cluster = newCentroid.coord;

    }

    std::vector<KMPoint> KMeansPThreads(std::vector<KMPoint> points, std::vector<KMPoint> centroids,
                                        std::vector<double> &nonParallelTimings) {

        std::map<COORD, std::vector<KMPoint *>> centroidMap;

        std::vector<KMPoint *> pointsPtrs(points.size());
        std::transform(points.begin(), points.end(), pointsPtrs.begin(), [](KMPoint &p) { return &p; });

        bool hasChanged = true;

        while (hasChanged) {

            /*
             * For each point, find closest centroid
             */

            ThreadArgClosestCentroid threadArguments[NTHREADS];
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
                threadArguments[t].input = &pointsPtrs;
                threadArguments[t].centroids = &centroids;
            }
            chunkIndices[NTHREADS] = chunkEnd;

            for (size_t t = 0; t < NTHREADS; ++t) {

                THREADS[t] = new std::thread(pthreadClosestCentroid, threadArguments, t);
            }

            for(size_t t=0; t < NTHREADS; ++t){
                THREADS[t]->join();
                delete THREADS[t];
            }

            /*
             * Gather pointsPtrs per centroid
             */
            auto tstart = second();
            centroids.clear();
            centroidMap.clear();

            for (auto &p : pointsPtrs) {

                centroidMap[p->cluster].push_back(p);
            }

            /*
             * get boolean, if at least on point has changed centroid
             */
            hasChanged = false;

            for (auto &p : pointsPtrs) {

                hasChanged |= p->hasChanged;
            }
            auto tstop = second();
            nonParallelTimings.push_back(tstop - tstart);

            /*
             * Find new centroids
             */
            if (hasChanged) {
                /*
                 * This is equivalent to mapToNewCentroid in Skeleton implementation
                 */
                for (auto &c : centroidMap) {

                    KMPoint newCentroid;

                    if (c.second.empty()) {

                        newCentroid.coord.first = fRand();
                        newCentroid.coord.second = fRand();
                        newCentroid.cluster = newCentroid.coord;

                    } else {
                        pThreadsFindNewCentroid(newCentroid, c.second);
                    }

                    centroids.push_back(newCentroid);
                }
            }
        }

        return points;
    }

    /*
     *
     */
    int KMeansMain(int argc, char **argv) {

        double tstart, tstop;

        size_t npoints = NPOINTS;
        size_t nclusters = NCLUSTERS;


        std::cout << "Problem K-Means" << std::endl;

#if defined(TIMING)
        for (nclusters = MIN_CLUSTERS; nclusters <= MAX_CLUSTERS; ++nclusters) {
            for (npoints = MIN_POINTS; npoints <= MAX_POINTS; npoints *= 10) {
#endif

                std::cout << "########################" << std::endl;
                std::cout << "K-Means problem with:\tMap and Reduce Skeletons" << std::endl;
                std::cout << "Number of 2-D Points:\t" << npoints << std::endl;
                std::cout << "Number of Clusters:\t" << nclusters << std::endl;
                std::cout << "Number of Threads:\t" << NTHREADS << std::endl;
                std::cout << "Hardware Concurrency:\t" << std::thread::hardware_concurrency() << std::endl;
                std::cout << "Number of Runs (for AVG):\t" << NRUNS << std::endl;
#if !defined(TIMING)
                std::cout << "########################" << std::endl;
#endif

                std::srand(SEED);

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


                double skelTime = 0.0, skelMapTime = 0.0, skelRedTime = 0.0, skelSeqTime = 0.0, seqTime = 0.0, seqNonParTime = 0.0, pthreadTime = 0.0;
                size_t r = 0;

                decltype(points) skelPoints, seqPoints, pthreadsPoints;

                while (r++ < NRUNS) {

                    /*
                     * Skeleton
                     */
                    std::vector<double> skelMapTiming, skelRedTiming, skelSeqTiming;
                    tstart = second();
                        skelPoints = KMeansSkel(points, centroids, skelMapTiming, skelRedTiming, skelSeqTiming);
                    tstop = second();
                    skelTime += tstop - tstart;

                    for (auto &t : skelMapTiming)
                        skelMapTime += t;
                    skelMapTiming.clear();

                    for (auto &t : skelRedTiming)
                        skelRedTime += t;
                    skelRedTiming.clear();

                    for (auto &t : skelSeqTiming)
                        skelSeqTime += t;
                    skelSeqTiming.clear();


                    /*
                     * Sequential
                     */
                    std::vector<double> nonParallelTimings;

                    tstart = second();
                        seqPoints = KMeansSeq(points, centroids, nonParallelTimings);
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
                        pthreadsPoints = KMeansPThreads(points,centroids,timings);
                    tstop = second();
                    pthreadTime += tstop - tstart;

                    /*
                     * Validate results
                     */
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
                std::cout << "skelMap Time:\t" << skelMapTime / NRUNS << std::endl;
                std::cout << "skelRed Time:\t" << skelRedTime / NRUNS << std::endl;
                std::cout << "skelSeq Time:\t" << skelSeqTime / NRUNS << std::endl;
                std::cout << "PThread Time:\t" << pthreadTime / NRUNS << std::endl;
                std::cout << "Seq Gain:\t" << seqTime / skelTime << std::endl;
                std::cout << "PThread Gain:\t" << pthreadTime / skelTime << std::endl;
            }
        }
#endif

#if !defined(TIMING)
        std::cout  << "-----Result-----" << std::endl;
        for(auto p : seqPoints) printKMPoint(p);
#endif

        return 0;
    }
}