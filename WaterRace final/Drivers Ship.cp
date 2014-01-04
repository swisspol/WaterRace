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
#include				"ISpRoutines.h"
#include				"Preferences.h"	
#include				"Drivers.h"

#include				"Matrix.h"
#include				"Vector.h"
#include				"Quaternion.h"

#include				"Keys.h"

//CONSTANTES:

#define					kDoorTolerance				4.0

#define					kMinCollisionSpeed			0.25

#define					kTurboActivationDistance	50.0

//ROUTINES:

#if __DISPLAY_CHECK_POINT__
inline void DisplayCheckPointText(PlayerConfigurationPtr player)
{
	BlockMove(kString_Driver_CheckPoint, player->infoText, kString_Driver_CheckPoint_L);
	player->infoMaxTime = worldState->frameTime + kTimeUnit;
}
#endif

#if __DISPLAY_MISSED_CHECK_POINT__
inline void DisplayMissedCheckPointText(PlayerConfigurationPtr player)
{
	BlockMove(kString_Driver_MissedCheckPoint, player->infoText, kString_Driver_MissedCheckPoint_L);
	player->infoMaxTime = worldState->frameTime + kTimeUnit;
}
#endif

#if __DISPLAY_WRONG_DIRECTION__
inline void DisplayWrongDirectionCheckPointText(PlayerConfigurationPtr player)
{
	BlockMove(kString_Driver_WrongDirection, player->infoText, kString_Driver_WrongDirection_L);
	player->infoMaxTime = worldState->frameTime + kTimeUnit;
}
#endif

inline void DisplayLapText(PlayerConfigurationPtr player, unsigned long time, Boolean best)
{
	Str31		text;
	
	if(best)
	BlockMove(kString_Driver_BestLapTime, player->infoText, kString_Driver_BestLapTime_L);
	else
	BlockMove(kString_Driver_LapTime, player->infoText, kString_Driver_LapTime_L);
	RaceTimeToString(time, text);
	BlockMove(&text[1], &player->infoText[player->infoText[0] + 1], text[0]);
	player->infoText[0] += text[0];
	
	if(best) {
		player->infoText[0] += 1;
		player->infoText[player->infoText[0]] = '!';
	}
	
	player->infoMaxTime = worldState->frameTime + kInfoTextDuration;
}

inline void DisplayTimeExtendedLapText(PlayerConfigurationPtr player)
{
	Str31		text;
	
	BlockMove(kString_Driver_TimeExtended, player->infoText, kString_Driver_TimeExtended_L);
	RaceTimeToString(gameConfig.globalBestLapTime, text);
	BlockMove(&text[1], &player->infoText[player->infoText[0] + 1], text[0]);
	player->infoText[0] += text[0];
	player->infoText[0] += 1;
	player->infoText[player->infoText[0]] = '!';
	
	player->infoMaxTime = worldState->frameTime + kInfoTextDuration;
}

/********************************************************************/
/*																	*/
/*					Ship is controlled by player - ISp				*/
/*																	*/
/********************************************************************/

void DriverShip_PlayerISp_Init(ExtendedShipPtr ship)
{
#if __BOT_ONLY__
	ship->player->turboCount += 50;
#endif
}

