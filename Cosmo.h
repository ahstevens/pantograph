#ifndef COSMO_H_
#define COSMO_H_
#include <algorithm>
#include "Object.h"


class Cosmo: public Object
{
    public:
        Cosmo();
        void read(std::string fileName);
        void resample( int samples );
        //void getSamples(int n, float radius, vec3 center);
		float getMaxDepth();
		float getMinDepth();

    protected:
        std::vector<Particle> vParticles;       
        int particleCount;
        int samples;        // # of random samples points

		float maxDepth, minDepth;

        std::vector<int> vSampleIDs;
};

#endif /*COSMO_H_*/
