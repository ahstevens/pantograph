#define _CRT_SECURE_NO_WARNINGS
#define GL_GLEXT_PROTOTYPES
#define FREEGLUT_LIB_PRAGMAS 0
#define GLEW_STATIC

#include "GL\gl_includes.h"

#include <iostream>
#include <math.h>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp> 
#include <glm\gtx\rotate_vector.hpp>
#include "Cosmo.h"
#include "Settings.h"
#include "TouchManager.h"
#include "Polhemus.h"

#define VP_LEFT		-10.0f
#define VP_RIGHT	10.0f
#define VP_BOTTOM	-10.0f
#define VP_TOP		10.0f
#define NEAR_CP		20.0f
#define COW_Z 		30.0f
#define FAR 		100.0f 
#define OSC_ANGLE 1.f
#define REFRESH 30

Settings* settings;

TouchManager* touchManager;

Polhemus* polhemus;

float rx,ry;
//float rw, rh;
float mXscreen = 0.0f, mYscreen = 0.0f, mZ = 0.f;  // mouse x in screen coords.
//float holdCowX = 0.0f, holdCowY = 0.0f, holdCowZ = 0.0f; // saves CoW for transitioning
glm::vec3 holdCow;
//float cowX = 0.0f,cowY= 0.0f, cowZ= 0.0f; // the CoW relativ to the data spaces
glm::vec3 cow;
float xeye = 0.0f, yeye= 0.0f, zeye = 0.0f; // the offsets calculated for a move on mouse down
glm::vec3 transitionVector = glm::vec3(0.f, 0.f, 0.f);

float rotX,rotY; // change the rotation about the cow
float holdMx, holdMy;

glm::vec3 holdRotationAxis;

bool cursorOnMouse = false;
bool leftMouseDown, rightMouseDown, middleMouseDown;

float dragX, dragY; // used for rotating the universe
float rotation[500];
int timer;
float winWidth = 1024;
float winHeight = 768; // screen dimensions
int rt; // rotation timer

//---------------------------------------------------------------------------- 
//----------------------------------------------------------------------------
float aspect = 1.0f;
float scale;
GLint mainWindow;
float eyeSep = 0.25f;

#define QUAD_BUFFER true
//float eyeOffset     = 3.25f * 10.0f/16.8f;

//---------------------------------------------------------------------------- 

std::string inputFiles[] = { "haloes_data/256cosmo.0.7.cosmo" };
//std::string inputFiles[] = { "/mnt/main/haloes_data/256cosmo.0.7.cosmo" };

Cosmo *cosmo;
bool velocity = false;
//----------------------------------------------------------------------------
#define TRANS_FRAMES 40
int transTimer;

const glm::vec4 advanceButtonColor = glm::vec4(0.f, 6.f, 3.f, 0.5f);
glm::vec2 advanceButtonPos;
glm::vec2 advanceButtonDim = glm::vec2(200.f, 100.f);
bool buttonActive = false;


void transition()  // used to translate smoothly
{	
	// get interpolated parametric value based off of transition timer
	float t = 1.f - float(transTimer)/TRANS_FRAMES;

	if (transTimer >= 0)
		cosmo->setPosition(holdCow + transitionVector * t);

	transTimer--;

	// reset mouse coords
	if (transTimer == 0) mXscreen = mYscreen = mZ = 0.f;
}

void calculateTransition(float x, float y)
{
	// get data object's model-view matrix and invert it
	double mv[16];
	cosmo->getMV(mv);
	glm::mat4 MVinv = glm::inverse(glm::make_mat4(mv));

	// get cursor location wrt cosmo model origin
	glm::vec3 newCow = glm::vec3( MVinv * glm::vec4(settings->currentlySelectedPoint[0],
		settings->currentlySelectedPoint[1],
		settings->currentlySelectedPoint[2],
		1.f) );

	// translation position needed to put cursor at focal center
	glm::vec3 newCowTrans = -newCow * scale;
	holdCow = -cow * scale;

	// calc the translation vector for the transition
	transitionVector = newCowTrans - holdCow;
	
	// start transition timer
	transTimer = TRANS_FRAMES;

	// update the CoW
	cow = newCow;

	settings->transitionRequested = false;
}