void DriverShip_PlayerISp_Callback(ExtendedShipPtr ship)
{
	Vector				temp;
	Vector				pos,
						lastPos;
	RaceDoorPtr			door = &theRace->doorList[ship->nextCheckPoint];
	float				a,
						b,
						x;
	
	//Gather player input
	InputSprocket_GetShipInput(&ship->mouseH, &ship->mouseV, &ship->turboButton);
	
#if 0 //Joystick
	if(ship->mouseV >= 0.0)
	ship->mouseV = Square(ship->mouseV);
	else
	ship->mouseV = -Square(ship->mouseV);
#endif
	
	//Are we close to the checkpoint?
	temp.x = door->middle.x - ship->megaObject.object.pos.w.x;
	temp.z = door->middle.z - ship->megaObject.object.pos.w.z;
	if((temp.x * temp.x + temp.z * temp.z) > door->squareDoorWidth) {
#if __DISPLAY_MISSED_CHECK_POINT__
		//Check if we missed the checkpoint
		if(ship->inCheckPoint) {
			DisplayMissedCheckPointText(ship->player);
			
			//Play sound
			if(ship == cameraTargetShip)
			Sound_PlaySFxHandle(gameFXSounds[kGameFX_Error]);
			
			ship->inCheckPoint = false;
		}
#endif
		return;
	}
	
#if __DISPLAY_MISSED_CHECK_POINT__
	ship->inCheckPoint = true;
#endif
	
	//Transform ship into door space
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->megaObject.object.pos.w, &pos);
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->lastPos, &lastPos);
	
	//Check if door was passed
	if(lastPos.z * pos.z > 0.0)
	return;
	a = (pos.z - lastPos.z) / (pos.x - lastPos.x);
	b = pos.z - a * pos.x;
	x = -b / a;
	if((x < -kDoorTolerance) || (x > door->doorWidth + kDoorTolerance))
	return;
	
	//Check if door was passed in the correct direction
	if(pos.z > 0.0) {
		ship->lastCheckPoint = ship->nextCheckPoint;
		ship->nextCheckPoint = theRace->doorList[ship->nextCheckPoint].nextCheckPoint;
		ship->checkPointTime = worldState->frameTime;
		
		//Check if we have a new lap
		if(ship->lastCheckPoint == 0) {
			++ship->player->lapCount;
			if(ship->player->lapCount > theRace->lapNumber + 1) //In case of network game
			ship->player->lapCount = theRace->lapNumber + 1;
			ship->player->lastLapTime = worldState->frameTime - ship->player->lastLapTime;
			if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace)) {
				DisplayTimeExtendedLapText(ship->player);
				if(ship->player->lastLapTime < ship->player->bestLapTime)
				ship->player->bestLapTime = ship->player->lastLapTime;
				if(ship->player->lapCount <= theRace->lapNumber) {
					gameConfig.expiredTime += gameConfig.globalBestLapTime;
					if(ship == cameraTargetShip)
					Sound_PlaySFxHandle(gameFXSounds[KGameFX_BestLap]); //FIXME: Time extended sound
				}
			}
			else {
				if(ship->player->lastLapTime < gameConfig.globalBestLapTime) {
					DisplayLapText(ship->player, ship->player->lastLapTime, true);
					gameConfig.globalBestLapTime = ship->player->lastLapTime;
					ship->player->bestLapTime = ship->player->lastLapTime;
					if(ship == cameraTargetShip)
					Sound_PlaySFxHandle(gameFXSounds[KGameFX_BestLap]);
				}
				else {
					if(ship->player->lastLapTime < ship->player->bestLapTime)
					ship->player->bestLapTime = ship->player->lastLapTime;
					DisplayLapText(ship->player, ship->player->lastLapTime, false);
					if((ship == cameraTargetShip) && (ship->player->lapCount <= theRace->lapNumber))
					Sound_PlaySFxHandle(gameFXSounds[KGameFX_NewLap]);
				}
			}
			ship->player->lastLapTime = worldState->frameTime;
		}
#if __DISPLAY_CHECK_POINT__
		else {
			DisplayCheckPointText(ship->player);
			if(ship == cameraTargetShip)
			Sound_PlaySFxHandle(gameFXSounds[kGameFX_CheckPoint]);
		}
#endif
		//Update rank
		if((gameConfig.gameMode != kGameMode_Practice) && !((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace)) && (worldState->frameTime >= gameConfig.startTime))
		Game_UpdateRank();
	}
#if __DISPLAY_WRONG_DIRECTION__
	else {
		DisplayWrongDirectionCheckPointText(ship->player);
		
		//Play sound
		if(ship == cameraTargetShip)
		Sound_PlaySFxHandle(gameFXSounds[kGameFX_Error]);
	}
#endif
	
#if __DISPLAY_MISSED_CHECK_POINT__
	ship->inCheckPoint = false;
#endif
}

/********************************************************************/
/*																	*/
/*					Ship is controlled by computer					*/
/*																	*/
/********************************************************************/

#if !__BOTS_CAN_PICKUP_TURBOS__
static long CountTurbosOnTerrain(TerrainPtr terrain)
{
	long				count = 0;
	long				i;
	
	if(terrain->itemReferenceCount == 0)
	return 0;
	
	for(i = 0; i < terrain->itemReferenceCount; ++i)
	if(((SpecialItemPtr) terrain->itemReferenceList[i].itemID)->itemType == kItem_Turbo)
	++count;
	
	return count;
}
#endif

