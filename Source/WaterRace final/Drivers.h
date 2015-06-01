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


#ifndef __WATERRACE_DRIVERS__
#define __WATERRACE_DRIVERS__

//CONSTANTES:

enum {kCameraMode_Follow, kCameraMode_Head, kCameraMode_OnBoard, kCameraMode_Chase, kCameraMode_TV, kCameraMode_FlyThrough, kCameraMode_Track, kCameraMode_Fake};

//PROTOTYPES:

typedef void (*CameraDriver_Proc)();
typedef void (*ShipDriver_Proc)(ExtendedShipPtr ship);
typedef void (*CameraTextDriver_Proc)(StatePtr state);

//VARIABLES:

extern CameraDriver_Proc		cameraDriver;
extern CameraTextDriver_Proc	cameraTextDriver;
extern long						cameraMode;
extern ExtendedShipPtr			cameraTargetShip;
extern long						onboardCameraNum;

//File: Drivers Camera.cp
void DriverCamera_Follow_Init();
void DriverCamera_Follow_Callback();
void DriverCamera_PilotHead_Init();
void DriverCamera_PilotHead_Callback();
void DriverCamera_OnBoard_Init();
void DriverCamera_OnBoard_Callback();
void DriverCamera_TV_Init();
void DriverCamera_TV_Callback();
void DriverCamera_FlyThrough_Init();
void DriverCamera_FlyThrough_Callback();
void DriverCamera_Chase_Init();
void DriverCamera_Chase_Callback();
void DriverCamera_Fake_Init();
void DriverCamera_Fake_Callback();
void DriverCamera_Track_Init();
void DriverCamera_Track_Callback();

//File: Drivers Ship.cp
void DriverShip_PlayerISp_Init(ExtendedShipPtr ship);
void DriverShip_PlayerISp_Callback(ExtendedShipPtr ship);
void DriverShip_Bot_Init(ExtendedShipPtr ship);
void DriverShip_Bot_Callback(ExtendedShipPtr ship);
void DriverShip_Remote_Init(ExtendedShipPtr ship);
void DriverShip_Remote_Callback(ExtendedShipPtr ship);
void DriverShip_Replay_Init(ExtendedShipPtr ship);
void DriverShip_Replay_Callback(ExtendedShipPtr ship);
#if __AUTOPILOT__
void DriverShip_AutoPilot_Init(ExtendedShipPtr ship);
void DriverShip_AutoPilot_Callback(ExtendedShipPtr ship);
#endif

#endif
