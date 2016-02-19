#include "Settings.h"

Settings::Settings()
{
	study = StudyManager::getInstance();
	resetToDefaults();
	//camera = new FocalCamera();
	//textMan = new TextureManager();
}

void Settings::resetToDefaults()
{
	pantographMode = false;
	mouseMode = false;

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

	worldDepths[0] = -1;
	worldDepths[1] = -1;
	
	dimmingRequested = false;
	transitionRequested = false;
	transitionOnLensExit = false;

	trackingCursor = false;
	cursorDistance = 0.f;	
	cursorTrackingRate = 100.f; // Hz
}