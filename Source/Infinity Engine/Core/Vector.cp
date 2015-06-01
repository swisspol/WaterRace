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
#include				"Vector.h"

//ROUTINES:

void Vector_Clear(VectorPtr v)
{
	v->x = 0.0;
	v->y = 0.0;
	v->z = 0.0;
}

float Vector_DotProduct(VectorPtr v1, VectorPtr v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

void Vector_Subtract(VectorPtr v1, VectorPtr v2, VectorPtr v3)
{
	v3->x = v1->x - v2->x;
	v3->y = v1->y - v2->y;
	v3->z = v1->z - v2->z;
}

void Vector_Add(VectorPtr v1, VectorPtr v2, VectorPtr v3)
{
	v3->x = v1->x + v2->x;
	v3->y = v1->y + v2->y;
	v3->z = v1->z + v2->z;
}

void Vector_Multiply(float f, VectorPtr v1, VectorPtr v2)
{
	v2->x = f * v1->x;
	v2->y = f * v1->y;
	v2->z = f * v1->z;
}

void Vector_MultiplyAdd(float l, VectorPtr v1, VectorPtr v2, VectorPtr v3)
{
	v3->x = l * v1->x + v2->x;
	v3->y = l * v1->y + v2->y;
	v3->z = l * v1->z + v2->z;
}

// r is a reflection of l through n
// R = 2(N * L)N - L
// taken from 3D Computer Graphics by Alan Watt, p19
// vectors must be normalized
void Vector_CalculateReflection(VectorPtr l, VectorPtr n, VectorPtr r)
{
	Vector temp;
	
	Vector_Multiply(Vector_DotProduct(l, n) * 2.0, n, &temp);
	Vector_Subtract(&temp, l, r);
}

void Vector_CrossProduct(VectorPtr v1, VectorPtr v2, VectorPtr v3)
{
	Vector vt;
	
	vt.x = v1->y * v2->z - v1->z * v2->y;
	vt.y = v1->z * v2->x - v1->x * v2->z;
	vt.z = v1->x * v2->y - v1->y * v2->x;
	
	*v3 = vt;
}

void Vector_Normalize(VectorPtr v1, VectorPtr v2)
{
	float l = HYPOT3(v1->x, v1->y, v1->z);
	
	if(l > 0.0) {
		v2->x = v1->x / l;
		v2->y = v1->y / l;
		v2->z = v1->z / l;
	}
	else {
		v2->x = 0.0;
		v2->y = 0.0;
		v2->z = 0.0;
	}
}

void Vector_Lerp(VectorPtr v1, VectorPtr v2, float t, VectorPtr v3)
{
	v3->x = v1->x + (v2->x - v1->x) * t;
	v3->y = v1->y + (v2->y - v1->y) * t;
	v3->z = v1->z + (v2->z - v1->z) * t;
}
