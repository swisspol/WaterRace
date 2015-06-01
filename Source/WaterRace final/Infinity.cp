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
#include				<QD3DAcceleration.h>

#include				"WaterRace.h"
#include				"Game.h"
#include				"Preferences.h"
#include				"Data Files.h"

#include				"Textures.h"
#include				"Infinity Rendering.h"

//CONSTANTES:

#define				kFurtherClippingFactor			1.50

#define				k3DfxLoadingPICTID				129

//ROUTINES:

TQAEngine* Get_GameEngine()
{
	TQAEngine*			theEngine;
	
	//User has a custom selection?
	if(thePrefs.raveEngineName[0] != 0)
	Get_RaveEngineByName(mainGDHandle, thePrefs.raveEngineName, &theEngine);
	else
	Get_BestRaveEngine(mainGDHandle, &theEngine);
	
	return theEngine;
}

TQAEngine* Get_InterfaceEngine()
{
	TQAEngine*			theEngine;
	
	//We need to find the Apple software engine
	if(thePrefs.engineFlags & kPref_EngineFlagUseSoftware) {
		if(Get_SoftwareEngine(&theEngine) != noErr)
		return nil;
		else
		return theEngine;
	}
	
	//Same engine as the selected engine
	return Get_GameEngine();
}

OSErr Infinity_GameInit(TQAEngine* theEngine, StatePtr* state, Point size, Point offset)
{	
	unsigned long		contextFlags = kInfinityRenderingFlag_NonLinearZBuffer;
	OSErr				theError;
	
	if(thePrefs.engineFlags & kPref_EngineFlagTriFiltering)
	contextFlags |= kInfinityRenderingFlag_TriFiltering;
	else if(thePrefs.engineFlags & kPref_EngineFlagFiltering)
	contextFlags |= kInfinityRenderingFlag_Filtering;
	if(thePrefs.engineFlags & kPref_EngineFlag32BitZBuffer)
	contextFlags |= kInfinityRenderingFlag_DeepZ;
	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	contextFlags |= kInfinityRenderingFlag_LowResolution;
	
	theError = InfinityRendering_CreateContext(mainGDHandle, theEngine, state, nil, size, offset, contextFlags);
	if(theError)
	return theError;
	
#if __DISPLAY_3DFX_LOADING_SCREEN__
	if(Is3DfxKnownEngine(theEngine) && (size.h >= 512) && (size.v >= 256)) {
		TQATexture*			texture;
		short				oldResFile = CurResFile();
		
		UseResFile(interfaceResFileID);
		if(Texture_NewARGB32TextureFromPictResource(theEngine, &texture, k3DfxLoadingPICTID, 0) == noErr) {	
			QARenderStart((*state)->drawContext, NULL, NULL);
			QASetInt((*state)->drawContext, kQATag_ZFunction, kQAZFunction_None);
			Texture_DrawSpriteOnScreen(*state, size.h / 2.0, size.v / 2.0, 0.001, 1.0, 512, 256, texture);
			QASetInt((*state)->drawContext, kQATag_ZFunction, kQAZFunction_LT);
			QARenderEnd((*state)->drawContext, NULL);
			QATextureDelete(theEngine, texture);
		}
		UseResFile(oldResFile);
	}
#endif
	
	InfinityRendering_NoClear(*state);
	
	return noErr;
}

void Infinity_SetTerrainState(StatePtr state, TerrainPtr terrain)
{
	//Setup light
	InfinityRendering_SetLight(state, terrain->ambientLight, terrain->sunIntensity, terrain->sunAngle_X, terrain->sunAngle_Z);
	
	//Setup parameters
	if(thePrefs.engineFlags & kPref_EngineFlagFurtherClipping)
	InfinityRendering_SetClipping(state, 0.06, terrain->clippingDistance * kFurtherClippingFactor);
	else
	InfinityRendering_SetClipping(state, 0.06, terrain->clippingDistance);
	InfinityRendering_SetZoom(state, kCameraZoom);
	
	//Setup background color
#if 0
	InfinityRendering_ColorClear(state, theTerrain.backColor_r, theTerrain.backColor_g, theTerrain.backColor_b);
#else
	InfinityRendering_NoClear(state);
#endif
	
	videoFX = false;
	if(!(thePrefs.engineFlags & kPref_EngineFlagVideoFX))
	return;
	
	//Setup fog
	switch(terrain->fogMode) {
		case kFog_None: QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_None); break;
		case kFog_Linear: QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_Linear); break;
		case kFog_Exponential: QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_Exponential); break;
		case kFog_ExponentialSquared: QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_ExponentialSquared); break;
	}
	QASetFloat(state->drawContext, kQATag_FogColor_a, 1.0);
	QASetFloat(state->drawContext, kQATag_FogColor_r, terrain->fogColor_r);
	QASetFloat(state->drawContext, kQATag_FogColor_g, terrain->fogColor_g);
	QASetFloat(state->drawContext, kQATag_FogColor_b, terrain->fogColor_b);
	if(thePrefs.engineFlags & kPref_EngineFlagFurtherClipping) {
		QASetFloat(state->drawContext, kQATag_FogStart, terrain->fogStart * kFurtherClippingFactor);
		QASetFloat(state->drawContext, kQATag_FogEnd, terrain->fogEnd * kFurtherClippingFactor);
	}
	else {
		QASetFloat(state->drawContext, kQATag_FogStart, terrain->fogStart);
		QASetFloat(state->drawContext, kQATag_FogEnd, terrain->fogEnd);
	}
	QASetFloat(state->drawContext, kQATag_FogMaxDepth, state->f);
	QASetFloat(state->drawContext, kQATag_FogDensity, terrain->fogDensity);
	
	videoFX = true;
	
	//Patch the Apple software engine fog
	if(IsAppleSoftwareEngine(state->engine) && ((terrain->fogMode == kFog_Exponential) || ((terrain->fogMode == kFog_ExponentialSquared))))
	QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_Linear);
}