void DriverShip_Bot_Init(ExtendedShipPtr ship)
{
#if __BOT_ONLY__
	ship->player->turboCount += 50;
#elif !__BOTS_CAN_PICKUP_TURBOS__
#if __BOTS_ADJUST_TURBOS__
	if(gameConfig.terrainDifficulty == kDifficulty_Medium)
	ship->player->turboCount += CountTurbosOnTerrain(&theTerrain) / 2;
	else if(gameConfig.terrainDifficulty == kDifficulty_Hard)
	ship->player->turboCount += CountTurbosOnTerrain(&theTerrain);
#else
	ship->player->turboCount += CountTurbosOnTerrain(&theTerrain);
#endif
#endif
}

inline void Bot_ComputeTargetOnDoor(long doorNum, float coeff, float maxTurn, VectorPtr target)
{
	RaceDoorPtr		current;
	
	//Get current door
	current = &theRace->doorList[doorNum];
	
	//Compute target
	if(current->doorAngle < maxTurn) {
		//Compute turn direction
		if(current->turningRight)
		coeff = 1.0 - coeff;
		
		//Compute target
		target->y = current->middle.y;
		target->x = coeff * (current->rightMark.x - current->leftMark.x) + current->leftMark.x;
		target->z = coeff * (current->rightMark.z - current->leftMark.z) + current->leftMark.z;
	}
	else
	*target = current->middle;
}

static long Bot_GetTarget(long nextDoor, float coeff, float maxTurn, float anticipationDistance, VectorPtr pos, VectorPtr target)
{
	Vector			temp,
					lastTarget;
	float			distance,
					distance2;
	
	/********* CHECK IF NEXT DOOR COULD BE TARGET ********/
	
	//Calculate distance to target on next door
	Bot_ComputeTargetOnDoor(nextDoor, coeff, maxTurn, target);
	
	//Do we reach the anticipation distance?
	temp.x = target->x - pos->x;
	temp.y = 0.0;
	temp.z = target->z - pos->z;
	distance = FVector_Length(temp);
	if(distance > anticipationDistance)
	return nextDoor;
	
	/********* COMPUTE INTERPOLATED TARGET **********/
	
	while(1) {
		//Save...
		lastTarget = *target;
		
		//Find next door
		++nextDoor;
		if(nextDoor >= theRace->doorCount)
		nextDoor = 0;
		
		//Calculate target on next door
		Bot_ComputeTargetOnDoor(nextDoor, coeff, maxTurn, target);
		
		//Do we reach the anticipation distance?
		temp.x = target->x - lastTarget.x;
		temp.y = 0.0;
		temp.z = target->z - lastTarget.z;
		distance2 = FVector_Length(temp);
		if(distance + distance2 < anticipationDistance) {
			distance += distance2;
			continue;
		}
		
		//Interpolate target
		target->x = lastTarget.x + (anticipationDistance - distance) * temp.x / distance2;
		target->z = lastTarget.z + (anticipationDistance - distance) * temp.z / distance2;
		
		//Done
		return nextDoor;
	}
}

#if __BOTS_IMPROVED_TURBOS__
inline long GetSecondShipNextCheckPoint()
{
	long			i;
	
	for(i = 0; i < shipCount; ++i)
	if(shipList[i].player->rank == 2)
	return shipList[i].nextCheckPoint;
	
	return -1;
}

