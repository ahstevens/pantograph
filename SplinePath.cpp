#include "SplinePath.h"

/************** Class constructors/destructor ********************************/
SplinePath::SplinePath()
{
	interpPoint = glm::vec3(0, 0, 0);
	previousPoint = glm::vec3(0, 0, 0);
	justPassed = 0;
	t = 0;
	CONTROLPOINTS = 0;
	total_time_ = 0;
	end_ = false;
}

SplinePath::~SplinePath()
{
    clear();
}
/*****************************************************************************/

/*************** Meta functions **********************************************/
int SplinePath::size() {
	return point.size();
}

bool SplinePath::isEnd() {
	return end_;
}

void SplinePath::clear() {
	point.clear();
	deltas.clear();
	interpPoint = glm::vec3(0, 0, 0);
	previousPoint = glm::vec3(0, 0, 0);
	justPassed = 0;
	t = 0;
	CONTROLPOINTS = 0;
	total_time_ = 0;
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
void SplinePath::addPoint(const glm::vec3 &p, float dt) {
	point.push_back(p);
	addDelta(dt);
	CONTROLPOINTS = point.size();
	total_time_ += dt;
}

void SplinePath::addLastPoint(const glm::vec3 &p) {

	point.push_back(p);
	addDelta(1);

	CONTROLPOINTS = point.size();

	//total_time_+=dt;

}

glm::vec3 SplinePath::getPoint(int i) {
	return point[i];
}
/*****************************************************************************/

/*************** Spline functions ********************************************/
glm::vec3 SplinePath::advanceAlongSpline()
{
	//JustPassed = future point to seek!!!
	if (justPassed == 0) {
		cp0 = point[CONTROLPOINTS - 1];
		cp1 = point[justPassed];
		cp2 = point[justPassed + 1];
		cp3 = point[justPassed + 2];
	}
	else if (justPassed == CONTROLPOINTS - 2) {
		cp0 = point[CONTROLPOINTS - 3];
		cp1 = point[CONTROLPOINTS - 2];
		cp2 = point[CONTROLPOINTS - 1];
		cp3 = point[0];
	}
	else if (justPassed == CONTROLPOINTS - 1) {
		cp0 = point[CONTROLPOINTS - 2];
		cp1 = point[CONTROLPOINTS - 1];
		cp2 = point[0];
		cp3 = point[1];
	}
	else {
		cp0 = point[justPassed - 1];
		cp1 = point[justPassed];
		cp2 = point[justPassed + 1];
		cp3 = point[justPassed + 2];
	}

	previousPoint = interpPoint;
	interpPoint = interpolate();

	t += deltas[justPassed];

	if (t >= 1.0f) {
		justPassed++;

		if (justPassed == CONTROLPOINTS) {
			end_ = true;
			justPassed = 0;
		}

		t = deltas[justPassed];
	}

	return interpPoint;

}

glm::vec3 SplinePath::getVectorAlongSpline(void)
{
	glm::vec3 Tmp;

	Tmp = interpPoint - previousPoint;
	Tmp = glm::normalize(Tmp);

	return Tmp;
}

void SplinePath::addDelta(const float dt) {
	deltas.push_back(dt);
}

glm::vec3 SplinePath::interpolate(void)
{
    float t2 = t * t;
    float t3 = t2 * t;

	glm::vec3 cp;

    cp.x =  0.5f * ((2 * cp1.x) +
                    (-cp0.x + cp2.x) * t +
                    (2 * cp0.x - 5*cp1.x + 4*cp2.x - cp3.x) * t2 +
                    (-cp0.x + 3*cp1.x - 3*cp2.x + cp3.x) * t3);

    cp.y =  0.5f * ((2 * cp1.y) +
                    (-cp0.y + cp2.y) * t +
                    (2 * cp0.y - 5*cp1.y + 4*cp2.y - cp3.y) * t2 +
                    (-cp0.y + 3*cp1.y - 3*cp2.y + cp3.y) * t3);

    cp.z =  0.5f * ((2 * cp1.z) +
                    (-cp0.z + cp2.z) * t +
                    (2 * cp0.z - 5*cp1.z + 4*cp2.z - cp3.z) * t2 +
                    (-cp0.z + 3*cp1.z - 3*cp2.z + cp3.z) * t3);

    return cp;
}