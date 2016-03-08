#include "TouchManager.h"
#include <iostream>

TouchManager::TouchManager(Settings* sets)
{
	ConnectServer("127.0.0.1", PQMT_DEFAULT_CLIENT_PORT);
	numFingersDown = 0;
	lastMultiX = 0;
	lastMultiY = 0;
	multiMoveMode = 0;
	fingerNav = false;
	ignoreGestures = false;
	gestureActive = false; 
	settings = sets;

	//pantograph stuff
	firstFingerID = -1;
	secondFingerID = -1;
	firstFingerX = -1;
	firstFingerY = -1;
	secondFingerX = -1;
	secondFingerY = -1;
	pantograph = new Pantograph();
	pantograph->settings = settings;
	pantograph->setLeftHanded(settings->study->isSubjectLeftHanded());
	

	settings->finger1sX = settings->finger1sY = settings->finger2sX = settings->finger2sY = -1;
}
TouchManager::~TouchManager()
{
	PQ_SDK_MultiTouch::DisconnectServer();
}

int TouchManager::Init()
{
	int err_code = PQMTE_SUCCESS;

	// initialize the handle functions of gestures;
	InitFuncOnTG();
	// set the functions on server callback
	SetFuncsOnReceiveProc();
	// connect to touch server
	printf("Connecting to touch server...");
	if((err_code = ConnectServer()) != PQMTE_SUCCESS)
	{
		printf("FAILED error code: %d\n", err_code);
		return err_code;
	}
	// send request to server
	printf("CONNECTED\n");
	printf("Sending request...");
	TouchClientRequest tcq = {0};
	tcq.type = RQST_RAWDATA_ALL | RQST_GESTURE_ALL;
	//tcq.type = RQST_GESTURE_ALL;
	if((err_code = SendRequest(tcq)) != PQMTE_SUCCESS){
		printf("FAILED error code: %d\n", err_code);
		return err_code;
	}

	//get server resolution
	if((err_code = GetServerResolution(OnGetServerResolution, NULL)) != PQMTE_SUCCESS)
	{
		printf("FAILED to get server resolution, error code: %d\n", err_code);
		return err_code;
	}
	//
	// start receiving
	printf("SUCCESS!\n");
	return err_code;
}

