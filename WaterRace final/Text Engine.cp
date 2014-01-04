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


#include		"WaterRace.h"
#include		"Text Engine.h"

//CONSTANTES:

#define			kZoomFactor			3
#define			kSpace				1

enum {kLeft, kCentered, kRight};

//MACROS:

#define Mask16(s, m) ( ((((s) & 0x001F) * (m) / 255) & 0x001F) \
						+ ((((s) & 0x03E0) * (m) / 255) & 0x03E0) \
						+ ((((s) & 0x7C00) * (m) / 255) & 0x7C00) )

//VARIABLES:

static GWorldPtr				fontGWorld = nil;
static Rect						worldRect;
static long						maxHeight,
								fontAscent,
								vSpace;
static long						correction = 0,
								localTopClip = 0,
								localBottomClip = 0,
								localLeftMarging = 0,
								localRightMarging = 0;

//ROUTINES:

OSErr TextEngine_Init(short maxFontSize, short maxBufferWidth)
{
	OSErr				theError;
	GWorldPtr			oldGWorld;
	GDHandle			oldGDHandle;
	
	SetRect(&worldRect, 0, 0, maxBufferWidth * (kZoomFactor + 1), maxFontSize * (kZoomFactor + 1));
	theError = NewGWorld(&fontGWorld, 8, &worldRect, nil, nil, 0L);
	if(theError)
	return theError;
	NoPurgePixels(GetGWorldPixMap(fontGWorld));
	LockPixels(GetGWorldPixMap(fontGWorld));
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetGWorld(fontGWorld, nil);
	BackColor(whiteColor);
	ForeColor(blackColor);
	
	SetGWorld(oldGWorld, oldGDHandle);
	
	return noErr;
}

void TextEngine_Settings_Text(short fontNum, short fontSize, Style fontStyle)
{
	GWorldPtr			oldGWorld;
	GDHandle			oldGDHandle;
	FontInfo			info;
	
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetGWorld(fontGWorld, nil);
	
	TextFont(fontNum);
	TextSize(fontSize * kZoomFactor);
	TextFace(fontStyle);
	
	GetFontInfo(&info);
	maxHeight = (info.ascent + info.descent) / kZoomFactor;
	fontAscent = info.ascent;
	vSpace = (info.ascent + info.descent + info.leading) / kZoomFactor;
	
#if 0
	if(fontStyle & italic)
	correction = (float) info.widMax * 0.1;
	else
	correction = 0;
#else
	correction = (float) info.widMax * 0.1;
#endif
	
	SetGWorld(oldGWorld, oldGDHandle);
}

void TextEngine_Settings_Clipping(short topClip, short bottomClip)
{
	localTopClip = topClip;
	localBottomClip = bottomClip;
}

void TextEngine_Settings_Margings(short leftMargin, short rightMargin)
{
	localLeftMarging = leftMargin;
	localRightMarging = rightMargin;
}

short TextEngine_GetInterline()
{
	return vSpace;
}

inline void Draw_TextLine_Clipped(long textWidth, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint)
{
	unsigned short		*dest;
	unsigned char		*source;
	Ptr					srcAddress,
						destAddress;
	long				srcRowBytes,
						destRowBytes;
	long				h,
						v,
						size;
						
	srcRowBytes = (**(fontGWorld->portPixMap)).rowBytes & 0x3FFF;
	srcAddress = (**(fontGWorld->portPixMap)).baseAddr + srcRowBytes + 1;
	destRowBytes = thePixMapPtr->rowBytes & 0x3FFF;
	destAddress = thePixMapPtr->baseAddr + destRowBytes * destPoint.v + destPoint.h * 2;
	for(v = 0; v < maxHeight; ++v) {
		//Do we need to clip?
		if(destPoint.v + v < localTopClip) {
			srcAddress += kZoomFactor * srcRowBytes;
			destAddress += destRowBytes;
			continue;
		}
		if(destPoint.v + v > localBottomClip - 1)
		break;
		
		//Plot line
		source = (unsigned char*) srcAddress;
		dest = (unsigned short*) destAddress;
		for(h = 0; h < textWidth; ++h) {
			size = (*(source - srcRowBytes - 1) + *(source - srcRowBytes) + *(source - srcRowBytes + 1)
					+ *(source - 1) + *source + *(source + 1)
					+ *(source + srcRowBytes - 1) + *(source + srcRowBytes) + *(source + srcRowBytes + 1)
					) / 9;
			
			if(size >= 0xFF)
			*dest = color;
			else if(size == 0x00)
			;
			else
			*dest = Mask16(color, size) + Mask16(*dest, 0xFF - size);
			
			source += 3;
			++dest;
		}
		srcAddress += kZoomFactor * srcRowBytes;
		destAddress += destRowBytes;
	}
}

