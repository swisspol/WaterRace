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
#include				"Data Files.h"
#include				"Interface.h"
#include				"Preferences.h"

#include				"Clock.h"
#include				"Infinity Audio Engine.h"

#include				"Sound Utils.h"

//CONSTANTES:

#define					kMaxPubDelay			1000

#define					kPubWaterRaceID			10000
#define					kPubFusionID			10001
#define					kPubSoundID				2000

//ROUTINES:

OSErr PubWaterRace_Display()
{
	PicHandle				backgroundPic;
	Rect					theRect,
							screenRect;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	Boolean					run = true;
	KeyMap					theKeys;
	Handle					theSound;
	unsigned long			startTime;
	
#if __SCREEN_FADES__
	//Fade in...
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
	UseResFile(mainResFileID);
	
	//Load graphics
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kPubWaterRaceID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	
	//Setup interface
	SetPort(mainWin);
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld2, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	SetRect(&screenRect, 0, 0, kInterface_Width, kInterface_Height);
	PaintRect(&screenRect);
	DrawPicture(backgroundPic, &theRect);
	
	//Load sound
	Sound_ResLoad(kPubSoundID, false, &theSound);
	
	//Display buffer
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &screenRect, &screenRect, srcCopy, nil);
	
	//Play sound
	Sound_PlaySFxHandle(theSound);
	
#if __SCREEN_FADES__
	//Fade out...
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	startTime = RClock_GetAbsoluteTime();
	while(run) {
		//Did the user press a key
		GetKeys(theKeys);
		if(theKeys[0] || theKeys[1] || theKeys[2] || theKeys[3])
		run = false;
		//Did the user press mouse button
		if(Button())
		run = false;
		//Did we ran out of time
		if(RClock_GetAbsoluteTime() - startTime > kMaxPubDelay)
		run = false;
	}
	
	//Clean up
	SetGWorld(savePort, saveDevice);
	DisposeHandle((Handle) backgroundPic);
	InfinityAudio_CleanUp(kCreatorType);
	DisposeHandle(theSound);
	
	return noErr;
}

OSErr PubFusion_Display()
{
	PicHandle				backgroundPic;
	Rect					theRect,
							screenRect;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	Boolean					run = true;
	KeyMap					theKeys;
	unsigned long			startTime;
	
#if __SCREEN_FADES__
	//Fade in...
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
	UseResFile(mainResFileID);
	
	//Load graphics
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kPubFusionID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	
	//Setup interface
	SetPort(mainWin);
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld2, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	SetRect(&screenRect, 0, 0, kInterface_Width, kInterface_Height);
	PaintRect(&screenRect);
	DrawPicture(backgroundPic, &theRect);
	
	//Display buffer
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &screenRect, &screenRect, srcCopy, nil);
	
#if __SCREEN_FADES__
	//Fade out...
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	startTime = RClock_GetAbsoluteTime();
	while(run) {
		//Did the user press a key
		GetKeys(theKeys);
		if(theKeys[0] || theKeys[1] || theKeys[2] || theKeys[3])
		run = false;
		//Did the user press mouse button
		if(Button())
		run = false;
		//Did we ran out of time
		if(RClock_GetAbsoluteTime() - startTime > kMaxPubDelay)
		run = false;
	}
	
	//Clean up
	SetGWorld(savePort, saveDevice);
	DisposeHandle((Handle) backgroundPic);
	
	return noErr;
}