//-------------------------------------------------------------------------------
void generate_theta()
{
	for(int i=0; i<2*REFRESH; ++i) // 0.5 Hz oscillation
	{
		rotation[i] = OSC_ANGLE * sin(float(i)*3.141592f/(REFRESH)) / 2.f;
	}
}

void stayInWorld(glm::vec3 &newPos, glm::vec3 oldPos)
{
	if (newPos.x < VP_LEFT || newPos.x > VP_RIGHT)
		newPos.x = oldPos.x;

	if (newPos.y < VP_BOTTOM || newPos.y > VP_TOP)
		newPos.y = oldPos.y;
	
	if (newPos.z < settings->worldDepths[0] || newPos.z > settings->worldDepths[1])
		newPos.z = oldPos.z;
}

void processPendingInteractions()
{
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ = 0.0;
	GLdouble posX, posY, posZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	//if fingers down convert screen coords to model coords
	if (settings->finger1sX != -1 && settings->finger1sY != -1 && settings->finger2sX != -1 && settings->finger2sY != -1)
	{
		//draw a plane at screen height to fill depth buffer so we can get selection depth from it
		glBegin(GL_QUADS);
			glNormal3f(0, 0, 1);
			glVertex3f(-100, 100, settings->worldDepths[ 1 ]);
			glVertex3f(-100, -100, settings->worldDepths[ 1 ]);
			glVertex3f(100, -100, settings->worldDepths[ 1 ]);
			glVertex3f(100, 100, settings->worldDepths[ 1 ]);
		glEnd();

		// this gives us the Z value in NDC (or is it Screen Coords?) from the plane we just created
		glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

		// get finger 1 model space coords
		gluUnProject(settings->finger1sX, settings->finger1sY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		settings->finger1modelCoords[0] = (float)posX;
		settings->finger1modelCoords[1] = (float)posY;
		settings->finger1modelCoords[2] = (float)posZ;

		// get finger 2 model space coords
		gluUnProject(settings->finger2sX, settings->finger2sY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		settings->finger2modelCoords[0] = (float)posX;
		settings->finger2modelCoords[1] = (float)posY;
		settings->finger2modelCoords[2] = (float)posZ;


		// change rotation axis to be vector between fingers
		//glm::vec3 yAxis = normalize(glm::vec3(settings->finger2modelCoords[0] - settings->finger1modelCoords[0],
		//										settings->finger2modelCoords[1] - settings->finger1modelCoords[1],
		//										settings->finger2modelCoords[2] - settings->finger1modelCoords[2]));
		//cosmo->setMovableRotationAxis(yAxis);
	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	//if actively positioning finger, maintain corresponding model coordinates
	if (settings->positioningPointLocation[0] != -1 && settings->positioningPointLocation[1] != -1 && settings->positioningPointLocation[2] != -1)
	{
		float depth = settings->worldDepths[0] + (settings->worldDepths[1] - settings->worldDepths[0]) * settings->positioningPointLocation[2];

		glBegin(GL_QUADS);
			glNormal3f(0, 0, 1);
			glVertex3f(-100, 100, depth);
			glVertex3f(-100, -100, depth);
			glVertex3f(100, -100, depth);
			glVertex3f(100, 100, depth);
		glEnd();

		glReadPixels(settings->positioningPointLocation[0], settings->positioningPointLocation[1], 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(settings->positioningPointLocation[0], settings->positioningPointLocation[1], winZ, modelview, projection, viewport, &posX, &posY, &posZ);
		
		settings->positioningModelCoords[0] = (float)posX;
		settings->positioningModelCoords[1] = (float)posY;
		settings->positioningModelCoords[2] = (float)posZ;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}//if need to update active positioning
	else
	{
		// reset positioning values
		settings->positioningModelCoords[0] = -1;
		settings->positioningModelCoords[1] = -1;
		settings->positioningModelCoords[2] = -1;

		//if(!settings->mouseMode) cosmo->setAxisMode(false);
	}

}//end processPendingInteractions()

void updateLens()
{
	// get data object's model-view matrix and invert it
	double mv[16];
	cosmo->getMV(mv);
	glm::mat4 MVinv = glm::inverse( glm::make_mat4( mv ) );

	// get model-space coordinate for pantograph point and set it
	glm::vec4 newPos = MVinv * glm::vec4(settings->currentlySelectedPoint[0],
		settings->currentlySelectedPoint[1],
		settings->currentlySelectedPoint[2],
		1.f);
	
	glm::vec3 *oldPos = new glm::vec3(*cosmo->getLensPosition());
	cosmo->setLensPosition(newPos.x, newPos.y, newPos.z);
	cosmo->setMovableRotationCenter(newPos.x, newPos.y, newPos.z);

	if (settings->study->isStudyStarted())
	{
		if (settings->modeSwitched && !settings->study->isTrialStarted())
		{
			settings->study->logData("interaction initiated", cosmo->getLensPosition(), cosmo->getFilament(), nullptr, true);
			settings->modeSwitched = false;
			settings->study->startTrial();
		}

		if (settings->trackingCursor)
		{
			settings->cursorDistance += glm::length(glm::vec3(newPos) - *oldPos);
			settings->study->logData("cursor track", cosmo->getLensPosition(), cosmo->getFilament(), &(settings->cursorDistance));
		}
	}

	delete oldPos;
}

void perRenderUpdates()
{
	if(settings->study->currentMode == StudyManager::PANTOGRAPH)
		processPendingInteractions();

	if (settings->study->modeRestriction == StudyManager::NONE ||
		settings->study->modeRestriction == StudyManager::POLHEMUS)
	{
		polhemus->update();

		if (polhemus->isTriggerDown())
		{
			if (settings->study->currentMode == StudyManager::NONE)
				settings->activate(StudyManager::POLHEMUS);

			glm::vec3 pos = (settings->polhemusOrigin - polhemus->getPosition()) * settings->polhemusMovementMultiplier;
			stayInWorld(pos, glm::vec3(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->currentlySelectedPoint[2]));

			settings->currentlySelectedPoint[0] = pos.x;
			settings->currentlySelectedPoint[1] = pos.y;
			settings->currentlySelectedPoint[2] = pos.z;
		}
		else if (settings->study->currentMode == StudyManager::POLHEMUS)
			settings->deactivate();
	}

	if (settings->study->currentState == StudyManager::ACTIVE)
		cosmo->setLensMode(true);
	else
		cosmo->setLensMode(false);

	if (settings->transitionRequested) 
		calculateTransition(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1]);

	if (settings->dimmingRequested)
	{
		cosmo->requestDimming();
		settings->dimmingRequested = false;
	}

	if(settings->transitionOnLensExit) transition();

	rt = timer % (REFRESH * 2); // 60 Hz on 60 Hz machine

	if (settings->study->getMotion()) cosmo->setMovableRotationAngle(rotation[rt]);
	else cosmo->setMovableRotationAngle(0.f);
	
	// adjust panto depth so that back of dataset always accessible
	//settings->worldDepths[0] = (-cosmo->getMaxDistance() - cow.z)*scale;
	//settings->worldDepths[1] = 10.f - 0.001f;

	if(touchManager->perRenderUpdate())
		cosmo->skipLensModeThisRender();

	if (settings->study->currentMode == StudyManager::MOUSE)
	{
		if (cursorOnMouse)
		{
			GLint viewport[4];
			GLdouble modelview[16];
			GLdouble projection[16];
			GLfloat winZ = 0.0;
			GLdouble posX, posY, posZ;

			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
			glGetDoublev(GL_PROJECTION_MATRIX, projection);
			glGetIntegerv(GL_VIEWPORT, viewport);

			glBegin(GL_QUADS);
				glNormal3f(0, 0, 1);
				glVertex3f(-100, 100, mZ);
				glVertex3f(-100, -100, mZ);
				glVertex3f(100, -100, mZ);
				glVertex3f(100, 100, mZ);
			glEnd();

			glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
			gluUnProject(rx, ry, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

			settings->currentlySelectedPoint[0] = (float)posX;
			settings->currentlySelectedPoint[1] = (float)posY;
			settings->currentlySelectedPoint[2] = (float)posZ;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			settings->currentlySelectedPoint[0] = mXscreen;
			settings->currentlySelectedPoint[1] = mYscreen;
			settings->currentlySelectedPoint[2] = mZ;
		}
	}

	if (settings->study->isStudyStarted() && settings->trackingCursor && cosmo->getRemainingTargets() == 0)
	{
		settings->trackingCursor = false;
		settings->study->logData("filament completed", cosmo->getLensPosition(), cosmo->getFilament(), &(settings->cursorDistance));
	}
	
	// RESET VARIABLES AT BEGINNING OF TRIAL
	if (cosmo->getRemainingTargets() == 0 && settings->study->currentState == StudyManager::OFF)
	{
		settings->cursorDistance = 0.f;
		mZ = 0.f;

		settings->study->next();

		if (settings->study->isStudyDone()) glutLeaveMainLoop();
	}

	if (settings->study->currentState == StudyManager::ACTIVE)
		updateLens();
	
	if (settings->study->isStudyStarted() && cosmo->checkHighlight()) {
		settings->trackingCursor = true;
		settings->study->logData("filament begun", cosmo->getLensPosition(), cosmo->getFilament(), &settings->cursorDistance, true);
	}
}

void drawScene(int eye) //0=left or mono, 1=right
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// translate from scene	origin 10 units behind near clipping plane to each eye
	float eyeOff = settings->study->getStereo() ? eyeSep : 0.f;
	glTranslatef(!eye ? eyeOff : -eyeOff, 0.0, -NEAR_CP - 10.0); // center of universe offset..

	//draw active positioning pole:
	//if (settings->positioningModelCoords[2] != -1)
	if (settings->study->currentState == StudyManager::ACTIVE && cosmo->lensModeThisRender())
	{
		glLineWidth(2);
		glColor4f(0.8, 0.8, 0.95, 0.25);
		glBegin(GL_LINES);
			glVertex3f(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->worldDepths[1]);
			glVertex3f(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->worldDepths[0]);
		glEnd();

		glColor4f(1.0, 1.0, 0.25, 1.0);
		glPointSize(4);
		glBegin(GL_POINTS);
			glVertex3f(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->currentlySelectedPoint[2]);
		glEnd();

		touchManager->draw3D();
		drawVolumeCursor(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->currentlySelectedPoint[2], cosmo->getLensSize());
	}

	// render cosmos point cloud
	cosmo->render();
}

void drawOverlay()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS);

	//draw 2D interface elements:
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); // save current projection matrix
	glLoadIdentity(); // start fresh
	glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT), -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);

	touchManager->draw2D();

	static float fps = 0.0f;
	static float lastTime = 0.0f;
	static int lastfps = 0;
	static char buffer[32] = { 0 };
	float currentTime = GetTickCount64() * 0.001f;
	++fps;

	if (currentTime - lastTime > 1.0f)
	{
		lastfps = (int)fps;
		fps = 0;
		lastTime = currentTime;
	}
	
	if (!settings->study->isStudyStarted())
	{
		sprintf(buffer, "Targets Remaining: %u", cosmo->getRemainingTargets());
		glColor3f(1, 1, 1);
		glLineWidth(10);
		drawStrokeLabel3D(20, 20, 0, 0.5, buffer);
	}

	std::string modeText;
	switch (settings->study->isStudyStarted() ? settings->study->modeRestriction : settings->study->currentMode)
	{
	case StudyManager::NONE:
		modeText = "NONE";
		break;
	case StudyManager::MOUSE:
		modeText = "MOUSE";
		break;
	case StudyManager::PANTOGRAPH:
		modeText = "PANTOGRAPH";
		break;
	case StudyManager::POLHEMUS:
		modeText = "POLHEMUS";
		break;
	}

	sprintf(buffer, "Interaction Mode: %s", modeText.c_str());
	glColor3f(1, 1, 1);
	glLineWidth(10);
	drawStrokeLabel3D(20, winHeight - 40, 0, 0.2, buffer);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void redraw( void )
{
	perRenderUpdates();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--------------------------------------------------
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(VP_LEFT,	VP_RIGHT,
		aspect*VP_BOTTOM, aspect*VP_TOP, NEAR_CP, FAR);
	//--------------------------------------------------
	drawScene(0);
	drawOverlay();

	//--------------------------------------------------
	glutSwapBuffers();

	++timer;
}

