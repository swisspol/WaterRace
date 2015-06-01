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

#include					"Keys.h"

//CONSTANTES:

#define						kBackgroundID			4000
#define						kLoadButtonResID		160

enum {kCharacterButton_OK = 0, kCharacterButton_Back, kCharacterButton_Left, kCharacterButton_Right, kCharacterNbButtons};

#define						kFontDefID				134

#define						kPreview_Left_H			44
#define						kPreview_Left_V			116
#define						kPreview_Central_H		117
#define						kPreview_Central_V		117
#define						kPreview_Right_H		222
#define						kPreview_Right_V		kPreview_Left_V

#define						kCharacterPosH			288 //(18 x 16)
#define						kCharacterPosV			50
#define						kCharacterWidth			(kInterface_Width - kCharacterPosH)
#define						kCharacterHeight		(kInterface_Height - kCharacterPosV)

#define						kDisplay_Left			10
#define						kDisplay_Top			80
#define						kDisplay_Right			288
#define						kDisplay_Bottom			366

//ROUTINES:

inline short ConvertNum(short num)
{
	if(num < 0)
	return coreData.characterCount + addOnData.characterCount - 1;
	else if(num >= coreData.characterCount + addOnData.characterCount)
	return 0;
	
	return num;
}

inline CharacterPreloadedDataPtr Character_GetDataByNum(short num)
{
	num = ConvertNum(num);
	
	if(num >= coreData.characterCount)
	return &addOnData.characterList[num - coreData.characterCount];
	else
	return &coreData.characterList[num];
}

static void CharacterMenu_DrawCharacter(short num)
{
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	Rect					displayRect = {kDisplay_Top, kDisplay_Left, kDisplay_Bottom, kDisplay_Right},
							theRect;
	Point					destPoint;
	PixMapPtr				pixmap = *GetGWorldPixMap(bufferGWorld);
	Str31					theString;
	CharacterPreloadedDataPtr	current,
								previous,
								next;
	short					temp;
	
	//Get data
	current = Character_GetDataByNum(num);
	previous = Character_GetDataByNum(num - 1);
	next = Character_GetDataByNum(num + 1);
	
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	
	//Copy background
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &displayRect, &displayRect, srcCopy, nil);
	
	//Display left preview
	theRect.left = kPreview_Left_H;
	theRect.top = kPreview_Left_V;
	theRect.right = kPreview_Left_H + (**previous->disablePreview).picFrame.right - (**previous->disablePreview).picFrame.left;
	theRect.bottom = kPreview_Left_V + (**previous->disablePreview).picFrame.bottom - (**previous->disablePreview).picFrame.top;
	DrawPicture(previous->disablePreview, &theRect);
	
	//Display right preview
	theRect.left = kPreview_Right_H;
	theRect.top = kPreview_Right_V;
	theRect.right = kPreview_Right_H + (**next->disablePreview).picFrame.right - (**next->disablePreview).picFrame.left;
	theRect.bottom = kPreview_Right_V + (**next->disablePreview).picFrame.bottom - (**next->disablePreview).picFrame.top;
	DrawPicture(next->disablePreview, &theRect);
	
	//Display central preview
	theRect.left = kPreview_Central_H;
	theRect.top = kPreview_Central_V;
	theRect.right = kPreview_Central_H + (**current->enablePreview).picFrame.right - (**current->enablePreview).picFrame.left;
	theRect.bottom = kPreview_Central_V + (**current->enablePreview).picFrame.bottom - (**current->enablePreview).picFrame.top;
	DrawPicture(current->enablePreview, &theRect);
	
	//Display pilot name
	TextEngine_Settings_Text(specs[0].fontNum, specs[0].fontSize, specs[0].fontStyle);
	destPoint.h = 153;
	destPoint.v = 207;
	TextEngine_DrawLine_Centered((Ptr) &current->description.realName[1], current->description.realName[0], specs[0].fontColor, pixmap, destPoint);
	
	//Display pilot age
	TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
	BlockMove("\pxx years old", theString, sizeof(Str31));
	temp = theString[0];
	NumToString(current->description.age, theString);
	theString[0] = temp;
	destPoint.h = 96;
	destPoint.v = 237;
	TextEngine_DrawLine_Centered((Ptr) &theString[1], theString[0], specs[1].fontColor, pixmap, destPoint);
	
	//Display pilot origin
	destPoint.h = 218;
	destPoint.v = 237;
	TextEngine_DrawLine_Centered((Ptr) &current->description.origin[1], current->description.origin[0], specs[1].fontColor, pixmap, destPoint);
	
	//Display pilot text
	TextEngine_Settings_Text(specs[2].fontNum, specs[2].fontSize, specs[2].fontStyle);
	destPoint.h = 32;
	destPoint.v = 260;
	TextEngine_DrawText_Left((Ptr) &current->description.description[1], current->description.description[0], specs[2].fontColor, pixmap, destPoint);
	
	//Display...
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &displayRect, &displayRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
}