void TouchManager:: InitFuncOnTG()
{
	m_pf_on_tges[TG_TOUCH_START] = &TouchManager::OnTG_TouchStart;
	
	// initialize the call back functions of toucha gestures;
	m_pf_on_tges[TG_TOUCH_START] = &TouchManager::OnTG_TouchStart;
	m_pf_on_tges[TG_DOWN] = &TouchManager::OnTG_Down;
	m_pf_on_tges[TG_MOVE] = &TouchManager::OnTG_Move;
	m_pf_on_tges[TG_UP] = &TouchManager::OnTG_Up;

	m_pf_on_tges[TG_SECOND_DOWN] = &TouchManager::OnTG_SecondDown;
	m_pf_on_tges[TG_SECOND_UP] = &TouchManager::OnTG_SecondUp;

	m_pf_on_tges[TG_SPLIT_START] = &TouchManager::OnTG_SplitStart;
	m_pf_on_tges[TG_SPLIT_APART] = &TouchManager::OnTG_SplitApart;
	m_pf_on_tges[TG_SPLIT_CLOSE] = &TouchManager::OnTG_SplitClose;
	m_pf_on_tges[TG_SPLIT_END] = &TouchManager::OnTG_SplitEnd;

	m_pf_on_tges[TG_TOUCH_END] = &TouchManager::OnTG_TouchEnd;

	m_pf_on_tges[TG_CLICK] = &TouchManager::OnTG_Click;
	m_pf_on_tges[TG_DB_CLICK] = &TouchManager::OnTG_DBClick;

	m_pf_on_tges[TG_BIG_DOWN] = &TouchManager::OnTG_BigDown;
	m_pf_on_tges[TG_BIG_MOVE] = &TouchManager::OnTG_BigMove;
	m_pf_on_tges[TG_BIG_UP] = &TouchManager::OnTG_BigUp;

	m_pf_on_tges[TG_MOVE_RIGHT] = &TouchManager::OnTG_MoveRight;
	m_pf_on_tges[TG_MOVE_UP] = &TouchManager::OnTG_MoveUp;
	m_pf_on_tges[TG_MOVE_LEFT] = &TouchManager::OnTG_MoveLeft;
	m_pf_on_tges[TG_MOVE_DOWN] = &TouchManager::OnTG_MoveDown;

	m_pf_on_tges[TG_SECOND_CLICK] = &TouchManager::OnTG_SecondClick;
	m_pf_on_tges[TG_SECOND_DB_CLICK] = &TouchManager::OnTG_SecondDBClick;
	
	m_pf_on_tges[TG_ROTATE_START] = &TouchManager::OnTG_RotateStart;
	m_pf_on_tges[TG_ROTATE_ANTICLOCK] = &TouchManager::OnTG_RotateAntiClock;
	m_pf_on_tges[TG_ROTATE_CLOCK] = &TouchManager::OnTG_RotateClock;
	m_pf_on_tges[TG_ROTATE_END] = &TouchManager::OnTG_RotateEnd;

	m_pf_on_tges[TG_NEAR_PARREL_DOWN] = &TouchManager::OnTG_NearParrelDown;
	m_pf_on_tges[TG_NEAR_PARREL_MOVE] = &TouchManager::OnTG_NearParrelMove;
	m_pf_on_tges[TG_NEAR_PARREL_UP] = &TouchManager::OnTG_NearParrelUp;
	m_pf_on_tges[TG_NEAR_PARREL_CLICK] = &TouchManager::OnTG_NearParrelClick;
	m_pf_on_tges[TG_NEAR_PARREL_DB_CLICK] = &TouchManager::OnTG_NearParrelDBClick;
	m_pf_on_tges[TG_NEAR_PARREL_MOVE_RIGHT] = &TouchManager::OnTG_NearParrelMoveRight;
	m_pf_on_tges[TG_NEAR_PARREL_MOVE_UP] = &TouchManager::OnTG_NearParrelMoveUp;
	m_pf_on_tges[TG_NEAR_PARREL_MOVE_LEFT] = &TouchManager::OnTG_NearParrelMoveLeft;
	m_pf_on_tges[TG_NEAR_PARREL_MOVE_DOWN] = &TouchManager::OnTG_NearParrelMoveDown;

	m_pf_on_tges[TG_MULTI_DOWN] = &TouchManager::OnTG_MultiDown;
	m_pf_on_tges[TG_MULTI_MOVE] = &TouchManager::OnTG_MultiMove;
	m_pf_on_tges[TG_MULTI_UP] = &TouchManager::OnTG_MultiUp;

	m_pf_on_tges[TG_MULTI_MOVE_RIGHT] = &TouchManager::OnTG_MultiMoveRight;
	m_pf_on_tges[TG_MULTI_MOVE_UP] = &TouchManager::OnTG_MultiMoveUp;
	m_pf_on_tges[TG_MULTI_MOVE_LEFT] = &TouchManager::OnTG_MultiMoveLeft;
	m_pf_on_tges[TG_MULTI_MOVE_DOWN] = &TouchManager::OnTG_MultiMoveDown;
}
void TouchManager::SetFuncsOnReceiveProc()
{
	PFuncOnReceivePointFrame old_rf_func = SetOnReceivePointFrame(&TouchManager::OnReceivePointFrame,this);
	PFuncOnReceiveGesture old_rg_func = SetOnReceiveGesture(&TouchManager::OnReceiveGesture,this);
	PFuncOnServerBreak old_svr_break = SetOnServerBreak(&TouchManager::OnServerBreak,NULL);
	PFuncOnReceiveError old_rcv_err_func = SetOnReceiveError(&TouchManager::OnReceiveError,NULL);
	PFuncOnGetDeviceInfo old_gdi_func = SetOnGetDeviceInfo(&TouchManager::OnGetDeviceInfo,NULL);
}

