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


#include				<Appearance.h>
#include				<QD3DAcceleration.h>
#include				<InputSprocket.h>

#include				"WaterRace.h"
#include				"Strings.h"
#include				"Data Files.h"
#include				"Interface.h"
#include				"Preferences.h"
#include				"ISpRoutines.h"
#include				"Network Engine.h"

#include				"Infinity Rendering.h"
#include				"Infinity Player.h"
#include				"Infinity Audio Engine.h"
#include				"Textures.h"

#include				"Keys.h"
#include				"Dialog Utils.h"

#if __USE_AUDIO_CD_TRACKS__
#include				"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include				"QDesign Player.h"
#endif

#if	__CD_PROTECTION__
#include				"CD Protection.h"
#endif

//CONSTANTES:

#define					kATI_MinimalVersion			0x00000041 //4.1
#define					kATI_MinimalVersion128		0x00000027 //2.7
#define					kATI_EngineID				4
#define					kATI_EngineID128			5
#define					kATI_EngineIDRadeon			6

#define					kATI_FogBugVersion128		0x0000005F //5.9.5
#define					kATI_FogBugVersionRadeon	0x0000000A //6.6.10

#define					k3Dfx_MinimalVersion		0x00000113 //1.1.3

#define					kAEWaitDelay				6

#if __PATCH_PREFERENCES__
#if __PATCH_PREFERENCES__ == 1
#define	__render_size_h__	1024
#define	__render_size_v__	768
#define	__render_depth__	32
#define	__low_res__			0
#define	__mipmap__			0
#elif __PATCH_PREFERENCES__ == 2
#define	__render_size_h__	640
#define	__render_size_v__	480
#define	__render_depth__	32
#define	__low_res__			0
#define	__mipmap__			0
#elif __PATCH_PREFERENCES__ == 3
#define	__render_size_h__	512
#define	__render_size_v__	384
#define	__render_depth__	16
#define	__low_res__			0
#define	__mipmap__			0
#else
#define	__render_size_h__	600
#define	__render_size_v__	400
#define	__render_depth__	16
#define	__low_res__			1
#define	__mipmap__			0
#endif
#endif

//MACROS:

#if __SECURE_CRASH__
#define ExitToShell() { properlyQuit = true; ExitToShell(); }
#endif

//PROTOTYPES:

void Hide_MenuBar();
void Show_MenuBar();

//VARIABLES:

Point					screenOffset,
						screenResolution,
						mainWinOffset;
GDeviceInfoPtr			screenInfo = nil;
GDHandle				mainGDHandle;
VDSwitchInfoRec			oldScreenSwitchInfo;
Handle					theDisplayState = nil;
#if __SCREEN_FADES__
GammaRef				mainGamma = nil;
#endif
WindowPtr				mainWin,
						backgroundWin;
GWorldPtr				bufferGWorld,
						bufferGWorld2;

Boolean					redefineScreens;
#if !__DEMO_MODE__
Boolean					networkAvailable;
#endif
short					mainResFileID;
#if __USE_AUDIO_CD_TRACKS__
short					audioCD;
#endif
#if __CD_PROTECTION__ && __NETWORK_ONLY_IF_CD_FAILURE__
Boolean					networkOnly = false;
#endif

#if __SECURE_CRASH__
static Boolean			properlyQuit = false,
						screenInited = false,
						fontAliased = false;
#endif

//ROUTINES:

static void Init_Toolbox()
{
	long		i;
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
	
	for(i = 0; i < 16; ++i)
	MoreMasters();
}

