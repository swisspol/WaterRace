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
#include				"Matrix.h"
#include				"Infinity Rendering.h"
#include				"Clipping Utils.h"

//CONSTANTES:

#define					kNbFields				8

//MACROS:

#define CLIPX(p1, p2, p3, xBound) Clip_TQAVGouraud(p1, p2, p3, (p2->x - xBound) / (p2->x - p1->x))
#define CLIPY(p1, p2, p3, yBound) Clip_TQAVGouraud(p1, p2, p3, (p2->y - yBound) / (p2->y - p1->y))
#define CLIPZ(p1, p2, p3, zBound) Clip_TQAVGouraud(p1, p2, p3, (p2->z - zBound) / (p2->z - p1->z))
#define Moderate(a,b) (a + (b - a) * weight)

//ROUTINES:

static void Clip_TQAVGouraud(TQAVGouraud* v1, TQAVGouraud* v2, TQAVGouraud* v3, float weight)
{
	float *f1, *f2, *f3;
	unsigned long loop = kNbFields;
	
	f1 = (float*) v1;
	f2 = (float*) v2;
	f3 = (float*) v3;
	
	do {
		*f3 = Moderate(*f2, *f1);
		++f1;
		++f2;
		++f3;
	} while(--loop);
}

static void Clip_BottomBound(State *state, TQAVGouraud* v1, TQAVGouraud* v2)
{
	TQAVGouraud v;
	float bound = state->viewHeight - boundrySlop;
	
	if(v1->y > bound){
		if(v2->y > bound){
			// all points are out of bounds, draw nothing
			return;
		} else {
			// 1 is out
			CLIPY(v1, v2, &v, bound);
			QADrawLine(state->drawContext, &v, v2);
		}
	} else {
		if(v2->y > bound){
			// 2 is out
			CLIPY(v2, v1, &v, bound);
			QADrawLine(state->drawContext, v1, &v);
		} else {
			// all are in
			QADrawLine(state->drawContext, v1, v2);
		}
	}
}

static void Clip_TopBound(State *state, TQAVGouraud* v1, TQAVGouraud* v2)
{
	TQAVGouraud v;
	float bound = 0 + boundrySlop;
	
	if(v1->y < bound){
		if(v2->y < bound){
			// all points are out of bounds, draw nothing
			return;
		} else {
			// 1 is out
			CLIPY(v1, v2, &v, bound);
			Clip_BottomBound(state, &v, v2);
		}
	} else {
		if(v2->y < bound){
			// 2 is out
			CLIPY(v2, v1, &v, bound);
			Clip_BottomBound(state, v1, &v);
		} else {
			// all are in
			Clip_BottomBound(state, v1, v2);
		}
	}
}

static void Clip_RightBound(State *state, TQAVGouraud* v1, TQAVGouraud* v2)
{
	TQAVGouraud v;
	float bound = state->viewWidth - boundrySlop;
	
	if(v1->x > bound){
		if(v2->x > bound){
			// all points are out of bounds, draw nothing
			return;
		} else {
			// 1 is out
			CLIPX(v1, v2, &v, bound);
			Clip_TopBound(state, &v, v2);
		}
	} else {
		if(v2->x > bound){
			// 2 is out
			CLIPX(v2, v1, &v, bound);
			Clip_TopBound(state, v1, &v);
		} else {
			// all are in
			Clip_TopBound(state, v1, v2);
		}
	}
}

static void Clip_LeftBound(State *state, TQAVGouraud* v1, TQAVGouraud* v2)
{
	TQAVGouraud v;
	float bound = 0 + boundrySlop;
	
	if(v1->x < bound){
		if(v2->x < bound) {
			// all points are out of bounds, draw nothing
			return;
		} else {
			// 1 is out
			CLIPX(v1, v2, &v, bound);
			Clip_RightBound(state, &v, v2);
		}
	} else {
		if(v2->x < bound){
			// 2 is out
			CLIPX(v2, v1, &v, bound);
			Clip_RightBound(state, v1, &v);
		} else {
			// all are in
			Clip_RightBound(state, v1, v2);
		}
	}
}

static void ConvertTo2D(State *state, TQAVGouraud* v1, TQAVGouraud* v2)
{
	float			iw;
	TQAVGouraud*	texturePtr;	
	TQAVGouraud		verts[2];
	
	verts[0] = *v1;
	verts[1] = *v2;

	texturePtr = &verts[0];
	iw = 1.0 / texturePtr->z;
	texturePtr->x = texturePtr->x * iw * state->pixelConversion + state->projectionOffset_X;
	texturePtr->y = texturePtr->y * iw * state->pixelConversion + state->projectionOffset_Y;
	if(state->linearZBuffer)
	texturePtr->z = (texturePtr->z - state->d) * state->invRange;
	else
	texturePtr->z = ((texturePtr->z * state->p1) - state->p2) / texturePtr->z;
	texturePtr->invW = iw;
	
	texturePtr = &verts[1];
	iw = 1.0 / texturePtr->z;
	texturePtr->x = texturePtr->x * iw * state->pixelConversion + state->projectionOffset_X;
	texturePtr->y = texturePtr->y * iw * state->pixelConversion + state->projectionOffset_Y;
	if(state->linearZBuffer)
	texturePtr->z = (texturePtr->z - state->d) * state->invRange;
	else
	texturePtr->z = ((texturePtr->z * state->p1) - state->p2) / texturePtr->z;
	texturePtr->invW = iw;
	
	Clip_LeftBound(state, &verts[0], &verts[1]);
}

static void Clip_Z(State *state, TQAVGouraud* v1, TQAVGouraud* v2)
{
	TQAVGouraud v;
	float bound = state->d + boundrySlop;
	
	if(v1->z < bound){
		if(v2->z < bound){
			// all points are out of bounds, draw nothing
			return;
		} else {
			// 1 is out
			CLIPZ(v1, v2, &v, bound);
			ConvertTo2D(state, &v, v2);
		}
	} else {
		if(v2->z < bound){
			// 2 is out
			CLIPZ(v2, v1, &v, bound);
			ConvertTo2D(state, v1, &v);
		} else {
			// all are in
			ConvertTo2D(state, v1, v2);
		}
	}
}

void Gouraud_DrawLine(State *state, TQAVGouraud* v1, TQAVGouraud* v2)
{
	if((v1->z > state->d) || (v2->z > state->d))
		if((v1->z < kMaxViewDistance) && (v2->z < kMaxViewDistance))
		Clip_Z(state, v1, v2);
}
