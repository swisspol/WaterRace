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


#ifndef __INFINITY_CLIPPING__
#define __INFINITY_CLIPPING__

#include				<fp.h>

#include				"Infinity Rendering.h"

//CONSTANTES:

#define					kMaxViewDistance		(state->f)
#define					boundrySlop				0.01

//INLINE ROUTINES:

inline long Sphere_Visible(MatrixPtr transformation, VectorPtr pos, float radius, StatePtr state)
{
	Vector			newPos;
	long			clip = kNoClipping;
	
	//Transform sphere into camera space
	Matrix_TransformVector(transformation, pos, &newPos);
	
	//Is the sphere in front or behind us?
	if(newPos.z + radius < state->d)
	return kNotVisible;
	if(newPos.z - radius > state->f)
	return kNotVisible;
	
	//Are we inside the sphere?
	if(newPos.x * newPos.x + newPos.y * newPos.y + newPos.z * newPos.z <= radius * radius)
	return kZClipping + kXYClipping;
	
	//Is Z-Clipping necessary?
	if((newPos.z - radius < state->d) || (newPos.z + radius > state->f))
	clip += kZClipping;
	
	//Do we need 2D clipping
	float			dx = state->h * state->f / state->d,
					dz = state->f,
					dy = dx * state->viewHeight / state->viewWidth,
					d,
					l;
	long			inside = 0;
	
	//Normalize vector
	l = sqrt(dx * dx + dz * dz);
	
	//Test against left plane
	d = (-dz * newPos.x - dx * newPos.z) / l;
	if(d > radius)
	return kNotVisible;
	if(d < -radius)
	++inside;
	
	//Test against right plane
	d = (dz * newPos.x - dx * newPos.z) / l;
	if(d > radius)
	return kNotVisible;
	if(d < -radius)
	++inside;
	
	//Normalize vector
	l = sqrt(dy * dy + dz * dz);
	
	//Test against top plane
	d = (-dz * newPos.y - dy * newPos.z) / l;
	if(d > radius)
	return kNotVisible;
	if(d < -radius)
	++inside;
	
	//Test against bottom plane
	d = (dz * newPos.y - dy * newPos.z) / l;
	if(d > radius)
	return kNotVisible;
	if(d < -radius)
	++inside;
	
	if(inside != 4)
	return clip + kXYClipping;
	
	return clip;
}

inline long Box_Visible(MatrixPtr transformation, Vector boundingBox[kBBSize], StatePtr state)
{
	long			i,
					clip = kNoClipping,
					behind = 0,
					front = 0,
					left = 0,
					right = 0,
					top = 0,
					bottom = 0;
	Vector			newPos;
	float			dx = state->h * state->f / state->d,
					dz = state->f,
					dy = dx * state->viewHeight / state->viewWidth,
					l1 = sqrt(dx * dx + dz * dz),
					l2 = sqrt(dy * dy + dz * dz);
	
	for(i = 0; i < kBBSize; ++i) {
		//Transform point into camera space
		Matrix_TransformVector(transformation, &boundingBox[i], &newPos);
		
		//Test Z-axis
		if(newPos.z < state->d)
		++behind;
		else if(newPos.z > state->f)
		++front;
		
		//Test against left plane
		if((-dz * newPos.x - dx * newPos.z) / l1 > 0.0)
		++left;
		
		//Test against right plane
		if((dz * newPos.x - dx * newPos.z) / l1 > 0.0)
		++right;
	
		//Test against top plane
		if((-dz * newPos.y - dy * newPos.z) / l2 > 0.0)
		++top;
		
		//Test against bottom plane
		if((dz * newPos.y - dy * newPos.z) / l2 > 0.0)
		++bottom;
	}
	
	//Box is fully outside frustrum
	if((front >= kBBSize) || (behind >= kBBSize))
	return kNotVisible;
	if((left >= kBBSize) || (right >= kBBSize) || (top >= kBBSize) || (bottom >= kBBSize))
	return kNotVisible;
	
	//Box is fully inside frustrum
	if((left == 0) && (right == 0) && (top == 0) && (bottom == 0) && (front == 0) && (behind == 0))
	return kNoClipping;
	
	//Box is partially inside
	if((left > 0) || (right > 0) || (top > 0) || (bottom > 0))
	clip += kXYClipping;
	if((front > 0) || (behind > 0))
	clip += kZClipping;
	
	return clip;
}

//ROUTINES:

//File: Clipping Texture.cp
void ClipTriangleTexture_XY(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3);
void ClipTriangleTexture_Z(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3);

//File: Clipping Gouraud.cp
void ClipTriangleGouraud_XY(State *state, TQAVGouraud* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3);
void ClipTriangleGouraud_Z(State *state, TQAVGouraud* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3);

#endif
