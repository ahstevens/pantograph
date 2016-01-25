#ifndef COSMO_H_
#define COSMO_H_
#include <algorithm>
#include <deque>
#include "Object.h"

#define DIM_FRAMES 40

class Cosmo: public Object
{
    public:
        Cosmo();
        void read(std::string fileName);
        void resample( int samples );
        //void getSamples(int n, float radius, vec3 center);
		float getMaxDistance();

		void setLensPosition(glm::vec3 pos);
		void setLensPosition(float x, float y, float z);
		void setLensSize(float radius);

		void setBrightnessRange(float min, float max);

		void setMovableRotationAxis(glm::vec3 axis);
		void setMovableRotationAxis(float x, float y, float z);
		glm::vec3 getMovableRotationAxis();
		void setMovableRotationCenter(glm::vec3 ctr);
		void setMovableRotationCenter(float x, float y, float z);
		glm::vec3 getMovableRotationCenter();
		void setMovableRotationAngle(float angle);
		void setMovableRotationAxisScale(float scale);
		void setAxisLensSize(float radius);

		float getMovableRotationAxisScale();

		void setLensMode(bool yesno);
		void setAxisMode(bool yesno);
		void setVelocityMode(bool yesno);

		void toggleTrailsMode();
		void toggleShowOscillationAxis();

		void getMV(double *mv);

		void requestDimming();

		void render();

		deque<glm::vec3> centerPoints;

    protected:
		struct MovableRotationAxis {
			glm::vec3 axis;
			glm::vec3 center;
			float angle;
			float scale;
			bool show;
		} oscAxis;

		void renderPoints();
		void renderPointsWithinSphere();
		void renderPointsWithinAxisCylinder();
		void renderStreaksWithin();
		void renderVelocities();
		void dim();
		float brightnessRange();

		float cylTest(const glm::vec3 & pt1, const glm::vec3 & pt2, float length_sq, float radius_sq, const glm::vec3 & testPt);
		float sphereTest(float radius_sq, const glm::vec3 & testPt);

        std::vector<Particle> vParticles;       
        int particleCount;
        int samples;        // # of random samples points

		bool lensMode, axisMode, velocityMode, showTrails;

		glm::vec3 lensPos;
		float sphereLensRadius, axisLensRadius;

		float maxBrightness, minBrightness, brightness;
		int dimTimer;

		float maxDistance;

        std::vector<int> vSampleIDs;
};

#endif /*COSMO_H_*/
