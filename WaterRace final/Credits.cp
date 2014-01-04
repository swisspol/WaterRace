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
#include					"Infinity Rendering.h"
#include					"Infinity Player.h"

#include					"Textures.h"

#include					"Keys.h"

//CONSTANTES:

#define						kCreditsBackgroundID	1000
#define						kCreditsButtonResID		170
#if __LANGUAGE__ == kLanguage_English
#define						kCreditsTextID			128
#elif __LANGUAGE__ == kLanguage_French
#define						kCreditsTextID			129
#elif __LANGUAGE__ == kLanguage_German
#define						kCreditsTextID			130
#elif __LANGUAGE__ == kLanguage_Italian
#define						kCreditsTextID			131
#elif
#error  __LANGUAGE__ undefined!
#endif
#define						kBottomMargin			88
#define						kTopMargin				116
#define						kLeftMargin				295
#define						kRightMargin			580
#define						kScrollingSpeed			20
#define						kFadeHeight				50
#define						kFontDefID				128

#define						kLeahPosH			0
#define						kLeahPosV			50
#define						kLeahWidth			160 //16x10
#define						kLeahHeight			(kInterface_Height - kLeahPosV)

enum {kCreditsButton_Back = 0, kCreditsNbButtons};

//VARIABLES LOCALES:

static Handle				text;
static long					creditsTextHeight;
	
//ROUTINES:

static OSErr Credits_Start()
{
	Rect					theRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	FSSpec					leahSpec;
	OSErr					theError;
	Str31					name;
	short					resID;
	Handle					fontSpecHandle;
	TE_ResFontDefinitionPtr	fontDef;
	
#if __SCREEN_FADES__
	//Fade in...
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
	UseResFile(interfaceResFileID);
	
	//Clean windows
	SetPort(backgroundWin);
	PaintRect(&(backgroundWin->portRect));
	SetPort(mainWin);
	PaintRect(&(mainWin->portRect));
	
	//Load buttons data
	buttonsData = (ButtonListHandle) Get1Resource(kButtonsResType, kCreditsButtonResID);
	if(buttonsData == nil)
	return ResError();
	DetachResource((Handle) buttonsData);
	HLock((Handle) buttonsData);
	if((**buttonsData).buttonCount != kCreditsNbButtons)
	return kError_FatalError;
	
	//Load button graphics
	for(i = 0; i < kCreditsNbButtons; ++i) {
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
	
	//Load graphics
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kCreditsBackgroundID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	
	UseResFile(mainResFileID);
	
	//Load text
	text = Get1Resource('TEXT', kCreditsTextID);
	if(text == nil)
	return ResError();
	GetResInfo(text, &resID, (unsigned long*) &i, name);
	name[0] = 0;
	while(name[name[0] + 1] != ' ')
	++name[0];
	StringToNum(name, &creditsTextHeight);
	DetachResource(text);
	HLock(text);
	
	UseResFile(interfaceResFileID);
	
	//Draw background
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld2, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	SetGWorld(savePort, saveDevice);
	SetRect(&theRect, 0, 0, kInterface_Width, kInterface_Height);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &theRect, &theRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &theRect, &theRect, srcCopy, nil);
	
	//Create text fonts
	fontSpecHandle = Get1Resource(kTE_FontResSpecID, kFontDefID);
	if(fontSpecHandle == nil) {
		SetGWorld(savePort, saveDevice);
		return ResError();
	}
	DetachResource(fontSpecHandle);
	HLock(fontSpecHandle);
	fontDef = (TE_ResFontDefinitionPtr) *fontSpecHandle;
	
	specs = (TE_FontSpecPtr) NewPtr(fontDef->fontSpecCount * sizeof(TE_FontSpec));
	for(i = 0; i < fontDef->fontSpecCount; ++i) {
		GetFNum(fontDef->fontSpecList[i].fontName, &specs[i].fontNum);
		if(specs[i].fontNum == 0)
		specs[i].fontNum = fontDef->fontSpecList[i].fontID;
		specs[i].fontSize = fontDef->fontSpecList[i].fontSize;
		specs[i].fontStyle = fontDef->fontSpecList[i].fontStyle;
		specs[i].fontColor = TextEngine_Color32To16(fontDef->fontSpecList[i].colorRed, fontDef->fontSpecList[i].colorGreen, fontDef->fontSpecList[i].colorBlue);
	}
	DisposeHandle(fontSpecHandle);
	
	//Load presenter
	Interface_GetLeahSpec(&leahSpec);
	theError = Interface_LoadAnimationWrapper(&leahSpec, kPlayer_DefaultTextureSet, kLeahPosH, kLeahPosV, kLeahWidth, kLeahHeight, kCameraCreditsID);
	if(theError)
	return theError;
	
	//Turn font smoothing off
	MacOSTextAliasing_Disable();
	
#if __SCREEN_FADES__
	//Fade out...
	InfinityPlayer_EraseBackground();
	Sound_PlaySFx(kSFxFadeOut);
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	return noErr;
}

