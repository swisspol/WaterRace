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
#include				"Data Files.h"
#include				"Interface.h"
#include				"Game.h"
#include				"Infinity Audio Engine.h"
#include				"Collision Engine.h"
#include				"Preferences.h"
#include				"Network Engine.h"
#include				"Drivers.h"

#include				"Vector.h"
#include				"Matrix.h"
#include				"Quaternion.h"

#include				"Cinematic Utils.h"
#include				"Shape Utils.h"
#include				"Terrain Utils.h"

//CONSTANTES:

#define		kHeadRotateSpeed				45.0
#define		kHeadMaxRotate					30.0
#define		kRudderRotateSpeed				60.0

#define		kMaxUpsideDownTime				(8 * kTimeUnit)
#define		kMaxUnderWaterTime				(5 * kTimeUnit)
#define		kDropHeight						5.0

#define		kGravity						9.81
#define		kWaterDensity					1000.0
#define		kAirDensity						1.293
#define		kStreamInfluence				500000.0

#define		kMinimalAirResistanceSpeed		0.1
#define		kMaximalStreamResistanceSpeed	1.0

#if __LIMIT_ROTATION_SPEED__
#define		kMaxRotationSpeed				360.0 //.s-1
#endif

#define		kDropShipEngineDisableTime		(kTimeUnit * 2)
#define		kDropShipMinimalShipDistance	15.0
#define		kDropShipOffsetDistance			-6.0

#define		kCollisionRebound				0.15
#define		kCollisionRebound2				0.20
#define		kCollisionMinSpeedSound			10.0
#define		kCollisionMaxSpeedSound			30.0

#define		kBackwardCxCoefficient			4.0

#define		kSlideMinAngle					0.939693 //cos 20¡
#define		kSlideFriction					0.35

#if __BOTS_ADJUST_TRAJECTORY__
#define		kBot_MaxTurnTarget				0.7 //old = 0.85 - max = 0.5 for ship
#endif

//MACROS:

#define Linear_Interpolation(s,e,t)	((s) + (((e) - (s)) * (t)))

//ROUTINES:

inline float GetBoundingBoxVolume(Vector bBox[kBBSize])
{
	return (bBox[1].x - bBox[0].x) * (bBox[4].y - bBox[0].y) * (bBox[2].z - bBox[0].z);
}

// must be called with all initial positional parameters set and a terrain loaded
void Physic_InitShip(ExtendedShipPtr ship)
{
	DynamicsPtr			 			dynamics = &ship->dynamics;
	Data_ShipPhysicParameterPtr		params = ship->params;
	float							minX,
									maxX,
									minY,
									maxY,
									minZ,
									maxZ;
	float							totalX = ship->megaObject.object.boundingBox[1].x - ship->megaObject.object.boundingBox[0].x,
									totalZ = ship->megaObject.object.boundingBox[2].z - ship->megaObject.object.boundingBox[0].z;
	float							coeff;
							
	Vector_Clear(&dynamics->velocityG);
	Vector_Clear(&dynamics->accelerationG);
	Vector_Clear(&dynamics->rotationspeedL);
	Vector_Clear(&dynamics->rotationAccelerationL);
	
	dynamics->targetPower 		= params->power_idle;
	dynamics->motorPower		= params->power_idle;
	
	dynamics->upsideDownTime	= 0;
	dynamics->underWaterTime	= 0;
	dynamics->collisionSoundTime	= 0;
	dynamics->colliding			= false;
	dynamics->status			= kStatus_Indefinite;
	
	minY = ship->megaObject.object.boundingBox[0].y;
	maxY = ship->megaObject.object.boundingBox[4].y;
	
	minX = ship->megaObject.object.boundingBox[0].x;
	maxX = 0.0;
	minZ = ship->megaObject.object.boundingBox[0].z;
	maxZ = 0.0;
	SetUpBBox_Corners(minX, minY, minZ, maxX, maxY, maxZ, dynamics->volumeList[0].boundingBox);
	dynamics->volumeList[0].mass = params->mass * (1.0 - fAbs(minX / totalX)) * (1.0 - fAbs(minZ / totalZ));
	dynamics->volumeList[0].center.x = (minX + maxX) / 2.0;
	dynamics->volumeList[0].center.y = (minY + maxY) / 2.0;
	dynamics->volumeList[0].center.z = (minZ + maxZ) / 2.0;
	dynamics->volumeList[0].volume = GetBoundingBoxVolume(dynamics->volumeList[0].boundingBox);
	coeff = fAbs(minX / totalX) * fAbs(minZ / totalZ);
	dynamics->volumeList[0].frontalArea = params->area_frontal * coeff;
	dynamics->volumeList[0].floorArea = params->area_floor * coeff;
	dynamics->volumeList[0].lateralArea = params->area_lateral * coeff;
	dynamics->volumeList[0].archimedeCorrectionFactor = params->archimedeCorrectionFactor_back;
	dynamics->volumeList[0].submergenceCenter = dynamics->volumeList[0].center;
	
	minX = 0.0;
	maxX = ship->megaObject.object.boundingBox[1].x;
	minZ = ship->megaObject.object.boundingBox[0].z;
	maxZ = 0.0;
	SetUpBBox_Corners(minX, minY, minZ, maxX, maxY, maxZ, dynamics->volumeList[1].boundingBox);
	dynamics->volumeList[1].mass = params->mass * (1.0 - fAbs(maxX / totalX)) * (1.0 - fAbs(minZ / totalZ));
	dynamics->volumeList[1].center.x = (minX + maxX) / 2.0;
	dynamics->volumeList[1].center.y = (minY + maxY) / 2.0;
	dynamics->volumeList[1].center.z = (minZ + maxZ) / 2.0;
	dynamics->volumeList[1].volume = GetBoundingBoxVolume(dynamics->volumeList[1].boundingBox);
	coeff = fAbs(maxX / totalX) * fAbs(minZ / totalZ);
	dynamics->volumeList[1].frontalArea = params->area_frontal * coeff;
	dynamics->volumeList[1].floorArea = params->area_floor * coeff;
	dynamics->volumeList[1].lateralArea = params->area_lateral * coeff;
	dynamics->volumeList[1].archimedeCorrectionFactor = params->archimedeCorrectionFactor_back;
	dynamics->volumeList[1].submergenceCenter = dynamics->volumeList[1].center;
	
	minX = ship->megaObject.object.boundingBox[0].x;
	maxX = 0.0;
	minZ = 0.0;
	maxZ = ship->megaObject.object.boundingBox[2].z;
	SetUpBBox_Corners(minX, minY, minZ, maxX, maxY, maxZ, dynamics->volumeList[2].boundingBox);
	dynamics->volumeList[2].mass = params->mass * (1.0 - fAbs(minX / totalX)) * (1.0 - fAbs(maxZ / totalZ));
	dynamics->volumeList[2].center.x = (minX + maxX) / 2.0;
	dynamics->volumeList[2].center.y = (minY + maxY) / 2.0;
	dynamics->volumeList[2].center.z = (minZ + maxZ) / 2.0;
	dynamics->volumeList[2].volume = GetBoundingBoxVolume(dynamics->volumeList[2].boundingBox);
	coeff = fAbs(minX / totalX) * fAbs(maxZ / totalZ);
	dynamics->volumeList[2].frontalArea = params->area_frontal * coeff;
	dynamics->volumeList[2].floorArea = params->area_floor * coeff;
	dynamics->volumeList[2].lateralArea = params->area_lateral * coeff;
	dynamics->volumeList[2].archimedeCorrectionFactor = params->archimedeCorrectionFactor_front;
	dynamics->volumeList[2].submergenceCenter = dynamics->volumeList[2].center;
	
	minX = 0.0;
	maxX = ship->megaObject.object.boundingBox[1].x;
	minZ = 0.0;
	maxZ = ship->megaObject.object.boundingBox[2].z;
	SetUpBBox_Corners(minX, minY, minZ, maxX, maxY, maxZ, dynamics->volumeList[3].boundingBox);
	dynamics->volumeList[3].mass = params->mass * (1.0 - fAbs(maxX / totalX)) * (1.0 - fAbs(maxZ / totalZ));
	dynamics->volumeList[3].center.x = (minX + maxX) / 2.0;
	dynamics->volumeList[3].center.y = (minY + maxY) / 2.0;
	dynamics->volumeList[3].center.z = (minZ + maxZ) / 2.0;
	dynamics->volumeList[3].volume = GetBoundingBoxVolume(dynamics->volumeList[3].boundingBox);
	coeff = fAbs(maxX / totalX) * fAbs(maxZ / totalZ);
	dynamics->volumeList[3].frontalArea = params->area_frontal * coeff;
	dynamics->volumeList[3].floorArea = params->area_floor * coeff;
	dynamics->volumeList[3].lateralArea = params->area_lateral * coeff;
	dynamics->volumeList[3].archimedeCorrectionFactor = params->archimedeCorrectionFactor_front;
	dynamics->volumeList[3].submergenceCenter = dynamics->volumeList[3].center;
	
#if __BOTS_ADJUST_TRAJECTORY__
	if(gameConfig.terrainDifficulty == kDifficulty_Easy)
	params->bot_Turn_Target = kBot_MaxTurnTarget;
	else if(gameConfig.terrainDifficulty == kDifficulty_Medium)
	params->bot_Turn_Target = (kBot_MaxTurnTarget + params->bot_Turn_Target) / 2.0;
#endif
	params->bot_Turn_TargetToleranceAngle = cos(params->bot_Turn_TargetToleranceAngle);
	params->bot_Turn_MinTargetAngle = cos(params->bot_Turn_MinTargetAngle);
	params->bot_Power_LowPowerTurnAngle = cos(params->bot_Power_LowPowerTurnAngle);
	params->bot_Turbo_TargetToleranceAngle = cos(params->bot_Turbo_TargetToleranceAngle);
	params->bot_Turbo_MaxAngle = cos(params->bot_Turbo_MaxAngle);
	params->bot_Turbo_CollisionAngle = cos(params->bot_Turbo_CollisionAngle);
	
	params->power_MaxSoundPitchRatio -= params->power_MinSoundPitchRatio;
}

