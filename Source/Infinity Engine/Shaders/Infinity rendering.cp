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
#include				"Infinity Error Codes.h"

#include				"Matrix.h"
#include				"Infinity rendering.h"
#include				"Camera Utils.h"
#include				"Infinity Utils.h"
#include				"Quaternion.h"
#include				"Textures.h"

//CONSTANTES:

#define					kVerticesBufferSize		10000
#define					kTrianglesBufferSize	10000
#define					kTriangles2BufferSize	kTrianglesBufferSize

//VARIABLES:

TQAIndexedTriangle		*trianglesBuffer,
						*trianglesBuffer2;
TQAVTexture				*vTextureBuffer;
TQAVGouraud				*vGouraudBuffer;

float					_sinTable[kTrigTableResolution],
						_cosTable[kTrigTableResolution];

#if __BENCHMARK__
unsigned long			_verticeCount,
						_triCount;
#endif

static long				initializePositionH,
						initializePositionV,
						initializeSizeH,
						initializeSizeV;
static PixMapHandle		initializePixMapHandle;
static Boolean			buffersAllocated = false;
static UnsignedWide		_lastFrameTime = {0,0};
static TQABufferNoticeMethod	BufferClearBackUp = nil;

//ROUTINES:

static void Buffer_InitializeMethod_None(const TQADrawContext* drawContext, const TQADevice* buffer, const TQARect* dirtyRect, void* refCon)
{
	;
}

void InfinityRendering_NoClear(StatePtr state)
{
	TQANoticeMethod		noticeMethod;
	
	noticeMethod.bufferNoticeMethod = (TQABufferNoticeMethod) Buffer_InitializeMethod_None;
	QASetNoticeMethod(state->drawContext, kQAMethod_ImageBufferInitialize, noticeMethod, nil);
}

static void Buffer_InitializeMethod_BufferCopy(const TQADrawContext* drawContext, const TQADevice* buffer, const TQARect* dirtyRect, void* refCon)
{
	PixMapPtr			thePixMapPtr;
	Ptr					destBaseAddress,
						sourceBaseAddress;
	long				sourceRowBytes,
						destRowBytes,
						height;
	TQADeviceMemory*	memoryDevice;
	double				*source,
						*dest;
	unsigned long		copyWidth;
												
	//Set up pointers
	thePixMapPtr = *initializePixMapHandle;
	sourceRowBytes = thePixMapPtr->rowBytes & 0x3FFF;
	sourceBaseAddress = thePixMapPtr->baseAddr;
	
	if(buffer->deviceType != kQADeviceMemory)
	return;
	memoryDevice = (TQADeviceMemory*) &buffer->device.memoryDevice;
	destRowBytes = memoryDevice->rowBytes;
	destBaseAddress = (Ptr) memoryDevice->baseAddr;
	
	//Copy data
	copyWidth = initializeSizeH * (thePixMapPtr->pixelSize / 8) / 8;
	sourceBaseAddress += sourceRowBytes * initializePositionV + (thePixMapPtr->pixelSize / 8) * initializePositionH;
	height = initializeSizeV;
	sourceBaseAddress += height * sourceRowBytes;
	destBaseAddress += height * destRowBytes;
	do {
		sourceBaseAddress -= sourceRowBytes;
		destBaseAddress -= destRowBytes;
		
		source = (double*) sourceBaseAddress;
		dest = (double*) destBaseAddress;
		for(unsigned long i = 0; i < copyWidth; ++i)
		*dest++ = *source++;
	} while(--height);
}

void InfinityRendering_CopyClear(StatePtr state, Point size, Point position, PixMapHandle copyPixMap)
{
	TQANoticeMethod		noticeMethod;
	
	initializePositionH = position.h;
	initializePositionV = position.v;
	initializeSizeH = size.h;
	initializeSizeV = size.v;
	initializePixMapHandle = copyPixMap;
	noticeMethod.bufferNoticeMethod = (TQABufferNoticeMethod) Buffer_InitializeMethod_BufferCopy;
	QASetNoticeMethod(state->drawContext, kQAMethod_ImageBufferInitialize, noticeMethod, nil);
}

