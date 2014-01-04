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


#ifndef __INFINITY_VECTOR__
#define __INFINITY_VECTOR__

//PROTOTYPES:

void Vector_Clear(VectorPtr v);
float Vector_DotProduct(VectorPtr v1, VectorPtr v2);
void Vector_Subtract(VectorPtr v1, VectorPtr v2, VectorPtr v3);
void Vector_Add(VectorPtr v1, VectorPtr v2, VectorPtr v3);
void Vector_MultiplyAdd(float l, VectorPtr v1, VectorPtr v2, VectorPtr v3);
void Vector_Multiply(float f, VectorPtr v1, VectorPtr v2);
void Vector_CalculateReflection(VectorPtr l, VectorPtr n, VectorPtr r);
void Vector_CrossProduct(VectorPtr v1, VectorPtr v2, VectorPtr v3);
void Vector_Normalize(VectorPtr v1, VectorPtr v2);
void Vector_Lerp(VectorPtr v1, VectorPtr v2, float t, VectorPtr v3);

//MACROS:

#define HYPOT3(a, b, c) sqrt((a) * (a) + (b) * (b) + (c) * (c))
#define FHYPOT3(a, b, c) FastSqrt((a) * (a) + (b) * (b) + (c) * (c))

#define Vector_Length(v1) HYPOT3(v1.x, v1.y, v1.z)
#define Vector_Distance(v1, v2) HYPOT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z)

#define FVector_Length(v1) FHYPOT3(v1.x, v1.y, v1.z)
#define FVector_Distance(v1, v2) FHYPOT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z)

#endif