inline void Draw_TextLine_Clipped_WithFade(long textWidth, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint, long fadeHeight)
{
	unsigned short		*dest;
	unsigned char		*source;
	Ptr					srcAddress,
						destAddress;
	long				srcRowBytes,
						destRowBytes;
	long				h,
						v,
						size;
						
	srcRowBytes = (**(fontGWorld->portPixMap)).rowBytes & 0x3FFF;
	srcAddress = (**(fontGWorld->portPixMap)).baseAddr + srcRowBytes + 1;
	destRowBytes = thePixMapPtr->rowBytes & 0x3FFF;
	destAddress = thePixMapPtr->baseAddr + destRowBytes * destPoint.v + destPoint.h * 2;
	for(v = 0; v < maxHeight; ++v) {
		//Do we need to clip?
		if(destPoint.v + v < localTopClip) {
			srcAddress += kZoomFactor * srcRowBytes;
			destAddress += destRowBytes;
			continue;
		}
		if(destPoint.v + v > localBottomClip - 1)
		break;
		
		//Do we need to fade
		source = (unsigned char*) srcAddress;
		dest = (unsigned short*) destAddress;
		if(destPoint.v + v < localBottomClip - fadeHeight) {
			//Plot line
			for(h = 0; h < textWidth; ++h) {
				size = (*(source - srcRowBytes - 1) + *(source - srcRowBytes) + *(source - srcRowBytes + 1)
						+ *(source - 1) + *source + *(source + 1)
						+ *(source + srcRowBytes - 1) + *(source + srcRowBytes) + *(source + srcRowBytes + 1)
						) / 9;
				
				if(size >= 0xFF)
				*dest = color;
				else if(size == 0x00)
				;
				else
				*dest = Mask16(color, size) + Mask16(*dest, 0xFF - size);
				
				source += 3;
				++dest;
			}
		}
		else {
			//Plot line - with fade!
			float factor = 1.0 - (float) (destPoint.v + v - (localBottomClip - fadeHeight)) / (float) fadeHeight;
			for(h = 0; h < textWidth; ++h) {
				size = (*(source - srcRowBytes - 1) + *(source - srcRowBytes) + *(source - srcRowBytes + 1)
						+ *(source - 1) + *source + *(source + 1)
						+ *(source + srcRowBytes - 1) + *(source + srcRowBytes) + *(source + srcRowBytes + 1)
						) / 9;
				
				if(size)
				*dest = Mask16(color, (long) (size * factor)) + Mask16(*dest, 0xFF - (long) (size * factor));
				
				source += 3;
				++dest;
			}
		}
		srcAddress += kZoomFactor * srcRowBytes;
		destAddress += destRowBytes;
	}
}

void TextEngine_DrawLine_Left(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint)
{
	GWorldPtr			oldGWorld;
	GDHandle			oldGDHandle;
	long				textWidth;
						
	//Draw text in text buffer
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetGWorld(fontGWorld, nil);
	EraseRect(&worldRect);
	MoveTo(kZoomFactor * kSpace, kZoomFactor * kSpace + fontAscent);
	DrawText(textPtr, 0, textLength);
	textWidth = (TextWidth(textPtr, 0, textLength) + correction) / kZoomFactor;
	SetGWorld(oldGWorld, oldGDHandle);
	
	//Copy and anti-alias text to buffer
	Draw_TextLine_Clipped(textWidth, color, thePixMapPtr, destPoint);
}

