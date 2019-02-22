// Stripped out all the padding stuff from original
// and converted from set-of-arrays to vector-of-structs
// for the data, presumably this was also originally an optimization?

// TODO: check the results are correct
// Re-insert the internal repetition option to beef up run-time without massive data
// Do this in a way which will not be optimsed away!

#define fptype float

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cassert>
#include <sys/time.h>
#include "../../Map.hpp"



typedef struct OptionData_ {
        fptype s;          // spot price
        fptype strike;     // strike price
        fptype r;          // risk-free interest rate
        fptype divq;       // dividend rate
        fptype v;          // volatility
        fptype t;          // time to maturity or option expiration in years 
                           //     (1yr = 1.0, 6mos = 0.5, 3mos = 0.25, ..., etc)  
        const char *OptionType;  // Option type.  "P"=PUT, "C"=CALL
        fptype divs;       // dividend vals (not used in this test)
        fptype DGrefval;   // DerivaGem Reference Value
} OptionData;

//++  modified by xiongww

OptionData data_init[] = {
    #include "optionData.txt"
};


int    * otype;
fptype * sptprice;
fptype * strike;
fptype * rate;
fptype * volatility;
fptype * otime;

double second() {
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

#define inv_sqrt_2xPI 0.39894228040143270286

fptype CNDF ( fptype InputX )
{
    int sign;

    fptype OutputX;
    fptype xInput;
    fptype xNPrimeofX;
    fptype expValues;
    fptype xK2;
    fptype xK2_2, xK2_3;
    fptype xK2_4, xK2_5;
    fptype xLocal, xLocal_1;
    fptype xLocal_2, xLocal_3;

    // Check for negative value of InputX
    if (InputX < 0.0) {
        InputX = -InputX;
        sign = 1;
    } else 
        sign = 0;

    xInput = InputX;
 
    // Compute NPrimeX term common to both four & six decimal accuracy calcs
    expValues = exp(-0.5f * InputX * InputX);
    xNPrimeofX = expValues;
    xNPrimeofX = xNPrimeofX * inv_sqrt_2xPI;

    xK2 = 0.2316419 * xInput;
    xK2 = 1.0 + xK2;
    xK2 = 1.0 / xK2;
    xK2_2 = xK2 * xK2;
    xK2_3 = xK2_2 * xK2;
    xK2_4 = xK2_3 * xK2;
    xK2_5 = xK2_4 * xK2;
    
    xLocal_1 = xK2 * 0.319381530;
    xLocal_2 = xK2_2 * (-0.356563782);
    xLocal_3 = xK2_3 * 1.781477937;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_4 * (-1.821255978);
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_5 * 1.330274429;
    xLocal_2 = xLocal_2 + xLocal_3;

    xLocal_1 = xLocal_2 + xLocal_1;
    xLocal   = xLocal_1 * xNPrimeofX;
    xLocal   = 1.0 - xLocal;

    OutputX  = xLocal;
    
    if (sign) {
        OutputX = 1.0 - OutputX;
    }
    
    return OutputX;
} 

// For debugging
void print_xmm(fptype in, char* s) {
    printf("%s: %f\n", s, in);
}


//////////////////////////////////////////////////////////////////////////////////////
fptype BlkSchlsEqEuroNoDiv( OptionData option)

//fptype sptprice,
//fptype strike, fptype rate, fptype volatility,
//fptype time, int otype)    
{
    fptype OptionPrice;

    // local private working variables for the calculation
    fptype xStockPrice;
    fptype xStrikePrice;
    fptype xRiskFreeRate;
    fptype xVolatility;
    fptype xTime;
    fptype xSqrtTime;

    fptype logValues;
    fptype xLogTerm;
    fptype xD1; 
    fptype xD2;
    fptype xPowerTerm;
    fptype xDen;
    fptype d1;
    fptype d2;
    fptype FutureValueX;
    fptype NofXd1;
    fptype NofXd2;
    fptype NegNofXd1;
    fptype NegNofXd2;    
    
    xStockPrice   = option.s;
    xStrikePrice  = option.strike;
    xRiskFreeRate = option.r;
    xVolatility   = option.v;
    xTime         = option.t;
    xSqrtTime     = sqrt(xTime);

    logValues     = log( xStockPrice / xStrikePrice );
        
    xLogTerm      = logValues;
        
    
    xPowerTerm = xVolatility * xVolatility;
    xPowerTerm = xPowerTerm * 0.5;
        
    xD1 = xRiskFreeRate + xPowerTerm;
    xD1 = xD1 * xTime;
    xD1 = xD1 + xLogTerm;

    xDen = xVolatility * xSqrtTime;
    xD1 = xD1 / xDen;
    xD2 = xD1 -  xDen;

    d1 = xD1;
    d2 = xD2;
    
    NofXd1 = CNDF( d1 );
    NofXd2 = CNDF( d2 );

    FutureValueX = xStrikePrice * ( exp( -(xRiskFreeRate)*(xTime) ) );        
    if (otype == 0) {            
        OptionPrice = (xStockPrice * NofXd1) - (FutureValueX * NofXd2);
    } else { 
        NegNofXd1 = (1.0 - NofXd1);
        NegNofXd2 = (1.0 - NofXd2);
        OptionPrice = (FutureValueX * NegNofXd2) - (xStockPrice * NegNofXd1);
    }
    return OptionPrice;
}

int main(int argc, char *argv[])
{
    int i;
    int loopnum;
    fptype * buffer;
    int * buffer2;
    int initOptionNum;
    double tstart, tstop;

    const size_t threads     = NTHREADS;
    const size_t noptions    = NOPTIONS;


    tstart = second();

    std::vector<OptionData> data(noptions);
    std::vector<fptype>     prices(noptions);

    initOptionNum =  ( (sizeof(data_init)) / sizeof(OptionData) );
    for ( loopnum = 0; loopnum < noptions; ++ loopnum )   
    {
	OptionData *temp = data_init + loopnum%initOptionNum;
	data[loopnum].OptionType = (const char*)malloc(strlen(temp->OptionType)+1);
        strcpy((char *)(data[loopnum].OptionType), temp->OptionType);
	data[loopnum].s = temp->s;
	data[loopnum].strike = temp->strike;
	data[loopnum].r = temp->r;
	data[loopnum].divq = temp->divq;
	data[loopnum].v = temp->v;
	data[loopnum].t = temp->t;
	data[loopnum].divs = temp->divs;
	data[loopnum].DGrefval = temp->DGrefval;
    }

    //printf("Num of Options: %d\n", noptions);
    //printf("Size of data: %d\n", noptions * (sizeof(OptionData) + sizeof(int)));

    auto blackscholes_map = Map(BlkSchlsEqEuroNoDiv, threads);
 
    blackscholes_map(prices, data);

    tstop = second();

    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NOPTIONS  << std::endl;

    // Remove this line during timing tests
    //std::cout << "the result is  " << result << std::endl;

    return 0;
}

