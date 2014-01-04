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
#include				"Infinity rendering.h"
#include				"Infinity Utils.h"
#include				"Clipping Utils.h"
#include				"Projection Utils.h"

//ROUTINES:

void Light_DrawShape(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip)
{
	long				x;
	Matrix				localPos;
	TQAIndexedTriangle	*trianglePtr;
	TQAVGouraud			*texturePtr;
	long				triangleCount,
						triangleCount2,
						verticeCount;
	ShapeData_LightPtr	data;
	float				top,
						bottom,
						r,
						g,
						b;
	
	//Extract light shape data
	if(shape->dataPtr == nil)
	return;
	data = (ShapeData_LightPtr) shape->dataPtr;
	
	//Get local matrix - max depth = 4
	localPos = *globalPos;
	if(shape->parent != nil)
	Shape_LinkMatrixByParent(shape, &localPos);
	if(shape->flags & kFlag_RelativePos)
	Matrix_Cat(&shape->pos, &localPos, &localPos);
	
	//Test visibility
	if(clip == kUndetermined) {
		Matrix			r1;
		
		Matrix_Cat(&localPos, negatedCamera, &r1);
		clip = Box_Visible(&r1, shape->boundingBox, state);
		if(clip == kNotVisible)
		return;
	}
	
	//Concatenate with camera matrix
	Matrix_Cat(&data->matrix, &localPos, &localPos);
	Matrix_Cat(&localPos, negatedCamera, &localPos);
	
	//Create vertice buffer and trianglesBuffer buffer
	verticeCount = 8;
	triangleCount = 0;
	triangleCount2 = 0;
	
	//Fill vertice buffer
	texturePtr = vGouraudBuffer;
	top = data->topBase / 2.0;
	bottom = data->bottomBase / 2.0;
	r = data->color_r;
	g = data->color_g;
	b = data->color_b;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = data->intensity;
	texturePtr->x = -top;
	texturePtr->y = 0.0;
	texturePtr->z = -top;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = data->intensity;
	texturePtr->x = -top;
	texturePtr->y = 0.0;
	texturePtr->z = top;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = data->intensity;
	texturePtr->x = top;
	texturePtr->y = 0.0;
	texturePtr->z = top;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = data->intensity;
	texturePtr->x = top;
	texturePtr->y = 0.0;
	texturePtr->z = -top;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = 0.0;
	texturePtr->x = -bottom;
	texturePtr->y = -data->length;
	texturePtr->z = -bottom;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = 0.0;
	texturePtr->x = -bottom;
	texturePtr->y = -data->length;
	texturePtr->z = bottom;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = 0.0;
	texturePtr->x = bottom;
	texturePtr->y = -data->length;
	texturePtr->z = bottom;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	texturePtr->r = r;
	texturePtr->g = g;
	texturePtr->b = b;
	texturePtr->a = 0.0;
	texturePtr->x = bottom;
	texturePtr->y = -data->length;
	texturePtr->z = -bottom;
	Matrix_TransformVector(&localPos, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
	++texturePtr;
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 0, 3, 4);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 0;
		trianglesBuffer[triangleCount].vertices[1] = 3;
		trianglesBuffer[triangleCount].vertices[2] = 4;
		++triangleCount;
	}
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 4, 3, 7);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 4;
		trianglesBuffer[triangleCount].vertices[1] = 3;
		trianglesBuffer[triangleCount].vertices[2] = 7;
		++triangleCount;
	}
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 3, 2, 7);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 3;
		trianglesBuffer[triangleCount].vertices[1] = 2;
		trianglesBuffer[triangleCount].vertices[2] = 7;
		++triangleCount;
	}
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 7, 2, 6);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 7;
		trianglesBuffer[triangleCount].vertices[1] = 2;
		trianglesBuffer[triangleCount].vertices[2] = 6;
		++triangleCount;
	}
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 2, 1, 6);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 2;
		trianglesBuffer[triangleCount].vertices[1] = 1;
		trianglesBuffer[triangleCount].vertices[2] = 6;
		++triangleCount;
	}
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 1, 5, 6);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 1;
		trianglesBuffer[triangleCount].vertices[1] = 5;
		trianglesBuffer[triangleCount].vertices[2] = 6;
		++triangleCount;
	}
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 1, 0, 5);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 1;
		trianglesBuffer[triangleCount].vertices[1] = 0;
		trianglesBuffer[triangleCount].vertices[2] = 5;
		++triangleCount;
	}
	
	if(clip & kZClipping)
	ClipTriangleGouraud_Z(state, vGouraudBuffer, &verticeCount, trianglesBuffer, &triangleCount, 0, 4, 5);
	else {
		trianglesBuffer[triangleCount].triangleFlags = 0;
		trianglesBuffer[triangleCount].vertices[0] = 0;
		trianglesBuffer[triangleCount].vertices[1] = 4;
		trianglesBuffer[triangleCount].vertices[2] = 5;
		++triangleCount;
	}
	
	//Project vGouraudBuffer
	Project_GouraudBuffer(state, verticeCount);
	
	if(clip & kXYClipping) {
		//Clip trianglesBuffer and fill triangle buffer #2
		trianglePtr = trianglesBuffer;
		for(x = 0; x < triangleCount; ++x) {
			ClipTriangleGouraud_XY(state, vGouraudBuffer, &verticeCount, trianglesBuffer2, &triangleCount2, trianglePtr->vertices[0], trianglePtr->vertices[1], trianglePtr->vertices[2]);
			++trianglePtr;
		}
		
		//Draw trianglesBuffer
		QASubmitVerticesGouraud(state->drawContext, verticeCount, vGouraudBuffer);
		QADrawTriMeshGouraud(state->drawContext, triangleCount2, trianglesBuffer2);
	}
	else {
		QASubmitVerticesGouraud(state->drawContext, verticeCount, vGouraudBuffer);
		QADrawTriMeshGouraud(state->drawContext, triangleCount, trianglesBuffer);
	}
	
	//Flush!
	//QAFlush(state->drawContext);
}
