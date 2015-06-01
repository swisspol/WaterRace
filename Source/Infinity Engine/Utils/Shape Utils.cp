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


#include					<fp.h>

#include					"Infinity Structures.h"
#include					"Infinity Error Codes.h"

#include					"Vector.h"
#include					"Infinity Audio Engine.h"

#include					"Infinity Utils.h"
#include					"Infinity Rendering.h"
#include					"Shape Utils.h"
#include					"Clipping Utils.h"

//CONSTANTES:

#define						kShape_ResType			'Shap'
#define						kShape_Version			0x0100

#define						kSpriteBSphereCF		1.732050807568877 //2.0

//STRUTURES:

typedef struct {
	short			version;
	long			shapeSize,
					shapeOffset,
					pointListSize,
					pointListOffset,
					triangleListSize,
					triangleListOffset,
					normalListSize,
					normalListOffset;
	unsigned long	flags;
} ShapeResourceHeader;
typedef ShapeResourceHeader* ShapeResourceHeaderPtr;

//ROUTINES:

static ShapePtr New_DefaultShape()
{
	ShapePtr			shape;
	Vector				nullVector = {0.0, 0.0, 0.0};
	long				i;
	
	shape = (ShapePtr) NewPtrClear(sizeof(Shape));
	if(shape == nil)
	return nil;
	
	//Setup default values
	shape->pointCount = 0;
	shape->pointList = nil;
	shape->triangleCount = 0;
	shape->triangleList = nil;
	
	shape->normalMode = kNoNormals;
	shape->renderingProc = nil;
	shape->normalCount = 0;
	shape->normalList = nil;
	
	shape->flags = 0;
	Matrix_Clear(&shape->pos);
	shape->rotateX = 0.0;
	shape->rotateY = 0.0;
	shape->rotateZ = 0.0;
	shape->scale = 1.0;
	shape->link = kNoLink;
	
	for(i = 0 ; i < kBBSize; ++i)
	shape->boundingBox[i] = nullVector;
	
	shape->texture = kNoTexture;
	shape->dataPtr = nil;
	
	shape->unused_1 = 0;
	shape->shading = kShader_None;
	shape->unused_2 = 0;
	
	shape->name[0] = 0;
	
	shape->texturePtr = nil;
	shape->parent = nil;
	
	shape->sphereRadius = 0.0;
	shape->sphereCenter = nullVector;
	
	shape->id = kNoID;
	
	return shape;
}

ShapePtr Shape_New()
{
	ShapePtr			shape;
	
	shape = New_DefaultShape();
	if(shape == nil)
	return nil;
	
	//Geometry specific
	shape->alpha = 1.0;
	shape->difuse = 1.0;
	shape->specular = 20.0;
	shape->backfaceCulling = 1;
	
	return shape;
}

ShapePtr Shape_New_Sprite(OSType textureID)
{
	ShapePtr			shape;
	ShapeData_SpritePtr	data;
	
	//Init a default shape
	shape = New_DefaultShape();
	if(shape == nil)
	return nil;
	
	//Sprite specific
	data = (ShapeData_SpritePtr) NewPtrClear(sizeof(ShapeData_Sprite));
	if(data == nil) {
		DisposePtr((Ptr) shape);
		return nil;
	}
	data->sprite.position.x = data->sprite.position.y = data->sprite.position.z = 0.0;
	data->sprite.size = 5.0;
	data->sprite.transparency = 1.0;
	data->sprite.intensity = 1.0;
	data->sprite.hiColor_r = data->sprite.hiColor_g = data->sprite.hiColor_b = 0.0;
	
	data->localPosition.x = data->localPosition.y = data->localPosition.z = 0.0;
	data->frontOffset = data->sprite.size / 2.0;
	data->flags = kFlag_FrontDisplay;
	
	//Modify general
	shape->pointCount = 4;
	shape->triangleCount = 2;
	shape->dataPtr = (Ptr) data;
	shape->shading = kShader_Sprite;
	shape->texture = textureID;
	BlockMove("\pFX Sprite", shape->name, sizeof(Str31));
	Shape_CalculateBounding(shape);
	
	return shape;
}

ShapePtr Shape_New_Light()
{
	ShapePtr			shape;
	ShapeData_LightPtr	data;
	
	//Init a default shape
	shape = New_DefaultShape();
	if(shape == nil)
	return nil;
	
	//Light specific
	data = (ShapeData_LightPtr) NewPtrClear(sizeof(ShapeData_Light));
	if(data == nil) {
		DisposePtr((Ptr) shape);
		return nil;
	}
	Matrix_Clear(&data->matrix);
	
	data->topBase = 0.2;
	data->bottomBase = 2.0;
	data->length = 6.0;
	
	data->intensity = 1.0;
	data->color_r = data->color_g = data->color_b = 1.0;
	
	//Modify general
	shape->pointCount = 8;
	shape->triangleCount = 8;
	shape->dataPtr = (Ptr) data;
	shape->shading = kShader_Light;
	BlockMove("\pFX Light", shape->name, sizeof(Str31));
	Shape_CalculateBounding(shape);
	
	return shape;
}

OSErr Shape_ParticleUpdateNumber(ShapePtr shape, long newNumber, MatrixPtr shapeMatrix)
{
	InfinityParticlePtr			newParticleList;
	ShapeData_ParticlePtr		data;
	long						i;
	
	if(shape->shading != kShader_Particle)
	return kError_BadItemType;
	if(shape->dataPtr == nil)
	return kError_BadItemType;
	data = (ShapeData_ParticlePtr) shape->dataPtr;

	newParticleList = (InfinityParticlePtr) NewPtr(newNumber * sizeof(InfinityParticle));
	if(newParticleList == nil)
	return MemError();
	
	for(i = 0; i < newNumber; ++i)
	Particle_Init(&newParticleList[i], data, shapeMatrix);
	
	if(shape->pointList != nil)
	DisposePtr((Ptr) shape->pointList);
	shape->pointList = (VertexPtr) newParticleList;
	data->particleCount = newNumber;
	shape->pointCount = newNumber * 4;
	shape->triangleCount = newNumber * 2;
	
	return noErr;
}

