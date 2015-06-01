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
#include				"Matrix.h"
#include				"Vector.h"
#include				"Infinity Rendering.h"
#include				"Infinity Utils.h"
#include				"Clipping Utils.h"

//CONSTANTES PREPROCESSEURS:

#define					__CLEAN_SPECULAR__		1

//ROUTINES:

void Texture_DrawSpriteOnScreen(StatePtr state, float x, float y, float z, float iw, float width, float height, TQATexture* texture)
{
	TQAVTexture			*texturePtr = vTextureBuffer;
	TQAIndexedTriangle	*trianglePtr = trianglesBuffer;
	float				top, left, right, bottom,
						topV, leftU, rightU, bottomV;
	float				spriteWidth = width / 2.0,
						spriteHeight = height / 2.0;
	float				viewWidth = state->viewWidth - boundrySlop,
						viewHeight = state->viewHeight - boundrySlop;
	
	//Check X-Y visibility
	if((x + spriteWidth < boundrySlop) || (x - spriteWidth > viewWidth)
	 || (y + spriteHeight < boundrySlop) || (y - spriteHeight > viewHeight))
	return;
	
	//Clip left
	if(x - spriteWidth < boundrySlop) {
		left = boundrySlop;
		leftU = iw * (boundrySlop - (x - spriteWidth)) / width;
	}
	else {
		left = x - spriteWidth;
		leftU = 0.0;
	}
	
	//Clip right
	if(x + spriteWidth > viewWidth) {
		right = viewWidth;
		rightU = iw - iw * (x + spriteWidth - viewWidth) / width;
	}
	else {
		right = x + spriteWidth;
		rightU = iw;
	}
	
	//Clip top
	if(y - spriteHeight < boundrySlop) {
		top = boundrySlop;
		topV = iw - iw * (boundrySlop - (y - spriteHeight)) / height;
	}
	else {
		top = y - spriteHeight;
		topV = iw;
	}
	
	//Clip bottom
	if(y + spriteHeight > viewHeight) {
		bottom = viewHeight;
		bottomV = iw * (y + spriteHeight - viewHeight) / height;
	}
	else {
		bottom = y + spriteHeight;
		bottomV = 0.0;
	}
	
	//Create top-left vertex
	texturePtr->uOverW = leftU;
	texturePtr->vOverW = topV;
#if __CLEAN_SPECULAR__
	texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
	texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
	texturePtr->a = 1.0;
	texturePtr->x = left;
	texturePtr->y = top;
	texturePtr->z = z;
	texturePtr->invW = iw;
	++texturePtr;
	
	//Create top-right vertex
	texturePtr->uOverW = rightU;
	texturePtr->vOverW = topV;
#if __CLEAN_SPECULAR__
	texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
	texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
	texturePtr->a = 1.0;
	texturePtr->x = right;
	texturePtr->y = top;
	texturePtr->z = z;
	texturePtr->invW = iw;
	++texturePtr;
	
	//Create bottom-right vertex
	texturePtr->uOverW = rightU;
	texturePtr->vOverW = bottomV;
#if __CLEAN_SPECULAR__
	texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
	texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
	texturePtr->a = 1.0;
	texturePtr->x = right;
	texturePtr->y = bottom;
	texturePtr->z = z;
	texturePtr->invW = iw;
	++texturePtr;
	
	//Create bottom-left vertex
	texturePtr->uOverW = leftU;
	texturePtr->vOverW = bottomV;
#if __CLEAN_SPECULAR__
	texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
	texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
#endif
	texturePtr->a = 1.0;
	texturePtr->x = left;
	texturePtr->y = bottom;
	texturePtr->z = z;
	texturePtr->invW = iw;
	++texturePtr;
	
	//Create triangle #1
	trianglePtr->triangleFlags = 0;
	trianglePtr->vertices[0] = 0;
	trianglePtr->vertices[1] = 1;
	trianglePtr->vertices[2] = 2;
	++trianglePtr;
	
	//Create triangle #2
	trianglePtr->triangleFlags = 0;
	trianglePtr->vertices[0] = 2;
	trianglePtr->vertices[1] = 3;
	trianglePtr->vertices[2] = 0;
	
	//Set texture
	if(texture != state->lastTexture) {
		QASetPtr(state->drawContext, kQATag_Texture, texture);
		state->lastTexture = texture;
	}
	
	//Set texture method
#if __CLEAN_SPECULAR__
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate);
#else
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_None);
#endif
	
	QASubmitVerticesTexture(state->drawContext, 4, vTextureBuffer);
	QADrawTriMeshTexture(state->drawContext, 2, trianglesBuffer);