inline Boolean Bot_Turbo(ExtendedShipPtr ship, long num)
{
	float			totalDistance,
					totalAngle;
	Vector			dir;
	long			i;
	
	//Do we have any turbo left or are we already using the turbo?
	if(!ship->player->turboCount || ship->turboState)
	return false;
	
	//Can we use turbo in air?
	if((ship->dynamics.status == kStatus_InAir) && !(ship->params->bot_flags & kBotFlag_UseTurboInAir))
	return false;
	
	//Check according to other ship positions
#if !__BOT_ONLY__
	if(shipCount > 1) {
#if __BOTS_SMART_TURBOS__
		//If this is last lap or race has not started, we don't care
		if(ship->player->lapCount && (ship->player->lapCount < theRace->lapNumber)) {
			//Check if local ship is just after this bot
			if((localShip->player->rank > ship->player->rank) && (FVector_Distance(ship->megaObject.object.pos.w, localShip->megaObject.object.pos.w) > kTurboActivationDistance))
			return false;
		}
#elif 0
		//If this is last lap or race has not started, we don't care
		if(ship->player->lapCount && (ship->player->lapCount < theRace->lapNumber)) {
			//Check if this bot is first and if the second is the localship far away or not
			if(ship->player->rank == 1) {
				if((localShip->player->rank > 2) && ((localShip->player->lapCount != ship->player->lapCount) || (localShip->nextCheckPoint != ship->nextCheckPoint)))
				return false;
			}
			//Check if localShip is first
			else if(localShip->player->rank != 1) {
				//Check if local player is behind us and getting close
				if((localShip->player->rank > ship->player->rank) && ((localShip->player->lapCount != ship->player->lapCount) || (localShip->nextCheckPoint != ship->nextCheckPoint)))
				return false;
			}
		}
#else
		//Check if we're first and if the second is far away or not - if this is last lap, we don't care
		if((ship->player->lapCount < theRace->lapNumber) && (ship->player->rank == 1) && (GetSecondShipNextCheckPoint() != ship->nextCheckPoint))
		return false;
#endif

		//Check for collision risks with each ship
		if(ship->params->bot_flags & kBotFlag_WatchForTurboCollision)
		for(i = 0; i < shipCount; ++i) {
			if(ship == &shipList[i])
			continue;
			
			if(FVector_Distance(ship->megaObject.object.pos.w, shipList[i].megaObject.object.pos.w) < ship->params->bot_Turbo_CollisionDistance) {
				Vector				temp;
				
				//Check if the ship is in front of us
				Vector_Subtract(&shipList[i].megaObject.object.pos.w, &ship->megaObject.object.pos.w, &temp);
				if(Vector_DotProduct(&temp, &ship->megaObject.object.pos.z) > ship->params->bot_Turbo_CollisionAngle)
				return false;
			}
		}
	}
#endif
	
	//Compute angle to next door
	dir.x = theRace->doorList[num].middle.x - ship->megaObject.object.pos.w.x;
	dir.y = 0.0;
	dir.z = theRace->doorList[num].middle.z - ship->megaObject.object.pos.w.z;
	totalAngle = FVector_Length(dir);
	dir.x /= totalAngle;
	dir.z /= totalAngle;
	totalAngle = dir.x * ship->megaObject.object.pos.z.x + dir.z * ship->megaObject.object.pos.z.z;
	
	//Are we in the correct direction?
	if(totalAngle < ship->params->bot_Turbo_TargetToleranceAngle)
	return false;
	
	//Compute distance to next door
	totalDistance = FVector_Distance(ship->megaObject.object.pos.w, theRace->doorList[num].middle);
	
	//Check if we have a straight line
	totalAngle = 1.0 - totalAngle;
	while(totalDistance < ship->params->bot_Turbo_MinDistance) {
		++num;
		if(num >= theRace->doorCount)
		num = 0;
		
		totalAngle += 1.0 - fAbs(theRace->doorList[num].doorAngle);
		totalDistance += theRace->doorList[num].distanceToNextDoor;
	}
	if(1.0 - totalAngle < ship->params->bot_Turbo_MaxAngle)
	return false;
	
	return true;
}
#endif

