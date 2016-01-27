#pragma once

#include <glm/glm.hpp>
#include <vector>
//#include "misc/array.h"

//using namespace ds3d;

//
//CATMULL_ROM Spline
//
class SplinePath
{
public:
	SplinePath();
	~SplinePath();

	int size();
	bool isEnd();
	void clear();

	void setStartTime(float time);
	float getTotalTime();

	void addPoint(const glm::vec3 &p, float dt);
	void addLastPoint(const glm::vec3 &p);
	glm::vec3 getPoint(int i);

	glm::vec3 advanceAlongSpline();	

private:
	void addDelta(const float dt);
	glm::vec3 interpolate();

	glm::vec3 getVectorAlongSpline();	

    std::vector<glm::vec3> point;
    std::vector<float> deltas;
	glm::vec3  cp0, cp1, cp2, cp3;
	glm::vec3 interpPoint, previousPoint;
    int justPassed;
    int CONTROLPOINTS;
    float t;//0...1
    bool end_;
    float total_time_;
};