void InfinityRendering_ColorClear(StatePtr state, float r, float g, float b)
{
	TQANoticeMethod		noticeMethod;
	
	QASetFloat(state->drawContext, kQATag_ColorBG_a, 1.0);
	QASetFloat(state->drawContext, kQATag_ColorBG_r, r);
	QASetFloat(state->drawContext, kQATag_ColorBG_g, g);
	QASetFloat(state->drawContext, kQATag_ColorBG_b, b);
	
	noticeMethod.bufferNoticeMethod = BufferClearBackUp;
	QASetNoticeMethod(state->drawContext, kQAMethod_ImageBufferInitialize, noticeMethod, nil);
}

static void Save_BufferClearMethod(StatePtr state)
{
	TQANoticeMethod		noticeMethod;
	void*				refCon;
	
	QAGetNoticeMethod(state->drawContext, kQAMethod_ImageBufferInitialize, &noticeMethod, &refCon);
	BufferClearBackUp = noticeMethod.bufferNoticeMethod;
}

static OSErr Setup_DrawContext(GDHandle screenDevice, StatePtr state, Point size, Point position, Boolean deepZ, Boolean aliasing, Boolean lowRes)
{
	TQAError			err;
	Rect				bounds;
	TQADevice			qaDevice;
	TQARect 			qaBoundsRect[2];
	unsigned long 		flags,
						response,
						optionalFeatures2;
	
	//Check for conflict
	if(aliasing && lowRes) {
		lowRes = false;
		aliasing = false;
	}
	
	//Should we enable anti-aliasing or lowRes?
	if(aliasing || lowRes) {
		if(QAEngineGestalt(state->engine, kQAGestalt_OptionalFeatures2, &optionalFeatures2) == noErr) {
			if(!((optionalFeatures2 & kQAOptional2_DrawContextScale) && (optionalFeatures2 & kQAOptional2_BitmapScale))) {
				lowRes = false;
				aliasing = false;
			}
		}
		else {
			lowRes = false;
			aliasing = false;
		}
	}
	
	if(aliasing) {
		SetRect(&bounds, 0, 0, size.h * 2.0, size.v * 2.0);
		qaBoundsRect[0].left = bounds.left + position.h;
		qaBoundsRect[0].right = bounds.right + position.h;
		qaBoundsRect[0].top = bounds.top + position.v;
		qaBoundsRect[0].bottom = bounds.bottom + position.v;
	
		qaBoundsRect[1].left = position.h;
		qaBoundsRect[1].right = position.h + size.h;
		qaBoundsRect[1].top = position.v;
		qaBoundsRect[1].bottom = position.v + size.v;
	}
	else if(lowRes) {
		SetRect(&bounds, 0, 0, size.h / 2.0, size.v / 2.0);
		qaBoundsRect[0].left = bounds.left + position.h;
		qaBoundsRect[0].right = bounds.right + position.h;
		qaBoundsRect[0].top = bounds.top + position.v;
		qaBoundsRect[0].bottom = bounds.bottom + position.v;
	
		qaBoundsRect[1].left = position.h;
		qaBoundsRect[1].right = position.h + size.h;
		qaBoundsRect[1].top = position.v;
		qaBoundsRect[1].bottom = position.v + size.v;
	}
	else {
		SetRect(&bounds, 0, 0, size.h, size.v);
		qaBoundsRect[0].left = bounds.left + position.h;
		qaBoundsRect[0].right = bounds.right + position.h;
		qaBoundsRect[0].top = bounds.top + position.v;
		qaBoundsRect[0].bottom = bounds.bottom + position.v;
	}
	state->viewRect = bounds;
	state->viewHeight = bounds.bottom;
	state->viewWidth = bounds.right;
	
	qaDevice.deviceType = kQADeviceGDevice;
	qaDevice.device.gDevice = screenDevice;
	
	flags = 0;
	if(state->doubleBuffer)
	flags += kQAContext_DoubleBuffer;
	if(state->noZBuffer)
	flags += kQAContext_NoZBuffer;
	if(aliasing || lowRes)
	flags += kQAContext_Scale;
	
	//Should we enable 32Bits Z-Buffer?
	err = QAEngineGestalt(state->engine, kQAGestalt_OptionalFeatures, &response);
	if((err == noErr) && (response & kQAOptional_DeepZ) && deepZ)
	flags += kQAContext_DeepZ;
	
	err = QADrawContextNew(&qaDevice, qaBoundsRect, nil, state->engine, flags, &state->drawContext);
	if(err)
	return err;
	
	QASetInt(state->drawContext, kQATag_ZFunction, state->alwaysVisible ? kQAZFunction_True : kQAZFunction_LT);
	QASetInt(state->drawContext, kQATag_Antialias, state->antialiasing);
	QASetInt(state->drawContext, kQATag_PerspectiveZ, state->perspectiveZ ? kQAPerspectiveZ_On : kQAPerspectiveZ_Off);
	QASetInt(state->drawContext, kQATag_TextureFilter, state->textureFilter);
	QASetInt(state->drawContext, kQATag_Blend, kQABlend_Interpolate);
	
	QASetFloat(state->drawContext, kQATag_ColorBG_a, 1.0);
	QASetFloat(state->drawContext, kQATag_ColorBG_r, 0.0);
	QASetFloat(state->drawContext, kQATag_ColorBG_g, 0.0);
	QASetFloat(state->drawContext, kQATag_ColorBG_b, 0.0);
	
	if(aliasing) {
		QASetFloat(state->drawContext, kQATag_BitmapScale_x, (size.h * 2.0 - 1.0) / size.h);
		QASetFloat(state->drawContext, kQATag_BitmapScale_y, (size.v * 2.0 - 1.0) / size.v);
	}
	else if(lowRes) {
		QASetFloat(state->drawContext, kQATag_BitmapScale_x, 0.5);
		QASetFloat(state->drawContext, kQATag_BitmapScale_y, 0.5);
	}
	
#if 1
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate + kQATextureOp_Highlight);
#endif
	
	return noErr;
}