ShapePtr Shape_New_ParticleSystem(OSType textureID, long numParticles)
{
	ShapePtr				shape;
	ShapeData_ParticlePtr	data;
	
	//Init a default shape
	shape = New_DefaultShape();
	if(shape == nil)
	return nil;
	
	//InfinityParticle specific
	data = (ShapeData_ParticlePtr) NewPtrClear(sizeof(ShapeData_Particle));
	if(data == nil) {
		DisposePtr((Ptr) shape);
		return nil;
	}
	
	data->particleCount = numParticles;
	data->lastTime = 0;
	data->flags = 0;
	
	data->gravity = -2.0;
	data->velocityDecrease = 0.5;
	data->energyDecrease = 4.0;
	data->sizeDecrease = 0.0;
	
	data->initPosition.x = data->initPosition.y = data->initPosition.z = 0.0;
	data->initPositionRandom.x = data->initPositionRandom.y = data->initPositionRandom.z = 0.0;
	data->initVelocity.x = 0.0;
	data->initVelocity.y = 6.0;
	data->initVelocity.z = 0.0;
	data->initVelocityRandom.x = -6.0;
	data->initVelocityRandom.y = 5.0;
	data->initVelocityRandom.z = -6.0;
	
	data->initEnergy = 0.0;
	data->initEnergyRandom = 16.0;
	
	data->initSize = 0.5;
	data->initSizeRandom = 0.5;
	
	data->modulate_r = data->modulate_g = data->modulate_b = 1.0;
	
	//Modify general
	shape->pointCount = numParticles * 4;
	shape->triangleCount = numParticles * 2;
	shape->dataPtr = (Ptr) data;
	shape->shading = kShader_Particle;
	shape->texture = textureID;
	BlockMove("\pFX Particle system", shape->name, sizeof(Str31));
	Shape_CalculateBounding(shape);
	
	if(Shape_ParticleUpdateNumber(shape, numParticles, nil) != noErr) {
		DisposePtr((Ptr) shape);
		DisposePtr((Ptr) data);
		return nil;
	}
	
	return shape;
}

ShapePtr Shape_New_Lens(OSType textureID)
{
	ShapePtr			shape;
	ShapeData_LensPtr	data;
	
	//Init a default shape
	shape = New_DefaultShape();
	if(shape == nil)
	return nil;
	
	//Lens specific
	data = (ShapeData_LensPtr) NewPtrClear(sizeof(ShapeData_Lens));
	if(data == nil) {
		DisposePtr((Ptr) shape);
		return nil;
	}
	data->size = 100.0;
	data->order = 1.0;
	data->depth = 0.99;
	data->flags = 0;
	data->fakePosition.x = data->fakePosition.y = data->fakePosition.z = 0.0;
	
	//Modify general
	shape->pointCount = 4;
	shape->triangleCount = 2;
	shape->dataPtr = (Ptr) data;
	shape->texture = textureID;
	shape->shading = kShader_Lens;
	BlockMove("\pFX Lens", shape->name, sizeof(Str31));
	Shape_CalculateBounding(shape);
	
	return shape;
}

ShapePtr Shape_New_Audio(Handle sound)
{
	ShapePtr			shape;
	ShapeData_AudioPtr	data;
	
	//Init a default shape
	shape = New_DefaultShape();
	if(shape == nil) {
		DisposeHandle(sound);
		return nil;
	}
	
	//Audio specific
	data = (ShapeData_AudioPtr) NewPtrClear(sizeof(ShapeData_Audio) + GetHandleSize(sound) - sizeof(ExtSoundHeader));
	if(data == nil) {
		DisposePtr((Ptr) shape);
		DisposeHandle(sound);
		return nil;
	}
	BlockMove(*sound, &data->soundHeader, GetHandleSize(sound));
	data->flags = 0;
	data->minDistance = 30.0;
	data->maxDistance = 50.0;
	data->minTime = 100;
	data->maxTime = 1000;
	data->position.x = data->position.y = data->position.z = 0.0;
	data->worldPosition.x = data->worldPosition.y = data->worldPosition.z = 0.0;
	data->volume = kInfinityAudio_MaxVolume;
	data->pitch = kInfinityAudio_NormalPitch;
	data->unused1 = 0;
	data->unused2 = 0;
	data->unused3 = 0;
	data->unused4 = 0;
	data->soundSize = GetHandleSize(sound);
	
	//Modify general
	shape->dataPtr = (Ptr) data;
	shape->shading = kShader_Audio;
	BlockMove("\pFX Audio", shape->name, sizeof(Str31));
	Shape_CalculateBounding(shape);
	
	//Clean up
	DisposeHandle(sound);
	
	return shape;
}

void Shape_UpdateMatrix(ShapePtr shape)
{
	Vector			p;
	Matrix			m;
	
	p = shape->pos.w;
	Matrix_Clear(&shape->pos);
	if(shape->scale != 1.0)
	Matrix_ScaleLocal(&shape->pos, shape->scale, &shape->pos);
	
	Matrix_SetRotateX(shape->rotateX, &m);
	Matrix_MultiplyByMatrix(&m, &shape->pos, &shape->pos);
	
	Matrix_SetRotateY(shape->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &shape->pos, &shape->pos);
	
	Matrix_SetRotateZ(shape->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &shape->pos, &shape->pos);
	
	shape->pos.w = p;
}
	
