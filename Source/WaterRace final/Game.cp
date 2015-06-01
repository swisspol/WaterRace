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


#include				<fp.h>
#include				<OpenTransport.h>

#include				"WaterRace.h"
#include				"Strings.h"
#include				"Data Files.h"
#include				"Drivers.h"
#include				"ISpRoutines.h"
#include				"Interface.h"
#include				"Play Modes.h"
#include				"Preferences.h"
#include				"Network Engine.h"

#include				"Infinity Audio Engine.h"
#include				"Vector.h"
#include				"Clock.h"
#include				"Infinity Rendering.h"

#include				"Camera Utils.h"
#include				"Model Animation Utils.h"
#include				"Cinematic Utils.h"
#include				"Shape Utils.h"
#include				"Terrain Utils.h"

#if __USE_AUDIO_CD_TRACKS__
#include				"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include				"QDesign Player.h"
#endif

#include				"Keys.h"
#include				"Sound Utils.h"

#if __RECORD_RACE__
#include				"Folder Utils.h"
#endif

#if __DISPLAY_FPS__
#include				"Numeric Utils.h"
#endif

#if	__CD_PROTECTION__
#include				"CD Protection.h"
#endif

//CONSTANTES:

#define						kCameraDistanceFromStartLine	15.0

#define						kNumLoadSteps					4

#define						kArrowShapeID					128

#define						kMaxDistanceBetweenChecks		500.0

#define						kStartLineOffset				-10.0

#define						kSlowMotionSpeed				1
#define						kFastForwardSpeed				50
#define						kReplayVariableSpeedDelay		(kTimeUnit / 25)

//MACROS:

#define						RaceTimeTolerance_Easy			(7 * kTimeUnit)
#define						RaceTimeTolerance_Medium		(3 * kTimeUnit)
#define						RaceTimeTolerance_Hard			(0 * kTimeUnit)

#define	RaceTime_Easy(t)	((t) + (t) * 0.14) //14%
#define	RaceTime_Medium(t)	((t) + (t) * 0.07) //7%
#define	RaceTime_Hard(t)	(t)

//VARIABLES:

ExtendedShipPtr				localShip;
GameConfiguration			gameConfig;
Handle						gameFXSounds[kNbGameFX];
Boolean						run,
							forceQuit;

static VDSwitchInfoRec		beforeRaveSwitchInfo;

//ROUTINES:

static OSErr Game_SetUpScreen()
{
	OSErr					theError;
	VDSwitchInfoRec*		switchInfoPtr;
	VDSwitchInfoRec			switchInfo;
	short					realWidth = thePrefs.renderWidth,
							realHeight = thePrefs.renderHeight,
							realDepth;
	
	if(thePrefs.engineFlags & kPref_EngineFlag32BitRendering)
	realDepth = 32;
	else
	realDepth = 16;
	
#if !__VR_MODE__
	//Do we already have a screen resolution and is it still available?
	if(!redefineScreens && (thePrefs.renderWidth == thePrefs.gameWidth) && (thePrefs.renderHeight == thePrefs.gameHeight)  && (realDepth == thePrefs.gameDepth) 
		&& (thePrefs.gameScreen.csMode != 0) && (SL_ModeAvailable(mainGDHandle, &thePrefs.gameScreen)))
	BlockMove(&thePrefs.gameScreen, &switchInfo, sizeof(VDSwitchInfoRec));
	else {
		//User want to choose refresh rate
		if(redefineScreens) {
			//Ask the user a refresh rate
			theError = SL_GetDeviceResolution(screenInfo, &realWidth, &realHeight, realDepth, &switchInfoPtr, kString_Interface_ManualResolution);
			if(theError) {
				Wrapper_Error_Display(130, theError, nil, false);
				return theError;
			}
			BlockMove(switchInfoPtr, &switchInfo, sizeof(VDSwitchInfoRec));
			redefineScreens = false;
		}
		else { //automatic choice
			theError = SL_LookForResolution(mainGDHandle, &realWidth, &realHeight, realDepth, &switchInfo);
			if(theError) {
				Wrapper_Error_Display(130, theError, nil, false);
				return theError;
			}
		}
		
		//Save the switch info
		BlockMove(&switchInfo, &thePrefs.gameScreen, sizeof(VDSwitchInfoRec));
		thePrefs.gameWidth = thePrefs.renderWidth;
		thePrefs.gameHeight = thePrefs.renderHeight;
		thePrefs.gameDepth = realDepth;
	}
#endif
	
	//Save screen state
	theError = SL_GetCurrent(mainGDHandle, &beforeRaveSwitchInfo);
	if(theError) {
		Wrapper_Error_Display(131, theError, nil, false);
		return theError;
	}
	
	//Fade in...
#if __SCREEN_FADES__
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
	AudioCD_SetVolume(0, 0);
#elif __USE_AUDIO_FILES__
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, QDChannel, 0);
#else
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
#endif
	
#if !__VR_MODE__
	//Set our screen
	theError = SL_Switch(mainGDHandle, &switchInfo);
	if(theError) {
		Wrapper_Error_Display(132, theError, nil, false);
		return theError;
	}
#endif
	
	//Move interface window
	mainWinOffset.h = ((**mainGDHandle).gdRect.right - (**mainGDHandle).gdRect.left - kInterface_Width) / 2;
	mainWinOffset.v = ((**mainGDHandle).gdRect.bottom - (**mainGDHandle).gdRect.top - kInterface_Height) / 2;
	MoveWindow(mainWin, mainWinOffset.h + (**mainGDHandle).gdRect.left, mainWinOffset.v + (**mainGDHandle).gdRect.top, false);
	SetPort(mainWin);
	PaintRect(&(backgroundWin->portRect));
	
	//Resize & Clear windows
	SetPort(backgroundWin);
	PaintRect(&(backgroundWin->portRect));
	
	//Calculate offset
#if 0
	screenOffset.h = (realWidth - thePrefs.renderWidth) / 2;
	screenOffset.v = (realHeight - thePrefs.renderHeight) / 2;
	screenResolution.h = realWidth;
	screenResolution.v = realHeight;
#else
	screenOffset.h = ((**mainGDHandle).gdRect.right - (**mainGDHandle).gdRect.left - thePrefs.renderWidth) / 2 + (**mainGDHandle).gdRect.left;
	screenOffset.v = ((**mainGDHandle).gdRect.bottom - (**mainGDHandle).gdRect.top - thePrefs.renderHeight) / 2 + (**mainGDHandle).gdRect.top;
	screenResolution.h = (**mainGDHandle).gdRect.right - (**mainGDHandle).gdRect.left;
	screenResolution.v = (**mainGDHandle).gdRect.bottom - (**mainGDHandle).gdRect.top;
#endif

	return noErr;
}

