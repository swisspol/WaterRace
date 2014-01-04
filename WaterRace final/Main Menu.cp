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


#include					"WaterRace.h"
#include					"Data Files.h"
#include					"Interface.h"
#include					"Preferences.h"

#include					"Clock.h"
#include					"Infinity Player.h"

#if __USE_AUDIO_CD_TRACKS__
#include					"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include					"QDesign Player.h"
#endif

#if __SHOW_SPINNING_CURSOR__
#include					"Dialog Utils.h"
#endif

//CONSTANTES:

#define						kBackgroundID		200
#define						kMainBitmapID		260
#define						kMainAlphaID		261
#define						kMainButtonResID	128
#define						kButtonImageOffset	300

#define						kLeahPosH			288 //(18 x 16)
#define						kLeahPosV			50
#define						kLeahWidth			(kInterface_Width - kLeahPosH)
#define						kLeahHeight			(kInterface_Height - kLeahPosV)

#define						kAlphaGWorldSizeH	300
#define						kAlphaGWorldSizeV	400

#define						kMainButtonSoundOffset	50
#define						kMainButtonWidth		205
#define						kMainButtonHeight		55
#define						kMainButtonWidth2		158
#define						kMainButtonHeight2		30
#define						kMainButtonTotalWidth	205
#define						kMainButtonTotalHeight	333
#define						kMainButtonOffsetH		0
#define						kMainButtonOffsetV		100
#define						kMainMoveSpeed			(kMainButtonTotalWidth / 60)
#define						kMainScrollDuration		120

//VARIABLES LOCALES:

static Boolean				firstPlay = true;

//ROUTINES:

static OSType GetRandomLoop()
{
	switch((Random() + 32767) % 7) {
		case 0: return kLoop_Main_1; break;
		case 1: return kLoop_Main_2; break;
		case 2: return kLoop_Main_3; break;
		case 3: return kLoop_Main_4; break;
		case 4: return kLoop_Main_5; break;
		case 5: return kLoop_Main_6; break;
		case 6: return kLoop_Main_7; break;
	}
	
	return kNoID;
}

static OSErr MainMenu_Start(Boolean firstCall, Boolean slideButtons)
{
	Rect					theRect,
							destRect,
							copyRect,
							alphaRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	FSSpec					leahSpec;
	OSErr					theError;
	PixMapHandle			pictPix;
	CTabHandle				theCLUT;
	unsigned long			startTime,
							time;
	Boolean					done[kMainNbButtons];
	short					pos,
							realPos;
	
#if __SCREEN_FADES__
	//Fade in...
	if(firstCall)
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	GetGWorld(&savePort, &saveDevice);
	UseResFile(interfaceResFileID);
	
	//Clean windows
	if(firstCall) {
		SetPort(backgroundWin);
		PaintRect(&(backgroundWin->portRect));
		SetPort(mainWin);
		PaintRect(&(mainWin->portRect));
	}
	
	//Load buttons data
	buttonsData = (ButtonListHandle) Get1Resource(kButtonsResType, kMainButtonResID);
	if(buttonsData == nil)
	return ResError();
	DetachResource((Handle) buttonsData);
	HLock((Handle) buttonsData);
	if((**buttonsData).buttonCount != kMainNbButtons)
	return kError_FatalError;
	
	//Animate character
	if(!firstCall)
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	
	//Load button graphics
	for(i = 0; i < kMainNbButtons; ++i) {
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
		
		//Animate character
		if(!firstCall)
		InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	}
	
	if(firstCall) {
		//Draw background
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
		CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &theRect, &theRect, srcCopy, nil);
		CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(bufferGWorld2), &theRect, &theRect, srcCopy, nil);
		
		//Create alpha world
		theCLUT = GetCTable(kGreyCLUTID);
		if(theCLUT == nil)
		return ResError();
		SetRect(&alphaRect, 0, 0, kAlphaGWorldSizeH, kAlphaGWorldSizeV);
		theError = NewGWorld(&alphaGWorld, 8, &alphaRect, theCLUT, NULL, NULL);
		if(theError)
		return theError;
		pictPix = GetGWorldPixMap(alphaGWorld);
		LockPixels(pictPix);
		SetGWorld(alphaGWorld, NULL);
		BackColor(whiteColor);
		ForeColor(blackColor);
		ReleaseResource((Handle) theCLUT);
	}
	else {
		SetGWorld(alphaGWorld, NULL);
	
		//Animate character
		InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	}
	
	//Load & Draw alpha map
	if(slideButtons) {
		backgroundPic = (PicHandle) Get1Resource(kPICTResType, kMainAlphaID);
		if(backgroundPic == nil)
		return ResError();
		DetachResource((Handle) backgroundPic);
		theRect = (**backgroundPic).picFrame;
		OffsetRect(&theRect, -theRect.left, -theRect.top);
		DrawPicture(backgroundPic, &theRect);
		DisposeHandle((Handle) backgroundPic);
	}
	
	//Load & draw image
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kMainBitmapID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	SetGWorld(bufferGWorld2, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left + kButtonImageOffset, -theRect.top + kMainButtonOffsetV);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	SetGWorld(savePort, saveDevice);
	
	if(firstCall) {
		//Load presenter
		Interface_GetLeahSpec(&leahSpec);
		theError = Interface_LoadAnimationWrapper(&leahSpec, kPlayer_DefaultTextureSet, kLeahPosH, kLeahPosV, kLeahWidth, kLeahHeight, kCameraMainMenuID);
		if(theError)
		return theError;
		
		//Fade out...
		if(!slideButtons) {
			SetGWorld(savePort, saveDevice);
			SetRect(&copyRect, kMainButtonOffsetH, kMainButtonOffsetV, kMainButtonOffsetH + kMainButtonTotalWidth, kMainButtonOffsetV + kMainButtonTotalHeight);
			SetRect(&theRect, kButtonImageOffset, kMainButtonOffsetV, kButtonImageOffset + kMainButtonTotalWidth, kMainButtonOffsetV + kMainButtonTotalHeight);
			CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &theRect, &copyRect, srcCopy, nil);
			CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &theRect, &copyRect, srcCopy, nil);
		}
		if(!firstPlay) {
			InfinityPlayer_PlayAnimation(GetRandomLoop());
			InfinityPlayer_RenderFrame();
		}
		else
		InfinityPlayer_EraseBackground();
		Sound_PlaySFx(kSFxFadeOut);
