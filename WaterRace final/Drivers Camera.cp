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


#include				<fp.h>

#include				"WaterRace.h"
#include				"Strings.h"
#include				"Pilots.h"
#include				"Game.h"
#include				"Drivers.h"
#include				"Preferences.h"

#include				"Infinity Rendering.h"
#include				"Vector.h"
#include				"Infinity Post FX.h"

#include				"Camera Utils.h"
#include				"Clipping Utils.h"

#include				"Keys.h"

//CONSTANTES:

#define					kHeadCamOffset				0.0
#define					kCameraFollowHeight			2.0
#define					kFollowCamSpeedFactor		2.0
				
#define					kMaxZoom					1.0
#define					kMaxZoomDistance			140.0
#define					kMinCameraDistance			8.0
#define					kCameraHeadZoom				1.0
#define					kCameraFollowZoom			1.0

#define					kCameraTrackHeight			2.0

#define					kCameraText_H				10.0
#define					kCameraText_H2				150.0
#define					kCameraText_H3				10.0
#define					kCameraText_V				10.0

#define					kColor_Alert_r				1.0
#define					kColor_Alert_g				alertColor
#define					kColor_Alert_b				alertColor

#define					kColor_Yellow_r				1.0
#define					kColor_Yellow_g				1.0
#define					kColor_Yellow_b				0.0

#define					kColor_White_r				1.0
#define					kColor_White_g				1.0
#define					kColor_White_b				1.0

#define					kColor_InfoText_r			1.0
#define					kColor_InfoText_g			0.5
#define					kColor_InfoText_b			0.0

#define					kBonusItemSpriteSize		32.0
#define					kBonusScreenDepth			0.001
#define					kBonusScreenIw				1.0

#define					kFollow_MinDistance			-5.0
#define					kFollow_MaxDistance			-7.0
#define					kFollow_MaxTurboDistance	-8.0
#define					kFollow_StartDistance		-6.0
#define					kFollow_AccelerationFactor	0.8
#define					kFollow_LateralAttenuation	100.0
#define					kFollow_SpeedAttenuation	11.0
#define					kFollow_MaxLateralOffset	2.0

#define					kChaseMax					15.0
#define					kChaseMin					4.0
#define					kChaseHeight				3.5

//VARIABLES:

CameraDriver_Proc		cameraDriver = nil;
CameraTextDriver_Proc	cameraTextDriver = nil;
long					cameraMode;
ExtendedShipPtr			cameraTargetShip = nil;
long					onboardCameraNum = 0;

//VARIABLES LOCALES:

static MatrixPtr		onBoardCameraMatrix;
static Str63			cameraText[] = {kString_Camera_Free, kString_Camera_Follow, \
	kString_Camera_Head, kString_Camera_Onboard, kString_Camera_TV, kString_Camera_Chase, kString_Camera_From, \
	kString_Camera_Of, kString_Camera_On, kString_Camera_Replay, kString_Camera_Separator, kString_Camera_Track};
static unsigned char*	cameraTextPtr;
static unsigned char*	cameraNamePtr;
static float			alertColor = 0.0,
						direction = 2.0;

//ROUTINES:

static void ReplayInfoText(StatePtr state, ExtendedShipPtr ship, Boolean displaySpeed)
{
	Vector			position;
	Str31			text;
	
	//Compute alert color
	alertColor += direction * (state->frameTime - state->lastFrameTime) / kTimeUnit;
	if(alertColor > 1.0) {
		alertColor = 1.0;
		direction = -direction;
	}
	else if(alertColor < 0.0) {
		alertColor = 0.0;
		direction = -direction;
	}

	//Display REPLAY text
	position.x = thePrefs.renderWidth - kCameraText_H;
	position.y = kCameraText_V;
	PostFX_Text_Display(state, cameraText[9][0], &cameraText[9][1], &position, kTextMode_Right, kColor_Alert_r, kColor_Alert_g, kColor_Alert_b);
	
	if(!displaySpeed)
	return;
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
		
	//Display speed in units
	text[0] = 8;
	FastNumToString3(UnitToSpeed(ship->player->speed), &text[1]);
	text[4] = ' ';
#if __UNIT_SYSTEM__ == kUnit_Meters
	text[5] = 'k'; text[6] = 'm'; text[7] = '/'; text[8] = 'h';
#elif __UNIT_SYSTEM__ == kUnit_Miles
	text[5] = 'm'; text[6] = 'p'; text[7] = 'h'; --text[0];
#endif
	
	position.x = thePrefs.renderWidth - kCameraText_H3;
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Right, kColor_White_r, kColor_White_g, kColor_White_b);
}

