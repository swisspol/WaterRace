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


#include				"Infinity Structures.h"
#include				"Infinity Terrain.h"
#include				"MetaFile Structures.h"
#include				"MetaFile Data Tags.h"
#include				"Infinity Error Codes.h"
#include				"Infinity Audio Engine.h"

#include				"Matrix.h"
#include				"Vector.h"
#include				"Infinity rendering.h"
#include				"Infinity Utils.h"
#include				"Textures.h"
#include				"Cinematic Utils.h"
#include				"Camera utils.h"
#include				"Clock.h"
#include				"Infinity Player.h"
#include				"MetaFile Utils.h"
#include				"Object Utils.h"

//CONSTANTES:

#define					kDefaultScale				1.0
#define					kMaxEmbendedSounds			32

#define					kCameraID					'camr'
#define					kShapeCameraZoom			0.5

//VARIABLES LOCALES:

static CameraState			theCamera;
static StatePtr				theState;
static MegaObject			megaObject;
static TextureMemory		videoMemory;
static Boolean				animation = false,
							audio;
static ScriptPtr			currentScript = nil;
static ShapePtr				cameraShape;
static long					soundCount;
static Data_SoundPtr		soundList[kMaxEmbendedSounds];
static OSType				soundIDList[kMaxEmbendedSounds];
static long					channelCount;
static TQABitmap*			backBitmap = nil;
static float				bitmapScaleFactor;

//ROUTINES:

static ShapePtr Find_CameraShape(ObjectPtr object, float* r, float* g, float *b)
{
	long					i;
	
	for(i = 0; i < object->shapeCount; ++i)
	if(object->shapeList[i]->id == kCameraID) {
		if(object->shapeList[i]->texture == kNoTexture) {
			*r = object->shapeList[i]->pointList[0].u;
			*g = object->shapeList[i]->pointList[0].v;
			*b = object->shapeList[i]->pointList[0].c;
		}
		return object->shapeList[i];
	}
	
	return nil;
}

