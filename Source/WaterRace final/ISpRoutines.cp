/*****************************************************************************
 * Copyright (c) 1998-2001, French Touch, SARL
 * http://www.french-touch.net
 * info@french-touch.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/


#include				<InputSprocket.h>

#include				"WaterRace.h"
#include				"Strings.h"
#include				"Preferences.h"
#include				"Game.h"
#include				"Drivers.h"
#include				"ISpRoutines.h"

#include				"Infinity Audio Engine.h"

#if __USE_AUDIO_CD_TRACKS__
#include				"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include				"QDesign Player.h"
#endif

//CONSTANTES:

#define					kSimple						100
#define					kDirectionIconID			200
#define					kThrustIconID				201
#define					kButtonTurboIconID			202
#define					kDirectionThrustIconID		203
#define					kGameButtonIconID			204

#define					kShip_NameResID				400
#define					kGame_NameResID				401

#define					kISpElementLabel_Btn_None	'????'

#define					kSetListResourceID			128

#define					kVolume_Step				16
#define					kVolume_Max					255

#if __ENABLE_AXIS_CONTROL__
enum
{
	kShip_Turbo = 0,
	kShip_Direction,
	kShip_Thrust,
	kShip_NumNeeds
};
#else
enum
{
	kShip_Turbo = 0,
	kShip_DirectionAndThrust,
	kShip_NumNeeds
};
#endif

enum
{
	kGame_ShipDrop = 0,
	kGame_Escape,
	kGame_Camera_Follow,
	kGame_Camera_Head,
	kGame_Camera_Onboard,
	kGame_Camera_Track,
	kGame_Camera_Terrain,
	kGame_Target_Local,
	kGame_Target_1,
	kGame_Target_2,
	kGame_Target_3,
	kGame_Target_4,
	kGame_Target_5,
	kGame_Target_6,
	kGame_Target_7,
	kGame_Target_8,
	kGame_TakeScreenshot,
	kGame_MusicVolumeDown,
	kGame_MusicVolumeUp,
	kGame_FXVolumeDown,
	kGame_FXVolumeUp,
#if __VR_MODE__
	kHead_Roll,
	kHead_Pitch,
	kHead_Yaw,
#endif	
	kGame_NumNeeds
};

#define					kNumNeeds					(kShip_NumNeeds + kGame_NumNeeds)
#define					kRefConOffset				0xFF

//VARIABLES:

static ISpElementReference		theElements[kNumNeeds];
static ISpElementListReference	theList;

//FONCTIONS:

static void InputSprocket_SetKeyboardActivation(Boolean active)
{
	ISpDeviceReference		buffer[kSimple];
	UInt32        			count;

	ISpDevices_ExtractByClass(kISpDeviceClass_Keyboard,kSimple,&count, buffer);

	if(active)
	ISpDevices_Activate(count,buffer);
	else
	ISpDevices_Deactivate(count, buffer);
}

static void InputSprocket_SetMouseActivation(Boolean active)
{
	ISpDeviceReference		buffer[kSimple];
	UInt32        			count;

	ISpDevices_ExtractByClass(kISpDeviceClass_Mouse, kSimple, &count, buffer);
	        
	if(active)
	ISpDevices_Activate(count,buffer);
	else
	ISpDevices_Deactivate(count, buffer);
}

OSErr InputSprocket_Init()
{
	ISpNeed					theNeeds[kNumNeeds];
	ISpNeed*				need;
	OSErr					theError;
	long					i;
	
	theError = ISpStartup();
	if(theError)
	return theError;
	
	//Create Turbo button
	need = &theNeeds[kShip_Turbo];
	GetIndString(need->name, kShip_NameResID, 3);
	need->iconSuiteResourceId = kButtonTurboIconID;
	need->playerNum = 0;
	need->group = 0;
	need->theKind = kISpElementKind_Button;
	need->theLabel = kISpElementLabel_Btn_Fire;
	need->flags = kISpNeedFlag_PolledOnly;
	need->reserved1 = 0;
	need->reserved2 = 0;
	need->reserved3 = 0;
	
	//Create Ship control
#if __ENABLE_AXIS_CONTROL__
	need = &theNeeds[kShip_Direction];
	GetIndString(need->name, kShip_NameResID, 1);
	need->iconSuiteResourceId = kDirectionIconID;
	need->playerNum = 0;
	need->group = 0;
	need->theKind = kISpElementKind_Axis;
	need->theLabel = kISpElementLabel_Axis_XAxis;
	need->flags = kISpNeedFlag_PolledOnly + kISpNeedFlag_NoMultiConfig;
	need->reserved1 = 0;
	need->reserved2 = 0;
	need->reserved3 = 0;
	
	need = &theNeeds[kShip_Thrust];
	GetIndString(need->name, kShip_NameResID, 2);
	need->iconSuiteResourceId = kThrustIconID;
	need->playerNum = 0;
	need->group = 0;
	need->theKind = kISpElementKind_Axis;
	need->theLabel = kISpElementLabel_Axis_YAxis;
	need->flags = kISpNeedFlag_PolledOnly + kISpNeedFlag_NoMultiConfig;
	need->reserved1 = 0;
	need->reserved2 = 0;
	need->reserved3 = 0;
#else
	need = &theNeeds[kShip_DirectionAndThrust];
	GetIndString(need->name, kShip_NameResID, 4);
	need->iconSuiteResourceId = kDirectionThrustIconID;
	need->playerNum = 0;
	need->group = 0;
	need->theKind = kISpElementKind_DPad;
	need->theLabel = kISpElementLabel_Pad_POV;
	need->flags = kISpNeedFlag_PolledOnly + kISpNeedFlag_NoMultiConfig;
	need->reserved1 = 0;
	need->reserved2 = 0;
	need->reserved3 = 0;
#endif

	//Create interface buttons
#if __VR_MODE__
	for(i = 0; i < kHead_Roll; ++i) {
#else
	for(i = 0; i < kGame_NumNeeds; ++i) {
#endif
		need = &theNeeds[kShip_NumNeeds + i];
		GetIndString(need->name, kGame_NameResID, i + 1);
		need->iconSuiteResourceId = kGameButtonIconID;
		need->playerNum = 0;
		need->group = 1;
		need->theKind = kISpElementKind_Button;
		need->theLabel = kISpElementLabel_Btn_None;
		need->flags = kISpNeedFlag_Utility + kISpNeedFlag_EventsOnly + kISpNeedFlag_NoMultiConfig;
		need->reserved1 = 0;
		need->reserved2 = 0;
		need->reserved3 = 0;
	}
	
#if __VR_MODE__
	//VR Mode
	need = &theNeeds[kShip_NumNeeds + kHead_Roll];	
	BlockMove("\pRoll", need->name, sizeof(Str31));
	need->iconSuiteResourceId = 0;
	need->playerNum = 0;
	need->group = 0;
	need->theKind = kISpElementKind_Axis;
	need->theLabel = kISpElementLabel_Axis_Rz;
	need->flags = kISpNeedFlag_PolledOnly + kISpNeedFlag_NoMultiConfig;
	need->reserved1 = 0;
	need->reserved2 = 0;
	need->reserved3 = 0;
	
	need = &theNeeds[kShip_NumNeeds + kHead_Pitch];	
	BlockMove("\pPitch", need->name, sizeof(Str31));
	need->iconSuiteResourceId = 0;
	need->playerNum = 0;
	need->group = 0;
	need->theKind = kISpElementKind_Axis;
	need->theLabel = kISpElementLabel_Axis_Rx;
	need->flags = kISpNeedFlag_PolledOnly + kISpNeedFlag_NoMultiConfig;
	need->reserved1 = 0;
	need->reserved2 = 0;
	need->reserved3 = 0;
	
	need = &theNeeds[kShip_NumNeeds + kHead_Yaw];	
	BlockMove("\pYaw", need->name, sizeof(Str31));
	need->iconSuiteResourceId = 0;
	need->playerNum = 0;
	need->group = 0;
	need->theKind = kISpElementKind_Axis;
	need->theLabel = kISpElementLabel_Axis_Ry;
	need->flags = kISpNeedFlag_PolledOnly + kISpNeedFlag_NoMultiConfig;
	need->reserved1 = 0;
	need->reserved2 = 0;
	need->reserved3 = 0;
#endif
	
	//Patch Escape button
	need = &theNeeds[kShip_NumNeeds + kGame_Escape];
	need->theLabel = kISpElementLabel_Btn_Quit;
	need->flags |= kISpNeedFlag_NoConfig;
	
	//Create virtuals
	theError = ISpElement_NewVirtualFromNeeds(kNumNeeds, theNeeds, theElements, 0);
	if(theError)
	return theError;
	
	//Create element list
	theError = ISpElementList_New(0, nil, &theList, 0);
	if(theError)
	return theError;
	
	//Populate element list
#if __VR_MODE__
	for(i = 0; i < kHead_Roll; ++i) {
#else
	for(i = 0; i < kGame_NumNeeds; ++i) {
#endif
		theError = ISpElementList_AddElements(theList, kRefConOffset + i, 1, &theElements[kShip_NumNeeds + i]);
		if(theError)
		return theError;
	}
	
#if __ENABLE_MOUSE_CONTROL__
	InputSprocket_SetMouseActivation(true);
#else
	if(thePrefs.controlFlags & kPref_ControlFlagMouseOn)
	InputSprocket_SetMouseActivation(true);
#endif
	InputSprocket_SetKeyboardActivation(true);
	
	//Init ISp
#if __VR_MODE__
	theError = ISpInit(kNumNeeds, theNeeds, theElements, kCreatorType, 'VRMd', 0, kSetListResourceID, 0);
#else
	theError = ISpInit(kNumNeeds, theNeeds, theElements, kCreatorType, 'vrs5', 0, kSetListResourceID, 0);
#endif
	if(theError)
	return theError;
	
	//Stop data flow
	ISpSuspend();
	
#if __VR_MODE__
	//Configure ISp
	ISpConfigure(nil);
#endif	
	
	return noErr;
}

void InputSprocket_Quit()
{
	ISpStop();
	ISpElement_DisposeVirtual(kNumNeeds, theElements);
	ISpElementList_Dispose(theList);
	ISpShutdown();
}

OSErr InputSprocket_Prepare()
{
	long						i;
	
	for(i = 0; i < kNumNeeds; ++i)
	ISpElement_Flush(theElements[i]);
	
	ISpResume();
	
	return noErr;
}

void InputSprocket_Finished()
{
	ISpSuspend();
}

#if __VR_MODE__
float			headRoll,
				headPitch,
				headYaw;
#endif

void InputSprocket_GetShipInput(float* deviceHPos, float* deviceVPos, Boolean* button)
{
	UInt32					data;
	
#if __ENABLE_AXIS_CONTROL__
	ISpElement_GetSimpleState(theElements[kShip_Direction], &data);
	*deviceHPos = 2.0 * (float) data / (float) kISpAxisMaximum - 1.0;
	
	ISpElement_GetSimpleState(theElements[kShip_Thrust], &data);
	*deviceVPos = 2.0 * (float) data / (float) kISpAxisMaximum - 1.0;
#else
	ISpElement_GetSimpleState(theElements[kShip_DirectionAndThrust], &data);
	switch(data) {
		case kISpPadIdle: *deviceHPos = 0.0; *deviceVPos = 0.0; break;
		case kISpPadLeft: *deviceHPos = -1.0; *deviceVPos = 0.0; break;
		case kISpPadUpLeft: *deviceHPos = -1.0; *deviceVPos = 1.0; break;
		case kISpPadUp: *deviceHPos = 0.0; *deviceVPos = 1.0; break;
		case kISpPadUpRight: *deviceHPos = 1.0; *deviceVPos = 1.0; break;
		case kISpPadRight: *deviceHPos = 1.0; *deviceVPos = 0.0; break;
		case kISpPadDownRight: *deviceHPos = 1.0; *deviceVPos = -1.0; break;
		case kISpPadDown: *deviceHPos = 0.0; *deviceVPos = -1.0; break;
		case kISpPadDownLeft: *deviceHPos = -1.0; *deviceVPos = -1.0; break;
	}
#endif
	
	ISpElement_GetSimpleState(theElements[kShip_Turbo], &data);
	if(data == kISpButtonDown)
	*button = true;
	else
	*button = false;
	
#if __VR_MODE__
	{
		long			data;
		
		ISpElement_GetSimpleState(theElements[kShip_NumNeeds + kHead_Roll], (unsigned long*) &data);
		headRoll = 180.0 * (float) data / (float) kISpAxisMiddle;
		ISpElement_GetSimpleState(theElements[kShip_NumNeeds + kHead_Pitch], (unsigned long*) &data);
		headPitch = 180.0 * (float) data / (float) kISpAxisMiddle;
		ISpElement_GetSimpleState(theElements[kShip_NumNeeds + kHead_Yaw], (unsigned long*) &data);
		headYaw = -180.0 * (float) data / (float) kISpAxisMiddle;
		
		headRoll = DegreesToRadians(headRoll);
		headPitch = DegreesToRadians(headPitch);
		headYaw = DegreesToRadians(headYaw);
	}
#endif
}

void InputSprocket_HandleGameInput()
{
	Boolean				wasEvent;
	ISpElementEvent		theEvent;
	long				num;
	
	//Read an event if any
	if(ISpElementList_GetNextEvent(theList, sizeof(ISpElementEvent), &theEvent, &wasEvent) != noErr)
	return;
	if(!wasEvent)
	return;
	if(theEvent.data != kISpButtonDown)
	return;
	
	//Dispatch event
	switch(theEvent.refCon - kRefConOffset) {
		
		case kGame_ShipDrop:
		if((gameConfig.gameMode == kGameMode_Replay) || (localShip == nil))
		break;
		Physic_DropShip(localShip, localShip->lastCheckPoint, kString_Drop_Manual);
		break;
		
		case kGame_Escape:
		if((gameConfig.gameMode == kGameMode_Replay) || (gameConfig.gameMode == kGameMode_Practice) || (localShip && !localShip->player->finished))
		run = false;
		else
		forceQuit = true;
		break;
		
		case kGame_Camera_Follow:
		if(!shipCount)
		break;
		if(gameConfig.gameMode == kGameMode_Replay)
		DriverCamera_Chase_Init();
		else
		DriverCamera_Follow_Init();
		break;
		
		case kGame_Camera_Head:
		if(!shipCount || (cameraTargetShip->headShape == nil))
		break;
		DriverCamera_PilotHead_Init();
		break;
		
		case kGame_Camera_Onboard:
		if(!shipCount || !cameraTargetShip->megaObject.cameraCount)
		break;
		DriverCamera_OnBoard_Init();
		break;
		
		case kGame_Camera_Track:
		if(!shipCount)
		break;
		DriverCamera_Track_Init();
		break;
		
		case kGame_Camera_Terrain:
		if(!shipCount || !theTerrain.cameraCount)
		break;
		DriverCamera_TV_Init();
		break;
		
		case kGame_TakeScreenshot:
		Take_ScreenShot();
		break;
		
		case kGame_Target_Local:
		case kGame_Target_1:
		case kGame_Target_2:
		case kGame_Target_3:
		case kGame_Target_4:
		case kGame_Target_5:
		case kGame_Target_6:
		case kGame_Target_7:
		case kGame_Target_8:
		if(!shipCount)
		break;
		if(theEvent.refCon - kRefConOffset == kGame_Target_Local) {
			if(localShip == nil)
			break;
			
			num = localShip->shipNum;
		}
		else {
			num = theEvent.refCon - kRefConOffset - kGame_Target_1;
			if(num >= shipCount)
			num = shipCount - 1;
		}
		switch(cameraMode) {
			
			case kCameraMode_Follow:
			case kCameraMode_Chase:
			case kCameraMode_Track:
#if !__TV_TARGET_FIRST__
			case kCameraMode_TV:
#endif
			cameraTargetShip = &shipList[num];
			break;
			
			case kCameraMode_Head:
			if(shipList[num].headShape != nil)
			cameraTargetShip = &shipList[num];
			break;
			
			case kCameraMode_OnBoard:
			if(shipList[num].megaObject.cameraCount) {
				cameraMode = 0; //Hack
				cameraTargetShip = &shipList[num];
				DriverCamera_OnBoard_Init();
			}
			break;
			
			/*case kCameraMode_FlyThrough:
			case kCameraMode_Fake:
			
			break;*/
			
		}
		break;
		
		case kGame_MusicVolumeDown:
		if(thePrefs.musicVolume > kVolume_Step)
		thePrefs.musicVolume -= kVolume_Step;
		else
		thePrefs.musicVolume = 0;
#if __USE_AUDIO_CD_TRACKS__
		AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
		QDPlayer_Volume(thePrefs.musicVolume);
#endif
		break;
		
		case kGame_MusicVolumeUp:
		if(thePrefs.musicVolume < kVolume_Max - kVolume_Step)
		thePrefs.musicVolume += kVolume_Step;
		else
		thePrefs.musicVolume = kVolume_Max;
#if __USE_AUDIO_CD_TRACKS__
		AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
		QDPlayer_Volume(thePrefs.musicVolume);
#endif
		break;
		
		case kGame_FXVolumeDown:
		if(thePrefs.soundFXVolume > kVolume_Step)
		thePrefs.soundFXVolume -= kVolume_Step;
		else
		thePrefs.soundFXVolume = 0;
		InfinityAudio_Volume(thePrefs.soundFXVolume);
		break;
		
		case kGame_FXVolumeUp:
		if(thePrefs.soundFXVolume < kVolume_Max - kVolume_Step)
		thePrefs.soundFXVolume += kVolume_Step;
		else
		thePrefs.soundFXVolume = kVolume_Max;
		InfinityAudio_Volume(thePrefs.soundFXVolume);
		break;
		
	}
}