#if __SCREEN_FADES__
#if __USE_AUDIO_CD_TRACKS__
		Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
		AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
		Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, QDChannel, thePrefs.musicVolume);
#else
		Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
#endif
	}
	else
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	
	//Animation...
	SetRect(&copyRect, kMainButtonOffsetH, kMainButtonOffsetV, kMainButtonOffsetH + kMainButtonTotalWidth, kMainButtonOffsetV + kMainButtonTotalHeight);
	for(i = 0; i < kMainNbButtons; ++i)
	done[i] = false;
	
	//Introduce presenter
	if(firstCall && firstPlay) {
		theError = InfinityPlayer_PlayAnimation(kStartID);
		if(theError)
		return theError;
	}
	
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#endif

	if(slideButtons) {
		startTime = RClock_GetAbsoluteTime();
		do {
			if(Button())
			break;
			
			time = RClock_GetAbsoluteTime() - startTime;
			
			//Animate character
			InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
			
			//Move buttons
			pos = time * kMainMoveSpeed;
			CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &copyRect, srcCopy, nil);
			for(i = 0; i < 3; ++i) {
				realPos = pos - i * kMainButtonWidth / kMainNbButtons;
				if(realPos < 0)
				continue;
				
				if((realPos >= kMainButtonWidth - kMainButtonSoundOffset) && !done[i]) {
					Sound_PlaySFx(kSfxAppear);
					done[i] = true;
				}
				if(realPos >= kMainButtonWidth)
				realPos = kMainButtonWidth;
				
				alphaRect.top = (**buttonsData).buttonList[i].where.v - kMainButtonOffsetV;
				alphaRect.bottom = alphaRect.top + kMainButtonHeight;
				alphaRect.left = kMainButtonWidth - realPos;
				alphaRect.right = kMainButtonWidth;
		
				theRect.top = (**buttonsData).buttonList[i].where.v;
				theRect.bottom = theRect.top + kMainButtonHeight;
				theRect.left = kButtonImageOffset + kMainButtonWidth - realPos;
				theRect.right = kButtonImageOffset + kMainButtonWidth;
		
				destRect.top = (**buttonsData).buttonList[i].where.v;
				destRect.bottom = destRect.top + kMainButtonHeight;
				destRect.right = realPos;
				destRect.left = 0;
				
				CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &alphaRect, &theRect, &destRect);
			}
			for(i = 3; i < kMainNbButtons; ++i) {
				realPos = pos - i * kMainButtonWidth / kMainNbButtons;
				if(realPos < 0)
				continue;
				
				if((realPos >= kMainButtonWidth2 - kMainButtonSoundOffset) && !done[i]) {
					Sound_PlaySFx(kSfxAppear);
					done[i] = true;
				}
				if(realPos >= kMainButtonWidth2)
				realPos = kMainButtonWidth2;
				
				alphaRect.top = (**buttonsData).buttonList[i].where.v - kMainButtonOffsetV;
				alphaRect.bottom = alphaRect.top + kMainButtonHeight2;
				alphaRect.left = kMainButtonWidth2 - realPos;
				alphaRect.right = kMainButtonWidth2;
		
				theRect.top = (**buttonsData).buttonList[i].where.v;
				theRect.bottom = theRect.top + kMainButtonHeight2;
				theRect.left = kButtonImageOffset + kMainButtonWidth2 - realPos;
				theRect.right = kButtonImageOffset + kMainButtonWidth2;
		
				destRect.top = (**buttonsData).buttonList[i].where.v;
				destRect.bottom = destRect.top + kMainButtonHeight2;
				destRect.right = realPos;
				destRect.left = 0;
				
				CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &alphaRect, &theRect, &destRect);
			}
			CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
		} while(time <= kMainScrollDuration);
		
		SetGWorld(savePort, saveDevice);
		SetRect(&theRect, kButtonImageOffset, kMainButtonOffsetV, kButtonImageOffset + kMainButtonTotalWidth, kMainButtonOffsetV + kMainButtonTotalHeight);
		CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &theRect, &copyRect, srcCopy, nil);
		CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &theRect, &copyRect, srcCopy, nil);
	}
	else
	SetGWorld(savePort, saveDevice);

	//End animation
	if(firstCall)
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
	
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
#endif

	//Set flag
	firstPlay = false;
	
	return noErr;
}

