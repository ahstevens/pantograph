#ifndef COSMO_H_
#define COSMO_H_
#include <algorithm>
#include <deque>
#include "Object.h"
#include "Filament.h"

#define DIM_FRAMES 40

class Cosmo: public Object
{
    public:
        Cosmo();
		~Cosmo();

		enum Lens {
			SPHERE_POINTS,
			SPHERE_VELOCITY,
			CYLINDER_POINTS,
			CYLINDER_VELOCITY
		} curLens;

        void read(std::string fileName);
        void resample( int samples );
		float getMaxDistance();

		void setLensPosition(glm::vec3 pos);
		void setLensPosition(float x, float y, float z);
		void setLensSize(float radius);
		float getLensSize();

		void setBrightness(float brightness);
		void setLensBrightnessRange(float inner, float outer);

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
		void setLensType(Cosmo::Lens l);
		Cosmo::Lens getLensType();
		void setVelocityMode(bool yesno);

		void toggleTrailsMode();
		void toggleShowOscillationAxis();

		void getMV(double *mv);

		void requestDimming();

		void generateFilament();

		unsigned int getRemainingTargets();

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
		void renderVelocities();
		void renderLens();
		void renderOscillationAxis();
		void renderCursorTrails();
		void dim();
		float lensBrightnessRange();

        std::vector<Particle> vParticles;
        int particleCount;
        int samples;        // # of random samples points

		bool lensMode, velocityMode, showTrails;

		glm::vec3 lensPos;
		float sphereLensRadius, axisLensRadius;

		float lensInnerBrightness, lensOuterBrightness, normalBrightness, brightnessRatio;
		int dimTimer;

		float maxDistance;

        std::vector<int> vSampleIDs;

		Filament* filament;
};

#endif /*COSMO_H_*/