static void Bot_CheckPoint(ExtendedShipPtr ship)
{
	Vector				temp;
	Vector				pos,
						lastPos;
	RaceDoorPtr			door = &theRace->doorList[ship->nextCheckPoint];
	float				a,
						b,
						x;
						
	//Are we close to the checkpoint?
	temp.x = door->middle.x - ship->megaObject.object.pos.w.x;
	temp.z = door->middle.z - ship->megaObject.object.pos.w.z;
	if((temp.x * temp.x + temp.z * temp.z) > door->squareDoorWidth) {
#if __BOTS_DETECT_MISSED_CHECK_POINTS__
#if 0
		if(ship->inCheckPoint) {
#else
		if(ship->inCheckPoint && ((temp.x * temp.x + temp.z * temp.z) > door->squareDoorWidth * 2.0)) {
#endif
			Physic_DropShip(ship, ship->lastCheckPoint, kString_Driver_MissedCheckPoint);
		
			ship->inCheckPoint = false;
		}
#endif
		return;
	}
	
	ship->inCheckPoint = true;
	
	//Transform ship into door space
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->megaObject.object.pos.w, &pos);
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->lastPos, &lastPos);
	
	//Check if door was passed
	if(lastPos.z * pos.z > 0.0)
	return;
	a = (pos.z - lastPos.z) / (pos.x - lastPos.x);
	b = pos.z - a * pos.x;
	x = -b / a;
	if((x < -kDoorTolerance) || (x > door->doorWidth + kDoorTolerance)) {
#if __BOTS_DETECT_MISSED_CHECK_POINTS__
		if((x > -door->doorWidth) && (x < door->doorWidth * 2.0) && (pos.z > door->doorWidth)) //we attempted to pass a checkpoint in the correct direction
		Physic_DropShip(ship, ship->lastCheckPoint, kString_Driver_MissedCheckPoint);
#endif
		return;
	}
	
	//Check if door was passed in the correct direction
	if(pos.z > 0.0) {
		ship->lastCheckPoint = ship->nextCheckPoint; //ship->nextDoor = (ship->nextCheckPoint + 1) % theRace->doorCount;
		ship->nextCheckPoint = theRace->doorList[ship->nextCheckPoint].nextCheckPoint;
		ship->checkPointTime = worldState->frameTime;
		
		//Check if we have a new lap
		if(ship->lastCheckPoint == 0) {
			++ship->player->lapCount;
			if(ship->player->lapCount > theRace->lapNumber + 1) //In case of network game
			ship->player->lapCount = theRace->lapNumber + 1;
			ship->player->lastLapTime = worldState->frameTime - ship->player->lastLapTime;
			if(ship->player->lastLapTime < gameConfig.globalBestLapTime) {
				DisplayLapText(ship->player, ship->player->lastLapTime, true);
				gameConfig.globalBestLapTime = ship->player->lastLapTime;
				ship->player->bestLapTime = ship->player->lastLapTime;
				if(ship == cameraTargetShip)
				Sound_PlaySFxHandle(gameFXSounds[KGameFX_BestLap]);
			}
			else {
				if(ship->player->lastLapTime < ship->player->bestLapTime)
				ship->player->bestLapTime = ship->player->lastLapTime;
				DisplayLapText(ship->player, ship->player->lastLapTime, false);
				if((ship == cameraTargetShip) && (ship->player->lapCount <= theRace->lapNumber))
				Sound_PlaySFxHandle(gameFXSounds[KGameFX_NewLap]);
			}
			ship->player->lastLapTime = worldState->frameTime;
		}
#if __DISPLAY_CHECK_POINT__
		else {
			DisplayCheckPointText(ship->player);
			if(ship == cameraTargetShip)
			Sound_PlaySFxHandle(gameFXSounds[kGameFX_CheckPoint]);
		}
#endif

		//Update rank
		if(worldState->frameTime >= gameConfig.startTime)
		Game_UpdateRank();
	}
	else
	Physic_DropShip(ship, ship->lastCheckPoint, kString_Driver_WrongDirection); //Fixme: change text
	
	ship->inCheckPoint = false;
}

