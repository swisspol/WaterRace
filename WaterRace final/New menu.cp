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
#include					"Play Modes.h"

#include					"Clock.h"
#include					"Infinity Player.h"
#include					"Infinity Audio Engine.h"

#if __USE_AUDIO_CD_TRACKS__
#include					"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include					"QDesign Player.h"
#endif

#if __SHOW_SPINNING_CURSOR__
#include					"Dialog Utils.h"
#endif

#include					"Keys.h"
#include					"Sound Utils.h"

//CONSTANTES:

#define						kNewBitmapID			300
#define						kNewAlphaID				301
#define						kNewButtonResID			129
#define						kButtonImageOffset		300

#define						kNewButtonSoundOffset	50
#define						kNewButtonTotalWidth	288
#define						kNewButtonTotalHeight	222
#define						kNewButtonOffsetH		0
#define						kNewButtonOffsetV		104
#define						kNewMoveSpeed			(kNewButtonTotalWidth / 60)
#define						kNewScrollDuration		80

#define						kUpdate_Left			130
#define						kUpdate_Right			252
#define						kUpdate_Top				50
#define						kUpdate_Bottom			150
#define						kUpdate_NameBottom		84

#define						kDefaultNameStringID	1002

#define						kSound_StartID			200
enum {kSound_Wrong, kSound_Key, kSound_Delete, kNbSounds};

#define						kFontDefID				129

#define						kBlickTimeStartDelay	90

//VARIABLES LOCALES:

static Handle				sounds[kNbSounds];
static unsigned long		startBlinkTime;
static long					pilotCount;
static Str31				pilotNameTable[kMaxPilots];
	
//ROUTINES:

static void BuildPilotNameTable()
{
	Str63					pilotFileName;
	CInfoPBRec				cipbr;
	HFileInfo				*fpb = (HFileInfo*) &cipbr;
	DirInfo					*dpb = (DirInfo*) &cipbr;
	long					idx;
	OSErr					theError;
	Pilot					tempPilot;
							
	pilotCount = 0;
	
	//Scan folder for pilot files
	fpb->ioVRefNum = pilotsFolder.vRefNum;
	fpb->ioNamePtr = pilotFileName;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = pilotsFolder.parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		if(fpb->ioFlFndrInfo.fdType == kPilotFileType) {
			//Load pilot
			theError = Pilot_Load(pilotFileName, &tempPilot);
			if(theError && (theError != kError_PilotFileCorrupted))
			return;
			
			//Copy name
			BlockMove(tempPilot.name, pilotNameTable[pilotCount], sizeof(Str31));
			++pilotCount;
			
			if(pilotCount >= kMaxPilots)
			return;
		}
	}
}

static Boolean NameAlreadyInUse(Str31 name)
{
	long					i;
	
	for(i = 0; i < pilotCount; ++i)
	if(EqualString(pilotNameTable[i], name, false, false))
	return true;
	
	return false;
}

static void NewMenu_DrawDisplay(Str31 name, OSType mode, short difficulty, PixMapPtr pixmap, short hOffset)
{
	Str63					text;
	Point					destPoint;
	
	TextEngine_Settings_Text(specs[0].fontNum, specs[0].fontSize, specs[0].fontStyle);
	
	//Draw name
	destPoint.h = hOffset + 194;
	destPoint.v = 161;
	if(NameAlreadyInUse(name))
	TextEngine_DrawLine_Centered((Ptr) &name[1], name[0], TextEngine_Color16(255,0,0), pixmap, destPoint);
	else
	TextEngine_DrawLine_Centered((Ptr) &name[1], name[0], specs[0].fontColor, pixmap, destPoint);
	
	TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
	
	//Draw difficulty
	GetIndString(text, kDifficultyStringID, difficulty);
	destPoint.h = hOffset + 194;
	destPoint.v = 191;
	TextEngine_DrawLine_Centered((Ptr) &text[1], text[0], specs[1].fontColor, pixmap, destPoint);
	
	//Draw mode
	PlayMode_GetName(mode, text);
	destPoint.h = hOffset + 194;
	destPoint.v = 221;
	TextEngine_DrawLine_Centered((Ptr) &text[1], text[0], specs[1].fontColor, pixmap, destPoint);
}