static OSErr Credits_Quit()
{
	long				i;
	
	InfinityPlayer_UnloadAnimation();
	
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kCreditsNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	
	DisposeHandle(text);
	DisposePtr((Ptr) specs);
	MacOSTextAliasing_Restore();
	
	return noErr;
}

OSErr Credits_Display()
{
	Rect					screenRect;
	Point					textPoint;
	unsigned long			startTime;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	Boolean					run = true,
							over;
	Point					mouse;
	OSErr					theError;
	short					overButton = kButton_None,
							button;
	long					i;
	EventRecord				theEvent;
	unsigned char			theKey;
	
	theError = Credits_Start();
	if(theError)
	return theError;
	
	theError = InfinityPlayer_PlayAnimation(kCreditsStartID);
	if(theError)
	return theError;
	
	//Scroll!
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld2, NULL);
	textPoint.v = kInterface_Height - kBottomMargin;
	SetRect(&screenRect, kLeftMargin, kTopMargin, kInterface_Width, kInterface_Height - kBottomMargin);
	TextEngine_Settings_Clipping(kTopMargin, kInterface_Height - kBottomMargin);
	TextEngine_Settings_Margings(kLeftMargin, kRightMargin);
	
	startTime = RClock_GetAbsoluteTime();
	SetPort(mainWin);
	InitCursor();
	while(run) {
		//Scroll text
		if(textPoint.v > -creditsTextHeight) {
			if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
				theError = InfinityPlayer_PlayAnimation(kCreditsDisplayID);
				if(theError)
				return theError;
			}
			
			//Draw background
			CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &screenRect, &screenRect, srcCopy, nil);
			
			//Write text
			textPoint.v = kInterface_Height - kBottomMargin - (RClock_GetAbsoluteTime() - startTime) * kScrollingSpeed / 100;
			TextEngine_DrawFormattedText_WithFade(*text, GetHandleSize(text), specs, GetWindowPixMapPtr(bufferGWorld), textPoint, kFadeHeight);
			
			//Display buffer
			CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &screenRect, &screenRect, srcCopy, nil);
		}
		else {
			if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
				theError = InfinityPlayer_PlayAnimation(kLoop_Credits);
				if(theError)
				return theError;
			}
		}
		
		GetMouse(&mouse);
		
		//Are we over a button?
		button = kButton_None;
		for(i = 0; i < kCreditsNbButtons; ++i)
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
					//Scroll text
					if(textPoint.v > -creditsTextHeight) {
						if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
							theError = InfinityPlayer_PlayAnimation(kCreditsDisplayID);
							if(theError)
							return theError;
						}
						
						//Draw background
						CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &screenRect, &screenRect, srcCopy, nil);
						
						//Write text
						textPoint.v = kInterface_Height - kBottomMargin - (RClock_GetAbsoluteTime() - startTime) * kScrollingSpeed / 100;
						TextEngine_DrawFormattedText_WithFade(*text, GetHandleSize(text), specs, GetWindowPixMapPtr(bufferGWorld), textPoint, kFadeHeight);
						
						//Display buffer
						CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &screenRect, &screenRect, srcCopy, nil);
					}
					else {
						if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
							theError = InfinityPlayer_PlayAnimation(kLoop_Credits);
							if(theError)
							return theError;
						}
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
					if(overButton == kCreditsButton_Back) {
						run = false;
					}
				}
			}
		}
		
		//Did the user press a key combination?
		if(OSEventAvail(everyEvent, &theEvent)) {
			if((theEvent.what == keyDown) || (theEvent.what == autoKey)) {
				theKey = (theEvent.message & keyCodeMask) >> 8;
				
				button = kButton_None;
				if(theKey == keyEscape) {
					button = kCreditsButton_Back;
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
	
	HideCursor();
	theError = InfinityPlayer_PlayAnimation(kCreditsQuitID);
	if(theError)
	return theError;
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
	
	//Clean up
	SetGWorld(savePort, saveDevice);
	Credits_Quit();
	
	return noErr;
}
