// This has been checked and produces the same results as the Skepu2 version

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>

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


const float G       = 1.0;
const float delta_t = 0.1;

// Particle data structure that is used as an element type.
struct Particle {
	float x, y, z;
	float vx, vy, vz;
	float m;
};

/*
 * Array user-function that is used for applying nbody computation,
 * All elements from parr and a single element (named 'pi') are accessible
 * to produce one output element of the same type.
 */
Particle move(size_t index, const std::vector<Particle> parr) {

        size_t i = index;
	size_t np = parr.size();
	
        Particle pi = parr[i];
	
	float ax = 0.0, ay = 0.0, az = 0.0;
	
	for (size_t j = 0; j < np; ++j)
	{
		if (i != j)
		{
			Particle pj = parr[j];
			
			float rij = sqrt((pi.x - pj.x) * (pi.x - pj.x)
			               + (pi.y - pj.y) * (pi.y - pj.y)
			               + (pi.z - pj.z) * (pi.z - pj.z));
			
			float dum = G * pi.m * pj.m / pow(rij, 3);
			
			ax += dum * (pi.x - pj.x);
			ay += dum * (pi.y - pj.y);
			az += dum * (pi.z - pj.z);
		}
	}
	
	pi.x += delta_t * pi.vx + delta_t * delta_t / 2 * ax;
	pi.y += delta_t * pi.vy + delta_t * delta_t / 2 * ay;
	pi.z += delta_t * pi.vz + delta_t * delta_t / 2 * az;
	
	pi.vx += delta_t * ax;
	pi.vy += delta_t * ay;
	pi.vz += delta_t * az;
	
	return pi;
}

// Generate user-function that is used for initializing particles array.
Particle init(size_t index, size_t np)
{
	int s = index;
	int d = np / 2 + 1;
	int i = s % np;
	int j = ((s - i) / np) % np;
	int k = (((s - i) / np) - j) / np;
	
	Particle p;
	
	p.x = i - d + 1;
	p.y = j - d + 1;
	p.z = k - d + 1;
	
	p.vx = 0.0;
	p.vy = 0.0;
	p.vz = 0.0;
	
	p.m = 1;
	
	return p;
}


void nbody(std::vector<Particle> &particles, size_t iterations, size_t threads) {

    size_t np = particles.size();
    std::vector<Particle> doublebuffer(particles.size());

    std::vector<size_t> indices(particles.size());
	
    auto nbody_init =  Map(init, threads);
    auto nbody_simulate_step = Map(move, threads);

    // initialization of indices vector
    for (size_t i = 0; i<particles.size(); i++) {
      indices[i] = i;
    }
    // particle vectors initialization
    nbody_init(particles, indices, np);
	
    for (size_t i = 0; i < iterations; i += 2) {
      nbody_simulate_step(doublebuffer, indices, particles);
      nbody_simulate_step(particles, indices, doublebuffer);
    }
}


// A helper function to write particle output values to standard output stream.
void save_step(std::vector<Particle> &particles, std::ostream &os = std::cout)
{
	int i = 0;
	for (Particle &p : particles)
	{
		os << std::setw( 4) << i++
			<< std::setw(15) << p.x
			<< std::setw(15) << p.y
			<< std::setw(15) << p.z
			<< std::setw(15) << p.vx
			<< std::setw(15) << p.vy
			<< std::setw(15) << p.vz << "\n";
	}
}

//! A helper function to write particle output values to a file.
void save_step(std::vector<Particle> &particles, const std::string &filename)
{
	std::ofstream out(filename);
	
	if (out.is_open())
		save_step(particles, out);
	else
		std::cerr << "Error: cannot open this file: " << filename << "\n";
}


int main(int argc, char** argv) {

    const size_t t = NTHREADS;
    const size_t np = NPARTICLES;
    const size_t iterations = NITERATIONS;

    double tstart, tstop;

    tstart = second();

    std::vector<Particle> particles(np);

    nbody(particles, iterations, t);

    #ifdef OUTPUT
    save_step(particles, "output.txt");
    #endif

    tstop = second();

    std::cout << tstop-tstart << ", " << NTHREADS <<  ", " << NDATABLOCKS << ", " << NPARTICLES << ", " << NITERATIONS <<  std::endl;

    return 0;
}

