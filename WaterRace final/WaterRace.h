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


#ifndef __WATERRACE__
#define __WATERRACE__

#include				<RAVE.h>

#include				"Infinity Structures.h"
#include				"Infinity Terrain.h"
#include				"MetaFile Structures.h"
#include				"Infinity Error codes.h"
#include				"Infinity Utils.h"

#include				"ScreenLib.h"

#if __SCREEN_FADES__
#include				"Local ColorGamma.h"
#endif

//WATERRACE ERRORS:
enum {
	kError_CantLoadSound = 64,
	kError_CharacterNotFound,
	kError_NoTerrainTag,
	kError_TrackNotFound,
	kError_FlyThroughNotFound,
	kError_VRAMError,
	kError_ShipNotFound, //70
	kError_TagMissing_Preview,
	kError_UnknownFileType,
	kError_ScreenshotNotAllowed,
	kError_CantCaptureVRAM,
	kError_IncorrectAlias,
	kError_LicenseNotAccepted,
	kError_TerrainNotFound,
	kError_AddOnFileInCoreFolder,
	kError_CoreFileInAddOnFolder,
	kError_WaterRaceCDNotFound, //80
	kError_AudioCDNotFound,
	kError_PilotFileCorrupted,
	kError_AddOnCharacter,
	kError_CoreFileCorrupted
};

//CONSTANTES:

#define					kGammaSpeed				15
#define					kButtonDelay			25
#define					kPICTResType			'PICT'

#define					kCreatorType			'WrRc'
#define					kReplayFileType			'RPly'

enum {
	folderFileNameResID	= 129,
	folderData			= 1,
	fileInterface,
	fileSounds,
	fileIntro,
	fileDemo,
	folderMusics,
	folderShips,
	folderLocations,
	folderCharacters,
	folderPilots,
	folderAddOns,
	leahFileName,
	filePractice,
	folderCore,
	leahShipName,
	leahTerrainName
};

enum {
	MusicFileNameResID	= 131,
	interfaceMusic		= 1,
	browserMusic,
	scoreMusic,
	championMusic
};

#define		kSFxStartID						128
enum {kSFxItemSelection = 0, kSFxConfirm, kSFxEscape, kSFxFadeOut, kSfxAppear, kSfxDisappear, kSFxSnapShot, kNbSFX};

#define		kMinEngineDistance				20.0
#define		kMaxEngineDistance				50.0

#define		kFXMinDistance					20.0
#define		kFXMaxDistance					50.0
#define		kFXVolume						1.0

#if __ENABLE_DATAFILE_SKIPPING__
#define		kSkipChar						'_'
#endif

//WORKAROUND:

typedef struct ExtendedShip_Definition ExtendedShip_Definition;
typedef ExtendedShip_Definition* ExtendedShipPtr;

//MACROS:

#define GetWindowPixMapPtr(w) *(((CGrafPtr)(w))->portPixMap)
#define	GWBitMapPtr(w) &(((GrafPtr)(w))->portBits)

//VARIABLES:

extern Point				screenOffset,
							screenResolution,
							mainWinOffset;
extern GDeviceInfoPtr		screenInfo;
extern GDHandle				mainGDHandle;
extern VDSwitchInfoRec		oldScreenSwitchInfo;
extern Handle				theDisplayState;
#if __SCREEN_FADES__
extern GammaRef				mainGamma;
#endif
extern WindowPtr			mainWin,
							backgroundWin;
extern GWorldPtr			bufferGWorld,
							bufferGWorld2;

extern Boolean				redefineScreens;
#if !__DEMO_MODE__
extern Boolean				networkAvailable;
#endif
extern short				mainResFileID;
#if __USE_AUDIO_CD_TRACKS__
extern short				audioCD;
#endif
#if __CD_PROTECTION__ && __NETWORK_ONLY_IF_CD_FAILURE__
extern Boolean				networkOnly;
#endif

//ROUTINES:

//File: Infinity.cp
TQAEngine* Get_GameEngine();
TQAEngine* Get_InterfaceEngine();
OSErr Infinity_GameInit(TQAEngine* theEngine, StatePtr* state, Point size, Point offset);
void Infinity_SetTerrainState(StatePtr state, TerrainPtr terrain);

//File: Utils.cp
void Wrapper_Error_Display(short actionID, OSErr errorID, ModalFilterProcPtr eventFilterProc, Boolean fatal);
OSErr Take_ScreenShot();
void MacOSTextAliasing_Disable();
void MacOSTextAliasing_Restore();
Boolean MacOSTextAliasing_IsEnabled();
void CopyBits_16WithAlpha(PixMap *alphaMap, PixMap *srcMap, PixMap *destMap, Rect* alphaRect, Rect *sourceRect, Rect *destRect);
OSErr Sound_InitSFx();
void Sound_QuitSFx();
void Sound_PlaySFx(short SFxID);
void Sound_PlaySFxHandle(Handle sound);
OSErr AudioCD_PlayAudioTrack_Wrapper(FSSpec* file);

//File: Progress bar.cp
OSErr Progress_Init(FSSpec* theFile, short location_H, short location_V);
void Progress_Update(short value, short max);
void Progress_Quit();

#endif
