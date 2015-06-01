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

#include				"Shape Utils.h"
#include				"Infinity Utils.h"
#include				"Infinity Error codes.h"

//CONSTANTES:

#define					kSnowID					7200

#define					kNbParticles			164

//VARIABLES LOCALES:

static TQATexture*		snowTexture;
static ShapePtr			snowSystem;
static Boolean			fxInited = false;

//ROUTINES:

OSErr PostFX_Snow_Init(StatePtr state, long textureFlags)
{
	OSErr					theError;
	ShapeData_ParticlePtr	data;
	
	if(fxInited)
	return kError_FatalError;
	
	snowSystem = Shape_New_ParticleSystem(kNoID, kNbParticles);
	if(snowSystem == nil)
	return kError_FatalError;
	
	theError = Texture_NewARGB32TextureFromPictResource(state->engine, &snowTexture, kSnowID, textureFlags | kTextureFlag_LowQuality);
	if(theError)
	return theError;
	
	data = (ShapeData_ParticlePtr) snowSystem->dataPtr;
	if(data == nil)
	return kError_FatalError;
	
	data->flags = kFlag_GlobalSystem + kFlag_KeepInBBox;
	data->gravity = -0.3;
	data->velocityDecrease = 0.5;
	data->energyDecrease = 4.0;
	data->sizeDecrease = 0.0;
	data->initPosition.x = 0.0;
	data->initPosition.y = 0.0;
	data->initPosition.z = 0.0;
	data->initPositionRandom.x = -3.0;
	data->initPositionRandom.y = -3.0;
	data->initPositionRandom.z = -1.0;
	data->initVelocity.x = 0.0;
	data->initVelocity.y = -1.0;
	data->initVelocity.z = 0.0;
	data->initVelocityRandom.x = -0.5;
	data->initVelocityRandom.y = -0.2;
	data->initVelocityRandom.z = 0.0;
	data->initEnergy = 20.0;
	data->initEnergyRandom = 11.0;
	data->initSize = 0.1;
	data->initSizeRandom = -0.05;
	data->modulate_r = data->modulate_g = data->modulate_b = 0.0;
	
	snowSystem->parent = nil;
	Shape_ResolveRendering(snowSystem);
	snowSystem->texturePtr = snowTexture;
	
	SetUpBBox_Corners(-3.5, -3.5, -1.5, 3.5, 3.5, 1.5, snowSystem->boundingBox);
	
	fxInited = true;
	
	return noErr;
}

void PostFX_Snow_Quit(StatePtr state)
{
	if(!fxInited)
	return;
	
	Shape_Dispose(snowSystem);
	QATextureDelete(state->engine, snowTexture);

	fxInited = false;
}

void PostFX_Snow_Display(StatePtr state, MatrixPtr camera, MatrixPtr negatedCamera)
{
	Matrix				snowPos;
	
	if(!fxInited)
	return;
	
	snowPos = *camera;
	Vector_MultiplyAdd(4.0, &camera->z, &snowPos.w, &snowPos.w);
	
	Shape_CallRenderingProc(snowSystem, state, &snowPos, negatedCamera, nil, nil, kXYClipping + kZClipping);
}
