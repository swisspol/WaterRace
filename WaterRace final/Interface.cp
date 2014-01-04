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


#include					<QD3DAcceleration.h>

#include					"WaterRace.h"
#include					"Data Files.h"
#include					"Interface.h"
#include					"Play Modes.h"
#include					"Preferences.h"
#include					"Network Engine.h"

#include					"Infinity Rendering.h"
#include					"Infinity Audio Engine.h"

#include					"Infinity Player.h"
#include					"Textures.h"

#include					"Dialog Utils.h"
#include					"Numeric Utils.h"

#if __USE_AUDIO_CD_TRACKS__
#include					"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include					"QDesign Player.h"
#endif

//VARIABLES:

PicHandle					buttonsPic[kMaxNbButtons][2];
ButtonListHandle			buttonsData;
TE_FontSpecPtr				specs;
GWorldPtr					alphaGWorld;

//ROUTINES:

void Interface_GetLeahSpec(FSSpec* fileSpec)
{
	short			oldResFile = CurResFile();
	
	UseResFile(mainResFileID);
	fileSpec->vRefNum = coreFolder.vRefNum;
	fileSpec->parID = coreFolder.parID;
	GetIndString(fileSpec->name, folderFileNameResID, leahFileName);
	UseResFile(oldResFile);
}

void Interface_ButtonDraw(short num, short state)
{
	Rect					buttonRect;
	
	buttonRect = (**buttonsPic[num][0]).picFrame;
	OffsetRect(&buttonRect, (**buttonsData).buttonList[num].where.h - buttonRect.left, (**buttonsData).buttonList[num].where.v - buttonRect.top);
	if(state == kActive)
	DrawPicture(buttonsPic[num][1], &buttonRect);
	else if(state == kOver)
	DrawPicture(buttonsPic[num][0], &buttonRect);
	else
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &buttonRect, &buttonRect, srcCopy, nil);
}

void Interface_SetCharacterParametersFromResource(short resID)
{
	Handle				camera;
	float				param1,
						param2,
						param3,
						param4,
						x,
						y,
						z,
						rx,
						ry,
						rz,
						zoom,
						hither,
						yon;
	unsigned char*		theChar;
						
	//Gather camera settings
	UseResFile(interfaceResFileID);
	camera = Get1Resource(kCameraResType, resID);
	if(camera == nil)
	return;
	DetachResource(camera);
	HLock(camera);
	theChar = (unsigned char*) *camera;
	
	//Extract light
	StringToFloat(theChar, &param1);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &param2);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &param3);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &param4);
	theChar += sizeof(Str31);
	
	//Extract camera
	StringToFloat(theChar, &x);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &y);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &z);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &rx);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &ry);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &rz);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &zoom);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &hither);
	theChar += sizeof(Str31);
	StringToFloat(theChar, &yon);
	theChar += sizeof(Str31);
	
	//Setup
	InfinityPlayer_SetLight(param1, param2, DegreesToRadians(param3), DegreesToRadians(param4));
	InfinityPlayer_SetCamera(x, y, z, DegreesToRadians(rx), DegreesToRadians(ry), DegreesToRadians(rz), zoom);
	InfinityPlayer_SetClipping(hither, yon);
	
	DisposeHandle(camera);
}

OSErr Interface_LoadAnimationWrapper(FSSpec* file, OSType textureSetID, short posH, short posV, short width, short height, short cameraResID)
{
	TQAEngine*			theEngine;
	OSErr				theError;
	long				vram;
	Boolean				useBitmap = false;
	unsigned long		textureFlags = 0,
						contextFlags = 0;
	long				vendorID,
						engineID;
						
	//Find the Rave engine
	theEngine = Get_InterfaceEngine();
	if(theEngine == nil)
	return kError_NoRaveEngineFound;
	
	QAEngineGestalt(theEngine, kQAGestalt_VendorID, &vendorID);
	QAEngineGestalt(theEngine, kQAGestalt_EngineID, &engineID);
			
	//Make sure the Apple engine is not selected
	if((vendorID == kQAVendor_Apple) && (engineID == kQAEngine_AppleSW)) {
		;
	}
	else { //setup params
		//Get available VRAM
		vram = InfinityRendering_AvailableVRAM(theEngine);
		if(vram <= 0)
		return kError_VRAMError;
		
		if(thePrefs.engineFlags & kPref_EngineFlagFiltering)
		contextFlags |= kInfinityRenderingFlag_Filtering;
		
		if(thePrefs.textureFlags & kTextureFlag_3DfxMode)
		textureFlags |= kTextureFlag_3DfxMode;
		
		if(vram > 3400) { //More than 3.4Mb available
			contextFlags |= kInfinityRenderingFlag_Aliasing;
			useBitmap = true;
		}
		else if(vram > 1600) { //More than 1.6Mb available
			useBitmap = true;
		}
		else if(vram > 1200) { //More than 1.2Mb available
			;
		}
		else { //Less than 800Kb available
			textureFlags |= kTextureFlag_Reduce4X;
		}
		
		if(thePrefs.textureFlags & kTextureFlag_3DfxMode)
		useBitmap = false;
	}
	
	//Load animation
	theError = InfinityPlayer_LoadAnimation(mainGDHandle, theEngine, file, textureSetID,
		mainWinOffset.h + posH, mainWinOffset.v + posV, width, height, contextFlags, 
		textureFlags, bufferGWorld->portPixMap, posH, posV, useBitmap);
	if(theError)
	return theError;
	
	//Place character & camera
	Interface_SetCharacterParametersFromResource(cameraResID);
	
	return noErr;
}

