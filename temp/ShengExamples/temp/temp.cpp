// NEXT: add a parameter for the stencil shape?  At the moment this is just an int indicating the
// (symmetric) number of neighbours on each side of the origin. Boundary will be
// wrap around.
//
// THEN: add a parameter to select boundary arrangements

// Thought: Will we need barriers or equivalent if we allow input = output, ie (in place)
// that could get messy and expensive - maybe better to forbid this, or make it
// a more expensive option

#include <cassert>
#include <sys/time.h>

#include "../../Stencil.hpp"

double second() {
	struct timeval tp;
	struct timezone tzp;
	int i;
	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}


// add -DOUTPUT to test if required

// g++ temp.cpp -std=c++11 -lpthread -O2 -DNTHREADS=32 -DHXRES=1024 -DHYRES=1024 -DITERMAX=10000 -DNDATABLOCKS=10000 -o temp

// ./tempScript 4 20 5 results.csv 1024 1024 1000


//#define hxres  2048           // horizonal resolution 
//#define hyres  2048           // vertical resolution  
//#define itermax 10000         // how many iterations to do 
 
typedef struct {
	int r;
	int g;
	int b;
} pixel_t;

FILE *outfile;

pixel_t stencilkernel (int neighbourhood[], int width) {
	int hx, hy;
	pixel_t pixel;
 
	hx = neighbourhood[width]%HXRES;
	hy = neighbourhood[width]/HYRES;

	pixel.r = hx*256/HXRES;
	pixel.g = hx*256/HXRES;
	pixel.b = hx*256/HXRES;
	
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
    auto stencil = Stencil(stencilkernel, 2, NTHREADS);
    stencil(image,in);

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
    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << ITERMAX << ", " << HXRES*HYRES <<  std::endl;
	return 0;
}
