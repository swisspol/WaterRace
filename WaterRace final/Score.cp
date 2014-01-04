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

#define						kBackgroundID			6000
#define						kLoadButtonResID		180

enum {kScoreButton_OK = 0, kScoreNbButtons};

#define						kFontDefID				131

#define						kCharacterPosH			288 //(18 x 16)
#define						kCharacterPosV			50
#define						kCharacterWidth			(kInterface_Width - kCharacterPosH)
#define						kCharacterHeight		(kInterface_Height - kCharacterPosV)

#define						kVSpace					25
#define						kTop					140

//VARIABLES:

OSErr ScoreMenu_Start()
{
	Rect					theRect;
	long					i,
							j;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	Handle					fontSpecHandle;
	TE_ResFontDefinitionPtr	fontDef;
	OSErr					theError;
	Boolean					localWinner;
	CharacterPreloadedDataPtr	winnerData;
	Point					destPoint;
	PixMapPtr				pixmap = *GetGWorldPixMap(bufferGWorld);;
	PlayerConfigurationPtr	player;
	Str31					text;
	PlayerConfigurationPtr	sortedPlayerList[kMaxPlayers];
	
	GetGWorld(&savePort, &saveDevice);
	UseResFile(interfaceResFileID);
	
	//Sort players by increasing rank
	for(i = 0; i < gameConfig.playerCount; ++i)
	sortedPlayerList[i] = &gameConfig.playerList[i];
	for(i = 0; i < gameConfig.playerCount - 1; ++i)
	for(j = 0; j < gameConfig.playerCount - i - 1; ++j)
	if(sortedPlayerList[j + 1]->rank < sortedPlayerList[j]->rank) {
		player = sortedPlayerList[j];
		sortedPlayerList[j] = sortedPlayerList[j + 1];
		sortedPlayerList[j + 1] = player;
	}
	
#if 0
	//Find victory character data and see if winner is local
	winnerData = GetCharacterData_ByID(&coreData, sortedPlayerList[0]->characterID);
	if(winnerData == nil)
	return kError_CharacterNotFound;
	
	if(sortedPlayerList[0]->shipPilotType == kPilot_Local)
	localWinner = true;
	else
	localWinner = false;
#else
	//Find local character data and see if winner
	if((gameConfig.gameMode_save == kGameMode_Host) || (gameConfig.gameMode_save == kGameMode_Join))
	winnerData = &leahCharacterData;
	else {
		winnerData = GetCharacterData_ByID(&coreData, localShip->player->characterID);
		if(winnerData == nil)
		return kError_CharacterNotFound;
	}
	
	if(localShip->player->rank == 1)
	localWinner = true;
	else
	localWinner = false;
#endif
	
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
	if((**buttonsData).buttonCount != kScoreNbButtons)
	return kError_FatalError;
	
	//Load button graphics
	for(i = 0; i < kScoreNbButtons; ++i) {
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
	
	//Display scores
	TextEngine_Settings_Text(specs[0].fontNum, specs[0].fontSize, specs[0].fontStyle);
	for(i = 0; i < gameConfig.playerCount; ++i) {
		//Calculate line position
		destPoint.v = kTop + i * kVSpace;
		
		//Draw rank
		destPoint.h = 68;
		if(sortedPlayerList[i]->rank < 1) {
			text[0] = 1;
			text[1] = '-';
		}
		else
		NumToString(sortedPlayerList[i]->rank, text);
		TextEngine_DrawLine_Left((Ptr) &text[1], text[0], specs[0].fontColor, pixmap, destPoint);
		
		//Draw name
		destPoint.h = 154;
		TextEngine_DrawLine_Centered((Ptr) &sortedPlayerList[i]->playerName[1], sortedPlayerList[i]->playerName[0], specs[0].fontColor, pixmap, destPoint);
		
		//Draw time
		destPoint.h = 276;
		if(sortedPlayerList[i]->finished) {
			RaceTimeToString(sortedPlayerList[i]->finishTime, text);
			TextEngine_DrawLine_Right((Ptr) &text[1], text[0], specs[0].fontColor, pixmap, destPoint);
		}
		else {
			BlockMove(kString_Interface_NA, text, sizeof(Str31));
			TextEngine_DrawLine_Right((Ptr) &text[1], text[0], specs[0].fontColor, pixmap, destPoint);
		}
	}
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &theRect, &theRect, srcCopy, nil);
	
	//Load character
	theError = Interface_LoadAnimationWrapper(&winnerData->fileSpec, kPlayer_DefaultTextureSet, kCharacterPosH, kCharacterPosV, kCharacterWidth, kCharacterHeight, kCameraScoreID);
	if(theError)
	return theError;
	
	//Start animation
	if(localWinner)
	theError = InfinityPlayer_PlayAnimation(kCharacter_Win);
	else
	theError = InfinityPlayer_PlayAnimation(kCharacter_Loose);
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

OSErr ScoreMenu_Run()
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
		for(i = 0; i < kScoreNbButtons; ++i)
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
					if(overButton == kScoreButton_OK)
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
					button = kScoreButton_OK;
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

OSErr ScoreMenu_Quit()
{
	long					i;
	
	HideCursor();
	
	InfinityPlayer_UnloadAnimation();
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kScoreNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	
	DisposePtr((Ptr) specs);
	MacOSTextAliasing_Restore();
	
	return noErr;
}

OSErr ScoreMenu_Display()
{
	OSErr				theError;
	
	theError = ScoreMenu_Start();
	if(theError)
	return theError;
	
	theError = ScoreMenu_Run();
	
	ScoreMenu_Quit();
	
	return theError;
}