void InfinityRendering_SetLight(StatePtr state, float ambient, float sun, float sun_Rx, float sun_Rz)
{
	Matrix				m;
	
	//Set up light
	state->lightVector.x = 0.0;
	state->lightVector.y = sun;
	state->lightVector.z = 0.0;
	state->ambient = ambient;
	
	//Rotate sun
	Matrix_SetRotateX(sun_Rx, &m);
	Matrix_TransformVector(&m, &state->lightVector, &state->lightVector);
	Matrix_SetRotateZ(sun_Rz, &m);
	Matrix_TransformVector(&m, &state->lightVector, &state->lightVector);
}

void InfinityRendering_UpdateStateParams(StatePtr state)
{
	state->h = state->d * state->zoom;
	state->c = YonDistance2HideDistance(state->f);
	state->pixelConversion = (-state->viewWidth / 2.0) / state->zoom;
	state->p1 = state->f / (state->f - state->d);
	state->p2 = state->d * state->p1;
	
	state->invRange = 1.0 / (state->f - state->d);
}

void InfinityRendering_SetZoom(StatePtr state, float zoom)
{
	state->zoom = zoom;
	
	InfinityRendering_UpdateStateParams(state);
}

void InfinityRendering_SetClipping(StatePtr state, float hither, float yon)
{
	state->d = hither;
	state->f = yon;
	
	InfinityRendering_UpdateStateParams(state);
}

