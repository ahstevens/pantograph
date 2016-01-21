#ifndef COSMO_H_
#define COSMO_H_
#include <algorithm>
#include <deque>
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
		void setLensOuterDimFactor(float factor);
		void setDimness(float dimness);

		void setMovableRotationAxis(glm::vec3 axis);
		void setMovableRotationAxis(float x, float y, float z);
		void setMovableRotationCenter(glm::vec3 ctr);
		void setMovableRotationCenter(float x, float y, float z);
		glm::vec3 getMovableRotationCenter();
		void setMovableRotationAngle(float angle);
		void setMovableRotationAxisScale(float scale);
		void setAxisLensSize(float radius);
		void setAxisLensOuterDimFactor(float factor);

		float getMovableRotationAxisScale();

		void setLensMode(bool yesno);
		void setAxisMode(bool yesno);
		void setVelocityMode(bool yesno);

		void toggleTrailsMode();
		void toggleShowOscillationAxis();

		void getMV(double *mv);

		void render();

		deque<glm::vec3> centerPoints;

    protected:
		void renderPoints();
		void renderPointsWithinSphere();
		void renderPointsWithinAxisCylinder();
		void renderStreaksWithin();
		void renderVelocities();

		float cylTest(const glm::vec3 & pt1, const glm::vec3 & pt2, float length_sq, float radius_sq, const glm::vec3 & testpt);

        std::vector<Particle> vParticles;       
        int particleCount;
        int samples;        // # of random samples points

		bool lensMode, axisMode, velocityMode, showTrails, showOscillationAxis;

		glm::vec3 movableRotationAxis, movableRotationCenter;
		float movableRotationAngle, movableAxisScale;

		glm::vec3 lensPos;
		float lensRadius, lensRadiusOuterDimFactor, axisRadius, axisRadiusOuterDimFactor, dimness;

		float maxDimension;

        std::vector<int> vSampleIDs;
};

#endif /*COSMO_H_*/