static OSErr Screen_Init()
{
	OSErr				theError;
	short				realWidth = kInterface_Width,
						realHeight = kInterface_Height;
	VDSwitchInfoRec*	switchInfoPtr;
	VDSwitchInfoRec		switchInfo;
	Rect				winBounds = {0,0,kInterface_Height,kInterface_Width};
	PixMapHandle		pictPix = NULL;
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	Rect				desktopRect;
	
	//Do we already have a screen resolution and is it still available?
	if(!redefineScreens && (thePrefs.interfaceScreen.csMode != 0) && (SL_ModeAvailable(mainGDHandle, &thePrefs.interfaceScreen)))
	BlockMove(&thePrefs.interfaceScreen, &switchInfo, sizeof(VDSwitchInfoRec));
	else {
		//User want to choose refresh rate
		if(redefineScreens) {
			//Gather informations about this screen
			theError = SL_ScanGDevice(mainGDHandle, &screenInfo);
			if(theError) {
				Wrapper_Error_Display(129, theError, nil, false);
				return theError;
			}
			
			//Ask the user a refresh rate
			SpinningCursor_Stop();
			theError = SL_GetDeviceResolution(screenInfo, &realWidth, &realHeight, kInterface_Depth, &switchInfoPtr, kString_Interface_ManualResolution);
			if(theError) {
				Wrapper_Error_Display(130, theError, nil, false);
				return theError;
			}
			BlockMove(switchInfoPtr, &switchInfo, sizeof(VDSwitchInfoRec));
			SpinningCursor_Start();
		}
		else { //automatic choice
			theError = SL_LookForResolution(mainGDHandle, &realWidth, &realHeight, kInterface_Depth, &switchInfo);
			if(theError) {
				Wrapper_Error_Display(130, theError, nil, false);
				return theError;
			}
		}
		
		//Save the switch info
		BlockMove(&switchInfo, &thePrefs.interfaceScreen, sizeof(VDSwitchInfoRec));
	}
	
	SpinningCursor_Stop();
	HideCursor();
	
	//Save screen state
	theError = SL_GetCurrent(mainGDHandle, &oldScreenSwitchInfo);
	if(theError) {
		Wrapper_Error_Display(131, theError, nil, false);
		return theError;
	}
	
#if __SCREEN_FADES__
	//Init fading
	StartFading(&mainGamma);
	
	//Fade in...
#if __FADE_ALL_SCREENS_ON_STARTUP__
	FadeToBlack(2 * kGammaSpeed, quadraticFade);
#else
	Fade1ToBlack(mainGDHandle, 2 * kGammaSpeed, quadraticFade, nil, 0);
#endif
#endif
	
	//Hide menubar
	Hide_MenuBar();
	
	//Start configuring displays
	DMBeginConfigureDisplays(&theDisplayState);
	
	//Set up black background window
	backgroundWin = GetNewCWindow(128, nil, (WindowPtr) -1L);
	desktopRect = (**LMGetGrayRgn()).rgnBBox;
#if __MACOS_X_CLASSIC__
	MoveWindow(backgroundWin, desktopRect.left, desktopRect.top, false);
	SizeWindow(backgroundWin, desktopRect.right - desktopRect.left, desktopRect.bottom - desktopRect.top, true);
#else	
	MoveWindow(backgroundWin, -16000, -16000, false);
	SizeWindow(backgroundWin, 32000, 32000, true);
#endif
	SetPort(backgroundWin);
	BackColor(whiteColor);
	ForeColor(blackColor);
	ShowHide(backgroundWin, true);
	
	//Set our screen
	theError = SL_Switch(mainGDHandle, &switchInfo);
	if(theError) {
		Wrapper_Error_Display(132, theError, nil, false);
		return theError;
	}
		
#if __SECURE_CRASH__
	screenInited = true;
#endif

	//Create GWorlds
	GetGWorld(&savePort, &saveDevice);
	theError = NewGWorld(&bufferGWorld, kInterface_Depth, &winBounds, NULL, NULL, NULL);
	if(theError) {
		Wrapper_Error_Display(133, theError, nil, false);
		return theError;
	}
	pictPix = GetGWorldPixMap(bufferGWorld);
	LockPixels(pictPix);
	SetGWorld(bufferGWorld, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	
	theError = NewGWorld(&bufferGWorld2, kInterface_Depth, &winBounds, NULL, NULL, NULL);
	if(theError) {
		Wrapper_Error_Display(133, theError, nil, false);
		return theError;
	}
	pictPix = GetGWorldPixMap(bufferGWorld2);
	LockPixels(pictPix);
	SetGWorld(bufferGWorld2, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	SetGWorld(savePort, saveDevice);
	
	//Set up interface window
	mainWin = GetNewCWindow(129, nil, (WindowPtr) -1L);
	mainWinOffset.h = ((**mainGDHandle).gdRect.right - (**mainGDHandle).gdRect.left - kInterface_Width) / 2;
	mainWinOffset.v = ((**mainGDHandle).gdRect.bottom - (**mainGDHandle).gdRect.top - kInterface_Height) / 2;
	MoveWindow(mainWin, mainWinOffset.h + (**mainGDHandle).gdRect.left, mainWinOffset.v + (**mainGDHandle).gdRect.top, false);
	SizeWindow(mainWin, kInterface_Width, kInterface_Height, true);
	SetPort(mainWin);
	BackColor(whiteColor);
	ForeColor(blackColor);
	ShowHide(mainWin, true);
	
	return noErr;
}

static OSErr Screen_Quit()
{
	OSErr						theError;
	
	//Kill windows
	DisposeWindow(backgroundWin);
	DisposeWindow(mainWin);
	
	//Kill GWorlds
	UnlockPixels(GetGWorldPixMap(bufferGWorld));
	DisposeGWorld(bufferGWorld);
	UnlockPixels(GetGWorldPixMap(bufferGWorld2));
	DisposeGWorld(bufferGWorld2);
	
	//Restore screen state
	theError = SL_Switch(mainGDHandle, &oldScreenSwitchInfo);
	
	//End configuring displays
	DMEndConfigureDisplays(theDisplayState);
	
	//Show menubar
	Show_MenuBar();
	
	//Restore
	if(screenInfo != nil)
	SL_DisposeGDeviceInfo(screenInfo);
	
#if __SCREEN_FADES__
	//Fade out...
#if __FADE_ALL_SCREENS_ON_STARTUP__
	FadeToGamma(mainGamma, kGammaSpeed, quadraticFade);
#else
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	//Clean up
	StopFading(mainGamma, true);
#endif
	
	return noErr;
}

static Boolean Init_Appeareance()
{
	OSErr					theError;
	long					response;
	
	//Do we have Appeareance Manager installed?
	theError = Gestalt(gestaltAppearanceAttr,&response);
	if((theError == noErr) && (BitTst(&response,31 - gestaltAppearanceExists))) {
    	theError = RegisterAppearanceClient();
    	if(theError == noErr)
    	return true;
	}
	
	//Display error alert
	Alert(128, nil);
	
	return false;
}

#if __CHECK_FOR_RAVE_1_6__
static void Check_Rave_1_6()
{
	TQAEngine*				theEngine;
	TQADevice				raveDevice;
	TQARect 				boundsRect;
	TQADrawContext*			drawContext;
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					text1,
							text2;
	Boolean					installed = false;
	
	//Let's find the Apple software engine
	if(Get_SoftwareEngine(&theEngine) == noErr) {
		//Create a dummy context
		raveDevice.deviceType = kQADeviceGDevice;
		raveDevice.device.gDevice = GetMainDevice();
		boundsRect.left = 0;
		boundsRect.top = 0;
		boundsRect.right = 16;
		boundsRect.bottom = 16;
		if(QADrawContextNew(&raveDevice, &boundsRect, nil, theEngine, kQAContext_NoZBuffer, &drawContext) == noErr) {
			//Check Rave version
			if(drawContext->version >= kQAVersion_1_6)
			installed = true;
			QADrawContextDelete(drawContext);
		}
	}
		
	if(!installed) {
		params.movable = true;
		params.helpButton = false;
		params.filterProc = nil;
		params.defaultText = kString_Misc_Continue;
		params.cancelText = nil;
		params.otherText = nil;
		params.defaultButton = kAlertStdAlertOKButton;
		params.cancelButton = 0;
		params.position = kWindowDefaultPosition;
		
		UseResFile(mainResFileID);
		GetIndString(text1, 200, 1);
		GetIndString(text2, 200, 2);
		StandardAlert(kAlertStopAlert, text1, text2, &params, &outItemHit);
	}
}
#endif

#if __CHECK_FOR_ATI_DRIVERS__
static void Check_ATIDrivers()
{
	TQAEngine*				theEngine;
	TQADevice				raveDevice;
	long					engineID,
							version;
	Boolean					installed = true;
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					text1,
							text2;
							
	//Let's find the ATI engine
	raveDevice.deviceType = kQADeviceGDevice;
	raveDevice.device.gDevice = mainGDHandle;
	theEngine = QADeviceGetFirstEngine(&raveDevice);
	while(theEngine != nil) {
		if(IsATIKnownEngine(theEngine))
		break;
		
		theEngine = QADeviceGetNextEngine(&raveDevice, theEngine);
	}
	if(theEngine == nil)
	return;
	
	//Check its version
	if(QAEngineGestalt(theEngine, kQAGestalt_EngineID, &engineID))
	return;
	if(QAEngineGestalt(theEngine, kQAGestalt_Revision, &version))
	return;
	
	//Setup alert
	params.movable = true;
	params.helpButton = false;
	params.filterProc = nil;
	params.defaultText = kString_Misc_Continue;
	params.cancelText = nil;
	params.otherText = nil;
	params.defaultButton = kAlertStdAlertOKButton;
	params.cancelButton = 0;
	params.position = kWindowDefaultPosition;
	UseResFile(mainResFileID);
	
	//Check versions
	if(((engineID == kATI_EngineID) && (version < kATI_MinimalVersion))
	 || ((engineID == kATI_EngineID128) && (version < kATI_MinimalVersion128))) {
	 	GetIndString(text1, 200, 7);
		GetIndString(text2, 200, 8);
		SpinningCursor_Stop();
		StandardAlert(kAlertCautionAlert, text1, text2, &params, &outItemHit);
		SpinningCursor_Start();
	}
	
	//Check for fog bug (Rage 128 & Radeon only)
	if(((engineID == kATI_EngineID128) && (version == kATI_FogBugVersion128))
		|| ((engineID == kATI_EngineIDRadeon) && (version == kATI_FogBugVersionRadeon))) {
	 	GetIndString(text1, 200, 15);
		GetIndString(text2, 200, 16);
		SpinningCursor_Stop();
		StandardAlert(kAlertCautionAlert, text1, text2, &params, &outItemHit);
		SpinningCursor_Start();
	}
}
#endif

#if __CHECK_FOR_3Dfx_DRIVERS__
static void Check_3DfxDrivers()
{
	TQAEngine*				theEngine;
	TQADevice				raveDevice;
	long					version;
	Boolean					installed = true;
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					text1,
							text2;
							
	//Let's find the 3Dfx engine
	raveDevice.deviceType = kQADeviceGDevice;
	raveDevice.device.gDevice = mainGDHandle;
	theEngine = QADeviceGetFirstEngine(&raveDevice);
	while(theEngine != nil) {
		if(Is3DfxKnownEngine(theEngine))
		break;
		
		theEngine = QADeviceGetNextEngine(&raveDevice, theEngine);
	}
	if(theEngine == nil)
	return;
	
	//Check its version
	if(QAEngineGestalt(theEngine, kQAGestalt_Revision, &version))
	return;
	if(version >= k3Dfx_MinimalVersion)
	return;
	
	params.movable = true;
	params.helpButton = false;
	params.filterProc = nil;
	params.defaultText = kString_Misc_Continue;
	params.cancelText = nil;
	params.otherText = nil;
	params.defaultButton = kAlertStdAlertOKButton;
	params.cancelButton = 0;
	params.position = kWindowDefaultPosition;
	
	UseResFile(mainResFileID);
	GetIndString(text1, 200, 13);
	GetIndString(text2, 200, 14);
	SpinningCursor_Stop();
	StandardAlert(kAlertCautionAlert, text1, text2, &params, &outItemHit);
	SpinningCursor_Start();
}
#endif

static Boolean Rave_Available()
{
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					text1,
							text2;
	
	//Is Rave installed
	if((RoutineDescriptorPtr) QADrawContextNew == (RoutineDescriptorPtr) kUnresolvedCFragSymbolAddress) {
		params.movable = true;
		params.helpButton = false;
		params.filterProc = nil;
		params.defaultText = kString_Misc_Quit;
		params.cancelText = nil;
		params.otherText = nil;
		params.defaultButton = kAlertStdAlertOKButton;
		params.cancelButton = 0;
		params.position = kWindowDefaultPosition;
		
		UseResFile(mainResFileID);
		GetIndString(text1, 200, 11);
		GetIndString(text2, 200, 12);
		StandardAlert(kAlertStopAlert, text1, text2, &params, &outItemHit);
		
		return false;
	}
	
#if __CHECK_FOR_RAVE_1_6__
	//Is Rave 1.6 installed?
	Check_Rave_1_6();
#endif

	return true;
}

static Boolean QT3_Available()
{
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					text1,
							text2;
	OSErr					theError;
	NumVersion				version;
	
	theError = Gestalt(gestaltQuickTimeVersion, (long*) &version);
    if((theError != noErr) || (version.majorRev < 0x03)) {
		params.movable = true;
		params.helpButton = false;
		params.filterProc = nil;
		params.defaultText = kString_Misc_Quit;
		params.cancelText = nil;
		params.otherText = nil;
		params.defaultButton = kAlertStdAlertOKButton;
		params.cancelButton = 0;
		params.position = kWindowDefaultPosition;
		
		UseResFile(mainResFileID);
		GetIndString(text1, 200, 3);
		GetIndString(text2, 200, 4);
		StandardAlert(kAlertStopAlert, text1, text2, &params, &outItemHit);
		
		return false;
	}
	
	return true;
}

static Boolean SoundManager3_3_Available()
{
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					text1,
							text2;
	NumVersionVariant		version;
	
	version.parts = SndSoundManagerVersion();
	if(version.whole < 0x03300000) {
		params.movable = true;
		params.helpButton = false;
		params.filterProc = nil;
		params.defaultText = kString_Misc_Quit;
		params.cancelText = nil;
		params.otherText = nil;
		params.defaultButton = kAlertStdAlertOKButton;
		params.cancelButton = 0;
		params.position = kWindowDefaultPosition;
		
		UseResFile(mainResFileID);
		GetIndString(text1, 200, 5);
		GetIndString(text2, 200, 6);
		StandardAlert(kAlertStopAlert, text1, text2, &params, &outItemHit);
		
		return false;
	}
	
	return true;
}

static Boolean InputSprocket_Available()
{
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					text1,
							text2;
	NumVersion				ispVersion;
	Boolean					ok = true;
	
	//Make sure ISp 1.3 or later is installed
	if((RoutineDescriptorPtr) ISpGetVersion == (RoutineDescriptorPtr) kUnresolvedCFragSymbolAddress)
	ok = false;
	else {
		ispVersion = ISpGetVersion();
		if((ispVersion.majorRev < 1) || ((ispVersion.majorRev == 1) && ((ispVersion.minorAndBugRev < 0x30))))
		ok = false;
	}
	
	if(!ok) {
		params.movable = true;
		params.helpButton = false;
		params.filterProc = nil;
		params.defaultText = kString_Misc_Quit;
		params.cancelText = nil;
		params.otherText = nil;
		params.defaultButton = kAlertStdAlertOKButton;
		params.cancelButton = 0;
		params.position = kWindowDefaultPosition;
		
		UseResFile(mainResFileID);
		GetIndString(text1, 200, 9);
		GetIndString(text2, 200, 10);
		StandardAlert(kAlertStopAlert, text1, text2, &params, &outItemHit);
		
		return false;
	}
	
	return true;
}

static GDHandle Get_Screen()
{
	GDHandle			screenDevice = nil;
	
	if(redefineScreens || (thePrefs.screenID == kInvalidDisplayID)) {
		SpinningCursor_Stop();
		thePrefs.screenID = SL_PickUpScreen(kString_Interface_ManualResolution);
		SpinningCursor_Start();
	}
	
	DMGetGDeviceByDisplayID(thePrefs.screenID, &screenDevice, true); //On error return main screen
	
	return screenDevice;
}

static Boolean CommandKeyDown()
{
	KeyMap			theKeys;
	long			value;
	
	GetKeys(theKeys);
	value = IsKeyDown(theKeys, keyCommand);
	if(value)
	return true;
	else
	return false;
}

static void Boot_Sequence()
{
	OSErr				theError;
	
	//Init ToolBox
	Init_Toolbox();
	mainResFileID = CurResFile();
	if(!Init_Appeareance())
	ExitToShell();
	
	//Is the command key down?
	redefineScreens = CommandKeyDown();
	
	//Check for Rave
	if(!Rave_Available())
	ExitToShell();
	//Check for QuickTime 3
	if(!QT3_Available())
	ExitToShell();
	//Check for Sound Manager 3.3
	if(!SoundManager3_3_Available())
	ExitToShell();
	//Check for Input Sprocket 1.2
	if(!InputSprocket_Available())
	ExitToShell();
	
#if __SECURE_CRASH__
	fontAliased = MacOSTextAliasing_IsEnabled();
#endif

#if	__CD_PROTECTION__

/*
Arktis CD release 1: "\pWaterRace - German", 13, 60, 37
FT CD release 1: "\pWaterRace - International", 13, 60, 37
Softline CD release 1: "\pWaterRace - Softline", 13, 60, 37
*/

	short driverRefNum = XCheckForCD("\pWaterRace - ", 13, 60, 37);
	if(!driverRefNum) {
#if __NETWORK_ONLY_IF_CD_FAILURE__
		Wrapper_Error_Display(177, kError_WaterRaceCDNotFound, nil, false);
		networkOnly = true;
#else
		properlyQuit = true;
		Wrapper_Error_Display(176, kError_WaterRaceCDNotFound, nil, true);
#endif
	}
#endif
	
#if __USE_AUDIO_CD_TRACKS__
	//Init audio CD driver
	audioCD = 0;
#if	__CD_PROTECTION__
	theError = AudioCD_Init_Manually(driverRefNum);
#else
	theError = AudioCD_Init_UseAppleDriver();
#endif
	if(theError)
	Wrapper_Error_Display(174, theError, nil, false);
	else {
#if	__CD_PROTECTION__ && __NETWORK_ONLY_IF_CD_FAILURE__
		if(AudioCD_IsCDInserted()) {
			if(networkOnly)
			audioCD = -1;
			else
			audioCD = 1;
		}
#else
		if(AudioCD_IsCDInserted())
		audioCD = 1;
#endif
		else
		Wrapper_Error_Display(175, kError_AudioCDNotFound, nil, false);
	}
#endif
	
	//Read preferences
	if(Preference_Read())
	ExitToShell();
#if __PATCH_PREFERENCES__
	thePrefs.renderWidth = __render_size_h__;
	thePrefs.renderHeight = __render_size_v__;
	if(__render_depth__ == 32)
	thePrefs.engineFlags |= kPref_EngineFlag32BitRendering;
	else
	thePrefs.engineFlags &= ~kPref_EngineFlag32BitRendering;
	if(__low_res__)
	thePrefs.engineFlags |= kPref_EngineFlagLowResolution;
	else
	thePrefs.engineFlags &= ~kPref_EngineFlagLowResolution;
	if(__mipmap__) {
		thePrefs.textureFlags &= ~kTextureFlag_MipMap;
		thePrefs.textureFlags |= kTextureFlag_ForceMipMap;
	}
	else {
		thePrefs.textureFlags |= kTextureFlag_MipMap;
		thePrefs.textureFlags &= ~kTextureFlag_ForceMipMap;
	}
	thePrefs.gameScreen.csMode = 0;
#endif

#if __MOVIE__
	thePrefs.renderWidth = 640;
	thePrefs.renderHeight = 480;
	thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagVideoFX + kPref_EngineFlag32BitRendering 
		+ kPref_EngineFlag32BitZBuffer + kPref_EngineFlagFurtherClipping + kPref_EngineFlagTriFiltering;
	thePrefs.textureFlags = kTextureFlag_MipMap;
	thePrefs.gameScreen.csMode = 0;
#endif

	//Start spinning cursor
	SpinningCursor_Init();
	SpinningCursor_Start();
	
	//Find data folders
	theError = Resolve_DataFolders();
	if(theError) {
		SpinningCursor_Quit();
		properlyQuit = true;
		Wrapper_Error_Display(134, theError, nil, true);
	}
	
	//Scan for ships & terrains
	theError = Preload_DataFolders();
	if(theError) {
		SpinningCursor_Quit();
		properlyQuit = true;
		Wrapper_Error_Display(135, theError, nil, true);
	}
	
	//Open data files
	if(Open_DataFiles()) {
		SpinningCursor_Quit();
		ExitToShell();
	}
	
#if !__DEMO_MODE__
	//Init network
	theError = Network_Init();
	if(theError == noErr)
	networkAvailable = true;
	else {
		Wrapper_Error_Display(173, theError, nil, false);
		networkAvailable = false;
	}
#endif
	
	//Init ISp
	UseResFile(mainResFileID);
	theError = InputSprocket_Init();
	if(theError)
	Wrapper_Error_Display(137, theError, nil, true);
	
	//Pick up our screen
	mainGDHandle = Get_Screen();
	
	//Check drivers versions
#if __CHECK_FOR_ATI_DRIVERS__
	Check_ATIDrivers();
#endif
#if __CHECK_FOR_3Dfx_DRIVERS__
	Check_3DfxDrivers();
#endif
	
	//Set screen... - this hides the cursor and stops spinning
	if(Screen_Init() != noErr) {
		SpinningCursor_Quit();
		ExitToShell();
	}
	
	//Init Text Engine
	theError = TextEngine_Init(32, kInterface_Width);
	if(theError) {
		SpinningCursor_Quit();
		Wrapper_Error_Display(138, theError, nil, true);
	}
	
	//Create 3D rendering buffers & Init audio engine
	theError = InfinityPlayer_Init(true);
	if(theError) {
		SpinningCursor_Quit();
		Wrapper_Error_Display(139, theError, nil, true);
	}
	InfinityAudio_SetMatrix(&worldCamera.camera);
	InfinityAudio_Volume(thePrefs.soundFXVolume);
	
	//Load interface sfx
	theError = Sound_InitSFx();
	if(theError) {
		SpinningCursor_Quit();
		Wrapper_Error_Display(136, theError, nil, true);
	}
	UseResFile(mainResFileID);
}

static void Shut_Down()
{
	//Fade in...
#if __SCREEN_FADES__
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToBlack(mainGDHandle, 2 * kGammaSpeed, quadraticFade, nil, 0);
	AudioCD_SetVolume(0, 0);
#elif __USE_AUDIO_FILES__
	Fade1ToBlack(mainGDHandle, 2 * kGammaSpeed, quadraticFade, QDChannel, 0);
#else
	Fade1ToBlack(mainGDHandle, 2 * kGammaSpeed, quadraticFade, nil, 0);
#endif
#endif
	
	//Do clean up
	SpinningCursor_Quit();
	TextEngine_Quit();
	Sound_QuitSFx();
	InfinityPlayer_Quit();
#if !__DEMO_MODE__
	if(networkAvailable)
	Network_Quit();
#endif
	InputSprocket_Quit();
	Screen_Quit();
#if __USE_AUDIO_CD_TRACKS__
	if(audioCD)
	AudioCD_Stop();
#elif __USE_AUDIO_FILES__
	QDPlayer_Stop();
#endif
	InitCursor();
	
	Close_DataFiles();
#if !__PATCH_PREFERENCES__ && !__MOVIE__
	Preference_Write();
#endif
	
	Preload_ReleaseMemory();

	//Quit!
	FlushEvents(everyEvent, 0);
	ExitToShell();
}

#if __USE_INI__
#include	"Numeric Utils.h"

#define	kInitFileName				"\pWaterRace.ini"
#define	kCommand_GameMode			"\pGameMode"
#define	kCommand_TerrainID			"\pTerrainID"
#define	kCommand_ShipID				"\pShipID"
#define	kCommand_PilotID			"\pPilotID"
#define	kCommand_PlayerName			"\pPlayerName"
#define	kCommand_TerrainDifficulty	"\pTerrainDifficulty"
#define	kEndOfLineChar				'\r'
#define	kOrderChar					'='
#define	kOrderIsDigitChar			'#'

static Ptr InitFile_Load(long* bufferSize)
{
	Ptr			buffer = nil;
	FSSpec		initFile;
	short		destFileID;
	long		bytes;
	
	BlockMove(kInitFileName, initFile.name, sizeof(Str31));
	HGetVol(nil, &initFile.vRefNum, &initFile.parID);
	
	if(FSpOpenDF(&initFile, fsRdPerm, &destFileID))
	return nil;
	GetEOF(destFileID, &bytes);
	
	buffer = NewPtrClear(bytes);
	if(buffer == nil)
	return nil;
	
	if(FSRead(destFileID, &bytes, buffer))
	return nil;
	
	FSClose(destFileID);
	
	*bufferSize = bytes;
	
	return buffer;
}

static long InitFile_ReadCommand(Ptr buffer, long size, Str31 command, void* result)
{
	unsigned char*		theChar = (unsigned char*) buffer;
	long				matchCount = 0,
						position = 1;
	
	//Scan for command
	while(size) {
		if(*theChar == command[position]) {
			++matchCount;
			++position;
			if((matchCount == command[0]) && (*(theChar + 1) == kOrderChar)) {
				theChar += 2;
				if(*theChar == kOrderIsDigitChar) {
					++theChar;
					*((long*) result) = Text2Integer(theChar, size);
					return sizeof(long);
				}
				else {
					matchCount = 0;
					while(*theChar != kEndOfLineChar) {
						*((unsigned char*) result) = *theChar;
						++((unsigned char*) result);
						++matchCount;
						++theChar;
					}
					return matchCount;
				}
			}
		}
		else {
			matchCount = 0;
			position = 1;
		}
		
		++theChar;
		--size;
	}
	
	return 0;
}

#endif

#if __RECORD_RACE__ && __REPLAY_AWARE__
static pascal OSErr ODOC_Handler(const AppleEvent* theAppleEvent, AppleEvent* reply, long handlerRefCon)
{	
	AEDescList		docList;
	AEKeyword		keyword;
	DescType		returnedType;
	FSSpec			theFSSpec;
	Size			actualSize;
	FInfo			fileInfo;
	long			nbItems;
	OSErr			theError;
	FSSpec*			file = (FSSpec*) handlerRefCon;
	
	theError = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if(theError)
	return theError;
	theError = AECountItems(&docList, &nbItems);
	if(theError)
	return theError;
	
	theError = AEGetNthPtr(&docList, 1, typeFSS, &keyword, &returnedType, (Ptr) &theFSSpec, sizeof(FSSpec), &actualSize);
	if(theError)
	return theError;
	
	theError = FSpGetFInfo(&theFSSpec, &fileInfo);
	if(theError)
	return theError;
		
	if(fileInfo.fdType == kReplayFileType)
	*file = theFSSpec;
	
	AEDisposeDesc(&docList);
	
	return nil;
}

static Boolean Check_ForReplay()
{
	UniversalProcPtr		ODOC_Handler_Routine;
	FSSpec					startUpReplayFile;
	EventRecord				event;
	unsigned long			startTime;
	
	ODOC_Handler_Routine = NewAEEventHandlerProc(ODOC_Handler);
	startUpReplayFile.name[0] = 0;
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, ODOC_Handler_Routine, (long) &startUpReplayFile, false);
	startTime = TickCount();
	while(TickCount() < startTime + kAEWaitDelay)
	if(WaitNextEvent(highLevelEventMask, &event, (long) 0, 0)) {
		if((OSType) event.message == typeAppleEvent)
		AEProcessAppleEvent(&event);
	}
	AERemoveEventHandler(kCoreEventClass, kAEOpenDocuments, ODOC_Handler_Routine, false);
	if(startUpReplayFile.name[0] == 0)
	return false;
	
	Replay_SetupGame(&startUpReplayFile);
	Game_New();
	
	return true;
}
#endif

#if __MOVIE__

FSSpec			destFolder;

static Boolean Request_Replay()
{
	StandardFileReply		theReply,
							theReply2;
	SFTypeList				list = {kReplayFileType};
	
	StandardGetFile(nil, 1, list, &theReply);
	if(!theReply.sfGood)
	return false;
	
	StandardPutFile("\pSelect where to save the screenshots", "\pTemp", &theReply2);
	if(!theReply.sfGood)
	return false;
	destFolder = theReply2.sfFile;
	
	HideCursor();
	Replay_SetupGame(&theReply.sfFile);
	Game_New();
	
	return true;
}
#endif

void main()
{
	OSErr			theError;
	
	//Boot...
	Boot_Sequence();

#if __MOVIE__
	Request_Replay();
	Shut_Down();
#endif

#if __RECORD_RACE__ && __REPLAY_AWARE__
	if(!Check_ForReplay()) {
#endif

#if __NETWORK_CLIENT_MODE__
	if(networkAvailable) {
		theError = NetworkMenu_Display();
		if(theError) {
			if(theError != kError_UserCancel)
			Wrapper_Error_Display(140, theError, nil, false);
		}
		else
		Game_New();
	}
	else
	Wrapper_Error_Display(141, kNetError_NoNetworkAvailable, nil, false);
#else

#if __CD_PROTECTION__ && __NETWORK_ONLY_IF_CD_FAILURE__
	if(networkOnly) {
		if(networkAvailable) {
			theError = NetworkMenu_Display();
			if(theError) {
				if(theError != kError_UserCancel)
				Wrapper_Error_Display(140, theError, nil, false);
			}
			else
			Game_New();
		}
		else
		Wrapper_Error_Display(141, kNetError_NoNetworkAvailable, nil, false);
	}
	else {
#endif

#if __USE_INI__
	Ptr					initBuffer;
	long				initBufferSize;
	long				gameMode = 0;
	OSType				terrainID = 'Jamc',
						shipID = 'SpdB',
						pilotID = 'WgLi';
	Str31				playerName;
	long				difficulty = kDifficulty_Easy;
	
	initBuffer = InitFile_Load(&initBufferSize);
	if(initBuffer != nil) {
		InitFile_ReadCommand(initBuffer, initBufferSize, kCommand_GameMode, &gameMode);
		InitFile_ReadCommand(initBuffer, initBufferSize, kCommand_TerrainID, &terrainID);
		InitFile_ReadCommand(initBuffer, initBufferSize, kCommand_TerrainDifficulty, &difficulty);
		InitFile_ReadCommand(initBuffer, initBufferSize, kCommand_ShipID, &shipID);
		InitFile_ReadCommand(initBuffer, initBufferSize, kCommand_PilotID, &pilotID);
		playerName[0] = InitFile_ReadCommand(initBuffer, initBufferSize, kCommand_PlayerName, &playerName[1]);
		
		if(gameMode == 1) {
			if(networkAvailable) {
				theError = NetworkMenu_Display();
				if(theError) {
					if(theError != kError_UserCancel)
					Wrapper_Error_Display(140, theError, nil, false);
				}
				else
				Game_New();
			}
			else
			Wrapper_Error_Display(141, kNetError_NoNetworkAvailable, nil, false);
		}
		else {
			Pilot		tempPilot;
			
			Pilot_New(&tempPilot, playerName, pilotID, kPilotType_FreePlay, difficulty);
			theError = Game_SetupLocal(kGameMode_Local, &tempPilot, terrainID, shipID);
			if(theError)
			Wrapper_Error_Display(142, theError, nil, false);
			else
			Game_New();
		}
		
		DisposePtr(initBuffer);
	}
	else {
#endif
#if __EXPO_MODE__
	{
		Pilot		thePilot;
		
		Pilot_New(&thePilot, "\pPlayer", kExpo_PilotSet, kExpo_PilotMode, kExpo_PilotDifficulty);
		Interface_RunLocalGame(&thePilot);
	}
#else

#if __PLAY_DEMO__
	//Play intro
	theError = Demo_Play();
	if(theError) {
		Shut_Down();
		ExitToShell();
	}
#else
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
	GetIndString(musicsFolder.name, MusicFileNameResID, interfaceMusic);
#if __USE_AUDIO_CD_TRACKS__
	theError = AudioCD_PlayAudioTrack_Wrapper(&musicsFolder);
#elif __USE_AUDIO_FILES__
	theError = QDPlayer_Start(&musicsFolder);
#endif //__USE_AUDIO_CD_TRACKS__
	if(theError)
	Wrapper_Error_Display(143, theError, nil, false);
			
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
	QDPlayer_Volume(thePrefs.musicVolume);
#endif //__USE_AUDIO_CD_TRACKS__
#endif //__USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
#endif //__PLAY_DEMO__

	//Interface
	Interface_Display();

#endif //__EXPO_MODE__

#if __SHOW_PUB__
	//Show pubs
	theError = PubWaterRace_Display();
	if(theError)
	Wrapper_Error_Display(144, theError, nil, false);
	theError = PubFusion_Display();
	if(theError)
	Wrapper_Error_Display(144, theError, nil, false);
#endif

#if __USE_INI__
	}
#endif
#if __CD_PROTECTION__ && __NETWORK_ONLY_IF_CD_FAILURE__
	}
#endif
#endif
#if __RECORD_RACE__ && __REPLAY_AWARE__
	}
#endif

	//Terminate...
	Shut_Down();
}

#if __SECURE_CRASH__
extern "C" void terminate();

void terminate()
{
	//Restore screen res, gamma & show cursor
	if(!properlyQuit) {
		SpinningCursor_Quit();
		TextEngine_Quit();
		Sound_QuitSFx();
		InfinityPlayer_Quit();
#if !__DEMO_MODE__
		if(networkAvailable) {
			Server_ShutDown();
			Client_Disconnect();
			Network_Quit();
		}
#endif
		InputSprocket_Quit();
		if(screenInited)
		Screen_Quit();
#if __USE_AUDIO_CD_TRACKS__
		if(audioCD)
		AudioCD_Stop();
#elif __USE_AUDIO_FILES__
		QDPlayer_Stop();
#endif
		InitCursor();
		Close_DataFiles();
		
		Preload_ReleaseMemory();
		if(fontAliased)
		MacOSTextAliasing_Restore();
	}
}

#endif
