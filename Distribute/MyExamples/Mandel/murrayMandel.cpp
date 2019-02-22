#include <cassert>
#include <sys/time.h>


#include "../../Map.hpp"


double second() {

        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}


// g++ murrayMandel.cpp -std=c++11 -lpthread -O2 -DNTHREADS=32 -DHXRES=1024 -DHYRES=1024 -DITERMAX=10000 -DNDATABLOCKS=10000 -o murrayMandel

// ./murrayMandelScript 4 20 5 results.csv 1024 1024 1000


//#define hxres  2048           // horizonal resolution 
//#define hyres  2048           // vertical resolution  
//#define itermax 10000         // how many iterations to do 
 
typedef struct {
  int r;
  int g;
  int b;
} pixel_t;

FILE *outfile;

pixel_t mandel (int taskid) {

  pixel_t pixel;
  double x,xx,y,cx,cy;
  int iteration,hx,hy, i;
  double magnify=1.0;             /* no magnification */
 
  hx = taskid%HXRES;        
  hy = taskid/HYRES;        
  cx = (((float)hx)/((float)HXRES)-0.5)/magnify*3.0-0.7;
  cy = (((float)hy)/((float)HYRES)-0.5)/magnify*3.0;
  x = 0.0; y = 0.0;
  for (iteration=1;iteration<ITERMAX;iteration++)  {
    xx = x*x-y*y+cx;
    y = 2.0*x*y+cy;
    x = xx;
    if (x*x+y*y>100.0)  iteration = ITERMAX+1;
  }

  if (iteration<=ITERMAX) {
    pixel.r = 0; pixel.g = 255; pixel.b = 255;
  } else {
    pixel.r = 180; pixel.g = 0; pixel.b = 0;
  }
  return pixel;
}


int main(int argc, char** argv) {
    FILE *outfile;
    
    size_t t = NTHREADS;

    double tstart, tstop;

    tstart = second();

    std::vector<int> in(HXRES*HYRES);

    for(size_t i = 0; i < HXRES*HYRES; ++i){
        in[i] = i;
    }

    std::vector<pixel_t> image (in.size());

    auto map = Map(mandel,t);
    map(image,in);

    #ifdef OUTPUT
    // Output results
    outfile = fopen("custompicture.ppm","w");
    fprintf(outfile,"P6\n# CREATOR: Eric R Weeks / mandel program\n");
    fprintf(outfile,"%d %d\n255\n",HYRES,HXRES);
    for (size_t i = 0; i<HXRES*HYRES; i++) {
      fputc((char)image[i].r,outfile);
      fputc((char)image[i].g,outfile);
      fputc((char)image[i].b,outfile);
    }
    #endif
   
    tstop = second();

    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << HXRES*HYRES <<  std::endl;

 return 0;
}