OSErr CharacterMenu_Start()
{
	Rect					theRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	Handle					fontSpecHandle;
	TE_ResFontDefinitionPtr	fontDef;
	OSErr					theError;
	
#if __SCREEN_FADES__
	//Fade in...
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	GetGWorld(&savePort, &saveDevice);
	UseResFile(interfaceResFileID);
	
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
	if((**buttonsData).buttonCount != kCharacterNbButtons)
	return kError_FatalError;
	
	//Load button graphics
	for(i = 0; i < kCharacterNbButtons; ++i) {
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
	
	//Create text fonts
	fontSpecHandle = Get1Resource(kTE_FontResSpecID, kFontDefID);
	if(fontSpecHandle == nil)
	return ResError();
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
	
	//Turn font smoothing off
	MacOSTextAliasing_Disable();
	TextEngine_Settings_Clipping(0, kInterface_Height);
	
	//Draw pilot list
	CharacterMenu_DrawCharacter(0);
	
	//Load character
	theError = Interface_LoadAnimationWrapper(&Character_GetDataByNum(0)->fileSpec, kPlayer_DefaultTextureSet, kCharacterPosH, kCharacterPosV, kCharacterWidth, kCharacterHeight, kCameraCharacterMenuID);
	if(theError)
	return theError;
	
	//Start animation
	theError = InfinityPlayer_PlayAnimation(kCharacter_Intro);
	if(theError)
	return theError;
	
#if __SCREEN_FADES__
	//Fade out...
	InfinityPlayer_EraseBackground();
	Sound_PlaySFx(kSFxFadeOut);
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
	AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, QDChannel, thePrefs.musicVolume);
#else
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
#endif

#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#endif
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
#endif
	
	return noErr;
}

static OSType GetRandomLoop()
{
	switch((Random() + 32767) % 2) {
		case 0: return kLoop_Character_1; break;
		case 1: return kLoop_Character_2; break;
	}
	
	return kNoID;
}

static OSErr Introduce_Character(short num)
{
	OSErr				theError;
	
	//Play quit animation
	theError = InfinityPlayer_PlayAnimation(kCharacter_Quit);
	if(theError)
	return theError;
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#else
	HideCursor();
#endif
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
	
	//Unload current character
	InfinityPlayer_UnloadAnimation();
	
	//Load new character
	theError = Interface_LoadAnimationWrapper(&Character_GetDataByNum(num)->fileSpec, kPlayer_DefaultTextureSet, kCharacterPosH, kCharacterPosV, kCharacterWidth, kCharacterHeight, kCameraCharacterMenuID);
	if(theError) {
		SpinningCursor_Stop();
		return theError;
	}

	//Start animation
	theError = InfinityPlayer_PlayAnimation(kCharacter_Intro);
	if(theError) {
		SpinningCursor_Stop();
		return theError;
	}
	CharacterMenu_DrawCharacter(num);
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
#else
	ShowCursor();
#endif
	
	return noErr;
}

OSErr CharacterMenu_Run(OSType* characterID)
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
	Boolean					loaded = false;
	OSErr					theError;
	
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
		for(i = 0; i < kCharacterNbButtons; ++i)
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
					if(overButton == kCharacterButton_OK) {
						loaded = true;
						run = false;
					}
					else if(overButton == kCharacterButton_Back)
					run = false;
					else if(overButton == kCharacterButton_Left) {
						selected = ConvertNum(selected - 1);
						theError = Introduce_Character(selected);
						if(theError)
						return theError;
					}
					else if(overButton == kCharacterButton_Right) {
						selected = ConvertNum(selected + 1);
						theError = Introduce_Character(selected);
						if(theError)
						return theError;
					}
				}
			}
		}
		
		//Did the user press a key combination?
		if(OSEventAvail(everyEvent, &theEvent)) {
			if((theEvent.what == keyDown) || (theEvent.what == autoKey)) {
				theKey = (theEvent.message & keyCodeMask) >> 8;
				
				button = kButton_None;
				if(theKey == keyArrowLeft) {
					button = kCharacterButton_Left;
					selected = ConvertNum(selected - 1);
					theError = Introduce_Character(selected);
					if(theError)
					return theError;
				}
				else if(theKey == keyArrowRight) {
					button = kCharacterButton_Right;
					selected = ConvertNum(selected + 1);
					theError = Introduce_Character(selected);
					if(theError)
					return theError;
				}
				else if((theKey == keyReturn) || (theKey == keyEnter) || (theKey == keyEnterPB)) {
					button = kCharacterButton_OK;
					loaded = true;
					run = false;
				}
				else if(theKey == keyEscape) {
					button = kCharacterButton_Back;
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
	
	//End current animation
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#else
	HideCursor();
#endif
	InfinityPlayer_RunSync(0, kPlayer_DefaultMaxFPS);
	
	//Play selected animation
	if(loaded)
	InfinityPlayer_PlayAnimation(kCharacter_Selected);
	else
	InfinityPlayer_PlayAnimation(kCharacter_Quit);
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
	
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
#endif

	if(loaded)
	*characterID = Character_GetDataByNum(selected)->ID;
	else {
		*characterID = kNoID;
		return kError_UserCancel;
	}
	
	return noErr;
}

OSErr CharacterMenu_Quit()
{
	long					i;
	
	HideCursor();
	
	InfinityPlayer_UnloadAnimation();
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kCharacterNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	
	DisposePtr((Ptr) specs);
	MacOSTextAliasing_Restore();
	
	return noErr;
}

OSErr CharacterMenu_Display(OSType* characterID)
{
	OSErr				theError;
	
	if((coreData.characterCount == 0) && (addOnData.characterCount == 0))
	return kError_FatalError;
	
	theError = CharacterMenu_Start();
	if(theError)
	return theError;
	
	theError = CharacterMenu_Run(characterID);
	
	CharacterMenu_Quit();
	
	return theError;
}