void Shape_CalculateBounding(ShapePtr shape)
{
	//We have a normal shape
	if(!IsFXShape(shape)) {
		long			i;
		float			minX = kHugeDistance,
						minY = kHugeDistance,
						minZ = kHugeDistance,
						maxX = -kHugeDistance,
						maxY = -kHugeDistance,
						maxZ = -kHugeDistance,
						radius = 0.0;
		Vector			center,
						point;
	
		//Update bounding box
		for(i = 0; i < shape->pointCount; ++i) {
			
			if(shape->pointList[i].point.x < minX)
			minX = shape->pointList[i].point.x;
			if(shape->pointList[i].point.x > maxX)
			maxX = shape->pointList[i].point.x;
			
			if(shape->pointList[i].point.y < minY)
			minY = shape->pointList[i].point.y;
			if(shape->pointList[i].point.y > maxY)
			maxY = shape->pointList[i].point.y;
			
			if(shape->pointList[i].point.z < minZ)
			minZ = shape->pointList[i].point.z;
			if(shape->pointList[i].point.z > maxZ)
			maxZ = shape->pointList[i].point.z;
			
		}
		SetUpBBox_Corners(minX, minY, minZ, maxX, maxY, maxZ, shape->boundingBox);
		
		//Update shape center
		center.x = (minX + maxX) / 2.0;
		center.y = (minY + maxY) / 2.0;
		center.z = (minZ + maxZ) / 2.0;
		shape->sphereCenter = center;
		
		//Update shape radius
		for(i = 0; i < shape->pointCount; ++i) {
			Vector_Subtract(&shape->pointList[i].point, &center, &point);
			if(Vector_Length(point) > radius)
			radius = Vector_Length(point);
		}
		shape->sphereRadius = radius;
	}
	else switch(shape->shading) {
		
		case kShader_Sprite:
		{
			ShapeData_SpritePtr		data;
			
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_SpritePtr) shape->dataPtr;
	
			if(data->flags & kFlag_ConstantSize) {
				SetUpBBox_Homogene(&data->localPosition, 1.0, shape->boundingBox);
				shape->sphereCenter = data->localPosition;
				shape->sphereRadius = 0.5 * kSpriteBSphereCF;
			}
			else {
				SetUpBBox_Homogene(&data->localPosition, data->sprite.size, shape->boundingBox);
				shape->sphereCenter = data->localPosition;
				shape->sphereRadius = data->sprite.size / 2.0 * kSpriteBSphereCF;
			}
		}
		break;
		
		case kShader_Light:
		{
			ShapeData_LightPtr		data;
			float					bigBase;
			Vector					direction = {0.0,-1.0,0.0};
			long					i;
			
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_LightPtr) shape->dataPtr;
	
			bigBase = fMax(data->topBase, data->bottomBase) / 2.0;
			SetUpBBox_Corners(-bigBase, -data->length, -bigBase, bigBase, 0.0, bigBase, shape->boundingBox);
			for(i = 0; i < kBBSize; ++i)
			Matrix_TransformVector(&data->matrix, &shape->boundingBox[i], &shape->boundingBox[i]);
			
			Matrix_RotateVector(&data->matrix, &direction, &direction);
			Vector_MultiplyAdd(data->length / 2.0, &direction, &data->matrix.w, &shape->sphereCenter);
			shape->sphereRadius = sqrt(data->length * data->length / 4.0 + data->bottomBase * data->bottomBase / 4.0);
		}
		break;
		
		case kShader_Particle:
		{
			ShapeData_ParticlePtr		data;
			
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_ParticlePtr) shape->dataPtr;
	
			SetUpBBox_Homogene(&data->initPosition, 1.0, shape->boundingBox);
			shape->sphereCenter = data->initPosition;
			shape->sphereRadius = 0.5;
		}
		break;
		
		case kShader_Lens:
		case kShader_Audio:
		{
			Vector			nullVector = {0.0,0.0,0.0};
			long			i;
			
			for(i = 0; i < kBBSize; ++i)
			shape->boundingBox[i] = nullVector;
			shape->sphereCenter = nullVector;
			shape->sphereRadius = 0.0;
		}
		break;
		
	}
}

void Shape_LinkMatrixByID(ObjectPtr object, ShapePtr shape, MatrixPtr m)
{
	ShapePtr tempShape_0, tempShape_1, tempShape_2, tempShape_3;
	
	if(shape->link == kNoLink)
	return;
	
	tempShape_0 = Shape_GetPtrFromID(object, shape->link);
	if(tempShape_0->link != kNoLink) {
		tempShape_1 = Shape_GetPtrFromID(object, tempShape_0->link);
		if(tempShape_1->link != kNoLink) {
			tempShape_2 = Shape_GetPtrFromID(object, tempShape_1->link);
			if(tempShape_2->link != kNoLink) {
				tempShape_3 = Shape_GetPtrFromID(object, tempShape_2->link);
				Matrix_Cat(&tempShape_3->pos, m, m);
			}
			Matrix_Cat(&tempShape_2->pos, m, m);
		}
		Matrix_Cat(&tempShape_1->pos, m, m);
	}
	Matrix_Cat(&tempShape_0->pos, m, m);
}