void TextEngine_DrawLine_Centered(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint)
{
	GWorldPtr			oldGWorld;
	GDHandle			oldGDHandle;
	long				textWidth;
						
	//Draw text in text buffer
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetGWorld(fontGWorld, nil);
	EraseRect(&worldRect);
	MoveTo(kZoomFactor * kSpace, kZoomFactor * kSpace + fontAscent);
	DrawText(textPtr, 0, textLength);
	textWidth = (TextWidth(textPtr, 0, textLength) + correction) / kZoomFactor;
	SetGWorld(oldGWorld, oldGDHandle);
	
	//Adjust text position
	destPoint.h -= textWidth / 2;
	
	//Copy and anti-alias text to buffer
	Draw_TextLine_Clipped(textWidth, color, thePixMapPtr, destPoint);
}

void TextEngine_DrawLine_Right(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint)
{
	GWorldPtr			oldGWorld;
	GDHandle			oldGDHandle;
	long				textWidth;
						
	//Draw text in text buffer
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetGWorld(fontGWorld, nil);
	EraseRect(&worldRect);
	MoveTo(kZoomFactor * kSpace, kZoomFactor * kSpace + fontAscent);
	DrawText(textPtr, 0, textLength);
	textWidth = (TextWidth(textPtr, 0, textLength) + correction) / kZoomFactor;
	SetGWorld(oldGWorld, oldGDHandle);
	
	//Adjust text position
	destPoint.h -= textWidth;
	
	//Copy and anti-alias text to buffer
	Draw_TextLine_Clipped(textWidth, color, thePixMapPtr, destPoint);
}

void TextEngine_DrawText_Left(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint)
{
	unsigned char			*thePtr = (unsigned char*) textPtr,
							*tempPtr;
	long					numCharsLeft = textLength,
							numChars;
							
	while(numCharsLeft > 0) {
		tempPtr = thePtr;
		numChars = 0;
		while((*tempPtr != '\r') && (numChars < numCharsLeft)) {
			++tempPtr;
			++numChars;
		}
		
		TextEngine_DrawLine_Left((Ptr) thePtr, numChars, color, thePixMapPtr, destPoint);
		
		destPoint.v += vSpace;
		thePtr += numChars + 1;
		numCharsLeft -= numChars + 1;
	}
}

void TextEngine_DrawFormattedText(Ptr textPtr, short textLength, TE_FontSpec fontSpecs[], PixMapPtr thePixMapPtr, Point destPoint)
{
	unsigned char			*thePtr = (unsigned char*) textPtr,
							*tempPtr;
	long					numCharsLeft = textLength,
							numChars;
	unsigned short			curColor = 0xFFFF;
	long					curAlign = kLeft,
							specNum;
	GWorldPtr				oldGWorld;
	GDHandle				oldGDHandle;
	long					textWidth;
							
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetGWorld(fontGWorld, nil);
	
	while(numCharsLeft > 0) {
		//Check for tag
		if(*thePtr == '<') {
			//Read alignement
			++thePtr;
			switch(*thePtr) {
				case 'L': curAlign = kLeft; break;
				case 'C': curAlign = kCentered; break;
				case 'R': curAlign = kRight; break;
			}
			//Read text spec
			++thePtr;
			specNum = *thePtr - '1';
			TextEngine_Settings_Text(fontSpecs[specNum].fontNum, fontSpecs[specNum].fontSize, fontSpecs[specNum].fontStyle);
			curColor = fontSpecs[specNum].fontColor;
			//Skip close tag
			thePtr += 2;
			
			numCharsLeft -= 4;
		}
		
		//Extract line of text
		tempPtr = thePtr;
		numChars = 0;
		while((*tempPtr != '\r') && (numChars < numCharsLeft)) {
			++tempPtr;
			++numChars;
		}
		
		//Draw line of text
		if((destPoint.v > -maxHeight) && (destPoint.v < localBottomClip)) {
			//Draw text in text buffer
			EraseRect(&worldRect);
			MoveTo(kZoomFactor * kSpace, kZoomFactor * kSpace + fontAscent);
			DrawText((Ptr) thePtr, 0, numChars);
			textWidth = (TextWidth((Ptr) thePtr, 0, numChars) + correction) / kZoomFactor;
			
			//Calculate text position
			switch(curAlign) {
				case kLeft: destPoint.h = localLeftMarging; break;
				case kCentered: destPoint.h = (localLeftMarging + localRightMarging) / 2 - textWidth / 2; break;
				case kRight: destPoint.h = localRightMarging - textWidth; break;
			}
			
			//Copy and anti-alias text to buffer
			Draw_TextLine_Clipped(textWidth, curColor, thePixMapPtr, destPoint);
		}
		
		destPoint.v += vSpace;
		thePtr += numChars + 1;
		numCharsLeft -= numChars + 1;
	}
	
	SetGWorld(oldGWorld, oldGDHandle);
}

