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
#include				"Clipping Utils.h"

#include				"Infinity Error codes.h"

//CONSTANTES PREPROCESSEURS:

#define	__HILITE_SCREEN__	1

//CONSTANTES:

#define					kFlareSize1				0.45
#define					kFlareSize2				0.06
#define					kFlareSize3				0.15
#define					kFlareSize4				0.30
#define					kSunHeight				0.0

#define					kMaxBrightness			0.5

#define					kNbLens					4
#define					kLensStartID			7000

//VARIABLES LOCALES:

static TQATexture*		flareTextureList[kNbLens];
static Boolean			fxInited = false;

//ROUTINES:

OSErr PostFX_LensFlare_Init(StatePtr state, long textureFlags)
{
	long				i;
	OSErr				theError;
	
	if(fxInited)
	return kError_FatalError;
	
	for(i = 0; i < kNbLens; ++i) {
		theError = Texture_NewARGB32TextureFromPictResource(state->engine, &flareTextureList[i], kLensStartID + i, textureFlags);
		if(theError)
		return theError;
	}
	
	fxInited = true;
	
	return noErr;
}

void PostFX_LensFlare_Quit(StatePtr state)
{
	long				i;
	
	if(!fxInited)
	return;
	
	for(i = 0; i < kNbLens; ++i)
	QATextureDelete(state->engine, flareTextureList[i]);
	
	fxInited = false;
}

void PostFX_LensFlare_Display(StatePtr state, float skyRadius, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	Vector				skyTop,
						sunPos;
	float				iw,
						projectionOffset_X = state->projectionOffset_X,
						projectionOffset_Y = state->projectionOffset_Y;
	long				saveBlend;
						
	if(!fxInited)
	return;
	
	//Get sun position
	Vector_Normalize(&state->lightVector, &sunPos);
	Vector_Multiply(skyRadius, &sunPos, &sunPos);
	Vector_Add(cameraPos, &sunPos, &skyTop);
	skyTop.y = kSunHeight + sunPos.y;
	
	//Calculate sun position
	Matrix_TransformVector(negatedCamera, &skyTop, &sunPos);
	
	//Project sun position
	iw = 1.0 / sunPos.z;
	sunPos.x = sunPos.x * iw * state->pixelConversion + projectionOffset_X;
	sunPos.y = sunPos.y * iw * state->pixelConversion + projectionOffset_Y;
	
	//Draw lens flare
	if((sunPos.z > state->d) && (sunPos.x > 0.0) && (sunPos.x < state->viewWidth) && (sunPos.y > 0.0) && (sunPos.y < state->viewHeight)) {
		//QAFlush(state->drawContext);
		QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_None);
		saveBlend = QAGetInt(state->drawContext, kQATag_Blend);
		QASetInt(state->drawContext, kQATag_Blend, kQABlend_PreMultiply);
		
		sunPos.x -= projectionOffset_X;
		sunPos.y -= projectionOffset_Y;
		
		//Draw flare
		Texture_DrawSpriteOnScreen(state, sunPos.x + projectionOffset_X, sunPos.y + projectionOffset_Y, 1.0, 1.0, 
			kFlareSize1 * state->viewWidth, kFlareSize1 * state->viewWidth, flareTextureList[0]);
		
		//Draw lens 1
		Texture_DrawSpriteOnScreen(state, 0.5 * sunPos.x + projectionOffset_X, 0.5 * sunPos.y + projectionOffset_Y, 0.03, 1.0 / 0.03, 
			kFlareSize2 * state->viewWidth, kFlareSize2 * state->viewWidth, flareTextureList[1]);
		
		//Draw lens 2
		Texture_DrawSpriteOnScreen(state, -0.1 * sunPos.x + projectionOffset_X, -0.1 * sunPos.y + projectionOffset_Y, 0.02, 1.0 / 0.02, 
			kFlareSize3 * state->viewWidth, kFlareSize3 * state->viewWidth, flareTextureList[2]);
			
		//Draw lens 3
		Texture_DrawSpriteOnScreen(state, -sunPos.x + projectionOffset_X, -sunPos.y + projectionOffset_Y, 0.01, 1.0 / 0.02, 
			kFlareSize4 * state->viewWidth, kFlareSize4 * state->viewWidth, flareTextureList[3]);
		
		QASetInt(state->drawContext, kQATag_Blend, kQABlend_Interpolate);
		
		//Set gamma
#if __HILITE_SCREEN__ //We suppose screen H > screen V
		float				brightness = kMaxBrightness * (1.0 - FastSqrt(2.0 * (sunPos.x * sunPos.x + sunPos.y * sunPos.y) / (state->viewHeight * state->viewHeight)));
		long				i;
		TQAVGouraud			verts[4];
	
		for(i = 0; i < 4; ++i) {
			verts[i].r = verts[i].g = verts[i].b = 1.0;
			verts[i].a = brightness;
			verts[i].z = 0.0001;
			verts[i].invW = 1.0;
		}
		verts[0].x = boundrySlop; verts[0].y = boundrySlop;
		verts[1].x = state->viewWidth - boundrySlop; verts[1].y = boundrySlop;
		verts[2].x = state->viewWidth - boundrySlop; verts[2].y = state->viewHeight - boundrySlop;
		verts[3].x = boundrySlop; verts[3].y = state->viewHeight - boundrySlop;
		
		QADrawTriGouraud(state->drawContext, &verts[0], &verts[1], &verts[2], 0);
		QADrawTriGouraud(state->drawContext, &verts[2], &verts[3], &verts[0], 0);
#endif
		
		QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_LT);
		QASetInt(state->drawContext, kQATag_Blend, saveBlend);
	}
}
