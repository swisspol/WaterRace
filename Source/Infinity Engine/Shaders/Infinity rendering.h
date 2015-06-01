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


#ifndef __INFINITY_RENDERING__
#define __INFINITY_RENDERING__

#include				"Matrix.h"

//CONSTANTES PREPROCESSEURS:

#define	__BENCHMARK__	0

//CONSTANTES:

#define					kInfinityRenderingFlag_Filtering		(1 << 0)
#define					kInfinityRenderingFlag_DeepZ			(1 << 1)
#define					kInfinityRenderingFlag_Aliasing			(1 << 2)
#define					kInfinityRenderingFlag_LowResolution	(1 << 3)
#define					kInfinityRenderingFlag_NonLinearZBuffer	(1 << 4)
#define					kInfinityRenderingFlag_TriFiltering		(1 << 5)

#define					kMaxTextures							512
#define					kDefaultSpriteByteOffset				sizeof(InfinitySprite)

enum {kUndetermined = -2, kNotVisible = -1, kNoClipping = 0, kZClipping = 1, kXYClipping = 2};

//Shape data flags
#define					kFlag_FrontDisplay		(1 << 0)
#define					kFlag_ConstantSize		(1 << 1)

#define					kFlag_GlobalSystem		(1 << 0)
#define					kFlag_NoZBuffer			(1 << 1)
#define					kFlag_Premultiplied		(1 << 2)
#define					kFlag_ReboundInBBox		(1 << 3)
#define					kFlag_KeepInBBox		(1 << 4)
#define					kFlag_LocalVelocity		(1 << 5)
#define					kFlag_AutoKill			(1 << 6)

#define					kFlag_FakePosition		(1 << 0)
#define					kFlag_RandomAudioPlay	(1 << 0)

//STRUCTURES:

//Sprite
typedef struct {
	Vector			position;
	float			size;
	float			transparency,
					intensity;
	float			hiColor_r,
					hiColor_g,
					hiColor_b;
} InfinitySprite;
typedef InfinitySprite* InfinitySpritePtr;

typedef struct {
	InfinitySprite	sprite;
	
	Vector			position,
					oldPosition;
	Vector			velocity;
	float			energy;
	Boolean			alive;
} InfinityParticle;
typedef InfinityParticle* InfinityParticlePtr;

//Special shape data
typedef struct {
	InfinitySprite	sprite;
	
	Vector			localPosition;
	float			frontOffset;
	unsigned long	flags;
} ShapeData_Sprite;
typedef ShapeData_Sprite* ShapeData_SpritePtr;

typedef struct {
	Matrix			matrix;
	
	float			topBase,
					bottomBase,
					length;
					
	float			intensity;
	float			color_r,
					color_g,
					color_b;
} ShapeData_Light;
typedef ShapeData_Light* ShapeData_LightPtr;

typedef struct {
	long			particleCount;
	unsigned long	lastTime;
	unsigned long	flags;
	
	float			gravity,
					velocityDecrease,
					energyDecrease,
					sizeDecrease;
					
	Vector			initPosition,
					initPositionRandom;
	Vector			initVelocity,
					initVelocityRandom;
	float			initEnergy,
					initEnergyRandom;
	float			initSize,
					initSizeRandom;
					
	float			modulate_r,
					modulate_g,
					modulate_b;
} ShapeData_Particle;
typedef ShapeData_Particle* ShapeData_ParticlePtr;

typedef struct {
	float			size,
					order,
					depth;
	unsigned long	flags;
	Vector			fakePosition;
} ShapeData_Lens;
typedef ShapeData_Lens* ShapeData_LensPtr;

typedef struct {
	unsigned long	flags;
	
	Vector			position,
					worldPosition;
	
	float			volume,
					pitch;
	float			minDistance,
					maxDistance;
	
	unsigned long	minTime,
					maxTime;
					
	long			unused1,
					unused2,
					unused3,
					unused4;
	
	unsigned long	soundSize;
	ExtSoundHeader	soundHeader;
} ShapeData_Audio;
typedef ShapeData_Audio* ShapeData_AudioPtr;

//Video memory
typedef struct {
	TQAEngine*			engine;
	long				textureCount;
	OSType				nameList[kMaxTextures];
	TQATexture*			textureList[kMaxTextures];
	CompressedTexturePtr	storageList[kMaxTextures];
} TextureMemory;
typedef TextureMemory* TextureMemoryPtr;

//Terrain workaround
typedef struct Terrain_Definition Terrain_Definition;
typedef Terrain_Definition* TerrainPtr;
typedef struct TerrainMesh_Definition TerrainMesh_Definition;
typedef TerrainMesh_Definition* TerrainMeshPtr;

//PROTOTYPES:

typedef void (*Rendering_Proc)(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);

//MACROS:

#define YonDistance2HideDistance(x) Square((x) * 2.0 / 3.0)
#define Shape_CallRenderingProc(shape, state, globalPos, negatedCamera, cameraPos, shapeList, clip) \
	(*((Rendering_Proc) (shape)->renderingProc))((state), (shape), (globalPos), (negatedCamera), (cameraPos), (shapeList), (clip))

#if __BENCHMARK__
#undef QASubmitVerticesGouraud
#define QASubmitVerticesGouraud(drawContext,nVertices,vertices) \
		_verticeCount += nVertices; (drawContext)->submitVerticesGouraud(drawContext,nVertices,vertices)
