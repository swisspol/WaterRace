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


#include				<RAVE.h>

#ifndef __INFINITY_STRUCTURES__
#define __INFINITY_STRUCTURES__

//CONSTANTES:

#define					kNoTexture				(-1)
#define					kNoLink					(-1)
#define					kNoNode					(-1)
#define					kNoID					'None'

#define					kBBSize					8

#define					kPi						3.141592653589793
#define					kTrigTableResolution	4096 //16Kb table
#define					kTrigTableMask			(kTrigTableResolution - 1)

#define					kHugeDistance			10000.0

#define					kMaxShapes				200

#define					kMaxAnimations			32
#define					kTimeUnit				100

//Normal modes
enum {
	kNoNormals = -1,
	kPerVertexNormals = 0
};

//Shape flags
#define					kFlag_RelativePos		(1 << 0)
#define					kFlag_MayHide			(1 << 1)
#define					kFlag_Hidden			(1 << 2)
#define					kFlag_NotInObjectBBox	(1 << 4)
#define					kFlag_HasSkeleton		(1 << 16)

//Shaders
#define					kShader_GeometricOffset	0
#define					kShader_FXOffset		16
enum {
	kShader_None = kShader_GeometricOffset, //Classical shaders - need 3D geometry
	kShader_Lambert,
	kShader_Phong,
	kShader_Glow,
	kShader_Sprite = kShader_FXOffset, //Special FX shaders - no 3D geometry needed
	kShader_Light,
	kShader_Particle,
	kShader_Lens,
	kShader_Audio
};
#define IsFXShape(s) ((s)->shading >= kShader_FXOffset)
#define IsGeometricShape(s) ((s)->shading < kShader_FXOffset)

//Texture flags
#define					kFlag_MipMap			(1 << 0)
#define					kFlag_Dithering			(1 << 1)

//Texture constants
#define					kMinTextureReduce		32
#define					kMaxTextureSizeH		1024
#define					kMaxTextureSizeV		512
#define					kMaxMipmapSizeH			512
#define					kMaxMipmapSizeV			kMaxMipmapSizeH
#define					kMax3DfxTextureSizeH	256
#define					kMax3DfxTextureSizeV	kMax3DfxTextureSizeH

enum {kAlpha_None = 0, kAlpha_1Bit = 1, kAlpha_8Bits = 8};

//Script types
#define					kType_Shape				'shpe'
#define					kType_Skeleton			'sklt'

//Script flags
#define					kFlag_Loop				(1 << 0)
#define					kFlag_ResetOnStop		(1 << 1)
#define					kFlag_SmoothStart		(1 << 2)
#define					kFlag_UseQuaternions	(1 << 3)
#define					kFlag_Running			(1 << 31)

//Animation types
#define					kType_Animation			'anim'
#define					kType_SoundAnimation	'snd '
#define					kType_SubTitle			'sbtl'

//Animation flags
#define					kFlag_Translation		(1 << 0)

//Custom containers flags
#define					kFlag_DataLocked		(1 << 0)

//Skeleton flags
#define					kFlag_VerticesAttached	(1 << 16)

//Skeleton constants
#define					kSkeleton_MaxVertices	128
#define					kSkeleton_MaxChildNodes	6
#define					kSkeleton_MaxNodes		40

//MegaObject constants
#define					kMega_MaxScripts		64
#define					kMega_MaxCameras		16
#define					kMega_MaxSkeletons		kMaxShapes
#define					kMega_MaxCollisionBoxes	8

//Version constants
#define					kVersion_MegaObject		0x0310
#define					kVersion_Shape			0x0100
#define					kVersion_Texture		0x0100
#define					kVersion_TextureSet		0x0100
#define					kVersion_Script			0x0100
#define					kVersion_Skeleton		0x0100
#define					kVersion_Camera			0x0101
#define					kVersion_CameraLayout	0x0100

//MACROS:

#define DegreesToRadians(x)	((float)((x) * kPi / 180.0))
#define RadiansToDegrees(x)	((float)((x) * 180.0 / kPi))

#define Square(x) ((x) * (x))

//VARIABLES:

extern float		_sinTable[kTrigTableResolution],
					_cosTable[kTrigTableResolution];

//INLINE ROUTINES

inline float fAbs(float x) {return ((x) < 0 ? (-(x)) : (x));}
inline long iAbs(long x) {return ((x) < 0 ? (-(x)) : (x));}

inline float fMax(float x, float y) {return ((x) >= (y) ? (x) : (y));};
inline float fMin(float x, float y) {return ((x) <= (y) ? (x) : (y));};
inline long iMax(long x, long y) {return ((x) >= (y) ? (x) : (y));};
inline long iMin(long x, long y) {return ((x) <= (y) ? (x) : (y));};

