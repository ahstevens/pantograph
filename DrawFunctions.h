#pragma once

#include "windows.h"
#include "gl_includes.h"
#include "ColorsAndSizes.h"
#include <glm\glm.hpp>
#include <string.h>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

void drawStrokeLabel3D(float x, float y, float z, float scale, char *text);

void drawHalfEllipse(float x, float y, float width, float height, bool archUp, int numSegs);

void drawBox(float x, float y, float sizeX, float sizeY, bool solid);

void drawLeftArrowBox(float x, float y, float width, float height);
void drawRightArrowBox(float x, float y, float width, float height);

void drawPercentageBox(float x, float y, float width, float height, float amount, float c1r, float c1g, float c1b, float c2r, float c2g, float c2b);

void drawNukeSymbol3D(float x, float y, float z, float radius);
void drawMapPoint3D(float x, float y, float z, float radius);

float getDigitsOffset(float value);

//-1 lineWidth draws filled
void drawCircle(float centerX, float centerY, float radius, float lineWidth, float numSegs);

void drawInsertButton(float x, float y, float width, float height);

void drawLabeled2LineButton(float x, float y, float width, float height, bool active, char* line1, char* line2);
void drawLabeledButton(float x, float y, float width, float height, bool active, char* label);

void drawReticle(float centerX, float centerY, float radius, float radian);

void drawMoveUpButton(float x, float y, float width, float height);
void drawMoveDownButton(float x, float y, float width, float height);

//pantograph:
void drawFingerIndicator(float centerX, float centerY, float radius, float color);
void draw3DReticle(float centerX, float centerY, float centerZ, float radius, float radian);


void drawArcButton(float centerX, float centerY, float innerRadius, float outerRadius, float startAngle, float endAngle, char* label, bool filled);
void drawArcButton(float centerX, float centerY, float innerRadius, float outerRadius, float startAngle, float endAngle, char* label1, char* label2, bool filled);
bool isOnArcButton(float x, float y, float centerX, float centerY, float innerRadius, float outerRadius, float minAngle, float maxAngle);
void drawArcScale(float centerX, float centerY, float radius, float startAngle, float endAngle, float indicatorAngle, char* startLabel, char* endLabel, char* scaleLabel);

void drawVolumeCursor(float x, float y, float z, float radius);

void drawAxes(float scale);

bool isOnButton(float mX, float mY, float bX, float bY, float bW, float bH, bool centered);