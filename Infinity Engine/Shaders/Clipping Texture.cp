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
#include				"Clipping Utils.h"

//CONSTANTES:

#define					kNbFields				16

//MACROS:

#define CLIPTEXTURE_X(p1, p2, p3, xBound) ClipVerticesTexture(p1, p2, p3, (p2->x - xBound) / (p2->x - p1->x))
#define CLIPTEXTURE_Y(p1, p2, p3, yBound) ClipVerticesTexture(p1, p2, p3, (p2->y - yBound) / (p2->y - p1->y))
#define CLIPTEXTURE_Z(p1, p2, p3, zBound) ClipVerticesTexture(p1, p2, p3, (p2->z - zBound) / (p2->z - p1->z))

#define Moderate(a,b) (a + (b - a) * weight)

#define CheckX(x) ((x >= 0.0) && (x < state->viewWidth))
#define CheckY(y) ((y >= 0.0) && (y < state->viewHeight))
#define Check_Vertex(v) (CheckX(v->x) && CheckY(v->y))
#define Check2_Vertex(v) (CheckX(v.x) && CheckY(v.y))

#define ADD_TRIANGLE() triangles[*triangleCount].triangleFlags = 0; \
				triangles[*triangleCount].vertices[0] = index1; \
				triangles[*triangleCount].vertices[1] = index2; \
				triangles[*triangleCount].vertices[2] = index3; \
				*triangleCount += 1;

//ROUTINES:

