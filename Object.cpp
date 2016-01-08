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
    vec4 c = vec4(1.0f, 1.0f, 1.0f, 0.85f);
    vec3 p = vec3(0.0, 0.0, 0.0);

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

//-------------------------------------------------------------------------------
void Object::renderPoints()
{   
    glBindVertexArray(vaoID);
        glDrawArrays( GL_POINTS, 0, numberOfPoints ); 
    glBindVertexArray(0);
}

void Object::renderPointsWithin( vec3 point, float radius, float dimness)
{
	std::vector<Particle>::iterator it;

	glBegin(GL_POINTS);
		for (it = vSample.begin(); it != vSample.end(); ++it)
		{
			if (it->pos.x <= (point.x + radius) && it->pos.x >= (point.x - radius) &&
				it->pos.y <= (point.y + radius) && it->pos.y >= (point.y - radius) &&
				it->pos.z <= (point.z + radius) && it->pos.z >= (point.z - radius) &&
				sqrtf(pow(it->pos.x - point.x,2) + pow(it->pos.y - point.y, 2) + pow(it->pos.z - point.z, 2)) <= radius)
			{
				//glColor4f(it->col.r, it->col.g, it->col.b, 0.85f);
				glColor4f(1.f, 1.f, 1.f, 0.55f);
			}
			else
				glColor4f(1.f, 1.f, 1.f, 0.025f + (0.875f * (1.f - dimness)));

			glVertex3f(it->pos.x, it->pos.y, it->pos.z);
		}

		for (it = vFocal.begin(); it != vFocal.end(); ++it)
		{
			if (it->pos.x <= (point.x + radius) && it->pos.x >= (point.x - radius) &&
				it->pos.y <= (point.y + radius) && it->pos.y >= (point.y - radius) &&
				it->pos.z <= (point.z + radius) && it->pos.z >= (point.z - radius) &&
				sqrtf(pow(it->pos.x - point.x, 2) + pow(it->pos.y - point.y, 2) + pow(it->pos.z - point.z, 2)) <= radius)
			{
				//glColor4f(it->col.r, it->col.g, it->col.b, 0.85f);
				glColor4f(1.f, 1.f, 1.f, 0.55f);
			}
			else
				glColor4f(1.f, 1.f, 1.f, 0.025f + (0.875f * (1.f - dimness)));

			glVertex3f(it->pos.x, it->pos.y, it->pos.z);
		}
	glEnd();
}

void Object::renderStreaksWithin(vec3 point, float radius, float dimness)
{
	std::vector<Particle>::iterator it;

	glBegin(GL_POINTS);
	for (it = vSample.begin(); it != vSample.end(); ++it)
	{
		if (it->pos.x <= (point.x + radius) && it->pos.x >= (point.x - radius) &&
			it->pos.y <= (point.y + radius) && it->pos.y >= (point.y - radius) &&
			it->pos.z <= (point.z + radius) && it->pos.z >= (point.z - radius) &&
			sqrtf(pow(it->pos.x - point.x, 2) + pow(it->pos.y - point.y, 2) + pow(it->pos.z - point.z, 2)) <= radius)
		{
			vec3 end = it->pos + it->vel * 0.005f;

			glEnd();

			glBegin(GL_LINES);
				glColor4f(it->col.r, it->col.g, it->col.b, 0.0f);
				glVertex3f(it->pos.x, it->pos.y, it->pos.z);

				glColor4f(it->col.r, it->col.g, it->col.b, 1.0f);
				glVertex3f(end.x, end.y, end.z);
			glEnd();

			glBegin(GL_POINTS);
			continue;
		}
		else
			glColor4f(1.f, 1.f, 1.f, 0.05f + (0.8f * (1.f - dimness)));

		glVertex3f(it->pos.x, it->pos.y, it->pos.z);
	}

	for (it = vFocal.begin(); it != vFocal.end(); ++it)
	{
		if (it->pos.x <= (point.x + radius) && it->pos.x >= (point.x - radius) &&
			it->pos.y <= (point.y + radius) && it->pos.y >= (point.y - radius) &&
			it->pos.z <= (point.z + radius) && it->pos.z >= (point.z - radius) &&
			sqrtf(pow(it->pos.x - point.x, 2) + pow(it->pos.y - point.y, 2) + pow(it->pos.z - point.z, 2)) <= radius)
		{
			vec3 end = it->pos + it->vel * 0.005f;

			glEnd();

			glBegin(GL_LINES);
			glColor4f(it->col.r, it->col.g, it->col.b, 0.0f);
			glVertex3f(it->pos.x, it->pos.y, it->pos.z);

			glColor4f(it->col.r, it->col.g, it->col.b, 1.0f);
			glVertex3f(end.x, end.y, end.z);
			glEnd();

			glBegin(GL_POINTS);
			continue;
		}
		else
			glColor4f(1.f, 1.f, 1.f, 0.05f + (0.8f * (1.f - dimness)));

		glVertex3f(it->pos.x, it->pos.y, it->pos.z);
	}
	glEnd();
}

//-------------------------------------------------------------------------------
void Object::renderVelocities()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );   


    vec3 v  = vec3( 0.0f, 0.0f, 0.0f ); // 63.71757, 19.2537, 0.0488669 
    vec4 c  = vec4( 1.0f, 1.0f, 1.0f, 0.5f);
    vec3 p;
    for (int i = 0; i < (int) vSample.size()/2 ; ++i)
    {
        p = vSample.at(i).pos;
        v = ( vSample.at(i).vel ) * 0.0035f /scale * vectorScale;
        c = vSample.at(i).col; 
        
        // its done in CPU, implement geometry shader and move to GPU
        glBegin(GL_LINES);
            glColor4f( c.x, c.y, c.z, 1.0f );
            glVertex3f(p.x, p.y, p.z);   
            p -=  v;
            glColor4f( c.x, c.y, c.z, 0.0f );
            glVertex3f( p.x, p.y, p.z );   
        glEnd();
    }

    for (int i = 0; i < (int) vFocal.size() ; ++i)
    {
        p = vFocal.at(i).pos;
        v = ( vFocal.at(i).vel ) * 0.0025f /scale * vectorScale;
        c = vFocal.at(i).col; 
        
        // its done in CPU, implement geometry shader and move to GPU
        glBegin(GL_LINES);
            glColor4f( c.x, c.y, c.z, 1.0f );
            glVertex3f(p.x, p.y, p.z);   
            p -=  v;
            glColor4f( c.x, c.y, c.z, 0.0f );
            glVertex3f( p.x, p.y, p.z );   
        glEnd();
    }
}

Object::~Object()
{
    glDeleteVertexArraysAPPLE(1, &vaoID);
    glDeleteBuffersARB(1, &vaoID);
}
