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
#define OSC_ANGLE 2.f
#define REFRESH 60
Stopwatch *aclock;

Settings* settings;

TouchManager* touchManager;

float rx,ry;
//float rw, rh;
float mXscreen = 0.0f ,mYscreen = 0.0f;  // mouse x in screen coords.
float holdCowX = 0.0f, holdCowY = 0.0f, holdCowZ = 0.0f; // saves CoW for transitioning
float cowX = 0.0f,cowY= 0.0f, cowZ= 0.0f; // the CoW relativ to the data spaces
float xeye = 0.0f, yeye= 0.0f, zeye = 0.0f; // the offsets calculated for a move on mouse down

float rotX,rotY; // change the rotation about the cow
float holdMx, holdMy;

bool leftMouseDown;
bool rightMouseDown;
float dragX, dragY; // used for rotating the universe
float rotation[500];
int timer;
float winWidth = 1024;
float winHeight = 768; // screen dimensions
int rt; // rotation timer
bool startStop;
bool autoRefresh = false;

//---------------------------------------------------------------------------- 
///MOVE SPEED NOW A FRACTION OF BBOX LENGTH
#define DEFAULTMOVESPEED 10

int processMouseClick;

bool takeSnapshot;

bool mouseLeftNav;
bool mouseRightNav;
int lastMouseNavX;
int lastMouseNavY;

bool zoomIn;
bool zooming;

float moveSpeed;
//----------------------------------------------------------------------------
glm::vec3 focalCenter = glm::vec3(0.0f, 0.0f, 0.0f);
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

#define DIM_FRAMES 40
float dimness = 0.f;


void transition()  // used to translate smoothly
{	
	float p = float(transTimer)/TRANS_FRAMES;

	if (transTimer >= 0)
	{
		cowX = holdCowX + xeye*(1.f-p);
		cowY = holdCowY + yeye*(1.f-p);
		cowZ = holdCowZ + zeye*(1.f-p);
		--transTimer;
	}
	if(transTimer == 0)
	{
		cout << "transition done" << endl;
		holdCowX = cowX;
		holdCowY = cowY;
		holdCowZ = cowZ;
		xeye = yeye = zeye = 0;
	}

}

void calculateTransition(float x, float y)
{
	xeye = settings->currentlySelectedPoint[0];
	yeye = settings->currentlySelectedPoint[1];
	zeye = settings->currentlySelectedPoint[2];

	holdCowX = 0.f;
	holdCowY = 0.f;
	holdCowZ = 0.f;

	transTimer = TRANS_FRAMES;

	settings->transitionRequested = false;
}

void dim()
{
	float t = (float)settings->dimTimer / (float) DIM_FRAMES;

	if (settings->dimTimer >= 0 && settings->dimTimer <= DIM_FRAMES)
	{
		dimness = 1.f - t;

		if (settings->dimming)
			--settings->dimTimer;
		else
			++settings->dimTimer;
	}

	if (settings->dimTimer < 0 && settings->dimming)
	{
		dimness = 1.f;
		settings->dimming = false;
	}

	if (settings->dimTimer > DIM_FRAMES)
	{
		dimness = 0.f;
		settings->dimTimer = -1;
	}

	cosmo->setDimness(dimness);
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
		cosmo->setVelocityMode(true);
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

		//float depthHere = dataset->getDepthAt(p1x, p1y);
		float depthHere = cosmo->getMaxDimension();
		if (depthHere != 0)
		{
			//store model depth
			settings->positioningModelCoords[2] = -depthHere;
			settings->positioningModelCoords[3] = depthHere;
		}
		else
		{
			settings->positioningModelCoords[2] = -1;
			settings->positioningModelCoords[3] = -1;
		}
	}//if need to update active positioning
	else
	{
		// reset positioning values
		settings->positioningModelCoords[0] = -1;
		settings->positioningModelCoords[1] = -1;
		settings->positioningModelCoords[2] = -1;

		// turn off lens mode
		cosmo->setLensMode(false);
		cosmo->setVelocityMode(false);
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}//end processPendingInteractions()