void TouchManager:: OnReceivePointFrame(int frame_id, int time_stamp, int moving_point_count, const TouchPoint * moving_point_array, void * call_back_object)
{
	TouchManager * sample = static_cast<TouchManager*>(call_back_object);
	//assert(sample != NULL);
	const char * tp_event[] = 
	{
		"down",
		"move",
		"up",
	};
	
	if (SHOW_DEBUG_MESSAGES) printf("Frame ID: %d, Time: %d ms, moving point count: %d\n", frame_id, time_stamp, moving_point_count);
	TouchManager* tm = static_cast<TouchManager*>(call_back_object);
	tm->numFingersDown = moving_point_count;
	//if (tm->multiMoveMode)
	//{
	//	if (tm->numFingersDown < 4)
	//		tm->settings->camera->StopMoving();
	//	else if (tm->numFingersDown == 4)
	//		tm->settings->camera->SetVelocity(2000);
	//	else if (tm->numFingersDown == 5)
	//		tm->settings->camera->SetVelocity(-2000);
	//}
	//cout << " frame_id:" << frame_id << " time:"  << time_stamp << " ms" << " moving point count:" << moving_point_count << endl;
	for(int i = 0; i < moving_point_count; ++ i){
		TouchPoint tp = moving_point_array[i];
		sample->OnTouchPoint(tp);
	}
	//throw exception("test exception here");
}
void TouchManager:: OnReceiveGesture(const TouchGesture & ges, void * call_back_object)
{
	//printf("A");
	TouchManager* tm = static_cast<TouchManager*>(call_back_object);
	//assert(sample != NULL);
	//printf("ges:%s\n", GetGestureName(ges));
	//printf("A");
	tm->OnTouchGesture(ges);
	//printf("B");
	//throw exception("test exception here");
}
void TouchManager:: OnServerBreak(void * param, void * call_back_object)
{
	// when the server break, disconenct server;
	//cout << "server break, disconnect here" << endl;
	printf("SERVER BREAK, disconnecting....\n");
	DisconnectServer();
}
void TouchManager::OnReceiveError(int err_code, void * call_back_object)
{
	switch(err_code)
	{
	case PQMTE_RCV_INVALIDATE_DATA:
		printf("ERROR: Received invalid data.\n");
		//cout << " error: receive invalidate data." << endl;
		break;
	case PQMTE_SERVER_VERSION_OLD:
		printf("ERROR: Server version too old for client, update server.\n");
		//cout << " error: the multi-touch server is old for this client, please update the multi-touch server." << endl;
		break;
	case PQMTE_EXCEPTION_FROM_CALLBACKFUNCTION:
		printf("ERROR: Exception thrown from call back function!\n");
		//cout << "**** some exceptions thrown from the call back functions." << endl;
		//assert(0); //need to add try/catch in the callback functions to fix the bug;
		break;
	default:
		printf("ERROR: Socket Error, code: %d\n", err_code);
		//cout << " socket error, socket error code:" << err_code << endl;
	}
}
void TouchManager:: OnGetServerResolution(int x, int y, void * call_back_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("Server resolution is: %d, %d\n", x, y);
	//cout << " server resolution:" << x << "," << y << endl;
}
void TouchManager::OnGetDeviceInfo(const TouchDeviceInfo & deviceinfo,void *call_back_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("Touch screen S/N is %s, width: %d mm, height: %d mm\n", deviceinfo.serial_number, deviceinfo.screen_width, deviceinfo.screen_height); 
	//cout << " touch screen, SerialNumber: " << deviceinfo.serial_number <<",(" << deviceinfo.screen_width << "," << deviceinfo.screen_height << ")."<<  endl;
}