#if __USE_TRIG_TABLES__
#define	sin(x)	TSin(x)
#define	cos(x)	TCos(x)
#endif

void Physic_StopShipTurbo(ExtendedShipPtr ship)
{
	ship->turboState = false;
	ship->turboFlame = false;
	
	if(ship->currentScript != nil)
	Script_Stop(ship->currentScript);
	if(ship->turbo)
	Script_DisplayTime(ship->turbo, 0);
	ship->currentScript = nil;
}

void Physic_DropShip(ExtendedShipPtr ship, long doorNum, Str31 text)
{
	DynamicsPtr	   			dynamics = &ship->dynamics;
	Data_ShipPhysicParameterPtr		params = ship->params;
	long					nextDoor = (doorNum + 1) % theRace->doorCount;
	MatrixPtr				shipMatrix = &ship->megaObject.object.pos;
	Vector					direction;
	Boolean					keepSeeking;
	long					i;
	
	if(ship->motorStartTime > worldState->frameTime)
	return;
	
	//Reset ship orientation
	Vector_Subtract(&theRace->doorList[nextDoor].middle, &theRace->doorList[doorNum].middle, &direction);
	Vector_Normalize(&direction, &shipMatrix->z);
	shipMatrix->y.x = 0.0;
	shipMatrix->y.y = 1.0;
	shipMatrix->y.z = 0.0;
	Vector_CrossProduct(&shipMatrix->y, &shipMatrix->z, &shipMatrix->x);
	
	//Reset ship position
	shipMatrix->w.x = theRace->doorList[doorNum].middle.x;
	shipMatrix->w.y = kDropHeight;
	shipMatrix->w.z = theRace->doorList[doorNum].middle.z;
	
	//Check if collision with another ship
	do {
		keepSeeking = false;
		for(i = 0; i < shipCount; ++i) {
			if(&shipList[i] == ship)
			continue;
			if(FVector_Distance(ship->megaObject.object.pos.w, shipList[i].megaObject.object.pos.w) < kDropShipMinimalShipDistance) {
				Vector_MultiplyAdd(kDropShipOffsetDistance, &shipMatrix->z, &shipMatrix->w, &shipMatrix->w);
				keepSeeking = true;
				break;
			}
		}
	} while(keepSeeking);
	
	//Reset ship dynamics
	Vector_Clear(&dynamics->velocityG);
	Vector_Clear(&dynamics->rotationspeedL);
	dynamics->targetPower = params->power_idle;
	dynamics->motorPower = params->power_idle;
	
	//Reset turbo
	if(ship->turboState)
	Physic_StopShipTurbo(ship);
	
	//Reset driver info
	ship->inCheckPoint = false;
#if 1
	if(doorNum == ship->lastCheckPoint)
	ship->checkPointTime = worldState->frameTime;
#endif
	if(ship->player->shipPilotType == kPilot_Bot)
	ship->nextDoor = (doorNum + 1) % theRace->doorCount;
	
	//Set info text
	BlockMove(text, ship->player->infoText, text[0] + 1);
	ship->player->infoMaxTime = worldState->frameTime + 2 * kTimeUnit;
	
	//Play sound
	if(ship == cameraTargetShip)
	Sound_PlaySFxHandle(gameFXSounds[kGameFX_Error]);
	
	//Disable engine
	ship->motorStartTime = worldState->frameTime + kDropShipEngineDisableTime;
}

static void Physic_UpdateShipGeometry(ExtendedShipPtr ship, Boolean turboOn, Boolean turboOff)
{
	if(ship->headShape) {	
		float			targetY = -ship->mouseH * DegreesToRadians(kHeadMaxRotate);
		float			diffTime = (float) (worldState->frameTime - worldState->lastFrameTime) / (float) kTimeUnit;
			
		if(ship->headShape->rotateY >= targetY) {
			ship->headShape->rotateY -= diffTime * DegreesToRadians(kHeadRotateSpeed);
			if(ship->headShape->rotateY < targetY)
			ship->headShape->rotateY = targetY;
		}
		else {
			ship->headShape->rotateY += diffTime * DegreesToRadians(kHeadRotateSpeed);
			if(ship->headShape->rotateY > targetY)
			ship->headShape->rotateY = targetY;
		}
		Shape_UpdateMatrix(ship->headShape);
	}
	
	if(ship->currentScript) {
		Script_Run(ship->currentScript, worldState->frameTime);
		if(!(ship->currentScript->flags & kFlag_Running)) {
			ship->currentScript = nil;
			if(ship->turboState)
			ship->turboFlame = true;
		}
	}
	else {
		if(turboOn && !ship->turboState) {
			if(ship->turbo) {
				Script_Start(ship->turbo, worldState->frameTime);
				ship->currentScript = ship->turbo;
			}
			else
			ship->turboFlame = true;
			ship->turboStartTime = worldState->frameTime;
			ship->turboState = true;
			ship->player->turboCount -= 1;
			
			InfinityAudio_InstallSound_AmbientFX(kCreatorType, ship->turboSound, nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, kFXVolume, kInfinityAudio_NormalPitch, nil);
		}
		else if(ship->turboState && turboOff) {
			if(ship->reverseTurbo) {
				Script_Start(ship->reverseTurbo, worldState->frameTime);
				ship->currentScript = ship->reverseTurbo;
			}
			ship->turboState = false;
			ship->turboFlame = false;

			InfinityAudio_InstallSound_AmbientFX(kCreatorType, ship->turboSound, nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, kFXVolume, kInfinityAudio_NormalPitch, nil);			
		}
	}
	
	if(ship->rudderShape_1) {
		float			targetY = ship->mouseH * ship->params->propulsion_MaxRotate;
#if __SLOW_RUDDERS__
		float			diffTime = (float) (worldState->frameTime - worldState->lastFrameTime) / (float) kTimeUnit;
#endif
		
#if __SLOW_RUDDERS__
		if(ship->rudderShape_1->rotateY >= targetY) {
			ship->rudderShape_1->rotateY -= diffTime * DegreesToRadians(kRudderRotateSpeed);
			if(ship->rudderShape_1->rotateY < targetY)
			ship->rudderShape_1->rotateY = targetY;
		}
		else {
			ship->rudderShape_1->rotateY += diffTime * DegreesToRadians(kRudderRotateSpeed);
			if(ship->rudderShape_1->rotateY > targetY)
			ship->rudderShape_1->rotateY = targetY;
		}
#else
		ship->rudderShape_1->rotateY = targetY;
#endif
		Shape_UpdateMatrix(ship->rudderShape_1);
		if(ship->rudderShape_2) {
			ship->rudderShape_2->rotateY = ship->rudderShape_1->rotateY;
			Shape_UpdateMatrix(ship->rudderShape_2);
		}
	}
}

