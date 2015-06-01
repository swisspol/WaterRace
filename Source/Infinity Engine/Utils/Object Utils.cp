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
#include					"Matrix.h"
#include					"Infinity Utils.h"
#include					"Shape Utils.h"

//ROUTINES:

void Object_CalculateBounding(ObjectPtr object)
{
	long			i,
					j;
	float			minX = kHugeDistance,
					minY = kHugeDistance,
					minZ = kHugeDistance,
					maxX = -kHugeDistance,
					maxY = -kHugeDistance,
					maxZ = -kHugeDistance,
					radius = 0.0;
	ShapePtr		shape;
	Matrix			r1;
	Vector			center,
					point;
	
	//Update bounding box
	for(j = 0; j < object->shapeCount; ++j) {
		shape = object->shapeList[j];
		
		if(shape->flags & kFlag_NotInObjectBBox)
		continue;
		
		Matrix_Clear(&r1);
		Shape_LinkMatrixByID(object, shape, &r1);
		if(shape->flags & kFlag_RelativePos)
		Matrix_Cat(&shape->pos, &r1, &r1);
		
		if(IsFXShape(shape))
		for(i = 0; i < kBBSize; ++i) {
			Matrix_TransformVector(&r1, &shape->boundingBox[i], &point);
			
			if(point.x < minX)
			minX = point.x;
			if(point.x > maxX)
			maxX = point.x;
			
			if(point.y < minY)
			minY = point.y;
			if(point.y > maxY)
			maxY = point.y;
			
			if(point.z < minZ)
			minZ = point.z;
			if(point.z > maxZ)
			maxZ = point.z;
			
		}
		else
		for(i = 0; i < shape->pointCount; ++i) {
			Matrix_TransformVector(&r1, &shape->pointList[i].point, &point);
			
			if(point.x < minX)
			minX = point.x;
			if(point.x > maxX)
			maxX = point.x;
			
			if(point.y < minY)
			minY = point.y;
			if(point.y > maxY)
			maxY = point.y;
			
			if(point.z < minZ)
			minZ = point.z;
			if(point.z > maxZ)
			maxZ = point.z;
			
		}
	}
	SetUpBBox_Corners(minX, minY, minZ, maxX, maxY, maxZ, object->boundingBox);
	
	//Update object center
	center.x = (minX + maxX) / 2.0;
	center.y = (minY + maxY) / 2.0;
	center.z = (minZ + maxZ) / 2.0;
	object->sphereCenter = center;
	
	//Update object radius
	for(j = 0; j < object->shapeCount; ++j) {
		shape = object->shapeList[j];
		
		if(shape->flags & kFlag_NotInObjectBBox)
		continue;
		
		Matrix_Clear(&r1);
		Shape_LinkMatrixByID(object, shape, &r1);
		if(shape->flags & kFlag_RelativePos)
		Matrix_Cat(&shape->pos, &r1, &r1);
		
		if(IsFXShape(shape))
		for(i = 0; i < kBBSize; ++i) {
			Matrix_TransformVector(&r1, &shape->boundingBox[i], &point);
			Vector_Subtract(&point, &center, &point);
			if(Vector_Length(point) > radius)
			radius = Vector_Length(point);
		}
		else
		for(i = 0; i < shape->pointCount; ++i) {
			Matrix_TransformVector(&r1, &shape->pointList[i].point, &point);
			Vector_Subtract(&point, &center, &point);
			if(Vector_Length(point) > radius)
			radius = Vector_Length(point);
		}
	}
	object->sphereRadius = radius;
}

void Object_UpdateMatrix(ObjectPtr object)
{
	Vector			p;
	Matrix			m;
	
	p = object->pos.w;
	Matrix_Clear(&object->pos);
	if(object->scale != 1.0)
	Matrix_ScaleLocal(&object->pos, object->scale, &object->pos);
	
	Matrix_SetRotateX(object->rotateX, &m);
	Matrix_MultiplyByMatrix(&m, &object->pos, &object->pos);
	
	Matrix_SetRotateY(object->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &object->pos, &object->pos);
	
	Matrix_SetRotateZ(object->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &object->pos, &object->pos);
	
	object->pos.w = p;
}

void Object_Scale(ObjectPtr object, float factor)
{
	long				i;
	ShapePtr			shape;
	
	for(i = 0; i < object->shapeCount; ++i) {
		shape = object->shapeList[i];
		Shape_Scale(shape, factor);
		shape->pos.w.x *= factor;
		shape->pos.w.y *= factor;
		shape->pos.w.z *= factor;
	}
	Object_CalculateBounding(object);
}

float Object_ScaleToBox(ObjectPtr object, float boxSize)
{
	float				maxSize = 0.0,
						factor;
	long				i;
	
	//Calculate scale
	for(i = 0; i < kBBSize; ++i)
	if(Vector_Length(object->boundingBox[i]) > maxSize)
	maxSize = Vector_Length(object->boundingBox[i]);
	
	//Calculate scale factor
	factor = boxSize / maxSize;
	
	//Scale object
	Object_Scale(object, factor);
	
	return factor;
}

float Object_IsClicked(ObjectPtr object, StatePtr state, MatrixPtr globalPos, MatrixPtr negatedCamera, Point whereMouse)
{
	long				i;
	Matrix				localPos;
	float				curDepth = kHugeDistance,
						depth;
	ShapePtr			shape;
						
	for(i = 0; i < object->shapeCount; ++i) {
		shape = object->shapeList[i];
		
		//Get local matrix - max depth = 4
		localPos = *globalPos;
		if(shape->link != kNoID)
		Shape_LinkMatrixByID(object, shape, &localPos);
		if(shape->flags & kFlag_RelativePos)
		Matrix_Cat(&shape->pos, &localPos, &localPos);
		
		//Is click inside the shape?
		if(IsFXShape(shape))
		depth = FXShape_IsClicked(state, shape, &localPos, negatedCamera, whereMouse);
		else
		depth = Shape_IsClicked(state, shape, &localPos, negatedCamera, whereMouse);
		if((depth > 0.0) && (depth <= curDepth))
		curDepth = depth;
	}
	
	//We found a shape
	if(curDepth < kHugeDistance)
	return curDepth;
		
	return -1.0;
}

