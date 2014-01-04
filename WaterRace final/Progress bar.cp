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


#include			"WaterRace.h"
#include			"Data Files.h"
#include			"Interface.h"
#include			"Preferences.h"

//CONSTANTES:

#define				kEmptyBarID				5000
#define				kBar1ID					5001
#define				kBar2ID					5002
#define				kBar3ID					5003
#define				kFakeTerrainPreviewID	5020
#define				kMapID					5010
#define				kMapAlphaID				5011

#define				kAlphaGWorldSizeH		500
#define				kAlphaGWorldSizeV		200

#define				kMapVOffset				20

#define				kBarHeight				56
#define				kBarWidth				81

#define				kTargetID				5012
#define				kTargetAlphaID			5013
#define				kTargetOffsetH			(34 - 3)
#define				kTargetOffsetV			(17 - 3)

//ROUTINES:

OSErr Progress_Init(FSSpec* theFile, short location_H, short location_V)
{
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	Rect					theRect,
							alphaRect,
							copyRect,
							destRect;
	PixMapHandle			pictPix;
	CTabHandle				theCLUT;
	OSErr					theError;
	
	UseResFile(interfaceResFileID);
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	
	//Draw preview image
	if(Terrain_ExtractPreview(theFile, &backgroundPic) != noErr) {
#if !__DEMO_MODE__
		backgroundPic = (PicHandle) Get1Resource(kPICTResType, kFakeTerrainPreviewID);
		if(ResError())
		return ResError();
		DetachResource((Handle) backgroundPic);
#endif
	}
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	OffsetRect(&theRect, 0, kInterface_Height - theRect.bottom);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Draw empty bar
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kEmptyBarID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
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
	
	//Draw map
	SetGWorld(bufferGWorld2, NULL);
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kMapID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Draw target
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kTargetID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	destRect = theRect;
	OffsetRect(&theRect, 0, kInterface_Height - theRect.bottom);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Draw alpha-target
	SetGWorld(alphaGWorld, NULL);
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kTargetAlphaID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	alphaRect = (**backgroundPic).picFrame;
	OffsetRect(&alphaRect, -alphaRect.left, -alphaRect.top);
	DrawPicture(backgroundPic, &alphaRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Copy target
	OffsetRect(&destRect, location_H + kTargetOffsetH, location_V + kTargetOffsetV);
	CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld2), &alphaRect, &theRect, &destRect);
	
	//Draw alpha-map
	SetGWorld(alphaGWorld, NULL);
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kMapAlphaID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	alphaRect = theRect;
	
	//Copy map
	SetGWorld(bufferGWorld2, NULL);
	theRect.left = (kInterface_Width - alphaRect.right) / 2;
	theRect.right = theRect.left + alphaRect.right;
	theRect.bottom = kInterface_Height - kMapVOffset;
	theRect.top = theRect.bottom - alphaRect.bottom;
	CopyBits_16WithAlpha(GetWindowPixMapPtr(alphaGWorld), GetWindowPixMapPtr(bufferGWorld2), GetWindowPixMapPtr(bufferGWorld), &alphaRect, &alphaRect, &theRect);
	
	//Draw bar #1
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kBar1ID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Draw bar #2
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kBar2ID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top + kBarHeight);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	//Draw bar #3
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kBar3ID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top + 2 * kBarHeight);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	
	UnlockPixels(GetGWorldPixMap(alphaGWorld));
	DisposeGWorld(alphaGWorld);
	
	//Frame rect
	SetGWorld(bufferGWorld, NULL);
	SetRect(&copyRect, 0, 0, kInterface_Width, kInterface_Height);
	ForeColor(whiteColor);
	FrameRect(&copyRect);
	ForeColor(blackColor);
	
	//Copy back
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
	
	return noErr;
}

void Progress_Quit()
{
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	
	GetGWorld(&savePort, &saveDevice);
	
	SetGWorld((CGrafPtr) mainWin, NULL);
	PaintRect(&mainWin->portRect);
	
	SetGWorld(savePort, saveDevice);
}

void Progress_Update(short value, short max)
{
	Rect					copyRect,
							destRect;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	
	GetGWorld(&savePort, &saveDevice);
	
	//Update bar
	copyRect.left = 0;
	copyRect.right = kBarWidth;
	copyRect.top = (value - 1) * kBarHeight;
	copyRect.bottom = value * kBarHeight;
	destRect.left = kInterface_Width - kBarWidth;
	destRect.right = kInterface_Width;
	destRect.top = 0;
	destRect.bottom = kBarHeight;
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &copyRect, &destRect, srcCopy, nil);
	
	//Copy back
	SetGWorld(bufferGWorld, NULL);
	SetRect(&copyRect, 0, 0, kInterface_Width, kInterface_Height);
	ForeColor(whiteColor);
	FrameRect(&copyRect);
	ForeColor(blackColor);
	copyRect.bottom = kBarHeight;
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &copyRect, &copyRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
}