//-------------------------------------------------------------------------------
void redraw_stereo(void)
{
	perRenderUpdates();

	float eyeOff = settings->study->getStereo() ? eyeSep : 0.f;
  	//--------------------------------------------------
	// LEFT EYE = 0, RIGHT EYE = 1
	for (int eye = 0; eye < 2; ++eye)
	{
		glDrawBuffer(!eye ? GL_BACK_LEFT : GL_BACK_RIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//--------------------------------------------------
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(VP_LEFT + (!eye ? eyeOff : -eyeOff),
			VP_RIGHT + (!eye ? eyeOff : -eyeOff),
			aspect*VP_BOTTOM, aspect*VP_TOP, NEAR_CP, FAR);
		//--------------------------------------------------
		drawScene(eye);
		drawOverlay();
	}

	//--------------------------------------------------
	glutSwapBuffers();

	++timer;
}

void mouseButton(int button, int state, int x, int y)
{
	rx = float(x); 
	ry = float(winHeight - y);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		leftMouseDown = true;
		holdMy = float(y);

		//if (cosmo->getRemainingTargets() == 0 && !settings->mouseMode && !settings->pantographMode && 
		//	isOnButton(rx, ry, advanceButtonPos.x, advanceButtonPos.y, advanceButtonDim.x, advanceButtonDim.y, true))
		//	cosmo->generateFilament();

		if ((settings->study->modeRestriction == StudyManager::NONE ||
			settings->study->modeRestriction == StudyManager::MOUSE) &&
			settings->study->currentMode == StudyManager::NONE)
		{
			settings->activate(StudyManager::MOUSE);

			mXscreen = (VP_RIGHT - VP_LEFT) * (rx / winWidth - 0.5f);
			mYscreen = aspect*(VP_TOP - VP_BOTTOM) * (ry / winHeight - 0.5f);
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		leftMouseDown = false;


		if ((settings->study->modeRestriction == StudyManager::NONE ||
			settings->study->modeRestriction == StudyManager::MOUSE) &&
			settings->study->currentMode == StudyManager::MOUSE)
		{
			settings->deactivate();
		}
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		rightMouseDown = true;
		holdMx = rx;
		holdMy = ry;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		rightMouseDown = false;
		
		rotY = rotY + dragX;	
		//cerr << "Rot Up Y " << rotY << "\n";
		dragX = dragY = 0.0;
	}

	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		// for using middle mouse button + drag to orient rotation axis
		//if (!leftMouseDown)
		//{
		//	holdRotationAxis = cosmo->getMovableRotationAxis();
		//	holdMx = rx;
		//	holdMy = ry;
		//}
		middleMouseDown = true;
	}

	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
	{
		middleMouseDown = false;
	}

	// MOUSE WHEEL EVENT
	// Each wheel event behaves like a mouse click (GLUT_DOWN and GLUT_UP)
	// so discard one of the events
	if ( ( (button == 3) || (button == 4) ) && state == GLUT_DOWN ) // It's a wheel event
   {

	   if (settings->study->currentMode == StudyManager::MOUSE)
	   {
		   if (button == 3) // wheel up
		   {
			   if (mZ - 1.f < settings->worldDepths[0]) mZ = settings->worldDepths[0];
			   else mZ -= 0.5f;
		   }
		   else // wheel down
		   {
			   if (mZ + 1.f > settings->worldDepths[1]) mZ = settings->worldDepths[1];
			   else mZ += 0.5f;
		   }
	   }
   }
}

