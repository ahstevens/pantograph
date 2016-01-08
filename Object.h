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

extern float scale;
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

    void renderVelocities();
    void renderPoints();
	void renderPointsWithin(vec3 point, float radius, float dimness);
	void renderStreaksWithin(vec3 point, float radius, float dimness);

  protected:
    std::vector<Particle> vSample;
    std::vector<Particle> vFocal;
    
    int numberOfPoints;
    
    GLuint vaoID;
    GLuint bufferID;

};

#endif /*OBJECT_H_*/
