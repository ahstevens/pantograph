#include "Cosmo.h"
//----------------------------------------------------------------------------

#include <GL/glut.h>
#include <iostream>
#include <math.h>
#include "Stopwatch.h"
#include <AntTweakBar.h>
#include "Settings.h"
#include "ColorsAndSizes.h"
#include "TouchManager.h"

/*
//--------------------------------------------------
float frustumLeft   =-29.65f, frustumRight  = 29.65f;    // 59.3
float frustumBottom =-16.8f,  frustumTop    = 16.8f;     // 33.6
float frustumNear   = 65.0f,  frustumFar    = 300.0f;   
float eyeOffset     = 3.25f;
float cowZ          = 30.f;
//--------------------------------------------------
*/

#define VP_LEFT		-10.0f
#define VP_RIGHT	10.0f
#define VP_BOTTOM	-10.0f
#define VP_TOP		10.0f
#define NEAR_CP		20.0f
#define COW_Z 		30.0f
#define FAR 		100.0f 
using namespace std;
#define OSC_ANGLE 10.f
#define REFRESH 60
Stopwatch *aclock;

Settings* settings;

TouchManager* touchManager;

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

bool leftMouseDown, rightMouseDown, middleMouseDown;

float dragX, dragY; // used for rotating the universe
float rotation[500];
int timer;
float winWidth = 1024;
float winHeight = 768; // screen dimensions
int rt; // rotation timer
bool startStop;
bool autoRefresh = false;

//---------------------------------------------------------------------------- 
//----------------------------------------------------------------------------
float aspect = 1.0f;
float scale;
float vectorScale = 1.0f;
TwBar *bar;
bool isBarVisible = false;  // for dragging
GLint mainWindow;
bool rotating = true;
bool alphaBlend = true;
bool stereo = true;

#define QUAD_BUFFER true
//float eyeOffset     = 3.25f * 10.0f/16.8f;
const float eyeOffset     = 0.5f;

//---------------------------------------------------------------------------- 

std::string inputFiles[] = { "haloes_data/256cosmo.0.7.cosmo" };
//std::string inputFiles[] = { "/mnt/main/haloes_data/256cosmo.0.7.cosmo" };

Cosmo *cosmo;
bool point = true;
bool velocity = false;
std::vector<Cosmo*> vCosmo;
//----------------------------------------------------------------------------
#define TRANS_FRAMES 40
int transTimer;


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