void Shape_LinkMatrixByID_WithScale(ObjectPtr object, ShapePtr shape, MatrixPtr m, float* s)
{
	ShapePtr tempShape_0, tempShape_1, tempShape_2, tempShape_3;
	
	if(shape->link == kNoLink)
	return;
	
	tempShape_0 = Shape_GetPtrFromID(object, shape->link);
	if(tempShape_0->link != kNoLink) {
		tempShape_1 = Shape_GetPtrFromID(object, tempShape_0->link);
		if(tempShape_1->link != kNoLink) {
			tempShape_2 = Shape_GetPtrFromID(object, tempShape_1->link);
			if(tempShape_2->link != kNoLink) {
				tempShape_3 = Shape_GetPtrFromID(object, tempShape_2->link);
				Matrix_Cat(&tempShape_3->pos, m, m);
				*s *= tempShape_3->scale;
			}
			Matrix_Cat(&tempShape_2->pos, m, m);
			*s *= tempShape_2->scale;
		}
		Matrix_Cat(&tempShape_1->pos, m, m);
		*s *= tempShape_1->scale;
	}
	Matrix_Cat(&tempShape_0->pos, m, m);
	*s *= tempShape_0->scale;
}

float FXShape_IsClicked(StatePtr state, ShapePtr shape, MatrixPtr shapeMatrix, MatrixPtr negatedCamera, Point whereMouse)
{
	Vector				mouse;
	Vector				sphereCenter;
	float				sphereRadius,
						depth,
						iw;
	
	if(!IsFXShape(shape))
	return -1.0;
	
	//Transform sphere to world space
	Matrix_TransformVector(shapeMatrix, &shape->sphereCenter, &sphereCenter);
	sphereRadius = shape->sphereRadius * shape->scale;
	
	//Transform sphere to camera space
	Matrix_TransformVector(negatedCamera, &sphereCenter, &sphereCenter);
	
	//Project sphere
	iw = 1.0 / sphereCenter.z;
	sphereCenter.x = sphereCenter.x * iw * state->pixelConversion + state->projectionOffset_X;
	sphereCenter.y = sphereCenter.y * iw * state->pixelConversion + state->projectionOffset_Y;
	sphereRadius = sphereRadius * iw * -state->pixelConversion;
	
	//we simply need a click inside the bounding sphere
	mouse.x = whereMouse.h;
	mouse.y = whereMouse.v;
	mouse.z = 0.0;
	depth = sphereCenter.z;
	sphereCenter.z = 0.0;
	if(Vector_Distance(sphereCenter, mouse) <= sphereRadius)
	return depth;
	
	return -1.0;
}

float Shape_IsClicked(StatePtr state, ShapePtr shape, MatrixPtr shapeMatrix, MatrixPtr negatedCamera, Point whereMouse)
{
	long				x;
	Vector				v1,
						v2,
						mouse,
						n,
						verts[3];
	Matrix				r1;
	TriFacePtr			tri;
	float				pixelConversion = state->pixelConversion,
						iw;
	VectorPtr			vg0,
						vg1,
						vg2;
	float				curDepth = kHugeDistance,
						depth;
						
	if(IsFXShape(shape))
	return -1.0;
	
	mouse.x = whereMouse.h;
	mouse.y = whereMouse.v;
	mouse.z = 0.0;
	
	//Calculate shape-to-camera matrix
	Matrix_Cat(shapeMatrix, negatedCamera, &r1);

	//Transform points
	for(x = 0; x < shape->pointCount; x++)
	Matrix_TransformVector(&r1, &shape->pointList[x].point, (VectorPtr) &vGouraudBuffer[x]);
	
	tri = shape->triangleList;
	for(x = 0; x < shape->triangleCount; x++) {
		//Map to VectorPtr
		vg0 = (VectorPtr) &vGouraudBuffer[tri->corner[0]];
		vg1 = (VectorPtr) &vGouraudBuffer[tri->corner[1]];
		vg2 = (VectorPtr) &vGouraudBuffer[tri->corner[2]];
		
		//Remove backfacing triangles
		if(shape->backfaceCulling) {
			Vector_Subtract(vg1, vg0, &v1);
			Vector_Subtract(vg2, vg0, &v2);
			Vector_CrossProduct(&v2, &v1, &n);
			if(Vector_DotProduct(vg0, &n) > 0) {
				++tri;
				continue;
			}
		}
		
		//Remove invisible triangles
		if((vg0->z < state->d) || (vg1->z < state->d) || (vg2->z < state->d)) {
			++tri;
			continue;
		}
		if((vg0->z > kMaxViewDistance) || (vg1->z > kMaxViewDistance) || (vg2->z > kMaxViewDistance)) {
			++tri;
			continue;
		}
		
		//Project points
		iw = 1.0 / vg0->z;
		verts[0].x = vg0->x * iw * pixelConversion + state->projectionOffset_X;
		verts[0].y = vg0->y * iw * pixelConversion + state->projectionOffset_Y;
		verts[0].z = vg0->z;
		iw = 1.0 / vg1->z;
		verts[1].x = vg1->x * iw * pixelConversion + state->projectionOffset_X;
		verts[1].y = vg1->y * iw * pixelConversion + state->projectionOffset_Y;
		verts[1].z = vg1->z;
		iw = 1.0 / vg2->z;
		verts[2].x = vg2->x * iw * pixelConversion + state->projectionOffset_X;
		verts[2].y = vg2->y * iw * pixelConversion + state->projectionOffset_Y;
		verts[2].z = vg2->z;
		
		//Is click inside the face?
		depth = ClickInFace(&mouse, verts);
		if((depth > 0.0) && (depth <= curDepth))
		curDepth = depth;
		
		++tri;
	}
	
	//We found a face
	if(curDepth < kHugeDistance)
	return curDepth;
	
	return -1.0;
}

void Shape_Scale(ShapePtr shape, float factor)
{
	long			i;
	
	if(IsFXShape(shape))
	return;
	
	for(i = 0; i < shape->pointCount; ++i) {
		
		shape->pointList[i].point.x *= factor;
		shape->pointList[i].point.y *= factor;
		shape->pointList[i].point.z *= factor;
		
	}
	Shape_CalculateBounding(shape);
}

