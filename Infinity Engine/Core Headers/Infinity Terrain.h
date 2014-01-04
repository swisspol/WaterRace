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


#ifndef __INFINITY_TERRAIN__
#define __INFINITY_TERRAIN__

//CONSTANTES:

//Textures IDs
#define				kTextureID_Sky				'sky '

//Fog
enum {kFog_None = 1, kFog_Linear, kFog_Exponential, kFog_ExponentialSquared};

//Sky:
#define				kLonBands				20
#define				kLatBands				8
#define				kSkyVScale				1.9
#define				kSkyDefaultRadius		50.0
#define				kSkyDefaultHeight		-7.0

//Sea
#define				kTrigLookUpTableSize	1024
#define				kTrigLookUpTableMask	(kTrigLookUpTableSize - 1)
#define				kSeaWaveSpeed			0.5
#define				kWaveHeight				0.012
#define				kWaveDistance			8
#define				kWaveDistance2 			6
#define				kWave_Length			1024
#define				kWave_LengthMask		(kWave_Length - 1)

//Camera
#define				kCameraStartHeight		1.0
#define				kCameraMinHeight		0.1
#define				kCameraMaxHeight		4.0
#define				kCameraZoom				0.8

//Source types
#define				kSource_Sound			'snd '
#define				kSource_Fog				'fog '

//Terrain flags
#define				kTerrainFlag_NoFogOnSky		(1 << 1)
#define				kFlag_Colorize				(1 << 16)

//Sky flags
#define				kSkyFlag_RenderLF		(1 << 0)
#define				kSkyFlag_RenderMoon		(1 << 1)
#define				kSkyFlag_RenderSnow		(1 << 2)
#define				kSkyFlag_RenderRain		(1 << 3)
#define				kSkyFlag_Tile			(1 << 16)

//Mesh flags
#define				kFlag_RemoveBF			(1 << 0)
#define				kFlag_Modulate			(1 << 1)
#define				kFlag_Highlight			(1 << 2)
#define				kFlag_UseMatrix			(1 << 3)
#define				kFlag_NoPostFX			(1 << 5)
#define				kFlag_MeshHidden		(1 << 6)

//Camera flags
#define				kFlag_Follow_H			(1 << 0)
#define				kFlag_Follow_HV			(1 << 1)
#define				kFlag_AutoZoom			(1 << 2)
#define				kFlag_PreserveDistance	(1 << 3)

//Sound flags
#define				kFlag_RandomPlay		(1 << 1)
#define				kFlag_Everywhere		(1 << 2)

//Reference flags
#define				kRefFlag_Float			(1 << 0)
#define				kRefFlag_CollisionDetection	(1 << 1)
#define				kRefFlag_Roll			(1 << 2)
#define				kRefFlag_Locked			(1 << 16)

//Animation flags
#define				kAnimFlag_CollisionDetection	(1 << 0)
#define				kAnimFlag_Loop			(1 << 1)
#define				kAnimFlag_AutoRemove	(1 << 2)

//Fly through camera fake ID
#define				kFlyThroughID			'flth'
#define				kFlyThroughZoom			0.8

//Track flags
#define				kTrackFlag_Hidden		(1 << 0)
#define				kCheckFlag_IsCheckPoint	(1 << 0)

//Visibility flags
#define				kVisible_Easy			(1 << 0)
#define				kVisible_Medium			(1 << 1)
#define				kVisible_Hard			(1 << 2)
#define				kVisible_FlyThrough		(1 << 16)

//Special items
#define				kItem_Bonus				'Bnus'
#define				kItem_Turbo				'Trbo'
#define				kItem_Joker				'Jker'
#define				kItem_RotateSpeed		1.0 //1rpm
#define				kItem_BonusHeight		0.5

//Max values
#define				kMaxSea					128
#define				kMaxMesh				128
#define				kMaxReferences_Mesh		128
#define				kMaxReferences_Sea		kMaxReferences_Mesh
#define				kMaxSources				64
#define				kMaxCameras				64
#define				kMaxAmbientSounds		64
#define				kMaxModelAnimations		64
#define				kMaxTracks				8
#define				kMaxItems				8
#define				kMaxEnclosures			32

//Version constants
#define				kVersion_Terrain		0x0104
#define				kVersion_TerrainMesh	0x0100
#define				kVersion_SeaMesh		0x0100
#define				kVersion_View			0x0100
#define				kVersion_CameraPosition	0x0100
#define				kVersion_AmbientSound	0x0100
#define				kVersion_ModelAnimation	0x0100
#define				kVersion_Track			0x0100
#define				kVersion_ItemList		0x0100
#define				kVersion_Enclosure		0x0100