static short MainMenu_Run()
{
	Boolean					run = true,
							over;
	long					i,
							startTime;
	short					overButton = kButton_None,
							button;
	Point					mouse;
	EventRecord				theEvent;
	char					theChar;
	
	//Loop presenter
	InfinityPlayer_PlayAnimation(GetRandomLoop());
	
	SetPort(mainWin);
#if !__SHOW_SPINNING_CURSOR__
	InitCursor();
#endif
	while(run) {
		if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS))
		InfinityPlayer_PlayAnimation(GetRandomLoop());
		
		GetMouse(&mouse);
		
		//Are we over a button?
		button = kButton_None;
		for(i = 0; i < kMainNbButtons; ++i)
		if(PtInRect(mouse, &(**buttonsData).buttonList[i].hotZone))
		button = i;
		
		if((button == kButton_None) && (overButton != kButton_None))
		Interface_ButtonDraw(overButton, kNormal);
		else if((button != kButton_None) && (overButton != button)) {
			if(overButton != kButton_None)
			Interface_ButtonDraw(overButton, kNormal);
			Interface_ButtonDraw(button, kOver);
			Sound_PlaySFx(kSFxItemSelection);
			
			switch(button) {
				case kMainButton_NewPilot: InfinityPlayer_PlayAnimation(kNewPilotID); break;
				case kMainButton_LoadPilot: InfinityPlayer_PlayAnimation(kLoadPilotID); break;
				case kMainButton_NetGame: InfinityPlayer_PlayAnimation(kNetGameID); break;
				case kMainButton_Options: InfinityPlayer_PlayAnimation(kOptionsID); break;
				case kMainButton_Credits: InfinityPlayer_PlayAnimation(kCreditsID); break;
				case kMainButton_Quit: InfinityPlayer_PlayAnimation(kQuitID); break;
			}
		}
		overButton = button;
		
		//Did the user press the mouse button?
		if(Button() && (overButton != kButton_None)) {
			Interface_ButtonDraw(overButton, kActive);
			Sound_PlaySFx(kSFxConfirm);
			
			//Track mouse
			over = true;
			while(Button()) {
				InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
				
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
				
				if(!InfinityPlayer_ScriptPlaying())
				InfinityPlayer_PlayAnimation(GetRandomLoop());
			}
			Interface_ButtonDraw(overButton, kNormal);
			if(over)
			run = false;
		}
		
		//Did the user press a key combination?
		if(OSEventAvail(everyEvent, &theEvent)) {
			if((theEvent.what == keyDown) || (theEvent.what == autoKey)) {
				theChar = (theEvent.message & charCodeMask);
				if((theChar >= 'a') && (theChar <= 'z')) //Map lower case to upper case
				theChar = theChar - 'a' + 'A';
				
				button = kButton_None;
				switch(theChar) {
					
					case 'N':
					button = kMainButton_NewPilot;
					break;
					
					case 'L':
					button = kMainButton_LoadPilot;
					break;
					
					case 'I':
					case 'G':
					button = kMainButton_NetGame;
					break;
					
					case 'O':
					button = kMainButton_Options;
					break;
					
					case 'C':
					button = kMainButton_Credits;
					break;
					
					case 'Q':
					button = kMainButton_Quit;
					break;
					
				}
				if(button != kButton_None) {
					if(overButton != kButton_None)
					Interface_ButtonDraw(overButton, kNormal);
					overButton = button;
					Interface_ButtonDraw(overButton, kActive);
					Sound_PlaySFx(kSFxConfirm);
					startTime = RClock_GetAbsoluteTime();
					while((RClock_GetAbsoluteTime() - startTime) < kButtonDelay) {
						InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
					}
					Interface_ButtonDraw(overButton, kNormal);
					run = false;
				}
			}
			FlushEvents(everyEvent, 0);
		}
	}
	
	//End current animation
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#else
	if(overButton != kMainButton_NewPilot)
	HideCursor();