inline float TSin(float x)
{
	if(x >= 0.0)
	return _sinTable[((long) ((x) * kTrigTableResolution / (2.0 * kPi))) & kTrigTableMask];
	else
	return -_sinTable[((long) ((-x) * kTrigTableResolution / (2.0 * kPi))) & kTrigTableMask];
}

inline float TCos(float x)
{
	if(x >= 0.0)
	return _cosTable[((long) ((x) * kTrigTableResolution / (2.0 * kPi))) & kTrigTableMask];
	else
	return _cosTable[((long) ((-x) * kTrigTableResolution / (2.0 * kPi))) & kTrigTableMask];
}

inline float FastSqrt(float x)
{
	float		isqrt,
				temp1,
				temp2;
	
	if(x == 0.0)
	return 0.0;
	
	isqrt = __frsqrte(x);
	temp1 = x * -0.5;
	temp2 = isqrt * isqrt;
	temp1 *= isqrt;
	isqrt *= 1.5;
	
	return 1.0 / (temp1 * temp2 + isqrt);
}

//STRUCTURES:

//Data definitions:

typedef struct {
	float			x,
					y,
					z;
} Vector;
typedef Vector* VectorPtr;

typedef struct {
	Vector			point;
	float			u;
	float			v;
	float			c;
} Vertex;
typedef Vertex* VertexPtr;

typedef struct {
	unsigned long	corner[3];
} TriFace;
typedef TriFace* TriFacePtr;

typedef struct {
	Vector			x;
	Vector			y;
	Vector			z;
	Vector			w;
} Matrix;
typedef Matrix* MatrixPtr;

//3D definitions:

typedef struct Shape Shape;
typedef Shape* ShapePtr;

struct Shape {
	long			pointCount;
	VertexPtr		pointList;
	long			triangleCount;
	TriFacePtr		triangleList;
	
	short			normalMode; //set to kNoNormals if no normals
	ProcPtr			renderingProc;
	long			normalCount;
	VectorPtr		normalList;
	
	unsigned long	flags;
	Matrix			pos;
	float			rotateX;
	float			rotateY;
	float			rotateZ;
	float			scale;
	long			link;
	
	Vector			boundingBox[kBBSize];
	
	OSType			texture;
	float			alpha;
	Ptr				dataPtr;
	float			difuse;
	float			specular;
	
	unsigned int	backfaceCulling : 1;
	short			unused_1;
	unsigned short	shading;
	short			unused_2;
	
	Str31			name;
	
	unsigned char	buffer[8];
	
	TQATexture*		texturePtr;
	ShapePtr		parent;
	
	float			sphereRadius;
	Vector			sphereCenter;
	
	OSType			id;
};

typedef struct {
	unsigned long	shapeCount;
	ShapePtr		shapeList[kMaxShapes];
	
	unsigned long	flags;
	Matrix			pos;
	float			rotateX;
	float			rotateY;
	float			rotateZ;
	float			scale;
	
	Vector			boundingBox[kBBSize];
	
	Str31			name;
	
	unsigned char	unused[16];
	
	float			sphereRadius;
	Vector			sphereCenter;
	
	OSType			id;
} Object;
typedef Object* ObjectPtr;
typedef Object** ObjectHandle;

typedef struct {
	float			d;	// the distance to the near clipping plane
	float			f;	// the distance to the far clipping plane
	float			h;	// half of the view width at distance d from camera
	float			c; //the hide distance
	
	unsigned int	doubleBuffer : 1;
	unsigned int	noZBuffer : 1;
	unsigned int	alwaysVisible: 1;
	short			antialiasing;
	unsigned int	perspectiveZ : 1;
	short			textureFilter;
	
	Vector			lightVector;
	float			ambient;

	TQAEngine*		engine;
	TQADrawContext*	drawContext;
	
	Rect			viewRect;
	float			viewHeight;
	float			viewWidth;
	
	//Data used by Infinity V3
	TQATexture*		lastTexture;
	float			p1,
					p2,
					pixelConversion,
					projectionOffset_X,
					projectionOffset_Y,
					zoom,
					invRange;
	Boolean			linearZBuffer;
	unsigned long	frameTime,
					lastFrameTime;
} State;
typedef State* StatePtr;

typedef struct {
	float			roll; //rotation around X axis
	float			pitch; //rotation around Y axis
	float			yaw; //rotation around Z axis
	Matrix			camera;
} CameraState;
typedef CameraState* CameraStatePtr;

//Texture definitions:

typedef struct {
	OSType			name;
	unsigned long	flags;
	
	unsigned short	width,
					height,
					depth; //for information only - depth of the source image
	PicHandle		image;
	unsigned long	uncompressedSize; //approximated required VRAM
	
	short			alphaDepth;
	unsigned char	alphaLayerData[];
} CompressedTexture;
typedef CompressedTexture* CompressedTexturePtr;
typedef CompressedTexture** CompressedTextureHandle;