//STRUCTURES:

typedef struct {
	OSType			itemID;
	unsigned long	flags;
	
	long			visibility;
	Vector			position;
	
	long			unused;
} SpecialItemReference_Definition;
typedef SpecialItemReference_Definition SpecialItemReference;
typedef SpecialItemReference_Definition* SpecialItemReferencePtr;

typedef struct {
	OSType			id;
	unsigned long	flags;
	
	OSType			itemType,
					modelID;
	long			itemValue;
	
	long			unused;
} SpecialItem_Definition;
typedef SpecialItem_Definition SpecialItem;
typedef SpecialItem_Definition* SpecialItemPtr;

typedef struct {
	long			time;
	
	float			rotateX,
					rotateY,
					rotateZ,
					rotateW;
	Vector			position;
} KeyFrame_Definition;
typedef KeyFrame_Definition KeyFrame;
typedef KeyFrame* KeyFramePtr;

typedef struct {
	OSType			modelID,
					flyID;
	unsigned long	flags;
	
	Matrix			pos;
	float			rotateX;
	float			rotateY;
	float			rotateZ;
	
	long			visibility;
	
	float			unused1;
	long			loopTime;
	
	long			endTime;
	Boolean			running;
	
	unsigned long	frameCount;
	KeyFrame		frameList[];
} ModelAnimation_Definition;
typedef ModelAnimation_Definition ModelAnimation;
typedef ModelAnimation_Definition* ModelAnimationPtr;

typedef struct {
	Str31			name;
	OSType			id;
	unsigned long	flags;
	
	unsigned long	minTime,
					maxTime;
	
	long			unused[2];
	
	float			volume,
					panoramic,
					pitch;
	
	unsigned long	soundSize;
	ExtSoundHeader	soundHeader;
} AmbientSound_Definition;
typedef AmbientSound_Definition AmbientSound;
typedef AmbientSound_Definition* AmbientSoundPtr;

typedef struct {
	OSType			sourceType;
	OSType			soundID;
	Vector			position;
	float			maxDistance,
					minDistance;
	long			visibility;
	
	float			volume,
					pitch;
					
	long			unused[2];
} Source_Definition;
typedef Source_Definition Source;
typedef Source_Definition* SourcePtr;

typedef struct {
	float			kd_r,
					kd_g,
					kd_b;
	float			ks_r,
					ks_g,
					ks_b;
} Lightning_Definition;
typedef Lightning_Definition Lightning;
typedef Lightning_Definition* LightningPtr;

typedef struct {
	OSType				id;
	unsigned long		flags;
	
	long				visibility;
	
	Matrix				pos;
	float				rotateX,
						rotateY,
						rotateZ;
	float				scale;
	
	float				inertia;
	
	long				unused[4];
} ModelReference_Definition;
typedef ModelReference_Definition ModelReference;
typedef ModelReference_Definition* ModelReferencePtr;

typedef struct {
	Str31			name;
	OSType			id;
	unsigned long	flags;
	
	Matrix			pos;
	float			rotateX;
	float			rotateY;
	float			rotateZ;
	float			scaleX,
					scaleY,
					scaleZ;
					
	long			pointCount;
	VertexPtr		pointList;
	LightningPtr	lightningList;
	long			triangleCount;
	TriFacePtr		triangleList;
	
	Vector			boundingBox[kBBSize];
	long			clipping;
	
	OSType			texture;
	float			diffuse;
	
	float			alpha;
	
	TQATexture*		texturePtr;
	
	char			unusedArray[252];
	
	unsigned long	lastUpdateTime;
	
	unsigned long	referenceCount;
	ModelReference	referenceList[kMaxReferences_Sea];
} SeaMesh_Definition;
typedef SeaMesh_Definition SeaMesh;
typedef SeaMesh_Definition* SeaMeshPtr;

typedef struct {
	Str31			name;
	OSType			id;
	unsigned long	flags;
	
	Matrix			pos;
	float			rotateX;
	float			rotateY;
	float			rotateZ;
	float			scaleX,
					scaleY,
					scaleZ;
	
	long			pointCount;
	VertexPtr		pointList;
	LightningPtr	lightningList;
	long			triangleCount;
	TriFacePtr		triangleList;
	
	Vector			boundingBox[kBBSize];
	long			clipping;
	
	OSType			texture;
	float			diffuse;
	
	VectorPtr		normalList;
	
	TQATexture*		texturePtr;
	
	char			unusedArray[252];
	
	unsigned long	referenceCount;
	ModelReference	referenceList[kMaxReferences_Mesh];
} TerrainMesh_Definition;
typedef TerrainMesh_Definition TerrainMesh;
typedef TerrainMesh_Definition* TerrainMeshPtr;

