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

    void Clear() {
		Point.clear();
		deltas.clear();
		InterpPoint = glm::vec3(0, 0, 0);
		PreviousPoint = glm::vec3(0, 0, 0);
		JustPassed = 0;
		t = 0;
		CONTROLPOINTS = 0;
		total_time_ = 0;
		end_ = false;
    }

    SplinePath() {
        InterpPoint = glm::vec3(0,0,0);
        PreviousPoint=  glm::vec3(0,0,0);
        JustPassed = 0;
        t=0;
        CONTROLPOINTS = 0;
        total_time_ = 0;
        end_ = false;
    };

//dt = duration time at p
    void addPoint(const glm::vec3 &p, float dt) {
        Point.push_back(p);
        addDelta(dt);
        CONTROLPOINTS=Point.size();
        total_time_+=dt;
    }

    void addLastPoint(const glm::vec3 &p);

    float getTotalTime() {
        return total_time_;
    }

    bool isEnd() {
        return end_;
    }

    int size() {
        return Point.size();
    }

    Vec3 getPoint(int i) {
        return Point[i];
    }

    ~SplinePath();

    void setStartTime(float time) {
        t=time;
    }

    Vec3 AdvanceAlongSpline();

private:
    void addDelta(const float dt) {
        deltas.push_back(dt);
    }
    //	int32 * GetMatrixAlongSpline(void);
    Vec3 GetVectorAlongSpline(void);
    Vec3 Interpolate(void);
    std::vector <Vec3> Point;
    std::vector <float> deltas;
    Vec3  cp0, cp1, cp2, cp3;
    Vec3 InterpPoint, PreviousPoint;
    int JustPassed;
    int CONTROLPOINTS;
    float t;//0...1
    bool end_;
    float total_time_;
};