static OSErr Game_RestoreScreen()
{
	OSErr				theError;
#if __FADE_TO_WHITE_AFTER_RACE__
	RGBColor			white = {0xFFFF, 0xFFFF, 0xFFFF};
#endif
	
	//Fade in...
#if __SCREEN_FADES__
#if __FADE_TO_WHITE_AFTER_RACE__
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToColor(mainGDHandle, &white, kGammaSpeed + 3, quadraticFade, nil, 0);
	AudioCD_SetVolume(0, 0);
#elif __USE_AUDIO_FILES__
	Fade1ToColor(mainGDHandle, &white, kGammaSpeed + 3, quadraticFade, QDChannel, 0);
#else
	Fade1ToColor(mainGDHandle, &white, kGammaSpeed + 3, quadraticFade, nil, 0);
#endif
#else
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToBlack(mainGDHandle, kGammaSpeed + 3, quadraticFade, nil, 0);
	AudioCD_SetVolume(0, 0);
#elif __USE_AUDIO_FILES__
	Fade1ToBlack(mainGDHandle, kGammaSpeed + 3, quadraticFade, QDChannel, 0);
#else
	Fade1ToBlack(mainGDHandle, kGammaSpeed + 3, quadraticFade, nil, 0);
#endif
#endif
#endif
	
#if !__VR_MODE__
	//Restore screen state
	theError = SL_Switch(mainGDHandle, &beforeRaveSwitchInfo);
	if(theError) {
		Wrapper_Error_Display(132, theError, nil, false);
		return theError;
	}
#endif
	
	//Refresh windows
	SetPort(backgroundWin);
	PaintRect(&(backgroundWin->portRect));
	
	//Move interface window
	mainWinOffset.h = ((**mainGDHandle).gdRect.right - (**mainGDHandle).gdRect.left - kInterface_Width) / 2;
	mainWinOffset.v = ((**mainGDHandle).gdRect.bottom - (**mainGDHandle).gdRect.top - kInterface_Height) / 2;
	MoveWindow(mainWin, mainWinOffset.h + (**mainGDHandle).gdRect.left, mainWinOffset.v + (**mainGDHandle).gdRect.top, false);
	SetPort(mainWin);
	PaintRect(&(backgroundWin->portRect));
	
	return noErr;
}

static OSErr Game_LoadShapes()
{
	OSErr					theError;
	
	//Load arrow shape
	theError = Shape_LoadFromResource(&arrowShape, kArrowShapeID);
	if(theError)
	return theError;
	
	//Resolve rendering
	Shape_ResolveRendering(arrowShape);

	return noErr;
}

static void Game_UnloadShapes()
{
	Shape_Dispose(arrowShape);
}

static void Compute_ShipPositions(Vector startPosition[kMaxPlayers], VectorPtr left, VectorPtr right, VectorPtr direction)
{
	long					numLines,
							numShips,
							i,
							j,
							c = 0;
	Vector					startLine;
	long					lines[kMaxPlayers];
	long					numShipsPerLine;
	
	//Compute number of ships per line
	if(gameConfig.playerCount == 4)
	numShipsPerLine = 2;
	else
	numShipsPerLine = 3;
	
	//Calculate number of lines
	numLines = gameConfig.playerCount / numShipsPerLine;
	if(gameConfig.playerCount % numShipsPerLine)
	++numLines;
	
	//Fill lines
	for(i = 0; i < numLines; ++i) {
		numShips = numShipsPerLine;
		if((i == numLines - 1) && (gameConfig.playerCount % numShipsPerLine))
		numShips = gameConfig.playerCount % numShipsPerLine;
		
		Vector_Subtract(left, right, &startLine);
		Vector_Multiply(1.0 / (float) numShips, &startLine, &startLine);
		for(j = 0; j < numShips; ++j, ++c) {
			Vector_MultiplyAdd(j + 0.5, &startLine, right, &startPosition[c]);
			if(i)
			Vector_MultiplyAdd(i * kStartLineOffset, direction, &startPosition[c], &startPosition[c]);
			startPosition[c].y = 0;
			
			lines[c] = i;
		}
	}
	
	//Randomize positions
	if(gameConfig.gameMode == kGameMode_Local) {
		GetDateTime((unsigned long*) &qd.randSeed);
		for(i = 0; i < gameConfig.playerCount; ++i) {
			long num = (Random() + 32767) % gameConfig.playerCount;
			Vector position = startPosition[i];
			startPosition[i] = startPosition[num];
			startPosition[num] = position;
			long temp = lines[i];
			lines[i] = lines[num];
			lines[num] = temp;
		}
	}

#if __ADJUST_LOCAL_PLAYER_POSITION__
	//Make sure local player position is correct - We assume local player is always the first in the list
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.playerList[0].shipPilotType == kPilot_Local)) {
		if((gameConfig.terrainDifficulty == kDifficulty_Hard) && (lines[0] != numLines - 1)) {
			for(i = 1; i < gameConfig.playerCount; ++i)
			if(lines[i] == numLines - 1) {
				Vector position = startPosition[0];
				startPosition[0] = startPosition[i];
				startPosition[i] = position;
				break;
			}
		}
		else if((gameConfig.terrainDifficulty != kDifficulty_Hard) && (lines[0] != 0)) {
			for(i = 1; i < gameConfig.playerCount; ++i)
			if(lines[i] == 0) {
				Vector position = startPosition[0];
				startPosition[0] = startPosition[i];
				startPosition[i] = position;
				break;
			}
		}
	}
#endif
}

inline void QuitNetworkGame()
{
#if !__DEMO_MODE__
	if(gameConfig.gameMode == kGameMode_Host)
	Server_ShutDown();
	else if((gameConfig.gameMode == kGameMode_Join) || (gameConfig.gameMode == kGameMode_BroadCastTV))
	Client_Disconnect();
#endif
}