static void PlayerInfoText(StatePtr state, ExtendedShipPtr ship)
{
	Str31			text;
	Vector			position;
	
	//Display speed in units
	text[0] = 8;
	FastNumToString3(UnitToSpeed(ship->player->speed), &text[1]);
	text[4] = ' ';
#if __UNIT_SYSTEM__ == kUnit_Meters
	text[5] = 'k'; text[6] = 'm'; text[7] = '/'; text[8] = 'h';
#elif __UNIT_SYSTEM__ == kUnit_Miles
	text[5] = 'm'; text[6] = 'p'; text[7] = 'h'; --text[0];
#endif	
	position.x = thePrefs.renderWidth - kCameraText_H3;
	position.y = kCameraText_V;
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Right, kColor_White_r, kColor_White_g, kColor_White_b);
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	if(gameConfig.gameMode != kGameMode_Practice) {
		//Display score
		if((ship->player->shipPilotType == kPilot_Local) && (gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_Tournament)) {
			text[0] = FastNumToString4(ship->player->score, &text[1]);
			text[++text[0]] = ' ';
#if 1 //__LANGUAGE__ == kLanguage_English
			text[++text[0]] = 'P';
			text[++text[0]] = 'T';
			text[++text[0]] = 'S';
#endif
			position.x = thePrefs.renderWidth / 2;
			PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
		}
		
		//Display lap count
#if __LANGUAGE__ == kLanguage_English
		text[0] = 7;
		text[1] = 'l'; text[2] = 'a'; text[3] = 'p'; text[4] = ' ';
		if(ship->player->lapCount <= theRace->lapNumber)
		FastNumToString1(ship->player->lapCount, &text[5]);
		else
		FastNumToString1(theRace->lapNumber, &text[5]);
		text[6] = '/';
		FastNumToString1(theRace->lapNumber, &text[7]);
#elif __LANGUAGE__ == kLanguage_French
		text[0] = 8;
		text[1] = 't'; text[2] = 'o'; text[3] = 'u'; text[4] = 'r'; text[5] = ' ';
		if(ship->player->lapCount <= theRace->lapNumber)
		FastNumToString1(ship->player->lapCount, &text[6]);
		else
		FastNumToString1(theRace->lapNumber, &text[6]);
		text[7] = '/';
		FastNumToString1(theRace->lapNumber, &text[8]);
#elif __LANGUAGE__ == kLanguage_German
		text[0] = 9;
		text[1] = 'r'; text[2] = 'u'; text[3] = 'n'; text[4] = 'd'; text[5] = 'e'; text[6] = ' ';
		if(ship->player->lapCount <= theRace->lapNumber)
		FastNumToString1(ship->player->lapCount, &text[7]);
		else
		FastNumToString1(theRace->lapNumber, &text[7]);
		text[8] = '/';
		FastNumToString1(theRace->lapNumber, &text[9]);
#elif __LANGUAGE__ == kLanguage_Italian
		text[0] = 8;
		text[1] = 'g'; text[2] = 'i'; text[3] = 'r'; text[4] = 'o'; text[5] = ' ';
		if(ship->player->lapCount <= theRace->lapNumber)
		FastNumToString1(ship->player->lapCount, &text[6]);
		else
		FastNumToString1(theRace->lapNumber, &text[6]);
		text[7] = '/';
		FastNumToString1(theRace->lapNumber, &text[8]);
#else
#error  __LANGUAGE__ undefined!
#endif
		position.x = thePrefs.renderWidth - kCameraText_H3;
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Right, kColor_White_r, kColor_White_g, kColor_White_b);
	}
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	//Display special items
	if(ship->player->shipPilotType != kPilot_Remote) {
		float			offset = 0.0;
		
		if(ship->player->hasJoker) {
			Texture_DrawSpriteOnScreen(state, kCameraText_H + 0.5 * kBonusItemSpriteSize, position.y + kBonusItemSpriteSize / 2.0, kBonusScreenDepth, kBonusScreenIw, kBonusItemSpriteSize, kBonusItemSpriteSize, jokerTexture);
			offset = 1.1 * kBonusItemSpriteSize;
		}
#if 0
		for(i = 0; i < ship->player->turboCount; ++i)
		Texture_DrawSpriteOnScreen(state, offset + kCameraText_H + 0.5 * kBonusItemSpriteSize + i * 1.1 * kBonusItemSpriteSize, position.y + kBonusItemSpriteSize / 2.0, kBonusScreenDepth, kBonusScreenIw, kBonusItemSpriteSize, kBonusItemSpriteSize, turboTexture);
#else	
#if __FLASHING_TURBO__
		if((!ship->turboState && ship->player->turboCount) || (ship->turboState && (worldState->frameTime % (kTimeUnit / 2) > kTimeUnit / 4))) {
#else
		if(ship->player->turboCount) {
#endif
			Texture_DrawSpriteOnScreen(state, offset + kCameraText_H + 0.5 * kBonusItemSpriteSize, position.y + kBonusItemSpriteSize / 2.0, kBonusScreenDepth, kBonusScreenIw, kBonusItemSpriteSize, kBonusItemSpriteSize, turboTexture);
#if __FLASHING_TURBO__
			if(ship->player->turboCount + ship->turboState > 1) {
#else
			if(ship->player->turboCount > 1) {
#endif
				float			save;
				
				text[1] = 'X';
#if __FLASHING_TURBO__
				text[0] = FastNumToString4(ship->player->turboCount + ship->turboState, &text[2]) + 1;
#else
				text[0] = FastNumToString4(ship->player->turboCount, &text[2]) + 1;
#endif
				position.x = offset + kCameraText_H + 1.1 * kBonusItemSpriteSize;
				save = position.y;
				position.y += kBonusItemSpriteSize / 4.0;
				PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
				position.y = save;
			}
		}
#endif
	}
	
	if((gameConfig.gameMode != kGameMode_Practice) && !((gameConfig.gameMode_save == kGameMode_Local) && (gameConfig.gameSubMode_save == kGameSubMode_Local_TimeRace))) {
		//Display rank
#if __LANGUAGE__ == kLanguage_English
		text[0] = 8;
		text[1] = 'r'; text[2] = 'a'; text[3] = 'n'; text[4] = 'k'; text[5] = ' ';
		FastNumToString1(ship->player->rank, &text[6]);
		text[7] = '/';
		FastNumToString1(shipCount, &text[8]);
#elif __LANGUAGE__ == kLanguage_French
		text[0] = 8;
		text[1] = 'r'; text[2] = 'a'; text[3] = 'n'; text[4] = 'g'; text[5] = ' ';
		FastNumToString1(ship->player->rank, &text[6]);
		text[7] = '/';
		FastNumToString1(shipCount, &text[8]);
#elif __LANGUAGE__ == kLanguage_German
		text[0] = 8;
		text[1] = 'r'; text[2] = 'a'; text[3] = 'n'; text[4] = 'g'; text[5] = ' ';
		FastNumToString1(ship->player->rank, &text[6]);
		text[7] = '/';
		FastNumToString1(shipCount, &text[8]);
#elif __LANGUAGE__ == kLanguage_Italian
		text[0] = 13;
		text[1] = 'p'; text[2] = 'o'; text[3] = 's'; text[4] = 'i'; text[5] = 'z'; text[6] = 'i'; text[7] = 'o'; text[8] = 'n'; text[9] = 'e'; text[10] = ' ';
		FastNumToString1(ship->player->rank, &text[11]);
		text[12] = '/';
		FastNumToString1(shipCount, &text[13]);
#else
#error  __LANGUAGE__ undefined!
#endif
		position.x = thePrefs.renderWidth - kCameraText_H3;
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Right, kColor_White_r, kColor_White_g, kColor_White_b);
	}
	
	//Compute alert color
	alertColor += direction * (state->frameTime - state->lastFrameTime) / kTimeUnit;
	if(alertColor > 1.0) {
		alertColor = 1.0;
		direction = -direction;
	}
	else if(alertColor < 0.0) {
		alertColor = 0.0;
		direction = -direction;
	}
	
	//Display ship info & reset
	if(ship->player->infoText[0]) {
		position.x = thePrefs.renderWidth / 2;
		position.y = thePrefs.renderHeight * 0.4;
		PostFX_Text_Display(state, cameraTargetShip->player->infoText[0], &cameraTargetShip->player->infoText[1], &position, kTextMode_Centered, kColor_InfoText_r, kColor_InfoText_g, kColor_InfoText_b);
		if(state->frameTime > ship->player->infoMaxTime)
		ship->player->infoText[0] = 0;
	}
	
	//Display Network text info
	if(ship->player->networkText[0]) {
		position.x = thePrefs.renderWidth / 2;
		position.y = thePrefs.renderHeight / 2;
		PostFX_Text_Display(state, ship->player->networkText[0], &ship->player->networkText[1], &position, kTextMode_Centered, kColor_Alert_r, kColor_Alert_g, kColor_Alert_b);
	}
	
#if __AUTOPILOT__
	//Auto-pilot text
	if(ship->player->finished) {
		BlockMove(kString_Driver_AutoPilot, text, sizeof(Str31));
		position.x = thePrefs.renderWidth / 2;
		position.y = thePrefs.renderHeight * 0.3;
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Centered, kColor_Alert_r, kColor_Alert_g, kColor_Alert_b);
		
		if((ship == localShip) && (gameConfig.gameMode == kGameMode_Local)) {
			BlockMove(kString_Driver_Finish, text, sizeof(Str31));
			position.x = thePrefs.renderWidth / 2;
			PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, kColor_Alert_r, kColor_Alert_g, kColor_Alert_b);
			PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Centered, kColor_Alert_r, kColor_Alert_g, kColor_Alert_b);
		}
	}