void Physic_UpdateTargetPower(ExtendedShipPtr ship)
{
	DynamicsPtr						dynamics = &ship->dynamics;
	float							mouseV = ship->mouseV;
	Data_ShipPhysicParameterPtr		params = ship->params;
	
	//set new targetpower based on position of thrust control
	if(mouseV >= 0) {
		dynamics->targetPower = params->power_Max * mouseV;
		if(dynamics->targetPower < params->power_idle)
		dynamics->targetPower = params->power_idle;
	}
	else {
		dynamics->targetPower = params->power_MaxReverse * mouseV;
		if(dynamics->targetPower > -params->power_idle)
		dynamics->targetPower = -params->power_idle;
	}
}

void Physic_UpdatePower(ExtendedShipPtr ship, float dt)
{
	DynamicsPtr						dynamics = &ship->dynamics;
	Data_ShipPhysicParameterPtr		params = ship->params;
	
	if((dynamics->motorPower * dynamics->targetPower) < 0) { //Pas correct
		if(dynamics->targetPower > 0)
		dynamics->motorPower = params->power_idle;
		else
		dynamics->motorPower = -params->power_idle;
	}
	else {
		if(dynamics->targetPower > dynamics->motorPower) {
			dynamics->motorPower += params->power_ChangePerSec * dt;
			if(dynamics->motorPower > dynamics->targetPower)
			dynamics->motorPower = dynamics->targetPower;
		}
		else {
			dynamics->motorPower -= params->power_ChangePerSec * dt;
			if(dynamics->motorPower < dynamics->targetPower)
			dynamics->motorPower = dynamics->targetPower;
		}
	}
}

inline void Physic_UpdatePowerSound(ExtendedShipPtr ship)
{
	float							pitch;
	
	//Engine
	pitch = ship->params->power_MinSoundPitchRatio + ship->params->power_MaxSoundPitchRatio * fAbs((float) (ship->dynamics.motorPower - ship->params->power_idle) / (float) (ship->params->power_Max - ship->params->power_idle));
	
	//Add turbo
	if(ship->turboFlame)
	pitch += ship->params->turbo_SoundPitch;
	
	InfinityAudio_SetSoundParams(kCreatorType, ship->engineSoundRef, ship->params->power_SoundVolume, pitch);
}

static short GetShipStatus(MatrixPtr m, Vector bBox[kBBSize], float seaHeight)
{
	long			i;
	Vector			temp;
	float			minY = kHugeDistance,
					maxY = -kHugeDistance;
	
	for(i = 0; i < kBBSize; ++i) {
		Matrix_TransformVector(m, &bBox[i], &temp);
		if(temp.y < minY)
		minY = temp.y;
		if(temp.y > maxY)
		maxY = temp.y;
	}
	
	if(minY > seaHeight)
	return kStatus_InAir;
	
	if(maxY < seaHeight)
	return kStatus_UnderWater;
	
	return kStatus_OnWater;
}

static float GetSubmergenceFactor(MatrixPtr matrix, Vector bBox[kBBSize], float seaHeight)
{
	long			i;
	Vector			temp;
	float			minY = kHugeDistance,
					maxY = -kHugeDistance;
	
	//Transform bounding box
	for(i = 0; i < kBBSize; ++i) {
		Matrix_TransformVector(matrix, &bBox[i], &temp);
		if(temp.y < minY)
		minY = temp.y;
		if(temp.y > maxY)
		maxY = temp.y;
	}
	
	if(maxY < seaHeight)
	return 1.0;
	
	if(minY > seaHeight)
	return 0.0;
	
	return (seaHeight - minY) / (maxY - minY);
}

inline float SquareFonction(float x)
{
	if(x > 0.0) {
		if(x > 1.0)
		return x * x;
		else
		return x;
	}
	else {
		if(-x > 1.0)
		return x * x;
		else
		return -x;
	}
}

void Physic_UpdateShip_Start(ExtendedShipPtr ship)
{
	MatrixPtr				shipMatrix = &ship->megaObject.object.pos;
	DynamicsPtr	   			dynamics = &ship->dynamics;
	
	//Save our position
	ship->lastPos = shipMatrix->w;
	
	//Check if out of terrain
	if((shipMatrix->w.x < border_MinX) || (shipMatrix->w.x > border_MaxX)
	 || (shipMatrix->w.z < border_MinZ) || (shipMatrix->w.z > border_MaxZ)) {
		Physic_DropShip(ship, ship->lastCheckPoint, kString_Drop_OffLimit);
		return;
	}
	
	//Check if we're upside-down
	if(shipMatrix->y.y < 0) {
		if(worldState->frameTime - dynamics->upsideDownTime > kMaxUpsideDownTime) {
			Physic_DropShip(ship, ship->lastCheckPoint, kString_Drop_UpsideDown);
			return;
		}
	}
	else
	dynamics->upsideDownTime = worldState->frameTime;
	
	//Check if under water
	if(dynamics->status == kStatus_UnderWater) {
		if(worldState->frameTime - dynamics->underWaterTime > kMaxUnderWaterTime) {
			Physic_DropShip(ship, ship->lastCheckPoint, kString_Drop_UnderWater);
			return;
		}
	}
	else
	dynamics->underWaterTime = worldState->frameTime;

	//Update ship geometry
	if(worldState->frameTime < ship->motorStartTime)
	Physic_UpdateShipGeometry(ship, false, false);
	else {
#if __STOP_TURBO_ON_REVERSE__
		if((ship->mouseV < 0.0) && ship->turboState)
		Physic_UpdateShipGeometry(ship, false, true);
		else
#endif	
		Physic_UpdateShipGeometry(ship, ship->turboButton && ship->player->turboCount, worldState->frameTime > ship->turboStartTime + ship->params->turbo_duration);
	}
	
	//Update motor target power
	Physic_UpdateTargetPower(ship);
}

void Physic_UpdateShip_End(ExtendedShipPtr ship)
{
	Matrix					inverseShipMatrix;
	
	//Update sound
	Physic_UpdatePowerSound(ship);
	
	/********************* CHECK FOR BONUS COLLISION **************/
	
	if(theTerrain.itemReferenceCount) {
		Matrix_Negate(&ship->megaObject.object.pos, &inverseShipMatrix);
		switch(ship->player->shipPilotType) {
			
			case kPilot_Local: //Pick everything
			Check_BonusCollision_Local(ship, &inverseShipMatrix);
			break;
			
#if __BOTS_CAN_PICKUP_TURBOS__
			case kPilot_Bot: //Pick only turbos
			Check_BonusCollision_Bot(ship, &inverseShipMatrix);
			break;
#endif
			
			/*case kPilot_Remote: //Pick nothing
			case kPilot_RemoteOffLine:
			case kPilot_Replay:
			
			break;*/
			
		}
	}
	
	/********************* UPDATE SHIP DATA ***********************/
	
	//Update player data
	ship->player->speed = FVector_Length(ship->dynamics.velocityG);
	if(ship->player->speed > ship->player->maxSpeed)
	ship->player->maxSpeed = ship->player->speed;
	ship->player->totalDistance += FVector_Distance(ship->lastPos, ship->megaObject.object.pos.w);
}