void DriverShip_Bot_Callback(ExtendedShipPtr ship)
{
	RaceDoorPtr					door;
	Vector						dir;
	float						d;
	Data_ShipPhysicParameterPtr	params = ship->params;
	Vector						target;
	Vector						temp;
	long						nextTargetDoor;
	
	/******** CHECK IF THE BOT IS STOPPED **********/
	
	if(ship->dynamics.colliding && (ship->player->speed < kMinCollisionSpeed)) {
		Physic_DropShip(ship, ship->lastCheckPoint, kString_Drop_Collision);
		return;
	}
	
	/******** CHECK IF WE'VE MISSED THE CHECKPOINT **********/
	
#if __BOTS_DETECT_MISSED_CHECK_POINTS__
	//If nextDoor is far from nextCheckPoint, assume we've missed the checkpoint
	if(ship->nextDoor < ship->lastCheckPoint) {
		if(theRace->doorCount - ship->lastCheckPoint + ship->nextDoor > theRace->maxDoorsBetweenCheckPoints * 2)
		Physic_DropShip(ship, ship->lastCheckPoint, kString_Driver_MissedCheckPoint);
	}
	else if(ship->nextDoor - ship->lastCheckPoint > theRace->maxDoorsBetweenCheckPoints * 2)
	Physic_DropShip(ship, ship->lastCheckPoint, kString_Driver_MissedCheckPoint);
#endif
	
	/******** COMPUTE TARGET **********/
	
	//Compute next door
	--ship->nextDoor;
	do {
		++ship->nextDoor;
		if(ship->nextDoor >= theRace->doorCount)
		ship->nextDoor = 0;
		door = &theRace->doorList[ship->nextDoor];
		
		//Transform ship into door space
		Matrix_TransformVector(&door->negatedDoorMatrix, &ship->megaObject.object.pos.w, &temp);
	} while(temp.z > 0.0);
	
	//Compute target
	nextTargetDoor = Bot_GetTarget(ship->nextDoor, params->bot_Turn_Target, params->bot_Turn_MinTargetAngle, params->bot_Anticipation, &ship->megaObject.object.pos.w, &target);
	
#if __RENDER_BOT_TARGET__
	ship->botTarget = target;
#endif

	//Compute our target direction
	dir.x = target.x - ship->megaObject.object.pos.w.x;
	dir.y = 0.0;
	dir.z = target.z - ship->megaObject.object.pos.w.z;
	d = Vector_Length(dir);
	dir.x /= d;
	dir.z /= d;
	
	/******** COMPUTE INPUT **********/
	
	//Compute turn
	d = dir.x * ship->megaObject.object.pos.z.x + dir.z * ship->megaObject.object.pos.z.z;
	
	//Compute direction
	if(d > params->bot_Turn_TargetToleranceAngle) {
		//This ship direction is correct - stop turning
		if(ship->dynamics.rotationspeedL.y > params->bot_Turn_MaxRotationSpeed)
		ship->mouseH = params->bot_Turn_Opposite;
		else if(ship->dynamics.rotationspeedL.y < -params->bot_Turn_MaxRotationSpeed)
		ship->mouseH = -params->bot_Turn_Opposite;
		else
		ship->mouseH = 0.0;
	}
	else {
		//Turn the ship in the correct direction
		if((dir.z * ship->megaObject.object.pos.z.x - dir.x * ship->megaObject.object.pos.z.z) >= 0.0) //Turn right
		ship->mouseH = params->bot_Turn_Max;
		else
		ship->mouseH = -params->bot_Turn_Max;
	}

	//Do we need to turn slowly?
	if(d < params->bot_Power_LowPowerTurnAngle)
	ship->mouseV = params->bot_Power_Min;
	else
	ship->mouseV = params->bot_Power_Max;
	
#if __BOTS_ADAPTATIVE_AI__
	if(gameConfig.terrainDifficulty == kDifficulty_Easy) {
		//Is the local player behind?
		if(localShip->player->lapCount < ship->player->lapCount)
		ship->mouseV = params->bot_Power_Min;
		else if(localShip->player->lapCount == ship->player->lapCount) {
			if((localShip->nextCheckPoint < ship->nextCheckPoint) || ((ship->nextCheckPoint == 0) && (localShip->nextCheckPoint != 0)))
			ship->mouseV = params->bot_Power_Min;
		}
	}
	else if(gameConfig.terrainDifficulty == kDifficulty_Medium) {
		//Is the local player behind?
		if(localShip->player->lapCount < ship->player->lapCount)
		ship->mouseV = (params->bot_Power_Min + ship->mouseV) / 2.0;
		else if(localShip->player->lapCount == ship->player->lapCount) {
			if((localShip->nextCheckPoint < ship->nextCheckPoint) || ((ship->nextCheckPoint == 0) && (localShip->nextCheckPoint != 0)))
			ship->mouseV = (params->bot_Power_Min + ship->mouseV) / 2.0;
		}
	}
#endif
	
	//Are we flying?
	if(ship->dynamics.status == kStatus_InAir)
	ship->mouseV = fMin(params->bot_Power_InAir, ship->mouseV);
	
	//Do we need to turn the turbo on?
#if __BOTS_IMPROVED_TURBOS__
	ship->turboButton = Bot_Turbo(ship, nextTargetDoor);
#else
	if(!(ship->dynamics.status == kStatus_InAir && !(kDriver_CanTurboInAir)) && (d > params->bot_Turbo_TargetToleranceAngle) && (FVector_Distance(ship->megaObject.object.pos.w, target) > params->bot_Turbo_MinDistance))
	ship->turboButton = true;
	else
	ship->turboButton = false;
#endif

#if __BOTS_ADAPTATIVE_AI__
	if(ship->mouseV == params->bot_Power_Min)
	ship->turboButton = false;
#endif
	
	/******** CHECK IF THE BOT PASSED A CHECKPOINT **********/
	
	Bot_CheckPoint(ship);
}

