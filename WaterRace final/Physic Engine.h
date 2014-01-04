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


#ifndef __WATERRACE_PHYSIC_ENGINE__
#define __WATERRACE_PHYSIC_ENGINE__

//CONSTANTES:

#define				kVolumeCount	4

enum {kStatus_Indefinite = -1, kStatus_UnderWater = 0, kStatus_OnWater, kStatus_InAir};

//MACROS:

#if __UNIT_SYSTEM__ == kUnit_Meters
#define	UnitToDistance(v)	((v) / 1000.0) //m - > km
#define UnitToSpeed(v)		((v) * 3.6) //m.s -> km/h
#elif __UNIT_SYSTEM__ == kUnit_Miles
#define	UnitToDistance(v)	((v) / 1610.0) //m -> miles
#define UnitToSpeed(v)		((v) * 2.236) //m.s -> mph
#endif

//STRUCTURES:

typedef struct Volume {
	Vector			boundingBox[kBBSize];
	Vector			center,
					submergenceCenter;
	float			mass,
					volume;
	float			frontalArea,
					floorArea,
					lateralArea;
	float			archimedeCorrectionFactor;
	
	float			submergence;
};
typedef Volume* VolumePtr;

typedef struct Dynamics
{
	Vector			velocityG,
					accelerationG,
					rotationspeedL,
					rotationAccelerationL;
				
	float			targetPower,
					motorPower;
	
	short			status;
	long			upsideDownTime,
					underWaterTime,
					collisionSoundTime;
	Boolean			colliding;
	
	Volume			volumeList[kVolumeCount];
};
typedef Dynamics* DynamicsPtr;

//PROTOTYPES:

//File: Physic Engine.cp
void Physic_InitShip(ExtendedShipPtr ship); 

void Physic_StopShipTurbo(ExtendedShipPtr ship);
void Physic_DropShip(ExtendedShipPtr ship, long doorNum, Str31 text);

void Physic_UpdateShip_Start(ExtendedShipPtr ship);
void Physic_UpdateShip(ExtendedShipPtr ship, unsigned long time);
void Physic_UpdateShip_End(ExtendedShipPtr ship);

void Physic_UpdateShip_Network_Start(ExtendedShipPtr ship);
void Physic_UpdateShip_Network(ExtendedShipPtr ship, unsigned long time);
void Physic_UpdateShip_Network_End(ExtendedShipPtr ship);

void Physic_UpdateShip_Replay(ExtendedShipPtr ship);

#endif
