#include "Object.h"
#include <ctime> 
#include <cmath> 
#include <vector>
#include <fstream>
#include <iostream>
#include <time.h> 
#include <algorithm>


//----------------------------------------------------------------------------
Object::Object()
{   
	rotation = 0.f;
	rotationAxis = glm::vec3(0.f, 1.f, 0.f);
	position = glm::vec3(0.f, 0.f, 0.f);
	scale = glm::vec3(1.f, 1.f, 1.f);
}

//----------------------------------------------------------------------------
void Object::push()
{  
    // Create a vertex array object
    glGenVertexArrays( 1, &vaoID );
    glBindVertexArray( vaoID );

    numberOfPoints = vSample.size();
    const int floatPerVertex = 3;    // 4 for points, 4 for colors
    unsigned int bufferSize = floatPerVertex * numberOfPoints;
    unsigned int bufferBytes = sizeof(GLfloat) * numberOfPoints * floatPerVertex;
    float* vertices  = new float[ bufferSize ];
    
    // Fill the vertex, color, normals coordinate buffers
    int v = 0;
	glm::vec4 c = glm::vec4(1.0f, 1.0f, 1.0f, 0.85f);
	glm::vec3 p = glm::vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < (int) vSample.size(); ++i)
    {
        p = vSample.at(i).pos;  // vertex coordinates
        vertices[v++] = p.x; vertices[v++] = p.y; vertices[v++] = p.z; //vertices[v++] = 1.0;    
    }

    // Create a vertex buffer for point cloud
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, bufferBytes, vertices, GL_STATIC_DRAW  ); // GL_DYNAMIC_DRAW, GL_STATIC_DRAW STREAM
    delete vertices; // free unnecessary vertices, since they were pushed to the GPU
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * floatPerVertex, 0);

    glBindVertexArray(0);
}


Object::~Object()
{
    glDeleteVertexArraysAPPLE(1, &vaoID);
    glDeleteBuffersARB(1, &vaoID);
}


void Object::setPosition(glm::vec3 pos)
{
	this->position = pos;
}

void Object::setPosition(float x, float y, float z)
{
	setPosition(glm::vec3(x, y, z));
}

void Object::setScale(glm::vec3 scale)
{
	this->scale = scale;
}

void Object::setScale(float x, float y, float z)
{
	setScale(glm::vec3(x, y, z));
}

void Object::setScale(float scale)
{
	setScale(glm::vec3(scale, scale, scale));
}

void Object::setRotation(float deg, glm::vec3 axis)
{
	this->rotation = deg;
	this->rotationAxis = axis;
}

void Object::setRotation(float deg, float x, float y, float z)
{
	setRotation(deg, glm::vec3(x, y, z));
}

void Object::setRotationAngle(float deg)
{
	this->rotation = deg;
}

// returns the SQUARED distance if the point is within the sphere, otherwise return -1
float Object::sphereTest(const glm::vec3 sphereCenter, const float radius_sq, const glm::vec3 & testPt)
{
	glm::vec3 ptToCtrVector = testPt - sphereCenter;
	float dist_sq = ptToCtrVector.x * ptToCtrVector.x + ptToCtrVector.y * ptToCtrVector.y + ptToCtrVector.z * ptToCtrVector.z;
	return dist_sq <= radius_sq ? dist_sq : -1.f;
}

// returns the SQUARED distance from the center axis if the point is within the cylinder, otherwise return -1
// this function is adapted from http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml by Greg James @ NVIDIA
float Object::cylTest(const glm::vec3 & pt1, const glm::vec3 & pt2, float length_sq, float radius_sq, const glm::vec3 & testPt)
{
	float dx, dy, dz;	    // vector d  from line segment point 1 to point 2
	float pdx, pdy, pdz;    // vector pd from point 1 to test point
	float dot, dsq;

	dx = pt2.x - pt1.x;	    // translate so pt1 is origin.  Make vector from
	dy = pt2.y - pt1.y;	    // pt1 to pt2.  Need for this is easily eliminated
	dz = pt2.z - pt1.z;

	pdx = testPt.x - pt1.x;	// vector from pt1 to test point.
	pdy = testPt.y - pt1.y;
	pdz = testPt.z - pt1.z;

	// Dot the d and pd vectors to see if point lies behind the 
	// cylinder cap at pt1.x, pt1.y, pt1.z
	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.
	if (dot < 0.f || dot > length_sq)
		return(-1.f);
	else
	{
		// Point lies within the parallel caps, so find
		// distance squared from point to line, using the fact that sin^2 + cos^2 = 1
		// the dot = cos() * |d||pd|, and cross*cross = sin^2 * |d|^2 * |pd|^2
		// Carefull: '*' means mult for scalars and dotproduct for vectors
		// In short, where dist is pt distance to cyl axis: 
		// dist = sin( pd to d ) * |pd|
		// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
		// dsq = ( 1 - (pd * d)^2 / (|pd|^2 * |d|^2) ) * |pd|^2
		// dsq = pd * pd - dot * dot / lengthsq
		//  where lengthsq is d*d or |d|^2 that is passed into this function 

		// distance squared to the cylinder axis:
		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot / length_sq;

		if (dsq > radius_sq)
			return(-1.f);
		else
			return(dsq);		// return distance squared to axis		
	}
}