static void ClipVerticesTexture(TQAVTexture* v1, TQAVTexture* v2, TQAVTexture* v3, float weight)
{
	float			*f1,
					*f2,
					*f3;
	unsigned long	loop = kNbFields;
	
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

static void ClipTriangleTexture_BottomBound(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3)
{
	TQAVTexture			*v1,
						*v2,
						*v3;
	float				bound = state->viewHeight - boundrySlop;
	
	//Get vertices
	v1 = &vertices[index1];
	v2 = &vertices[index2];
	v3 = &vertices[index3];
	
	//Clip triangle
	if(v1->y > bound){
		if(v2->y > bound){
			if(v3->y > bound){
				// all points are out of bounds, draw nothing
				return;
			} else {
				// 1 & 2 are out
				CLIPTEXTURE_Y(v1, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Y(v2, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			}
		} else {
			if(v3->y > bound){
				// 1 & 3 are out
				CLIPTEXTURE_Y(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Y(v3, v2, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			} else {
				// 1 is out
				CLIPTEXTURE_Y(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
				
				CLIPTEXTURE_Y(v1, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			}

		}
	} else {
		if(v2->y > bound){
			if(v3->y > bound){
				// 2 & 3 are out
				CLIPTEXTURE_Y(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Y(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			} else {
				// 2 is out
				CLIPTEXTURE_Y(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
				
				CLIPTEXTURE_Y(v2, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			}
		} else {
			if(v3->y > bound){
				// 3 is out
				CLIPTEXTURE_Y(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
				
				CLIPTEXTURE_Y(v3, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			} else {
				// all are in
				ADD_TRIANGLE()
			}
		}
	}
}

static void ClipTriangleTexture_TopBound(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3)
{
	TQAVTexture			*v1,
						*v2,
						*v3;
	float				bound = 0 + boundrySlop;
	
	//Get vertices
	v1 = &vertices[index1];
	v2 = &vertices[index2];
	v3 = &vertices[index3];
	
	//Clip triangle
	if(v1->y < bound){
		if(v2->y < bound){
			if(v3->y < bound){
				// all points are out of bounds, draw nothing
				return;
			} else {
				// 1 & 2 are out
				CLIPTEXTURE_Y(v1, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Y(v2, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->y < bound){
				// 1 & 3 are out
				CLIPTEXTURE_Y(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Y(v3, v2, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// 1 is out
				CLIPTEXTURE_Y(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_Y(v1, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}

		}
	} else {
		if(v2->y < bound){
			if(v3->y < bound){
				// 2 & 3 are out
				CLIPTEXTURE_Y(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Y(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
			} else {
				// 2 is out
				CLIPTEXTURE_Y(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_Y(v2, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->y < bound){
				// 3 is out
				CLIPTEXTURE_Y(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_Y(v3, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// all are in
				ClipTriangleTexture_BottomBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		}
	}
}

static void ClipTriangleTexture_RightBound(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3)
{
	TQAVTexture			*v1,
						*v2,
						*v3;
	float				bound = state->viewWidth - boundrySlop;
	
	//Get vertices
	v1 = &vertices[index1];
	v2 = &vertices[index2];
	v3 = &vertices[index3];
	
	//Clip triangle
	if(v1->x > bound){
		if(v2->x > bound){
			if(v3->x > bound){
				// all points are out of bounds, draw nothing
				return;
			} else {
				// 1 & 2 are out
				CLIPTEXTURE_X(v1, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_X(v2, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->x > bound){
				// 1 & 3 are out
				CLIPTEXTURE_X(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_X(v3, v2, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// 1 is out
				CLIPTEXTURE_X(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_X(v1, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}

		}
	} else {
		if(v2->x > bound){
			if(v3->x > bound){
				// 2 & 3 are out
				CLIPTEXTURE_X(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_X(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// 2 is out
				CLIPTEXTURE_X(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_X(v2, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->x > bound){
				// 3 is out
				CLIPTEXTURE_X(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_X(v3, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// all are in
				ClipTriangleTexture_TopBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		}
	}
}

void ClipTriangleTexture_XY(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3)
{
	TQAVTexture			*v1,
						*v2,
						*v3;
	float				bound = 0 + boundrySlop;
	
	//Get vertices
	v1 = &vertices[index1];
	v2 = &vertices[index2];
	v3 = &vertices[index3];
	
	//Clip triangle
	if(v1->x < bound){
		if(v2->x < bound){
			if(v3->x < bound){
				// all points are out of bounds, draw nothing
				return;
			} else {
				// 1 & 2 are out
				CLIPTEXTURE_X(v1, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_X(v2, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->x < bound){
				// 1 & 3 are out
				CLIPTEXTURE_X(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_X(v3, v2, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// 1 is out
				CLIPTEXTURE_X(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_X(v1, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}

		}
	} else {
		if(v2->x < bound){
			if(v3->x < bound){
				// 2 & 3 are out
				CLIPTEXTURE_X(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_X(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// 2 is out
				CLIPTEXTURE_X(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_X(v2, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->x < bound){
				// 3 is out
				CLIPTEXTURE_X(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_X(v3, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// all are in
				ClipTriangleTexture_RightBound(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		}
	}
}

static void ClipTriangleTexture_Hitter(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3)
{
	TQAVTexture			*v1,
						*v2,
						*v3;
	float				bound = state->d + boundrySlop / 10.0;
	
	//Get vertices
	v1 = &vertices[index1];
	v2 = &vertices[index2];
	v3 = &vertices[index3];
	
	//Clip triangle
	if(v1->z < bound){
		if(v2->z < bound){
			if(v3->z < bound){
				// all points are out of bounds, draw nothing
				return;
			} else {
				// 1 & 2 are out
				CLIPTEXTURE_Z(v1, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Z(v2, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			}
		} else {
			if(v3->z < bound){
				// 1 & 3 are out
				CLIPTEXTURE_Z(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Z(v3, v2, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			} else {
				// 1 is out
				CLIPTEXTURE_Z(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
				
				CLIPTEXTURE_Z(v1, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			}

		}
	} else {
		if(v2->z < bound){
			if(v3->z < bound){
				// 2 & 3 are out
				CLIPTEXTURE_Z(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Z(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
			} else {
				// 2 is out
				CLIPTEXTURE_Z(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
				
				CLIPTEXTURE_Z(v2, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()				
			}
		} else {
			if(v3->z < bound){
				// 3 is out
				CLIPTEXTURE_Z(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
				
				CLIPTEXTURE_Z(v3, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ADD_TRIANGLE()
				
			} else {
				// all are in
				ADD_TRIANGLE()
			}
		}
	}
}

void ClipTriangleTexture_Z(State *state, TQAVTexture* vertices, long* verticeCount, TQAIndexedTriangle* triangles, long* triangleCount, long index1, long index2, long index3)
{
	TQAVTexture			*v1,
						*v2,
						*v3;
	float				bound = state->f - boundrySlop;
	
	//Get vertices
	v1 = &vertices[index1];
	v2 = &vertices[index2];
	v3 = &vertices[index3];
	
	//Clip triangle
	if(v1->z > bound){
		if(v2->z > bound){
			if(v3->z > bound){
				// all points are out of bounds, draw nothing
				return;
			} else {
				// 1 & 2 are out
				CLIPTEXTURE_Z(v1, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Z(v2, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->z > bound){
				// 1 & 3 are out
				CLIPTEXTURE_Z(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Z(v3, v2, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// 1 is out
				CLIPTEXTURE_Z(v1, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_Z(v1, v3, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}

		}
	} else {
		if(v2->z > bound){
			if(v3->z > bound){
				// 2 & 3 are out
				CLIPTEXTURE_Z(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				CLIPTEXTURE_Z(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// 2 is out
				CLIPTEXTURE_Z(v2, v1, &vertices[*verticeCount], bound);
				index2 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_Z(v2, v3, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		} else {
			if(v3->z > bound){
				// 3 is out
				CLIPTEXTURE_Z(v3, v1, &vertices[*verticeCount], bound);
				index3 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
				
				CLIPTEXTURE_Z(v3, v2, &vertices[*verticeCount], bound);
				index1 = *verticeCount;
				*verticeCount += 1;
				
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			} else {
				// all are in
				ClipTriangleTexture_Hitter(state, vertices, verticeCount, triangles, triangleCount, index1, index2, index3);
			}
		}
	}
}