void Shape_ScaleBox(ShapePtr shape, float size)
{
	long			i;
	float			minX = kHugeDistance,
					minY = kHugeDistance,
					minZ = kHugeDistance,
					maxX = -kHugeDistance,
					maxY = -kHugeDistance,
					maxZ = -kHugeDistance;
	float			width;
	
	if(IsFXShape(shape))
	return;
	
	for(i = 0; i < shape->pointCount; ++i) {
		
		if(shape->pointList[i].point.x < minX)
		minX = shape->pointList[i].point.x;
		else if(shape->pointList[i].point.x > maxX)
		maxX = shape->pointList[i].point.x;
		
		if(shape->pointList[i].point.y < minY)
		minY = shape->pointList[i].point.y;
		else if(shape->pointList[i].point.y > maxY)
		maxY = shape->pointList[i].point.y;
		
		if(shape->pointList[i].point.z < minZ)
		minZ = shape->pointList[i].point.z;
		else if(shape->pointList[i].point.z > maxZ)
		maxZ = shape->pointList[i].point.z;
		
	}
	
	width = maxX - minX;
	if((maxY - minY) > width)
	width = maxY - minY;
	if((maxZ - minZ) > width)
	width = maxZ - minZ;
	
	Shape_Scale(shape, size / width);
}

void Shape_ApplyMatrix(ShapePtr shape)
{
	long				i;
	
	if(!(shape->flags & kFlag_RelativePos))
	return;
	
	if(IsFXShape(shape))
	switch(shape->shading) {
	
		case kShader_Sprite:
		{
			ShapeData_SpritePtr	data;
	
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_SpritePtr) shape->dataPtr;
			
			Matrix_TransformVector(&shape->pos, &data->localPosition, &data->localPosition);
			data->sprite.size *= shape->scale;
		}
		break;
		
		case kShader_Light:
		{
			ShapeData_LightPtr	data;
	
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_LightPtr) shape->dataPtr;
			
			Matrix_Cat(&data->matrix, &shape->pos, &data->matrix);
			data->topBase *= shape->scale;
			data->bottomBase *= shape->scale;
			data->length *= shape->scale;
		}
		break;
		
		case kShader_Particle:
		{
			ShapeData_ParticlePtr	data;
	
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_ParticlePtr) shape->dataPtr;
			
			Matrix_TransformVector(&shape->pos, &data->initPosition, &data->initPosition);
			Matrix_RotateVector(&shape->pos, &data->initPositionRandom, &data->initPositionRandom);
			Matrix_RotateVector(&shape->pos, &data->initVelocity, &data->initVelocity);
			Matrix_RotateVector(&shape->pos, &data->initVelocityRandom, &data->initVelocityRandom);
			data->initSize *= shape->scale;
			data->initSizeRandom *= shape->scale;
		}
		break;
	
	}
	else {
		for(i = 0; i < shape->pointCount; i++)
		Matrix_TransformVector(&shape->pos, &shape->pointList[i].point, &shape->pointList[i].point);
		
		if(shape->normalMode != kNoNormals)
		for(i = 0; i < shape->normalCount; i++) {
			Matrix_RotateVector(&shape->pos, &shape->normalList[i], &shape->normalList[i]);
			Vector_Normalize(&shape->normalList[i], &shape->normalList[i]);
		}
	}
	
	shape->rotateX = 0.0;
	shape->rotateY = 0.0;
	shape->rotateZ = 0.0;
	shape->scale = 1.0;
	Matrix_Clear(&shape->pos);
	shape->flags &= ~kFlag_RelativePos;
	Shape_CalculateBounding(shape);
}

void Shape_MoveOrigin(ShapePtr shape, float moveX, float moveY, float moveZ)
{
	long				i;
	
	if(IsFXShape(shape))
	switch(shape->shading) {
	
		case kShader_Sprite:
		{
			ShapeData_SpritePtr	data;
	
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_SpritePtr) shape->dataPtr;
			
			data->localPosition.x -= moveX;
			data->localPosition.y -= moveY;
			data->localPosition.z -= moveZ;
		}
		break;
		
		case kShader_Light:
		{
			ShapeData_LightPtr	data;
	
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_LightPtr) shape->dataPtr;
			
			data->matrix.w.x -= moveX;
			data->matrix.w.y -= moveY;
			data->matrix.w.z -= moveZ;
		}
		break;
		
		case kShader_Particle:
		{
			ShapeData_ParticlePtr	data;
	
			if(shape->dataPtr == nil)
			break;
			data = (ShapeData_ParticlePtr) shape->dataPtr;
			
			data->initPosition.x -= moveX;
			data->initPosition.y -= moveY;
			data->initPosition.z -= moveZ;
		}
		break;
	
	}
	else {
		for(i = 0; i < shape->pointCount; i++) {
			shape->pointList[i].point.x -= moveX;
			shape->pointList[i].point.y -= moveY;
			shape->pointList[i].point.z -= moveZ;
		}
	}
	
	for(i = 0; i < kBBSize; i++) {
		shape->boundingBox[i].x -= moveX;
		shape->boundingBox[i].y -= moveY;
		shape->boundingBox[i].z -= moveZ;
	}
	
	shape->pos.w.x += moveX * shape->scale;
	shape->pos.w.y += moveY * shape->scale;
	shape->pos.w.z += moveZ * shape->scale;
}
	