OSErr Interface_RunLocalGame(PilotPtr pilot)
{
	OSErr				theError;
	Boolean				loop = true;
	OSType				gameMode,
						terrainID,
						shipID;
						
	//Sort levels and ships according to difficulty level
	PlayMode_SortDataSet(pilot->mode, pilot->difficulty);
	
	//Stay in browser
	Browser_Reset();
	do {
		theError = Browser_Display(&gameMode, pilot, &terrainID, &shipID);
		if(theError) {
			if(theError != kError_UserCancel) {
				Wrapper_Error_Display(150, theError, nil, __FORCE_QUIT_ON_ERROR__);
				return theError;
			}
			else
			loop = false;
		}
		else {
			theError = Game_SetupLocal(gameMode, pilot, terrainID, shipID);
			if(theError) {
				Wrapper_Error_Display(171, theError, nil, __FORCE_QUIT_ON_ERROR__);
				loop = false;
			}
			else {
				theError = Game_New();
				if(theError)
				loop = false;
				else if(gameMode != kGameMode_Practice) {
					Pilot_Update(pilot);					
#if !__EXPO_MODE__
					//Save / Update pilot file
					theError = Pilot_Save(pilot);
					if(theError)
					Wrapper_Error_Display(151, theError, nil, false);
#endif

#if !__DEMO_MODE__
					//Check if champion
					if(pilot->status == kStatus_Champion) {
						if(pilot->mode == kPilotType_Tournament)
						theError = ChampionMenu_Display(&leahCharacterData, true);
						else
						theError = ChampionMenu_Display(GetCharacterData_ByID(&coreData, pilot->character), false);
						if(theError)
						Wrapper_Error_Display(178, theError, nil, __FORCE_QUIT_ON_ERROR__);
						loop = false;
					}
#endif
				}
			}
		}
	} while(loop);
	
#if !__EXPO_MODE__
	//Save / Update pilot file
	theError = Pilot_Save(pilot);
	if(theError)
	Wrapper_Error_Display(151, theError, nil, false);
#endif

	return noErr;
}

void Interface_Display()
{
	OSErr				theError;
	Boolean				loop = true;
	short				button;
	Str63				name;
	OSType				characterID,
						mode;
	short				difficulty;
	Pilot				thePilot;
	
	//Go!
	while(loop) {
		theError = MainMenu_Display(&button, name, &mode, &difficulty);
		if(theError) {
			Wrapper_Error_Display(152, theError, nil, __FORCE_QUIT_ON_ERROR__);
			return;
		}
		switch(button) {
			
			case kMainButton_NewPilot:
			theError = CharacterMenu_Display(&characterID);
			if(theError) {
				if(theError == kError_UserCancel)
				break;
				Wrapper_Error_Display(153, theError, nil, __FORCE_QUIT_ON_ERROR__);
				return;
			}
			Pilot_New(&thePilot, name, characterID, mode, difficulty);
			Interface_RunLocalGame(&thePilot);
			break;
			
			case kMainButton_LoadPilot:
			theError = PilotMenu_Display(name);
			if(theError) {
				if(theError == kError_UserCancel)
				break;
				Wrapper_Error_Display(154, theError, nil, __FORCE_QUIT_ON_ERROR__);
				return;
			}
			theError = Pilot_Load(name, &thePilot);
			if(theError) {
				Wrapper_Error_Display(155, theError, nil, false);
				return;
			}
			Interface_RunLocalGame(&thePilot);
			break;
			
			case kMainButton_NetGame:
#if __DEMO_MODE__
			Wrapper_Error_Display(179, -1, nil, false);
#else
			if(networkAvailable) {
				theError = NetworkMenu_Display();
				if(theError) {
					if(theError == kError_UserCancel)
					break;
					Wrapper_Error_Display(140, theError, nil, __FORCE_QUIT_ON_ERROR__);
					return;
				}
				theError = Game_New();
			}
			else
			Wrapper_Error_Display(141, kNetError_NoNetworkAvailable, nil, false);
#endif
			break;
			
			case kMainButton_Options:
			Preferences_Set(&thePrefs);
#if __USE_AUDIO_CD_TRACKS__
			AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
			QDPlayer_Volume(thePrefs.musicVolume);
#endif
			InfinityAudio_Volume(thePrefs.soundFXVolume);
			break;
			
			case kMainButton_Credits:
			theError = Credits_Display();
			if(theError) {
				Wrapper_Error_Display(156, theError, nil, __FORCE_QUIT_ON_ERROR__);
				return;
			}
			break;
			
			case kMainButton_Quit:
			loop = false;
			break;
			
		}
	}
}