#undef QASubmitVerticesTexture
#define QASubmitVerticesTexture(drawContext,nVertices,vertices) \
		_verticeCount += nVertices; (drawContext)->submitVerticesTexture(drawContext,nVertices,vertices)
#undef QADrawTriMeshGouraud
#define QADrawTriMeshGouraud(drawContext,nTriangle,triangles) \
		_triCount += nTriangle; (drawContext)->drawTriMeshGouraud (drawContext,nTriangle,triangles)
#undef QADrawTriMeshTexture
#define QADrawTriMeshTexture(drawContext,nTriangle,triangles) \
		_triCount += nTriangle; (drawContext)->drawTriMeshTexture (drawContext,nTriangle,triangles)
#undef QARenderStart
#define QARenderStart(drawContext,dirtyRect,initialContext) \
		_verticeCount = 0; _triCount = 0; (drawContext)->renderStart (drawContext,dirtyRect,initialContext)
#endif

//VARIABLES:

extern TQAIndexedTriangle		*trianglesBuffer,
								*trianglesBuffer2;
extern TQAVTexture				*vTextureBuffer;
extern TQAVGouraud				*vGouraudBuffer;

#if __BENCHMARK__
extern	unsigned long			_verticeCount,
								_triCount;
#endif

//ROUTINES:

//File: Infinity rendering.cp
void InfinityRendering_NoClear(StatePtr state);
void InfinityRendering_CopyClear(StatePtr state, Point size, Point position, PixMapHandle copyPixMap);
void InfinityRendering_ColorClear(StatePtr state, float r, float g, float b);
void InfinityRendering_SetLight(StatePtr state, float ambient, float sun, float sun_Rx, float sun_Rz);
void InfinityRendering_UpdateStateParams(StatePtr state);
void InfinityRendering_SetZoom(StatePtr state, float zoom);
void InfinityRendering_SetClipping(StatePtr state, float hither, float yon);
OSErr InfinityRendering_CreateContext(GDHandle screenDevice, TQAEngine* theEngine, StatePtr* state, CameraStatePtr cameraState, Point size, Point position, unsigned long contextFlags);
OSErr InfinityRendering_DisposeContext(StatePtr theState);
OSErr InfinityRendering_AllocateBuffers();
void InfinityRendering_DisposeBuffers();
long InfinityRendering_AvailableVRAM(TQAEngine* theEngine);
void InfinityRendering_MaxFPS(float maxFPS);

void VRAM_Clear(TextureMemoryPtr vram, TQAEngine* engine);
OSErr VRAM_LoadTextureSet(TextureMemoryPtr vram, TextureSetPtr textureSet, long flags);
OSErr VRAM_LoadTextureList(TextureMemoryPtr vram, CompressedTexturePtr textureList[], long textureCount, long flags);
void VRAM_Purge(TextureMemoryPtr vram);

void Shape_ResolveRendering(ShapePtr shape);
void Shape_ResolveLinking(ObjectPtr object, ShapePtr shape);
void Shape_DisableRendering(ShapePtr shape);

void MegaObject_Resolve(MegaObjectPtr megaObject, TextureMemoryPtr vram);
void MegaObject_AttachTextureSet(MegaObjectPtr megaObject, TextureSetPtr textureSet);

//File: Infinity rendering Gouraud.cp
void Gouraud_DrawShape_NoShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void Gouraud_DrawShape_LambertShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void Gouraud_DrawShape_PhongShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void Gouraud_DrawShape_GlowShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);

//File: Infinity rendering Texture.cp
void Texture_DrawShape_NoShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void Texture_DrawShape_LambertShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void Texture_DrawShape_PhongShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void Texture_DrawShape_GlowShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);

//File: Infinity rendering WireFrame.cp
void WireFrame_DrawShape_NoShading(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void WireFrame_DrawShape_Opaque(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void WireFrame_SetColorMode(float r, float g, float b, Boolean overrideGouraudColors);

//File: Infinity rendering Line.cp
void Gouraud_DrawLine(State *state, TQAVGouraud* v1, TQAVGouraud* v2);

//File: Infinity rendering Sprite.cp
void Texture_DrawSprites(StatePtr state, long spriteCount, InfinitySpritePtr spriteList, long byteOffset, TQATexture* texture, Boolean scale);
void Sprite_DrawShape(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);
void Texture_DrawSpriteOnScreen(StatePtr state, float x, float y, float z, float iw, float width, float height, TQATexture* texture);

//File: Infinity rendering Light.cp
void Light_DrawShape(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);

//File: Infinity rendering Particle.cp
void Particle_Init(InfinityParticlePtr particle, ShapeData_ParticlePtr system, MatrixPtr systemMatrix);
void Particle_DrawShape(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);

//File: Infinity rendering Lens.cp
void Shape_DrawLens(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip);

//File: Infinity rendering Terrain.cp
void Terrain_RenderSky(ShapePtr shape, StatePtr state, MatrixPtr localPos, MatrixPtr negatedCamera);
void Terrain_RenderMesh_Textured(TerrainMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos);
void Terrain_RenderMesh_WireFrame(TerrainMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos, float r, float g, float b);

#endif
