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


#include				"WaterRace.h"
#include				"Strings.h"
#include				"Data Files.h"
#include				"Interface.h"
#include				"Preferences.h"

#include				"Infinity Player.h"
#include				"Clock.h"

#if __USE_AUDIO_CD_TRACKS__
#include				"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include				"QDesign Player.h"
#endif

#include				"Keys.h"

//CONSTANTES:

#define						kBackgroundID			7000
#define						kBackgroundGCID			7001
#define						kLoadButtonResID		190

enum {kChampionButton_OK = 0, kChampionNbButtons};

#define						kCharacterPosH			288 //(18 x 16)
#define						kCharacterPosV			50
#define						kCharacterWidth			(kInterface_Width - kCharacterPosH)
#define						kCharacterHeight		(kInterface_Height - kCharacterPosV)

//VARIABLES:

OSErr ChampionMenu_Start(CharacterPreloadedDataPtr characterData, Boolean grandChampion)
{
	Rect					theRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	OSErr					theError;
	PixMapPtr				pixmap = *GetGWorldPixMap(bufferGWorld);;
	
	GetGWorld(&savePort, &saveDevice);
	UseResFile(interfaceResFileID);
	
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
	//Start music
	GetIndString(musicsFolder.name, MusicFileNameResID, scoreMusic);
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

	//Clean windows
	SetPort(backgroundWin);
	PaintRect(&(backgroundWin->portRect));
	SetPort(mainWin);
	PaintRect(&(mainWin->portRect));
	
	//Load buttons data
	buttonsData = (ButtonListHandle) Get1Resource(kButtonsResType, kLoadButtonResID);
	if(buttonsData == nil)
	return ResError();
	DetachResource((Handle) buttonsData);
	HLock((Handle) buttonsData);
	if((**buttonsData).buttonCount != kChampionNbButtons)
	return kError_FatalError;
	
	//Load button graphics
	for(i = 0; i < kChampionNbButtons; ++i) {
		buttonsPic[i][0] = (PicHandle) Get1Resource(kPICTResType, (**buttonsData).buttonList[i].overID);
		if(buttonsPic[i][0] == nil)
		return ResError();
		DetachResource((Handle) buttonsPic[i][0]);
		//OffsetRect(&(**buttonsPic[i][0]).picFrame, -(**buttonsPic[i][0]).picFrame.left, -(**buttonsPic[i][0]).picFrame.top);
		
		buttonsPic[i][1] = (PicHandle) Get1Resource(kPICTResType, (**buttonsData).buttonList[i].downID);
		if(buttonsPic[i][1] == nil)
		return ResError();
		DetachResource((Handle) buttonsPic[i][1]);
		//OffsetRect(&(**buttonsPic[i][1]).picFrame, -(**buttonsPic[i][1]).picFrame.left, -(**buttonsPic[i][1]).picFrame.top);
	}
	
	//Draw background
	if(grandChampion)
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kBackgroundGCID);
	else
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kBackgroundID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	SetGWorld(bufferGWorld, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	SetGWorld(savePort, saveDevice);
	SetRect(&theRect, 0, 0, kInterface_Width, kInterface_Height);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(bufferGWorld2), &theRect, &theRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &theRect, &theRect, srcCopy, nil);
	
	//Load character
	theError = Interface_LoadAnimationWrapper(&characterData->fileSpec, kPlayer_DefaultTextureSet, kCharacterPosH, kCharacterPosV, kCharacterWidth, kCharacterHeight, kCameraChampionID);
	if(theError)
	return theError;
	
	//Start animation
	theError = InfinityPlayer_PlayAnimation(kCharacter_Win);
	if(theError)
	return theError;
	
#if __SCREEN_FADES__
	//Fade out...
	InfinityPlayer_RenderFrame();
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
	AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, QDChannel, thePrefs.musicVolume);
#else
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
#endif
	
	return noErr;
}

