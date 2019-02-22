//
// Created by tonio on 24/06/18.
//


#include <cassert>

#include <list>

#include <map>

#include <string>

#include "testMapReduce.hpp"


#include "def_file.hpp"
#include "miscFun.hpp"

#include "MapReduce.hpp"



std::list<std::pair<int,double>> MRelemental(int input) {

    std::list<std::pair<int,double>> res;
    int i = 0;

#ifdef UNBALANCED
    if( input > NITEMS/2 ) {
        justAMoment();
        while(i++ < NITEMS/2)
            res.push_back(std::make_pair(i, (double) input));
    }
#else
    justAMoment();
    while(i++ < NITEMS)
        res.push_back(std::make_pair(i, (double) input));
#endif
    return res;

}

std::list<double> MRcombiner(int key, std::list<double> values) {

    std::list<double> res;
    double bucket = 0.0;

#ifdef UNBALANCED
    if( key > NITEMS/4 ){
        justAMoment();
        for( auto &v : values )
            bucket += v;
    }
#else
    justAMoment();
    for( auto &v : values )
        bucket += v;
#endif
    res.push_back( bucket );
    return res;
}

bool strCmp(std::pair<int, std::vector<double>> p1, std::pair<int, std::vector<double>> p2) {

    return p1.first < p2.first;
}

int MapReduceMain(int argc, char** argv) {


    size_t RUNS = NRUNS;
    size_t r = 0;

    size_t T = MTHREADS;
    size_t t = 1;

    double seqSum = 0.0, skelSum = 0.0;

    std::vector<int> in(NITEMS);
    for(int i=1;i<=NITEMS;++i)
        in[i-1] = i;

    //
    //
    // Sequential
    //
    //
    std::map<int,std::list<double>> tempRes;
    std::vector<std::pair<int,std::vector<double>>> seqRes;
    while(r++<RUNS){

        tempRes.clear();
        seqRes.clear();

        tstart = second();
        for( auto &i : in ) {
            auto l = MRelemental(i);
            for( auto &p : l) {
                tempRes[ p.first ].push_back( p.second );
            }
        }
        for( auto &p : tempRes ) {

            auto o = MRcombiner(p.first, p.second);

//                for(auto &rI : p.second ) delete rI;

            seqRes.push_back( {p.first, std::vector<double>{o.begin(),o.end()}} );
        }
        tstop = second();
        seqTime[r-1] = tstop-tstart;
        seqSum += seqTime[r-1];

//        if( r < RUNS ){
//            for(auto &sR : seqRes){
//                delete sR.second[0];
//            }
//        }
    }
    double seqAVG = seqSum / RUNS;

    //
    //
    // Skeleton
    //
    //
    std::vector<std::pair<int,std::vector<double>>> skelRes;
    r = 0;
    while(t<=T) {
        while(r++<RUNS){

            printMessage("Run: "+std::to_string(r));

            auto mapReduce = MapReduce( MRelemental, MRcombiner, [](int k){ return std::hash<int>{}(k); },t );

            tstart = second();
            mapReduce(skelRes,in);
            tstop = second();
            skelTime[r-1] = tstop-tstart;
            skelSum += skelTime[r-1];

            assert( skelRes.size() == seqRes.size() );
            sort(skelRes.begin(), skelRes.end(), strCmp);
            sort(seqRes.begin(), seqRes.end(), strCmp);
            for( size_t p = 0; p < skelRes.size(); ++p  ){
                assert(skelRes[p].first==seqRes[p].first);
                assert(skelRes[p].second.size()==seqRes[p].second.size());
                for( size_t v = 0; v < skelRes[p].second.size(); ++v  ) {
                    assert( skelRes[p].second[v] == seqRes[p].second[v]  );
//                    delete skelRes[p].second[v];
                }
            }
        }

        double skelAVG = skelSum / RUNS;
        skelSum = 0.0;

        double gain = seqAVG / skelAVG;

        printResults(t,RUNS,seqTime,seqAVG,skelTime,skelAVG,gain);

        r = 0;
        //t *= 2;
        t += 1;
    }

//    for(auto &sR : seqRes){
//        delete sR.second[0];
//    }

    return 0;
} /**/