// called when a mouse is in motion with a button down
void motion(int x, int y) 
{
	rx = float(x); ry = float(winHeight - y);

	// LEFT MOUSE DOWN
	if (settings->study->currentMode == StudyManager::MOUSE)
	{
		mXscreen = (VP_RIGHT - VP_LEFT) * (rx / winWidth - 0.5f);
		mYscreen = aspect*(VP_TOP - VP_BOTTOM) * (ry / winHeight - 0.5f);
	}

	// use middle mouse button + drag to orient rotation axis
	//if ((settings->study->currentMode == StudyManager::TRAINING || settings->study->currentMode == StudyManager::MOUSE) && middleMouseDown && !leftMouseDown)
	//{
	//	float h_displacement = (float) (holdMx - rx) / 10.f;
	//	float v_displacement = (float)(holdMy - ry) / 10.f;

	//	glm::vec3 newAxis = glm::vec3( glm::rotate(glm::radians(h_displacement), glm::vec3(0.f, 0.f, 1.f)) * glm::vec4(holdRotationAxis, 0.f));

	//	cosmo->setMovableRotationAxis(newAxis);
	//}
}

// called when a mouse is in motion
void passiveMotion(int x, int y)
{
	rx = float(x); ry = float(winHeight - y);

	//if (rx <= advanceButtonPos.x + advanceButtonDim.x / 2.f && rx >= advanceButtonPos.x - advanceButtonDim.x / 2.f &&
	//	ry <= advanceButtonPos.y + advanceButtonDim.y / 2.f && ry >= advanceButtonPos.y - advanceButtonDim.y / 2.f)
	//	buttonActive = true;
	//else
	//	buttonActive = false;

	//buttonActive = isOnButton(rx, ry, advanceButtonPos.x, advanceButtonPos.y, advanceButtonDim.x, advanceButtonDim.y, true);
}