static void NewMenu_BlinkName()
{
	Rect					theRect,
							copyRect;
	
	SetRect(&copyRect, kNewButtonOffsetH + kUpdate_Left, kNewButtonOffsetV + kUpdate_Top, kNewButtonOffsetH + kUpdate_Right, kNewButtonOffsetV + kUpdate_NameBottom);
	if(TickCount() < startBlinkTime)
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
	else {
		if(TickCount() % 60 > 30)
		CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
		else {
			SetRect(&theRect, kButtonImageOffset + kUpdate_Left, kNewButtonOffsetV + kUpdate_Top, kButtonImageOffset + kUpdate_Right, kNewButtonOffsetV + kUpdate_NameBottom);
			CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &theRect, &copyRect, srcCopy, nil);
		}
	}
}

static void NewMenu_UpdateDisplay(Str31 name, OSType mode, short difficulty)
{
	Rect					theRect,
							copyRect;
	PixMapPtr				pixmap = *GetGWorldPixMap(bufferGWorld);
	
	//Clean background
	SetRect(&theRect, kButtonImageOffset + kUpdate_Left, kNewButtonOffsetV + kUpdate_Top, kButtonImageOffset + kUpdate_Right, kNewButtonOffsetV + kUpdate_Bottom);
	SetRect(&copyRect, kNewButtonOffsetH + kUpdate_Left, kNewButtonOffsetV + kUpdate_Top, kNewButtonOffsetH + kUpdate_Right, kNewButtonOffsetV + kUpdate_Bottom);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &theRect, &copyRect, srcCopy, nil);
	
	NewMenu_DrawDisplay(name, mode, difficulty, pixmap, 0);
	
	//Copy
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
	
	startBlinkTime = TickCount() + kBlickTimeStartDelay;
}

OSErr NewMenu_Start()
{
	Rect					theRect,
							destRect,
							copyRect,
							alphaRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	unsigned long			startTime,
							time;
	short					realPos;
	PicHandle				backgroundPic;
	Boolean					done = false;
	Handle					fontSpecHandle;
	TE_ResFontDefinitionPtr	fontDef;
	Str31					defaultName;
	OSErr					theError;
	
	GetGWorld(&savePort, &saveDevice);
	UseResFile(interfaceResFileID);
	
	//Load buttons data
	buttonsData = (ButtonListHandle) Get1Resource(kButtonsResType, kNewButtonResID);
	if(buttonsData == nil)
	return ResError();
	DetachResource((Handle) buttonsData);
	HLock((Handle) buttonsData);
	if((**buttonsData).buttonCount != kNewNbButtons)
	return kError_FatalError;
	
	//Animate character
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
		
	//Load button graphics
	for(i = 0; i < kNewNbButtons; ++i) {
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
		InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);	
	}
	
	//Load & Draw alpha map
	SetGWorld(alphaGWorld, NULL);
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kNewAlphaID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Animate character
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
		
	//Load & draw image
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kNewBitmapID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	SetGWorld(bufferGWorld2, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left + kButtonImageOffset, -theRect.top + kNewButtonOffsetV);
	DrawPicture(backgroundPic, &theRect);
	SetGWorld(savePort, saveDevice);
	
	//Animate character
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	
	//Load sounds
	UseResFile(soundResFileID);
	for(i = 0; i < kNbSounds; ++i) {
		theError = Sound_ResLoad(kSound_StartID + i, false, &sounds[i]);
		if(theError)
		return theError;
		
		//Animate character
		InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	}
	UseResFile(interfaceResFileID);
	
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
	
	//Draw default text
	GetIndString(defaultName, kDefaultNameStringID, 1);
#if __DEMO_MODE__
	NewMenu_DrawDisplay(defaultName, kPilotType_FreePlay, kDifficulty_Easy, *GetGWorldPixMap(bufferGWorld2), kButtonImageOffset);
#else
	NewMenu_DrawDisplay(defaultName, kPilotType_FreePlay, kDifficulty_Medium, *GetGWorldPixMap(bufferGWorld2), kButtonImageOffset);