void Shape_Center(ShapePtr shape)
{
	float			xOffset,
					yOffset,
					zOffset;
	long			i;
	float			minX = kHugeDistance,
					minY = kHugeDistance,
					minZ = kHugeDistance,
					maxX = -kHugeDistance,
					maxY = -kHugeDistance,
					maxZ = -kHugeDistance;
	
	if(IsFXShape(shape))
	return;
	
	for(i = 0; i < shape->pointCount; ++i) {
		
		if(shape->pointList[i].point.x < minX)
		minX = shape->pointList[i].point.x;
		else if(shape->pointList[i].point.x > maxX)
		maxX = shape->pointList[i].point.x;
		
		if(shape->pointList[i].point.y < minY)
		minY = shape->pointList[i].point.y;
		else if(shape->pointList[i].point.y > maxY)
		maxY = shape->pointList[i].point.y;
		
		if(shape->pointList[i].point.z < minZ)
		minZ = shape->pointList[i].point.z;
		else if(shape->pointList[i].point.z > maxZ)
		maxZ = shape->pointList[i].point.z;
		
	}
	
	xOffset = (minX + maxX) / 2;
	yOffset = (minY + maxY) / 2;
	zOffset = (minZ + maxZ) / 2;
	
	for(i = 0; i < shape->pointCount; ++i) {
		shape->pointList[i].point.x -= xOffset;
		shape->pointList[i].point.y -= yOffset;
		shape->pointList[i].point.z -= zOffset;
	}
	Shape_CalculateBounding(shape);
}

void Shape_ReCenterOrigin(ShapePtr shape)
{
	float			xOffset,
					yOffset,
					zOffset;
	long			i;
	float			minX = kHugeDistance,
					minY = kHugeDistance,
					minZ = kHugeDistance,
					maxX = -kHugeDistance,
					maxY = -kHugeDistance,
					maxZ = -kHugeDistance;
	
	if(IsFXShape(shape))
	return;
	
	for(i = 0; i < shape->pointCount; ++i) {
		
		if(shape->pointList[i].point.x < minX)
		minX = shape->pointList[i].point.x;
		else if(shape->pointList[i].point.x > maxX)
		maxX = shape->pointList[i].point.x;
		
		if(shape->pointList[i].point.y < minY)
		minY = shape->pointList[i].point.y;
		else if(shape->pointList[i].point.y > maxY)
		maxY = shape->pointList[i].point.y;
		
		if(shape->pointList[i].point.z < minZ)
		minZ = shape->pointList[i].point.z;
		else if(shape->pointList[i].point.z > maxZ)
		maxZ = shape->pointList[i].point.z;
		
	}
	
	xOffset = (minX + maxX) / 2;
	yOffset = (minY + maxY) / 2;
	zOffset = (minZ + maxZ) / 2;
	
	for(i = 0; i < shape->pointCount; ++i) {
		shape->pointList[i].point.x -= xOffset;
		shape->pointList[i].point.y -= yOffset;
		shape->pointList[i].point.z -= zOffset;
	}
	Shape_CalculateBounding(shape);
	
	shape->pos.w.x += xOffset * shape->scale;
	shape->pos.w.y += yOffset * shape->scale;
	shape->pos.w.z += zOffset * shape->scale;
}

ShapePtr Shape_Copy(ShapePtr shape)
{
	ShapePtr		copy;
	
	copy = (ShapePtr) NewPtr(sizeof(Shape));
	BlockMove(shape, copy, sizeof(Shape));
	BlockMove(" - copy", &copy->name[copy->name[0] + 1], 7);
	copy->name[0] += 7;
	
	copy->id = 'C' | (copy->id & 0xFFFFFF00);
	
	if(shape->pointList != nil) {
		copy->pointList = (VertexPtr) NewPtr(GetPtrSize((Ptr) shape->pointList));
		BlockMove(shape->pointList, copy->pointList, GetPtrSize((Ptr) shape->pointList));
	}
	if(shape->triangleList != nil) {
		copy->triangleList = (TriFacePtr) NewPtr(GetPtrSize((Ptr) shape->triangleList));
		BlockMove(shape->triangleList, copy->triangleList, GetPtrSize((Ptr) shape->triangleList));
	}
	if(shape->normalMode != kNoNormals) {
		copy->normalList = (VectorPtr) NewPtr(GetPtrSize((Ptr) shape->normalList));
		BlockMove(shape->normalList, copy->normalList, GetPtrSize((Ptr) shape->normalList));
	}
	if(shape->dataPtr != nil) {
		copy->dataPtr = NewPtr(GetPtrSize((Ptr) shape->dataPtr));
		BlockMove(shape->dataPtr, copy->dataPtr, GetPtrSize(shape->dataPtr));
	}
	
	//The new shape does not have any skeleton
	copy->flags &= ~kFlag_HasSkeleton;
	
	return copy;
}

void Shape_PointAt(ShapePtr source, VectorPtr target)
{
	if(IsFXShape(source))
	return;
	
	//Rotate shape
	Vector_Subtract(target, &source->pos.w, &source->pos.z);
	source->pos.y.x = 0.0;
	source->pos.y.y = 1.0;
	source->pos.y.z = 0.0;
	Vector_CrossProduct(&source->pos.y, &source->pos.z, &source->pos.x);
	Vector_Normalize(&source->pos.x, &source->pos.x);
	Vector_Normalize(&source->pos.y, &source->pos.y);
	Vector_Normalize(&source->pos.z, &source->pos.z);
	
	//Extract angles
	Matrix_ExtractAngles(&source->pos, &source->rotateX, &source->rotateY, &source->rotateZ);
	
	//Re-build matrix
	Shape_UpdateMatrix(source);
}

#if 0
#define						kShape_NewVersion		0x0200

typedef struct {
	short			version;
	unsigned long	flags;
	
	long			shapeSize,
					shapeOffset,
					pointListSize,
					pointListOffset,
					triangleListSize,
					triangleListOffset,
					normalListSize,
					normalListOffset,
					dataSize,
					dataOffset;
					
	unsigned char	unused[512];
} NewShapeResourceHeader;
typedef NewShapeResourceHeader* NewShapeResourceHeaderPtr;