static OSErr Game_Start()
{
	OSErr					theError;
	long					i;
	Point					size,
							offset;
	Vector					startLine,
							position,
							zVector = {0.0, 0.0, 1.0},
							direction;
	float					rotateY;
	TerrainPreloadedDataPtr	terrainData;
	ShipPreloadedDataPtr	shipData;
	Vector					startPosition[kMaxPlayers];
	
	//Find terrain by ID
	switch(gameConfig.gameMode) {
		
		case kGameMode_Local:
		if(gameConfig.terrainID == leahTerrainData.ID)
		terrainData = &leahTerrainData;
		else
		terrainData = PlayMode_GetTerrain(gameConfig.gameSubMode, gameConfig.terrainID);
#if __USE_INI__
		if(gameConfig.terrainID == practiceTerrainData.ID)
		terrainData = &practiceTerrainData;
#endif
		break;
		
		case kGameMode_Practice:
		terrainData = &practiceTerrainData;
		break;
		
		case kGameMode_Host:
		case kGameMode_Join:
		case kGameMode_BroadCastTV:
		case kGameMode_Replay:
		if(gameConfig.terrainID == leahTerrainData.ID)
		terrainData = &leahTerrainData;
		else {
			terrainData = GetTerrainData_ByID(&coreData, gameConfig.terrainID);
			if(terrainData == nil)
			terrainData = GetTerrainData_ByID(&addOnData, gameConfig.terrainID);
		}
		break;
		
	}
	if(terrainData == nil) {
		Wrapper_Error_Display(147, kError_TerrainNotFound, nil, __FORCE_QUIT_ON_ERROR__);
		return kError_TerrainNotFound;
	}
	
	//Setup loading screen
	theError = Progress_Init(&terrainData->fileSpec, terrainData->description.location_H, terrainData->description.location_V);
	if(theError) {
		Wrapper_Error_Display(170, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	//Calculate rendering zone
	size.h = thePrefs.renderWidth;
	size.v = thePrefs.renderHeight;
	if(thePrefs.renderWidth > screenResolution.h)
	size.h = screenResolution.h;
	if(thePrefs.renderHeight > screenResolution.v)
	size.v = screenResolution.v;
	offset.h = (screenResolution.h - size.h) / 2;
	offset.v = (screenResolution.v - size.v) / 2;

	//Init engine
#if __MACOS_X_CLASSIC__
	GrafPtr		savePort;
	GetPort(&savePort);
	SetPort(mainWin);
#endif
	theError = Infinity_GameInit(Get_GameEngine(), &worldState, size, offset);
	if(theError) {
		Wrapper_Error_Display(146, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
#if __MACOS_X_CLASSIC__
	SetPort(savePort);
#endif
	
	//Start game music
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_Stop();
#elif __USE_AUDIO_FILES__
	QDPlayer_Stop();
#endif
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
	if(thePrefs.soundFlags & kPref_SoundFlagGameMusic) {
		//Get music file name
		BlockMove(terrainData->description.musicName, musicsFolder.name, sizeof(Str63));
		
		//Play music
#if __USE_AUDIO_CD_TRACKS__
		theError = AudioCD_PlayAudioTrack_Wrapper(&musicsFolder);
#elif __USE_AUDIO_FILES__
		theError = QDPlayer_Start(&musicsFolder);
#endif
		if(theError)
		Wrapper_Error_Display(143, theError, nil, false);
		else
#if __USE_AUDIO_CD_TRACKS__
		AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
		QDPlayer_Volume(thePrefs.musicVolume);
#endif
	}
#endif
	
#if __SCREEN_FADES__
	//Fade out
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	//Load terrain by ID
	theError = World_Load(worldState, &terrainData->fileSpec, gameConfig.terrainDifficulty, kNoFlyThrough);
	if(theError) {
		Wrapper_Error_Display(147, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	//Update progress
	Progress_Update(1, kNumLoadSteps);

	//Calculate ship direction
	direction.x = (theRace->doorList[1].leftMark.x + theRace->doorList[1].rightMark.x) / 2.0 - (theRace->doorList[0].leftMark.x + theRace->doorList[0].rightMark.x) / 2.0;
	direction.y = 0.0;
	direction.z = (theRace->doorList[1].leftMark.z + theRace->doorList[1].rightMark.z) / 2.0 - (theRace->doorList[0].leftMark.z + theRace->doorList[0].rightMark.z) / 2.0;
	Vector_Normalize(&direction, &direction);
	rotateY = Vector_DotProduct(&direction, &zVector);
	if(direction.x < 0.0)
	rotateY = -acos(rotateY);
	else
	rotateY = acos(rotateY);
	
	//Calculate ship positions
	Compute_ShipPositions(startPosition, &theRace->doorList[0].leftMark, &theRace->doorList[0].rightMark, &direction);
	
	//Load ships by ID
	for(i = 0; i < gameConfig.playerCount; ++i) {
		//Find ship
		switch(gameConfig.gameMode) {
			
			case kGameMode_Local:
			if(gameConfig.playerList[i].shipID == leahShipData.ID)
			shipData = &leahShipData;
			else
			shipData = PlayMode_GetShip(gameConfig.gameSubMode, gameConfig.playerList[i].shipID);
			break;
			
			case kGameMode_Practice:
			case kGameMode_Host:
			case kGameMode_Join:
			case kGameMode_BroadCastTV:
			case kGameMode_Replay:
			if(gameConfig.playerList[i].shipID == leahShipData.ID)
			shipData = &leahShipData;
			else {
				shipData = GetShipData_ByID(&coreData, gameConfig.playerList[i].shipID);
				if(shipData == nil)
				shipData = GetShipData_ByID(&addOnData, gameConfig.playerList[i].shipID);
			}
			break;
			
		}
		if(shipData == nil) {
			Wrapper_Error_Display(159, kError_ShipNotFound, nil, __FORCE_QUIT_ON_ERROR__);
			QuitNetworkGame();
			World_Unload();
			return kError_ShipNotFound;
		}
		
		//Load ship
		theError = World_InstallShip(&shipData->fileSpec, &gameConfig.playerList[i], &startPosition[i], rotateY);
		if(theError) {
			Wrapper_Error_Display(159, theError, nil, __FORCE_QUIT_ON_ERROR__);
			QuitNetworkGame();
			World_Unload();
			return theError;
		}
	}
	
	//Load special shapes
	theError = Game_LoadShapes();
	if(theError) {
		Wrapper_Error_Display(160, theError, nil, __FORCE_QUIT_ON_ERROR__);
		QuitNetworkGame();
		World_Unload();
		return theError;
	}
	
	//Update progress
	Progress_Update(2, kNumLoadSteps);

	//Setup ship drivers
	localShip = nil;
	for(i = 0; i < shipCount; ++i)
	switch(shipList[i].player->shipPilotType) {
	
		case kPilot_Local:
		DriverShip_PlayerISp_Init(&shipList[i]);
		shipList[i].DriverShipRoutine = (ProcPtr) DriverShip_PlayerISp_Callback;
		localShip = &shipList[i];
		break;
		
		case kPilot_Bot:
		DriverShip_Bot_Init(&shipList[i]);
		shipList[i].DriverShipRoutine = (ProcPtr) DriverShip_Bot_Callback;
		break;
		
		case kPilot_Remote:
		DriverShip_Remote_Init(&shipList[i]);
		shipList[i].DriverShipRoutine = (ProcPtr) DriverShip_Remote_Callback;
		break;
		
		case kPilot_Replay:
		DriverShip_Replay_Init(&shipList[i]);
		shipList[i].DriverShipRoutine = (ProcPtr) DriverShip_Replay_Callback;
		break;
		
	}
	
	//Setup camera
	if(gameConfig.gameMode == kGameMode_Replay) {
		cameraTargetShip = &shipList[0];
		if(theTerrain.cameraCount)
		DriverCamera_TV_Init();
		else
		DriverCamera_Chase_Init();
	}
	else {
		if(shipCount) {
#if __BOT_ONLY__
			cameraTargetShip = &shipList[1];
#else
			if(localShip != nil)
			cameraTargetShip = localShip;
			else
			cameraTargetShip = &shipList[0];
#endif
#if __VR_MODE__
			DriverCamera_PilotHead_Init();
#else
			DriverCamera_Follow_Init();
#endif
		}
		else {
			//Calculate camera position
			Vector_Add(&theRace->doorList[0].rightMark, &theRace->doorList[0].leftMark, &startLine);
			Vector_Multiply(0.5, &startLine, &startLine);
			Vector_Subtract(&theRace->doorList[1].rightMark, &theRace->doorList[0].rightMark, &direction);
			Vector_Normalize(&direction, &direction);
			Vector_Multiply(-kCameraDistanceFromStartLine, &direction, &direction);
			Vector_Add(&startLine, &direction, &position);
			
			worldCamera.camera.w.x = position.x;
			worldCamera.camera.w.y = kCameraStartHeight;
			worldCamera.camera.w.z = position.z;
			worldCamera.roll = 0.0;
			worldCamera.pitch = rotateY;
			worldCamera.yaw = 0.0;
			CameraState_UpdateMatrix(&worldCamera);
			
			DriverCamera_Fake_Init();
		}
	}
	
	//Load sounds
	UseResFile(soundResFileID);
	for(i = 0; i < kNbGameFX; ++i) {
		if(Sound_ResLoad(kGameFXStartID + i, false, &gameFXSounds[i]) != noErr) {
			Wrapper_Error_Display(161, ResError(), nil, __FORCE_QUIT_ON_ERROR__);
			QuitNetworkGame();
			World_Unload();
			Game_UnloadShapes();
			return kError_CantLoadSound;
		}
	}
	
	//Update progress
	Progress_Update(3, kNumLoadSteps);

	//Prepare ISp
	InputSprocket_Prepare();

#if __RECORD_RACE__
	//Prepare recording
	if((gameConfig.gameMode != kGameMode_Replay) && (gameConfig.gameMode != kGameMode_Practice))
	Replay_Init();
#endif
	
	//Synchronize game
	switch(gameConfig.gameMode) {
		
		case kGameMode_Local:
		case kGameMode_Practice:
		RClock_SetTime(0);
		theError = noErr;
		break;
		
		case kGameMode_Replay:
		if(gameConfig.startTime - kRaceCountDown > 0)
		RClock_SetTime(gameConfig.startTime - kRaceCountDown);
		else
		RClock_SetTime(0);
		break;
		
#if !__DEMO_MODE__
		case kGameMode_Host:
		RClock_SetTime(0);
		theError = Server_WaitForClients();
		break;
		
		case kGameMode_Join:
		theError = Client_WaitForServer();
		break;
#endif

	}
	if(theError) {
		if(theError != kError_UserCancel)
		Wrapper_Error_Display(162, theError, nil, false);
		QuitNetworkGame();
		World_Unload();
		Game_UnloadShapes();
		InputSprocket_Finished();
		return theError;
	}
	
	//Make sure cursor is hidden
	HideCursor();
	
	World_InstallAudio();
	World_Prepare();
	
	//Black out & kill progress bar
#if __SCREEN_FADES__
	Fade1ToBlack(mainGDHandle, 1, linearFade, nil, 0);
#endif
	Progress_Quit();

	//Prepare camera
	cameraDriver();
		
	//Reset times
	if(gameConfig.gameMode != kGameMode_Replay) {
		if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace)) {
			gameConfig.globalBestLapTime = terrainData->description.parTime[gameConfig.terrainDifficulty - 1];
			switch(gameConfig.terrainDifficulty) {
				case kDifficulty_Easy: gameConfig.globalBestLapTime += RaceTimeTolerance_Easy; break;
				case kDifficulty_Medium: gameConfig.globalBestLapTime += RaceTimeTolerance_Medium; break;
				case kDifficulty_Hard: gameConfig.globalBestLapTime += RaceTimeTolerance_Hard; break;
			}
		}
		else
		gameConfig.globalBestLapTime = 3600 * kTimeUnit; //1 hour
		gameConfig.startTime = 0;
	}
	
	//Render first frame
	worldState->frameTime = RClock_GetTime();
	Anim_Run(&theTerrain, worldState->frameTime);
	World_UpdateSea();
	Draw_Frame(worldState);
	Draw_Frame(worldState);
	
#if __SCREEN_FADES__
	//Fade out...
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	return noErr;
}

static OSErr Game_Quit()
{
	long				i;
#if __FADE_TO_WHITE_AFTER_RACE__
	RGBColor			white = {0xFFFF, 0xFFFF, 0xFFFF};
#endif
	
#if __SCREEN_FADES__
	//Fade in...
#if __FADE_TO_WHITE_AFTER_RACE__
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToColor(mainGDHandle, &white, kGammaSpeed + 3, quadraticFade, nil, 0);
	AudioCD_SetVolume(0, 0);
#elif __USE_AUDIO_FILES__
	Fade1ToColor(mainGDHandle, &white, kGammaSpeed + 3, quadraticFade, QDChannel, 0);
#else
	Fade1ToColor(mainGDHandle, &white, kGammaSpeed + 3, quadraticFade, nil, 0);
#endif
#else
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToBlack(mainGDHandle, kGammaSpeed + 3, quadraticFade, nil, 0);
	AudioCD_SetVolume(0, 0);
#elif __USE_AUDIO_FILES__
	Fade1ToBlack(mainGDHandle, kGammaSpeed + 3, quadraticFade, QDChannel, 0);
#else
	Fade1ToBlack(mainGDHandle, kGammaSpeed + 3, quadraticFade, nil, 0);
#endif
#endif
#endif

	//Disconnect if network game
	QuitNetworkGame();
	
#if __RECORD_RACE__
	//Dispose recording data
	Replay_Quit();
#endif
	
	//Unload data
	World_Unload();
	Game_UnloadShapes();
	
	//Clean up
	InputSprocket_Finished();
	
	if(thePrefs.soundFlags & kPref_SoundFlagGameMusic)
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_Stop();
#elif __USE_AUDIO_FILES__
	QDPlayer_Stop();
#endif
	
	InfinityAudio_CleanUp(kCreatorType);
	for(i = 0; i < kNbGameFX; ++i)
	DisposeHandle(gameFXSounds[i]);
	
	return noErr;
}

#if __RECORD_RACE__
static void Game_Replay()
{
	long			i,
					j;
	
	//Reset ships
	for(i = 0; i < shipCount; ++i) {
		shipList[i].player->infoText[0] = 0;
		shipList[i].player->networkText[0] = 0;
		InfinityAudio_SetSoundParams(kCreatorType, shipList[i].engineSoundRef, shipList[i].params->power_SoundVolume, shipList[i].params->power_MinSoundPitchRatio);
		
		if(shipList[i].turboState) {
			shipList[i].turboState = false;
			shipList[i].turboFlame = false;
			if(shipList[i].currentScript != nil)
			Script_Stop(shipList[i].currentScript);
			if(shipList[i].turbo)
			Script_DisplayTime(shipList[i].turbo, 0);
			shipList[i].currentScript = nil;
		}
		
		for(j = 0; j < kWakeNumSprites; ++j) {
			Vector_Clear(&shipList[i].wakePositions[j]);
			Vector_Clear(&shipList[i].wakeDirections[j]);
			shipList[i].wakeTime[j] = 0;
		}
		Vector_Clear(&shipList[i].wakeLastPosition);
		
		//shipList[i].player->shipPilotType = kPilot_Replay; - Conflict with Score screen
		DriverShip_Replay_Init(&shipList[i]);
		shipList[i].DriverShipRoutine = (ProcPtr) DriverShip_Replay_Callback;
		shipList[i].lastCheckPoint = 0;
		shipList[i].nextCheckPoint = theRace->doorList[0].nextCheckPoint;
		shipList[i].player->lapCount = 1;
	}
	Replay_Reset();
	
	//Remove bonus
	if(theTerrain.itemReferenceCount) {
		theTerrain.itemReferenceCount = 0;
		ItemList_Dispose(theTerrain.itemReferenceList);
	}
	
	//Replay race
	RClock_SetTime(gameConfig.startTime - kRaceCountDown);
	gameConfig.gameMode = kGameMode_Replay;
	gameConfig.gameSubMode = kGameSubMode_None;
	
	//Reset world
	World_Prepare();
	
	//Select camera
	if(localShip)
	cameraTargetShip = localShip;
	else
	cameraTargetShip = &shipList[0];
	if(theTerrain.cameraCount)
	DriverCamera_TV_Init();
	else
	DriverCamera_Chase_Init();
}
#endif

static Boolean Check_Replay()
{
	KeyMap			theKeys;
#if __RECORD_RACE__
	Boolean			replaySaved = false;
#endif
	
	//Wait until escape key is up
	do {
		GetKeys(theKeys);
	} while(IsKeyDown(theKeys, keyEscape));
	
	do {
		GetKeys(theKeys);
		
		if(IsKeyDown(theKeys, keyEscape))
		return false;
		
		if(IsKeyDown(theKeys, keySpace))
		return true;
		
#if __RECORD_RACE__
		if(IsKeyDown(theKeys, keyReturn) && !replaySaved) {
			FSSpec			file;
			DateTimeRec		time;
			Str31			temp;
			
			GetTime(&time);
			BlockMove(kString_Misc_ReplayFileName, file.name, sizeof(Str31));
			NumToString(time.hour, temp);
			BlockMove(&temp[1], &file.name[file.name[0] + 1], temp[0]);
			file.name[0] += temp[0];
			NumToString(time.minute, temp);
			BlockMove(&temp[1], &file.name[file.name[0] + 1], temp[0]);
			file.name[0] += temp[0];
			NumToString(time.day, temp);
			BlockMove(&temp[1], &file.name[file.name[0] + 1], temp[0]);
			file.name[0] += temp[0];
			NumToString(time.month, temp);
			BlockMove(&temp[1], &file.name[file.name[0] + 1], temp[0]);
			file.name[0] += temp[0];
			NumToString(time.year, temp);
			BlockMove(&temp[1], &file.name[file.name[0] + 1], temp[0]);
			file.name[0] += temp[0];
			
			file.parID = fsRtDirID;
			file.vRefNum = GetStartUpVolume();
			
			if(Replay_Save(&file) != noErr)
			SysBeep(0);
			
			replaySaved = true;
		}
#endif
	} while(1);
	
	return false;
}

static void Game_Over()
{
	long				i;
	
	//Set camera to terrain
	if(theTerrain.cameraCount && (gameConfig.gameMode != kGameMode_Replay)) {
		if(localShip != nil)
		cameraTargetShip = localShip;
		else
		cameraTargetShip = &shipList[0];
		DriverCamera_TV_Init();
		DriverCamera_TV_Callback();
	}
	
	//Hack camera text driver with Score display routine
	cameraTextDriver = GameOverText;
	
	//Render a frame
	Draw_Frame(worldState);
	
#if 1
	//Shut down engine sounds
	for(i = 0; i < shipCount; ++i)
	InfinityAudio_SetSoundParams(kCreatorType, shipList[i].engineSoundRef, shipList[i].params->power_SoundVolume, kInfinityAudio_StoppedPitch);
#endif
	
	//Disconnect if network game
	QuitNetworkGame();
	
#if __RECORD_RACE__
	if(!Check_Replay())
	run = false;
	else
	Game_Replay();
#else
	Check_Replay();
	run = false;
#endif
}

#if __AUTOPILOT__
static void StopShip(ExtendedShipPtr ship)
{
	//Reset driver info
	if(ship->turboState)
	Physic_StopShipTurbo(ship);
	
	DriverShip_AutoPilot_Init(ship);
	ship->DriverShipRoutine = (ProcPtr) DriverShip_AutoPilot_Callback;
	
#if 1
	//Force camera to follow camera
	if(ship == cameraTargetShip)
	DriverCamera_Track_Init();
#endif
}
#else
static void StopShip(ExtendedShipPtr ship)
{
	//Reset interface
	ship->mouseH = 0.0;
	ship->mouseV = 0.0;
	ship->turboButton = false;
	
	//Reset driver info
	ship->wrongDirection = false;
	if(ship->turboState)
	Physic_StopShipTurbo(ship);
	
	//Reset physic info
#if 1
	Vector_Normalize(&ship->dynamics.velocityG, &ship->dynamics.velocityG);
	Vector_Multiply(60.0 / 3.6, &ship->dynamics.velocityG, &ship->dynamics.velocityG); //Set velocity to 60Km/h
#else
	Vector_Clear(&ship->dynamics.velocityG);
#endif
	Vector_Clear(&ship->dynamics.accelerationG);
	Vector_Clear(&ship->dynamics.velocityG);
	Vector_Clear(&ship->dynamics.rotationAccelerationL);
	ship->dynamics.motorPower = ship->params->power_idle;
	ship->dynamics.targetPower = ship->params->power_idle;
}
#endif

void Game_UpdateRank()
{
	float			magic[kMaxPlayers];
	long			num[kMaxPlayers];
	long			i,
					j;
	float			tempMagic;
	long			tempNum;
	
	//Compute magic values - the greater the better
	for(i = 0; i < shipCount; ++i) {
		magic[i] = (shipList[i].player->lapCount * theRace->doorCount + shipList[i].lastCheckPoint + 1) * kMaxRaceTime - shipList[i].checkPointTime;
		num[i] = i;
	}
	
	//Sort ships based on magic values
	for(i = 0; i < shipCount - 1; ++i)
	for(j = 0; j < shipCount - i - 1; ++j)
	if(magic[j + 1] > magic[j]) {
		tempMagic = magic[j];
		magic[j] = magic[j + 1];
		magic[j + 1] = tempMagic;
		
		tempNum = num[j];
		num[j] = num[j + 1];
		num[j + 1] = tempNum;
	}
	
	//Copy back ranks
	for(i = 0; i < shipCount; ++i) {
		for(j = 0; j < shipCount; ++j)
		if(num[j] == i) {
			if((j + 1 == 1) && (shipList[i].player->rank != 1)) {
				BlockMove(kString_Driver_FirstPlace, shipList[i].player->infoText, kString_Driver_FirstPlace_L);
				shipList[i].player->infoMaxTime = worldState->frameTime + kTimeUnit;
				if(&shipList[i] == cameraTargetShip)
				Sound_PlaySFxHandle(gameFXSounds[kGameFX_FirstPlace]);
			}
			shipList[i].player->rank = j + 1;
			break;
		}
	}
}

OSErr Game_Run()
{
	long			i;
	unsigned long	nextRankUpdateTime = 0;
#if __DISPLAY_FPS__
	Str31			frameRate;
	float			framesPerSecond;
	Point			framePoint = {16000 + screenResolution.v, 16000 + screenResolution.h - 45};
	Rect			frameRect = {16000 + screenResolution.v - 15, 16000 + screenResolution.h - 45, 16000 + screenResolution.v, 16000 + screenResolution.h};
	unsigned long	numFrames = 0;
	OTTimeStamp		OTTime;
	unsigned long	ellapsedMilliseconds;
	
	SetPort(backgroundWin);
	ForeColor(redColor);
#endif
	
	//Compute Race starting time
	if(gameConfig.startTime == 0) {
		if(gameConfig.gameMode == kGameMode_Practice)
		gameConfig.startTime = 0;
		else {
			gameConfig.startTime = worldState->frameTime + kRaceCountDown;
#if !__DEMO_MODE__
			if(gameConfig.gameMode == kGameMode_Host)
			Server_SendStartTime();
#endif
			if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace))
			gameConfig.expiredTime = gameConfig.startTime + gameConfig.globalBestLapTime;
			else
			gameConfig.expiredTime = 0;
		}
	}
	for(i = 0; i < shipCount; ++i) {
		shipList[i].motorStartTime = gameConfig.startTime;
		shipList[i].player->lastLapTime = gameConfig.startTime;
	}
	
	//Run game!
	run = true;
	forceQuit = false;
	while(run) {
#if __DISPLAY_FPS__
		OTGetTimeStamp(&OTTime);
#endif
		
		//Get time
		worldState->lastFrameTime = worldState->frameTime;
#if __MOVIE__
		if(gameConfig.gameMode == kGameMode_Replay) {
			float			mouseH,
							mouseV;
			Boolean			slowMotion;

			InputSprocket_GetShipInput(&mouseH, &mouseV, &slowMotion);
			if(slowMotion) {
				Take_ScreenShot();
				RClock_SetTime(worldState->frameTime + 4); //25FPS
			}
		}
#elif __VARIABLE_SPEED_REPLAY__
		if(gameConfig.gameMode == kGameMode_Replay) {
			float			mouseH,
							mouseV;
			Boolean			slowMotion;

			InputSprocket_GetShipInput(&mouseH, &mouseV, &slowMotion);
			if(slowMotion) {
				RClock_Delay(kReplayVariableSpeedDelay);
				RClock_SetTime(worldState->frameTime + kSlowMotionSpeed);
			}
			else if(mouseH > 0.8) {
				RClock_Delay(kReplayVariableSpeedDelay);
				RClock_SetTime(worldState->frameTime + kFastForwardSpeed);
			}
		}
#endif
		worldState->frameTime = RClock_GetTime();
		
#if !__DEMO_MODE__ && __SYNCHRONIZE_NETWORK_GAME__
		if(gameConfig.gameMode == kGameMode_Host)
		Server_SendClockTime();
#endif
		
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Main:Check_KeyBoard");
#endif
		//Check keyboard
		InputSprocket_HandleGameInput();
#if __PROFILE__
		LOG_EXIT; }
#endif
		
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Main:Anim_Run");
#endif
		//Run scripts
		Anim_Run(&theTerrain, worldState->frameTime);
#if __PROFILE__
		LOG_EXIT; }
#endif
		
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Main:World_UpdateSea");
#endif
		//Animate sea
		World_UpdateSea();
#if __PROFILE__
		LOG_EXIT; }
#endif
		//Update ships
		switch(gameConfig.gameMode) {
			
			case kGameMode_Local:
			case kGameMode_Practice:
			if(worldState->frameTime <= worldState->lastFrameTime) //Check for too high FPS
			break;
			{
				long			i;
				unsigned long	time = worldState->lastFrameTime;
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Main:Drive_Ship");
#endif
				//Drive ships
				for(i = 0; i < shipCount; ++i)
#if __AUTOPILOT__
				if(shipList[i].DriverShipRoutine != nil)
#else
				if((shipList[i].DriverShipRoutine != nil) && !shipList[i].player->finished)
#endif
				(*((ShipDriver_Proc) shipList[i].DriverShipRoutine))(&shipList[i]);
#if __PROFILE__
		LOG_EXIT; }
#endif
				
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Main:Physic_UpdateShip");
#endif
				//Update physics
				for(i = 0; i < shipCount; ++i)
				Physic_UpdateShip_Start(&shipList[i]);
				while(time < worldState->frameTime) {
					for(i = 0; i < shipCount; ++i)
					Physic_UpdateShip(&shipList[i], time);
					++time;
				}
				for(i = 0; i < shipCount; ++i)
				Physic_UpdateShip_End(&shipList[i]);
#if __PROFILE__
		LOG_EXIT; }
#endif
			}
			break;
			