OSErr InfinityRendering_CreateContext(GDHandle screenDevice, TQAEngine* theEngine, StatePtr* state, CameraStatePtr cameraState, Point size, Point position, unsigned long contextFlags)
{
	StatePtr			localState;
	OSErr				theError;
	Boolean				deepZ = false,
						aliasing = false,
						lowRes = false;
						
	//Make sure we have a Rave engine
	if(theEngine == nil)
	return kError_NoRaveEngineFound;
	
	//Create and fill State structure
	*state = (StatePtr) NewPtrClear(sizeof(State));
	if(*state == nil)
	return MemError();
	localState = *state;
	
	localState->d = 0.06;
	localState->f = 60.0;
	localState->zoom = 0.2;
	
	localState->doubleBuffer = 1;
	localState->noZBuffer = 0;
	localState->alwaysVisible = 0;
	localState->perspectiveZ = 1;
	if(contextFlags & kInfinityRenderingFlag_TriFiltering)
	localState->textureFilter = kQATextureFilter_Best;
	else if(contextFlags & kInfinityRenderingFlag_Filtering)
	localState->textureFilter = kQATextureFilter_Mid;
	else
	localState->textureFilter = kQATextureFilter_Fast;
	localState->antialiasing = 0;
	localState->engine = theEngine;
	localState->lastTexture = nil;
	if(contextFlags & kInfinityRenderingFlag_NonLinearZBuffer)
	localState->linearZBuffer = false;
	else
	localState->linearZBuffer = true;
	localState->frameTime = localState->lastFrameTime = 0;
	
	//Set up default lighting environnement
	InfinityRendering_SetLight(localState, 0.5, 1.0, DegreesToRadians(30.0), DegreesToRadians(-30.0));
	
	//Setup default camera
	if(cameraState != nil) {
		Matrix_Clear(&cameraState->camera);
		cameraState->camera.w.z = 20.0;
		cameraState->roll = 0.0;
		cameraState->pitch = DegreesToRadians(180.0);
		cameraState->yaw = 0.0;
		CameraState_UpdateMatrix(cameraState);
	}
	
	if(contextFlags & kInfinityRenderingFlag_DeepZ)
	deepZ = true;
	if(contextFlags & kInfinityRenderingFlag_Aliasing)
	aliasing = true;
	if(contextFlags & kInfinityRenderingFlag_LowResolution)
	lowRes = true;
	theError = Setup_DrawContext(screenDevice, localState, size, position, deepZ, aliasing, lowRes);
	if(theError) {
		DisposePtr((Ptr) *state);
		return theError;
	}
	
	InfinityRendering_UpdateStateParams(localState);
	localState->projectionOffset_X = localState->viewWidth / 2.0;
	localState->projectionOffset_Y = localState->viewHeight / 2.0;
	
	Save_BufferClearMethod(localState);
	
	return noErr;
}

OSErr InfinityRendering_DisposeContext(StatePtr theState)
{
	QADrawContextDelete(theState->drawContext);
	DisposePtr((Ptr) theState);

	return noErr;
}

OSErr InfinityRendering_AllocateBuffers()
{
	long				i;
	float 				angle;
	
	if(buffersAllocated)
	return noErr;
	
	trianglesBuffer = (TQAIndexedTriangle*) NewPtr(sizeof(TQAIndexedTriangle) * kTrianglesBufferSize);
	if(trianglesBuffer == nil)
	return MemError();
	
	trianglesBuffer2 = (TQAIndexedTriangle*) NewPtr(sizeof(TQAIndexedTriangle) * kTriangles2BufferSize);
	if(trianglesBuffer2 == nil)
	return MemError();
	
	vTextureBuffer = (TQAVTexture*) NewPtr(sizeof(TQAVTexture) * kVerticesBufferSize);
	if(vTextureBuffer == nil)
	return MemError();
	
	vGouraudBuffer = (TQAVGouraud*) NewPtr(sizeof(TQAVGouraud) * kVerticesBufferSize);
	if(vGouraudBuffer == nil)
	return MemError();
	
	//Populate trig look up tables
	for(i = 0; i < kTrigTableResolution; ++i) {
		angle = (float) i / (float) kTrigTableResolution * 2.0 * kPi;
		_sinTable[i] = sin(angle);
		_cosTable[i] = cos(angle);
	}
	
	buffersAllocated = true;
	
	return Texture_InitBuffers();
}

void InfinityRendering_DisposeBuffers()
{
	if(!buffersAllocated)
	return;
	
	DisposePtr((Ptr) trianglesBuffer);
	DisposePtr((Ptr) trianglesBuffer2);
	DisposePtr((Ptr) vTextureBuffer);
	DisposePtr((Ptr) vGouraudBuffer);
	
	buffersAllocated = false;
	
	Texture_DisposeBuffers();
}

long InfinityRendering_AvailableVRAM(TQAEngine* theEngine)
{
	long				response;
	OSErr				theError;
	
	theError = QAEngineGestalt(theEngine, kQAGestalt_FastTextureMemory, &response);
	if(theError)
	return -1;
	
	return response / 1024;
}

void InfinityRendering_MaxFPS(float maxFPS)
{
	UnsignedWide	time;
	float			diffTime = 1000000.0 / maxFPS;
	
	do {
		Microseconds(&time);
	} while(time.lo - _lastFrameTime.lo < diffTime);
	
	_lastFrameTime = time;
}

void VRAM_Clear(TextureMemoryPtr vram, TQAEngine* engine)
{
	long					i;
	
	vram->engine = engine;
	vram->textureCount = 0;
	
	for(i = 0; i < kMaxTextures; ++i) {
		vram->textureList[i] = nil;
		vram->storageList[i] = nil;
	}
}

