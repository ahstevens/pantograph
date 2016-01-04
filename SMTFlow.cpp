// Kinect3DViewer.cpp : Defines the entry point for the console application.
//
#include "GL/glew.h"
#include <math.h>
#include <time.h>
#include <windows.h>
#include "stdio.h"
#include "GL/glut.h"
#include "Settings.h"
#include "DrawFunctions.h"
#include "ColorsAndSizes.h"
#include "TouchManager.h"

#define DTOR            0.0174532925


//References for yaw/pitch in radians instead of degrees
#define YAW (double)settings->yaw * 3.14159 / 180
#define PITCH (double)settings->pitch * 3.14159 / 180
//90 degrees in radians
#define NINETYDEGINRAD 1.57079633

///MOVE SPEED NOW A FRACTION OF BBOX LENGTH
#define DEFAULTMOVESPEED 10

//Default camera start location and orientation
#define DEFAULTYAW 0
#define DEFAULTPITCH 0

Settings* settings;

TouchManager* touchManager;

//speed of movement
float moveSpeed;
float mouseSens;

bool mouseLeftNav;
bool mouseRightNav;
int lastMouseNavX;
int lastMouseNavY;

float lastMouseClickX;
float lastMouseClickY;

bool zoomIn;
bool zooming;

bool leftMouseDown;
bool rightMouseDown;
bool dyeMode;

int processMouseClick;

bool takeSnapshot;





//mouse zoom function
void zoom()
{
	if (zooming)
	{
		if (zoomIn)
		{
			//zoom in
			settings->camera->MoveForward(settings->camera->moveSpeed);
		}
		else
		{
			//zoom out
			settings->camera->MoveBackward(settings->camera->moveSpeed);
		}
	}
}

void perRenderUpdates()
{
	//mouse zooming
	zoom();
	
	touchManager->perRenderUpdate();	
}

void drawOverlay()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS);

	//draw 2D interface elements:
	glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

	glDisable(GL_LIGHTING);

	touchManager->draw2D();

	static float fps = 0.0f;
    static float lastTime= 0.0f;
	static int lastfps = 0;
	static char buffer[32] = {0};
	float currentTime = GetTickCount64() * 0.001f;
	++fps;
    	
	if( currentTime - lastTime > 1.0f )
    {
		lastfps = (int)fps;
		fps = 0;
		lastTime = currentTime;
    }
	sprintf(buffer, "FPS: %d", lastfps);
	glColor3f(1,1,1);
	glLineWidth(1);
	drawStrokeLabel3D(glutGet(GLUT_WINDOW_WIDTH)-50, 2, 0, 0.075, buffer);

	glColor3f(1,1,1);
	glLineWidth(1);
	drawStrokeLabel3D(glutGet(GLUT_WINDOW_WIDTH)-150, 2, 0, 0.075, buffer);

	if (dyeMode)
	{
		glColor3f(1,0,0);
		glLineWidth(2);
		drawStrokeLabel3D(5, 5, 0, 0.150, "DYE TOOL ENABLED");
	}


	glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

	//glEnable(GL_LIGHTING);

}

void drawStereoDebug()
{
	glPointSize(5);
	glBegin(GL_POINTS);
		glColor3f(1,0,0);
		glVertex3f(-100,-100,0);

		glColor3f(0,1,0);
		glVertex3f(-100,-100,-20);

		glColor3f(0,0,1);
		glVertex3f(-100,-100,-40);

		glColor3f(1,0,0);
		glVertex3f(-100,-100,-60);

		glColor3f(0,1,0);
		glVertex3f(-100,-100,-80);

		glColor3f(0,0,1);
		glVertex3f(-100,-100,-100);
	glEnd();


}