#endif
}

static void HUD_LowRes(StatePtr state)
{
	Str31				text;
	Vector				position;
	
	position.x = thePrefs.renderWidth / 4;
	position.y = kCameraText_V;
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace))
	RaceTimeToString(gameConfig.expiredTime - state->frameTime, text);
	else
	RaceTimeToString(state->frameTime, text);
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
}

static void HUD_Detailed(StatePtr state)
{
	Vector				position;
	Str31				text;
	
	position.x = kCameraText_H;
	position.y = kCameraText_V;
	PostFX_Text_Display(state, cameraTextPtr[0], &cameraTextPtr[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	position.x = kCameraText_H;
	PostFX_Text_Display(state, cameraTargetShip->player->playerName[0], &cameraTargetShip->player->playerName[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
#if __RENDER_SHIP_NAME__
	PostFX_Text_Display(state, cameraText[8][0], &cameraText[8][1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
	PostFX_Text_Display(state, cameraTargetShip->megaObject.object.name[0], &cameraTargetShip->megaObject.object.name[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
#endif
	
	if(gameConfig.gameMode != kGameMode_Replay)
	PlayerInfoText(state, cameraTargetShip);
	else
	ReplayInfoText(state, cameraTargetShip, true);
	
	position.x = thePrefs.renderWidth / 2;
	position.y = kCameraText_V;
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace))
	RaceTimeToString(gameConfig.expiredTime - state->frameTime, text);
	else
	RaceTimeToString(state->frameTime, text);
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
}

static void HUD_OnBoard(StatePtr state)
{
	Vector				position;
	Str31				text;
	
	position.x = kCameraText_H;
	position.y = kCameraText_V;
	PostFX_Text_Display(state, cameraTextPtr[0], &cameraTextPtr[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	position.x = kCameraText_H;
	PostFX_Text_Display(state, cameraTargetShip->player->playerName[0], &cameraTargetShip->player->playerName[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
#if __RENDER_CAMERA_NAMES__
	PostFX_Text_Display(state, cameraText[10][0], &cameraText[10][1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
	PostFX_Text_Display(state, cameraNamePtr[0], &cameraNamePtr[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
#endif
#if __RENDER_SHIP_NAME__
	PostFX_Text_Display(state, cameraText[7][0], &cameraText[7][1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
	PostFX_Text_Display(state, cameraTargetShip->megaObject.object.name[0], &cameraTargetShip->megaObject.object.name[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
#endif
	
	if(gameConfig.gameMode != kGameMode_Replay)
	PlayerInfoText(state, cameraTargetShip);
	else
	ReplayInfoText(state, cameraTargetShip, true);
	
	position.x = thePrefs.renderWidth / 2;
	position.y = kCameraText_V;
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace))
	RaceTimeToString(gameConfig.expiredTime - state->frameTime, text);
	else
	RaceTimeToString(state->frameTime, text);
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
}

static void HUD_WaterRaceTV(StatePtr state)
{
	Vector				position;
	Str31				text;
	
	position.x = kCameraText_H;
	position.y = kCameraText_V;
	PostFX_Text_Display(state, cameraTextPtr[0], &cameraTextPtr[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	position.x = kCameraText_H;
	//PostFX_Text_Display(state, cameraText[5][0], &cameraText[5][1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
	PostFX_Text_Display(state, cameraTargetShip->player->playerName[0], &cameraTargetShip->player->playerName[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
#if __RENDER_CAMERA_NAMES__
	PostFX_Text_Display(state, cameraText[6][0], &cameraText[6][1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
	PostFX_Text_Display(state, cameraNamePtr[0], &cameraNamePtr[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
#endif
	
	position.x = thePrefs.renderWidth / 2;
	position.y = kCameraText_V;
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace))
	RaceTimeToString(gameConfig.expiredTime - state->frameTime, text);
	else
	RaceTimeToString(state->frameTime, text);
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);

	if(gameConfig.gameMode == kGameMode_Replay)
	ReplayInfoText(state, cameraTargetShip, false);
}

static void HUD_Track(StatePtr state)
{
	Vector				position;
	Str31				text;
	
	position.x = kCameraText_H;
	position.y = kCameraText_V;
	PostFX_Text_Display(state, cameraTextPtr[0], &cameraTextPtr[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	position.x = kCameraText_H;
	PostFX_Text_Display(state, cameraTargetShip->player->playerName[0], &cameraTargetShip->player->playerName[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
	
	position.x = thePrefs.renderWidth / 2;
	position.y = kCameraText_V;
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace))
	RaceTimeToString(gameConfig.expiredTime - state->frameTime, text);
	else
	RaceTimeToString(state->frameTime, text);
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);

	if(gameConfig.gameMode == kGameMode_Replay)
	ReplayInfoText(state, cameraTargetShip, false);
}

static void HUD_Chase(StatePtr state)
{
	Vector				position;
	Str31				text;
	
	position.x = kCameraText_H;
	position.y = kCameraText_V;
	PostFX_Text_Display(state, cameraTextPtr[0], &cameraTextPtr[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);
	
	//New line
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	position.x = kCameraText_H;
	PostFX_Text_Display(state, cameraTargetShip->player->playerName[0], &cameraTargetShip->player->playerName[1], &position, kTextMode_Left, kColor_White_r, kColor_White_g, kColor_White_b);
	
	position.x = thePrefs.renderWidth / 2;
	position.y = kCameraText_V;
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace))
	RaceTimeToString(gameConfig.expiredTime - state->frameTime, text);
	else
	RaceTimeToString(state->frameTime, text);
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Yellow_r, kColor_Yellow_g, kColor_Yellow_b);

	if(gameConfig.gameMode == kGameMode_Replay)
	ReplayInfoText(state, cameraTargetShip, false);
}

/********************************************************************/
/*																	*/
/*					Camera is behind the player's ship				*/
/*																	*/
/********************************************************************/
			
void DriverCamera_Follow_Init()
{
	worldState->zoom = kCameraFollowZoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	cameraTextPtr = cameraText[1];
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	cameraTextDriver = HUD_LowRes;
	else
	cameraTextDriver = HUD_Detailed;
	
	cameraMode = kCameraMode_Follow;
	cameraDriver = (CameraDriver_Proc) DriverCamera_Follow_Callback;
}

void DriverCamera_Follow_Callback()
{
	Vector				p;
	
	//Compute camera position - Z
	p.z = kFollow_MinDistance - FVector_Length(cameraTargetShip->dynamics.velocityG) / kFollow_SpeedAttenuation;
	
	//Compute camera position - Y
	p.y = 0.0;
	
	//Compute camera position - X
	p.x = RadiansToDegrees(cameraTargetShip->dynamics.rotationspeedL.y) / kFollow_LateralAttenuation;
	if(p.x > kFollow_MaxLateralOffset)
	p.x = kFollow_MaxLateralOffset;
	else if(p.x < -kFollow_MaxLateralOffset)
	p.x = -kFollow_MaxLateralOffset;
	
	//Convert back to world-relative position
	Matrix_TransformVector(&((ObjectPtr) cameraTargetShip)->pos, &p, &worldCamera.camera.w);
	worldCamera.camera.w.y = kCameraFollowHeight;

	//Compute camera orientation matrix
	worldCamera.camera.z.x = ((ObjectPtr) cameraTargetShip)->pos.w.x - worldCamera.camera.w.x;
	worldCamera.camera.z.y = 0.0;
	worldCamera.camera.z.z = ((ObjectPtr) cameraTargetShip)->pos.w.z - worldCamera.camera.w.z;
	Vector_Normalize(&worldCamera.camera.z, &worldCamera.camera.z);
	worldCamera.camera.y.x = 0.0;
	worldCamera.camera.y.y = 1.0;
	worldCamera.camera.y.z = 0.0;
	Vector_CrossProduct(&worldCamera.camera.y, &worldCamera.camera.z, &worldCamera.camera.x);
}

/********************************************************************/
/*																	*/
/*					Camera is the pilot's head						*/
/*																	*/
/********************************************************************/

void DriverCamera_PilotHead_Init()
{
#if __VR_MODE__
	worldState->zoom = 1.3;
#else
	worldState->zoom = kCameraHeadZoom;
#endif
	InfinityRendering_UpdateStateParams(worldState);
	
	cameraTextPtr = cameraText[2];
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	cameraTextDriver = HUD_LowRes;
	else
	cameraTextDriver = HUD_Detailed;
	
	cameraMode = kCameraMode_Head;
	cameraDriver = (CameraDriver_Proc) DriverCamera_PilotHead_Callback;
}

#if __VR_MODE__
extern float	headRoll,
				headPitch,
				headYaw;

//XYZ ZXY

void Matrix_BuildRotationVR(float rX, float rY, float rZ, MatrixPtr matrix)
{
	Matrix				m;
	
	Matrix_SetRotateY(rY, matrix);
	Matrix_SetRotateX(rX, &m);
	Matrix_MultiplyByMatrix(&m, matrix, matrix);
	Matrix_SetRotateZ(rZ, &m);
	Matrix_MultiplyByMatrix(&m, matrix, matrix);
	
	matrix->w.x = matrix->w.y = matrix->w.z = 0.0;
}
#endif

void DriverCamera_PilotHead_Callback()
{
	Matrix			cameraMatrix;
	
#if __VR_MODE__
	Matrix_BuildRotationVR(headRoll, headPitch, headYaw, &cameraMatrix);
	cameraMatrix.w = cameraTargetShip->headShape->pos.w;
	cameraMatrix.w.z = cameraTargetShip->headShape->pos.w.z + kHeadCamOffset;
#else
	Matrix_Clear(&cameraMatrix);
	cameraMatrix.w = cameraTargetShip->headShape->pos.w;
	cameraMatrix.w.z = cameraTargetShip->headShape->pos.w.z + kHeadCamOffset;
#endif
	Matrix_Cat(&cameraMatrix, &((ObjectPtr) cameraTargetShip)->pos, &worldCamera.camera);
}
		
/********************************************************************/
/*																	*/
/*					Camera is one of the on-board cam				*/
/*																	*/
/********************************************************************/

void DriverCamera_OnBoard_Init()
{
	if(cameraMode == kCameraMode_OnBoard)
	++onboardCameraNum;
	onboardCameraNum %= cameraTargetShip->megaObject.cameraCount;
		
	onBoardCameraMatrix = &cameraTargetShip->megaObject.cameraList[onboardCameraNum]->pos;
	
	worldState->zoom = cameraTargetShip->megaObject.cameraList[onboardCameraNum]->zoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	cameraTextPtr = cameraText[3];
	cameraNamePtr = cameraTargetShip->megaObject.cameraList[onboardCameraNum]->name;
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	cameraTextDriver = HUD_LowRes;
	else
	cameraTextDriver = HUD_OnBoard;
	
	cameraMode = kCameraMode_OnBoard;
	cameraDriver = (CameraDriver_Proc) DriverCamera_OnBoard_Callback;
}

void DriverCamera_OnBoard_Callback()
{
	Matrix_Cat(onBoardCameraMatrix, &((ObjectPtr) cameraTargetShip)->pos, &worldCamera.camera);
}
		
/********************************************************************/
/*																	*/
/*					Camera is one of the terrain cam				*/
/*																	*/
/********************************************************************/

void DriverCamera_TV_Init()
{
	worldState->zoom = kCameraZoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	cameraTextPtr = cameraText[4];
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	cameraTextDriver = HUD_LowRes;
	else
	cameraTextDriver = HUD_WaterRaceTV;
	
	cameraMode = kCameraMode_TV;
	cameraDriver = (CameraDriver_Proc) DriverCamera_TV_Callback;
}

static Boolean ShipVisible_FromCamera(ExtendedShipPtr ship, MatrixPtr cameraMatrix, float newZoom)
{
	Matrix			m;
	long			visible;
	Vector			center;
	
	Matrix_Negate(cameraMatrix, &m);
	Matrix_TransformVector(&ship->megaObject.object.pos, &ship->megaObject.object.sphereCenter, &center);
	
	worldState->zoom = newZoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	visible = Sphere_Visible(&m, &center, ship->megaObject.object.sphereRadius * ship->megaObject.object.scale, worldState);
	if(visible == kNotVisible)
	return false;
	
	return true;
}

#if __TV_TARGET_FIRST__
static ExtendedShipPtr FindFirstShip()
{
	ExtendedShipPtr		ship = &shipList[0];
	long				i;
	float				num,
						best = 0.0;
	
	//Compute magic values - the greater the better
	for(i = 0; i < shipCount; ++i) {
		num = (shipList[i].player->lapCount * theRace->doorCount + shipList[i].lastCheckPoint + 1) * kMaxRaceTime - shipList[i].checkPointTime;
		if(num > best) {
			ship = &shipList[i];
			best = num;
		}
	}
	
	return ship;
}
#endif

void DriverCamera_TV_Callback()
{
	long			i,
					cameraNum;
	float			distance,
					minDistance = kHugeDistance * kHugeDistance;
	Vector			cameraPos,
					pos;
	
#if __TV_TARGET_FIRST__
	//Find target
	cameraTargetShip = FindFirstShip();
#endif
	
	//Find closest camera
	pos = ((ObjectPtr) cameraTargetShip)->pos.w;
	for(i = 0; i < theTerrain.cameraCount; ++i) {
		cameraPos = theTerrain.cameraList[i]->pos.w;
		distance = (cameraPos.x - pos.x) * (cameraPos.x - pos.x)/* + (cameraPos.y - pos.y) * (cameraPos.y - pos.y)*/ + (cameraPos.z - pos.z) * (cameraPos.z - pos.z);
		if(distance < minDistance) {
			//This camera is static - make sure the target ship is visible
			if((theTerrain.cameraList[i]->flags != 0) || ShipVisible_FromCamera(cameraTargetShip, &theTerrain.cameraList[i]->pos, theTerrain.cameraList[i]->zoom)) {
				cameraNum = i;
				minDistance = distance;
			}
		}
	}
	minDistance = FastSqrt(minDistance);
	
	if(theTerrain.cameraList[cameraNum]->flags & kFlag_AutoZoom) {
		worldState->zoom = 0.1 + kMaxZoom - minDistance * kMaxZoom / kMaxZoomDistance;
		if(worldState->zoom < 0.1)
		worldState->zoom = 0.1;
	}
	else
	worldState->zoom = theTerrain.cameraList[cameraNum]->zoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	if(theTerrain.cameraList[cameraNum]->flags & kFlag_Follow_H) {
		cameraPos = theTerrain.cameraList[cameraNum]->pos.w;
	
		worldCamera.camera.z.x = pos.x - cameraPos.x;
		worldCamera.camera.z.y = 0.0;
		worldCamera.camera.z.z = pos.z - cameraPos.z;
		Vector_Normalize(&worldCamera.camera.z, &worldCamera.camera.z);
		worldCamera.camera.y.x = 0.0;
		worldCamera.camera.y.y = 1.0;
		worldCamera.camera.y.z = 0.0;
		Vector_CrossProduct(&worldCamera.camera.y, &worldCamera.camera.z, &worldCamera.camera.x);
		worldCamera.camera.w = cameraPos;
		
		if((theTerrain.cameraList[cameraNum]->flags & kFlag_PreserveDistance) && (minDistance < kMinCameraDistance)) {
			worldCamera.camera.w.x -= worldCamera.camera.z.x * (kMinCameraDistance - minDistance);
			worldCamera.camera.w.y -= worldCamera.camera.z.y * (kMinCameraDistance - minDistance);
			worldCamera.camera.w.z -= worldCamera.camera.z.z * (kMinCameraDistance - minDistance);
		}
	}
	else if(theTerrain.cameraList[cameraNum]->flags & kFlag_Follow_HV) {
		cameraPos = theTerrain.cameraList[cameraNum]->pos.w;
		
		worldCamera.camera.z.x = pos.x - cameraPos.x;
		worldCamera.camera.z.y = pos.y - cameraPos.y;
		worldCamera.camera.z.z = pos.z - cameraPos.z;
		Vector_Normalize(&worldCamera.camera.z, &worldCamera.camera.z);
		
		worldCamera.camera.x.x = worldCamera.camera.z.z;
		worldCamera.camera.x.y = 0.0;
		worldCamera.camera.x.z = -worldCamera.camera.z.x;
		Vector_Normalize(&worldCamera.camera.x, &worldCamera.camera.x);
		
		Vector_CrossProduct(&worldCamera.camera.z, &worldCamera.camera.x, &worldCamera.camera.y);
		worldCamera.camera.w = cameraPos;
		
		if((theTerrain.cameraList[cameraNum]->flags & kFlag_PreserveDistance) && (minDistance < kMinCameraDistance)) {
			worldCamera.camera.w.x -= worldCamera.camera.z.x * (kMinCameraDistance - minDistance);
			worldCamera.camera.w.y -= worldCamera.camera.z.y * (kMinCameraDistance - minDistance);
			worldCamera.camera.w.z -= worldCamera.camera.z.z * (kMinCameraDistance - minDistance);
		}
	}
	else
	worldCamera.camera = theTerrain.cameraList[cameraNum]->pos;
	
	cameraNamePtr = theTerrain.cameraList[cameraNum]->name;
}

/********************************************************************/
/*																	*/
/*					Camera is on the track							*/
/*																	*/
/********************************************************************/

void DriverCamera_Track_Init()
{
	worldState->zoom = kCameraZoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	cameraTextPtr = cameraText[11];
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	cameraTextDriver = HUD_LowRes;
	else
	cameraTextDriver = HUD_Detailed; //HUD_Track;
	
	cameraMode = kCameraMode_Track;
	cameraDriver = (CameraDriver_Proc) DriverCamera_Track_Callback;
}

void DriverCamera_Track_Callback()
{
	Vector			pos,
					cameraPos,
					cameraPos1,
					cameraPos2;
	float			distance1,
					distance2,
					distance;
	
	//Pre-store ship position
	pos = ((ObjectPtr) cameraTargetShip)->pos.w;
	
	//Compute camera position #1
	cameraPos1.x = theRace->doorList[cameraTargetShip->lastCheckPoint].middle.x;
	cameraPos1.y = kCameraTrackHeight;
	cameraPos1.z = theRace->doorList[cameraTargetShip->lastCheckPoint].middle.z;
	distance1 = FVector_Distance(pos, cameraPos1);
	
	//Compute camera position #2
	cameraPos2.x = theRace->doorList[cameraTargetShip->nextCheckPoint].middle.x;
	cameraPos2.y = kCameraTrackHeight;
	cameraPos2.z = theRace->doorList[cameraTargetShip->nextCheckPoint].middle.z;
	distance2 = FVector_Distance(pos, cameraPos2);
	
	//Compare distance
	if(distance2 < distance1) {
		cameraPos = cameraPos2;
		distance = distance2;
	}
	else {
		cameraPos = cameraPos1;
		distance = distance1;
	}
	
	//Compute zoom
	worldState->zoom = 0.1 + kMaxZoom - distance * kMaxZoom / kMaxZoomDistance;
	if(worldState->zoom < 0.1)
	worldState->zoom = 0.1;
	InfinityRendering_UpdateStateParams(worldState);
	
	//Compute camera orientation
	worldCamera.camera.z.x = pos.x - cameraPos.x;
	worldCamera.camera.z.y = pos.y - cameraPos.y;
	worldCamera.camera.z.z = pos.z - cameraPos.z;
	Vector_Normalize(&worldCamera.camera.z, &worldCamera.camera.z);
	
	worldCamera.camera.x.x = worldCamera.camera.z.z;
	worldCamera.camera.x.y = 0.0;
	worldCamera.camera.x.z = -worldCamera.camera.z.x;
	Vector_Normalize(&worldCamera.camera.x, &worldCamera.camera.x);
	
	Vector_CrossProduct(&worldCamera.camera.z, &worldCamera.camera.x, &worldCamera.camera.y);
	
	worldCamera.camera.w = cameraPos;
}

/********************************************************************/
/*																	*/
/*				Camera is controlled by fly-through animation		*/
/*																	*/
/********************************************************************/

void DriverCamera_FlyThrough_Init()
{
	worldState->zoom = kCameraZoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	cameraTextDriver = nil;
	
	cameraMode = kCameraMode_FlyThrough;
	cameraDriver = (CameraDriver_Proc) DriverCamera_FlyThrough_Callback;
}

void DriverCamera_FlyThrough_Callback()
{
	worldCamera.camera = flyThroughCameraAnimation->pos;
}

/********************************************************************/
/*																	*/
/*					Camera is chasing the ship						*/
/*																	*/
/********************************************************************/

void DriverCamera_Chase_Init()
{
	worldState->zoom = kCameraZoom;
	InfinityRendering_UpdateStateParams(worldState);
	
	cameraTextPtr = cameraText[5];
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	cameraTextDriver = HUD_LowRes;
	else
	cameraTextDriver = HUD_Chase;
	
	cameraMode = kCameraMode_Chase;
	cameraDriver = (CameraDriver_Proc) DriverCamera_Chase_Callback;
}

void DriverCamera_Chase_Callback()
{
	float		distance;
	Vector		d;
	
	distance = FVector_Distance(worldCamera.camera.w, cameraTargetShip->megaObject.object.pos.w);
	if(distance > kChaseMax) {
		Vector_Subtract(&cameraTargetShip->megaObject.object.pos.w, &worldCamera.camera.w, &d);
		Vector_Normalize(&d, &d);
		Vector_MultiplyAdd(-kChaseMax, &d, &cameraTargetShip->megaObject.object.pos.w, &worldCamera.camera.w);
	}
	else if(distance < kChaseMin) {
		Vector_Subtract(&cameraTargetShip->megaObject.object.pos.w, &worldCamera.camera.w, &d);
		Vector_Normalize(&d, &d);
		Vector_MultiplyAdd(-kChaseMin, &d, &cameraTargetShip->megaObject.object.pos.w, &worldCamera.camera.w);
	}
	worldCamera.camera.w.y = kChaseHeight;
	
	Vector_Subtract(&cameraTargetShip->megaObject.object.pos.w, &worldCamera.camera.w, &worldCamera.camera.z);
	Vector_Normalize(&worldCamera.camera.z, &worldCamera.camera.z);
	
	worldCamera.camera.x.x = 1.0;
	worldCamera.camera.x.y = 0.0;
	worldCamera.camera.x.z = -worldCamera.camera.z.x / worldCamera.camera.z.z;
	if(worldCamera.camera.z.z * worldCamera.camera.x.x - worldCamera.camera.z.x * worldCamera.camera.x.z < 0.0) {
		worldCamera.camera.x.x = -1.0;
		worldCamera.camera.x.z = worldCamera.camera.z.x / worldCamera.camera.z.z;
	}
	Vector_Normalize(&worldCamera.camera.x, &worldCamera.camera.x);
	
	Vector_CrossProduct(&worldCamera.camera.z, &worldCamera.camera.x, &worldCamera.camera.y);
}

/********************************************************************/
/*																	*/
/*				Camera is controlled by fly-through animation		*/
/*																	*/
/********************************************************************/

void DriverCamera_Fake_Init()
{
	cameraTextDriver = nil;
	
	cameraMode = kCameraMode_Fake;
	cameraDriver = (CameraDriver_Proc) DriverCamera_Fake_Callback;
}

void DriverCamera_Fake_Callback()
{
	;
}
