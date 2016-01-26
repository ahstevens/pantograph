#include "spline.h"



SplinePath::~SplinePath()
{

    Clear();
}

void SplinePath::addLastPoint(const Vec3 &p) {

    Point.push_back(p);
    addDelta(1);

    CONTROLPOINTS=Point.size();

    //total_time_+=dt;

}

Vec3 SplinePath::Interpolate(void)
{
    float t2 = t * t;
    float t3 = t2 * t;

    Vec3 cp;

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

Vec3 SplinePath::GetVectorAlongSpline(void)
{
    Vec3 Tmp;

    Tmp=InterpPoint- PreviousPoint;
	Tmp.normalize();

    return Tmp;
}

/*
Matrix4x4 CCRSpline::GetMatrixAlongSpline(void)
{
	Matrix4x4 World;

	Vector4 Look = GetVectorAlongSpline();

	Vector4 Up = Vector4(0.0f, 1.0f, 0.0f, 1.0f);

	Vector4 Right = Look.Cross(Up);

	Up = Right.Cross(Look);

	World(0,0) = Right.x;
	World(0,1) = Right.y;
	World(0,2) = Right.z;
	World(0,3) = 0.0f;

	World(1,0) = Up.x;
	World(1,1) = Up.y;
	World(1,2) = Up.z;
	World(1,3) = 0.0f;

	World(2,0) = Look.x;
	World(2,1) = Look.y;
	World(2,2) = Look.z;
	World(2,3) = 0.0f;

	World(3,0) = InterpPoint.x;
	World(3,1) = InterpPoint.y;
	World(3,2) = InterpPoint.z;
	World(3,3) = 1.0f;

	return World;
}
*/
Vec3 SplinePath::AdvanceAlongSpline()
{
    //JustPassed = future point to seek!!!
    if (JustPassed == 0) {
        cp0 = Point[CONTROLPOINTS-1];
        cp1 = Point[JustPassed];
        cp2 = Point[JustPassed+1];
        cp3 = Point[JustPassed+2];
    } else if (JustPassed == CONTROLPOINTS -2) {
        cp0 = Point[CONTROLPOINTS - 3];
        cp1 = Point[CONTROLPOINTS - 2];
        cp2 = Point[CONTROLPOINTS - 1];
        cp3 = Point[0];
    } else if (JustPassed == CONTROLPOINTS - 1) {
        cp0 = Point[CONTROLPOINTS - 2];
        cp1 = Point[CONTROLPOINTS - 1];
        cp2 = Point[0];
        cp3 = Point[1];
    } else {
        cp0 = Point[JustPassed - 1];
        cp1 = Point[JustPassed];
        cp2 = Point[JustPassed + 1];
        cp3 = Point[JustPassed + 2];
    }

    PreviousPoint = InterpPoint;
    InterpPoint = Interpolate();

    t+=deltas[JustPassed];

    if (t >= 1.0f) {
        JustPassed++;

        if (JustPassed == CONTROLPOINTS) {
            end_=true;
            JustPassed = 0;

        }

//	t=0;
        t=deltas[JustPassed];

    }

    return InterpPoint;

}