OSErr VRAM_LoadTextureSet(TextureMemoryPtr vram, TextureSetPtr textureSet, long flags)
{
	long					i,
							j;
	OSErr					theError;
	Boolean					found;
	
	for(i = 0; i < textureSet->textureCount; ++i) {
		//Check for duplicate
		found = false;
		for(j = 0; j < vram->textureCount; ++j)
		if(vram->nameList[j] == textureSet->textureList[i]->name) {
			found = true;
			break;
		}
		if(found)
		continue;
		
		//Load texture in VRAM
		theError = Texture_NewFromCompressedTexture(vram->engine, &vram->textureList[vram->textureCount], textureSet->textureList[i], flags);
		if(theError)
		return theError;
		
		//Update name list
		vram->nameList[vram->textureCount] = textureSet->textureList[i]->name;
		
		//Update texture count
		vram->textureCount += 1;
	}
	
	return noErr;
}

OSErr VRAM_LoadTextureList(TextureMemoryPtr vram, CompressedTexturePtr textureList[], long textureCount, long flags)
{
	long					i,
							j;
	OSErr					theError;
	Boolean					found;
	
	for(i = 0; i < textureCount; ++i) {
		//Check for duplicate
		found = false;
		for(j = 0; j < vram->textureCount; ++j)
		if(vram->nameList[j] == textureList[i]->name) {
			found = true;
			break;
		}
		if(found)
		continue;
		
		//Load texture in VRAM
		theError = Texture_NewFromCompressedTexture(vram->engine, &vram->textureList[vram->textureCount], textureList[i], flags);
		if(theError)
		return theError;
		
		//Update name list
		vram->nameList[vram->textureCount] = textureList[i]->name;
		
		//Update texture count
		vram->textureCount += 1;
	}
	
	return noErr;
}

void VRAM_Purge(TextureMemoryPtr vram)
{
	long					i;
	
	//Purge previous textures
	for(i = 0; i < vram->textureCount; ++i)
	if(vram->textureList[i] != nil)
	QATextureDelete(vram->engine, vram->textureList[i]);
	
	//Reset
	vram->textureCount = 0;
}

inline TQATexture* GetTQATexturePtr(OSType texID, TQATexture* textureList[], OSType nameList[], long textureCount)
{
	long				i;
	
	for(i = 0; i < textureCount; ++i)
	if(nameList[i] == texID)
	return textureList[i];
	
	return nil;
}

static void Audio_DrawShape(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip)
{
	/*ShapeData_AudioPtr	data;
	
	//Extract audio shape data
	if(shape->dataPtr == nil)
	return;
	data = (ShapeData_AudioPtr) shape->dataPtr;
	
	//Just update the world position vector
	Matrix_TransformVector(globalPos, &data->position, &data->worldPosition);*/
}

static void DrawShape_Blind(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip)
{
	;
}

void Shape_DisableRendering(ShapePtr shape)
{
	shape->renderingProc = (ProcPtr) DrawShape_Blind;
}

void Shape_ResolveRendering(ShapePtr shape)
{
	switch(shape->shading) {
	
		//Geometric shaders
		
		case kShader_None:
		if(shape->texture != kNoTexture)
		shape->renderingProc = (ProcPtr) Texture_DrawShape_NoShading;
		else
		shape->renderingProc = (ProcPtr) Gouraud_DrawShape_NoShading;
		break;
		
		case kShader_Lambert:
		if(shape->texture != kNoTexture)
		shape->renderingProc = (ProcPtr) Texture_DrawShape_LambertShading;
		else
		shape->renderingProc = (ProcPtr) Gouraud_DrawShape_LambertShading;
		break;
		
		case kShader_Phong:
		if(shape->texture != kNoTexture)
		shape->renderingProc = (ProcPtr) Texture_DrawShape_PhongShading;
		else
		shape->renderingProc = (ProcPtr) Gouraud_DrawShape_PhongShading;
		break;
		
		case kShader_Glow:
		if(shape->texture != kNoTexture)
		shape->renderingProc = (ProcPtr) Texture_DrawShape_GlowShading;
		else
		shape->renderingProc = (ProcPtr) Gouraud_DrawShape_GlowShading;
		break;
		
		//FX shaders
		
		case kShader_Sprite:
		if(shape->texture != kNoTexture)
		shape->renderingProc = (ProcPtr) Sprite_DrawShape;
		else
		shape->renderingProc = (ProcPtr) DrawShape_Blind;
		break;
		
		case kShader_Light:
		shape->renderingProc = (ProcPtr) Light_DrawShape;
		break;
		
		case kShader_Particle:
		if(shape->texture != kNoTexture)
		shape->renderingProc = (ProcPtr) Particle_DrawShape;
		else
		shape->renderingProc = (ProcPtr) DrawShape_Blind;
		break;
		
		case kShader_Lens:
		if(shape->texture != kNoTexture)
		shape->renderingProc = (ProcPtr) Shape_DrawLens;
		else
		shape->renderingProc = (ProcPtr) DrawShape_Blind;
		break;
		
		case kShader_Audio:
		shape->renderingProc = (ProcPtr) Audio_DrawShape;
		break;
		
		default:
		shape->renderingProc = (ProcPtr) DrawShape_Blind;
		break;
	
	}
}

