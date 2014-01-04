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
#include					"Strings.h"
#include					"Data Files.h"
#include					"Interface.h"
#include					"Play Modes.h"
#include					"Preferences.h"

#include					"Clock.h"

#if __USE_AUDIO_CD_TRACKS__
#include					"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include					"QDesign Player.h"
#endif

#include					"Keys.h"

//CONSTANTES:

#define						kBackgroundID			3000
#define						kLoadButtonResID		130
#define						kSelectionBitmapID		3001
#define						kSelectionAlphaID		3002

#define						kAlphaGWorldSizeH		530
#define						kAlphaGWorldSizeV		30

#define						kUpdate_Left			68
#define						kUpdate_Right			595
#define						kUpdate_Top				125
#define						kUpdate_Bottom			360

#define						kMaxDisplayPilots		11
#define						kVSpace					21
#define						kTop					129

#define						kLineWidth				524
#define						kLineHeight				kVSpace
#define						kLinePosH				68

enum {kLoadButton_OK = 0, kLoadButton_Back, kLoadButton_Delete, kLoadButton_Up, kLoadButton_Down, kLoadNbButtons};

#define						kFontDefID				130

//VARIABLES LOCALES:

static long					pilotCount,
							pageNum;
PilotPtr					pilotList;

//ROUTINES:

static OSErr PilotMenu_BuildList()
{
	Str63					pilotFileName;
	CInfoPBRec				cipbr;
	HFileInfo				*fpb = (HFileInfo*) &cipbr;
	DirInfo					*dpb = (DirInfo*) &cipbr;
	long					idx;
	OSErr					theError;
	Pilot					tempPilot;
	long					i,
							j;
							
	//Clear
	pilotCount = 0;
	
	//Allocate memory
	pilotList = (PilotPtr) NewPtr(kMaxPilots * sizeof(Pilot));
	if(pilotList == nil)
	return MemError();
	
	//Scan folder for pilot files
	fpb->ioVRefNum = pilotsFolder.vRefNum;
	fpb->ioNamePtr = pilotFileName;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = pilotsFolder.parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		if(pilotCount == kMaxPilots)
		break;
		
		if(fpb->ioFlFndrInfo.fdType == kPilotFileType) {
#if __ENABLE_DATAFILE_SKIPPING__
			//If this pilot file begins with "_", skip it
			if(pilotFileName[1] == kSkipChar)
			continue;
#endif
			
			//Load pilot
			theError = Pilot_Load(pilotFileName, &pilotList[pilotCount]);
			if((theError == noErr) || (theError == kError_PilotFileCorrupted))
			++pilotCount;
		}
	}
	
	//Sort by score - best score first
	for(i = 0; i < pilotCount - 1; ++i)
	for(j = 0; j < pilotCount - i - 1; ++j)
	if(pilotList[j + 1].points > pilotList[j].points) {
		tempPilot = pilotList[j];
		pilotList[j] = pilotList[j + 1];
		pilotList[j + 1] = tempPilot;
	}
	
	return noErr;
}

static void PilotMenu_DrawPilot(short num, Boolean selected)
{
	PilotPtr				pilot = &pilotList[num];
	Str63					text;
	Point					destPoint;
	PixMapPtr				pixmap = *GetGWorldPixMap(bufferGWorld);
	Rect					theRect,
							copyRect;
	CharacterPreloadedDataPtr	characterData;
							
	//Check visibility
	if((num < pageNum * kMaxDisplayPilots) || (num >= (pageNum + 1) * kMaxDisplayPilots))
	return;
	
	//Calculate line position
	destPoint.v = kTop + (num - pageNum * kMaxDisplayPilots) * kVSpace;
	
	//Draw background if necessary
	if(selected) {
		SetRect(&copyRect, 0, 0, kLineWidth, kLineHeight);
		SetRect(&theRect, kLinePosH, destPoint.v, kLinePosH + kLineWidth, destPoint.v + kLineHeight);
		CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &copyRect, &copyRect, &theRect);
	}
	destPoint.v += 1;
	
	TextEngine_Settings_Text(specs[0].fontNum, specs[0].fontSize, specs[0].fontStyle);
	
	//Draw name
	destPoint.h = 78;
	TextEngine_DrawLine_Left((Ptr) &pilot->name[1], pilot->name[0], specs[0].fontColor, pixmap, destPoint);
	
	//Draw score
	destPoint.h = 224;
	PlayMode_GetStatus(pilot, text);
	TextEngine_DrawLine_Right((Ptr) &text[1], text[0], specs[0].fontColor, pixmap, destPoint);
	
	TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
	
	//Draw mode
	PlayMode_GetName(pilot->mode, text);
	destPoint.h = 272;
	TextEngine_DrawLine_Centered((Ptr) &text[1], text[0], specs[1].fontColor, pixmap, destPoint);
	
	//Draw difficulty
	GetIndString(text, kDifficultyStringID, pilot->difficulty);
	destPoint.h = 358;
	TextEngine_DrawLine_Centered((Ptr) &text[1], text[0], specs[1].fontColor, pixmap, destPoint);
	
	//Draw pilot name
	destPoint.h = 408;
	characterData = GetCharacterData_ByID(&coreData, pilot->character);
	if(characterData == nil)
	TextEngine_DrawLine_Left("?", 1, specs[1].fontColor, pixmap, destPoint);
	else
	TextEngine_DrawLine_Left((Ptr) &characterData->description.realName[1], characterData->description.realName[0], specs[1].fontColor, pixmap, destPoint);
}

