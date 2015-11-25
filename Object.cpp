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