#endif
	
	//Animation...
	SetRect(&copyRect, kNewButtonOffsetH, kNewButtonOffsetV, kNewButtonOffsetH + kNewButtonTotalWidth, kNewButtonOffsetV + kNewButtonTotalHeight);
	
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#endif

	startTime = RClock_GetAbsoluteTime();
	do {
		if(Button())
		break;
		
		time = RClock_GetAbsoluteTime() - startTime;
		
		//Animate character
		InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
		
		//Move palette
		realPos = time * kNewMoveSpeed;
		CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &copyRect, srcCopy, nil);
		
		if((realPos >= kNewButtonTotalWidth - kNewButtonSoundOffset) && !done) {
			Sound_PlaySFx(kSfxAppear);
			done = true;
		}
		if(realPos >= kNewButtonTotalWidth)
		realPos = kNewButtonTotalWidth;
		
		alphaRect.top = 0;
		alphaRect.bottom = alphaRect.top + kNewButtonTotalHeight;
		alphaRect.left = kNewButtonTotalWidth - realPos;
		alphaRect.right = kNewButtonTotalWidth;

		theRect.top = kNewButtonOffsetV;
		theRect.bottom = theRect.top + kNewButtonTotalHeight;
		theRect.left = kButtonImageOffset + kNewButtonTotalWidth - realPos;
		theRect.right = kButtonImageOffset + kNewButtonTotalWidth;

		destRect.top = kNewButtonOffsetV;
		destRect.bottom = destRect.top + kNewButtonTotalHeight;
		destRect.right = realPos;
		destRect.left = 0;
		
		CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &alphaRect, &theRect, &destRect);
		CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
	} while(time <= kNewScrollDuration);
	
	SetGWorld(savePort, saveDevice);
	SetRect(&theRect, kButtonImageOffset, kNewButtonOffsetV, kButtonImageOffset + kNewButtonTotalWidth, kNewButtonOffsetV + kNewButtonTotalHeight);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &theRect, &copyRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &theRect, &copyRect, srcCopy, nil);
	
	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
#endif
	
	//Restore intact image
	SetGWorld(bufferGWorld2, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left + kButtonImageOffset, -theRect.top + kNewButtonOffsetV);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	SetGWorld(savePort, saveDevice);
	
	return noErr;
}

static OSType GetRandomLoop()
{
	switch((Random() + 32767) % 4) {
		case 0: return kLoop_New_1; break;
		case 1: return kLoop_New_2; break;
		case 2: return kLoop_New_3; break;
		case 3: return kLoop_New_4; break;
	}
	
	return kNoID;
}

short NewMenu_Run(Str31 name, OSType* mode, short* difficulty)
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
	unsigned char			theKey;
#if __REPLACE_NAME__
	Boolean					nameModified = false;
#endif
	
	//Init variables
	BuildPilotNameTable();
#if __DEMO_MODE__
	*difficulty = kDifficulty_Easy;
#else
	*difficulty = kDifficulty_Medium;
