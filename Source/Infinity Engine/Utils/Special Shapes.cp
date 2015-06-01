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
#include				"Infinity Terrain.h"

#include				"Matrix.h"
#include				"Vector.h"
#include				"Shape Utils.h"

//ROUTINES:

ShapePtr Shape_NewSky(float radius)
{
	long				x,
						y;
	ShapePtr			shape;
	VertexPtr			p;
	TriFacePtr			t;
	float				cy,
						sy;
	float				u1, u2;
	float				v1, v2;
	long				p1, p2;
	long				q1, q2;
	
	shape = Shape_New();
	shape->pointCount = kLatBands * (kLonBands + 1);
	shape->pointList = (VertexPtr) NewPtrClear(sizeof(Vertex) * shape->pointCount);
	
	p = shape->pointList;
	for(x = 0; x < kLonBands; x++) {
		p->point.x = 0.0;
		p->point.y = 1.0;
		p->point.z = 0.0;
		p++;
	}
	for(y = 1; y < kLatBands; y++) {
		cy = cos(kPi / kSkyVScale * y / kLatBands);
		sy = sin(kPi / kSkyVScale * y / kLatBands);
		for(x = 0; x < kLonBands; x++) {
			p->point.y = cy;
			p->point.x = sin(kPi * 2 * x / kLonBands) * sy;
			p->point.z = cos(kPi * 2 * x / kLonBands) * sy;
			p++;
		}
		p->point.y = cy;
		p->point.x = sin(kPi * 2 * 0.0 / kLonBands) * sy;
		p->point.z = cos(kPi * 2 * 0.0 / kLonBands) * sy;
		p++;
	}
	
	shape->triangleCount = (kLatBands - 1) * kLonBands * 2 - kLonBands;
	shape->triangleList = (TriFace*) NewPtrClear((sizeof(TriFace) * shape->triangleCount));
	t = shape->triangleList;
	
	for(y = 0; y < kLatBands - 1; y++){
		v1 = 1.0 - ((float) y) / kLatBands;
		v2 = 1.0 - ((float) y + 1) / kLatBands;
		for(x = 0; x < kLonBands; x++){
			u1 = ((float) x) / kLonBands;
			u2 = ((float) x + 1) / kLonBands;
			p2 = kLonBands + x + y * (kLonBands + 1);
			p1 = p2 - (kLonBands + 1);
			q2 = p2 + 1;
			q1 = p1 + 1;
			
			if(y == 0) {
				t->corner[0] = x;
				t->corner[1] = q2;
				t->corner[2] = p2;
				
				t++;
			} else {
				t->corner[0] = p2;
				t->corner[1] = p1;
				t->corner[2] = q2;
				
				t++;

				t->corner[0] = p1;
				t->corner[1] = q1;
				t->corner[2] = q2;
				
				t++;
			}
		}
	}
	
	Shape_Scale(shape, radius);
	Shape_CalculateBounding(shape);
	shape->texture = kTextureID_Sky;
	shape->alpha = 1.0;
	shape->difuse = 1.0;
	shape->backfaceCulling = 0;
	shape->shading = kShader_None;
	
	return shape;
}

ShapePtr Shape_NewPlane(float sizeH, float sizeV, float resolutionH, float resolutionV, VectorPtr color1, VectorPtr color2)
{
	ShapePtr		shape;
	long			h,
					v,
					k,
					l,
					t;
	float			r, g, b;
	VectorPtr		normal;
	Vector			upVector = {0.0,1.0,0.0};
	
	shape = Shape_New();
	if(shape == nil)
	return nil;
	
	shape->pointCount = resolutionH * resolutionV * 4;
	shape->pointList = (VertexPtr) NewPtrClear(sizeof(Vertex) * shape->pointCount);
	
	shape->triangleCount = resolutionH * resolutionV * 2;
	shape->triangleList = (TriFace*) NewPtrClear((sizeof(TriFace) * shape->triangleCount));
	
	t = 0;
	k = 0;
	for(v = 0; v < resolutionV; ++v)
	for(h = 0; h < resolutionH; ++h) {
		shape->pointList[k].point.x = h * sizeH / (float) resolutionH;
		shape->pointList[k].point.y = 0;
		shape->pointList[k].point.z = v * sizeV / (float) resolutionV;
		
		shape->pointList[k + 1].point.x = h * sizeH / (float) resolutionH;
		shape->pointList[k + 1].point.y = 0;
		shape->pointList[k + 1].point.z = (v + 1) * sizeV / (float) resolutionV;
		
		shape->pointList[k + 2].point.x = (h + 1) * sizeH / (float) resolutionH;
		shape->pointList[k + 2].point.y = 0;
		shape->pointList[k + 2].point.z = (v + 1) * sizeV / (float) resolutionV;
		
		shape->pointList[k + 3].point.x = (h + 1) * sizeH / (float) resolutionH;
		shape->pointList[k + 3].point.y = 0;
		shape->pointList[k + 3].point.z = v * sizeV / (float) resolutionV;
		
		if(v % 2) {
			if(h % 2) {
				r = color1->x;
				g = color1->y;
				b = color1->z;
			}
			else {
				r = color2->x;
				g = color2->y;
				b = color2->z;
			}
		}
		else {
			if(h % 2) {
				r = color2->x;
				g = color2->y;
				b = color2->z;
			}
			else {
				r = color1->x;
				g = color1->y;
				b = color1->z;
			}
		}
		
		for(l = 0; l < 4; ++l) {
			shape->pointList[k + l].u = r;
			shape->pointList[k + l].v = g;
			shape->pointList[k + l].c = b;
		}
		
		shape->triangleList[t].corner[0] = k;
		shape->triangleList[t].corner[1] = k + 1;
		shape->triangleList[t].corner[2] = k + 2;
		++t;
		
		shape->triangleList[t].corner[0] = k + 2;
		shape->triangleList[t].corner[1] = k + 3;
		shape->triangleList[t].corner[2] = k + 0;
		++t;
		
		k += 4;
	}
		
	Shape_CalculateBounding(shape);
	shape->alpha = 1.0;
	shape->difuse = 1.0;
	shape->backfaceCulling = 0;
	shape->shading = kShader_Lambert;
	
	shape->normalCount = shape->pointCount;
	shape->normalList = (Vector*) NewPtrClear(sizeof(Vector) * shape->pointCount);
	normal = shape->normalList;
	for(h = 0; h < shape->normalCount; ++h, ++normal)
	*normal = upVector;
	
	return shape;
}