void processPendingInteractions()
{
	//printf("Processing Mouse Click\n");
	//draw a ground plane at height zero to fill depth buffer so we can get selection depth from it
	glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glVertex3f(-100, 100, 10);
		glVertex3f(-100, -100, 10);
		glVertex3f(100, -100, 10);
		glVertex3f(100, 100, 10);
	glEnd();

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
		// get finger 1 model space coords
		glReadPixels(settings->finger1sX, settings->finger1sY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(settings->finger1sX, settings->finger1sY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		float p1x, p1y, p1z;
		p1x = (float)posX;
		p1y = (float)posY;
		p1z = (float)posZ;

		settings->finger1modelCoords[0] = p1x;
		settings->finger1modelCoords[1] = p1y;
		settings->finger1modelCoords[2] = p1z;

		// get finger 2 model space coords
		glReadPixels(settings->finger2sX, settings->finger2sY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(settings->finger2sX, settings->finger2sY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		p1x = (float)posX;
		p1y = (float)posY;
		p1z = (float)posZ;

		settings->finger2modelCoords[0] = p1x;
		settings->finger2modelCoords[1] = p1y;
		settings->finger2modelCoords[2] = p1z;

		// activate lens mode
		cosmo->setLensMode(true);
		//cosmo->setAxisMode(true);

		// draw axes
		glm::vec3 yAxis = normalize(glm::vec3(settings->finger2modelCoords[0] - settings->finger1modelCoords[0],
			settings->finger2modelCoords[1] - settings->finger1modelCoords[1],
			settings->finger2modelCoords[2] - settings->finger1modelCoords[2]));

		cosmo->setMovableRotationAxis(yAxis);
	}

	//if actively positioning finger, maintain corresponding model coordinates
	if (settings->positioningXYFingerLocation[0] != -1 && settings->positioningZFingerLocation[2] != -1)
	{
		glReadPixels(settings->positioningXYFingerLocation[0], settings->positioningXYFingerLocation[1], 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(settings->positioningXYFingerLocation[0], settings->positioningXYFingerLocation[1], winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		float p1x, p1y, p1z;
		p1x = (float)posX;
		p1y = (float)posY;
		p1z = (float)posZ;

		settings->positioningModelCoords[0] = p1x;
		settings->positioningModelCoords[1] = p1y;

	}//if need to update active positioning
	else
	{
		// reset positioning values
		settings->positioningModelCoords[0] = -1;
		settings->positioningModelCoords[1] = -1;
		settings->positioningModelCoords[2] = -1;

		// turn off lens mode
		if (!settings->mouseMode) cosmo->setLensMode(false);
		//if(!settings->mouseMode) cosmo->setAxisMode(false);
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}//end processPendingInteractions()

void updateLens()
{
	// get data object's model-view matrix and invert it
	double mv[16];
	cosmo->getMV(mv);
	glm::mat4 MVinv = glm::inverse( glm::make_mat4( mv ) );

	// get model-space coordinate for pantograph point and set it
	glm::vec4 newpt = MVinv * glm::vec4(settings->currentlySelectedPoint[0],
		settings->currentlySelectedPoint[1],
		settings->currentlySelectedPoint[2],
		1.f);
	
	cosmo->setLensPosition(newpt.x, newpt.y, newpt.z);
	cosmo->setMovableRotationCenter(newpt.x, newpt.y, newpt.z);
}

void perRenderUpdates()
{
	if (settings->transitionRequested) 
		calculateTransition(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1]);

	if (settings->dimmingRequested)
	{
		cosmo->requestDimming();
		settings->dimmingRequested = false;
	}

	transition();

	rt = timer % 120; // 60 Hz on 60 Hz machine

	if (startStop) cosmo->setMovableRotationAngle(rotation[rt]);
	else cosmo->setRotationAngle(rotY + dragX);
	
	// adjust panto depth so that back of dataset always accessible
	settings->worldDepths[0] = (-cosmo->getMaxDistance() - cow.z)*scale;

	touchManager->perRenderUpdate();

	processPendingInteractions();

	if (settings->mouseMode)
	{
		settings->currentlySelectedPoint[0] = mXscreen;
		settings->currentlySelectedPoint[1] = mYscreen;
		settings->currentlySelectedPoint[2] = mZ;
	}

	if (settings->pantographMode || settings->mouseMode)
		updateLens();
}

void drawScene(int eye) //0=left or mono, 1=right
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// translate from scene	origin 10 units behind near clipping plane to each eye
	glTranslatef(!eye ? eyeOffset : -eyeOffset, 0.0, -NEAR_CP - 10.0); // center of universe offset..
	settings->worldDepths[1] = 10.f - 0.001f;

	// render cosmos point cloud
	cosmo->render();
		
	//draw active positioning pole:
	//if (settings->positioningModelCoords[2] != -1)
	if (settings->pantographMode || settings->mouseMode)
	{
		glLineWidth(2);
		glColor4f(0.8, 0.8, 0.95, 1.0);
		glBegin(GL_LINES);
			glVertex3f(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->worldDepths[1]);
			glVertex3f(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->worldDepths[0]);
		glEnd();

		glColor4f(1.0, 1.0, 0.25, 1.0);
		glPointSize(6);
		glBegin(GL_POINTS);
			glVertex3f(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1], settings->currentlySelectedPoint[2]);
		glEnd();
	}

	touchManager->draw3D();

	if (isBarVisible) { TwSetCurrentWindow(mainWindow); TwDraw(); }
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
	sprintf(buffer, "FPS: %d", lastfps);
	glColor3f(1, 1, 1);
	glLineWidth(1);
	drawStrokeLabel3D(glutGet(GLUT_WINDOW_WIDTH) - 50, 2, 0, 0.075, buffer);

	glColor3f(1, 1, 1);
	glLineWidth(1);
	drawStrokeLabel3D(glutGet(GLUT_WINDOW_WIDTH) - 150, 2, 0, 0.075, buffer);
	
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
	glFrustum(VP_LEFT + eyeOffset,	VP_RIGHT + eyeOffset,
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

  	//--------------------------------------------------
	// LEFT EYE = 0, RIGHT EYE = 1
	for (int eye = 0; eye < 2; ++eye)
	{
		glDrawBuffer(!eye ? GL_BACK_LEFT : GL_BACK_RIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//--------------------------------------------------
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(VP_LEFT + (!eye ? eyeOffset : -eyeOffset),
			VP_RIGHT + (!eye ? eyeOffset : -eyeOffset),
			aspect*VP_BOTTOM, aspect*VP_TOP, NEAR_CP, FAR);
		//--------------------------------------------------
		drawScene(eye);
		drawOverlay();
	}

	//--------------------------------------------------
	glutSwapBuffers();

	++timer;
}

static int mouseButton(int button, int state, int x, int y)
{
	if(isBarVisible) {
        TwSetCurrentWindow(glutGetWindow());
        return TwEventMouseButtonGLUT(button, state, x, y);
    }

	rx = float(x); 
	ry = float(winHeight - y);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		leftMouseDown = true;
		holdMy = float(y);

		if (settings->mouseMode)
		{
			mXscreen = (VP_RIGHT - VP_LEFT) * (rx / winWidth - 0.5f);
			mYscreen = aspect*(VP_TOP - VP_BOTTOM) * (ry / winHeight - 0.5f);
		}		
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		leftMouseDown = false;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		rightMouseDown = true;
		holdMx = rx;
		holdMy = ry;

		settings->mouseMode = !settings->mouseMode;

		if (settings->mouseMode)
		{
			settings->dimmingRequested = true;
			//cosmo->setAxisMode(true);
			cosmo->setLensMode(true);
		}
		else
		{
			settings->dimmingRequested = false;
			//cosmo->setAxisMode(false);
			cosmo->setLensMode(false);
			settings->transitionRequested = true;
		}
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
		if (!leftMouseDown)
		{
			holdRotationAxis = cosmo->getMovableRotationAxis();
			holdMx = rx;
			holdMy = ry;
		}
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
	   if (settings->mouseMode)
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
static int motion(int x, int y) 
{
	if(isBarVisible) {
        TwSetCurrentWindow(glutGetWindow());
        return TwEventMouseMotionGLUT(x,y);
    }

	rx = float(x); ry = float(winHeight - y);

	// LEFT MOUSE DOWN
	if (settings->mouseMode && leftMouseDown)
	{
		mXscreen = (VP_RIGHT - VP_LEFT) * (rx / winWidth - 0.5f);
		mYscreen = aspect*(VP_TOP - VP_BOTTOM) * (ry / winHeight - 0.5f);
	}

	if (settings->mouseMode && middleMouseDown && !leftMouseDown)
	{
		float h_displacement = (float) (holdMx - rx) / 10.f;
		float v_displacement = (float)(holdMy - ry) / 10.f;

		glm::vec3 newAxis = glm::vec3( glm::rotate(glm::radians(h_displacement), glm::vec3(0.f, 0.f, 1.f)) * glm::vec4(holdRotationAxis, 0.f));

		cosmo->setMovableRotationAxis(newAxis);
	}
}

void reset_values()
{
	timer = 0;
	holdCow = glm::vec3(0.f);
	xeye = 0.0f;
	cow = glm::vec3(0.f);
	rotX = rotY = 0.0;
	transitionVector = glm::vec3(0.f, 0.f, 0.f);

	startStop = true;
	transTimer = -1;
	//winWidth = 1024; winHeight = 1024;
	scale = 0.2f;
	vectorScale = 1.0f;
}

int zoom = 1;
static int keyboard( unsigned char key, int x, int y )
{
	cerr << "Key " << key << " int " << int(key) << "\n";

	if(key == 'q') exit(0);
	if(key == ',') { 
		scale = scale*0.90f; 
		if (autoRefresh) {	
			if(zoom%10 == 0) cosmo->resample(100000); zoom++;
		}
		cout << "scale: "<< (int) scale *100 << endl;
		//if( ((int) (scale * 100)) %10 == 0) cosmo->resample(100000);
	}
	if(key == '.') { 
		scale = scale*1.10f; 
		if (autoRefresh) {		
			if(zoom%10 == 0) cosmo->resample(100000); zoom--;
		}
		cout << "scale: "<< (int) scale *100 << endl;
		//if( ((int) (scale * 100)) %10 == 0) cosmo->resample(100000);
	}

	if(key == '[') cow.z += 0.1f*scale;
	if(key == ']') cow.z -= 0.1f*scale;

	float dx = -sin(rotY*3.141592f/180.0f);
	float dz = cos(rotY*3.141592f/180.0f);
	if(key == 'a') { cow.z += 0.5f*dz/scale; cow.x += 0.5f*dx/scale;};
	if(key == 'z') { cow.z -= 0.5f*dz/scale; cow.x -= 0.5f*dx/scale;};

	if(key == '0') startStop = !startStop;

	if(key == 'g') cosmo->resample(100000);

	if (key == 'p')	point = !point;
	if (key == 'v')
	{
		velocity = !velocity;
		cosmo->setVelocityMode(velocity);
	}

	if(key == ' ') { reset_values(); cosmo->resample(100000); }

	if (key == '/') { cosmo->toggleTrailsMode(); }
	if (key == '\'') { cosmo->toggleShowOscillationAxis(); }
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
	if(key == 'w') { cosmo->resample(100000); }
	if(key == 'e') { cosmo->resample(100000); }
	if(key == 'r') { cosmo->resample(1000000); }
	if(key == 't') { cosmo->resample(1500000); }
	if(key == 'y') { cosmo->resample(2500000); }
/*
	if(key == 'i') cosmo->radius *= 0.95;
	if(key == 'o') cosmo->radius *= 1.05;
*/
    if(key == 'm') isBarVisible = !isBarVisible;



	TwSetCurrentWindow(glutGetWindow());
    return TwEventKeyboardGLUT(key, x, y);  
}


void reshape(int w, int h)
{ 
	aspect = (float) h / (float) w;

	//std::cout << "aspect: " << aspect << std::endl;

	winWidth = w, winHeight = h;
	glViewport(0,0,winWidth,winHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum( VP_LEFT, VP_RIGHT, aspect*VP_BOTTOM, aspect*VP_TOP, NEAR_CP, FAR);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); 

	// Send the new window size to AntTweakBar
    TwSetCurrentWindow(mainWindow);
    TwWindowSize(w, h);

}

//-------------------------------------------------------------------------------
// Special key event callbacks
int specialFunction(int glutKey, int mouseX, int mouseY) 
{
	if (glutKey == GLUT_KEY_PAGE_UP)
		cosmo->setMovableRotationAxisScale(cosmo->getMovableRotationAxisScale() * 1.1f);
	if (glutKey == GLUT_KEY_PAGE_DOWN)
		cosmo->setMovableRotationAxisScale(cosmo->getMovableRotationAxisScale() * 0.9f);

	if (glutKey == GLUT_KEY_RIGHT)
	{		
		if (cosmo->getLensType() == Cosmo::Lens::CYLINDER_VELOCITY)
			cosmo->setLensType(Cosmo::Lens::SPHERE_POINTS);
		else
			cosmo->setLensType(static_cast<Cosmo::Lens>( ( (int)cosmo->getLensType() ) + 1) );
	}
	if (glutKey == GLUT_KEY_LEFT)
	{
		if (cosmo->getLensType() == Cosmo::Lens::SPHERE_POINTS)
			cosmo->setLensType(Cosmo::Lens::CYLINDER_VELOCITY);
		else
			cosmo->setLensType(static_cast<Cosmo::Lens>(((int)cosmo->getLensType()) - 1));
	}

    TwSetCurrentWindow(glutGetWindow());
    return TwEventSpecialGLUT(glutKey,mouseX,mouseY);   
}

//-------------------------------------------------------------------------------
// tweak methods
	void TW_CALL set_rotate(const void *value, void *clientData)
	{ 
	    rotating = *(const bool *)value;
	}

	void TW_CALL get_rotate(void *value, void *clientData)
	{ 
	    *(bool *)value = rotating;  // for instance
	}

	void TW_CALL set_velocity(const void *value, void *clientData)
	{ 
	    velocity = *(const bool *)value;
	}

	void TW_CALL get_velocity(void *value, void *clientData)
	{ 
	    *(bool *)value = velocity;  // for instance
	}

	void TW_CALL set_auto_refresh(const void *value, void *clientData)
	{ 
		autoRefresh = *(const bool *)value;
	}

	void TW_CALL get_auto_refresh(void *value, void *clientData)
	{ 
	    *(bool *)value = autoRefresh;  // for instance
	}

	void TW_CALL set_alpha(const void *value, void *clientData)
	{ 
	    alphaBlend = *(const bool *)value;

	    if(alphaBlend) {
	        glEnable(GL_BLEND);
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );     
	    } 
	    else { 
	      glDisable(GL_BLEND);   
	    }
	}

	void TW_CALL get_alpha(void *value, void *clientData)
	{ 
	    *(bool *)value = alphaBlend;  // for instance

	    if(alphaBlend) {
	        glEnable(GL_BLEND);
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );     
	    } 
	    else { 
	      glDisable(GL_BLEND);   
	    } 
	}

	void TW_CALL set_stereo(const void *value, void *clientData)
	{ 
	    stereo = *(const bool *)value;
	}

	void TW_CALL get_stereo(void *value, void *clientData)
	{ 
	    *(bool *)value = stereo;  // for instance
	}
//-------------------------------------------------------------------------------

//This is the GLUT initialization function
void init(void)
{
	leftMouseDown = false;
	rightMouseDown = false;

	srand(time(NULL));
	settings = new Settings();

	touchManager = new TouchManager(settings);
	int err_code = touchManager->Init();
	if (err_code != PQMTE_SUCCESS)
	{
		printf("ERROR initializing touch manager!\n");
	}

	glClearColor(MAIN_BACKGROUND_COLOR, 1.0);
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char *argv[])
{
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
	init();

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


	aclock = new Stopwatch();
	aclock->start();

	reset_values();

    cosmo = new Cosmo();
	cosmo->read( inputFiles[0] );
	cosmo->resample(500000);
	cosmo->setScale(scale);
	cosmo->setBrightness(0.2f);
	cosmo->setLensBrightnessRange(0.9f, 0.025f);
	cosmo->setLensSize(5.f);
	cosmo->setAxisLensSize(5.f);
	cosmo->setMovableRotationCenter(0.f, 0.f, 0.f);
	cosmo->setMovableRotationAxis(0.f, 1.f, 0.f);
	cosmo->setMovableRotationAngle(1.f);
	cosmo->setMovableRotationAxisScale(20.f);
	cosmo->setVelocityMode(false);

	vCosmo.push_back(cosmo);


    //---------------------------------------------------------------------------
    // control panel
	    // Initialize AntTweakBar
	    TwInit(TW_OPENGL, NULL);
	    //TwInit(TW_OPENGL_CORE, NULL); // for core profile
	    TwSetCurrentWindow(mainWindow);

	    // Create a tweak bar
	    bar = TwNewBar("ControlPanel");
	    TwDefine(" GLOBAL help='This application demonstrates Cosmo data.' "); // Message added to the help bar.
		TwDefine(" GLOBAL iconalign=horizontal "); // icons will be aligned horizontally
		//TwDefine(" GLOBAL fontscaling=3 "); // double the size of all fonts
		TwDefine(" TW_HELP visible=false ");  // help bar is hidden

	    TwDefine(" ControlPanel size='400 400' color='96 216 224' "); // change default tweak bar size and color
		TwDefine(" ControlPanel position='50 50' "); // move bar to position (200, 40)
		TwDefine(" ControlPanel movable=false "); // mybar cannot be moved
		TwDefine(" ControlPanel resizable=false "); // mybar cannot be resized
		TwDefine(" ControlPanel label='Engine properties' ");


	    TwAddVarRW(bar, "scale", TW_TYPE_FLOAT, &scale, 
	    " min=0.0 max=100.0 step=0.1 keyIncr=> keyDecr=< help='Scale the object.' ");

	    // Add callback to toggle auto-rotate mode (callback functions are defined above).
	    TwAddVarCB(bar, "stereo", TW_TYPE_BOOL32, set_stereo, get_stereo, NULL, 
	               " label='stereo' key=S help='stereo mode on/off.' ");
	    // Add callback to toggle auto-rotate mode (callback functions are defined above).
	    TwAddVarCB(bar, "rotate", TW_TYPE_BOOL32, set_rotate, get_rotate, NULL, 
	               " label='rotating' key=R help='Toggle auto-rotate mode.' ");
	    TwAddSeparator(bar, NULL, NULL);
	    //---------------------------------------------------------------------------

	    // add another control
	    //TwAddVarRW(bar, "point size", TW_TYPE_FLOAT, &pointSize, 
	    // " min=1.0 max=5.0 step=0.1 keyIncr=+ keyDecr=- help='Point size the object.' ");
	    //TwAddSeparator(bar, NULL, NULL);

	    // add another control
	    TwAddVarRW(bar, "vector length", TW_TYPE_FLOAT, &vectorScale, 
	     " min=0.1 max=10.0 step=0.1 keyIncr=9 keyDecr=6 help='Point size the object.' ");
	    TwAddSeparator(bar, NULL, NULL);
	    //---------------------------------------------------------------------------

	    // Add callback to toggle auto-rotate mode (callback functions are defined above).
	    TwAddVarCB(bar, "alpha blending", TW_TYPE_BOOL32, set_alpha, get_alpha, NULL, 
	               " label='alpha blending' key=A help='alpha blending on/off.' ");
	    TwAddVarCB(bar, "auto refresh", TW_TYPE_BOOL32, set_auto_refresh, get_auto_refresh, NULL, 
	               " label='auto refresh' key=Z help='alpha blending on/off.' ");

	    TwAddSeparator(bar, NULL, NULL);

	    
	    //---------------------------------------------------------------------------

	    // Add callback to toggle auto-rotate mode (callback functions are defined above).
	    TwAddVarCB(bar, "velocity", TW_TYPE_BOOL32, set_velocity, get_velocity, NULL, 
	               " label='velocity' key=V help='velocity on/off' ");
    //---------------------------------------------------------------------------
    std::cout << " -------------------------------" << std::endl;
    printf(" Vendor: %s\n"           , glGetString(GL_VENDOR));
    printf(" Renderer: %s\n"         , glGetString(GL_RENDERER));
    printf(" GL Version = %s\n"      , glGetString(GL_VERSION));
    printf(" GLSL Version = %s\n"    , glGetString(GL_SHADING_LANGUAGE_VERSION));
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // directly redirect GLUT events to AntTweakBar
    glutMotionFunc((GLUTmousemotionfun) motion);
    glutKeyboardFunc((GLUTkeyboardfun) keyboard);
    glutMouseFunc((GLUTmousebuttonfun) mouseButton);
    glutSpecialFunc((GLUTspecialfun) specialFunction);
	glutReshapeFunc(reshape);

    // send the ''glutGetModifers'' function pointer to AntTweakBar
    TwGLUTModifiersFunc(glutGetModifiers);

	glutMainLoop();
	return 0;
}
