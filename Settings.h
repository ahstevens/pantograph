#pragma once

#include <vector>
#include <glm\glm.hpp>
#include <random>

#include "StudyManager.h"

class Settings
{

public:
    Settings();

	void activate(StudyManager::InteractionMode m);
	void deactivate();

	void resetToDefaults();

	StudyManager *study;

	bool modeSwitched;

	int positioningPointLocation[3];
	float positioningModelCoords[4]; //x,y,maxDepth,actualDepth,selectedDepth
	float currentlySelectedPoint[3];//x,y,depth

	//  maxDepth is the furthest world-space coords of cursor from screen
	float worldDepths[2];//maxDepth,minDepth

	float finger1sX, finger1sY, finger2sX, finger2sY; //finger 1 and 2 screen coords
	float finger1modelCoords[3];
	float finger2modelCoords[3];

	bool dimmingRequested, transitionRequested;

	bool transitionOnLensExit;

	bool trackingCursor;
	float cursorDistance, cursorTrackingRate;

	float polhemusMovementMultiplier;

	glm::vec3 polhemusOrigin;

private:

};