typedef struct {
	unsigned long		flags;
	
	Vector				rightMark,
						leftMark;
						
	unsigned char		unused[40];
} CheckPoint_Definition;
typedef CheckPoint_Definition Check;
typedef CheckPoint_Definition* CheckPtr;

typedef struct {
	Str31				name;
	OSType				id;
	unsigned long		flags;
	
	float				color_r,
						color_g,
						color_b;
	
	long				lapNumber;
	
	unsigned char		unused[256];
	
	unsigned long		checkCount;
	Check				checkList[];
} RaceTrack_Definition;
typedef RaceTrack_Definition RaceTrack;
typedef RaceTrack_Definition* RaceTrackPtr;

typedef struct {
	Vector			left,
					right;
	
	Matrix			inverseMatrix;
	float			width;
	
	long			unused[4];
} CollisionWall;
typedef CollisionWall* CollisionWallPtr;

typedef struct {
	unsigned long	flags;
	Boolean			closed;
	
	float			bounding_min_x,
					bounding_max_x,
					bounding_min_z,
					bounding_max_z;
	
	long			unused[4];
	
	long			wallCount;
	CollisionWall	wallList[];
} CollisionEnclosure;
typedef CollisionEnclosure* CollisionEnclosurePtr;

typedef struct {
	Str63			name;
	OSType			id;
	unsigned long	flags;
	
	unsigned char	unused[124];
	
	long			enclosureCount;
	CollisionEnclosurePtr	enclosureList[kMaxEnclosures];
					
	float			timeScale,
					waveAmplitude,
					waveScaleX,
					waveScaleZ;
	float			seaMoveU,
					seaMoveV;
					
	float			ambientLight,
					sunIntensity,
					sunAngle_X,
					sunAngle_Z;
					
	unsigned long	skyFlags;
	short			numTiles;
	float			skyScale,
					skyRotation;
					
	float			unused_1,
					unused_2,
					unused_3;
					
	short			fogMode;
	float			fogColor_r,
					fogColor_g,
					fogColor_b,
					fogDensity,
					fogStart,
					fogEnd;
	
	float			clippingDistance;
	float			skyRadius,
					skyHeight;
	
	unsigned long	itemReferenceCount;
	SpecialItemReferencePtr	itemReferenceList;
	
	unsigned long	specialItemCount;
	SpecialItem		specialItemList[kMaxItems];
	
	unsigned char	unused2[32];
	
	unsigned long	trackCount;
	RaceTrackPtr	trackList[kMaxTracks];
	
	unsigned long	meshCount;
	TerrainMeshPtr	meshList[kMaxMesh];
	unsigned long	seaCount;
	SeaMeshPtr		seaList[kMaxSea];
	
	unsigned long	terrainTextureSize_NoMipMap,
					terrainTextureSize_MipMap,
					terrainTextureSize_FullMipMap;
	unsigned long	modelTextureSize_NoMipMap,
					modelTextureSize_MipMap,
					modelTextureSize_FullMipMap;
					
	long			sourceCount;
	Source			sourceList[kMaxSources];
	
	long			cameraCount;
	CameraPtr		cameraList[kMaxCameras];
	
	long			soundCount;
	AmbientSoundPtr	soundList[kMaxAmbientSounds];
	
	unsigned long	unused3;
	long			animStartTime,
					animLength,
					unused4;
	unsigned long	animCount;
	ModelAnimationPtr	animList[kMaxModelAnimations];
} Terrain_Definition;
typedef Terrain_Definition Terrain;
typedef Terrain_Definition* TerrainPtr;
typedef Terrain_Definition** TerrainHandle;

typedef struct {
	Str31			name;
	unsigned long	flags;
						
	short			width,
					height;
	short			posH,
					posV;
					
	Matrix			camera;
	float			roll; //rotation around X axis
	float			pitch; //rotation around Y axis
	float			yaw; //rotation around Z axis
	float			zoom;
	
	WindowPtr		window;
	StatePtr		state;
	
	void*			refCon;
} View_Definition;
typedef View_Definition View;
typedef View_Definition* ViewPtr;

typedef struct {
	Str31			name;
	float			roll; //rotation around X axis
	float			pitch; //rotation around Y axis
	float			yaw; //rotation around Z axis
	Vector			position;
} CameraPosition;
typedef CameraPosition* CameraPositionPtr;

#endif
