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


#ifndef __WATERRACE_INTERFACE__
#define __WATERRACE_INTERFACE__

#include				"Text Engine.h"
#include				"Pilots.h"
#include				"Game.h"

//CONSTANTES:

#define					kInterface_Width		640
#define					kInterface_Height		480
#define					kInterface_Depth		16

#define					kGreyCLUTID				128

#define					kButtonsResType			'Btns'
#define					kMaxNbButtons			16
#define					kButton_None			(-1)

#define					kDifficultyStringID		1000
#define					kModeStringID			1001

enum {kMainButton_NewPilot = 0, kMainButton_LoadPilot, kMainButton_NetGame, kMainButton_Options, kMainButton_Credits, kMainButton_Quit, kMainNbButtons};
enum {kNewButton_OK = 0, kNewButton_Back, kNewButton_Difficulty, kNewButton_Mode, kNewNbButtons};

enum {kNormal = 1, kOver, kActive};

//LEAH SCRIPT IDs:

#define					kLeahID					'Leah'

#define					kStartID				'Strt'
#define					kEndID					'End '
#define					kNewPilotID				'NwPl'
#define					kLoadPilotID			'LdPl'
#define					kNetGameID				'NtGm'
#define					kOptionsID				'Optn'
#define					kCreditsID				'Crdt'
#define					kGenericTransitionID	'Tran'
#define					kNewTransitionID		'NTrn'
#define					kQuitID					'Quit'

#define					kLoop_Main_1			'LpM1'
#define					kLoop_Main_2			'LpM2'
#define					kLoop_Main_3			'LpM3'
#define					kLoop_Main_4			'LpM4'
#define					kLoop_Main_5			'LpM5'
#define					kLoop_Main_6			'LpM6'
#define					kLoop_Main_7			'LpM7'

#define					kNewPilotOKID			'NPOK'
#define					kNewPilotBackID			'NPBk'
#define					kNewEndID				'NEnd'
#define					kNewBackTransitionID	'NBTr'
#define					kNewEasy				'NEsy'
#define					kNewMedium				'NMed'
#define					kNewHard				'NHrd'
#define					kNewTournament			'NTrm'
#define					kNewTimeRace			'NTmR'
#define					kNewDuel				'NDel'
#define					kNewFreePlay			'NFrP'
#define					kNewAddOns				'NAOn'

#define					kLoop_New_1				'LpN1'
#define					kLoop_New_2				'LpN2'
#define					kLoop_New_3				'LpN3'
#define					kLoop_New_4				'LpN4'

#define					kCreditsStartID			'CrSt'
#define					kCreditsDisplayID		'CrRn'
#define					kCreditsQuitID			'CrQt'

#define					kLoop_Credits			'LpCd'

//PILOTS SCRIPT IDs:

#define					kCharacter_Intro		'CItr'
#define					kLoop_Character_1		'LpC1'
#define					kLoop_Character_2		'LpC2'
#define					kCharacter_Quit			'CQit'
#define					kCharacter_Selected		'CSel'
#define					kCharacter_Selected		'CSel'
#define					kCharacter_Win			'CWin'
#define					kCharacter_Loose		'CLos'
#define					kCharacter_Score_Loop_1	'CLp1'
#define					kCharacter_Score_Loop_2	'CLp2'

//CAMERA CONSTANTES:

#define					kCameraResType			'Camr'
#define					kCameraMainMenuID		128
#define					kCameraCreditsID		129
#define					kCameraCharacterMenuID	130
#define					kCameraShipPreviewID	131
#define					kCameraChampionID		132
#define					kCameraScoreID			133

//STRUCTURES:

#pragma options align=mac68k

typedef struct {
	Rect				hotZone;
	Point				where;
	short				overID,
						downID;
} ButtonDefinition;

typedef struct {
	short				buttonCount;
	ButtonDefinition	buttonList[];
} ButtonList;
typedef ButtonList* ButtonListPtr;
typedef ButtonList** ButtonListHandle;

#pragma options align=reset

//VARIABLES:

extern PicHandle			buttonsPic[kMaxNbButtons][2];
extern ButtonListHandle		buttonsData;
extern TE_FontSpecPtr		specs;
extern GWorldPtr			alphaGWorld;

//PROTOTYPES:

//File: Interface.cp
OSErr Interface_RunLocalGame(PilotPtr pilot);
void Interface_GetLeahSpec(FSSpec* fileSpec);
void Interface_ButtonDraw(short num, short state);
void Interface_SetCharacterParametersFromResource(short resID);
OSErr Interface_LoadAnimationWrapper(FSSpec* file, OSType textureSetID, short posH, short posV, short width, short height, short cameraResID);
void Interface_Display();

//File: Main Menu.cp
OSErr MainMenu_Display(short* button, Str31 name, OSType* mode, short* difficulty);

//File: New menu.cp
OSErr NewMenu_Start();
short NewMenu_Run(Str31 name, OSType* mode, short* difficulty);
OSErr NewMenu_Quit();

//File: Pilot menu.cp
OSErr PilotMenu_Display(Str63 name);

//File: Character menu.cp
OSErr CharacterMenu_Display(OSType* characterID);

//File: Network menu.cp
OSErr NetworkMenu_Display();

//File: Browser.cp
OSErr Browser_Display(OSType* gameMode, PilotPtr pilot, OSType* terrainID, OSType* shipID);
void Browser_Reset();

//File: Score.cp
OSErr ScoreMenu_Display();

//File: Champion.cp
OSErr ChampionMenu_Display(CharacterPreloadedDataPtr characterData, Boolean grandChampion);

//File: Credits.cp
OSErr Credits_Display();

//File: Pub.cp
OSErr PubWaterRace_Display();
OSErr PubFusion_Display();

//File: Demo.cp
OSErr Demo_Play();

#endif