#pragma export on
#ifdef __cplusplus
extern "C" {
#endif

Boolean InfinityPlayer_RaveAvailable()
{
	if((RoutineDescriptorPtr) QADrawContextNew == (RoutineDescriptorPtr) kUnresolvedCFragSymbolAddress)
	return false;
	
	return true;
}

OSErr InfinityPlayer_Init(Boolean enableAudio)
{
	OSErr				theError;
	
	theError = InfinityRendering_AllocateBuffers();
	if(theError)
	return theError;
	
	if(enableAudio) {
		theError = InfinityAudio_Init(&theCamera.camera);
		if(theError)
		return theError;
		InfinityAudio_Start();
	}
	audio = enableAudio;
	
	return noErr;
}

void InfinityPlayer_Quit()
{
	InfinityRendering_DisposeBuffers();
	if(audio)
	InfinityAudio_Quit();
}

OSErr InfinityPlayer_GetBestRaveEngine(GDHandle screenDevice, TQAEngine** theEngine)
{
	return Get_BestRaveEngine(screenDevice, theEngine);
}

OSErr InfinityPlayer_LoadAnimation(GDHandle screenDevice, TQAEngine* theEngine, FSSpec* animationFile, OSType textureSetID,
	short positionH, short positionV, short renderWidth, short renderHeight, unsigned long contextFlags, 
	unsigned long textureFlags, PixMapHandle backgroundPixMap, short bkgLeft, short bkgTop, Boolean useBitMap)
{
	OSErr					theError;
	Point					contextPosition,
							contextSize;
	long					i;
	MetaFile_ReferencePtr	reference;
	TextureSetPtr			textureSet;
	OSType					dataType;
	TQAImage				image;
	float					r = 0.0, g = 0.0, b = 0.0;
	
	if(animation)
	return kError_AnimationAlreadyLoaded;
	
	//If aliasing or low res, we need bitmap
	if(((contextFlags & kInfinityRenderingFlag_Aliasing) || (contextFlags & kInfinityRenderingFlag_LowResolution)) && (backgroundPixMap != nil) && !useBitMap)
	contextFlags &= ~(kInfinityRenderingFlag_Aliasing + kInfinityRenderingFlag_LowResolution);
	
	//Find a Rave engine
	if(theEngine == nil) {
		theError = InfinityPlayer_GetBestRaveEngine(screenDevice, &theEngine);
		if(theError)
		return theError;
	}
	
	//Init engine
	contextPosition.h = positionH;
	contextPosition.v = positionV;
	contextSize.h = renderWidth;
	contextSize.v = renderHeight;
	theError = InfinityRendering_CreateContext(screenDevice, theEngine, &theState, &theCamera, contextSize, contextPosition, contextFlags);
	if(theError)
	return theError;
	VRAM_Clear(&videoMemory, theState->engine);
	
	//Correct scale bitmap
	if(contextFlags & kInfinityRenderingFlag_Aliasing)
	bitmapScaleFactor = 2.0;
	else if(contextFlags & kInfinityRenderingFlag_LowResolution)
	bitmapScaleFactor = 0.5;
	else
	bitmapScaleFactor = 1.0;
	
	//Create background bitmap
	if(backgroundPixMap != nil) {
		if(useBitMap && ((**backgroundPixMap).pixelSize != 8)) {
			image.width = renderWidth;
			image.height = renderHeight;
			image.rowBytes = (**backgroundPixMap).rowBytes & 0x3FFF;
			image.pixmap = GetPixBaseAddr(backgroundPixMap) + image.rowBytes * bkgTop + bkgLeft * ((**backgroundPixMap).pixelSize / 8);
			
			/*if((**backgroundPixMap).pixelSize == 8)
			theError = QABitmapNew(theState->engine, kQABitmap_None, kQAPixel_CL8, &image, &backBitmap);*/ //Bitmaps drawing at 8 Bits depth produces incorrect results
			if((**backgroundPixMap).pixelSize == 16)
			theError = QABitmapNew(theState->engine, kQABitmap_None, kQAPixel_RGB16, &image, &backBitmap);
			else if((**backgroundPixMap).pixelSize == 32)
			theError = QABitmapNew(theState->engine, kQABitmap_None, kQAPixel_RGB32, &image, &backBitmap);
			if(theError)
			return theError;
			
			InfinityRendering_NoClear(theState);
		}
		else {
			backBitmap = nil;
			
			contextPosition.h = bkgLeft;
			contextPosition.v = bkgTop;
			InfinityRendering_CopyClear(theState, contextSize, contextPosition, backgroundPixMap);
		}
	}
	
	//Open MetaFile
	theError = MetaFile_Read_OpenFile(animationFile, &reference);
	if(theError)
	return theError;
	
	//Load model
	theError = MetaFile_Read_MegaObject_ByIndex(reference, &megaObject, 1, kMetaFile_SkipCameras);
	if(theError)
	return theError;
	
	//Load default texture set
	if(textureSetID == kPlayer_DefaultTextureSet)
	theError = MetaFile_Read_GetModelDefaultTextureSet(reference, &textureSet, &megaObject);
	else {
		theError = MetaFile_Read_TextureSet_ByID(reference, &textureSet, megaObject.object.id, textureSetID);
		if(theError == kError_ItemNotFound)
		theError = MetaFile_Read_GetModelDefaultTextureSet(reference, &textureSet, &megaObject);
	}
	if(theError)
	return theError;
	
	//Load sound data
	if(audio) {
		soundCount = MetaFile_CountDataSubTags(reference, kSubTag_Sound);
		for(i = 0; i < soundCount; ++i) {
			//Read sound data
			dataType = kSubTag_Sound;
			theError = MetaFile_Read_Data(reference, &dataType, i + 1, (Ptr*) &soundList[i], &soundIDList[i]);
			if(theError)
			return theError;
		}
		Script_AudioEnable(soundCount, soundList, soundIDList);
	}
	
	//Close MetaFile
	MetaFile_Read_CloseFile(reference);
	
	//Check for scripts
#if 0
	if(megaObject.scriptCount == 0)
	return kError_NoScriptFound;
#endif
	
	//Load texture in VRAM
	theError = VRAM_LoadTextureSet(&videoMemory, textureSet, textureFlags);
	if(theError)
	return theError;
	MegaObject_AttachTextureSet(&megaObject, textureSet);
	TextureSet_Dispose(textureSet);
	
	//Resolve
	MegaObject_Resolve(&megaObject, &videoMemory);
	
	//Find camera
	cameraShape = Find_CameraShape(&megaObject.object, &r, &g, &b);
	if(cameraShape) {
		InfinityRendering_SetZoom(theState, kShapeCameraZoom);
		if(backgroundPixMap == nil)
		InfinityRendering_ColorClear(theState, r, g, b);
	}
	
	//Place model
	Matrix_Clear(&megaObject.object.pos);
	megaObject.object.rotateX = 0.0;
	megaObject.object.rotateY = 0.0;
	megaObject.object.rotateZ = 0.0;
	megaObject.object.scale = kDefaultScale;
	Object_UpdateMatrix(&megaObject.object);
	
	animation = true;
	currentScript = nil;
	
	//Start audio
	if(audio)
	InfinityAudio_InstallObject(kCinematic_AudioClient, &megaObject.object, &megaObject.object.pos);
	
	return noErr;
}

void InfinityPlayer_SetClearColor(float r, float g, float b)
{
	InfinityRendering_ColorClear(theState, r, g, b);
}

void InfinityPlayer_SetModelMatrix(float x, float y, float z, float rx, float ry, float rz, float scale)
{
	//Place model
	Matrix_Clear(&megaObject.object.pos);
	megaObject.object.pos.w.x = x;
	megaObject.object.pos.w.x = y;
	megaObject.object.pos.w.x = z;
	megaObject.object.rotateX = rx;
	megaObject.object.rotateY = ry;
	megaObject.object.rotateZ = rz;
	megaObject.object.scale = scale;
	Object_UpdateMatrix(&megaObject.object);
}

void InfinityPlayer_SetCamera(float pos_X, float pos_Y, float pos_Z, float roll, float pitch, float yaw, float zoom)
{
	if(!animation)
	return;
	
	//Setup camera position
	Matrix_Clear(&theCamera.camera);
	theCamera.camera.w.x = pos_X;
	theCamera.camera.w.y = pos_Y;
	theCamera.camera.w.z = pos_Z;
	theCamera.roll = roll;
	theCamera.pitch = pitch;
	theCamera.yaw = yaw;
	CameraState_UpdateMatrix(&theCamera);
	
	//Setup camera position
	InfinityRendering_SetZoom(theState, zoom);
}

void InfinityPlayer_SetLight(float ambient, float sun, float sun_Rx, float sun_Rz)
{
	if(!animation)
	return;
	
	InfinityRendering_SetLight(theState, ambient, sun, sun_Rx, sun_Rz);
}

void InfinityPlayer_SetClipping(float hither, float yon)
{
	if(!animation)
	return;
	
	InfinityRendering_SetClipping(theState, hither, yon);
}

OSErr InfinityPlayer_UnloadAnimation()
{
	long				i;
	
	if(!animation)
	return kError_NoAnimationLoaded;
	
	//Clean up engine
	MegaObject_ReleaseData(&megaObject);
	VRAM_Purge(&videoMemory);
	if(backBitmap != nil)
	QABitmapDelete(theState->engine, backBitmap);
	InfinityRendering_DisposeContext(theState);
	
	if(audio) {
		for(i = 0; i < soundCount; ++i)
		DisposePtr((Ptr) soundList[i]);
		Script_AudioDisable();
	}
	
	animation = false;
	
	return noErr;
}

#ifdef __cplusplus
}
#endif
#pragma export off