typedef struct {
	Str63			name;
	OSType			id;
	unsigned long	flags;
	long			size; //incorrect calculation!!!
	OSType			textureTable[kMaxShapes];
	
	unsigned long			textureCount;
	CompressedTexturePtr	textureList[kMaxShapes];
} TextureSet;
typedef TextureSet* TextureSetPtr;
typedef TextureSet** TextureSetHandle;

//Orientation representations:

// quaternion is represented as (w,[x,y,z])
// where: w       - scalar part
//        x, y, z - vector part

typedef struct Quaternion_Definition { 
  float w, x, y, z;
};
typedef Quaternion_Definition Quaternion;
typedef Quaternion_Definition* QuaternionPtr;

//Cinematic definitions:

typedef struct {
	long			time;
	
	float			rotateX,
					rotateY,
					rotateZ,
					rotateW;
	Vector			position;
} Event;
typedef Event* EventPtr;

typedef struct {
	long			time;
	
	OSType			soundID;
	char			unused[24];
} SoundEvent;
typedef SoundEvent* SoundEventPtr;

typedef struct {
	long			time;
	
	OSType			subTitleID;
	long			subTitleNum;
	char			unused[20];
} SubTitleEvent;
typedef SubTitleEvent* SubTitleEventPtr;

typedef struct {
	OSType			shapeID;
	unsigned long	flags;
	OSType			type;
	
	long			endTime;
	Boolean			running;
	
	unsigned long	eventCount;
	Event			eventList[];
} CinematicAnimation;
typedef CinematicAnimation Animation;
typedef CinematicAnimation* AnimationPtr;

typedef struct {
	Str31			name;
	unsigned long	flags;
	OSType			id;
	
	OSType			type;
	OSType			skeletonID;
	
	long			startTime;
	long			length;
	long			loopTime;
	
	unsigned long	animationCount;
	AnimationPtr	animationList[kMaxAnimations];
} CinematicScript;
typedef CinematicScript Script;
typedef CinematicScript* ScriptPtr;
typedef CinematicScript** ScriptHandle;

//Camera definition:

typedef struct {
	Str31			name;
	char			unused[28];
	long			visibility;
	unsigned long	flags;
	OSType			id;
	
	Matrix			pos;
	float			roll; //rotation around X axis
	float			pitch; //rotation around Y axis
	float			yaw; //rotation around Z axis
	float			zoom;
} Camera_Definition;
typedef Camera_Definition Camera;
typedef Camera_Definition* CameraPtr;

//Skeleton definitions:

typedef struct {
	Vector				vector,
						normal;
} ShapePoint_Definition;
typedef ShapePoint_Definition ShapePoint;
typedef ShapePoint_Definition* ShapePointPtr;

typedef struct {
	unsigned long		parentNode,
						childNodeCount,
						childNodeList[kSkeleton_MaxChildNodes];
	
	Matrix				pos;
	float				rotateX,
						rotateY,
						rotateZ;
	
	unsigned long		verticeCount;
	unsigned long		verticeList[kSkeleton_MaxVertices];
	
	Str31				name;
} Node_Definition;
typedef Node_Definition Node;
typedef Node_Definition* NodePtr;
typedef Node_Definition** NodeHandle;

typedef struct {
	OSType			id;
	unsigned long	flags;
	OSType			shapeID;
	
	unsigned long	nodeCount;
	Node			nodeList[kSkeleton_MaxNodes];
	
	ShapePtr		shapePtr;
	
	long			unused[3];
	
	ShapePoint		points[];
} Skeleton_Definition;
typedef Skeleton_Definition Skeleton;
typedef Skeleton_Definition* SkeletonPtr;

typedef struct {
	Vector			center;
	Vector			xVector,
					yVector,
					zVector;
	
	float			halfSize_x,
					halfSize_y,
					halfSize_z;
	Matrix			matrix;
	Vector			corners[kBBSize];
	
	long			unused[4];
} CollisionBox;
typedef CollisionBox* CollisionBoxPtr;

typedef struct {
	Object			object;
	
	long			skeletonCount;
	SkeletonPtr		skeletonList[kMega_MaxSkeletons];
	
	long			scriptCount;
	ScriptPtr		scriptList[kMega_MaxScripts];
	
	long			cameraCount;
	CameraPtr		cameraList[kMega_MaxCameras];
	
	unsigned long	boxCount;
	CollisionBox	boxList[kMega_MaxCollisionBoxes];
} MegaObject;
typedef MegaObject* MegaObjectPtr;
typedef MegaObject** MegaObjectHandle;

#endif