static void PilotMenu_Update(short selected)
{
	long					i;
	Rect					copyRect;
	
	//Clean background
	SetRect(&copyRect, kUpdate_Left, kUpdate_Top, kUpdate_Right, kUpdate_Bottom);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &copyRect, srcCopy, nil);
	
	for(i = pageNum * kMaxDisplayPilots; i < iMin(pilotCount, (pageNum + 1) * kMaxDisplayPilots); ++i) {
		if(i == selected)
		PilotMenu_DrawPilot(i, true);
		else
		PilotMenu_DrawPilot(i, false);
	}
	
	//Copy
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
}

static void PilotMenu_Delete(short* num)
{
	OSErr					theError;
	long					i;
	
	theError = Pilot_Delete(pilotList[*num].name);
	if(theError) {
		Wrapper_Error_Display(149, theError, nil, false);
		InitCursor();
		return;
	}
	
	for(i = *num + 1; i < pilotCount; ++i)
	pilotList[i - 1] = pilotList[i];
	
	--pilotCount;
	if(*num >= pilotCount)
	*num -= 1;
	if(pageNum > pilotCount / kMaxDisplayPilots)
	pageNum = pilotCount / kMaxDisplayPilots;
}

OSErr PilotMenu_Start()
{
	Rect					theRect,
							alphaRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	OSErr					theError;
	PixMapHandle			pictPix;
	CTabHandle				theCLUT;
	Handle					fontSpecHandle;
	TE_ResFontDefinitionPtr	fontDef;
	
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
	if((**buttonsData).buttonCount != kLoadNbButtons)
	return kError_FatalError;
	
	//Load button graphics
	for(i = 0; i < kLoadNbButtons; ++i) {
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
	
	//Load & Draw alpha map
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kSelectionAlphaID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Load & draw image
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kSelectionBitmapID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	SetGWorld(bufferGWorld2, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	SetGWorld(savePort, saveDevice);
	
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
	pageNum = 0;
	PilotMenu_DrawPilot(0, true);
	for(i = 1; i < iMin(pilotCount, kMaxDisplayPilots); ++i)
	PilotMenu_DrawPilot(i, false);
	SetRect(&theRect, 0, 0, kInterface_Width, kInterface_Height);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &theRect, &theRect, srcCopy, nil);
	
#if __SCREEN_FADES__
	//Fade out...
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

	return noErr;
}

inline void MakeSelectedVisible(short selected)
{
	if((selected < pageNum * kMaxDisplayPilots) || (selected >= (pageNum + 1) * kMaxDisplayPilots))
	pageNum = selected / kMaxDisplayPilots;
}

Boolean PilotMenu_Run(Str63 name)
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
	short					selected = 0,
							newSelected;
	Boolean					loaded = false;
	Rect					zone = {kUpdate_Top, kUpdate_Left, kUpdate_Bottom, kUpdate_Right};
	unsigned long			lastClick = 0;
	Point					lastMouse = {0,0};
	Boolean					buttonDown;
	
	SetPort(mainWin);
	InitCursor();
	while(run) {
		GetMouse(&mouse);
		
		//Are we over a button?
		button = kButton_None;
		for(i = 0; i < kLoadNbButtons; ++i)
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
					if(overButton == kLoadButton_OK) {
						loaded = true;
						run = false;
					}
					else if(overButton == kLoadButton_Back)
					run = false;
					else if(overButton == kLoadButton_Delete) {
						PilotMenu_Delete(&selected);
						PilotMenu_Update(selected);
						if(pilotCount == 0)
						run = false;
					}
					else if(overButton == kLoadButton_Up) {
						if(pageNum > 0) {
							--pageNum;
							PilotMenu_Update(selected);
						}
					}
					else if(overButton == kLoadButton_Down) {
						if(pageNum < pilotCount / kMaxDisplayPilots) {
							++pageNum;
							PilotMenu_Update(selected);
						}
					}
				}
			}
			else if(PtInRect(mouse, &zone)) {
				newSelected = (mouse.v - kTop) / kVSpace;
				
				buttonDown = true;
				if((newSelected >= 0) && (newSelected + pageNum * kMaxDisplayPilots < pilotCount)) {
					newSelected += pageNum * kMaxDisplayPilots;
					if(newSelected != selected) {
						selected = newSelected;
						Sound_PlaySFx(kSFxItemSelection);
						PilotMenu_Update(selected);
					}
					else if((TickCount() - lastClick < GetDblTime()) && ((mouse.h - lastMouse.h) * (mouse.h - lastMouse.h) + (mouse.v - lastMouse.v) * (mouse.v - lastMouse.v) < 10 * 10)) {
						if(overButton != kButton_None)
						Interface_ButtonDraw(overButton, kNormal);
						overButton = kLoadButton_OK;
						Interface_ButtonDraw(overButton, kActive);
						Sound_PlaySFx(kSFxConfirm);
						startTime = RClock_GetAbsoluteTime();
						while((RClock_GetAbsoluteTime() - startTime) < kButtonDelay)
						;
						Interface_ButtonDraw(overButton, kNormal);
						
						loaded = true;
						run = false;
					}
				}
			}
		}
		else if(buttonDown) {
			lastClick = TickCount();
			lastMouse = mouse;
			buttonDown = false;
		}
		
		//Did the user press a key combination?
		if(OSEventAvail(everyEvent, &theEvent)) {
			if((theEvent.what == keyDown) || (theEvent.what == autoKey)) {
				theKey = (theEvent.message & keyCodeMask) >> 8;
				
				button = kButton_None;
				if(theKey == keyArrowUp) {
					--selected;
					if(selected < 0)
					selected = 0;
					Sound_PlaySFx(kSFxItemSelection);
					MakeSelectedVisible(selected);
					PilotMenu_Update(selected);
				}
				else if(theKey == keyArrowDown) {
					++selected;
					if(selected >= pilotCount)
					selected = pilotCount - 1;
					Sound_PlaySFx(kSFxItemSelection);
					MakeSelectedVisible(selected);
					PilotMenu_Update(selected);
				}
				else if(theKey == keyBackSpace) {
					button = kLoadButton_Delete;
					PilotMenu_Delete(&selected);
					MakeSelectedVisible(selected);
					PilotMenu_Update(selected);
					if(pilotCount == 0)
					run = false;
				}
				else if((theKey == keyReturn) || (theKey == keyEnter) || (theKey == keyEnterPB)) {
					button = kLoadButton_OK;
					loaded = true;
					run = false;
				}
				else if(theKey == keyEscape) {
					button = kLoadButton_Back;
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
	
	if(loaded)
	BlockMove(pilotList[selected].name, name, sizeof(Str31));
	
	return loaded;
}

OSErr PilotMenu_Quit()
{
	long					i;
	
	HideCursor();
	
	UnlockPixels(GetGWorldPixMap(alphaGWorld));
	DisposeGWorld(alphaGWorld);
				
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kLoadNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	
	DisposePtr((Ptr) specs);
	MacOSTextAliasing_Restore();
	
	DisposePtr((Ptr) pilotList);
	
	return noErr;
}

OSErr PilotMenu_Display(Str63 name)
{
	OSErr				theError;
	Boolean				loaded;
	
	theError = PilotMenu_BuildList();
	if(theError)
	return theError;
	if(pilotCount == 0) {
		DisposePtr((Ptr) pilotList);
		return kError_UserCancel;
	}
	
	theError = PilotMenu_Start();
	if(theError)
	return theError;
	
	loaded = PilotMenu_Run(name);
	
	PilotMenu_Quit();
	
	if(!loaded)
	return kError_UserCancel;
	
	return noErr;
}