#if 0
	trianglePtr = trianglesBuffer;
	long i;
	for(i = 0; i < 2; ++i, ++trianglePtr) {
		QADrawLine(state->drawContext, (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[0]], (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[1]]);
		QADrawLine(state->drawContext, (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[1]], (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[2]]);
		QADrawLine(state->drawContext, (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[2]], (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[0]]);
	}
#endif
}

void Texture_DrawSprites(StatePtr state, long spriteCount, InfinitySpritePtr spriteList, long byteOffset, TQATexture* texture, Boolean scale)
{
	long				i;
	Vector				position;
	InfinitySpritePtr	sprite = spriteList;
	float				spriteSize;
	long				verticeCount = 0,
						triangleCount = 0;
	TQAVTexture			*texturePtr = vTextureBuffer;
	float				pixelConversion = state->pixelConversion,
						iw,
						p1 = state->p1,
						p2 = state->p2,
						projectionOffset_X = state->projectionOffset_X,
						projectionOffset_Y = state->projectionOffset_Y,
						width = state->viewWidth,
						height = state->viewHeight,
						invRange = state->invRange;
	TQAIndexedTriangle	*trianglePtr = trianglesBuffer;
	float				top, left, right, bottom,
						topV, leftU, rightU, bottomV;
	
	//Fill vertice & triangles buffer
	for(i = 0; i < spriteCount; ++i, (Ptr) sprite += byteOffset) {
		//Check Z - visibility
		if((sprite->position.z < state->d) || (sprite->position.z > kMaxViewDistance))
		continue;
		
		//Project sprite
		iw = 1.0 / sprite->position.z;
		position.x = sprite->position.x * iw * pixelConversion + projectionOffset_X;
		position.y = sprite->position.y * iw * pixelConversion + projectionOffset_Y;
		if(state->linearZBuffer)
		position.z = (sprite->position.z - state->d) * invRange;
		else
		position.z = ((sprite->position.z * p1) - p2) / sprite->position.z;
		if(scale)
		spriteSize = -sprite->size / 2.0 * iw * pixelConversion;
		else
		spriteSize = sprite->size / 2.0 * width;
		
		//Check X-Y visibility
		if((position.x + spriteSize < boundrySlop) || (position.x - spriteSize > width - boundrySlop)
		 || (position.y + spriteSize < boundrySlop) || (position.y - spriteSize > height - boundrySlop))
		continue;
		
		//Clip left
		if(position.x - spriteSize < boundrySlop) {
			left = boundrySlop;
			leftU = iw * (boundrySlop - (position.x - spriteSize)) / (spriteSize * 2.0);
		}
		else {
			left = position.x - spriteSize;
			leftU = 0.0;
		}
		
		//Clip right
		if(position.x + spriteSize > width - boundrySlop) {
			right = width - boundrySlop;
			rightU = iw - iw * (position.x + spriteSize - width + boundrySlop) / (spriteSize * 2.0);
		}
		else {
			right = position.x + spriteSize;
			rightU = iw;
		}
		
		//Clip top
		if(position.y - spriteSize < boundrySlop) {
			top = boundrySlop;
			topV = iw - iw * (boundrySlop - (position.y - spriteSize)) / (spriteSize * 2.0);
		}
		else {
			top = position.y - spriteSize;
			topV = iw;
		}
		
		//Clip bottom
		if(position.y + spriteSize > height - boundrySlop) {
			bottom = height - boundrySlop;
			bottomV = iw * (position.y + spriteSize - height + boundrySlop) / (spriteSize * 2.0);
		}
		else {
			bottom = position.y + spriteSize;
			bottomV = 0.0;
		}
		
		//Create top-left vertex
		texturePtr->uOverW = leftU;
		texturePtr->vOverW = topV;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = sprite->intensity;
		texturePtr->ks_r = sprite->hiColor_r;
		texturePtr->ks_g = sprite->hiColor_g;
		texturePtr->ks_b = sprite->hiColor_b;
		texturePtr->a = sprite->transparency;
		texturePtr->x = left;
		texturePtr->y = top;
		texturePtr->z = position.z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create top-right vertex
		texturePtr->uOverW = rightU;
		texturePtr->vOverW = topV;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = sprite->intensity;
		texturePtr->ks_r = sprite->hiColor_r;
		texturePtr->ks_g = sprite->hiColor_g;
		texturePtr->ks_b = sprite->hiColor_b;
		texturePtr->a = sprite->transparency;
		texturePtr->x = right;
		texturePtr->y = top;
		texturePtr->z = position.z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create bottom-right vertex
		texturePtr->uOverW = rightU;
		texturePtr->vOverW = bottomV;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = sprite->intensity;
		texturePtr->ks_r = sprite->hiColor_r;
		texturePtr->ks_g = sprite->hiColor_g;
		texturePtr->ks_b = sprite->hiColor_b;
		texturePtr->a = sprite->transparency;
		texturePtr->x = right;
		texturePtr->y = bottom;
		texturePtr->z = position.z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create bottom-left vertex
		texturePtr->uOverW = leftU;
		texturePtr->vOverW = bottomV;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = sprite->intensity;
		texturePtr->ks_r = sprite->hiColor_r;
		texturePtr->ks_g = sprite->hiColor_g;
		texturePtr->ks_b = sprite->hiColor_b;
		texturePtr->a = sprite->transparency;
		texturePtr->x = left;
		texturePtr->y = bottom;
		texturePtr->z = position.z;
		texturePtr->invW = iw;
		++texturePtr;
		
		//Create triangle #1
		trianglePtr->triangleFlags = 0;
		trianglePtr->vertices[0] = verticeCount;
		trianglePtr->vertices[1] = verticeCount + 1;
		trianglePtr->vertices[2] = verticeCount + 2;
		++trianglePtr;
		
		//Create triangle #2
		trianglePtr->triangleFlags = 0;
		trianglePtr->vertices[0] = verticeCount + 2;
		trianglePtr->vertices[1] = verticeCount + 3;
		trianglePtr->vertices[2] = verticeCount;
		++trianglePtr;
		
		//Update counters
		verticeCount += 4;
		triangleCount += 2;
	}
	
	//Check for triangles
	if(triangleCount == 0)
	return;
	
	//Set texture
	if(texture != state->lastTexture) {
		QASetPtr(state->drawContext, kQATag_Texture, texture);
		state->lastTexture = texture;
	}
	
	//Set texture method
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate + kQATextureOp_Highlight);
	
	//Render
	QASubmitVerticesTexture(state->drawContext, verticeCount, vTextureBuffer);
	QADrawTriMeshTexture(state->drawContext, triangleCount, trianglesBuffer);