/********************************************************************/
/*																	*/
/*					Ship is controlled by autopilot					*/
/*																	*/
/********************************************************************/

#if __AUTOPILOT__
void DriverShip_AutoPilot_Init(ExtendedShipPtr ship)
{
	;
}

void DriverShip_AutoPilot_Callback(ExtendedShipPtr ship)
{
	RaceDoorPtr					door;
	Vector						dir;
	float						d;
	Data_ShipPhysicParameterPtr	params = ship->params;
	Vector						target;
	Vector						temp;
	long						nextTargetDoor;
	
	/******** CHECK IF THE BOT IS STOPPED **********/
	
	if(ship->dynamics.colliding && (ship->player->speed < kMinCollisionSpeed)) {
		Physic_DropShip(ship, ship->lastCheckPoint, kString_Drop_Collision);
		return;
	}

	/******** COMPUTE TARGET **********/
	
	//Compute next door
	--ship->nextDoor;
	do {
		++ship->nextDoor;
		if(ship->nextDoor >= theRace->doorCount)
		ship->nextDoor = 0;
		door = &theRace->doorList[ship->nextDoor];
		
		//Transform ship into door space
		Matrix_TransformVector(&door->negatedDoorMatrix, &ship->megaObject.object.pos.w, &temp);
	} while(temp.z > 0.0);
	
	//Compute target
	nextTargetDoor = Bot_GetTarget(ship->nextDoor, params->bot_Turn_Target, params->bot_Turn_MinTargetAngle, params->bot_Anticipation, &ship->megaObject.object.pos.w, &target);
	
#if __RENDER_BOT_TARGET__
	ship->botTarget = target;
#endif

	//Compute our target direction
	dir.x = target.x - ship->megaObject.object.pos.w.x;
	dir.y = 0.0;
	dir.z = target.z - ship->megaObject.object.pos.w.z;
	d = Vector_Length(dir);
	dir.x /= d;
	dir.z /= d;
	
	/******** COMPUTE INPUT **********/
	
	//Compute turn
	d = dir.x * ship->megaObject.object.pos.z.x + dir.z * ship->megaObject.object.pos.z.z;
	
	//Compute direction
	if(d > params->bot_Turn_TargetToleranceAngle) {
		//This ship direction is correct - stop turning
		if(ship->dynamics.rotationspeedL.y > params->bot_Turn_MaxRotationSpeed)
		ship->mouseH = params->bot_Turn_Opposite;
		else if(ship->dynamics.rotationspeedL.y < -params->bot_Turn_MaxRotationSpeed)
		ship->mouseH = -params->bot_Turn_Opposite;
		else
		ship->mouseH = 0.0;
	}
	else {
		//Turn the ship in the correct direction
		if((dir.z * ship->megaObject.object.pos.z.x - dir.x * ship->megaObject.object.pos.z.z) >= 0.0) //Turn right
		ship->mouseH = params->bot_Turn_Max;
		else
		ship->mouseH = -params->bot_Turn_Max;
	}
	
	ship->mouseV = params->autoPilotThrottle;
	
	//Turbo off
	ship->turboButton = false;
}
#endif

/********************************************************************/
/*																	*/
/*					Ship is controlled by remote client				*/
/*																	*/
/********************************************************************/

void DriverShip_Remote_Init(ExtendedShipPtr ship)
{
	;
}

