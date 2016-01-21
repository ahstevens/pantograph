#include "Settings.h"

Settings::Settings()
{
	resetToDefaults();
	//camera = new FocalCamera();
	//textMan = new TextureManager();
}

void Settings::resetToDefaults()
{
	mirror = true;
	quadSteeringActive = false;
	seeding = true;

	positioningAUVWaypoint = false;
	positioningAUVGrid = 0;
	gridMode = 0;
	speedToNextWaypoint = 3;


	pantographMode = false;

	positioningDyePot = false;
	positioningXYFingerLocation[0] = -1;
	positioningXYFingerLocation[1] = -1;
	positioningZFingerLocation[0] = -1;
	positioningZFingerLocation[1] = -1;

	positioningModelCoords[0] = -1;
	positioningModelCoords[1] = -1;
	positioningModelCoords[2] = -1;
	positioningModelCoords[3] = -1;

	currentlySelectedPoint[0] = -1;
	currentlySelectedPoint[1] = -1;
	currentlySelectedPoint[2] = -1;
	
	drawTemps = false;

	dimming = false;
	dimTimer = -1;

	transitionRequested = false;
}