void keyboard( unsigned char key, int x, int y )
{
	//cerr << "Key " << key << " int " << int(key) << "\n";


	// POLHEMUS CALIBRATION
	if (key == ' ')
	{
		settings->polhemusOrigin = polhemus->getPosition();
		std::cout << "New calibrated Polhemus origin is at ( " << settings->polhemusOrigin.x << ", " << settings->polhemusOrigin.y << ", " << settings->polhemusOrigin.z << " )" << std::endl;
		polhemus->calibrate();
	}

	// DISABLE KEYBOARD WHEN NOT IN FREE MODE
	if (settings->study->modeRestriction != StudyManager::NONE) return;

	// BEGIN STUDY
	if (key == 13)
	{
		cursorOnMouse = false;
		settings->study->begin();
	}

	// QUIT
	if(key == 'q') glutLeaveMainLoop();

	if (key == 'f') cosmo->generateFilament();
	if (key == 'm') settings->study->toggleMotion();
	if (key == 's') settings->study->toggleStereo();
	if (key == 'o') cosmo->toggleShowOscillationAxis();
	

	if (key == '\\') cursorOnMouse = !cursorOnMouse;


	//if(key == ',') { 
	//	scale = scale*0.90f; 
	//	std::cout << "scale: "<< (int) scale *100 << std::endl;
	//}
	//if(key == '.') { 
	//	scale = scale*1.10f; 
	//	std::cout << "scale: "<< (int) scale *100 << std::endl;
	//}

	//if(key == '[') touchManager->setPantoHand(false);
	//if(key == ']') touchManager->setPantoHand(true);

	//float dx = -sin(rotY*3.141592f/180.0f);
	//float dz = cos(rotY*3.141592f/180.0f);
	//if(key == 'a') { cow.z += 0.5f*dz/scale; cow.x += 0.5f*dx/scale;};
	//if(key == 'z') { cow.z -= 0.5f*dz/scale; cow.x -= 0.5f*dx/scale;};


	//if (key == 'v')
	//{
	//	velocity = !velocity;
	//	cosmo->setVelocityMode(velocity);
	//}
		

	//if (key == '/') { cosmo->toggleTrailsMode(); }
/*
	if(key == '1') cosmo = vCosmo.at(0);
	if(key == '2') cosmo = vCosmo.at(1);
	if(key == '3') cosmo = vCosmo.at(2);
	if(key == '4') cosmo = vCosmo.at(3);
	if(key == '5') cosmo = vCosmo.at(4);
	//if(key == '6') cosmo = vCosmo.at(5);
	if(key == '7') cosmo = vCosmo.at(6);
	if(key == '8') cosmo = vCosmo.at(7);
	//if(key == '9') cosmo = vCosmo.at(8);
	if(key == '0') cosmo = vCosmo.at(9);
*/
	//if(key == 'w') { cosmo->resample(100000); }
	//if(key == 'e') { cosmo->resample(100000); }
	//if(key == 'r') { cosmo->resample(1000000); }

	// LENS TRANSITION
	//if (key == 't') { 
	//	//settings->transitionOnLensExit = !settings->transitionOnLensExit;
	//	//std::cout << "Transition on lens exit set to " << settings->transitionOnLensExit << std::endl; 
	//	std::cout << "Transition on lens exit is unavailable in this version" << std::endl;
	//}

/*
	if(key == 'i') cosmo->radius *= 0.95;
	if(key == 'o') cosmo->radius *= 1.05;
*/
}

