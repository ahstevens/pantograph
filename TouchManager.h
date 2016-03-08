#pragma once

#include "PQMTClient.h"
#include <stdio.h>
#include <map>
#include "Settings.h"
#include "GL\gl_includes.h"
#include "DrawFunctions.h"
#include "Pantograph.h"

#define SHOW_DEBUG_MESSAGES false

#define RAD2DEG 57.29578

#define GESTURE_NULL			0
#define GESTURE_MOVING_PANEL	1
#define GESTURE_BUTTON_PRESSED	2
#define GESTURE_MAP_MENU		3
#define GESTURE_PANEL_MENU		4


#define RADIAL_MENU_LINGER_TIME 350

using namespace PQ_SDK_MultiTouch;

class TouchManager
{

public:
	TouchManager(Settings* sets);
	~TouchManager();

	void draw2D();
	void draw3D();
	bool perRenderUpdate();
	
	int Init();

	void setPantoHand(bool rightHanded);

	Settings* settings;
	

private:
	//////////////////////call back functions///////////////////////
	// OnReceivePointFrame: function to handle when recieve touch point frame
	//	the unmoving touch point won't be sent from server. The new touch point with its pointevent is TP_DOWN
	//	and the leaving touch point with its pointevent will be always sent from server;
	static void OnReceivePointFrame(int frame_id,int time_stamp,int moving_point_count,const TouchPoint * moving_point_array, void * call_back_object);
	// OnReceivePointFrame: function to handle when recieve touch gesture
	static void OnReceiveGesture(const TouchGesture & ges, void * call_back_object);
	// OnServerBreak: function to handle when server break(disconnect or network error)
	static void OnServerBreak(void * param, void * call_back_object);
	// OnReceiveError: function to handle when some errors occur on the process of receiving touch datas.
	static void OnReceiveError(int err_code,void * call_back_object);
	// OnGetServerResolution: function to get the resolution of the server system.attention: not the resolution of touch screen. 
	static void OnGetServerResolution(int x, int y, void * call_back_object);
	// OnGetDeviceInfo: function to get the information of the touch device.
	static void OnGetDeviceInfo(const TouchDeviceInfo & device_info, void * call_back_object);
	//////////////////////call back functions end ///////////////////////

	// functions to handle TouchGestures, attention the means of the params
	void InitFuncOnTG();
	// set the call back functions while reciving touch data;
	void SetFuncsOnReceiveProc();

	// OnTouchPoint: function to handle TouchPoint
	void OnTouchPoint(const TouchPoint & tp);
	// OnTouchGesture: function to handle TouchGesture
	void OnTouchGesture(const TouchGesture & tg);
	//

	//here use function pointer table to handle the different gesture type;
	typedef void (*PFuncOnTouchGesture)(const TouchGesture & tg,void * call_object);
	static void DefaultOnTG(const TouchGesture & tg,void * call_object); // just show the gesture

	static void OnTG_TouchStart(const TouchGesture & tg,void * call_object);
	static void OnTG_Down(const TouchGesture & tg,void * call_object);
	static void OnTG_Move(const TouchGesture & tg,void * call_object);
	static void OnTG_Up(const TouchGesture & tg,void * call_object);

	//
	static void OnTG_SecondDown(const TouchGesture & tg,void * call_object);
	static void OnTG_SecondUp(const TouchGesture & tg,void * call_object);

	//
	static void OnTG_SplitStart(const TouchGesture & tg,void * call_object);
	static void OnTG_SplitApart(const TouchGesture & tg,void * call_object);
	static void OnTG_SplitClose(const TouchGesture & tg,void * call_object);
	static void OnTG_SplitEnd(const TouchGesture & tg,void * call_object);

	// OnTG_TouchEnd: to clear what need to clear;
	static void OnTG_TouchEnd(const TouchGesture & tg,void * call_object);

	//NEW ONES:
	static void OnTG_Click(const TouchGesture & tg,void * call_object);
	static void OnTG_DBClick(const TouchGesture & tg,void * call_object);
	static void OnTG_SecondClick(const TouchGesture & tg,void * call_object);
	static void OnTG_SecondDBClick(const TouchGesture & tg,void * call_object);
		
	static void OnTG_BigDown(const TouchGesture & tg,void * call_object);
	static void OnTG_BigMove(const TouchGesture & tg,void * call_object);
	static void OnTG_BigUp(const TouchGesture & tg,void * call_object);
	
	static void OnTG_MoveRight(const TouchGesture & tg,void * call_object);
	static void OnTG_MoveUp(const TouchGesture & tg,void * call_object);
	static void OnTG_MoveLeft(const TouchGesture & tg,void * call_object);
	static void OnTG_MoveDown(const TouchGesture & tg,void * call_object);
	
	
	static void OnTG_RotateStart(const TouchGesture & tg,void * call_object);
	static void OnTG_RotateAntiClock(const TouchGesture & tg,void * call_object);
	static void OnTG_RotateClock(const TouchGesture & tg,void * call_object);
	static void OnTG_RotateEnd(const TouchGesture & tg,void * call_object);

	static void OnTG_NearParrelDown(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelMove(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelUp(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelClick(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelDBClick(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelMoveRight(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelMoveUp(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelMoveLeft(const TouchGesture & tg,void * call_object);
	static void OnTG_NearParrelMoveDown(const TouchGesture & tg,void * call_object);
	
	static void OnTG_MultiDown(const TouchGesture & tg,void * call_object);
	static void OnTG_MultiMove(const TouchGesture & tg,void * call_object);
	static void OnTG_MultiUp(const TouchGesture & tg,void * call_object);
	static void OnTG_MultiMoveRight(const TouchGesture & tg,void * call_object);
	static void OnTG_MultiMoveUp(const TouchGesture & tg,void * call_object);
	static void OnTG_MultiMoveLeft(const TouchGesture & tg,void * call_object);
	static void OnTG_MultiMoveDown(const TouchGesture & tg,void * call_object);
	

private:
	PFuncOnTouchGesture m_pf_on_tges[TG_TOUCH_END + 1];

	std::map <int, int> gestureTypeMap;

	bool ignoreGestures;
	bool gestureActive;

	float lastMultiX;
	float lastMultiY;

	float initParallelX;
	float initParallelY;
	float lastParallelX;
	float lastParallelY;
	float initParallelDownTime;
	float lastParallelDownTime;

	int numFingersDown;
	bool multiMoveMode;
	float multiClickTimer;

	bool fingerNav;
	float lastFingerNavX;
	float lastFingerNavY;

	std::map <int, float> fingerDownTimeMap;
	std::map <int, float> fingerDownXMap;
	std::map <int, float> fingerDownYMap;

	//pantograph positioning
	Pantograph *pantograph;
	int firstFingerID;
	float firstFingerX, firstFingerY;
	int secondFingerID;
	float secondFingerX, secondFingerY;


	void resetFingers();

};