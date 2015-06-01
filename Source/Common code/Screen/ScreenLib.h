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


#ifndef __SCREEN_LIB__
#define __SCREEN_LIB__

//CONSTANTES:

#define					kMaxModes					128
#define					kMaxPerfectMatchs			10

//ScreenLib Errors
enum {
	kError_NoMatchFound = 1000
};

//STRUCTURES:

//STRUCTURES

typedef struct DepthInfo_Definition
{
	short					depth;
	Boolean					valid;
	VDSwitchInfoRec			switchInfo;
};
typedef DepthInfo_Definition DepthInfo;
typedef DepthInfo_Definition* DepthInfoPtr;

typedef struct ModeInfo_Definition
{
	Str63					modeName;
	Rect					resolution;
	Fixed					hRes,
							vRes;
	long					depthCount;
	DepthInfo				depthList[];
};
typedef ModeInfo_Definition ModeInfo;
typedef ModeInfo_Definition* ModeInfoPtr;

typedef struct GDeviceInfo_Definition
{
	GDHandle				gDevice;
	DisplayIDType			id;
	long					modeCount;
	ModeInfoPtr				modeList[kMaxModes];
};
typedef GDeviceInfo_Definition GDeviceInfo;
typedef GDeviceInfo_Definition* GDeviceInfoPtr;

//ROUTINES:

OSErr SL_GetCurrent(GDHandle device, VDSwitchInfoRec* switchInfo);
Boolean SL_ModeAvailable(GDHandle device, VDSwitchInfoRec* switchInfo);
OSErr SL_Switch(GDHandle device, VDSwitchInfoRec* switchInfo);
OSErr SL_GetDeviceResolution(GDeviceInfoPtr g, short* width, short* height, short depth, VDSwitchInfoRec** switchInfo, Str255 message);
OSErr SL_ScanGDevice(GDHandle theGDevice, GDeviceInfoPtr* theInfo);
OSErr SL_DisposeGDeviceInfo(GDeviceInfoPtr info);

OSErr SL_LookForResolution(GDHandle device, short* width, short* height, short depth, VDSwitchInfoRec* switchInfo);
OSErr SL_GetMaxResolution(GDHandle device, short* width, short* height, short depth);

DisplayIDType SL_PickUpScreen(Str255 message);

#endif