void TouchManager::resetFingers()
{
	firstFingerID = -1;
	firstFingerX = -1;
	firstFingerY = -1;
	secondFingerID = -1;
	secondFingerX = -1;
	secondFingerY = -1;
	settings->positioningPointLocation[0] = -1;
	settings->positioningPointLocation[1] = -1;
	settings->positioningPointLocation[2] = -1;
}

// here, just record the position of point,
//	you can do mouse map like "OnTG_Down" etc;
void TouchManager::OnTouchPoint(const TouchPoint & tp)
{
	if ((settings->study->modeRestriction != StudyManager::PANTOGRAPH &&
		settings->study->modeRestriction != StudyManager::NONE) ||
		(settings->study->currentMode != StudyManager::PANTOGRAPH &&
			settings->study->currentMode != StudyManager::NONE))
		return;

	float x = tp.x - glutGet(GLUT_WINDOW_X);
	float y = glutGet(GLUT_WINDOW_HEIGHT)-(tp.y-glutGet(GLUT_WINDOW_Y));
		
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PANTOGRAPH MODE////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//if (settings->positioningDyePotPantograph)
	{
		switch(tp.point_event)
		{
		case TP_DOWN:
			if (firstFingerID == -1)
			{
				firstFingerID = tp.id;
				firstFingerX = x;
				firstFingerY = y;
				pantograph->setFinger1(x,y);
				break;
			}
			else if (secondFingerID == -1)
			{
				if (!pantograph->secondFingerSet())//if second finger hasnt been set yet, do it now
				{
					secondFingerID = tp.id;
					secondFingerX = x;
					secondFingerY = y;
					pantograph->setFinger2(x,y);
					pantograph->setDrawReticle(true);
					settings->activate(StudyManager::PANTOGRAPH);
					
					//check if swap needed:
					if (pantograph->swapNeeded())
					{
						//printf("SWAPPING\n");
						secondFingerID = firstFingerID;
						secondFingerX = firstFingerX;
						secondFingerY = firstFingerY;
						firstFingerID = tp.id;
						firstFingerX = x;
						firstFingerY = y;
					}

					break;
				}
				else //second finger was set previously
				{
					//check if on dragBubble or not
					if (pantograph->isOnDragBubble(x,y))
					{
						//restore positioning controls
						secondFingerID = tp.id;
						secondFingerX = x;
						secondFingerY = y;
						break;
					}
					else //not on dragBubble
					{
						//check if on accept/select/deselect button
						if (pantograph->isOnAcceptBubble(x,y))
						{
							return;						
						}//end if on accept button
					}
				}//end if second finger set previously
				break;
			}//end else if second finger id == -1
			else//third (or higher) finger
			{
				//check if on accept/select/deselect button
				if (pantograph->isOnAcceptBubble(x,y))
				{
					return;
				}//end if on accept button
			}//end else if third (or higher) finger
			break;
		case TP_MOVE:
			if (tp.id == firstFingerID)
			{
				firstFingerX = x;
				firstFingerY = y;
				if (!pantograph->secondFingerSet() || (pantograph->secondFingerSet() && secondFingerID != -1))
					pantograph->setFinger1(x,y);
				break;
			}
			else if (tp.id == secondFingerID)
			{
				secondFingerX = x;
				secondFingerY = y;
				pantograph->setFinger2(x,y);
				break;
			}
			break;
		case TP_UP:
			if (tp.id == firstFingerID)
			{
				firstFingerID = -1;
				firstFingerX = -1;
				firstFingerY = -1;
				secondFingerID = -1;
				secondFingerX = -1;
				secondFingerY = -1;
				pantograph->resetFingers();
				settings->positioningPointLocation[0] = -1;
				settings->positioningPointLocation[1] = -1;
				settings->positioningPointLocation[2] = -1;
				settings->deactivate();
				break;
			}
			else if (tp.id == secondFingerID)
			{
				secondFingerID = -1;
				secondFingerX = -1;
				secondFingerY = -1;
				break;
			}
			break;
		}//end switch
		return;
	}//end pantograph mode

}