//-------------------------------------------------------------------------------
// Special key event callbacks
void specialFunction(int glutKey, int mouseX, int mouseY)
{
	if (settings->study->modeRestriction != StudyManager::NONE) return;

	if (glutKey == GLUT_KEY_PAGE_UP)
		cosmo->setMovableRotationAxisScale(cosmo->getMovableRotationAxisScale() * 1.1f);
	if (glutKey == GLUT_KEY_PAGE_DOWN)
		cosmo->setMovableRotationAxisScale(cosmo->getMovableRotationAxisScale() * 0.9f);

	if (glutKey == GLUT_KEY_F12) settings->study->snapshotTGA("snapshot");


	if (glutKey == GLUT_KEY_RIGHT)
	{
		if (cosmo->getLensType() == Cosmo::Lens::CYLINDER_VELOCITY)
			cosmo->setLensType(Cosmo::Lens::SPHERE_POINTS);
		else
			cosmo->setLensType(static_cast<Cosmo::Lens>(((int)cosmo->getLensType()) + 1));
	}
	if (glutKey == GLUT_KEY_LEFT)
	{
		if (cosmo->getLensType() == Cosmo::Lens::SPHERE_POINTS)
			cosmo->setLensType(Cosmo::Lens::CYLINDER_VELOCITY);
		else
			cosmo->setLensType(static_cast<Cosmo::Lens>(((int)cosmo->getLensType()) - 1));
	}
}