void TextEngine_DrawFormattedText_WithFade(Ptr textPtr, short textLength, TE_FontSpec fontSpecs[], PixMapPtr thePixMapPtr, Point destPoint, long fadeHeight)
{
	unsigned char			*thePtr = (unsigned char*) textPtr,
							*tempPtr;
	long					numCharsLeft = textLength,
							numChars;
	unsigned short			curColor = 0xFFFF;
	long					curAlign = kLeft,
							specNum;
	GWorldPtr				oldGWorld;
	GDHandle				oldGDHandle;
	long					textWidth;
							
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetGWorld(fontGWorld, nil);
	
	while(numCharsLeft > 0) {
		//Check for tag
		if(*thePtr == '<') {
			//Read alignement
			++thePtr;
			switch(*thePtr) {
				case 'L': curAlign = kLeft; break;
				case 'C': curAlign = kCentered; break;
				case 'R': curAlign = kRight; break;
			}
			//Read text spec
			++thePtr;
			specNum = *thePtr - '1';
			TextEngine_Settings_Text(fontSpecs[specNum].fontNum, fontSpecs[specNum].fontSize, fontSpecs[specNum].fontStyle);
			curColor = fontSpecs[specNum].fontColor;
			//Skip close tag
			thePtr += 2;
			
			numCharsLeft -= 4;
		}
		
		//Extract line of text
		tempPtr = thePtr;
		numChars = 0;
		while((*tempPtr != '\r') && (numChars < numCharsLeft)) {
			++tempPtr;
			++numChars;
		}
		
		//Draw line of text
		if((destPoint.v > -maxHeight) && (destPoint.v < localBottomClip)) {
			//Draw text in text buffer
			EraseRect(&worldRect);
			MoveTo(kZoomFactor * kSpace, kZoomFactor * kSpace + fontAscent);
			DrawText((Ptr) thePtr, 0, numChars);
			textWidth = (TextWidth((Ptr) thePtr, 0, numChars) + correction) / kZoomFactor;
			
			//Calculate text position
			switch(curAlign) {
				case kLeft: destPoint.h = localLeftMarging; break;
				case kCentered: destPoint.h = (localLeftMarging + localRightMarging) / 2 - textWidth / 2; break;
				case kRight: destPoint.h = localRightMarging - textWidth; break;
			}
			
			//Copy and anti-alias text to buffer
			if(destPoint.v < localBottomClip - fadeHeight)
			Draw_TextLine_Clipped(textWidth, curColor, thePixMapPtr, destPoint);
			else
			Draw_TextLine_Clipped_WithFade(textWidth, curColor, thePixMapPtr, destPoint, fadeHeight);
		}
		
		destPoint.v += vSpace;
		thePtr += numChars + 1;
		numCharsLeft -= numChars + 1;
	}
	
	SetGWorld(oldGWorld, oldGDHandle);
}

OSErr TextEngine_Quit()
{
	if(fontGWorld == nil)
	return noErr;
	
	UnlockPixels(GetGWorldPixMap(fontGWorld));
	DisposeGWorld(fontGWorld);

	return noErr;
}