void TouchManager:: OnTouchGesture(const TouchGesture & tg)
{
	if (ignoreGestures || settings->study->currentMode == StudyManager::PANTOGRAPH)
		return;

	if(TG_NO_ACTION == tg.type)
		return ;

	PFuncOnTouchGesture pf = m_pf_on_tges[tg.type];

	if (tg.type == 86) //There is some type "86" gesture, that is not documented anywhere, and causes all sorts of stuff to crash e.g. GetGestureName()
	{                  //So lets just ignore any of these type 86 (Hex 0x0056) gestures
		//printf("ERROR:type86found!\n");
		return; 
	}

	if(NULL != pf){
		pf(tg,this);
	}
}

void TouchManager:: OnTG_TouchStart(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_TOUCH_START: No Params, can be used to initialize stuff.\n");
	//cout << "  here, the touch start, initialize something." << endl;
}
void TouchManager:: DefaultOnTG(const TouchGesture & tg,void * call_object) // just show the gesture
{
	//printf("GESTURE: %s, type: %d, #parms: %d -", GetGestureName(tg), tg.type, tg.param_size);
	//cout <<"ges,name:"<< GetGestureName(tg) << " type:" << tg.type << ",param size:" << tg.param_size << " ";
	//for(int i=0;i<tg.param_size;i++)
	//	printf(" %d",tg.params[i]);
		//cout << tg.params[i] << " ";
	//printf("\n");
	//cout << endl;
}
void TouchManager:: OnTG_Down(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_DOWN at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	if (!tm->fingerNav)
	{
		tm->fingerNav = true;
		tm->lastFingerNavX = tg.params[0];
		tm->lastFingerNavY = tg.params[1];
	}
}
void TouchManager:: OnTG_Move(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MOVE at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	if (tm->fingerNav)
	{
		float amountToMoveX = tg.params[0] - tm->lastFingerNavX;
		float amountToMoveY = tg.params[1] - tm->lastFingerNavY;
		//tm->settings->camera->TranslateAlongGroundPlane(amountToMoveX, -amountToMoveY);
		tm->lastFingerNavX = tg.params[0];
		tm->lastFingerNavY = tg.params[1];
	}
	
}
void TouchManager:: OnTG_Up(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_UP at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	if (tm->fingerNav)
		tm->fingerNav = false;
}


