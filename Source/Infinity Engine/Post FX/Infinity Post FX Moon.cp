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
#include				"Vector.h"

#include				"Infinity Error codes.h"

//CONSTANTES:

#define					kMoonSize				0.10
#define					kMoonHeight				0.0

#define					kMoonID					7100

//VARIABLES LOCALES:

static TQATexture*		moonTexture;
static Boolean			fxInited = false;

//ROUTINES:

OSErr PostFX_Moon_Init(StatePtr state, long textureFlags)
{
	OSErr				theError;
	
	if(fxInited)
	return kError_FatalError;
	
	theError = Texture_NewARGB32TextureFromPictResource(state->engine, &moonTexture, kMoonID, textureFlags | kTextureFlag_LowQuality);
	if(theError)
	return theError;
	
	fxInited = true;
	
	return noErr;
}

void PostFX_Moon_Quit(StatePtr state)
{
	if(!fxInited)
	return;
	
	QATextureDelete(state->engine, moonTexture);
	
	fxInited = false;
}

void PostFX_Moon_Display(StatePtr state, float skyRadius, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	Vector				skyTop,
						moonPos;
	float				iw;
						
	if(!fxInited)
	return;
	
	//Get moon position
	Vector_Normalize(&state->lightVector, &moonPos);
	Vector_Multiply(skyRadius, &moonPos, &moonPos);
	Vector_Add(cameraPos, &moonPos, &skyTop);
	skyTop.y = kMoonHeight + moonPos.y;
	
	//Calculate moon position
	Matrix_TransformVector(negatedCamera, &skyTop, &moonPos);
	
	//Project moon position
	iw = 1.0 / moonPos.z;
	moonPos.x = moonPos.x * iw * state->pixelConversion + state->projectionOffset_X;
	moonPos.y = moonPos.y * iw * state->pixelConversion + state->projectionOffset_Y;
	
	//Draw moon
	if((moonPos.z > state->d) && (moonPos.x > 0.0) && (moonPos.x < state->viewWidth) && (moonPos.y > 0.0) && (moonPos.y < state->viewHeight)) {
		QASetInt(state->drawContext, kQATag_Blend, kQABlend_PreMultiply);
		
		Texture_DrawSpriteOnScreen(state, moonPos.x, moonPos.y, 0.9999, 1.0 / 0.9999, //z value precision required for 16Bits Z-Buffer
			kMoonSize * state->viewWidth, kMoonSize * state->viewWidth, moonTexture);
		
		QASetInt(state->drawContext, kQATag_Blend, kQABlend_Interpolate);
	}
}