void processPendingInteractions()
{
		//printf("Processing Mouse Click\n");
		//draw a ground plane at height zero to fill depth buffer so we can get selection depth from it
		glBegin(GL_QUADS);
			glNormal3f(0,0,1);
			glVertex3f(-10000, 0, 0);
			glVertex3f(-10000, 10000, 0);
			glVertex3f(10000, 10000, 0);
			glVertex3f(10000, 0, 0);
		glEnd();

		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ = 0.0;
		GLdouble posX, posY, posZ;

		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );

		//if fingers down convert screen coords to model coords
		if (settings->finger1sX != -1 && settings->finger1sY != -1 && settings->finger2sX != -1 && settings->finger2sY != -1)
		{
			glReadPixels( settings->finger1sX, settings->finger1sY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
			gluUnProject( settings->finger1sX, settings->finger1sY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
			
			float p1x, p1y, p1z;
			p1x = (float)posX;
			p1y = (float)posY;
			p1z = (float)posZ;
			
			settings->finger1modelCoords[0] = p1x;
			settings->finger1modelCoords[1] = p1y;
			settings->finger1modelCoords[2] = p1z;
			
			glReadPixels( settings->finger2sX, settings->finger2sY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
			gluUnProject( settings->finger2sX, settings->finger2sY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
			
			p1x = (float)posX;
			p1y = (float)posY;
			p1z = (float)posZ;
			
			settings->finger2modelCoords[0] = p1x;
			settings->finger2modelCoords[1] = p1y;
			settings->finger2modelCoords[2] = p1z;

			//project to screen plane   //THIS DOESNT WORK THIS WAY, NEED TO DRAW A QUAD AT FOCAL DIST AND PROJECT TO THAT NOT TERRAIN
			/*
			Vec3 f1;
			Vec3 f2;
			f1.x = settings->finger1modelCoords[0];
			f1.y = settings->finger1modelCoords[1];
			f1.z = settings->finger1modelCoords[2];

			f2.x = settings->finger2modelCoords[0];
			f2.y = settings->finger2modelCoords[1];
			f2.z = settings->finger2modelCoords[2];

			Vec3 camLoc = settings->camera->position;
			Vec3 f1toCam = f1 - camLoc;
			Vec3 f2toCam = f2 - camLoc;
			f1toCam.normalize();
			f2toCam.normalize();
			
			settings->finger1modelCoords[0] = camLoc.x + (f1toCam.x * settings->camera->focalLength);
			settings->finger1modelCoords[1] = camLoc.y + (f1toCam.y * settings->camera->focalLength);
			settings->finger1modelCoords[2] = camLoc.z + (f1toCam.z * settings->camera->focalLength);

			settings->finger2modelCoords[0] = camLoc.x + (f2toCam.x * settings->camera->focalLength);
			settings->finger2modelCoords[1] = camLoc.y + (f2toCam.y * settings->camera->focalLength);
			settings->finger2modelCoords[2] = camLoc.z + (f2toCam.z * settings->camera->focalLength);
			*/
		}

		//if actively positioning finger, maintain corresponding model coordinates
		if (settings->positioningXYFingerLocation[0] != -1 && settings->positioningZFingerLocation[2] != -1)
		{
			glReadPixels( settings->positioningXYFingerLocation[0], settings->positioningXYFingerLocation[1], 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
			gluUnProject( settings->positioningXYFingerLocation[0], settings->positioningXYFingerLocation[1], winZ, modelview, projection, viewport, &posX, &posY, &posZ);
			
			float p1x, p1y, p1z;
			p1x = (float)posX;
			p1y = (float)posY;
			p1z = (float)posZ;
			
			settings->positioningModelCoords[0] = p1x;
			settings->positioningModelCoords[1] = p1y;
			
			float depthHere = dataset->getDepthAt(p1x,p1y);
			if (depthHere != -1 && depthHere != 0)
			{
				//store model depth
				settings->positioningModelCoords[2] = depthHere;

				//get actual depth from model
				float interpolatedLayer = (-depthHere/dataset->dataBoundsMaxFakeDepth)*dataset->numDepths;
				int aboveLayer = floor((double)interpolatedLayer);
				int belowLayer = ceil((double)interpolatedLayer);
				float aboveDepth = dataset->getActualDepthAtLayer(aboveLayer);
				float belowDepth = dataset->getActualDepthAtLayer(belowLayer);
				float factor = interpolatedLayer - aboveLayer;
				float depth = factor*belowDepth + (1-factor)*aboveDepth;
				
				//store actual depth
				settings->positioningModelCoords[3] = depth;
				//printf("depths model %f, actual %f\n", settings->positioningModelCoords[2], settings->positioningModelCoords[3]);
			}
			else
			{
				settings->positioningModelCoords[2] = -1;
				settings->positioningModelCoords[3] = -1;
			}
		}//if need to update active positioning
		else
		{
			settings->positioningModelCoords[0] = -1;
			settings->positioningModelCoords[1] = -1;
			settings->positioningModelCoords[2] = -1;
		}
	

		for (int i=0;i<settings->toProcessCode.size();i++)
		{
			if (settings->toProcessCode.at(i) == ADD_DYEPOLE) //if add dyePole to location
			{
				
				winX = (float)settings->toProcessX.at(i);
				winY = (float)settings->toProcessY.at(i);
				//printf("winx %f, winy %f\n", winX, winY);
				glReadPixels( (int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

				//printf("depth buffer is %f\n", (float)winZ);

				gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
				
				//gluUnProject( winX, winY, 0.0, selectionModelview, selectionProjection, selectionViewport, &posX, &posY, &posZ);

				float p1x, p1y, p1z;
				p1x = (float)posX;
				p1y = (float)posY;
				p1z = (float)posZ;
				//printf("Clicked on %f, %f, %f\n", (float)posX, (float)posY, (float)posZ);

				float depthHere = dataset->getDepthAt(p1x,p1y);
				if (depthHere != -1 && depthHere != 0)
				{
					particleSystem->addDyePole(p1x,p1y, dataset->getDepthAt(p1x,p1y), 0);
					panelMan->addDyePolePanel(particleSystem->getDyePoleClosestTo(p1x,p1y), winX, winY);
				}
				else if (depthHere != -1)
					printf("ERROR: No depth info where clicked, off map?\n");
				else if (depthHere != 0)
					printf("ERROR: Can't spawn a dye pole on land!\n");
				settings->toProcessCode.erase(settings->toProcessCode.begin()+i);
				settings->toProcessX.erase(settings->toProcessX.begin()+i);
				settings->toProcessY.erase(settings->toProcessY.begin()+i);
				settings->toProcessZ.erase(settings->toProcessZ.begin()+i);
				i++;
			}//end if add dye to location
			else if (settings->toProcessCode.at(i) == ADD_PRECISE_DYEPOT) //if add precise dyePot to location
			{
				float ptx = (float)settings->toProcessX.at(i);
				float pty = (float)settings->toProcessY.at(i);
				float ptz = (float)settings->toProcessZ.at(i);
				particleSystem->addDyePole(ptx,pty, dataset->getDepthAt(ptx,pty), 0);
				DyePole* tempDP = particleSystem->getDyePoleClosestTo(ptx,pty);
				tempDP->deleteEmitter(0);
				tempDP->addEmitter(ptz, ptz);
				tempDP->changeEmitterSpread(0,1);
				tempDP->changeEmitterColor(0,1);
				panelMan->addDyePolePanel(tempDP, settings->positioningXYFingerLocation[0], settings->positioningXYFingerLocation[1]);

				/*
				//add uneditable point emitter:
				particleSystem->addDyePole((float)settings->toProcessX.at(i),(float)settings->toProcessY.at(i), (float)settings->toProcessZ.at(i), (float)settings->toProcessZ.at(i));
				panelMan->addDyePolePanel(particleSystem->getDyePoleClosestTo((float)settings->toProcessX.at(i),(float)settings->toProcessY.at(i)), settings->positioningXYFingerLocation[0], settings->positioningXYFingerLocation[1]);
				*/

				settings->toProcessCode.erase(settings->toProcessCode.begin()+i);
				settings->toProcessX.erase(settings->toProcessX.begin()+i);
				settings->toProcessY.erase(settings->toProcessY.begin()+i);
				settings->toProcessZ.erase(settings->toProcessZ.begin()+i);
				i++;
			}//end if add dye to location
			else if (settings->toProcessCode.at(i) == ADD_AUV_WAYPOINT) //if add AUV waypoint to location
			{
				float ptx = (float)settings->toProcessX.at(i);
				float pty = (float)settings->toProcessY.at(i);
				float ptz = (float)settings->toProcessZ.at(i);
				
				if (dataset->isWaterAt(ptx, pty, -ptz))
					pathPanel->path->addWaypoint(ptx, pty, ptz);
				//pathPanel->path->smooth();

				settings->toProcessCode.erase(settings->toProcessCode.begin()+i);
				settings->toProcessX.erase(settings->toProcessX.begin()+i);
				settings->toProcessY.erase(settings->toProcessY.begin()+i);
				settings->toProcessZ.erase(settings->toProcessZ.begin()+i);
				i++;
			}//end if add AUV waypoint
			else if (settings->toProcessCode.at(i) == DELETE_ALL_DYEPOLES) //delete dyepole(s)
			{
				particleSystem->deleteAllDyePoles();
				settings->toProcessCode.erase(settings->toProcessCode.begin()+i);
				settings->toProcessX.erase(settings->toProcessX.begin()+i);
				settings->toProcessY.erase(settings->toProcessY.begin()+i);
				settings->toProcessZ.erase(settings->toProcessZ.begin()+i);
				i++;
			}//end if delete all dyepoles
			else if (settings->toProcessCode.at(i) == SELECT_DYEPOLE || 
					 settings->toProcessCode.at(i) == DELETE_DYEPOLE) //if select or delete existing dyepole
			{
				winX = (float)settings->toProcessX.at(i);
				winY = (float)settings->toProcessY.at(i);
				//printf("winx %f, winy %f\n", winX, winY);
				glReadPixels( (int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

				//printf("depth buffer is %f\n", (float)winZ);

				gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
				
				//gluUnProject( winX, winY, 0.0, selectionModelview, selectionProjection, selectionViewport, &posX, &posY, &posZ);

				float p1x, p1y, p1z;
				p1x = (float)posX;
				p1y = (float)posY;
				p1z = (float)posZ;
				//printf("Clicked on %f, %f, %f\n", (float)posX, (float)posY, (float)posZ);

				//find closest dyepole
				float minDist = 10000000;
				int closestDP = -1;
				for (int j=0;j<particleSystem->dyePoles.size();j++)
				{
					float dpx = particleSystem->dyePoles.at(j)->x;
					float dpy = particleSystem->dyePoles.at(j)->y;
					float dist = sqrt( (p1x-dpx)*(p1x-dpx) + (p1y-dpy)*(p1y-dpy));
					if (dist < minDist)
					{
						minDist = dist;
						closestDP = j;
					}
				}
				if (closestDP > -1)
				{
					if (settings->toProcessCode.at(i) == SELECT_DYEPOLE)
						panelMan->addDyePolePanel(particleSystem->dyePoles.at(closestDP), winX, winY);
					else if (settings->toProcessCode.at(i) == DELETE_DYEPOLE)
					{
						particleSystem->deleteDyePole(closestDP);
					}
				}

				settings->toProcessCode.erase(settings->toProcessCode.begin()+i);
				settings->toProcessX.erase(settings->toProcessX.begin()+i);
				settings->toProcessY.erase(settings->toProcessY.begin()+i);
				settings->toProcessZ.erase(settings->toProcessZ.begin()+i);
				i++;
				
			}//end if select existing dyepole
			
		}//end for each pending interaction


		glClear (GL_COLOR_BUFFER_BIT);
		glClear (GL_DEPTH_BUFFER_BIT);

}//end processPendingInteractions()



void drawScene(int eye) //0=left or mono, 1=right
{

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	if (eye == 0)
		processPendingInteractions();

	////draw sensor location (origin)
	//glColor3f(0,0,0);
	//glPointSize(10);
	//glBegin(GL_POINTS);
	//	glVertex3f(0,0,0);
	//glEnd();
	


	//draw axes
	glLineWidth(1.0);
	glBegin(GL_LINES);
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(100,0,0);

		glColor3f(0,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,100,0);

		glColor3f(0,0,1);
		glVertex3f(0,0,0);
		glVertex3f(0,0,100);
	glEnd();

	//glBegin(GL_TRIANGLES);
	//glColor3f(1,1,0);
	//		glVertex3f(0,0,10);
	//		glVertex3f(0,400,10);
	//		glVertex3f(400,0,10);
	//glEnd();

	//dataset->draw3Ddebug();
	//dataset->drawLandAndSea();
	//dataset->drawLand3D();

	//dataset->drawTerrain(); //cost: 15 FPS
	dataset->drawTerrainVBO();

	//drawStereoDebug();

	//Vec3 focalPoint = settings->camera->getFocalPointOnGround();
	glColor3f(0,0,0);
	glPointSize(1);
	glBegin(GL_POINTS);
		glVertex3f(settings->camera->focalPoint.x,settings->camera->focalPoint.y,settings->camera->focalPoint.z+10);
	glEnd();

	glColor3f(1,1,1);
	glPointSize(3);
	glBegin(GL_POINTS);
		glVertex3f(settings->camera->focalPoint.x,settings->camera->focalPoint.y,settings->camera->focalPoint.z+10);
	glEnd();

	glColor3f(0,0,0);
	glPointSize(5);
	glBegin(GL_POINTS);
		glVertex3f(settings->camera->focalPoint.x,settings->camera->focalPoint.y,settings->camera->focalPoint.z+10);
	glEnd();


	/*glColor3f(1,1,1);
	glPointSize(10);
	glBegin(GL_POINTS);
		glVertex3f(settings->camera->tempLeftPoint.x, settings->camera->tempLeftPoint.y, settings->camera->tempLeftPoint.z);
	glEnd();*/



	//particleSystem->draw();  //cost: 2 FPS
	
	//float tick = GetTickCount64();

	particleSystem->drawDyePoles();
	particleSystem->drawVBOs();

	particleSystem->drawTakeoffs();

	pathPanel->path->draw();


	
	//float tock = GetTickCount64();
	//printf("Time to Draw: %f ms\n", tock-tick);


	glColor3f(COASTAL_SHAPEFILE_COLOR);
  //japanCoastline->drawPolygon3DVBO();
	//japanCoastline->draw3D(dataset->dataBoundsXOrigin, dataset->dataBoundsYOrigin, 1000,1000, 1);

	glPointSize(3);
	glColor3f(CITIES_COLOR);
	//japanCities->draw3D(dataset->dataBoundsXOrigin, dataset->dataBoundsYOrigin, 1000,1000, 1);
	//japanCities->drawMatchingLabeledPoints(22, "Tokyo", dataset->dataBoundsXOrigin, dataset->dataBoundsYOrigin, 1000,1000, 10, 2, 20);
	glColor3f(NUKEPLANTS_COLOR);
	//japanNukePlants->draw3D(dataset->dataBoundsXOrigin, dataset->dataBoundsYOrigin, 1000,1000);
	japanNukePlants->drawIconPoints3D(dataset->dataBoundsXOrigin, dataset->dataBoundsYOrigin, 1000,1000, 10, 1, 25);
	//japanNukePlants->drawIconPoints3D(dataset->dataBoundsXOrigin, dataset->dataBoundsYOrigin, 1000,1000, 10, 1, 60);


	//draw active positioning pole:
	if (settings->positioningModelCoords[2] != -1)
	{
		glLineWidth(2);
		glColor4f(0.8,0.8,0.95,1.0);
		glBegin(GL_LINES);
			glVertex3f(settings->positioningModelCoords[0],settings->positioningModelCoords[1], 0);
			glVertex3f(settings->positioningModelCoords[0],settings->positioningModelCoords[1], settings->positioningModelCoords[2]);
		glEnd();
		glColor4f(1.0,1.0,0.25,1.0);
		glPointSize(6);
		glBegin(GL_POINTS);
			glVertex3f(settings->positioningModelCoords[0], settings->positioningModelCoords[1], settings->currentlySelectedPoint[2]);
		glEnd();
	}

	touchManager->draw3D();

}



//This function is the GLUT timer for redisplay
void timer(int id)
{
	glutPostRedisplay();
	//display();
}

//This is the main GLUT display function.
void display(void)
{
	//printf("cam = %f, %f, %f Y: %f, P %f\n", settings->cam[0], settings->cam[1], settings->cam[2], settings->yaw, settings->pitch);
	//mouse zoom if needed
	//zoom();
	
	perRenderUpdates();

	//printf("display()\n");
	
	//clear buffers
	if (STEREO_MODE) //settings->stereoEnabled)//
	{
		//calc eye locations
		settings->camera->UpdateEyePositions();


		//tweak focal length to matach distance to focal point on the ground plane
		float dist = settings->camera->getDistToFocalPointOnGround();
		settings->camera->eyeSeparation = dist / 20;
		if (settings->camera->eyeSeparation > 500)
			settings->camera->eyeSeparation = 500;
		settings->camera->focalLength = dist;
		settings->camera->nearDist = 100;
		settings->camera->farDist = dist*6;

		//misc vars
		float ratio = (float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT);
		float radians = DTOR * settings->camera->aperture / 2;
		float wd2 = settings->camera->nearDist * tan(radians);
		float ndfl = settings->camera->nearDist / settings->camera->focalLength;
		float left, right, top, bottom;

		//draw left eye:
			if (SWAP_EYES)
				glDrawBuffer(GL_BACK_RIGHT);
			else
				glDrawBuffer(GL_BACK_LEFT);
			glClear (GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//gluPerspective(45,(float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT),5, 10000);
			left = -ratio * wd2 + 0.5 * settings->camera->eyeSeparation * ndfl;
			right = ratio * wd2 + 0.5 * settings->camera->eyeSeparation * ndfl;
			top = wd2;
			bottom = -wd2;
			glFrustum(left, right, bottom, top, settings->camera->nearDist, settings->camera->farDist);
			
			//load modelview matrix
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		
			gluLookAt(settings->camera->leftPosition.x, settings->camera->leftPosition.y, settings->camera->leftPosition.z,
				settings->camera->leftPosition.x + settings->camera->directionVector.x,
				settings->camera->leftPosition.y + settings->camera->directionVector.y,
				settings->camera->leftPosition.z + settings->camera->directionVector.z,
				settings->camera->upVector.x, settings->camera->upVector.y, settings->camera->upVector.z);

			drawScene(0);
			drawOverlay();
		
		//draw right eye:
			if (SWAP_EYES)
				glDrawBuffer(GL_BACK_LEFT);
			else
				glDrawBuffer(GL_BACK_RIGHT);
			glClear (GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//gluPerspective(45,(float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT),5, 10000);
			left = -ratio * wd2 - 0.5 * settings->camera->eyeSeparation * ndfl;
			right = ratio * wd2 - 0.5 * settings->camera->eyeSeparation * ndfl;
			top = wd2;
			bottom = -wd2;
			glFrustum(left, right, bottom, top, settings->camera->nearDist, settings->camera->farDist);
			
			

			//load modelview matrix
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			
			gluLookAt(settings->camera->rightPosition.x, settings->camera->rightPosition.y, settings->camera->rightPosition.z,
				settings->camera->rightPosition.x + settings->camera->directionVector.x,
				settings->camera->rightPosition.y + settings->camera->directionVector.y,
				settings->camera->rightPosition.z + settings->camera->directionVector.z,
				settings->camera->upVector.x, settings->camera->upVector.y, settings->camera->upVector.z);

			drawScene(1);
			drawOverlay();

		glutSwapBuffers();

	}
	else //non-stereo
	{
		glClear (GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

		settings->camera->UpdateEyePositions();


		//tweak focal length to matach distance to focal point on the ground plane
		float dist = settings->camera->getDistToFocalPointOnGround();
		settings->camera->eyeSeparation = dist / 20;
		if (settings->camera->eyeSeparation > 500)
			settings->camera->eyeSeparation = 500;
		settings->camera->focalLength = dist;
		settings->camera->nearDist = 100;
		settings->camera->farDist = dist*6;

		//misc vars
		float ratio = (float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT);
		float radians = DTOR * settings->camera->aperture / 2;
		float wd2 = settings->camera->nearDist * tan(radians);
		float ndfl = settings->camera->nearDist / settings->camera->focalLength;
		float left, right, top, bottom;

		//draw left eye:
			if (SWAP_EYES)
				glDrawBuffer(GL_BACK_RIGHT);
			else
				glDrawBuffer(GL_BACK_LEFT);
			glClear (GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//gluPerspective(45,(float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT),5, 10000);
			left = -ratio * wd2 + 0.5 * settings->camera->eyeSeparation * ndfl;
			right = ratio * wd2 + 0.5 * settings->camera->eyeSeparation * ndfl;
			top = wd2;
			bottom = -wd2;
			glFrustum(left, right, bottom, top, settings->camera->nearDist, settings->camera->farDist);
			
			//load modelview matrix
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		
			gluLookAt(settings->camera->leftPosition.x, settings->camera->leftPosition.y, settings->camera->leftPosition.z,
				settings->camera->leftPosition.x + settings->camera->directionVector.x,
				settings->camera->leftPosition.y + settings->camera->directionVector.y,
				settings->camera->leftPosition.z + settings->camera->directionVector.z,
				settings->camera->upVector.x, settings->camera->upVector.y, settings->camera->upVector.z);

			drawScene(0);
			drawOverlay();

		glutSwapBuffers();
	}




	//cap screenshot if requested
	//if (takeSnapshot)
	//{
	//	printf("taking snapshot");
	//	takeSnapshot = false;

	//	IplImage* snapshot = cvCreateImage(cvSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)), IPL_DEPTH_8U, 3);

	//	glReadBuffer(GL_FRONT);
	//	glReadPixels(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), GL_RGB, GL_UNSIGNED_BYTE, snapshot->imageData);

	//	cvConvertImage(snapshot, snapshot, 3);

	//	time_t ltime;                                                                
	//	time(&ltime);                                                                
	//	char* outFileName2 = ctime(&ltime);
	//	//printf("ctime gives: %s\n",outFileName2);
	//	while (strchr(outFileName2, ' ') != NULL)
	//	{
	//		*strchr(outFileName2, ' ') = '-';
	//	}
	//	while (strchr(outFileName2, ':') != NULL)
	//	{
	//		*strchr(outFileName2, ':') = '-';
	//	}
	//	char filename[128];
	//	strcpy(filename,"snapshots\\");
	//	strncat(filename,outFileName2,strlen(outFileName2)-1);
	//	strcat(filename,".png");
	//	//printf("filename is: %s\n",filename);

	//	if(cvSaveImage(filename,snapshot))
	//		printf("Snapshot saved as: %s\n",filename);
	//	else
	//		printf("Could not save: %s\n",filename);

	//}//end take screenshot

	glutTimerFunc( 33, timer, 0);


}//end display



//This is the GLUT reshape function
void reshape (int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);

}

//This is the GLUT initialization function
void init(void)
{
	leftMouseDown = false;
	rightMouseDown = false;

	srand(time(NULL));
	settings = new Settings();
	
	touchManager = new TouchManager(settings, panelMan, dataset);
	int err_code = touchManager->Init();
	if(err_code != PQMTE_SUCCESS)
	{
		printf("ERROR initializing touch manager!\n");
	}

	//japanCoastline = new Shapefile("shapefiles\\jp_dcw_cst_pgn.shp");
	//japanCoastline->buildPolygon3DVBO(dataset->dataBoundsXOrigin, dataset->dataBoundsYOrigin, 1000,1000, 1);

	////japanCities = new Shapefile("shapefiles\\japan_nima.shp"); 
	//japanCities = new Shapefile("shapefiles\\japan_nima.shp", "shapefiles\\japan_nima.dbf"); 
	////japanCitiesAttributes = new Attributefile("shapefiles\\japan_nima.dbf"); 


	////japanNukePlants = new Shapefile("shapefiles\\jp_nuclear_plants.shp");
	//japanNukePlants = new Shapefile("shapefiles\\jp_nuclear_plants.shp", "shapefiles\\jp_nuclear_plants.dbf");
	////japanNukePlantsAttributes = new Attributefile("shapefiles\\jp_nuclear_plants.dbf"); 

	//
	//particleSystem = new ParticleSystem(dataset, settings);
	//panelMan->addParticleSystemPanel(particleSystem);
	//pathPanel = panelMan->addAUVPathPanel(dataset);


	takeSnapshot = false;
	dyeMode = false;
	processMouseClick = 0;
	
	lastMouseNavX = 0;
	lastMouseNavY = 0;
	mouseLeftNav = false;
	mouseRightNav = false;
	zoomIn = true;
	zooming = false;
	moveSpeed = DEFAULTMOVESPEED;


	glClearColor(MAIN_BACKGROUND_COLOR,1.0);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_NORMALIZE); 
	//glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
//	glShadeModel(GL_SMOOTH);


	//NEW QAT CAM SETTINGS
	/*settings->camera->SetPosition(0,0,0);
	settings->camera->SetHeading(0);
	settings->camera->SetPitch(0);
	settings->camera->SetSpeed(75);*/

//	settings->camera->SetPosition(1494,3092,-9241);
	//settings->camera->SetHeading(0);
	//settings->camera->SetPitch(-2.6);
//	settings->camera->SetYawPitch(0,-2.6);
//	settings->camera->SetSpeed(75);


	settings->camera->setFocalPoint(1500,3000,0);
	settings->camera->setViewAngle(180, 160);
	settings->camera->setViewDistance(10000);



//	//move lights to illuminate terrain
//	glEnable(GL_LIGHTING);
//	float sun1[] = {(wrightsville->commonBBoxXMax+wrightsville->commonBBoxXMin)/2, 450000, wrightsville->commonBBoxZMax, 1.0};
//	float sun2[] = {wrightsville->commonBBoxXMin, 700000, (wrightsville->commonBBoxZMin+wrightsville->commonBBoxZMax)/2, 1.0};
//	float ambient0[] = {.05,.05,.05, 1};
//	float diff0[] = {1, 1, 1, 1};
//	float specular0[] = {.25, .25, .25, 1};
//
//	//attach to LIGHT0:
//	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0);
//	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
//	glLightfv(GL_LIGHT0,GL_POSITION, sun1);
//	glEnable(GL_LIGHT0);
//
//	//attach to LIGHT1:
//	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient0);
//	glLightfv(GL_LIGHT1, GL_DIFFUSE, diff0);
//	glLightfv(GL_LIGHT1, GL_SPECULAR, specular0);
//	glLightfv(GL_LIGHT1,GL_POSITION, sun2);
//	glEnable(GL_LIGHT1);

}



//This is the GLUT mouse selection callback function
void mouseSelection(int button, int state, int x, int y)
{
	y = glutGet(GLUT_WINDOW_HEIGHT) - y;

	if (state == GLUT_UP && button == 3)
	{
		//zoom in
		//settings->camera->MoveForward(settings->camera->moveSpeed*10);
		settings->camera->changeViewDistance(-settings->camera->viewDistSpeed);
		zoomIn = true;
	}
	else if (state == GLUT_UP && button == 4)
	{
		//zoom out
		//settings->camera->MoveBackward(settings->camera->moveSpeed*10);
		settings->camera->changeViewDistance(settings->camera->viewDistSpeed);
		zoomIn = false;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		rightMouseDown = true;
		lastMouseNavX = x;
		lastMouseNavY = y;
		mouseRightNav = true;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		rightMouseDown = false;
		if (mouseRightNav)//mouse navigation done
		{
			mouseRightNav = false;
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		leftMouseDown = true;
		lastMouseNavX = x;
		lastMouseNavY = y;
		if (dyeMode)
		{
			//processMouseClick = 1;
			settings->toProcessCode.push_back(1);
			settings->toProcessX.push_back(x);
			settings->toProcessY.push_back(y);
			settings->toProcessY.push_back(0);
			
		}
		else
		{
			
			mouseLeftNav = true;
		}
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		leftMouseDown = false;
		if (mouseLeftNav)//mouse navigation done
		{
			mouseLeftNav = false;
		}
	}

	glutPostRedisplay();
}



//This is the GLUT mouse motion callback function
void motionFunc(int x, int y)
{
	y = glutGet(GLUT_WINDOW_HEIGHT) - y;
	
	
	/*if (leftMouseDown && dyeMode)
	{
		settings->toProcessCode.push_back(1);
		settings->toProcessX.push_back(x);
		settings->toProcessY.push_back(y);
	}*/

	if (mouseRightNav)
	{
		float deltaX = x - lastMouseNavX;
		float deltaY = y - lastMouseNavY;

		//settings->camera->ChangeHeading(deltaX/15);
		//settings->camera->ChangePitch(-deltaY/15);

		//settings->camera->ChangeYawPitch(deltaX/15, -deltaY/15);
		//settings->camera->changeViewAngle(-deltaX/8, deltaY/8);
	}
	if (mouseLeftNav)
	{
		float deltaX = x - lastMouseNavX;
		float deltaY = y - lastMouseNavY;

		//settings->camera->TranslateAlongGroundPlane(deltaX, deltaY);
	}
	else if (zooming)
	{
		float deltaX = lastMouseNavX - x;
		float deltaY = lastMouseNavY - y;

		//settings->camera->ChangeHeading(-deltaX/15);
		//settings->camera->ChangePitch(deltaY/15);
		//settings->camera->ChangeYawPitch(-deltaX/15, deltaY/15);
		//settings->camera->changeViewAngle(-deltaX/8, deltaY/8);
	}
	
	lastMouseNavX = x;
	lastMouseNavY = y;

	glutPostRedisplay();
}

//This is the GLUT mouse passive motion callback function
void passiveFunc(int x, int y)
{
	glutPostRedisplay();
}

//This is the GLUT keyboard callback function
void keyboard (unsigned char key, int x, int y)
{
	if (key == 27) //escape key
		exit(0);
	else if (key == 'q')
	{
		takeSnapshot = true;
	}
	else if (key == '1')
	{
		moveSpeed = 10;
	}
	else if (key == '2')
	{
		moveSpeed = 20;
	}
	else if (key == '3')
	{
		moveSpeed = 40;
	}
	else if (key == '4')
	{
		moveSpeed = 75;
	}
	else if (key == 'r')
	{
		moveSpeed = 10;
		//settings->cam[0] = 0;
		//settings->cam[1] = 0;
		//settings->cam[2] = 0;
		//settings->yaw = 0;
		//settings->pitch = 0;


		settings->camera->setFocalPoint(1500,3000,0);
		settings->camera->setViewAngle(180, 160);
		settings->camera->setViewDistance(10000);



//		settings->camera->SetPosition(0,0,0);
		//settings->camera->SetHeading(0);
		//settings->camera->SetPitch(0);
//		settings->camera->SetYawPitch(0,0);
//		settings->camera->SetSpeed(100);
	}
	else if (key == 't')
	{
		//settings->camera->SetYawPitch(0,0);

//		float distToFocalPoint = settings->camera->getDistToFocalPointOnGround();
//		Vec3 focalPoint = settings->camera->getFocalPointOnGround();
//		settings->camera->SetYawPitch(0,0);
//		Vec3 newCamLoc;
//		newCamLoc.x = focalPoint.x;
//		newCamLoc.y = focalPoint.y;
//		newCamLoc.z = focalPoint.z - distToFocalPoint;
		//printf("new cam loc: %f, %f, %f\n", newCamLoc.x, newCamLoc.y, newCamLoc.z);
//		settings->camera->SetPosition(newCamLoc.x, newCamLoc.y, newCamLoc.z);

		//panelMan->addDyePolePanel(particleSystem->dyePoles.at(0));
		settings->drawTemps = !settings->drawTemps;
	}

	else if (key == 'n')
	{
		//settings->camera->setYaw(180);
		dataset->currentTimestep++;
		particleSystem->pause();
		dataset->loadTimestep(dataset->currentTimestep);
		particleSystem->unPause();
	}
	else if (key == 'd')
	{
		dyeMode = !dyeMode;
	}
	else if (key == 's')
	{
		//if (settings->stereoEnabled)
		//{
		//	glDrawBuffer(GL_BACK_LEFT);
		//	glClear (GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
		//	glDrawBuffer(GL_BACK_LEFT);
		//	glClear (GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
		//}
		//settings->stereoEnabled = !settings->stereoEnabled;
	}
	else if (key == 'c')
	{
		//printf("cam xyz: %0.2f, %0.2f, %0.2f heading: %0.2f, pitch: %0.2f\n", settings->camera->position.x, settings->camera->position.y, settings->camera->position.z, settings->camera->heading, settings->camera->pitch);
		printf("focal point xyz: %0.2f, %0.2f, %0.2f\n", settings->camera->focalPoint.x, settings->camera->focalPoint.y, settings->camera->focalPoint.z);
		printf("cam position xyz: %0.2f, %0.2f, %0.2f\n", settings->camera->position.x, settings->camera->position.y, settings->camera->position.z);
		printf("yaw %0.3f, pitch %0.3f, dist %0.3f\n", settings->camera->yaw, settings->camera->pitch, settings->camera->viewDist);
	}
	else if (key == 'f')
	{
		settings->seeding = !settings->seeding;
	}
	else if (key == 'p')
	{
		//Particle tempPart(2000,3500,0,3000, 3000, dataset);
		//particles.push_back(tempPart);
		//printf("Added particle %d\n", particles.size());
	}
	else if (key == 'l')
	{
		settings->camera->navigationConstraintsActive = !settings->camera->navigationConstraintsActive;
	}

	glutPostRedisplay();
}

void idle() {

	display();
}


int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	if (STEREO_MODE)
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_STEREO);
	else
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	//glutInitWindowSize(1000, 800);
	//glutInitWindowPosition(0, 0);
	
	glutInitWindowSize(1906, 1002);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("SMT Flow - Nov 4th 2014");
	init();

	if (GLEW_OK != glewInit())
	{
		printf("ERROR with glew init!\n");
		return 1;
	}


	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseSelection);
	glutMotionFunc(motionFunc);

	glutMainLoop();
	
	return 0;
}