void TouchManager:: OnTG_SecondDown(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SECOND_DOWN at (%0.0f, %0.0f), first at: (%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
}
void TouchManager:: OnTG_SecondUp(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SECOND_UP at (%0.0f, %0.0f), first still at: (%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
	
}

void TouchManager:: OnTG_SplitStart(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SPLIT_START at (%0.0f, %0.0f),(%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
	
}

void TouchManager:: OnTG_SplitApart(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SPLIT_APART by %0.0f pix/mm? (ratio: %0.4f) at (%0.0f, %0.0f),(%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3], tg.params[4], tg.params[5]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->changeViewDistance(tg.params[0]*12);
}
void TouchManager:: OnTG_SplitClose(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SPLIT_CLOSE by %0.0f pix/mm? (ratio: %0.4f) at (%0.0f, %0.0f),(%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3], tg.params[4], tg.params[5]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->changeViewDistance(-tg.params[0]*12);
}
void TouchManager:: OnTG_SplitEnd(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SPLIT_END at (%0.0f, %0.0f),(%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
	
}
// OnTG_TouchEnd: to clear what need to clear
void TouchManager:: OnTG_TouchEnd(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_TOUCH_END: All fingers left, screen empty\n");
}


//NEW ONES:

//////////////////////////////////////////////////////////////////////////////////
//Click functions
//////////////////////////////////////////////////////////////////////////////////
void TouchManager::OnTG_Click(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_CLICK at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_DBClick(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_DB_CLICK at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_SecondClick(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SECOND_CLICK at (%0.0f, %0.0f), first at: (%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
}

void TouchManager::OnTG_SecondDBClick(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_SECOND_DB_CLICK at (%0.0f, %0.0f), first at: (%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
}

//////////////////////////////////////////////////////////////////////////////////
//Big (Fist) functions
//////////////////////////////////////////////////////////////////////////////////

void TouchManager::OnTG_BigDown(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_BIG_DOWN at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_BigMove(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_BIG_MOVE at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_BigUp(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_BIG_UP at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

//////////////////////////////////////////////////////////////////////////////////
//Move functions (single touching finger moves x direction)
//////////////////////////////////////////////////////////////////////////////////

void TouchManager::OnTG_MoveRight(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MOVE_RIGHT at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->MoveRight(1000);
}

void TouchManager::OnTG_MoveUp(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MOVE_UP at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->MoveUp(1000);
}

void TouchManager::OnTG_MoveLeft(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MOVE_LEFT at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->MoveLeft(1000);
}

void TouchManager::OnTG_MoveDown(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MOVE_DOWN at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->MoveDown(1000);
}


//////////////////////////////////////////////////////////////////////////////////
//Rotate functions
//////////////////////////////////////////////////////////////////////////////////

void TouchManager::OnTG_RotateStart(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_ROTATE_START Anchor at (%0.0f, %0.0f) Round at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
}

void TouchManager::OnTG_RotateAntiClock(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_ROTATE_ANTICLOCK Angle: %0.3f rads, Fingers:A(%0.0f, %0.0f),R(%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3], tg.params[4]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->Roll(-tg.params[0]*RAD2DEG);
}

void TouchManager::OnTG_RotateClock(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_ROTATE_CLOCK Angle: %0.3f rads, Fingers:A(%0.0f, %0.0f),R(%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3], tg.params[4]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->Roll(tg.params[0]*RAD2DEG);
}

void TouchManager::OnTG_RotateEnd(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_ROTATE_END Anchor at (%0.0f, %0.0f) Round at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3]);
}

//////////////////////////////////////////////////////////////////////////////////
//Nearby two touch functions:
//////////////////////////////////////////////////////////////////////////////////

void TouchManager::OnTG_NearParrelDown(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_DOWN at (%0.0f, %0.0f) ((%0.0f, %0.0f) and (%0.0f, %0.0f))\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3], tg.params[4], tg.params[5]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	tm->gestureActive = true;
	tm->initParallelX = tg.params[0];
	tm->initParallelY = tg.params[1];
	tm->lastParallelX = tg.params[0];
	tm->lastParallelY = tg.params[1];
	tm->initParallelDownTime = GetTickCount64();
	//tm->lastParallelDownTime = GetTickCount64();
}

void TouchManager::OnTG_NearParrelMove(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_MOVE at (%0.0f, %0.0f) ((%0.0f, %0.0f) and (%0.0f, %0.0f))\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3], tg.params[4], tg.params[5]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	//tm->settings->camera->changeViewDistance((tm->lastParallelY - tg.params[1])*12);  //Xcomp = (tm->lastParallelX - tg.params[0])/8
	tm->lastParallelX = tg.params[0];
	tm->lastParallelY = tg.params[1];
	//tm->settings->toProcessCode.push_back(1);
	//tm->settings->toProcessX.push_back((int)tg.params[0]);
	//tm->settings->toProcessY.push_back(glutGet(GLUT_WINDOW_HEIGHT)-(int)tg.params[1]);
}

void TouchManager::OnTG_NearParrelUp(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_UP at (%0.0f, %0.0f) ((%0.0f, %0.0f) and (%0.0f, %0.0f))\n", tg.params[0], tg.params[1], tg.params[2], tg.params[3], tg.params[4], tg.params[5]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	tm->gestureActive = false;
	float timeSinceDown = GetTickCount64()-tm->initParallelDownTime;
	if (timeSinceDown > 700)//check if down long enough
	{
		float distFromInitDown = sqrt( (tm->lastParallelX-tm->initParallelX)*(tm->lastParallelX-tm->initParallelX) + (tm->lastParallelY-tm->initParallelY)*(tm->lastParallelY-tm->initParallelY));
		if (distFromInitDown < 15) //check if didnt much much
		{
			//do stuff
		}
	}
}

void TouchManager::OnTG_NearParrelClick(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_CLICK at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
}


void TouchManager::OnTG_NearParrelDBClick(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_DB_CLICK at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
}

void TouchManager::OnTG_NearParrelMoveRight(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_MOVE_RIGHT at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_NearParrelMoveUp(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_MOVE_UP at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_NearParrelMoveLeft(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_MOVE_LEFT at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_NearParrelMoveDown(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_NEAR_PARREL_MOVE_DOWN at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

//////////////////////////////////////////////////////////////////////////////////
//Multi-finger functions
//////////////////////////////////////////////////////////////////////////////////

void TouchManager::OnTG_MultiDown(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MULTI_DOWN at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	tm->gestureActive = true;
	tm->lastMultiX = tg.params[0];
	tm->lastMultiY = tg.params[1];
	//tm->multiMoveMode = true;
	//tm->multiClickTimer = GetTickCount64();
}

void TouchManager::OnTG_MultiMove(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MULTI_MOVE at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	
	tm->lastMultiX = tg.params[0];
	tm->lastMultiY = tg.params[1];
}

void TouchManager::OnTG_MultiUp(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MULTI_UP at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
	TouchManager* tm = static_cast<TouchManager*>(call_object);
	tm->gestureActive = false;
}

void TouchManager::OnTG_MultiMoveRight(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MULTI_MOVE_RIGHT at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_MultiMoveUp(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MULTI_MOVE_UP at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_MultiMoveLeft(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MULTI_MOVE_LEFT at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}

void TouchManager::OnTG_MultiMoveDown(const TouchGesture & tg,void * call_object)
{
	if (SHOW_DEBUG_MESSAGES) printf("TG_MULTI_MOVE_DOWN at (%0.0f, %0.0f)\n", tg.params[0], tg.params[1]);
}


void TouchManager::draw2D()
{
	pantograph->draw2D();
}

void TouchManager::draw3D()
{
	pantograph->draw3D();
}

void TouchManager::setPantoHand(bool rightHanded)
{
	pantograph->setLeftHanded(!rightHanded);
}

// Returns TRUE when finger screen coords received, FALSE otherwise
bool TouchManager::perRenderUpdate()
{
	float pantoX, pantoY, pantoDepth;
	//send new screen coords to process into model coords during next render
	if (pantograph->getSelectPoint(&pantoX, &pantoY) && pantograph->getDepthFactor(&pantoDepth))
	{
		settings->positioningPointLocation[0] = pantoX;
		settings->positioningPointLocation[1] = pantoY;
		settings->positioningPointLocation[2] = pantoDepth;

		settings->finger1sX = firstFingerX;
		settings->finger1sY = firstFingerY;
		settings->finger2sX = secondFingerX;
		settings->finger2sY = secondFingerY;		

		//and if a render returned model coords
		if (settings->positioningModelCoords[0] != -1 && settings->positioningModelCoords[1] != -1)
		{
			settings->currentlySelectedPoint[0] = settings->positioningModelCoords[0];
			settings->currentlySelectedPoint[1] = settings->positioningModelCoords[1];
			settings->currentlySelectedPoint[2] = settings->positioningModelCoords[2];
		}
		else
			return true;

	}//end if valid selection
	else
	{
		settings->finger1sX = -1;
		settings->finger1sY = -1;
		settings->finger2sX = -1;
		settings->finger2sY = -1;
	}

	return false;
}