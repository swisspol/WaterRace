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

#include				"Infinity Structures.h"
#include				"Infinity rendering.h"
#include				"Textures.h"
#include				"Infinity Post FX.h"

#include				"Infinity Utils.h"
#include				"Clipping Utils.h"
#include				"Infinity Error codes.h"

//CONSTANTES:

#define					kTextBigPoliceID		7400
#define					kTextSmallPoliceID		7410
#define					kTextTableID			7400
#define					kPoliceTableResType		'PlTb'

//Small police
#define					kTextSmall_TileSizeH	18
#define					kTextSmall_TileSizeV	20
#define					kTextSmall_HSpace		15
#define					kTextSmall_VSpace		20
#define					kTextSmall_CharWidth	((float) kTextSmall_TileSizeH / 256.0)
#define					kTextSmall_CharHeight	((float) kTextSmall_TileSizeV / 128.0)

//Big police
#define					kTextBig_TileSizeH		22
#define					kTextBig_TileSizeV		25
#define					kTextBig_HSpace			22
#define					kTextBig_VSpace			30
#define					kTextBig_CharWidth		((float) kTextBig_TileSizeH / 256.0)
#define					kTextBig_CharHeight		((float) kTextBig_TileSizeV / 128.0)

#define					kCharactersPerLine		11
#define					kSpaceChar				' '
#define					kTextDepth				0.001

//CONSTANTES PREPROCESSEURS:

#define	__CLEAN_SPECULAR__	1
#define	__CHECK_X_Y_VISIBILITY__	1
#define	__TURN_OFF_Z_BUFFER__	0
#define	__XY_CLIPPING__	1

//STRUTURES LOCALES:

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct PoliceCharacter
{
	unsigned char		character;
	char				nullByte;
	short				unused;
};
typedef PoliceCharacter* PoliceCharacterPtr;

typedef struct PoliceTable
{
	short				entryCount;
	PoliceCharacter		entryList[];
};
typedef PoliceTable** PoliceTableHandle;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

//VARIABLES LOCALES:

static TQATexture*			textTexture;
static long					currentTextSize = 0;
static PoliceTableHandle	policeTable;

//ROUTINES:

OSErr PostFX_Text_Init(StatePtr state, long textureFlags, long textSize)
{
	OSErr					theError;
	
	if(currentTextSize)
	return kError_FatalError;
	
	if((textureFlags & kTextureFlag_Reduce4X) || (textureFlags & kTextureFlag_Reduce16X)) {
		textureFlags &= ~(kTextureFlag_Reduce4X + kTextureFlag_Reduce16X);
		textureFlags |= kTextureFlag_Compress;
	}
	textureFlags |= kTextureFlag_LowQuality;
	
	if(textSize == kTextSize_Small)
	theError = Texture_NewARGB32TextureFromPictResource(state->engine, &textTexture, kTextSmallPoliceID, textureFlags);
	else if(textSize == kTextSize_Big)
	theError = Texture_NewARGB32TextureFromPictResource(state->engine, &textTexture, kTextBigPoliceID, textureFlags);
	else
	return kError_FatalError;
	if(theError)
	return theError;
	
	policeTable = (PoliceTableHandle) GetResource(kPoliceTableResType, kTextTableID);
	if(policeTable == nil) {
		QATextureDelete(state->engine, textTexture);
		return ResError();
	}
	DetachResource((Handle) policeTable);
	HLock((Handle) policeTable);
	
	currentTextSize = textSize;
	
	return noErr;
}

void PostFX_Text_Quit(StatePtr state)
{
	if(!currentTextSize)
	return;
	
	QATextureDelete(state->engine, textTexture);
	DisposeHandle((Handle) policeTable);
	
	currentTextSize = 0;
}

