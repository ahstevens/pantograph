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

struct Particle {
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec4 col;
    int id;
    float mass;
	bool highlighted;
};

class Object
{
  public:
    Object();
    ~Object();
    void push();

	void setPosition(glm::vec3 pos);
	void setPosition(float x = 0.f, float y = 0.f, float z = 0.f);

	void setScale(glm::vec3 scale);
	void setScale(float x, float y, float z);
	void setScale(float scale = 1.f);

	void setRotation(float deg, glm::vec3 axis);
	void setRotation(float deg, float x, float y, float z);
	void setRotationAngle(float deg);

	virtual void render() = 0;

  protected:
    std::vector<Particle> vSample;
    
    int numberOfPoints;
    
    GLuint vaoID;
    GLuint bufferID;

	glm::vec3 position, scale, rotationAxis;
	float rotation;
};

#endif /*OBJECT_H_*/
