#pragma once
#include <vrpn/vrpn_Tracker.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

class Polhemus
{
public:
	static Polhemus* getInstance( );
	Polhemus(std::string trackerName, std::string serverLocation);
	~Polhemus(void);

	static void VRPN_CALLBACK handle_tracker_callback( void* userData, const vrpn_TRACKERCB t );
	void VRPN_CALLBACK handle_tracker( void* userData, const vrpn_TRACKERCB t );

	void update();

	glm::vec3 getPosition();
	glm::quat getQuaternion();
	
	void calibrate();
	void calibrateReset();
	void calibrateRollUp( float degree = 1.f );
	void calibrateRollDown( float degree = 1.f );
	void calibratePitchUp( float degree = 1.f );
	void calibratePitchDown( float degree = 1.f );
	void calibrateYawUp( float degree = 1.f );
	void calibrateYawDown( float degree = 1.f );

	glm::quat getCalibration();
	void setCalibration( glm::quat c );

private:
	static Polhemus* instance;
	vrpn_Tracker_Remote* vrpnTracker;

	glm::quat quat, calibration;
	glm::vec3 pos;
};