OSErr Shape_LoadFromResource(ShapePtr* shape, short resID)
{
	Handle					shapeHandle;
	
	shapeHandle = Get1Resource(kShape_ResType, resID);
	if(shapeHandle == nil)
	return ResError();
	DetachResource(shapeHandle);
	HLock(shapeHandle);
	
	//Check version
	switch(*((short*) *shapeHandle)) {
	
		case kShape_NewVersion:
		{
			NewShapeResourceHeaderPtr	headerPtr = (NewShapeResourceHeaderPtr) *shapeHandle;
	
			//Allocate memory
			*shape = (ShapePtr) NewPtr(sizeof(Shape));
			if(*shape == nil)
			return MemError();
			
			//Copy shape data
			BlockMove(*shapeHandle + headerPtr->shapeOffset, *shape, sizeof(Shape));
			
			//Copy point data
			(*shape)->pointList = (VertexPtr) NewPtr(headerPtr->pointListSize);
			BlockMove(*shapeHandle + headerPtr->pointListOffset, (*shape)->pointList, headerPtr->pointListSize);
			
			//Copy triangle data
			(*shape)->triangleList = (TriFacePtr) NewPtr(headerPtr->triangleListSize);
			BlockMove(*shapeHandle + headerPtr->triangleListOffset, (*shape)->triangleList, headerPtr->triangleListSize);
			
			//Copy normal data
			if(headerPtr->normalListSize != 0) {
				(*shape)->normalList = (VectorPtr) NewPtr(headerPtr->normalListSize);
				BlockMove(*shapeHandle + headerPtr->normalListOffset, (*shape)->normalList, headerPtr->normalListSize);
			}
			else
			(*shape)->normalList = nil;
			
			//Copy custom data
			if(headerPtr->dataSize != 0) {
				(*shape)->dataPtr = NewPtr(headerPtr->dataSize);
				BlockMove(*shapeHandle + headerPtr->dataOffset, (*shape)->dataPtr, headerPtr->dataSize);
			}
			else
			(*shape)->dataPtr = nil;
		}
		break;
		
		case kShape_Version:
		{
			ShapeResourceHeaderPtr	headerPtr = (ShapeResourceHeaderPtr) *shapeHandle;
			
			//Allocate memory
			*shape = (ShapePtr) NewPtr(sizeof(Shape));
			if(*shape == nil)
			return MemError();
			
			//Copy shape data
			BlockMove(*shapeHandle + headerPtr->shapeOffset, *shape, sizeof(Shape));
			
			//Copy point data
			(*shape)->pointList = (VertexPtr) NewPtr(headerPtr->pointListSize);
			BlockMove(*shapeHandle + headerPtr->pointListOffset, (*shape)->pointList, headerPtr->pointListSize);
			
			//Copy triangle data
			(*shape)->triangleList = (TriFacePtr) NewPtr(headerPtr->triangleListSize);
			BlockMove(*shapeHandle + headerPtr->triangleListOffset, (*shape)->triangleList, headerPtr->triangleListSize);
			
			//Copy normal data
			if(headerPtr->normalListSize != 0) {
				(*shape)->normalList = (VectorPtr) NewPtr(headerPtr->normalListSize);
				BlockMove(*shapeHandle + headerPtr->normalListOffset, (*shape)->normalList, headerPtr->normalListSize);
			}
			else
			(*shape)->normalList = nil;
		}
		break;
	
		default:
		DisposeHandle(shapeHandle);
		return kError_BadVersion;
		break;
	
	}
	
	DisposeHandle(shapeHandle);

	return noErr;
}

