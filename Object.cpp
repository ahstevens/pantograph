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

    numberOfPoints = vSample.size() + vFocal.size();
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

    for (int i = 0; i < (int) vFocal.size(); ++i)
    {
        p = vFocal.at(i).pos;  // vertex coordinates
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