static void Draw_Frame(ObjectPtr object, StatePtr state)
{
	long				i;
	Matrix				negatedCamera;
	TQAVGouraud			v;
	
	if(cameraShape != nil) {
		theCamera.camera = megaObject.object.pos;
		//We suppose camera is not linked
		Matrix_Cat(&cameraShape->pos, &theCamera.camera, &theCamera.camera);
	}
	Matrix_Negate(&theCamera.camera, &negatedCamera);
	
	//Start rendering
	QARenderStart(theState->drawContext, NULL, NULL);
	theState->lastFrameTime = state->frameTime;
	theState->frameTime = RClock_GetAbsoluteTime();
	
	//Draw background
	if(backBitmap != nil) {
		v.r = v.g = v.b = v.a = 1.0;
		v.x = 0.0;
		v.y = 0.0;
		v.z = 0.9999;
		v.invW = 1.0 / 0.9999;
		QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_None);
		QADrawBitmap(state->drawContext, &v, backBitmap);
		QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_LT);
	}
	
	//Draw model
	for(i = 0; i < object->shapeCount; ++i)
	Shape_CallRenderingProc(object->shapeList[i], state, &object->pos, &negatedCamera, &theCamera.camera.w, object->shapeList, kUndetermined);
	
	//End rendering
	QARenderEnd(theState->drawContext, NULL);
}