#endif
	InfinityPlayer_RunSync(0, kPlayer_DefaultMaxFPS);
	
	//Start transfer animation
	switch(overButton) {
	
		case kMainButton_NewPilot:
		InfinityPlayer_PlayAnimation(kNewTransitionID);
		break;
		
		case kMainButton_LoadPilot:
		case kMainButton_Options:
		
		break;
		
		case kMainButton_Credits:
		InfinityPlayer_PlayAnimation(kGenericTransitionID);
		break;
		
		case kMainButton_NetGame:
		;
		break;
		
		case kMainButton_Quit:
		InfinityPlayer_PlayAnimation(kEndID);
		break;
		
	}
	
	return overButton;
}

static OSErr MainMenu_Quit(Boolean lastCall, Boolean slideButtons)
{
	Rect					theRect,
							destRect,
							copyRect,
							alphaRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	unsigned long			startTime,
							time;
	Boolean					done[kMainNbButtons];
	short					pos,
							realPos;
	
	UseResFile(interfaceResFileID);
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(alphaGWorld, NULL);
	
	//Load & Draw alpha map
	if(slideButtons) {
		backgroundPic = (PicHandle) Get1Resource(kPICTResType, kMainAlphaID);
		if(backgroundPic == nil) {
#if __SHOW_SPINNING_CURSOR__
			SpinningCursor_Stop();
#endif
			return ResError();
		}
		DetachResource((Handle) backgroundPic);
		theRect = (**backgroundPic).picFrame;
		OffsetRect(&theRect, -theRect.left, -theRect.top);
		DrawPicture(backgroundPic, &theRect);
		DisposeHandle((Handle) backgroundPic);
		SetGWorld(savePort, saveDevice);
	}
	
	//Animate character
	if(!lastCall)
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	
	//Animation...
	SetRect(&copyRect, kMainButtonOffsetH, kMainButtonOffsetV, kMainButtonOffsetH + kMainButtonTotalWidth, kMainButtonOffsetV + kMainButtonTotalHeight);
	for(i = 0; i < kMainNbButtons; ++i)
	done[i] = false;
	
	if(slideButtons) {
		startTime = RClock_GetAbsoluteTime();
		do {
			if(Button())
			break;
			
			time = RClock_GetAbsoluteTime() - startTime;
			
			//Animate character
			InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
			
			//Move buttons
			pos = time * kMainMoveSpeed;
			CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &copyRect, srcCopy, nil);
			for(i = 0; i < 3; ++i) {
				realPos = kMainButtonWidth - (pos - i * kMainButtonWidth / kMainNbButtons);
				if(realPos < 0)
				continue;
				
				if((realPos < kMainButtonWidth) && !done[i]) {
					Sound_PlaySFx(kSfxDisappear);
					done[i] = true;
				}
				if(realPos >= kMainButtonWidth)
				realPos = kMainButtonWidth;
				
				alphaRect.top = (**buttonsData).buttonList[i].where.v - kMainButtonOffsetV;
				alphaRect.bottom = alphaRect.top + kMainButtonHeight;
				alphaRect.left = kMainButtonWidth - realPos;
				alphaRect.right = kMainButtonWidth;
		
				theRect.top = (**buttonsData).buttonList[i].where.v;
				theRect.bottom = theRect.top + kMainButtonHeight;
				theRect.left = kButtonImageOffset + kMainButtonWidth - realPos;
				theRect.right = kButtonImageOffset + kMainButtonWidth;
		
				destRect.top = (**buttonsData).buttonList[i].where.v;
				destRect.bottom = destRect.top + kMainButtonHeight;
				destRect.right = realPos;
				destRect.left = 0;
				
				CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &alphaRect, &theRect, &destRect);
			}
			for(i = 3; i < kMainNbButtons; ++i) {
				realPos = kMainButtonWidth2 - (pos - i * kMainButtonWidth / kMainNbButtons);
				if(realPos < 0)
				continue;
				
				if((realPos < kMainButtonWidth2) && !done[i]) {
					Sound_PlaySFx(kSfxDisappear);
					done[i] = true;
				}
				if(realPos >= kMainButtonWidth2)
				realPos = kMainButtonWidth2;
				
				alphaRect.top = (**buttonsData).buttonList[i].where.v - kMainButtonOffsetV;
				alphaRect.bottom = alphaRect.top + kMainButtonHeight2;
				alphaRect.left = kMainButtonWidth2 - realPos;
				alphaRect.right = kMainButtonWidth2;
		
				theRect.top = (**buttonsData).buttonList[i].where.v;
				theRect.bottom = theRect.top + kMainButtonHeight2;
				theRect.left = kButtonImageOffset + kMainButtonWidth2 - realPos;
				theRect.right = kButtonImageOffset + kMainButtonWidth2;
		
				destRect.top = (**buttonsData).buttonList[i].where.v;
				destRect.bottom = destRect.top + kMainButtonHeight2;
				destRect.right = realPos;
				destRect.left = 0;
				
				CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &alphaRect, &theRect, &destRect);
			}
			CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
		} while(time <= kMainScrollDuration);
		
		SetGWorld(savePort, saveDevice);
		CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
		CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &copyRect, srcCopy, nil);
	}
	else
	SetGWorld(savePort, saveDevice);
	
	//End animation
	if(lastCall)
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
	HideCursor();