#endif
	*mode = kPilotType_FreePlay;
	
	//Loop presenter
	InfinityPlayer_PlayAnimation(GetRandomLoop());
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
	
	GetIndString(name, kDefaultNameStringID, 1);
	NewMenu_UpdateDisplay(name, *mode, *difficulty);
	SetPort(mainWin);
	FlushEvents(everyEvent, 0);
	startBlinkTime = TickCount() + kBlickTimeStartDelay;
	while(run) {
		if(InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS))
		InfinityPlayer_PlayAnimation(GetRandomLoop());
		
		NewMenu_BlinkName();
		
		GetMouse(&mouse);
		
		//Are we over a button?
		button = kButton_None;
		for(i = 0; i < kNewNbButtons; ++i)
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
				case kNewButton_OK: InfinityPlayer_PlayAnimation(kNewPilotOKID); break;
				case kNewButton_Back: InfinityPlayer_PlayAnimation(kNewPilotBackID); break;
			}
		}
		overButton = button;
		
		//Did the user press the mouse button?
		if(Button() && (overButton != kButton_None)) {
			Interface_ButtonDraw(overButton, kActive);
			if((overButton == kNewButton_OK) && !name[0])
			Sound_PlaySFxHandle(sounds[kSound_Wrong]);
			else
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
			if(over) {
				if(overButton == kNewButton_OK) {
#if __REPLACE_NAME__
					if(name[0] && !NameAlreadyInUse(name) && nameModified)
#else
					if(name[0] && !NameAlreadyInUse(name))
#endif
					run = false;
					else
					Sound_PlaySFxHandle(sounds[kSound_Wrong]);
				}
				else if(overButton == kNewButton_Back)
				run = false;
				else {
					if(overButton == kNewButton_Difficulty) {
						*difficulty += 1;
						if(*difficulty > kDifficulty_Hard)
						*difficulty = kDifficulty_Easy;
						
						switch(*difficulty) {
							case kDifficulty_Easy: InfinityPlayer_PlayAnimation(kNewEasy); break;
							case kDifficulty_Medium: InfinityPlayer_PlayAnimation(kNewMedium); break;
							case kDifficulty_Hard: InfinityPlayer_PlayAnimation(kNewHard); break;
						}
					}
					else if(overButton == kNewButton_Mode) {
						PlayMode_GetNext(mode);
						InfinityPlayer_PlayAnimation(PlayMode_GetLeahAnim(*mode));
					}
					NewMenu_UpdateDisplay(name, *mode, *difficulty);
				}
			}
		}
		
		//Did the user press a key?
		if(OSEventAvail(everyEvent, &theEvent)) {
			if((theEvent.what == keyDown) || (theEvent.what == autoKey)) {
				theChar = (theEvent.message & charCodeMask);
				if(((theChar >= 'a') && (theChar <= 'z')) || ((theChar >= 'A') && (theChar <= 'Z')) || (theChar == ' ')) {
#if __REPLACE_NAME__
					if(!nameModified) {
						name[0] = 0;
						nameModified = true;
					}
#endif
					if(name[0] < kPilot_MaxNameSize) {
						name[name[0] + 1] = theChar;
						++name[0];
						Sound_PlaySFxHandle(sounds[kSound_Key]);
						NewMenu_UpdateDisplay(name, *mode, *difficulty);
					}
					else
					Sound_PlaySFxHandle(sounds[kSound_Wrong]);
				}
				else {
					theKey = (theEvent.message & keyCodeMask) >> 8;
					
					button = kButton_None;
					if((theKey == keyArrowUp) || (theKey == keyArrowDown)) {
						button = kNewButton_Mode;
						
						PlayMode_GetNext(mode);
						InfinityPlayer_PlayAnimation(PlayMode_GetLeahAnim(*mode));
						NewMenu_UpdateDisplay(name, *mode, *difficulty);
					}
					else if((theKey == keyArrowLeft) || (theKey == keyArrowRight)) {
						button = kNewButton_Difficulty;
						if(theKey == keyArrowLeft)
						*difficulty -= 1;
						else
						*difficulty += 1;
						if(*difficulty > kDifficulty_Hard)
						*difficulty = kDifficulty_Easy;
						if(*difficulty < kDifficulty_Easy)
						*difficulty = kDifficulty_Hard;
						
						switch(*difficulty) {
							case kDifficulty_Easy: InfinityPlayer_PlayAnimation(kNewEasy); break;
							case kDifficulty_Medium: InfinityPlayer_PlayAnimation(kNewMedium); break;
							case kDifficulty_Hard: InfinityPlayer_PlayAnimation(kNewHard); break;
						}
						NewMenu_UpdateDisplay(name, *mode, *difficulty);
					}
					else if(theKey == keyBackSpace) {
#if __REPLACE_NAME__
						if(!nameModified) {
							name[0] = 0;
							Sound_PlaySFxHandle(sounds[kSound_Delete]);
							NewMenu_UpdateDisplay(name, *mode, *difficulty);
							nameModified = true;
						}
						else {
#endif
							if(name[0]) {
								--name[0];
								Sound_PlaySFxHandle(sounds[kSound_Delete]);
								NewMenu_UpdateDisplay(name, *mode, *difficulty);
							}
							else
							Sound_PlaySFxHandle(sounds[kSound_Wrong]);
#if __REPLACE_NAME__
						}
#endif
					}
					else if((theKey == keyReturn) || (theKey == keyEnter) || (theKey == keyEnterPB)) {
						button = kNewButton_OK;
#if __REPLACE_NAME__
						if(name[0] && !NameAlreadyInUse(name) && nameModified)
#else
						if(name[0] && !NameAlreadyInUse(name))
#endif
						run = false;
						else {
							if(overButton != kButton_None)
							Interface_ButtonDraw(overButton, kNormal);
							overButton = button;
							Interface_ButtonDraw(overButton, kActive);
							Sound_PlaySFxHandle(sounds[kSound_Wrong]);
							startTime = RClock_GetAbsoluteTime();
							while((RClock_GetAbsoluteTime() - startTime) < kButtonDelay) {
								InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
							}
							Interface_ButtonDraw(overButton, kNormal);
						}
					}
					else if(theKey == keyEscape) {
						button = kNewButton_Back;
						run = false;
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
					}
				}
			}
			FlushEvents(everyEvent, 0);
		}
	}
	
	//End current animation
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#else
	if(overButton == kNewButton_OK)
	HideCursor();