void updateLens()
{
	// get data object's model-view matrix and invert it
	double mv[16];
	cosmo->getMV(mv);
	glm::mat4 MVinv = glm::inverse( glm::make_mat4( mv ) );

	// get current world-space pantograph point coord
	glm::vec4 pt = glm::vec4(settings->currentlySelectedPoint[0],
		settings->currentlySelectedPoint[1],
		settings->currentlySelectedPoint[2],
		1.f);

	// get model-space coordinate for pantograph point and set it
	glm::vec4 newpt = MVinv * pt;

	cosmo->setLensPosition(newpt.x, newpt.y, newpt.z);
	cosmo->setMovableRotationCenter(newpt.x, newpt.y, newpt.z);
}

void perRenderUpdates()
{
	//mouse zooming
	//zoom();

	if (settings->transitionRequested) calculateTransition(settings->currentlySelectedPoint[0], settings->currentlySelectedPoint[1]);

	transition();

	dim();

	rt = timer % 120; // 60 Hz on 60 Hz machine

	focalCenter = glm::vec3(cowX*VP_LEFT, cowY*VP_BOTTOM, -cowZ*(NEAR_CP - 10));

	if (startStop)
	{
		cosmo->setMovableRotationAngle(rotation[rt] + rotY + dragX);
		cosmo->setMovableRotationAxis(settings->rotationAxis);
	}
	else cosmo->setRotation(rotY + dragX, settings->rotationAxis);

	cosmo->setPosition(cowX, cowY, cowZ);

	touchManager->perRenderUpdate();

	processPendingInteractions();

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
	
	// Draw world-space y-axis
	//glBegin(GL_LINES);
	//	glVertex3f(0.0,  20.0, 0.0);
	//	glVertex3f(0.0,   1.0, 0.0);
	//	glVertex3f(0.0,  -1.0, 0.0);
	//	glVertex3f(0.0, -20.0, 0.0);
	//glEnd();

	// render cosmos point cloud
	cosmo->render();
		
	//draw active positioning pole:
	if (settings->positioningModelCoords[2] != -1)
	{
		glLineWidth(2);
		glColor4f(0.8, 0.8, 0.95, 1.0);
		glBegin(GL_LINES);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], 10);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2] + 1.f);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2]);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], -10);
		glEnd();

		glColor4f(1.0, 1.0, 0.25, 1.0);
		glPointSize(6);
		glBegin(GL_POINTS);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2]);
		glEnd();
		
		// draw axes
		glm::vec3 yAxis = normalize(glm::vec3(settings->finger2modelCoords[0] - settings->finger1modelCoords[0],
			settings->finger2modelCoords[1] - settings->finger1modelCoords[1],
			settings->finger2modelCoords[2] - settings->finger1modelCoords[2]) );

		settings->rotationAxis = yAxis;

		glm::vec3 xAxis = normalize( glm::cross(yAxis, glm::vec3(0.f, 0.f, 1.f)) );

		glLineWidth(1);
		glBegin(GL_LINES);
			glColor4f(1.f, 0.f, 0.f, 1.f);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2]);
			glVertex3f(settings->positioningModelCoords[0] + xAxis.x, settings->positioningModelCoords[1] + xAxis.y, settings->currentlySelectedPoint[2] + xAxis.z);
			glColor4f(0.f, 1.f, 0.f, 1.f);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2]);
			glVertex3f(settings->positioningModelCoords[0] + yAxis.x, settings->positioningModelCoords[1] + yAxis.y, settings->currentlySelectedPoint[2] + yAxis.z);
			glColor4f(0.f, 0.f, 1.f, 1.f);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2]);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2] + 1.f);
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
		holdMy = float(y);

		mXscreen = (VP_RIGHT - VP_LEFT) * (float(x) / winWidth - 0.5f);
		mYscreen = aspect*(VP_TOP - VP_BOTTOM) * (float(y) / winHeight - 0.5f);

		float roty = atan2(mXscreen, 20.0f);
		float rotx = atan2(mYscreen, 20.0f);

		cout << "(x,y)               = ( " << x << ", " << y << " )" << endl;
		cout << "(mXscreen,mYscreen) = ( " << mXscreen << ", " << mYscreen << " )" << endl;
		cout << "(rotx,roty)         = ( " << rotx << ", " << roty << " )" << endl;
		cout << endl;
		cout << endl;

		// calculate the translation 
		xeye = -COW_Z *   sin(roty) / scale;
		zeye = COW_Z * (cos(roty) - 1.f) / scale;
		yeye = COW_Z *   sin(rotx) / scale;

		// now rotate by the current view direction
		float dz = sin(rotY * M_PI / 180.0f);
		float dx = cos(rotY * M_PI / 180.0f);

		xeye = dx * xeye - dz * zeye;
		zeye = dz * xeye + dx * zeye;

		transTimer = TRANS_FRAMES;
		// update the view angle for off center targets
		rotY += roty * 180.0f / M_PI;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{

		rightMouseDown = true;
		holdMx = rx;
		holdMy = ry;
		
		cerr << "Rot Y Down " << rotY << "\n";
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		rightMouseDown = false;
		
		rotY = rotY + dragX;	
		//cerr << "Rot Up Y " << rotY << "\n";
		dragX = dragY = 0.0;

	}
	if ((button == 3) || (button == 4)) // It's a wheel event
   {

	   // find rotations given current universe Rotation
	   float dx, dz;
	   dx = -sin(rotY * M_PI / 180.0f);
	   dz = cos(rotY * M_PI / 180.0f);
       // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
       if (state == GLUT_UP) return 0; // Disregard redundant GLUT_UP events
       //printf("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", x, y);
	   if(button == 3)
	   {
		   cowZ = cowZ - 0.5f*dz/scale; 
		   cowX = cowX - 0.5f*dx/scale; 
	   }
	   else 
	   {
			cowZ = cowZ + 0.5f*dz/scale; 
			cowX = cowX + 0.5f*dx/scale; 
	   }

	   holdCowZ = cowZ;
	   holdCowX = cowX;
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

	if(rightMouseDown)// used for rotating with the right mouse button
	{
		dragX = (rx - holdMx)*0.1f;
		//cerr << "Rot Y Drag " << dragX << " " << rotY + dragX << "\n";
	}
}