#endif
	
	if(lastCall)
	InfinityPlayer_UnloadAnimation();
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kMainNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	
	if(lastCall) {
		UnlockPixels(GetGWorldPixMap(alphaGWorld));
		DisposeGWorld(alphaGWorld);
	}
	
	return noErr;
}

OSErr MainMenu_Display(short* button, Str31 name, OSType* mode, short* difficulty)
{
	OSErr				theError;
	Boolean				run = true;
	short				newButton;
	
	if(firstPlay)
	theError = MainMenu_Start(true, true);
	else
	theError = MainMenu_Start(true, false);
	if(theError)
	return theError;
	
	do {
		*button = MainMenu_Run();
		if(*button == kMainButton_NewPilot) {
			theError = MainMenu_Quit(false, true);
			if(theError)
			return theError;
			
			theError = NewMenu_Start();
			if(theError)
			return theError;
			
			newButton = NewMenu_Run(name, mode, difficulty);
			
			theError = NewMenu_Quit();
			if(theError)
			return theError;
			
			if(newButton == kNewButton_OK) {
				InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
				InfinityPlayer_UnloadAnimation();
				UnlockPixels(GetGWorldPixMap(alphaGWorld));
				DisposeGWorld(alphaGWorld);
				run = false;
			}
			else {
				theError = MainMenu_Start(false, true);
				if(theError)
				return theError;
			}
		}
		else {
			if(*button == kMainButton_Quit)
			theError = MainMenu_Quit(true, true);
			else
			theError = MainMenu_Quit(true, false);
			run = false;
		}
	} while(run);
	
	return theError;
}