#if 0
	trianglePtr = trianglesBuffer;
	for(i = 0; i < triangleCount; ++i, ++trianglePtr) {
		QADrawLine(state->drawContext, (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[0]], (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[1]]);
		QADrawLine(state->drawContext, (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[1]], (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[2]]);
		QADrawLine(state->drawContext, (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[2]], (TQAVGouraud*) &vTextureBuffer[trianglePtr->vertices[0]]);
	}
#endif
}

void Sprite_DrawShape(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip)
{
	ShapeData_SpritePtr	data;
	Matrix				localPos;
	
	//Extract sprite shape data
	if(shape->dataPtr == nil)
	return;
	data = (ShapeData_SpritePtr) shape->dataPtr;
	
	//Get local matrix - max depth = 4
	localPos = *globalPos;
	if(shape->parent != nil)
	Shape_LinkMatrixByParent(shape, &localPos);
	if(shape->flags & kFlag_RelativePos)
	Matrix_Cat(&shape->pos, &localPos, &localPos);
	
	//Convert sprite to world space
	Matrix_TransformVector(&localPos, &data->localPosition, &data->sprite.position);
	
	//Move sprite if necessary
	if(data->flags & kFlag_FrontDisplay) {
		Vector				cameraVector;
		float				l;
		
		cameraVector.x = cameraPos->x - data->sprite.position.x;
		cameraVector.y = cameraPos->y - data->sprite.position.y;
		cameraVector.z = cameraPos->z - data->sprite.position.z;
		l = FVector_Length(cameraVector);
		data->sprite.position.x += data->frontOffset * cameraVector.x / l;
		data->sprite.position.y += data->frontOffset * cameraVector.y / l;
		data->sprite.position.z += data->frontOffset * cameraVector.z / l;
	}
	
	//Convert sprite to camera space
	Matrix_TransformVector(negatedCamera, &data->sprite.position, &data->sprite.position);
	
	//Do Z-clipping - not necessary: also done in Texture_DrawSprites
	if((data->sprite.position.z < state->d) || (data->sprite.position.z > kMaxViewDistance))
	return;
	
	//Render the sprite in dataPtr
	if(data->flags & kFlag_ConstantSize)
	Texture_DrawSprites(state, 1, (InfinitySpritePtr) data, kDefaultSpriteByteOffset, shape->texturePtr, false);
	else
	Texture_DrawSprites(state, 1, (InfinitySpritePtr) data, kDefaultSpriteByteOffset, shape->texturePtr, true);
}