#endif
	InfinityPlayer_RunSync(0, kPlayer_DefaultMaxFPS);
	
	//Start transfer animation
	switch(overButton) {
	
		case kNewButton_OK:
		InfinityPlayer_PlayAnimation(kNewEndID);
		break;
		
		case kNewButton_Back:
		InfinityPlayer_PlayAnimation(kNewBackTransitionID);
		break;
		
	}
	
	return overButton;
}

OSErr NewMenu_Quit()
{
	Rect					theRect,
							destRect,
							copyRect,
							alphaRect;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	unsigned long			startTime,
							time;
	Boolean					done;
	short					realPos;
	long					i;
	
	UseResFile(interfaceResFileID);
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(alphaGWorld, NULL);
	
	//Load & Draw alpha map
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kNewAlphaID);
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
	
	//Copy back image
	SetRect(&theRect, kButtonImageOffset, kNewButtonOffsetV, kButtonImageOffset + kNewButtonTotalWidth, kNewButtonOffsetV + kNewButtonTotalHeight);
	SetRect(&copyRect, kNewButtonOffsetH, kNewButtonOffsetV, kNewButtonOffsetH + kNewButtonTotalWidth, kNewButtonOffsetV + kNewButtonTotalHeight);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(bufferGWorld2), &copyRect, &theRect, srcCopy, nil);
	
	//Animate character
	InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
		
	//Animation...
	SetRect(&copyRect, kNewButtonOffsetH, kNewButtonOffsetV, kNewButtonOffsetH + kNewButtonTotalWidth, kNewButtonOffsetV + kNewButtonTotalHeight);
	
	startTime = RClock_GetAbsoluteTime();
	do {
		if(Button())
		break;
	
		time = RClock_GetAbsoluteTime() - startTime;
		
		//Animate character
		InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS);
		
		//Move palette
		CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &copyRect, srcCopy, nil);
		
		realPos = kNewButtonTotalWidth - time * kNewMoveSpeed;
		if(realPos < 0)
		continue;
		
		if((realPos < kNewButtonTotalWidth) && !done) {
			Sound_PlaySFx(kSfxDisappear);
			done = true;
		}
		if(realPos >= kNewButtonTotalWidth)
		realPos = kNewButtonTotalWidth;
		
		alphaRect.top = 0;
		alphaRect.bottom = alphaRect.top + kNewButtonTotalHeight;
		alphaRect.left = kNewButtonTotalWidth - realPos;
		alphaRect.right = kNewButtonTotalWidth;

		theRect.top = kNewButtonOffsetV;
		theRect.bottom = theRect.top + kNewButtonTotalHeight;
		theRect.left = kButtonImageOffset + kNewButtonTotalWidth - realPos;
		theRect.right = kButtonImageOffset + kNewButtonTotalWidth;

		destRect.top = kNewButtonOffsetV;
		destRect.bottom = destRect.top + kNewButtonTotalHeight;
		destRect.right = realPos;
		destRect.left = 0;
			
		CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &alphaRect, &theRect, &destRect);
		CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
	} while(time <= kNewScrollDuration);
	
	SetGWorld(savePort, saveDevice);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &copyRect, srcCopy, nil);

	InfinityPlayer_RunSync(kPlayerFlag_StopOnMouse + kPlayerFlag_DisplayLastFrameOnStop, kPlayer_DefaultMaxFPS);
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
	HideCursor();
#endif
	
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kNewNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	for(i = 0; i < kNbSounds; ++i)
	DisposeHandle(sounds[i]);
	
	DisposePtr((Ptr) specs);
	MacOSTextAliasing_Restore();
	
	return noErr;
}
