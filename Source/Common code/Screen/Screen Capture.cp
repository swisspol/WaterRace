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


#include				"Screen Capture.h"

//CONSTANTES:

#define					kPICTFileOffset				512

//MACROS:

#define	GWBitMapPtr(w) &(((GrafPtr)(w))->portBits)

//ROUTINES:

PicHandle Capture_VRAM(GDHandle device, Rect* copyRect, Rect* finalRect)
{
	PicHandle			thePic = nil;
	GWorldPtr			oldGWorld;
	GDHandle			oldGDHandle;
	GWorldPtr			tempGWorld;
	Rect				tempRect;
	PixMapPtr			sourcePixMap,
						destPixMap;
	Ptr					sourceBaseAddress,
						destBaseAddress;
	long				height,
						sourceRowBytes,
						destRowBytes;
	
	//Create temp GWorld
	GetGWorld(&oldGWorld, &oldGDHandle);
	tempRect = *copyRect;
	OffsetRect(&tempRect, -tempRect.left, -tempRect.top);
	sourcePixMap = *((**device).gdPMap);
	if(NewGWorld(&tempGWorld, sourcePixMap->pixelSize, &tempRect, NULL, NULL, useTempMem))
	return nil;
	SetGWorld(tempGWorld, nil);
	LockPixels(GetGWorldPixMap(tempGWorld));
	BackColor(whiteColor);
	ForeColor(blackColor);
	destPixMap = *(tempGWorld->portPixMap);
	
	//Copy VRAM data
	sourceRowBytes = sourcePixMap->rowBytes & 0x3FFF;
	destRowBytes = destPixMap->rowBytes & 0x3FFF;
	sourceBaseAddress = sourcePixMap->baseAddr + copyRect->top * sourceRowBytes + copyRect->left * sourcePixMap->pixelSize / 8;
	destBaseAddress = destPixMap->baseAddr;
	height = copyRect->bottom - copyRect->top;
	do {
		BlockMove(sourceBaseAddress, destBaseAddress, (copyRect->right - copyRect->left) * sourcePixMap->pixelSize / 8);
		sourceBaseAddress += sourceRowBytes;
		destBaseAddress += destRowBytes;
	} while(--height);
	
	//Resize picture
	if(finalRect->top != -1)
	CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(tempGWorld), &tempRect, finalRect, srcCopy, nil);
	else
	*finalRect = tempRect;
	
	//Create picture
	thePic = OpenPicture(&tempRect);
	CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(tempGWorld), finalRect, finalRect, srcCopy, nil);
	if(QDError()) {
		ClosePicture();
		DisposeHandle((Handle) thePic);
		thePic = nil;
		SysBeep(0);
	}
	else {
		ClosePicture();
		(**thePic).picFrame = *finalRect;
	}
	
	//Clean up
	UnlockPixels(GetGWorldPixMap(tempGWorld));
	DisposeGWorld(tempGWorld);
	SetGWorld(oldGWorld, oldGDHandle);
	
	if(GetHandleSize((Handle) thePic) <= sizeof(Picture)) {
		DisposeHandle((Handle) thePic);
		thePic = nil;
		SysBeep(0);
	}
	
	return thePic;
}

OSErr Capture_WritePICTFile(PicHandle pic, FSSpec* destFile)
{
	OSErr					theError;
	short					destFileID;
	long					bytes;
	
	theError = FSpCreate(destFile, kScreenShot_Creator, kScreenShot_Type, smSystemScript);
	if(theError)
	return theError;
	
	theError = FSpOpenDF(destFile, fsRdWrPerm, &destFileID);
	if(theError)
	return theError;
	SetEOF(destFileID, kPICTFileOffset);
	SetFPos(destFileID, fsFromStart, kPICTFileOffset);
	bytes = GetHandleSize((Handle) pic);
	HLock((Handle) pic);
	FSWrite(destFileID, &bytes, *pic);
	HUnlock((Handle) pic);
	
	FSClose(destFileID);
	
	return noErr;
}
