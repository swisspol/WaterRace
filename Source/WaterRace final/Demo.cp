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


#include				<QD3DAcceleration.h>

#include				"WaterRace.h"
#include				"Data Files.h"
#include				"Interface.h"
#include				"Preferences.h"
#include				"Drivers.h"

#include				"Clock.h"
#include				"Infinity Rendering.h"

#include				"Infinity Player.h"
#include				"Textures.h"
#include				"Camera Utils.h"
#include				"Model Animation Utils.h"

#if __PLAY_JINGLE__
#include				"Infinity Audio Engine.h"
#include				"Sound Utils.h"
#endif

#if __USE_AUDIO_CD_TRACKS__
#include				"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include				"QDesign Player.h"
#endif

#if __DISPLAY_FPS__
#include				"OpenTransport.h"
#include				"Numeric Utils.h"
#endif

//CONSTANTES:

#if __PLAY_JINGLE__
#define					kFTLogoDisplayDelay		300
#else
#define					kFTLogoDisplayDelay		200
#endif

#define					kFTLogoPICTID			128
#define					kLoadingPICTID			129

#define					kMovieHOffset			0
#define					kMovieVOffset			60
#define					kMovieWidth				640
#define					kMovieHeight			360

#define					kDemoFlyThroughID		'Demo'

#define					kSyncDelay				(0 * kTimeUnit)

//ROUTINES:

static OSErr FlyThrough()
{
	OSErr					theError;
	Point					size,
							offset;
	FSSpec					demoFile;
	unsigned long			startLoadingTime;
	Rect					winBounds = {0,0,kInterface_Height,kInterface_Width};
	PicHandle				thePic;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	Rect					pictRect;
	Boolean					run = true;
	TQAEngine*				demoEngine;
#if __KEYBOARD_STOPS_DEMO__
	KeyMap					theKeys;
#endif
	
	GetGWorld(&savePort, &saveDevice);
	UseResFile(interfaceResFileID);
	
	//Display Loading picture
	thePic = (PicHandle) Get1Resource(kPICTResType, kLoadingPICTID);
	if(thePic == nil) {
		Wrapper_Error_Display(157, ResError(), nil, __FORCE_QUIT_ON_ERROR__);
		return ResError();
	}
	SetGWorld(bufferGWorld, NULL);
	pictRect = (**thePic).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);
	OffsetRect(&pictRect, (kInterface_Width - pictRect.right) / 2, (kInterface_Height - pictRect.bottom) / 2);
	PaintRect(&winBounds);
	DrawPicture(thePic, &pictRect);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &winBounds, &winBounds, srcCopy, nil);
	SetGWorld(savePort, saveDevice);
	ReleaseResource((Handle) thePic);
	
	UseResFile(mainResFileID);
	
	startLoadingTime = RClock_GetAbsoluteTime();
	
	//Calculate rendering zone
	demoEngine = Get_GameEngine();