#if !__DEMO_MODE__
			case kGameMode_Host:
			if(worldState->frameTime <= worldState->lastFrameTime) //Check for too high FPS
			break;
			{
				long			i;
				unsigned long	time = worldState->lastFrameTime;
				
				//Drive ships
				for(i = 0; i < shipCount; ++i)
#if __AUTOPILOT__
				if(shipList[i].DriverShipRoutine != nil)
#else
				if((shipList[i].DriverShipRoutine != nil) && !shipList[i].player->finished)
#endif
				(*((ShipDriver_Proc) shipList[i].DriverShipRoutine))(&shipList[i]);
				
				//Update physic
				Server_Enter();
				for(i = 0; i < shipCount; ++i) {
					if((shipList[i].player->shipPilotType == kPilot_Local) || (shipList[i].player->shipPilotType == kPilot_Bot))
					Physic_UpdateShip_Start(&shipList[i]);
					else if(shipList[i].player->shipPilotType == kPilot_Remote)
					Physic_UpdateShip_Network_Start(&shipList[i]);
				}
				while(time < worldState->frameTime) {
					for(i = 0; i < shipCount; ++i) {
						if((shipList[i].player->shipPilotType == kPilot_Local) || (shipList[i].player->shipPilotType == kPilot_Bot))
						Physic_UpdateShip(&shipList[i], time);
						else if(shipList[i].player->shipPilotType == kPilot_Remote)
						Physic_UpdateShip_Network(&shipList[i], time);
					}
					++time;
				}
				for(i = 0; i < shipCount; ++i) {
					if((shipList[i].player->shipPilotType == kPilot_Local) || (shipList[i].player->shipPilotType == kPilot_Bot)) {
						Physic_UpdateShip_End(&shipList[i]);
						Server_SendData(&shipList[i]);
					}
					else if(shipList[i].player->shipPilotType == kPilot_Remote)
					Physic_UpdateShip_Network_End(&shipList[i]);
				}
				Server_Leave();
			}
			break;
			
			case kGameMode_Join:
			case kGameMode_BroadCastTV:
			if(worldState->frameTime <= worldState->lastFrameTime) //Check for too high FPS
			break;
			{
				long			i;
				unsigned long	time = worldState->lastFrameTime;
				
				//Drive ships
				for(i = 0; i < shipCount; ++i)
#if __AUTOPILOT__
				if(shipList[i].DriverShipRoutine != nil)
#else
				if((shipList[i].DriverShipRoutine != nil) && !shipList[i].player->finished)
#endif
				(*((ShipDriver_Proc) shipList[i].DriverShipRoutine))(&shipList[i]);
				
				//Update physic
				Client_Enter();
				for(i = 0; i < shipCount; ++i) {
					if(shipList[i].player->shipPilotType == kPilot_Local)
					Physic_UpdateShip_Start(&shipList[i]);
					else if(shipList[i].player->shipPilotType == kPilot_Remote)
					Physic_UpdateShip_Network_Start(&shipList[i]);
				}
				while(time < worldState->frameTime) {
					for(i = 0; i < shipCount; ++i) {
						if(shipList[i].player->shipPilotType == kPilot_Local)
						Physic_UpdateShip(&shipList[i], time);
						else if(shipList[i].player->shipPilotType == kPilot_Remote)
						Physic_UpdateShip_Network(&shipList[i], time);
					}
					++time;
				}
				for(i = 0; i < shipCount; ++i) {
					if((shipList[i].player->shipPilotType == kPilot_Local) || (shipList[i].player->shipPilotType == kPilot_Bot)) {
						Physic_UpdateShip_End(&shipList[i]);
						Client_SendData(&shipList[i]);
					}
					else if(shipList[i].player->shipPilotType == kPilot_Remote)
					Physic_UpdateShip_Network_End(&shipList[i]);
				}
				Client_Leave();
			}
			break;