void PostFX_Text_Display(StatePtr state, long length, unsigned char* text, VectorPtr position, long mode, float r, float g, float b)
{
	long				num;
	unsigned char		*tableChar,
						localText;
	long				verticeCount = 0,
						triangleCount = 0;
	TQAVTexture			*texturePtr = vTextureBuffer;
	TQAIndexedTriangle	*trianglePtr = trianglesBuffer;
	float				top, left, right, bottom,
						topV, leftU, rightU, bottomV;
	float				viewWidth = state->viewWidth - boundrySlop,
						viewHeight = state->viewHeight - boundrySlop;
	long				index;
	float				x = (long) position->x,
						y = (long) position->y,
						z = kTextDepth,
						iw = 1.0;
	
	if(!currentTextSize)
	return;
	
	if(mode == kTextMode_Fake_NewLine) {
		if(currentTextSize == kTextSize_Small)
		position->y += kTextSmall_VSpace;
		else
		position->y += kTextBig_VSpace;
		return;
	}
	
	if(mode == kTextMode_Space) {
		if(currentTextSize == kTextSize_Small)
		position->x += kTextSmall_HSpace * length;
		else
		position->x += kTextBig_HSpace * length;
		return;
	}
	
	if(currentTextSize == kTextSize_Small) {
		if(mode == kTextMode_Right)
		x -= kTextSmall_HSpace * length;
		else if(mode == kTextMode_Centered)
		x -= kTextSmall_HSpace * length / 2.0;
	}
	else {
		if(mode == kTextMode_Right)
		x -= kTextBig_HSpace * length;
		else if(mode == kTextMode_Centered)
		x -= kTextBig_HSpace * length / 2.0;
	}
		
	//Round to integer
	x = (long) x;
	y = (long) y;
	
	while(length--) {
		//Get character
		localText = *((unsigned char*) text);
		
		//Check if we have a space char
		if(localText == kSpaceChar) {
			if(currentTextSize == kTextSize_Small)
			x += kTextSmall_HSpace;
			else
			x += kTextBig_HSpace;
			++text;
			continue;
		}
		
		//Map lower to capitals
		if((localText >= 'a') && (localText <= 'z'))
		localText = localText - 'a' + 'A';
		
		//Find the char in the table - if any
		num = (**policeTable).entryCount;
		tableChar = &(**policeTable).entryList[0].character;
		index = -1;
		while(num--) {
			if(*tableChar == localText) {
				index = (**policeTable).entryCount - num - 1;
				break;
			}
			tableChar += sizeof(PoliceCharacter);
		}
		if(index < 0) {
			++text;
			continue;
		}
		
		//Compute position
		left = x;
		top = y;
		if(currentTextSize == kTextSize_Small) {
			right = x + kTextSmall_TileSizeH;
			bottom = y + kTextSmall_TileSizeV;
		}
		else {
			right = x + kTextBig_TileSizeV;
			bottom = y + kTextBig_TileSizeV;
		}
		
#if __CHECK_X_Y_VISIBILITY__
		//Check X-Y visibility
		if((right < boundrySlop) || (left > viewWidth)
		 || (bottom < boundrySlop) || (top > viewHeight)) {
			++text;
			continue;
		}
#endif
		
		//Compute texture
		if(currentTextSize == kTextSize_Small) {
			leftU = (index % kCharactersPerLine) * kTextSmall_CharWidth;
			rightU = leftU + kTextSmall_CharWidth;
			topV = 1.0 - (index / kCharactersPerLine) * kTextSmall_CharHeight;
			bottomV = topV - kTextSmall_CharHeight;
			
#if __XY_CLIPPING__
			//Clip left
			if(left < boundrySlop) {
				leftU = leftU + kTextSmall_CharWidth * (boundrySlop - left) / kTextSmall_TileSizeH;
				left = boundrySlop;
			}
			
			//Clip right
			if(right > viewWidth) {
				rightU = rightU - kTextSmall_CharWidth * (right - viewWidth) / kTextSmall_TileSizeH;
				right = viewWidth;
			}
			
			//Clip top
			if(top < boundrySlop) {
				topV = topV - kTextSmall_CharHeight * (boundrySlop - top) / kTextSmall_TileSizeV;
				top = boundrySlop;
			}
			
			//Clip bottom
			if(bottom > viewHeight) {
				bottomV = bottomV + kTextSmall_CharHeight * (bottom - viewHeight) / kTextSmall_TileSizeV;
				bottom = viewHeight;
			}
#endif
		}
		else {
			leftU = (index % kCharactersPerLine) * kTextBig_CharWidth;
			rightU = leftU + kTextBig_CharWidth;
			topV = 1.0 - (index / kCharactersPerLine) * kTextBig_CharHeight;
			bottomV = topV - kTextBig_CharHeight;
			
#if __XY_CLIPPING__
			//Clip left
			if(left < boundrySlop) {
				leftU = leftU + kTextSmall_CharWidth * (boundrySlop - left) / kTextBig_TileSizeH;
				left = boundrySlop;
			}
			
			//Clip right
			if(right > viewWidth) {
				rightU = rightU - kTextSmall_CharWidth * (right - viewWidth) / kTextBig_TileSizeH;
				right = viewWidth;
			}
			
			//Clip top
			if(top < boundrySlop) {
				topV = topV - kTextBig_CharHeight * (boundrySlop - top) / kTextBig_TileSizeV;
				top = boundrySlop;
			}
			
			//Clip bottom
			if(bottom > viewHeight) {
				bottomV = bottomV + kTextBig_CharHeight * (bottom - viewHeight) / kTextBig_TileSizeV;
				bottom = viewHeight;
			}
#endif
		}
		
		//Convert texture coordinates
#if 0
		leftU *= iw;
		rightU *= iw;
		topV *= iw;
		bottomV *= iw;
#endif
		
		//Create top-left vertex
		texturePtr->uOverW = leftU;
		texturePtr->vOverW = topV;
		texturePtr->kd_r = r;
		texturePtr->kd_g = g;
		texturePtr->kd_b = b;
#if __CLEAN_SPECULAR__
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
		texturePtr->a = 1.0;
		texturePtr->x = left;
		texturePtr->y = top;
		texturePtr->z = z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create top-right vertex
		texturePtr->uOverW = rightU;
		texturePtr->vOverW = topV;
		texturePtr->kd_r = r;
		texturePtr->kd_g = g;
		texturePtr->kd_b = b;
#if __CLEAN_SPECULAR__
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
		texturePtr->a = 1.0;
		texturePtr->x = right;
		texturePtr->y = top;
		texturePtr->z = z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create bottom-right vertex
		texturePtr->uOverW = rightU;
		texturePtr->vOverW = bottomV;
		texturePtr->kd_r = r;
		texturePtr->kd_g = g;
		texturePtr->kd_b = b;
#if __CLEAN_SPECULAR__
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
		texturePtr->a = 1.0;
		texturePtr->x = right;
		texturePtr->y = bottom;
		texturePtr->z = z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create bottom-left vertex
		texturePtr->uOverW = leftU;
		texturePtr->vOverW = bottomV;
		texturePtr->kd_r = r;
		texturePtr->kd_g = g;
		texturePtr->kd_b = b;
#if __CLEAN_SPECULAR__
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
		texturePtr->a = 1.0;
		texturePtr->x = left;
		texturePtr->y = bottom;
		texturePtr->z = z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create triangle #1
		trianglePtr->triangleFlags = 0;
		trianglePtr->vertices[0] = verticeCount;
		trianglePtr->vertices[1] = verticeCount + 1;
		trianglePtr->vertices[2] = verticeCount + 2;
		++trianglePtr;
		
		//Create triangle #2
		trianglePtr->triangleFlags = 0;
		trianglePtr->vertices[0] = verticeCount + 2;
		trianglePtr->vertices[1] = verticeCount + 3;
		trianglePtr->vertices[2] = verticeCount;
		++trianglePtr;
		
		//Update counters
		verticeCount += 4;
		triangleCount += 2;
		
		//Get next character
		text++;
		
		//Update position
		if(currentTextSize == kTextSize_Small)
		x += kTextSmall_HSpace;
		else
		x += kTextBig_HSpace;
	}
	
	//Check for triangles
	if(triangleCount == 0)
	return;
	
	//Set texture
	if(textTexture != state->lastTexture) {
		QASetPtr(state->drawContext, kQATag_Texture, textTexture);
		state->lastTexture = textTexture;
	}
	
	//Set texture method
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate);
	
	//Render
#if __TURN_OFF_Z_BUFFER__
	QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_None);
#endif
	QASubmitVerticesTexture(state->drawContext, verticeCount, vTextureBuffer);
	QADrawTriMeshTexture(state->drawContext, triangleCount, trianglesBuffer);
#if __TURN_OFF_Z_BUFFER__
	QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_LT);
#endif

	position->x = x;
	position->y = y;
	position->z = kTextDepth;
}
