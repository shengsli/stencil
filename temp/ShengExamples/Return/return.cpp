/**
 * g++ return.cpp -std=c++11 -O2 -lpthread -DWIDTH=2 -DNTHREADS=4 -DHXRES=1024 -DHYRES=1024 -DITERMAX=1000 -DNDATABLOCKS=100 -DOUTPUT -o return
 * ./return
 * gimp stenciltest.ppm &
 */

#include <cassert>
#include <sys/time.h>

#include "../../Stencil1D.hpp"

double second() {
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}
 
typedef struct {
	int r;
	int g;
	int b;
} pixel_t;

FILE *outfile;

/**
 * Take an integer and scale it to 0..255.   
 */
pixel_t stencilkernel (int neighbourhood[], int width) {
	pixel_t pixel;
	int color =  neighbourhood[width]*255 / (HXRES*HYRES);
	pixel.r = pixel.g = pixel.b = color;
	return pixel;
}

int main(int argc, char** argv) {
    FILE *outfile;
    double tstart, tstop;
    tstart = second();
	
    std::vector<int> in(HXRES*HYRES);
    for(size_t i = 0; i < HXRES*HYRES; ++i)
        in[i] = i;

    std::vector<pixel_t> image (in.size());
    auto stencil = Stencil1D(stencilkernel, WIDTH, NTHREADS);
    stencil(image, in);

#ifdef OUTPUT
    // Output results
    outfile = fopen("stenciltest.ppm","w");
    fprintf(outfile,"P6\n# Stencil testing\n");
    fprintf(outfile,"%d %d\n255\n",HYRES,HXRES);
    for (size_t i = 0; i<HXRES*HYRES; i++)
	{
		fputc((char)image[i].r,outfile);
		fputc((char)image[i].g,outfile);
		fputc((char)image[i].b,outfile);
    }
#endif
   
    tstop = second();
    std::cout << tstop-tstart << ", " << WIDTH << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << HXRES*HYRES <<  std::endl;
	return 0;
}
