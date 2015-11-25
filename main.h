#ifndef __MAIN_H__
#define __MAIN_H__

#define GL_GLEXT_PROTOTYPES

#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/spline.hpp>

#define vec2 glm::vec2
#define vec3 glm::vec3
#define vec4 glm::vec4
#define mat4 glm::mat4

using namespace std; 
#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stack>

using namespace std; 

#ifdef __APPLE__ 
  #define FREEGLUT_LIB_PRAGMAS 0
  //#define TW_STATIC
  //#define FREEGLUT_STATIC
  #define GLEW_STATIC

  #define _Longlong int
  //#include <OpenGL/gl3.h>
  //#include <OpenGL/OpenGL.h>
  //#include <OpenGL/gl3.h>
  //#include <OpenGL/glu.h>
  #include <GL/glew.h>
  #include <AntTweakBar.h>
  #include <GL/freeglut.h>
  #include <GL/glext.h>
  //#include <GL/glxext.h>
  //#include <GL/wglext.h>

  //#define FREEGLUT_LIB_PRAGMAS 0
  //#include <GL/freeglut.h>

  #define glBindVertexArray glBindVertexArrayAPPLE
  #define glGenVertexArrays glGenVertexArraysAPPLE
  #define sprintf_s sprintf

#else
  // --------------------------
  //#define FREEGLUT_STATIC
  #define FREEGLUT_LIB_PRAGMAS 0
  //#define TW_STATIC
  #define GLEW_STATIC
  //#define GLFW_INCLUDE_GLU
  #include <GL/glew.h>
  //#define _LIB
  #include <AntTweakBar.h>
  #include <GL/freeglut.h>
  //#include <GL/glui.h>
  #include <GL/wglew.h>
  #include <GL/wglext.h>
  #include <GL/wglext.h>
#define _CRT_SECURE_NO_WARNINGS

#ifndef M_PI
 #define M_PI 3.1415926535897932384626433832795
#endif

  
#endif  // __APPLE__


#endif  // __MAIN_H__

