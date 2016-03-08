#ifndef __Pantograph_h__
#define __Pantograph_h__

#include <windows.h>
#include "GL\gl_includes.h"
#include <math.h>
#include "DrawFunctions.h"
#include "stdio.h"
#include "glm\glm.hpp"
#include "Settings.h"

#define DRAG_BUBBLE_RADIUS 35
#define BUBBLE_PADDING 20
#define BUTTON_RADIUS 35

#define SELECT_INNER_RADIUS 10
#define SELECT_OUTER_RADIUS 100
#define SELECT_ANGLE_START 0.0
#define SELECT_ANGLE_END 1.8

class Pantograph
{
public:
	Pantograph();
	virtual ~Pantograph();

	void setBounds( float xmin, float xmax, float ymin, float ymax, float zmin, float zmax );
	void setScreenSize( float width, float height );
	void setFinger1( float x, float y );
	void setFinger2( float x, float y );
	
	float selection[ 3 ];
	void getSelection( float* x, float* y, float* z );
	void recalcSelection();

	Settings* settings;

	void getFinger1( float* x, float* y, float* z );
	void getFinger2( float* x, float* y, float* z );

	void draw3D();
	void draw2D();

	bool firstFingerSet();
	bool secondFingerSet();
	void resetFingers();
	bool isOnDragBubble( float X, float Y );
	bool swapChecked;
	bool swapNeeded();

	bool leftHanded;
	void setLeftHanded( bool useLeftHand );

	float acceptBubbleX, acceptBubbleY;
	bool isOnAcceptBubble( float X, float Y );

	void setDrawReticle( bool reticleDrawn );
	bool drawReticle;

	//look and feel variables:
	float distanceOutwards;
	float minDistanceBetween;
	float maxDistanceBetween;
	
	float xMin, xMax, xRange, yMin, yMax, yRange, zMin, zMax, zRange;
	float screenWidth, screenHeight;
	//float currentSelection[3];
	float finger1X, finger1Y, finger2X, finger2Y;

	//last draw info
	float lastDistanceBetween;
	float lastVectorX;
	float lastVectorY;


	//new access functions:
	bool getSelectPoint( float *x, float *y );
	bool getDepthFactor( float *depthFactor );

	float lastInterfingerAngle;

};

#endif