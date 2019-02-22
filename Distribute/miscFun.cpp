/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <sys/time.h>
#include <iostream>

#include "miscFun.hpp"


double tstart,tstop, skelTime[NRUNS], seqTime[NRUNS];


void justAMoment(size_t n) {

    if( n < 1 ) return;

    while(n!=1) {

        if(n%2) n = 3*n + 1;
        else n = n / 2;
    }
}

double second()
{
/* struct timeval { long        tv_sec; 
            long        tv_usec;        };

struct timezone { int   tz_minuteswest;
             int        tz_dsttime;      };     */

        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

void printResults(size_t threads, size_t RUNS, double* seqTime, double seqAVG, double* skelTime, double skelAVG, double gain) {
    
    std::cout << "----------------" << std::endl;
    std::cout << "Number of threads:\t\t" << threads << std::endl;
    for(size_t r = 0; r < RUNS; ++r) std::cout << "Sequential Run " << r+1 << " :\t" << seqTime[r] << std::endl;
    std::cout << "Sequential took (avg sec):\t" << seqAVG << std::endl;
    for(size_t r = 0; r < RUNS; ++r) std::cout << "Skeleton Run " << r+1 << " :\t" << skelTime[r] << std::endl;
    std::cout << "Skeleton took (avg sec):\t" << skelAVG << std::endl;
    std::cout << "Gain:\t\t\t\t" << gain << std::endl;
    
}

void printMessage(std::string s) {

    std::cout << s << std::endl;
}


int iRand(int iMin, int iMax)
{
    double i = (double)rand() / RAND_MAX;
    return (int)(iMin + i * (iMax - iMin));
}

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