void reshape(int w, int h)
{ 
	aspect = (float) h / (float) w;

	//std::cout << "aspect: " << aspect << std::endl;

	winWidth = w, winHeight = h;
	
	advanceButtonPos = glm::vec2(winWidth / 2.f, winHeight / 2.f);

	glViewport(0,0,winWidth,winHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum( VP_LEFT, VP_RIGHT, aspect*VP_BOTTOM, aspect*VP_TOP, NEAR_CP, FAR);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); 
}

void reset_values()
{
	timer = 0;
	holdCow = glm::vec3(0.f);
	xeye = 0.0f;
	cow = glm::vec3(0.f);
	rotX = rotY = 0.0;
	transitionVector = glm::vec3(0.f, 0.f, 0.f);

	transTimer = -1;
	//winWidth = 1024; winHeight = 1024;
	scale = 0.2f;

	settings->worldDepths[0] = -75.f * scale;
	settings->worldDepths[1] = 10.f - 0.000001f;
}

//This is the GLUT initialization function
void init(std::string name, bool isRightHanded)
{
	leftMouseDown = false;
	rightMouseDown = false;

	// trial advance buttons in center of screen
	advanceButtonPos = glm::vec2(glutGet(GLUT_WINDOW_WIDTH) / 2.f, glutGet(GLUT_WINDOW_HEIGHT) / 2.f);

	srand(time(NULL));

	settings = new Settings();	
	settings->study->init(cosmo, name, isRightHanded, 3, 5);

	touchManager = new TouchManager(settings);
	int err_code = touchManager->Init();
	if (err_code != PQMTE_SUCCESS)
	{
		printf("ERROR initializing touch manager!\n");
	}

	polhemus = Polhemus::getInstance();

	glClearColor(MAIN_BACKGROUND_COLOR, 1.0);
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char *argv[])
{
	std::string name;
	std::cout << "Enter subject's name: ";
	std::cin >> name;

	std::string handedness;
	while (!(handedness == "L" || handedness == "R"))
	{
		std::cout << "Enter subject's handedness (L or R): ";
		std::cin >> handedness;
		std::transform(handedness.begin(), handedness.end(), handedness.begin(), ::toupper);
	}

	generate_theta();
	
	glutInit(&argc, argv);

   #ifdef __APPLE__
        glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    #else
        if(QUAD_BUFFER) { glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_STEREO);}
        else            { glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL); }
    #endif

	glutInitWindowSize( winWidth, winHeight );
    glutInitWindowPosition( 200, 100 );  
    mainWindow = glutCreateWindow("Pantograph");
    glutSetWindow(mainWindow);
    glutFullScreen();
	cosmo = new Cosmo();
	init(name, (handedness == "R" ? true : false));

	//---------------------------------------------------------------------------
    // initialize Open GL Extension library
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { fprintf(stderr, "Error: %s\n", glewGetErrorString(err)); }
	

	#ifdef __APPLE__
        ;
    #else
        if(QUAD_BUFFER) { if(wglSwapIntervalEXT != NULL) wglSwapIntervalEXT(2); }
        else            { if(wglSwapIntervalEXT != NULL) wglSwapIntervalEXT(1); }
    #endif

    if(QUAD_BUFFER) { glutDisplayFunc( redraw_stereo ); glutIdleFunc( redraw_stereo ); }
    else { glutDisplayFunc( redraw ); glutIdleFunc( redraw ); }

	reset_values();

    
	cosmo->read( inputFiles[0] );
	cosmo->resample(50000);
	cosmo->setScale(scale);
	cosmo->setBrightness(0.2f);
	cosmo->setLensBrightnessRange(0.95f, 0.1f);
	cosmo->setLensSize(5.f);
	cosmo->setAxisLensSize(5.f);
	cosmo->setMovableRotationCenter(0.f, 0.f, 0.f);
	cosmo->setMovableRotationAxis(0.f, 1.f, 0.f);
	cosmo->setMovableRotationAngle(1.f);
	cosmo->setMovableRotationAxisScale(20.f);
	cosmo->setVelocityMode(false);
	cosmo->generateFilament();
		    
    //---------------------------------------------------------------------------
    std::cout << " -------------------------------" << std::endl;
    printf(" Vendor: %s\n"           , glGetString(GL_VENDOR));
    printf(" Renderer: %s\n"         , glGetString(GL_RENDERER));
    printf(" GL Version = %s\n"      , glGetString(GL_VERSION));
    printf(" GLSL Version = %s\n"    , glGetString(GL_SHADING_LANGUAGE_VERSION));
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // directly redirect GLUT events to AntTweakBar
    glutMotionFunc(motion);
	glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseButton);
    glutSpecialFunc(specialFunction);
	glutReshapeFunc(reshape);

	glutMainLoop();
	return 0;
}