#if 0
	if(Is3DfxKnownEngine(demoEngine)) {
#else
	if((Is3DfxKnownEngine(demoEngine) == 1) || (Is3DfxKnownEngine(demoEngine) == 2)) {
#endif
		size.h = 640;
		size.v = 480;
		offset.h = 0;
		offset.v = 0;
	}
	else {
		size.h = kMovieWidth;
		size.v = kMovieHeight;
		offset.h = mainWinOffset.h + kMovieHOffset;
		offset.v = mainWinOffset.v + kMovieVOffset;
	}
	
#if __FADE_BEFORE_DEMO__
	//Fade in...
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	//Setup fake game mode
	gameConfig.gameMode = kGameMode_Demo;
	gameConfig.gameSubMode = kGameSubMode_None;
	gameConfig.gameMode_save = gameConfig.gameMode;
	gameConfig.gameSubMode_save = gameConfig.gameSubMode;
	gameConfig.startTime = 0;
	
	//Init engine
	theError = Infinity_GameInit(demoEngine, &worldState, size, offset);
	if(theError) {
		Wrapper_Error_Display(146, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	//Get demo file
	demoFile.vRefNum = coreFolder.vRefNum;
	demoFile.parID = coreFolder.parID;
	GetIndString(demoFile.name, folderFileNameResID, fileDemo);
	
	theError = World_Load(worldState, &demoFile, kDifficulty_Easy, kDemoFlyThroughID);
	if(theError) {
		Wrapper_Error_Display(147, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	RClock_SetTime(0);
	World_InstallAudio();
	World_Prepare();
	
#if __FADE_BEFORE_DEMO__
	//Render first frame
	worldState->frameTime = RClock_GetTime();
	Anim_Run(&theTerrain, worldState->frameTime);
	Draw_Frame(worldState);
	Draw_Frame(worldState);
	
	//Fade out...
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	//Setup camera
	DriverCamera_FlyThrough_Init();
	
#if __USE_AUDIO_FILES__
	//Wait for sync!
	while(RClock_GetAbsoluteTime() < startLoadingTime + kSyncDelay)
	;
#endif
	
#if __DISPLAY_FPS__
	Str31			frameRate;
	float			framesPerSecond;
	Point			framePoint = {kInterface_Height, kInterface_Width - 45};
	Rect			frameRect = {kInterface_Height - 15, kInterface_Width - 45, kInterface_Height, kInterface_Width};
	unsigned long	numFrames = 0;
	OTTimeStamp		OTTime;
	unsigned long	ellapsedMilliseconds;
	
	SetPort(mainWin);
	ForeColor(redColor);
#endif

	//ACTION!
	while(run) {
#if __DISPLAY_FPS__
		OTGetTimeStamp(&OTTime);
#endif

#if __KEYBOARD_STOPS_DEMO__
		//Did the user press a key
		GetKeys(theKeys);
		theKeys[1] &= ~(1 << 1); //Mask the Caps Lock key
		if(theKeys[0] || theKeys[1] || theKeys[2] || theKeys[3])
		run = false;
#endif

#if __MOUSE_STOPS_DEMO__
		//Did the user press mouse button
		if(Button())
		run = false;
#endif

		//Get time
		worldState->lastFrameTime = worldState->frameTime;
		worldState->frameTime = RClock_GetTime();
		
		//Run scripts
		Anim_Run(&theTerrain, worldState->frameTime);
		
		//Animate sea
		World_UpdateSea();
		
		//Drive camera
		DriverCamera_FlyThrough_Callback();
		
		Draw_Frame(worldState);
		
#if __DISPLAY_FPS__
		MoveTo(framePoint.h,framePoint.v);
		EraseRect(&frameRect);
		ellapsedMilliseconds = OTElapsedMilliseconds(&OTTime);
		framesPerSecond = 1000.0 / (float) ellapsedMilliseconds;
		FloatToString(framesPerSecond, frameRate);
		DrawString(frameRate);
#endif
	}
#if __DISPLAY_FPS__
	BackColor(whiteColor);
	ForeColor(blackColor);
#endif

	//Clean up
	World_Unload();
	
	return noErr;
}

OSErr Demo_Play()
{
	PicHandle				thePic;
	Rect					pictRect;
	Rect					winBounds = {0,0,kInterface_Height,kInterface_Width};
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	OSErr					theError;
	FSSpec					introFile;
	Boolean					run = true,
							userStop = false;
	unsigned long			startTime;
	TQAEngine*				theEngine;
	long					vram;
	unsigned long			textureFlags = 0,
							contextFlags = 0;
	long					vendorID,
							engineID;
#if __KEYBOARD_STOPS_DEMO__
	KeyMap					theKeys;
#endif
#if __PLAY_JINGLE__
	Handle					theSound;
#endif
				
	GetGWorld(&savePort, &saveDevice);
	UseResFile(mainResFileID);
	
#if __32_BITS_RENDERING__
	if(thePrefs.engineFlags & kPref_EngineFlag32BitRendering)
	SetDepth(mainGDHandle, 32, 0, 0);
#endif

#if __PLAY_JINGLE__	
	//Load jingle sound
	UseResFile(mainResFileID);
	Sound_ResLoad(1000, false, &theSound);
	Sound_PlaySFxHandle(theSound);
#else
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
	//Start music
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
#endif	
	
	UseResFile(interfaceResFileID);
	
	//Display FT logo
	thePic = (PicHandle) Get1Resource(kPICTResType, kFTLogoPICTID);
	if(thePic == nil) {
		Wrapper_Error_Display(157, ResError(), nil, __FORCE_QUIT_ON_ERROR__);
		return ResError();
	}
	SetGWorld(bufferGWorld, NULL);
	pictRect = (**thePic).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);
	OffsetRect(&pictRect, (kInterface_Width - pictRect.right) / 2, (kInterface_Height - pictRect.bottom) / 2);
	PaintRect(&winBounds);
	DrawPicture(thePic, &pictRect);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &winBounds, &winBounds, srcCopy, nil);
	SetGWorld(savePort, saveDevice);
	ReleaseResource((Handle) thePic);
	
	UseResFile(mainResFileID);
	
#if __SCREEN_FADES__
	//Fade out...
	Fade1ToGamma(mainGDHandle, mainGamma, 2 * kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	startTime = RClock_GetAbsoluteTime();
	
	//Get intro file
	introFile.vRefNum = coreFolder.vRefNum;
	introFile.parID = coreFolder.parID;
	GetIndString(introFile.name, folderFileNameResID, fileIntro);
	
	//Find a Rave engine
	theEngine = Get_GameEngine();
	if(theEngine == nil) {
		Wrapper_Error_Display(146, kError_NoRaveEngineFound, nil, __FORCE_QUIT_ON_ERROR__);
		return kError_NoRaveEngineFound;
	}
	QAEngineGestalt(theEngine, kQAGestalt_VendorID, &vendorID);
	QAEngineGestalt(theEngine, kQAGestalt_EngineID, &engineID);
	
	//Make sure the Apple engine is not selected
	if((vendorID == kQAVendor_Apple) && (engineID == kQAEngine_AppleSW)) {
		;
	}
	else { //setup params
		//Get available VRAM
		vram = InfinityRendering_AvailableVRAM(theEngine);
		if(vram <= 0) {
			Wrapper_Error_Display(158, kError_VRAMError, nil, __FORCE_QUIT_ON_ERROR__);
			return kError_VRAMError;
		}
	
		if(vram < 1200) //Less than 1.2Mb available
		textureFlags |= kTextureFlag_Compress;
	}
	
	if(thePrefs.engineFlags & kPref_EngineFlagFiltering)
	contextFlags |= kInfinityRenderingFlag_Filtering;
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	contextFlags |= kInfinityRenderingFlag_LowResolution;
	
	if(thePrefs.textureFlags & kTextureFlag_3DfxMode)
	textureFlags |= kTextureFlag_3DfxMode;
	
#if 0
	if(Is3DfxKnownEngine(theEngine))
#else
	if((Is3DfxKnownEngine(theEngine) == 1) || (Is3DfxKnownEngine(theEngine) == 2))
#endif
	theError = InfinityPlayer_LoadAnimation(mainGDHandle, theEngine, &introFile, kPlayer_DefaultTextureSet,
		0, 0, kInterface_Width, kInterface_Height, contextFlags, 
		textureFlags, nil, 0, 0, false);
	else
	theError = InfinityPlayer_LoadAnimation(mainGDHandle, theEngine, &introFile, kPlayer_DefaultTextureSet,
		mainWinOffset.h + kMovieHOffset, mainWinOffset.v + kMovieVOffset, kMovieWidth, kMovieHeight, contextFlags, 
		textureFlags, nil, 0, 0, false);
	if(theError) {
		Wrapper_Error_Display(158, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	//Delay...
	while(run) {
#if __KEYBOARD_STOPS_DEMO__
		//Did the user press a key
		GetKeys(theKeys);
		theKeys[1] &= ~(1 << 1); //Mask the Caps Lock key
		if(theKeys[0] || theKeys[1] || theKeys[2] || theKeys[3]) {
			run = false;
			userStop = true;
		}
#endif

#if __MOUSE_STOPS_DEMO__
		//Did the user press mouse button
		if(Button()) {
			run = false;
			userStop = true;
		}
#endif
		//Did we ran out of time
		if(RClock_GetAbsoluteTime() > startTime + kFTLogoDisplayDelay)
		run = false;
	}

#if __PLAY_JINGLE__
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
	//Start music
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
#endif
	
#if __PLAY_JINGLE__
	DisposeHandle(theSound);
#endif
	if(userStop) {
#if __PLAY_JINGLE__
		InfinityAudio_CleanUp(kCreatorType);
#endif
		InfinityPlayer_UnloadAnimation();
		return noErr;
	}
	
#if __SCREEN_FADES__
	//Fade in...
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	//Clear background
	SetPort(mainWin);
	PaintRect(&(mainWin->portRect));
	
#if __SCREEN_FADES__
	//Fade out & play...
	Set1MonitorState(mainGDHandle, mainGamma);
#endif
	
	theError = InfinityPlayer_PlayAnimation(kPlayer_DefaultAnimationID);
	if(theError) {
		Wrapper_Error_Display(158, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	//Run...
#if __KEYBOARD_STOPS_DEMO__ && __MOUSE_STOPS_DEMO__
	userStop = InfinityPlayer_RunSync(kPlayerFlag_StopOnKeyBoard | kPlayerFlag_StopOnMouse, kPlayer_DefaultMaxFPS);
#elif __KEYBOARD_STOPS_DEMO__
	userStop = InfinityPlayer_RunSync(kPlayerFlag_StopOnKeyBoard, kPlayer_DefaultMaxFPS);
#else
	userStop = InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse, kPlayer_DefaultMaxFPS);
#endif
	
	//Clean up...
	InfinityPlayer_UnloadAnimation();
	
	theError = noErr;
	
#if __PLAY_FLY_THROUGH__
	if(!userStop)
	theError = FlyThrough();
#endif

#if __32_BITS_RENDERING__
	if(thePrefs.engineFlags & kPref_EngineFlag32BitRendering)
	SetDepth(mainGDHandle, kInterface_Depth, 0, 0);
#endif
	
	return theError;
}
