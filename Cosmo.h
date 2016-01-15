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
		float getMaxDimension();

		void setLensPosition(glm::vec3 pos);
		void setLensPosition(float x, float y, float z);
		void setLensSize(float radius);
		void setDimness(float dimness);

		void setMovableRotation(float angle, glm::vec3 axis, glm::vec3 center);
		void setMovableRotationAxis(glm::vec3 axis);
		void setMovableRotationAxis(float x, float y, float z);
		void setMovableRotationCenter(glm::vec3 ctr);
		void setMovableRotationCenter(float x, float y, float z);
		void setMovableRotationAngle(float angle);

		void setLensMode(bool yesno);
		void setVelocityMode(bool yesno);

		void getMV(double *mv);

		void render();

    protected:
		void renderPoints();
		void renderPointsWithin();
		void renderStreaksWithin();
		void renderVelocities();

        std::vector<Particle> vParticles;       
        int particleCount;
        int samples;        // # of random samples points

		bool lensMode, velocityMode;

		glm::vec3 movableRotationAxis, movableRotationCenter;
		float movableRotationAngle;

		glm::vec3 lensPos;
		float lensRadius, dimness;

		float maxDimension;

        std::vector<int> vSampleIDs;
};

#endif /*COSMO_H_*/