void Shape_ResolveLinking(ObjectPtr object, ShapePtr shape)
{
	//Resolve linking
	if(shape->link != kNoLink)
	shape->parent = Shape_GetPtrFromID(object, shape->link);
	else
	shape->parent = nil;
}

inline SkeletonPtr Skeleton_GetPtrFromID(SkeletonPtr skeletonList[], long skeletonCount, OSType ID)
{
	long				i;
	
	for(i = 0; i < skeletonCount; ++i)
	if(skeletonList[i]->id == ID)
	return skeletonList[i];
	
	return nil;
}

static void Script_Resolve(ScriptPtr script, ObjectPtr object, SkeletonPtr skeletonList[], long skeletonCount)
{
	long				i,
						j;
	AnimationPtr		animation;
	EventPtr			timeEvent;
	
	if(script->type == kType_Skeleton)
	script->skeletonID = (unsigned long) Skeleton_GetPtrFromID(skeletonList, skeletonCount, script->skeletonID);
	else
	for(i = 0; i < script->animationCount; ++i)
	script->animationList[i]->shapeID = (unsigned long) Shape_GetPtrFromID(object, script->animationList[i]->shapeID);
	
	//Precalculate quaternions
	if(script->flags & kFlag_UseQuaternions)
	for(i = 0; i < script->animationCount; ++i) {
		animation = script->animationList[i];
		timeEvent = animation->eventList;
		if(animation->type == kType_Animation)
		for(j = 0; j < animation->eventCount; ++j, ++timeEvent)
		Quaternion_EulerToQuaternion(-timeEvent->rotateX, -timeEvent->rotateY, -timeEvent->rotateZ, (QuaternionPtr) &timeEvent->rotateX);
	}
}

void MegaObject_Resolve(MegaObjectPtr megaObject, TextureMemoryPtr vram)
{
	long					i;
	ShapePtr				shape;
	SkeletonPtr				skeleton;
	ScriptPtr				script;
	
	//Prepare shapes
	for(i = 0; i < megaObject->object.shapeCount; ++i) {
		shape = megaObject->object.shapeList[i];
		
		//Resolve texture
		if(shape->texture != kNoTexture)
		shape->texturePtr = GetTQATexturePtr(shape->texture, vram->textureList, vram->nameList, vram->textureCount);
		
		//Resolve linking
		Shape_ResolveLinking(&megaObject->object, shape);
		
		//Resolve rendering
		Shape_ResolveRendering(shape);
	}
	
	//Prepare skeletons
	for(i = 0; i < megaObject->skeletonCount; ++i) {
		skeleton = megaObject->skeletonList[i];
		
		skeleton->shapePtr = Shape_GetPtrFromID(&megaObject->object, skeleton->shapeID);
	}
	
	//Prepare scripts
	for(i = 0; i < megaObject->scriptCount; ++i) {
		script = megaObject->scriptList[i];
		
		Script_Resolve(script, &megaObject->object, megaObject->skeletonList, megaObject->skeletonCount);
	}
}

void MegaObject_AttachTextureSet(MegaObjectPtr megaObject, TextureSetPtr textureSet)
{
	long					i;
	
	//Update shape texture IDs
	for(i = 0; i < megaObject->object.shapeCount; ++i)
	megaObject->object.shapeList[i]->texture = textureSet->textureTable[i];
}
