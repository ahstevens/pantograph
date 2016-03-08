#include "Settings.h"

Settings::Settings()
{
	study = StudyManager::getInstance();
	resetToDefaults();
	//camera = new FocalCamera();
	//textMan = new TextureManager();
}

void Settings::activate(StudyManager::InteractionMode m)
{
	this->study->currentMode = m;
	this->study->currentState = StudyManager::ACTIVE;
	this->modeSwitched = true;
	this->dimmingRequested = true;
}

void Settings::deactivate()
{
	this->study->currentMode = StudyManager::NONE;
	this->study->currentState = StudyManager::OFF;
	this->dimmingRequested = false;
	this->transitionRequested = true;
}

void Settings::resetToDefaults()
{
	positioningPointLocation[0] = -1;
	positioningPointLocation[1] = -1;
	positioningPointLocation[2] = -1;

	positioningModelCoords[0] = -1;
	positioningModelCoords[1] = -1;
	positioningModelCoords[2] = -1;
	positioningModelCoords[3] = -1;

	currentlySelectedPoint[0] = -1;
	currentlySelectedPoint[1] = -1;
	currentlySelectedPoint[2] = -1;

	worldDepths[0] = -1;
	worldDepths[1] = -1;
	
	modeSwitched = false;
	dimmingRequested = false;
	transitionRequested = false;
	transitionOnLensExit = false;

	trackingCursor = false;
	cursorDistance = 0.f;	
	cursorTrackingRate = 100.f; // Hz

	polhemusMovementMultiplier = 35.f;
}