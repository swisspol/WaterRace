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
#include				"Vector.h"
#include				"Infinity Rendering.h"

//ROUTINES:

void Shape_DrawLens(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip)
{
	Vector				lensPos;
	float				iw;
	ShapeData_LensPtr	data;
	long				saveBlend;
	
	//Extract sprite shape data
	if(shape->dataPtr == nil)
	return;
	data = (ShapeData_LensPtr) shape->dataPtr;
	
	if(data->flags & kFlag_FakePosition) {
		Matrix_TransformVector(negatedCamera, &data->fakePosition, &lensPos);
	}
	else {
		//Get real sun position
		Vector_Multiply(kHugeDistance, &state->lightVector, &lensPos);
		Vector_Add(cameraPos, &lensPos, &lensPos);
		Matrix_TransformVector(negatedCamera, &lensPos, &lensPos);
	}
	if(lensPos.z < state->d)
	return;
	
	//Project sun position
	iw = 1.0 / lensPos.z;
	lensPos.x = lensPos.x * iw * state->pixelConversion;
	lensPos.y = lensPos.y * iw * state->pixelConversion;
	
	//Check visibility
	if(!((lensPos.x > -state->projectionOffset_X) && (lensPos.x < state->projectionOffset_X) 
		&& (lensPos.y > -state->projectionOffset_Y) && (lensPos.y < state->projectionOffset_Y)))
	return;
	
	//Get lens position
	lensPos.x = lensPos.x * data->order + state->projectionOffset_X;
	lensPos.y = lensPos.y * data->order + state->projectionOffset_Y;
	
	//Draw lens
	saveBlend = QAGetInt(state->drawContext, kQATag_Blend);
	QASetInt(state->drawContext, kQATag_Blend, kQABlend_PreMultiply);
	
	Texture_DrawSpriteOnScreen(state, lensPos.x, lensPos.y, data->depth, 1.0 / data->depth, data->size * state->viewWidth, 
		data->size * state->viewWidth, shape->texturePtr);
	
	QASetInt(state->drawContext, kQATag_Blend, saveBlend);
}