#endif
			
			case kGameMode_Replay:
			if(worldState->frameTime <= worldState->lastFrameTime) //Check for too high FPS
			break;
			{
				long			i;
				
				//Drive ships & Update physics
				for(i = 0; i < shipCount; ++i) {
					if(shipList[i].DriverShipRoutine != nil)
					(*((ShipDriver_Proc) shipList[i].DriverShipRoutine))(&shipList[i]);
					Physic_UpdateShip_Replay(&shipList[i]);
				}
			}
			break;
			
		}
		
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Main:Drive_Camera");
#endif
		//Drive camera
		cameraDriver();
#if __PROFILE__
		LOG_EXIT; }
#endif
		
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Main:Draw_Frame");
#endif
		Draw_Frame(worldState);
#if __PROFILE__
		LOG_EXIT; }
#endif
		
		//Check if race is over
#if __LIMIT_RACE_TIME__
		if(worldState->frameTime > kMaxRaceTime)
		run = false;
#endif
		switch(gameConfig.gameMode) {
			
			case kGameMode_Local:
			if(gameConfig.gameSubMode == kGameSubMode_Local_TimeRace) {
				if(worldState->frameTime >= gameConfig.expiredTime) {
					//Play finish sound
					Sound_PlaySFxHandle(gameFXSounds[kGameFX_Finish]);
						
					//Save race length
					localShip->player->finishTime = worldState->frameTime - gameConfig.startTime;
					
					//Save race length
					gameConfig.raceLength = worldState->frameTime;
					
					//Save time left - ONLY valid in Time race mode
					gameConfig.expiredTime = 0;
					
					//Display game over screen
					Game_Over();
				}
				else if(localShip->player->lapCount > theRace->lapNumber) {
					//Play finish sound
					Sound_PlaySFxHandle(gameFXSounds[kGameFX_Finish]);
						
					//Save race length
					localShip->player->finishTime = worldState->frameTime - gameConfig.startTime;
					
					//This player has finished the race
					localShip->player->finished = true;
						
					//Save race length
					gameConfig.raceLength = worldState->frameTime;
					
					//Save time left - ONLY valid in Time race mode
					gameConfig.expiredTime = gameConfig.expiredTime - worldState->frameTime;
					
					//Display game over screen
					Game_Over();
				}
			}
			else {
				long			i,
								j;
#if __BOTS_FINISH_RACE__
				Boolean			raceFinished = true;
#endif
				
				for(i = 0; i < shipCount; ++i)
				if((shipList[i].player->lapCount > theRace->lapNumber) && !shipList[i].player->finished) {
					//Play finish sound
					Sound_PlaySFxHandle(gameFXSounds[kGameFX_Finish]);
						
					//Save race length
					shipList[i].player->finishTime = worldState->frameTime - gameConfig.startTime;
					
					//This player has finished the race
					shipList[i].player->finished = true;
						
					//Stop the bot ship
					StopShip(&shipList[i]);
						
#if !__BOTS_FINISH_RACE__
					if(localShip != &shipList[i]) {
#endif					
					//Post message to ship driver
					if(shipList[i].player->rank == 1)
					BlockMove(kString_Driver_LocalWon, shipList[i].player->infoText, sizeof(Str31));
					else
					BlockMove(kString_Driver_LocalFinish, shipList[i].player->infoText, sizeof(Str31));
					shipList[i].player->infoMaxTime = worldState->frameTime + 2 * kTimeUnit;
					
					//Post message to other ship drivers
					for(j = 0; j < shipCount; ++j)
					if(j != i) {
						BlockMove(gameConfig.playerList[i].playerName, shipList[j].player->infoText, sizeof(Str31));
						BlockMove(kString_Driver_Finished, &shipList[j].player->infoText[shipList[j].player->infoText[0] + 1], kString_Driver_Finished_L);
						shipList[j].player->infoText[0] += kString_Driver_Finished_L;
						shipList[j].player->infoMaxTime = worldState->frameTime + 2 * kTimeUnit;
					}
					
#if !__BOTS_FINISH_RACE__
					}
					else {
						//Save race length
						gameConfig.raceLength = worldState->frameTime;
						
						//Display game over screen
						Game_Over();
					}
#endif
				}
#if __BOTS_FINISH_RACE__
				else if(!shipList[i].player->finished)
				raceFinished = false;
				
				//Check if all the ships have finished the race
				if(raceFinished || forceQuit) {
					//Save race length
					gameConfig.raceLength = worldState->frameTime;
				
					//Display game over screen
					Game_Over();
				}
#endif
			}
			break;
			
#if !__DEMO_MODE__
			case kGameMode_Host:
			case kGameMode_Join:
			case kGameMode_BroadCastTV:
			if(!shipCount)
			break;
			{
				long			i;
				Boolean			raceFinished = true;
				
				for(i = 0; i < gameConfig.playerCount; ++i)
				if(gameConfig.playerList[i].shipPilotType != kPilot_RemoteOffLine) {
					if((gameConfig.playerList[i].lapCount > theRace->lapNumber) && !shipList[i].player->finished) {
						//Play finish sound
						Sound_PlaySFxHandle(gameFXSounds[kGameFX_Finish]);
						
						//Stop the ship
						if((gameConfig.playerList[i].shipPilotType == kPilot_Local) || (gameConfig.playerList[i].shipPilotType == kPilot_Bot))
						StopShip(gameConfig.playerList[i].playerShip);
						
						//Save race length
						gameConfig.playerList[i].finishTime = worldState->frameTime - gameConfig.startTime;
						
						//Post message to local player if necessary
						if((localShip != nil) && (localShip != gameConfig.playerList[i].playerShip)) {
							BlockMove(gameConfig.playerList[i].playerName, localShip->player->infoText, sizeof(Str31));
							BlockMove(kString_Driver_Finished, &localShip->player->infoText[localShip->player->infoText[0] + 1], kString_Driver_Finished_L);
							localShip->player->infoText[0] += kString_Driver_Finished_L;
							localShip->player->infoMaxTime = worldState->frameTime + 2 * kTimeUnit;
						}
						
						//This player has finished the race
						shipList[i].player->finished = true;
					}
					else if(!shipList[i].player->finished)
					raceFinished = false;
				}
				
				//Check if all the ships have finished the race
				if(raceFinished) {
					//Save race length
					gameConfig.raceLength = worldState->frameTime;
				
					//Display game over screen
					Game_Over();
				}
			}
			break;
#endif
			
			/*case kGameMode_Practice:
			
			break;*/
			
			case kGameMode_Replay:
			if((worldState->frameTime < gameConfig.raceLength) && (worldState->frameTime < kMaxReplayTime))
			break;
			{
				//Play finish sound
				Sound_PlaySFxHandle(gameFXSounds[kGameFX_Finish]);
				
				//Display game over screen
				Game_Over();
			}
			break;
			
		}
		