#pragma export on
#ifdef __cplusplus
extern "C" {
#endif

void InfinityPlayer_EraseBackground()
{
	TQAVGouraud		v;
	
	if(!animation)
	return;
	
	QARenderStart(theState->drawContext, NULL, NULL);
	if(backBitmap != nil) {
		v.r = v.g = v.b = v.a = 1.0;
		v.x = 0.0;
		v.y = 0.0;
		v.z = 0.9999;
		v.invW = 1.0 / 0.9999;
		QASetInt(theState->drawContext, kQATag_ZFunction, kQAZFunction_None);
		QADrawBitmap(theState->drawContext, &v, backBitmap);
		QASetInt(theState->drawContext, kQATag_ZFunction, kQAZFunction_LT);
	}
	QARenderEnd(theState->drawContext, NULL);
}

OSErr InfinityPlayer_PlayAnimation(OSType animationID)
{
	long					i,
							scriptNum = -1;
	
	if(!animation)
	return kError_NoAnimationLoaded;
	
	//Find the script
	if(animationID == kPlayer_DefaultAnimationID)
	scriptNum = 0;
	else {
		for(i = 0; i < megaObject.scriptCount; ++i)
		if(megaObject.scriptList[i]->id == animationID)
		scriptNum = i;
		if(scriptNum == -1)
		return kError_ScriptNotFound;
	}
	
	//Stop current script
	if(currentScript != nil)
	Script_Stop(currentScript);
	
	//Start script
	Script_Start(megaObject.scriptList[scriptNum], RClock_GetAbsoluteTime());
	currentScript = megaObject.scriptList[scriptNum];
	
	return noErr;
}

void InfinityPlayer_StopAnimation()
{
	if(!animation)
	return;
	if(currentScript == nil)
	return;
	
	//Stop!
	Script_Stop(currentScript);
	currentScript = nil;
}

void InfinityPlayer_RenderFrame()
{
	if(!animation)
	return;
	
	//Draw frame
	Draw_Frame(&megaObject.object, theState);
}

Boolean InfinityPlayer_RunAsync(float maxFPS)
{
	if(!animation)
	return true;
	if(currentScript == nil)
	return true;
	
	Script_Run(currentScript, RClock_GetAbsoluteTime());
	
	//Draw frame
	Draw_Frame(&megaObject.object, theState);
	
	//Wait for next frame
	if(maxFPS > 0.0)
	InfinityRendering_MaxFPS(maxFPS);
		
	if(currentScript->flags & kFlag_Running)
	return false;
	
	currentScript = nil;
	return true;
}

Boolean InfinityPlayer_RunSync(long flags, float maxFPS)
{
	KeyMap					theKeys;
	Boolean					run = true,
							forceStop = false;
	
	if(!animation)
	return false;
	if(currentScript == nil)
	return false;
	
	//Run!
	while(run) {
		//Did the user press a key?
		if(flags & kPlayerFlag_StopOnKeyBoard) {
			GetKeys(theKeys);
			theKeys[1] &= ~(1 << 1); //Mask the Caps Lock key
			if(theKeys[0] || theKeys[1] || theKeys[2] || theKeys[3]) {
				run = false;
				forceStop = true;
			}
		}
		
		//Did the user press mouse button?
		if(flags & kPlayerFlag_StopOnMouse) {
			if(Button()) {
				run = false;
				forceStop = true;
			}
		}
		
		//Is the animation done
		if(!(currentScript->flags & kFlag_Running))
		run = false;
		
		//Animation!
		Script_Run(currentScript, RClock_GetAbsoluteTime());
		
		//Draw frame
		Draw_Frame(&megaObject.object, theState);
		
		//Wait for next frame
		if(maxFPS > 0.0)
		InfinityRendering_MaxFPS(maxFPS);
	}
	Script_Stop(currentScript);
	if(forceStop && (flags & kPlayerFlag_DisplayLastFrameOnStop)) {
		Script_DisplayTime(currentScript, currentScript->length);
		Draw_Frame(&megaObject.object, theState);
	}
	currentScript = nil;
	
	return forceStop;
}

Boolean InfinityPlayer_ScriptPlaying()
{
	if(!animation)
	return false;
	if(currentScript == nil)
	return false;
	
	return true;
}

unsigned long InfinityPlayer_GetCurrentAnimationLength()
{
	if(!animation || (currentScript == nil))
	return 0;
	
	return currentScript->length;
}

void InfinityPlayer_SetCurrentAnimationTime(unsigned long localTime)
{
	if(!animation)
	return;
	if(currentScript == nil)
	return;
	
	if(currentScript->flags & kFlag_Running) {
		currentScript->flags &= ~kFlag_Running;
		Script_DisplayTime(currentScript, localTime);
		currentScript->flags |= kFlag_Running;
	}
	else
	Script_DisplayTime(currentScript, localTime);
}

void InfinityPlayer_MaxFPS(float maxFPS)
{
	InfinityRendering_MaxFPS(maxFPS);
}

#ifdef __cplusplus
}
#endif
#pragma export off

MegaObjectPtr InfinityPlayer_ReturnMegaObject()
{
	if(!animation)
	return nil;
	
	return &megaObject;
}