OSErr Shape_SaveInResource(ShapePtr shape, short resID)
{
	Handle					shapeHandle;
	unsigned long			size,
							offset = 0;
	NewShapeResourceHeader	header;
	Ptr						dest;
	
	//Allocate memory
	size = sizeof(NewShapeResourceHeader) + sizeof(Shape) + GetPtrSize((Ptr) shape->pointList) + GetPtrSize((Ptr) shape->triangleList);
	if(shape->normalMode != kNoNormals)
	size += GetPtrSize((Ptr) shape->normalList);
	if(shape->dataPtr != nil)
	size += GetPtrSize(shape->dataPtr);
	shapeHandle = NewHandleClear(size);
	if(shapeHandle == nil)
	return MemError();
	
	//Setup header
	offset = sizeof(NewShapeResourceHeader);
	header.version = kShape_NewVersion;
	header.flags = 0;
	
	header.shapeSize = sizeof(Shape);
	header.shapeOffset = offset;
	offset += sizeof(Shape);
	
	header.pointListSize = GetPtrSize((Ptr) shape->pointList);
	header.pointListOffset = offset;
	offset += GetPtrSize((Ptr) shape->pointList);
	
	header.triangleListSize = GetPtrSize((Ptr) shape->triangleList);
	header.triangleListOffset = offset;
	offset += GetPtrSize((Ptr) shape->triangleList);
	
	if(shape->normalMode != kNoNormals) {
		header.normalListSize = GetPtrSize((Ptr) shape->normalList);
		header.normalListOffset = offset;
		offset += GetPtrSize((Ptr) shape->normalList);
	}
	else {
		header.normalListSize = 0;
		header.normalListOffset = 0;
	}
	
	if(shape->dataPtr != nil) {
		header.dataSize = GetPtrSize(shape->dataPtr);
		header.dataOffset = offset;
		offset += GetPtrSize(shape->dataPtr);
	}
	else {
		header.dataSize = 0;
		header.dataOffset = 0;
	}
	
	//Copy data
	HLock(shapeHandle);
	dest = *shapeHandle;
	
	BlockMove(&header, dest, sizeof(NewShapeResourceHeader));
	dest += sizeof(NewShapeResourceHeader);
	
	BlockMove(shape, dest, sizeof(Shape));
	((ShapePtr) dest)->link = kNoLink;
	dest += sizeof(Shape);
	
	BlockMove(shape->pointList, dest, GetPtrSize((Ptr) shape->pointList));
	dest += GetPtrSize((Ptr) shape->pointList);
	
	BlockMove(shape->triangleList, dest, GetPtrSize((Ptr) shape->triangleList));
	dest += GetPtrSize((Ptr) shape->triangleList);
	
	if(shape->normalMode != kNoNormals) {
		BlockMove(shape->normalList, dest, GetPtrSize((Ptr) shape->normalList));
		dest += GetPtrSize((Ptr) shape->normalList);
	}
	
	if(shape->dataPtr != nil) {
		BlockMove(shape->dataPtr, dest, GetPtrSize(shape->dataPtr));
		dest += GetPtrSize(shape->dataPtr);
	}
	
	HUnlock(shapeHandle);

	//Write resource
	AddResource(shapeHandle, kShape_ResType, resID, shape->name);
	if(ResError())
	return ResError();
	WriteResource(shapeHandle);
	if(ResError())
	return ResError();
	ReleaseResource(shapeHandle);
	
	return noErr;
}
#else
OSErr Shape_LoadFromResource(ShapePtr* shape, short resID)
{
	Handle					shapeHandle;
	ShapeResourceHeaderPtr	headerPtr;

	shapeHandle = Get1Resource(kShape_ResType, resID);
	if(shapeHandle == nil)
	return ResError();
	DetachResource(shapeHandle);
	HLock(shapeHandle);
	
	//Decode header
	headerPtr = (ShapeResourceHeaderPtr) *shapeHandle;
	if(headerPtr->version != kShape_Version)
	return kError_BadVersion;
	
	*shape = (ShapePtr) NewPtr(sizeof(Shape));
	if(*shape == nil)
	return MemError();
	
	//Copy shape data
	BlockMove(*shapeHandle + headerPtr->shapeOffset, *shape, sizeof(Shape));
	
	//Copy point data
	(*shape)->pointList = (VertexPtr) NewPtr(headerPtr->pointListSize);
	BlockMove(*shapeHandle + headerPtr->pointListOffset, (*shape)->pointList, headerPtr->pointListSize);
	
	//Copy triangle data
	(*shape)->triangleList = (TriFacePtr) NewPtr(headerPtr->triangleListSize);
	BlockMove(*shapeHandle + headerPtr->triangleListOffset, (*shape)->triangleList, headerPtr->triangleListSize);
	
	//Copy normal data
	if(headerPtr->normalListSize != 0) {
		(*shape)->normalList = (VectorPtr) NewPtr(headerPtr->normalListSize);
		BlockMove(*shapeHandle + headerPtr->normalListOffset, (*shape)->normalList, headerPtr->normalListSize);
	}
	else
	(*shape)->normalList = nil;
	
	DisposeHandle(shapeHandle);

	return noErr;
}

OSErr Shape_SaveInResource(ShapePtr shape, short resID)
{
	Handle					shapeHandle;
	unsigned long			size;
	ShapeResourceHeader		header;
	Ptr						dest;
	
	if(IsFXShape(shape))
	return kError_BadItemType;
	
	//Allocate memory
	size = sizeof(ShapeResourceHeader) + sizeof(Shape) + GetPtrSize((Ptr) shape->pointList) + GetPtrSize((Ptr) shape->triangleList);
	if(shape->normalMode != kNoNormals)
	size += GetPtrSize((Ptr) shape->normalList);
	shapeHandle = NewHandle(size);
	if(shapeHandle == nil)
	return MemError();
	
	//Setup header
	header.version = kShape_Version;
	header.shapeSize = sizeof(Shape);
	header.shapeOffset = sizeof(ShapeResourceHeader);
	header.pointListSize = GetPtrSize((Ptr) shape->pointList);
	header.pointListOffset = header.shapeOffset + header.shapeSize;
	header.triangleListSize = GetPtrSize((Ptr) shape->triangleList);
	header.triangleListOffset = header.pointListOffset + header.pointListSize;
	if(shape->normalMode != kNoNormals) {
		header.normalListSize = GetPtrSize((Ptr) shape->normalList);
		header.normalListOffset = header.triangleListOffset + header.triangleListSize;
	}
	else {
		header.normalListSize = 0;
		header.normalListOffset = 0;
	}
	header.flags = 0;
	
	//Copy data
	HLock(shapeHandle);
	dest = *shapeHandle;
	BlockMove(&header, dest, sizeof(ShapeResourceHeader));
	((ShapePtr) dest)->link = kNoLink;
	dest += sizeof(ShapeResourceHeader);
	BlockMove(shape, dest, sizeof(Shape));
	dest += sizeof(Shape);
	BlockMove(shape->pointList, dest, GetPtrSize((Ptr) shape->pointList));
	dest += GetPtrSize((Ptr) shape->pointList);
	BlockMove(shape->triangleList, dest, GetPtrSize((Ptr) shape->triangleList));
	dest += GetPtrSize((Ptr) shape->triangleList);
	if(shape->normalMode != kNoNormals)
	BlockMove(shape->normalList, dest, GetPtrSize((Ptr) shape->normalList));
	HUnlock(shapeHandle);

	//Write resource
	AddResource(shapeHandle, kShape_ResType, resID, shape->name);
	if(ResError())
	return ResError();
	WriteResource(shapeHandle);
	if(ResError())
	return ResError();
	ReleaseResource(shapeHandle);
	
	return noErr;
}
#endif