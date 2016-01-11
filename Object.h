#ifndef OBJECT_H_
#define OBJECT_H_

#include <iostream>
#include <vector>
#include <ctime> 
#include <cmath> 
#include <fstream>
#include <time.h> 
#include <algorithm>

#include "main.h"
#include "DrawFunctions.h"

extern float vectorScale;

extern vec3 focalCenter;


struct Particle {
    vec3 pos;
    vec3 vel;
    vec4 col;
    int id;
    float mass;
};


class Object
{
  public:
    Object();
    ~Object();
    void push();

	void setPosition(vec3 pos);
	void setPosition(float x = 0.f, float y = 0.f, float z = 0.f);

	void setScale(vec3 scale);
	void setScale(float x, float y, float z);
	void setScale(float scale = 1.f);

	void setRotation(float deg, vec3 axis);
	void setRotation(float deg, float x, float y, float z);

	virtual void render() = 0;

  protected:
    std::vector<Particle> vSample;
    std::vector<Particle> vFocal;
    
    int numberOfPoints;
    
    GLuint vaoID;
    GLuint bufferID;

	vec3 position, scale, rotationAxis;
	float rotation;
};

#endif /*OBJECT_H_*/