static OSType GetRandomLoop()
{
	switch((Random() + 32767) % 2) {
		case 0: return kCharacter_Score_Loop_1; break;
		case 1: return kCharacter_Score_Loop_2; break;
	}
	
	return kNoID;
}

OSErr ChampionMenu_Run()
{
	Boolean					run = true,
							over;
	long					i,
							startTime;
	short					overButton = kButton_None,
							button;
	Point					mouse;
	EventRecord				theEvent;
	unsigned char			theKey;
	short					selected = 0;
	
	SetPort(mainWin);
	InitCursor();
	while(run) {
		if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
			InfinityPlayer_PlayAnimation(GetRandomLoop());
		}
		
		GetMouse(&mouse);
		
		//Are we over a button?
		button = kButton_None;
		for(i = 0; i < kChampionNbButtons; ++i)
		if(PtInRect(mouse, &(**buttonsData).buttonList[i].hotZone))
		button = i;
		
		if((button == kButton_None) && (overButton != kButton_None))
		Interface_ButtonDraw(overButton, kNormal);
		else if((button != kButton_None) && (overButton != button)) {
			if(overButton != kButton_None)
			Interface_ButtonDraw(overButton, kNormal);
			Interface_ButtonDraw(button, kOver);
			Sound_PlaySFx(kSFxItemSelection);
		}
		overButton = button;
		
		//Did the user press the mouse button?
		if(Button()) {
			if(overButton != kButton_None) {
				Interface_ButtonDraw(overButton, kActive);
				Sound_PlaySFx(kSFxConfirm);
				
				//Track mouse
				over = true;
				while(Button()) {
					if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
						InfinityPlayer_PlayAnimation(GetRandomLoop());
					}
					
					GetMouse(&mouse);
					//Are we still over the button?
					if(PtInRect(mouse, &(**buttonsData).buttonList[overButton].hotZone)) {
						if(over == false)
						Interface_ButtonDraw(overButton, kActive);
						over = true;
					}
					else {
						if(over == true)
						Interface_ButtonDraw(overButton, kNormal);
						over = false;
					}
				}
				Interface_ButtonDraw(overButton, kNormal);
				if(over) {
					if(overButton == kChampionButton_OK)
					run = false;
				}
			}
		}
		
		//Did the user press a key combination?
		if(OSEventAvail(everyEvent, &theEvent)) {
			if((theEvent.what == keyDown) || (theEvent.what == autoKey)) {
				theKey = (theEvent.message & keyCodeMask) >> 8;
				
				button = kButton_None;
				if((theKey == keyReturn) || (theKey == keyEnter) || (theKey == keyEnterPB) || (theKey == keyEscape)) {
					button = kChampionButton_OK;
					run = false;
				}
				
				if(button != kButton_None) {
					if(overButton != kButton_None)
					Interface_ButtonDraw(overButton, kNormal);
					overButton = button;
					Interface_ButtonDraw(overButton, kActive);
					Sound_PlaySFx(kSFxConfirm);
					startTime = RClock_GetAbsoluteTime();
					while((RClock_GetAbsoluteTime() - startTime) < kButtonDelay)
					;
					Interface_ButtonDraw(overButton, kNormal);
				}
			}
			FlushEvents(everyEvent, 0);
		}
	}
	
	//End current animation if 
	HideCursor();
	
	return noErr;
}

OSErr ChampionMenu_Quit()
{
	long					i;
	
	HideCursor();
	
	InfinityPlayer_UnloadAnimation();
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kChampionNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	
	DisposePtr((Ptr) specs);
	MacOSTextAliasing_Restore();
	
	return noErr;
}

OSErr ChampionMenu_Display(CharacterPreloadedDataPtr characterData, Boolean grandChampion)
{
	OSErr				theError;
	
	theError = ChampionMenu_Start(characterData, grandChampion);
	if(theError)
	return theError;
	
	theError = ChampionMenu_Run();
	
	ChampionMenu_Quit();
	
	return theError;
}
