#ifndef __Settings_h__
#define __Settings_h__

#define STEREO_MODE true


#define SWAP_EYES false

//#include "cv.h"
#include <vector>
#include <glm\glm.hpp>

#define ADD_DYEPOLE 1
#define DELETE_ALL_DYEPOLES 2
#define SELECT_DYEPOLE 3
#define DELETE_DYEPOLE 4
#define ADD_PRECISE_DYEPOT 5
#define ADD_AUV_WAYPOINT 6


class Settings
{

public:
    Settings();

	void resetToDefaults();

	//FocalCamera* camera;
	//TextureManager* textMan;
	
	
	bool mirror;

	//float cam[3];
	//float yaw;
	//float pitch;

	std::vector <int> toProcessX;
	std::vector <int> toProcessY;
	std::vector <int> toProcessZ;
	std::vector <int> toProcessCode;


	std::vector <float> RegionX;
	std::vector <float> RegionY;


	bool positioningDyePot;
	bool pantographMode, mouseMode;

	bool positioningAUVWaypoint;
	int positioningAUVGrid; //0=not in mode, 1=select depth mode, 2=select extents
	int gridMode; //0=constant depth, 1=constant height
	float speedToNextWaypoint;

	int movingAUVWaypoints; //0=not in mode, 1=waiting for first touch, 2=moving points

	int positioningXYFingerLocation[2];
	int positioningZFingerLocation[2];
	float positioningModelCoords[4]; //x,y,maxDepth,actualDepth,selectedDepth
	float currentlySelectedPoint[3];//x,y,depth

	//  maxDepth is the furthest world-space coords of cursor from screen
	float worldDepths[2];//maxDepth,minDepth

	float finger1sX, finger1sY, finger2sX, finger2sY; //finger 1 and 2 screen coords
	float finger1modelCoords[3];
	float finger2modelCoords[3];


	bool quadSteeringActive;
	int quadSteerCoords[8];

	//bool stereoEnabled;

	bool seeding;

	bool drawTemps;

	bool dimmingRequested, transitionRequested;

private:

};

#endif