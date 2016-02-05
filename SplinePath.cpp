#include "SplinePath.h"

/************** Class constructors/destructor ********************************/
SplinePath::SplinePath()
{
	interpPoint = glm::vec3(0.f, 0.f, 0.f);
	previousPoint = glm::vec3(0.f, 0.f, 0.f);
	justPassed = 0;
	t = 0;
	total_time_ = 0.f;
	end_ = false;
}

SplinePath::~SplinePath()
{
}
/*****************************************************************************/

/*************** Meta functions **********************************************/
int SplinePath::size()
{
	return points.size();
}

float SplinePath::length()
{
	return len;
}

bool SplinePath::isEnd()
{
	return end_;
}

void SplinePath::clear()
{
	points.clear();
	deltas.clear();
	interpPoint = glm::vec3(0.f, 0.f, 0.f);
	previousPoint = glm::vec3(0.f, 0.f, 0.f);
	justPassed = 0;
	t = 0;
	total_time_ = 0.f;
	end_ = false;
}
/*****************************************************************************/

/*************** Time functions **********************************************/
void SplinePath::setStartTime(float time) {
	t = time;
}

float SplinePath::getTotalTime() {
	return total_time_;
}
/*****************************************************************************/

/*************** Point functions *********************************************/
// dt = duration time at p
void SplinePath::addPoint(const glm::vec3 &p, float dt)
{
	points.push_back(p);
	addDelta(dt);
	total_time_ += dt;
}

void SplinePath::addLastPoint(const glm::vec3 &p)
{
	points.push_back(p);
	addDelta(1);
}

glm::vec3 SplinePath::getPoint(int i)
{
	return points[i];
}
/*****************************************************************************/

/*************** Spline functions ********************************************/
glm::vec3 SplinePath::advanceAlongSpline()
{
	int nCtrlPts = points.size();

	//JustPassed = future point to seek!!!
	if (justPassed == 0) {
		cp0 = points[nCtrlPts - 1];
		cp1 = points[justPassed];
		cp2 = points[justPassed + 1];
		cp3 = points[justPassed + 2];
	}
	else if (justPassed == nCtrlPts - 2) {
		cp0 = points[nCtrlPts - 3];
		cp1 = points[nCtrlPts - 2];
		cp2 = points[nCtrlPts - 1];
		cp3 = points[0];
	}
	else if (justPassed == nCtrlPts - 1) {
		cp0 = points[nCtrlPts - 2];
		cp1 = points[nCtrlPts - 1];
		cp2 = points[0];
		cp3 = points[1];
	}
	else {
		cp0 = points[justPassed - 1];
		cp1 = points[justPassed];
		cp2 = points[justPassed + 1];
		cp3 = points[justPassed + 2];
	}

	previousPoint = interpPoint;
	interpPoint = interpolate();

	if (justPassed != 0 || t > 0.f) len += (interpPoint - previousPoint).length();

	t += deltas[justPassed];

	if (t >= 1.f && justPassed == nCtrlPts - 2 )
	{
		end_ = true;
		//justPassed = 0;
	}
	else if (t >= 1.f) {
		justPassed++;
		t = deltas[justPassed];
	}

	return interpPoint;

}

glm::vec3 SplinePath::getVectorAlongSpline(void)
{
	return glm::normalize(interpPoint - previousPoint);
}

void SplinePath::addDelta(const float dt)
{
	deltas.push_back(dt);
}

glm::vec3 SplinePath::interpolate(void)
{
    float t2 = t * t;
    float t3 = t2 * t;

	glm::vec3 cp;

    cp.x =  0.5f * ((2.f * cp1.x) +
                    (-cp0.x + cp2.x) * t +
                    (2.f * cp0.x - 5.f * cp1.x + 4.f * cp2.x - cp3.x) * t2 +
                    (-cp0.x + 3.f * cp1.x - 3.f * cp2.x + cp3.x) * t3);

    cp.y =  0.5f * ((2.f * cp1.y) +
                    (-cp0.y + cp2.y) * t +
                    (2.f * cp0.y - 5.f * cp1.y + 4.f * cp2.y - cp3.y) * t2 +
                    (-cp0.y + 3.f * cp1.y - 3.f * cp2.y + cp3.y) * t3);

    cp.z =  0.5f * ((2.f * cp1.z) +
                    (-cp0.z + cp2.z) * t +
                    (2.f * cp0.z - 5.f * cp1.z + 4.f * cp2.z - cp3.z) * t2 +
                    (-cp0.z + 3.f * cp1.z - 3.f * cp2.z + cp3.z) * t3);

    return cp;
}