void Physic_UpdateShip(ExtendedShipPtr ship, unsigned long time)
{
	Data_ShipPhysicParameterPtr		params = ship->params;
	DynamicsPtr	   			dynamics = &ship->dynamics;
	float					dt = 1.0 / (float) kTimeUnit;
	Vector					forceSum,
							torqueSum;
	long					i;
	Matrix					inverseShipMatrix;
	MatrixPtr				shipMatrix = &ship->megaObject.object.pos;
	float					amplitude = theTerrain.waveAmplitude,
							scaleX = theTerrain.waveScaleX,
							scaleZ = theTerrain.waveScaleZ,
							seaTime,
							shipSpeed;
	float					seaHeight;
	Vector					force,
							torque,
							position,
							temp,
							localSpeed;
	float					speed,
							resistance,
							motorForce,
							rotateY;
	Matrix					backUpMatrix;
	CollisionTilePtr		tile;
	
	//Find the tile we're in
	tile = &collisionMap[(long) ((-terrain_MinZ + ship->megaObject.object.pos.w.z) / kCollisionMapResolution) * collisionMapWidth
		 + (long) ((-terrain_MinX + ship->megaObject.object.pos.w.x) / kCollisionMapResolution)];
	
	//Update physics
	seaTime = (float) time / theTerrain.timeScale;
	Vector_Clear(&forceSum);
	Vector_Clear(&torqueSum);
	
	//Precalculate some values
	Matrix_Negate(shipMatrix, &inverseShipMatrix);
	shipSpeed = Vector_Length(dynamics->velocityG);
	
	//Update engine
	if(time == ship->motorStartTime)
	dynamics->motorPower = params->power_idle;
	Physic_UpdatePower(ship, dt);
	
	//Update ship status
	seaHeight = WaveHeight(seaTime, shipMatrix->w.x, shipMatrix->w.z, scaleX, scaleZ, amplitude);
	dynamics->status = GetShipStatus(shipMatrix, ship->megaObject.object.boundingBox, seaHeight);
	
	//Update volume submergence
	if(dynamics->status == kStatus_OnWater)
	for(i = 0; i < kVolumeCount; ++i) {
		Matrix_TransformVector(shipMatrix, &dynamics->volumeList[i].center, &position);
		seaHeight = WaveHeight(seaTime, position.x, position.z, scaleX, scaleZ, amplitude);
		dynamics->volumeList[i].submergence = GetSubmergenceFactor(shipMatrix, dynamics->volumeList[i].boundingBox, seaHeight);
		dynamics->volumeList[i].submergenceCenter.y = ship->megaObject.object.boundingBox[0].y + dynamics->volumeList[i].submergence * (ship->megaObject.object.boundingBox[4].y - ship->megaObject.object.boundingBox[0].y);
	}
	else if(dynamics->status == kStatus_UnderWater)
	for(i = 0; i < kVolumeCount; ++i) {
		dynamics->volumeList[i].submergence = 1.0;
		dynamics->volumeList[i].submergenceCenter.y = dynamics->volumeList[i].center.y;
	}
	
	/********************* GRAVITY ***********************/
	forceSum.y += -kGravity * params->mass;
	
	/********************* ARCHIMEDE ***********************/
	if((dynamics->status == kStatus_UnderWater) || (dynamics->status == kStatus_OnWater))
	for(i = 0; i < kVolumeCount; ++i) {
		force.x = 0.0;
		force.y = kGravity * kWaterDensity * dynamics->volumeList[i].volume * dynamics->volumeList[i].submergence * dynamics->volumeList[i].archimedeCorrectionFactor;
		force.z = 0.0;
	
		Vector_Add(&force, &forceSum, &forceSum);
		
		Matrix_RotateVector(&inverseShipMatrix, &force, &force);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
	}
	
	/********************* STREAM RESISTANCE ***********************/
	if(((dynamics->status == kStatus_UnderWater) || (dynamics->status == kStatus_OnWater)) && (shipSpeed < kMaximalStreamResistanceSpeed))
	for(i = 0; i < kVolumeCount; ++i) {
		force.x = -theTerrain.seaMoveU * dynamics->volumeList[i].submergence * (params->Cx_H + params->Cx_L) / 2.0 / kVolumeCount * kStreamInfluence;
		force.y = 0.0;
		force.z = -theTerrain.seaMoveV * dynamics->volumeList[i].submergence * (params->Cx_H + params->Cx_L) / 2.0 / kVolumeCount * kStreamInfluence;
		
		Vector_Add(&force, &forceSum, &forceSum);
		
		Matrix_RotateVector(&inverseShipMatrix, &force, &force);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
	}
	
	/********************* WATER RESISTANCE ***********************/
	if((dynamics->status == kStatus_UnderWater) || (dynamics->status == kStatus_OnWater))
	for(i = 0; i < kVolumeCount; ++i) {
		Matrix_RotateVector(&inverseShipMatrix, &dynamics->velocityG, &localSpeed);
		
		//Calculate water resistance - vertical
		resistance = params->Cx_V * kWaterDensity * SquareFonction(localSpeed.y) / 2.0 * dynamics->volumeList[i].floorArea;
		if(localSpeed.y >= 0.0)
		resistance = -resistance;
		force.x = 0.0;
		force.y = resistance * 2.0 * dynamics->volumeList[i].submergence;
		force.z = 0.0;
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		
		Vector_Multiply(params->stabilityCorrection, &force, &force);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
		
		//Calculate water resistance - frontal
		resistance = params->Cx_H * kWaterDensity * SquareFonction(localSpeed.z) / 2.0 * dynamics->volumeList[i].frontalArea;
		if(localSpeed.z < 0.0) //If going backward, augment Cx_H
		resistance *= kBackwardCxCoefficient;
		if(localSpeed.z >= 0.0)
		resistance = -resistance;
		force.x = 0.0;
		force.y = 0.0;
		force.z = resistance * dynamics->volumeList[i].submergence;
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		
		Vector_Multiply(params->stabilityCorrection, &force, &force);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
		
		//Calculate water resistance - lateral
		resistance = params->Cx_L * kWaterDensity * SquareFonction(localSpeed.x) / 2.0 * dynamics->volumeList[i].lateralArea;
		if(localSpeed.x >= 0.0)
		resistance = -resistance;
		force.x = resistance * dynamics->volumeList[i].submergence;
		force.y = 0.0;
		force.z = 0.0;
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		
		Vector_Multiply(params->stabilityCorrection, &force, &force);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
	}
	
	if((dynamics->status == kStatus_UnderWater) || (dynamics->status == kStatus_OnWater))
	for(i = 0; i < kVolumeCount; ++i) {
		Vector_CrossProduct(&dynamics->rotationspeedL, &dynamics->volumeList[i].center, &localSpeed);
		
		//Calculate water resistance - vertical
		resistance = params->Cx_V * kWaterDensity * SquareFonction(localSpeed.y) / 2.0 * dynamics->volumeList[i].floorArea;
		if(localSpeed.y >= 0.0)
		resistance = -resistance;
		force.x = 0.0;
		force.y = resistance * dynamics->volumeList[i].submergence;
		force.z = 0.0;
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
		
		//Calculate water resistance - frontal
		resistance = params->Cx_H * kWaterDensity * SquareFonction(localSpeed.z) / 2.0 * dynamics->volumeList[i].frontalArea;
		if(localSpeed.z < 0.0) //If going backward change Cx_H
		resistance *= 2;
		if(localSpeed.z >= 0.0)
		resistance = -resistance;
		force.x = 0.0;
		force.y = 0.0;
		force.z = resistance * dynamics->volumeList[i].submergence;
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
		
		//Calculate water resistance - lateral
		resistance = params->Cx_L * kWaterDensity * SquareFonction(localSpeed.x) / 2.0 * dynamics->volumeList[i].lateralArea;
		if(localSpeed.x >= 0.0)
		resistance = -resistance;
		force.x = resistance * dynamics->volumeList[i].submergence;
		force.y = 0.0;
		force.z = 0.0;
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		Vector_CrossProduct(&dynamics->volumeList[i].submergenceCenter, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
	}
	
	if(dynamics->status == kStatus_InAir) {
		//Calculate air resistance
		speed = Vector_Length(dynamics->rotationspeedL);
		if(speed > 0.01) {
			resistance = params->Cx_R * kAirDensity * SquareFonction(speed) / 2.0 * params->area_rotationnal;
			torque.x = -dynamics->rotationspeedL.x / speed * resistance;
			torque.y = -dynamics->rotationspeedL.y / speed * resistance;
			torque.z = -dynamics->rotationspeedL.z / speed * resistance;
			Vector_Add(&torque, &torqueSum, &torqueSum);
		}
	}
	
	/********************* AIR RESISTANCE ***********************/
	if(dynamics->status == kStatus_InAir) {
		//Calculate air resistance
		if(shipSpeed > kMinimalAirResistanceSpeed) {
			Vector_Multiply(1.0 / shipSpeed, &dynamics->velocityG, &temp);
			resistance = params->Cw * kAirDensity * SquareFonction(shipSpeed) / 2.0 * params->area_floor;
			Vector_Multiply(-resistance, &temp, &force);
			
			Vector_Add(&force, &forceSum, &forceSum);
		}
		
		//Calculate air portance
		Vector_Normalize(&ship->megaObject.object.pos.z, &temp);
		speed = Vector_DotProduct(&dynamics->velocityG, &temp);
		resistance = params->Cf * kAirDensity * SquareFonction(speed) / 2.0 * params->area_floor;
		Vector_Normalize(&ship->megaObject.object.pos.y, &temp);
		Vector_Multiply(resistance, &temp, &force);
		
		Vector_Add(&force, &forceSum, &forceSum);
	}
	
	/********************* RUDDER ***********************/
	if(params->rudderType == kRudder) {
		float			density;
		
		//Check rudder position related to water
		Matrix_TransformVector(shipMatrix, &params->rudder_fakePosition, &position);
		seaHeight = WaveHeight(seaTime, position.x, position.z, scaleX, scaleZ, amplitude);
		if(position.y <= seaHeight)
		density = kWaterDensity;
		else
		density = kAirDensity;
		
		rotateY = ship->mouseH * params->rudder_MaxRotate;
		
		//Calculate resistance - frontal
		Vector_Normalize(&ship->megaObject.object.pos.z, &temp);
		speed = Vector_DotProduct(&dynamics->velocityG, &temp);
		resistance = density * SquareFonction(speed) / 2.0 * params->rudder_width * params->rudder_height * fAbs(sin(rotateY));
		if(speed >= 0.0)
		resistance = -resistance;
		
		force.x = 0.0;
		force.y = 0.0;
		force.z = resistance;
		
		position.x = params->rudder_position.x - sin(rotateY) * params->rudder_width / 2.0;
		position.y = params->rudder_position.y;
		position.z = params->rudder_position.z + cos(rotateY) * params->rudder_width / 2.0;
		
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		
		Vector_CrossProduct(&position, &force, &torque);
#if 0
		if((ship == localShip) && (shipSpeed < 27.78)) { //100Km/h
			float a = (1.0 - 100.0) / (27.78 - 1);
			
			Vector_Multiply(shipSpeed * a + 100.0 - a, &torque, &torque);
		}
#endif
		Vector_Add(&torque, &torqueSum, &torqueSum);
	}
	
	/********************* MOTOR ***********************/
	
	//Check motor position related to water
	if(time >= ship->motorStartTime) {
		Matrix_TransformVector(shipMatrix, &params->propulsion_fakePosition, &position);
		seaHeight = WaveHeight(seaTime, position.x, position.z, scaleX, scaleZ, amplitude);
		if(((params->propulsionType == kWaterPropellor) && (position.y <= seaHeight) && (dynamics->status == kStatus_OnWater))
			 || ((params->propulsionType == kWaterJet) && (position.y <= seaHeight) && (dynamics->status != kStatus_InAir))
			 || ((params->propulsionType == kAirPropellor) && (position.y >= seaHeight) && (dynamics->status != kStatus_UnderWater))) {
			if(dynamics->motorPower > 0.0)
			motorForce = (dynamics->motorPower - params->power_idle) / (shipSpeed + 1.0);
			else
			motorForce = (dynamics->motorPower + params->power_idle) / (shipSpeed + 1.0);
			if(params->propulsionSteering == kDirectionalPropulsion) {
				rotateY = ship->mouseH * params->propulsion_MaxRotate;
				force.x = sin(rotateY) * motorForce;
				force.y = 0.0;
				force.z = cos(rotateY) * motorForce;
			}
			else {
				force.x = 0.0;
				force.y = 0.0;
				force.z = motorForce;
			}
			
			//Apply motor angle
			force.y = sin(params->propulsion_Angle) * force.z;
			force.z = cos(params->propulsion_Angle) * force.z;
			if(dynamics->motorPower < 0.0) //If we're going backward
			force.y = -force.y;
			
			Matrix_RotateVector(shipMatrix, &force, &temp);
			Vector_Add(&temp, &forceSum, &forceSum);
			
			Vector_CrossProduct(&params->propulsion_position, &force, &torque);
			Vector_Add(&torque, &torqueSum, &torqueSum);
		}
	}
	
	/********************* TURBO ***********************/

	if(ship->turboFlame) {
		motorForce = params->power_Turbo / (shipSpeed + 1.0);
		if(params->turboSteering == kDirectionalTurbo) {
			rotateY = ship->mouseH * params->turbo_MaxRotate;
			force.x = sin(rotateY) * motorForce;
			force.y = 0.0;
			force.z = cos(rotateY) * motorForce;
		}
		else {
			force.x = 0.0;
			force.y = 0.0;
			force.z = motorForce;
		}
		
		//Apply turbo angle
		force.y = sin(params->turbo_Angle) * force.z;
		force.z = cos(params->turbo_Angle) * force.z;
		
		Matrix_RotateVector(shipMatrix, &force, &temp);
		Vector_Add(&temp, &forceSum, &forceSum);
		
		Vector_CrossProduct(&params->turbo_position, &force, &torque);
		Vector_Add(&torque, &torqueSum, &torqueSum);
	}

	/********************* SAVE POSITION ***********************/
	
	backUpMatrix = *shipMatrix;
	
	/********************* UPDATE POSITION ***********************/
	
	//Apply Newton
	Vector_Multiply(1.0 / params->mass, &forceSum, &dynamics->accelerationG);
	//Update speed
	Vector_MultiplyAdd(dt, &dynamics->accelerationG, &dynamics->velocityG, &dynamics->velocityG);
	//Update position
	Vector_MultiplyAdd(dt, &dynamics->velocityG, &shipMatrix->w, &shipMatrix->w);
	
	/********************* UPDATE ORIENTATION ***********************/
	
	//Apply law (?)
	dynamics->rotationAccelerationL.x = torqueSum.x / params->inertia_roll;
	dynamics->rotationAccelerationL.y = torqueSum.y / params->inertia_pitch;
	dynamics->rotationAccelerationL.z = torqueSum.z / params->inertia_yaw;
	//Update rotation speed
	Vector_MultiplyAdd(dt, &dynamics->rotationAccelerationL, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
#if __LIMIT_ROTATION_SPEED__
	rotateY = FVector_Length(dynamics->rotationspeedL);
	if(rotateY > DegreesToRadians(kMaxRotationSpeed))
	Vector_Multiply(DegreesToRadians(kMaxRotationSpeed) / rotateY, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
#endif
	//Update orientation
	Matrix_RotateAroundLocalAxisY(shipMatrix, dynamics->rotationspeedL.y * dt);
	Matrix_RotateAroundLocalAxisZ(shipMatrix, dynamics->rotationspeedL.z * dt);
	Matrix_RotateAroundLocalAxisX(shipMatrix, dynamics->rotationspeedL.x * dt);

	shipSpeed = Vector_Length(dynamics->velocityG);
	dynamics->colliding = false;
	
	/********************* CHECK REFERENCE COLLISION ***********************/

	for(i = 0; i < tile->referenceCount; ++i) {
		Vector		normal;
		
		if(Object_Collision_WithNormal(&ship->megaObject, shipMatrix, 1.0, (MegaObjectPtr) tile->referenceList[i]->id, &tile->referenceList[i]->pos, tile->referenceList[i]->scale, &normal)) {
#if __RENDER_SHIP_COLLISION_NORMAL__
			ship->normal = normal;
#endif
			
			float value = Vector_DotProduct(&dynamics->velocityG, &normal);
			if(value < 0.0) {
				//Restore ship position
				*shipMatrix = backUpMatrix;
				
				//Stop engine if necessary
				if((value < -kSlideMinAngle * shipSpeed)/* || (dynamics->status == kStatus_InAir)*/) {
					//Clear dynamics
					Vector_Multiply(-kCollisionRebound, &dynamics->velocityG,  &dynamics->velocityG);
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					if(dynamics->motorPower > 0)
					dynamics->motorPower = -params->power_idle;
					else
					dynamics->motorPower = params->power_idle;
					
					//Play sound
					if((shipSpeed > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
						InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (shipSpeed - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
						dynamics->collisionSoundTime = worldState->frameTime;
					}
				
					//Stop turbo
					if(ship->turboState)
					ship->turboStartTime = 0;
				}
				else {
					Vector		temp;
					
					Vector_CrossProduct(&normal, &dynamics->velocityG, &temp);
					if(Vector_DotProduct(&dynamics->rotationspeedL, &temp) > 0.0)
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					
					Vector_Multiply(value, &normal, &normal);
					Vector_Subtract(&dynamics->velocityG, &normal, &dynamics->velocityG);
					
					//Update position
					Vector_MultiplyAdd(kSlideFriction * dt, &dynamics->velocityG, &shipMatrix->w, &shipMatrix->w);
					
					//Update orientation
					Matrix_RotateAroundLocalAxisY(shipMatrix, dynamics->rotationspeedL.y * dt);
					Matrix_RotateAroundLocalAxisZ(shipMatrix, dynamics->rotationspeedL.z * dt);
					Matrix_RotateAroundLocalAxisX(shipMatrix, dynamics->rotationspeedL.x * dt);
				}
			}
			
			//Stop checking
			dynamics->colliding = true;
			break;
		}
	}

	/********************* CHECK ENCLOSURE COLLISION ***********************/

	if(!dynamics->colliding)
	for(i = 0; i < theTerrain.enclosureCount; ++i) {
		Vector		normal;
		
		if(ShipEnclosure_Collision_WithNormal(ship, theTerrain.enclosureList[i], &normal)) {
			float value = Vector_DotProduct(&dynamics->velocityG, &normal);
			if(value < 0.0) {
				//Restore ship position
				*shipMatrix = backUpMatrix;
				
				//Stop engine if necessary
				if((value < -kSlideMinAngle * shipSpeed) || (dynamics->status == kStatus_InAir)) {
					//Clear dynamics
					Vector_Multiply(-kCollisionRebound, &dynamics->velocityG,  &dynamics->velocityG);
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					if(dynamics->motorPower > 0)
					dynamics->motorPower = -params->power_idle;
					else
					dynamics->motorPower = params->power_idle;
					
					//Play sound
					if((shipSpeed > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
						InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (shipSpeed - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
						dynamics->collisionSoundTime = worldState->frameTime;
					}
					
					//Stop turbo
					if(ship->turboState)
					ship->turboStartTime = 0;
				}
				else {
					Vector		temp;
					
					Vector_CrossProduct(&normal, &dynamics->velocityG, &temp);
					if(Vector_DotProduct(&dynamics->rotationspeedL, &temp) > 0.0)
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					
					Vector_Multiply(value, &normal, &normal);
					Vector_Subtract(&dynamics->velocityG, &normal, &dynamics->velocityG);
					
					//Update position
					Vector_MultiplyAdd(kSlideFriction * dt, &dynamics->velocityG, &shipMatrix->w, &shipMatrix->w);
					
					//Update orientation
					Matrix_RotateAroundLocalAxisY(shipMatrix, dynamics->rotationspeedL.y * dt);
					Matrix_RotateAroundLocalAxisZ(shipMatrix, dynamics->rotationspeedL.z * dt);
					Matrix_RotateAroundLocalAxisX(shipMatrix, dynamics->rotationspeedL.x * dt);
				}
			}
			
			//Stop checking
			dynamics->colliding = true;
			break;
		}
	}

	/********************* CHECK ANIM COLLISION ***********************/

	//Look for collision
	if(!dynamics->colliding)
	for(i = 0; i < theTerrain.animCount; ++i) {
		if(!(theTerrain.animList[i]->flags & kAnimFlag_CollisionDetection))
		continue;
		
		Vector		normal;
		
		if(Object_Collision_WithNormal(&ship->megaObject, &ship->megaObject.object.pos, 1.0, (MegaObjectPtr) theTerrain.animList[i]->modelID, &theTerrain.animList[i]->pos, 1.0, &normal)) {
#if __RENDER_SHIP_COLLISION_NORMAL__
			ship->normal = normal;
#endif
			float value = Vector_DotProduct(&dynamics->velocityG, &normal);
			if(value < 0.0) {
				//Restore ship position
				*shipMatrix = backUpMatrix;
				
				//Stop engine if necessary
				if((value < -kSlideMinAngle * shipSpeed) || (dynamics->status == kStatus_InAir)) {
					//Clear dynamics
					Vector_Multiply(-kCollisionRebound, &dynamics->velocityG,  &dynamics->velocityG);
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					if(dynamics->motorPower > 0)
					dynamics->motorPower = -params->power_idle;
					else
					dynamics->motorPower = params->power_idle;
					
					//Play sound
					if((shipSpeed > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
						InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (shipSpeed - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
						dynamics->collisionSoundTime = worldState->frameTime;
					}
					
					//Stop turbo
					if(ship->turboState)
					ship->turboStartTime = 0;
				}
				else {
					Vector		temp;
					
					Vector_CrossProduct(&normal, &dynamics->velocityG, &temp);
					if(Vector_DotProduct(&dynamics->rotationspeedL, &temp) > 0.0)
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					
					Vector_Multiply(value, &normal, &normal);
					Vector_Subtract(&dynamics->velocityG, &normal, &dynamics->velocityG);
					
					//Update position
					Vector_MultiplyAdd(kSlideFriction * dt, &dynamics->velocityG, &shipMatrix->w, &shipMatrix->w);
					
					//Update orientation
					Matrix_RotateAroundLocalAxisY(shipMatrix, dynamics->rotationspeedL.y * dt);
					Matrix_RotateAroundLocalAxisZ(shipMatrix, dynamics->rotationspeedL.z * dt);
					Matrix_RotateAroundLocalAxisX(shipMatrix, dynamics->rotationspeedL.x * dt);
				}
			}
			
			//Stop checking
			dynamics->colliding = true;
			break;
		}
	}
	
	/********************* CHECK SHIP COLLISION ***********************/
		
#if __NO_COLLISION_WITH_FINISHED__
	if(!ship->player->finished)
#endif
	for(i = 0; i < shipCount; ++i) {
		//Make sure we do not self-test
		if(ship == &shipList[i])
		continue;
		
#if __NO_COLLISION_WITH_FINISHED__
		//If this ship is offline or if the player has finished the race, skip it
		if((shipList[i].player->shipPilotType == kPilot_RemoteOffLine) || shipList[i].player->finished)
		continue;
#endif
		
		//Check ship collision
#if __SLIDING_SHIP_COLLISIONS__
		Vector		normal;
		
		if(Object_Collision_WithNormal(&ship->megaObject, &ship->megaObject.object.pos, 1.0, &shipList[i].megaObject, &shipList[i].megaObject.object.pos, 1.0, &normal)) {
#if __RENDER_SHIP_COLLISION_NORMAL__
			ship->normal = normal;
#endif
			
			float value = Vector_DotProduct(&dynamics->velocityG, &normal);
			if(value < 0.0) {
				//Restore ship position
				*shipMatrix = backUpMatrix;
				
#if __REBOUND_SHIP_COLLISIONS__
				//Stop engine if necessary
				if((value < -kSlideMinAngle * shipSpeed)/* || (dynamics->status == kStatus_InAir)*/) {
					//Clear dynamics
					Vector_Multiply(-kCollisionRebound, &dynamics->velocityG,  &dynamics->velocityG);
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					if(dynamics->motorPower > 0)
					dynamics->motorPower = -params->power_idle;
					else
					dynamics->motorPower = params->power_idle;
					
					//Stop turbo
					if(ship->turboState)
					ship->turboStartTime = 0;
				}
				else {
#endif
					Vector		temp;
					
					Vector_CrossProduct(&normal, &dynamics->velocityG, &temp);
					if(Vector_DotProduct(&dynamics->rotationspeedL, &temp) > 0.0)
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					
					Vector_Multiply(value, &normal, &normal);
					
#if __IMPROVED_SHIP_COLLISIONS__
					/*Vector_Subtract(&dynamics->velocityG, &normal, &newVelocityG);
					//Update position
					Vector_MultiplyAdd(kSlideFriction * dt, &newVelocityG, &shipMatrix->w, &shipMatrix->w);*/
					
					dynamics->velocityG.x = (2.0 * dynamics->velocityG.x - normal.x) / 2.0;
					dynamics->velocityG.y = (2.0 * dynamics->velocityG.y - normal.y) / 2.0;
					dynamics->velocityG.z = (2.0 * dynamics->velocityG.z - normal.z) / 2.0;
#else				
					Vector_Subtract(&dynamics->velocityG, &normal, &dynamics->velocityG);
#endif	
					//Update position
					Vector_MultiplyAdd(kSlideFriction * dt, &dynamics->velocityG, &shipMatrix->w, &shipMatrix->w);
					
					//Update orientation
					Matrix_RotateAroundLocalAxisY(shipMatrix, dynamics->rotationspeedL.y * dt);
					Matrix_RotateAroundLocalAxisZ(shipMatrix, dynamics->rotationspeedL.z * dt);
					Matrix_RotateAroundLocalAxisX(shipMatrix, dynamics->rotationspeedL.x * dt);
#if __REBOUND_SHIP_COLLISIONS__
				}
#endif
				
				//Play sound
				if((-value > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
					InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (-value - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
					dynamics->collisionSoundTime = worldState->frameTime;
				}
			}
			
			//Stop checking
			//dynamics->colliding = true;
			break;
		}
#else
		if(Object_Collision(&ship->megaObject, &ship->megaObject.object.pos, 1.0, &shipList[i].megaObject, &shipList[i].megaObject.object.pos, 1.0)) {
			Vector		temp;
			
			//Switch velocities
			temp = ship->dynamics.velocityG;
			ship->dynamics.velocityG = shipList[i].dynamics.velocityG;
			shipList[i].dynamics.velocityG = temp;
			
			temp = ship->dynamics.rotationspeedL;
			ship->dynamics.rotationspeedL = shipList[i].dynamics.rotationspeedL;
			shipList[i].dynamics.rotationspeedL = temp;
			
			//Restore ship position
			*shipMatrix = backUpMatrix;
			
			//Play sound
			if((shipSpeed > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
				InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (shipSpeed - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
				dynamics->collisionSoundTime = worldState->frameTime;
			}
			
			//Stop turbo
			if(ship->turboState)
			ship->turboStartTime = 0;
			
			//Stop checking
			//dynamics->colliding = true;
			break;
		}
#endif
	}
	
	/********************* RECORD RACE ***********************/
	
#if __RECORD_RACE__
	//Record data
	if((ship->eventBuffer != nil) && (time % kReplayEventDistance == 0) && (ship->eventCount < kMaxReplayEvents)) {
		ReplayEventPtr		eventPtr = &ship->eventBuffer[ship->eventCount];
		
		eventPtr->timeStamp = time;
		eventPtr->mouseH = ship->mouseH * kShortMax;
		eventPtr->motorPower = dynamics->motorPower / kPowerUnitToWatt;
		eventPtr->turboState = ship->turboState;
		eventPtr->position = shipMatrix->w;
		Quaternion_MatrixToQuaternion(shipMatrix, &eventPtr->orientation);
		
		++eventPtr;
		ship->eventCount += 1;
	}
#endif
}

void Physic_UpdateShip_Network_Start(ExtendedShipPtr ship)
{
	ShipStatePtr			shipState = &networkShipBuffers[ship->shipNum];
	
	//Save our position
	ship->lastPos = ship->megaObject.object.pos.w;
	
	//Update geometry
	ship->mouseH = (float) shipState->mouseH / kShortMax;
	Physic_UpdateShipGeometry(ship, shipState->turboState, !shipState->turboState);
	
	//Update motor power
	ship->dynamics.motorPower = shipState->motorPower * kPowerUnitToWatt;
}

void Physic_UpdateShip_Network_End(ExtendedShipPtr ship)
{
	//Update sound
	Physic_UpdatePowerSound(ship);
	
	//Update player data
	ship->player->speed = FVector_Length(ship->dynamics.velocityG);
	if(ship->player->speed > ship->player->maxSpeed)
	ship->player->maxSpeed = ship->player->speed;
	ship->player->totalDistance += FVector_Distance(ship->lastPos, ship->megaObject.object.pos.w);
}

void Physic_UpdateShip_Network(ExtendedShipPtr ship, unsigned long time)
{
	float					dt;
	MatrixPtr				shipMatrix = &ship->megaObject.object.pos;
	ShipStatePtr			shipState = &networkShipBuffers[ship->shipNum];
	Quaternion				localQuaternion,
							currentOrientation;
	unsigned long			localTime;
	Matrix					backUpMatrix;
	float					shipSpeed;
	DynamicsPtr	   			dynamics = &ship->dynamics;
	
	shipSpeed = FVector_Length(dynamics->velocityG);
	
	/********************* SAVE POSITION ***********************/
	
	backUpMatrix = *shipMatrix;
	
	/********************* UPDATE POSITION ***********************/
	
	//We only have events in the past - we need to extrapolate from last event
	if(time >= shipState->timeStamp) {
		//Reset orientation
		localQuaternion.w = shipState->orientation.w;
		localQuaternion.x = -shipState->orientation.x;
		localQuaternion.y = -shipState->orientation.y;
		localQuaternion.z = -shipState->orientation.z;
		Quaternion_QuaternionToMatrix(&localQuaternion, shipMatrix);
		dynamics->rotationspeedL = shipState->rotationspeedL;
		dynamics->rotationAccelerationL = shipState->rotationAccelerationL;
		
		//Reset position
		shipMatrix->w = shipState->position;
		dynamics->velocityG = shipState->velocityG;
		dynamics->accelerationG = shipState->accelerationG;
		
		localTime = shipState->timeStamp;
		dt = 1.0 / (float) kTimeUnit;
		while(localTime < time) {
			Vector_MultiplyAdd(dt, &dynamics->accelerationG, &dynamics->velocityG, &dynamics->velocityG);
			Vector_MultiplyAdd(dt, &dynamics->velocityG, &shipMatrix->w, &shipMatrix->w);
			
			Vector_MultiplyAdd(dt, &dynamics->rotationAccelerationL, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
			Matrix_RotateAroundLocalAxisY(shipMatrix, dynamics->rotationspeedL.y * dt);
			Matrix_RotateAroundLocalAxisZ(shipMatrix, dynamics->rotationspeedL.z * dt);
			Matrix_RotateAroundLocalAxisX(shipMatrix, dynamics->rotationspeedL.x * dt);
			
			//Update time stamp
			++localTime;
		}
	}
	else { //we have a future event => interpolate
		dt = (float) (time - (time - 1)) / (float) (shipState->timeStamp - (time - 1));
		
		//Interpolate orientation
		Quaternion_MatrixToQuaternion(shipMatrix, &currentOrientation);
		Quaternion_LerpInterpolation(&currentOrientation,&shipState->orientation, dt, &localQuaternion);
		localQuaternion.x = -localQuaternion.x;
		localQuaternion.y = -localQuaternion.y;
		localQuaternion.z = -localQuaternion.z;
		Quaternion_Normalize(&localQuaternion);
		Quaternion_QuaternionToMatrix(&localQuaternion, shipMatrix);
		
		//Interpolate position
		shipMatrix->w.x = Linear_Interpolation(shipMatrix->w.x, shipState->position.x, dt);
		shipMatrix->w.y = Linear_Interpolation(shipMatrix->w.y, shipState->position.y, dt);
		shipMatrix->w.z = Linear_Interpolation(shipMatrix->w.z, shipState->position.z, dt);
	}
	
	/********************* CHECK SHIP COLLISION ***********************/
		
#if __CHECK_NETWORK_SHIP_COLLISION__
#if __NO_COLLISION_WITH_FINISHED__
	if(!ship->player->finished)
#endif
	for(i = 0; i < shipCount; ++i) {
		//Make sure we do not self-test
		if(ship == &shipList[i])
		continue;
		
#if __NO_COLLISION_WITH_FINISHED__
		//If this ship is offline or if the player has finished the race, skip it
		if((shipList[i].player->shipPilotType == kPilot_RemoteOffLine) || shipList[i].player->finished)
		continue;
#endif
		
		//Check ship collision
#if __SLIDING_SHIP_COLLISIONS__
		Vector		normal;
		
		if(Object_Collision_WithNormal(&ship->megaObject, &ship->megaObject.object.pos, 1.0, &shipList[i].megaObject, &shipList[i].megaObject.object.pos, 1.0, &normal)) {
#if __RENDER_SHIP_COLLISION_NORMAL__
			ship->normal = normal;
#endif
			
			float value = Vector_DotProduct(&dynamics->velocityG, &normal);
			if(value < 0.0) {
				//Restore ship position
				*shipMatrix = backUpMatrix;
				
#if __REBOUND_SHIP_COLLISIONS__
				//Stop engine if necessary
				if((value < -kSlideMinAngle * shipSpeed)/* || (dynamics->status == kStatus_InAir)*/) {
					//Clear dynamics
					Vector_Multiply(-kCollisionRebound, &dynamics->velocityG,  &dynamics->velocityG);
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					if(dynamics->motorPower > 0)
					dynamics->motorPower = -params->power_idle;
					else
					dynamics->motorPower = params->power_idle;
					
					//Play sound
					if((shipSpeed > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
						InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (shipSpeed - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
						dynamics->collisionSoundTime = worldState->frameTime;
					}
					
					//Stop turbo
					if(ship->turboState)
					ship->turboStartTime = 0;
				}
				else {
#endif
					Vector		temp;
					Vector		newVelocityG;
					
					Vector_CrossProduct(&normal, &dynamics->velocityG, &temp);
					if(Vector_DotProduct(&dynamics->rotationspeedL, &temp) > 0.0)
					Vector_Multiply(-kCollisionRebound, &dynamics->rotationspeedL, &dynamics->rotationspeedL);
					
					Vector_Multiply(value, &normal, &normal);
#if __IMPROVED_SHIP_COLLISIONS__
					/*Vector_Subtract(&dynamics->velocityG, &normal, &newVelocityG);
					//Update position
					Vector_MultiplyAdd(kSlideFriction * dt, &newVelocityG, &shipMatrix->w, &shipMatrix->w);*/
					
					dynamics->velocityG.x = (2.0 * dynamics->velocityG.x - normal.x) / 2.0;
					dynamics->velocityG.y = (2.0 * dynamics->velocityG.y - normal.y) / 2.0;
					dynamics->velocityG.z = (2.0 * dynamics->velocityG.z - normal.z) / 2.0;
#else				
					Vector_Subtract(&dynamics->velocityG, &normal, &dynamics->velocityG);
					
					//Update position
					Vector_MultiplyAdd(kSlideFriction * dt, &dynamics->velocityG, &shipMatrix->w, &shipMatrix->w);
#endif	
					//Update orientation
					Matrix_RotateAroundLocalAxisY(shipMatrix, dynamics->rotationspeedL.y * dt);
					Matrix_RotateAroundLocalAxisZ(shipMatrix, dynamics->rotationspeedL.z * dt);
					Matrix_RotateAroundLocalAxisX(shipMatrix, dynamics->rotationspeedL.x * dt);
#if __REBOUND_SHIP_COLLISIONS__
				}
#endif
				//Play sound
				if((-value > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
					InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (-value - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
					dynamics->collisionSoundTime = worldState->frameTime;
				}
			}
			
			//Stop checking
			//dynamics->colliding = true;
			break;
		}
#else
		if(Object_Collision(&ship->megaObject, &ship->megaObject.object.pos, 1.0, &shipList[i].megaObject, &shipList[i].megaObject.object.pos, 1.0)) {
			Vector		temp;
			
			//Switch velocities
			temp = ship->dynamics.velocityG;
			shipState->velocityG = shipList[i].dynamics.velocityG;
			Vector_Clear(&shipState->accelerationG);
			ship->dynamics.velocityG = shipList[i].dynamics.velocityG;
			shipList[i].dynamics.velocityG = temp;
			
			temp = ship->dynamics.rotationspeedL;
			shipState->rotationspeedL = shipList[i].dynamics.rotationspeedL;
			Vector_Clear(&shipState->rotationAccelerationL);
			ship->dynamics.rotationspeedL = shipList[i].dynamics.rotationspeedL;
			shipList[i].dynamics.rotationspeedL = temp;
			
			//Restore ship position
			*shipMatrix = backUpMatrix;
			
			//Play sound
			if((shipSpeed > kCollisionMinSpeedSound) && (time >= dynamics->collisionSoundTime)) {
				InfinityAudio_InstallSound_AmbientFX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Collision], nil, &ship->megaObject.object.pos.w, kFXMinDistance, kFXMaxDistance, (shipSpeed - kCollisionMinSpeedSound) / kCollisionMaxSpeedSound, kInfinityAudio_NormalPitch, nil);
				dynamics->collisionSoundTime = worldState->frameTime;
			}
			
			//Stop checking
			//dynamics->colliding = true;
			break;
		}
#endif
	}
#endif
	
	/********************* RECORD RACE ***********************/
	
#if __RECORD_RACE__
	//Record data
	if((ship->eventBuffer != nil) && (time % kReplayEventDistance == 0) && (ship->eventCount < kMaxReplayEvents)) {
		ReplayEventPtr		eventPtr = &ship->eventBuffer[ship->eventCount];
		
		eventPtr->timeStamp = time;
		eventPtr->mouseH = ship->mouseH * kShortMax;
		eventPtr->motorPower = ship->dynamics.motorPower / kPowerUnitToWatt;
		eventPtr->turboState = ship->turboState;
		eventPtr->position = shipMatrix->w;
		Quaternion_MatrixToQuaternion(shipMatrix, &eventPtr->orientation);
		
		++eventPtr;
		ship->eventCount += 1;
	}
#endif
}

void Physic_UpdateShip_Replay(ExtendedShipPtr ship)
{
	DynamicsPtr	   			dynamics = &ship->dynamics;
	ReplayEventPtr			prevEvent,
							nextEvent;
	MatrixPtr				shipMatrix = &((ObjectPtr) ship)->pos;
	float					dTime;
	Quaternion				localQuaternion;
	ReplayEventPtr			eventBuffer;
	
	//Save our position
	ship->lastPos = shipMatrix->w; //Fixme: incorrect for first call: should set to first event
	
	//Find event buffer
	eventBuffer = ship->eventBuffer;
	if((eventBuffer == nil) || (ship->eventCount < 4) || (eventBuffer[0].timeStamp > worldState->frameTime))
	return;
	
	//Find event
	while((eventBuffer[ship->lastEventNum + 1].timeStamp <= worldState->frameTime) && (ship->lastEventNum < ship->eventCount - 1))
	ship->lastEventNum += 1;
	if(ship->lastEventNum >= ship->eventCount - 1)
	return;
	
	prevEvent = &eventBuffer[ship->lastEventNum];
	nextEvent = &eventBuffer[ship->lastEventNum + 1];
	dTime = (float) (worldState->frameTime - prevEvent->timeStamp) / (float) (nextEvent->timeStamp - prevEvent->timeStamp);
	
	//Interpolate input
	ship->mouseH = Linear_Interpolation((float) prevEvent->mouseH, (float) nextEvent->mouseH, dTime) / kShortMax;
	//ship->mouseV = ?;
	//ship->turboButton = ?;
	
	//Update ship geometry
	Physic_UpdateShipGeometry(ship, prevEvent->turboState || nextEvent->turboState, !(prevEvent->turboState || nextEvent->turboState));
	
	//Update motor power
	ship->dynamics.motorPower = Linear_Interpolation(prevEvent->motorPower, nextEvent->motorPower, dTime) * kPowerUnitToWatt;
	Physic_UpdatePowerSound(ship);
	
	//Interpolate angles
	Quaternion_LerpInterpolation(&prevEvent->orientation, &nextEvent->orientation, dTime, &localQuaternion);
	localQuaternion.x = -localQuaternion.x;
	localQuaternion.y = -localQuaternion.y;
	localQuaternion.z = -localQuaternion.z;
	Quaternion_Normalize(&localQuaternion);
	Quaternion_QuaternionToMatrix(&localQuaternion, shipMatrix);
	
	//Interpolate position
	shipMatrix->w.x = Linear_Interpolation(prevEvent->position.x, nextEvent->position.x, dTime);
	shipMatrix->w.y = Linear_Interpolation(prevEvent->position.y, nextEvent->position.y, dTime);
	shipMatrix->w.z = Linear_Interpolation(prevEvent->position.z, nextEvent->position.z, dTime);
	
	/********************* UPDATE SHIP DATA ***********************/
	Vector_Subtract(&shipMatrix->w, &ship->lastPos, &ship->dynamics.velocityG);
	Vector_Multiply((float) kTimeUnit / (float) (worldState->frameTime - worldState->lastFrameTime), &ship->dynamics.velocityG, &ship->dynamics.velocityG);
	ship->player->speed = FVector_Length(ship->dynamics.velocityG);
}