void reset_values()
{
	timer = 0;
	holdCowX = holdCowY = holdCowZ = 0.0;
	xeye = 0.0f;
	cowX = cowY = cowZ = 0.0;
	rotX = rotY = 0.0;

	startStop = true;
	transTimer = -1;
	//winWidth = 1024; winHeight = 1024;
	scale = 1.f;
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

	if(key == '[') cowZ += 0.1f*scale;
	if(key == ']') cowZ -= 0.1f*scale;

	float dx = -sin(rotY*3.141592f/180.0f);
	float dz = cos(rotY*3.141592f/180.0f);
	if(key == 'a') { cowZ += 0.5f*dz/scale; cowX += 0.5f*dx/scale;};
	if(key == 'z') { cowZ -= 0.5f*dz/scale; cowX -= 0.5f*dx/scale;};

	if(key == '0') startStop = !startStop;

	if(key == 'g') cosmo->resample(100000);

	if (key == 'p')	point = !point;
	if (key == 'v')
	{
		velocity = !velocity;
		cosmo->setVelocityMode(velocity);
	}

	if(key == ' ') { reset_values(); cosmo->resample(100000); }
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

	//particleSystem = new ParticleSystem(dataset, settings);
	//panelMan->addParticleSystemPanel(particleSystem);
	//pathPanel = panelMan->addAUVPathPanel(dataset);

	takeSnapshot = false;
	processMouseClick = 0;

	lastMouseNavX = 0;
	lastMouseNavY = 0;
	mouseLeftNav = false;
	mouseRightNav = false;
	zoomIn = true;
	zooming = false;
	moveSpeed = DEFAULTMOVESPEED;


	glClearColor(MAIN_BACKGROUND_COLOR, 1.0);
	//glClearDepth(0.0);
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_NORMALIZE);
	//glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//	glShadeModel(GL_SMOOTH);
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
	cosmo->resample(100000);
	cosmo->setScale(0.2f);
	cosmo->setLensSize(5.f);
	cosmo->setMovableRotationAngle(1.f);

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