#if __DISPLAY_FPS__
		MoveTo(framePoint.h,framePoint.v);
		EraseRect(&frameRect);
		ellapsedMilliseconds = OTElapsedMilliseconds(&OTTime);
		framesPerSecond = 1000.0 / (float) ellapsedMilliseconds;
		FloatToString(framesPerSecond, frameRate);
		DrawString(frameRate);
#endif
	}
	Sound_PlaySFx(kSFxEscape);
	
#if __DISPLAY_FPS__
	ForeColor(blackColor);
#endif
	
	return noErr;
}

static void TournamentScore_Update()
{
	long						i;
	PlayerConfigurationPtr		player;
	
	//Update players scores
	for(i = 0; i < gameConfig.playerCount; ++i) {
		player = &gameConfig.playerList[i];
		
		//Add rank bonus points to player score
		if(player->rank) {
			if(player->rank == 1)
			player->score += 400;
			else if(player->rank == 2)
			player->score += 300;
		}
		
		//Add best lap bonus if any to player score
		if(player->bestLapTime <= gameConfig.globalBestLapTime)
		player->score += 100;
	}
}

OSErr Game_New()
{
	OSErr		theError;
	
	theError = Game_SetUpScreen();
	if(theError)
	return theError;
	
#if __CD_PROTECTION__
	if(!networkOnly) {
		if(!XCheckForCD("\pWaterRace", 13, 60, 37))
		*((long*) -1) = 0x0239;
	}
#endif	
	
	theError = Game_Start();
	if(theError) {
		Game_RestoreScreen(); //SL_Switch(mainGDHandle, &beforeRaveSwitchInfo);
		return theError;
	}
	
	Game_Run();
	
	theError = Game_RestoreScreen();
	if(theError)
	return theError;
	
	theError = Game_Quit();
	if(theError) {
		Wrapper_Error_Display(163, theError, nil, false);
		return theError;
	}
	
#if __DISPLAY_SCORE_SCREEN__
	//Display Score
	if((gameConfig.gameMode_save != kGameMode_Practice) && (gameConfig.gameMode_save != kGameMode_Replay) 
		&& !((gameConfig.gameMode_save == kGameMode_Local) && (gameConfig.gameSubMode_save == kGameSubMode_Local_TimeRace)) && !(localShip && !localShip->player->finished)) {
		if((gameConfig.gameMode_save == kGameMode_Local) && (gameConfig.gameSubMode_save == kGameSubMode_Local_Tournament))
		TournamentScore_Update();
		theError = ScoreMenu_Display();
		if(theError) {
			Wrapper_Error_Display(164, theError, nil, __FORCE_QUIT_ON_ERROR__);
			return theError;
		}
	}
#endif

	//Start music
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
	GetIndString(musicsFolder.name, MusicFileNameResID, interfaceMusic);
#if __USE_AUDIO_CD_TRACKS__
	theError = AudioCD_PlayAudioTrack_Wrapper(&musicsFolder);
#elif __USE_AUDIO_FILES__
	theError = QDPlayer_Start(&musicsFolder);
#endif
	if(theError)
	Wrapper_Error_Display(143, theError, nil, false);
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
	QDPlayer_Volume(thePrefs.musicVolume);
#endif
#endif
	
	return noErr;
}

OSErr Game_SetupLocal(OSType gameMode, PilotPtr pilot, OSType terrainID, OSType shipID)
{
	long						i;
	OSErr						theError;
	
	//Setup global game info
	gameConfig.version = kNetGameVersion;
	gameConfig.gameMode = gameMode;
	if(gameMode == kGameMode_Practice)
	gameConfig.gameSubMode = kGameSubMode_None;
	else if(gameMode == kGameMode_Local)
	gameConfig.gameSubMode = pilot->mode;
	else
	return kError_FatalError;
	gameConfig.gameMode_save = gameConfig.gameMode;
	gameConfig.gameSubMode_save = gameConfig.gameSubMode;
	gameConfig.terrainID = terrainID;
	gameConfig.terrainDifficulty = pilot->difficulty;
	gameConfig.playerCount = 0;
	
	//Clear players
	for(i = 0; i < kMaxPlayers; ++i)
	Clear_Player(&gameConfig.playerList[i]);
	
	//Setup players
	if(gameMode == kGameMode_Practice)
	theError = PlayMode_Practice_SetUpPlayers(pilot, shipID);
	else
	theError = PlayMode_Local_SetUpPlayers(pilot, shipID);
	
	return theError;
}