void DriverShip_Remote_Callback(ExtendedShipPtr ship)
{
	Vector				temp;
	Vector				pos,
						lastPos;
	RaceDoorPtr			door = &theRace->doorList[ship->nextCheckPoint];
	float				a,
						b,
						x;
						
	//Are we close to the checkpoint?
	temp.x = door->middle.x - ship->megaObject.object.pos.w.x;
	temp.z = door->middle.z - ship->megaObject.object.pos.w.z;
	if((temp.x * temp.x + temp.z * temp.z) > door->squareDoorWidth)
	return;
	
	//Transform ship into door space
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->megaObject.object.pos.w, &pos);
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->lastPos, &lastPos);
	
	//Check if door was passed
	if(lastPos.z * pos.z > 0.0)
	return;
	a = (pos.z - lastPos.z) / (pos.x - lastPos.x);
	b = pos.z - a * pos.x;
	x = -b / a;
	if((x < -kDoorTolerance) || (x > door->doorWidth + kDoorTolerance))
	return;
	
	//Check if door was passed in the correct direction
	if(pos.z > 0.0) {
		ship->lastCheckPoint = ship->nextCheckPoint;
		ship->nextCheckPoint = theRace->doorList[ship->nextCheckPoint].nextCheckPoint;
		ship->checkPointTime = worldState->frameTime;
		
		//Check if we have a new lap
		if(ship->lastCheckPoint == 0) {
			++ship->player->lapCount;
			if(ship->player->lapCount > theRace->lapNumber + 1) //In case of network game
			ship->player->lapCount = theRace->lapNumber + 1;
			ship->player->lastLapTime = worldState->frameTime - ship->player->lastLapTime;
			if(ship->player->lastLapTime < gameConfig.globalBestLapTime) {
				DisplayLapText(ship->player, ship->player->lastLapTime, true);
				gameConfig.globalBestLapTime = ship->player->lastLapTime;
				ship->player->bestLapTime = ship->player->lastLapTime;
				if(ship == cameraTargetShip)
				Sound_PlaySFxHandle(gameFXSounds[KGameFX_BestLap]);
			}
			else {
				if(ship->player->lastLapTime < ship->player->bestLapTime)
				ship->player->bestLapTime = ship->player->lastLapTime;
				DisplayLapText(ship->player, ship->player->lastLapTime, false);
				if((ship == cameraTargetShip) && (ship->player->lapCount <= theRace->lapNumber))
				Sound_PlaySFxHandle(gameFXSounds[KGameFX_NewLap]);
			}
			ship->player->lastLapTime = worldState->frameTime;
		}
#if __DISPLAY_CHECK_POINT__
		else {
			DisplayCheckPointText(ship->player);
			if(ship == cameraTargetShip)
			Sound_PlaySFxHandle(gameFXSounds[kGameFX_CheckPoint]);
		}
#endif

		//Update rank
		if(worldState->frameTime >= gameConfig.startTime)
		Game_UpdateRank();
	}
}

/********************************************************************/
/*																	*/
/*					Ship is controled by replay data				*/
/*																	*/
/********************************************************************/

void DriverShip_Replay_Init(ExtendedShipPtr ship)
{
	;
}

void DriverShip_Replay_Callback(ExtendedShipPtr ship)
{
	Vector				temp;
	Vector				pos,
						lastPos;
	RaceDoorPtr			door = &theRace->doorList[ship->nextCheckPoint];
	float				a,
						b,
						x;
						
	//Are we close to the checkpoint?
	temp.x = door->middle.x - ship->megaObject.object.pos.w.x;
	temp.z = door->middle.z - ship->megaObject.object.pos.w.z;
	if((temp.x * temp.x + temp.z * temp.z) > door->squareDoorWidth)
	return;
	
	//Transform ship into door space
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->megaObject.object.pos.w, &pos);
	Matrix_TransformVector(&door->negatedDoorMatrix, &ship->lastPos, &lastPos);
	
	//Check if door was passed
	if(lastPos.z * pos.z > 0.0)
	return;
	a = (pos.z - lastPos.z) / (pos.x - lastPos.x);
	b = pos.z - a * pos.x;
	x = -b / a;
	if((x < -kDoorTolerance) || (x > door->doorWidth + kDoorTolerance))
	return;
	
	//Check if door was passed in the correct direction
	if(pos.z > 0.0) {
		ship->lastCheckPoint = ship->nextCheckPoint;
		ship->nextCheckPoint = theRace->doorList[ship->nextCheckPoint].nextCheckPoint;
		ship->checkPointTime = worldState->frameTime;
		
		//Check if we have a new lap
		if(ship->lastCheckPoint == 0) {
			++ship->player->lapCount;
			if(ship->player->lapCount > theRace->lapNumber + 1) //In case of network game
			ship->player->lapCount = theRace->lapNumber + 1;
		}
	}
}
