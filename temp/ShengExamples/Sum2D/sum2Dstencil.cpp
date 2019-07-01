// Simple test for the 2D stencil skeleton
// Assumes wrapround boundaries
// The stencil kernel function simply returns the sum of the neighbourhood values (easy to check).

// add -DOUTPUT to write input and output to file if required for checking
// [fantine]mic: g++ sum2Dstencil.cpp -std=c++11 -lpthread -O2 -DNTHREADS=4 -DNROWS=10 -DNCOLS=10 -DNDATABLOCKS=2 -DOUTPUT -o sum2Dstencil

// [fantine]mic: ./sum2Dstencil

// [fantine]mic: more sum2Dstenciltest.txt 
// Input: 

// Output: 


// The above is correct because each result is the sum of its 9 element neighbourhood (with wrapraound)

// To run the script which repeats the experiment, across a number of threads, and gathers stats
//  [fantine]mic: ./sum2DstencilScript 4 20 5 results.csv 100 100

// To process and view the timing data 
//  [fantine]mic: Rscript Rtest.R results.csv results.pdf
//  [fantine]mic: okular results.pdf


#include <cassert>
#include <sys/time.h>

#include "../../Stencil2D.hpp"

double second() {

        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

FILE *outfile;


// We would really like to be able to declare the neighbourhood parameter
// as a 2D array, but this creates problems with the library code: the
// library has to know statically how big the neighbourhood will be,
// and so if want flexible neighbourhood size we have to stick with
// a 1D version and just used careful array indexing to "pretend"
// this is really 2D

int sumkernel2D (int neighbourhood[], int width) {
  int sum=0;

  for (size_t i = 0; i < (width*2 + 1); i++) {
     for (size_t j = 0; j < (width*2 + 1); j++) {

       // what we really mean here is neighbourhood[i][j]
       sum += neighbourhood[i*(width*2+1)+ j];
     }
  }

  return sum;
}

int main(int argc, char** argv) {
    FILE *outfile;
    size_t t = NTHREADS;
    double tstart, tstop;

    tstart = second();

    std::vector<int> in(NROWS*NCOLS);
    for(size_t i = 0; i < NROWS*NCOLS; ++i){
        in[i] = i%3;
    }

    std::vector<int> results (in.size());

    auto mystencil = Stencil2D(sumkernel2D, 1, NROWS, NCOLS, t);
    mystencil(results,in);

    #ifdef OUTPUT
    // Output results
    outfile = fopen("sum2Dstenciltest.txt","w");
    fprintf(outfile,"Input: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile," %d", in[i]);
    }

    fprintf(outfile,"\nOutput: ");
    for (size_t i = 0; i<NROWS*NCOLS; i++) {
      if (i%NCOLS == 0) fprintf(outfile," \n");
      fprintf(outfile," %d", results[i]);
    }
    #endif
   
    tstop = second();

    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NROWS << 'x' << NCOLS <<  std::endl;

 return 0;
}
 
