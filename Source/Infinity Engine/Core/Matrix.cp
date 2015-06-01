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

//VARIABLES LOCALES:

static Matrix unit = {
	1, 0, 0,
	0, 1, 0,
	0, 0, 1,
	0, 0, 0
};

//CONSTANTES LOCALES:

#define fp_epsilon    ((float)1.19209290e-07)
#define kSmallPositive   (16.0 * fp_epsilon)

//ROUTINES:

void Matrix_Clear(MatrixPtr m1)
{
	*m1 = unit;	
}

// m3 = m2 X m1
void Matrix_Cat(MatrixPtr m1, MatrixPtr m2, MatrixPtr m3)
{
	Matrix mt;
	
	Matrix_RotateVector(m2, &m1->x, &mt.x);
	Matrix_RotateVector(m2, &m1->y, &mt.y);
	Matrix_RotateVector(m2, &m1->z, &mt.z);
	Matrix_TransformVector(m2, &m1->w, &mt.w);
	
	*m3 = mt;
}

// m3 = m2 X m1
void Matrix_MultiplyByMatrix(MatrixPtr m1, MatrixPtr m2, MatrixPtr m3)
{
	Matrix mt;
	
	Matrix_RotateVector(m2, &m1->x, &mt.x);
	Matrix_RotateVector(m2, &m1->y, &mt.y);
	Matrix_RotateVector(m2, &m1->z, &mt.z);
	Matrix_RotateVector(m2, &m1->w, &mt.w);
	
	*m3 = mt;
}

void Matrix_TransformVector(MatrixPtr m1, Vector* v1, Vector* v2)
{
	Vector vt;
	
	vt.x = m1->x.x * v1->x + m1->y.x * v1->y + m1->z.x * v1->z + m1->w.x;
	vt.y = m1->x.y * v1->x + m1->y.y * v1->y + m1->z.y * v1->z + m1->w.y;
	vt.z = m1->x.z * v1->x + m1->y.z * v1->y + m1->z.z * v1->z + m1->w.z;
	
	*v2 = vt;
}

void Matrix_RotateVector(MatrixPtr m1, Vector* v1, Vector* v2)
{
	Vector vt;
	
	vt.x = m1->x.x * v1->x + m1->y.x * v1->y + m1->z.x * v1->z;
	vt.y = m1->x.y * v1->x + m1->y.y * v1->y + m1->z.y * v1->z;
	vt.z = m1->x.z * v1->x + m1->y.z * v1->y + m1->z.z * v1->z;
	
	*v2 = vt;
}

void Matrix_SetRotateX(float theta, MatrixPtr m1)
{
	*m1 = unit;
	m1->y.y = m1->z.z = cos(theta);
	m1->z.y = -(m1->y.z = sin(theta));
}

void Matrix_SetRotateY(float theta, MatrixPtr m1)
{
	*m1 = unit;
	m1->x.x = m1->z.z = cos(theta);
	m1->x.z = -(m1->z.x = sin(theta));
}

void Matrix_SetRotateZ(float theta, MatrixPtr m1)
{
	*m1 = unit;
	m1->y.y = m1->x.x = cos(theta);
	m1->y.x = -(m1->x.y = sin(theta));
}

void Matrix_Negate(MatrixPtr m1, MatrixPtr m2)
{
	m2->w.x = m1->x.x * -m1->w.x + m1->x.y * -m1->w.y + m1->x.z * -m1->w.z;
	m2->w.y = m1->y.x * -m1->w.x + m1->y.y * -m1->w.y + m1->y.z * -m1->w.z;
	m2->w.z = m1->z.x * -m1->w.x + m1->z.y * -m1->w.y + m1->z.z * -m1->w.z;

	m2->x.x = m1->x.x;
	m2->x.y = m1->y.x;
	m2->x.z = m1->z.x;

	m2->y.x = m1->x.y;
	m2->y.y = m1->y.y;
	m2->y.z = m1->z.y;

	m2->z.x = m1->x.z;
	m2->z.y = m1->y.z;
	m2->z.z = m1->z.z;
}

void Matrix_ScaleLocal(MatrixPtr m1, float scaleFactor, MatrixPtr m2)
{	
	unsigned long		loop = 3;
	float				*source = (float*) m1,
						*dest = (float*) m2;
	
	do {
						
		*dest = *source * scaleFactor;
		++source;
		++dest;
		*dest = *source * scaleFactor;
		++source;
		++dest;
		*dest = *source * scaleFactor;
		++source;
		++dest;
	} while(--loop);
}

void Matrix_ExtractAngles(MatrixPtr m, float* rotate_Y , float* rotate_Z, float* rotate_X)
{
	float		cosYaw;

	cosYaw = sqrt(m->x.x * m->x.x + m->y.x * m->y.x);
	if(cosYaw > kSmallPositive) {
		*rotate_Y = -atan2(m->z.y, m->z.z);
		*rotate_Z = -atan2(-m->z.x, cosYaw);
		*rotate_X = -atan2(m->y.x, m->x.x);
	} else {
		*rotate_Y = -atan2(-m->y.z, m->y.y);
		*rotate_Z = -atan2(-m->z.x, cosYaw);
		*rotate_X = 0;
	}
}

void Matrix_RotateAroundGlobalAxisX(MatrixPtr m, float alpha)
{
	float		ca = cos(-alpha),
				sa = sin(-alpha);
	Matrix		m2;
	
	m2.x.x = m->x.x;
	m2.x.y = m->x.y * ca + m->x.z * sa;
	m2.x.z = -m->x.y * sa + m->x.z * ca;
	
	m2.y.x = m->y.x;
	m2.y.y = m->y.y * ca + m->y.z * sa;
	m2.y.z = -m->y.y * sa + m->y.z * ca;
	
	m2.z.x = m->z.x;
	m2.z.y = m->z.y * ca + m->z.z * sa;
	m2.z.z = -m->z.y * sa + m->z.z * ca;
	
	m2.w = m->w;
	
	*m = m2;
}

void Matrix_RotateAroundGlobalAxisY(MatrixPtr m, float alpha)
{
	float		ca = cos(alpha),
				sa = sin(alpha);
	Matrix		m2;
	
	m2.x.x = m->x.x * ca + m->x.z * sa;
	m2.x.y = m->x.y;
	m2.x.z = -m->x.x * sa + m->x.z * ca;
	
	m2.y.x = m->y.x * ca + m->y.z * sa;
	m2.y.y = m->y.y;
	m2.y.z = -m->y.x * sa + m->y.z * ca;
	
	m2.z.x = m->z.x * ca + m->z.z * sa;
	m2.z.y = m->z.y;
	m2.z.z = -m->z.x * sa + m->z.z * ca;
	
	m2.w = m->w;
	
	*m = m2;
}

void Matrix_RotateAroundGlobalAxisZ(MatrixPtr m, float alpha)
{
	float		ca = cos(-alpha),
				sa = sin(-alpha);
	Matrix		m2;
	
	m2.x.x = m->x.x * ca + m->x.y * sa;
	m2.x.y = -m->x.x * sa + m->x.y * ca;
	m2.x.z = m->x.z;
	
	m2.y.x = m->y.x * ca + m->y.y * sa;
	m2.y.y = -m->y.x * sa + m->y.y * ca;
	m2.y.z = m->y.z;
	
	m2.z.x = m->z.x * ca + m->z.y * sa;
	m2.z.y = -m->z.x * sa + m->z.y * ca;
	m2.z.z = m->z.z;
	
	m2.w = m->w;
	
	*m = m2;
}

void Matrix_RotateAroundLocalAxisX(MatrixPtr m, float alpha)
{
	float		ca = cos(-alpha),
				sa = sin(-alpha);
	Matrix		m2;
	
	m2.x = m->x;
		
	m2.y.x = -m->z.x * sa + m->y.x * ca;
	m2.y.y = -m->z.y * sa + m->y.y * ca;
	m2.y.z = -m->z.z * sa + m->y.z * ca;
	
	m2.z.x = m->z.x * ca + m->y.x * sa;
	m2.z.y = m->z.y * ca + m->y.y * sa;
	m2.z.z = m->z.z * ca + m->y.z * sa;
	
	m2.w = m->w;
	
	*m = m2;
}

void Matrix_RotateAroundLocalAxisY(MatrixPtr m, float alpha)
{
	float		ca = cos(-alpha),
				sa = sin(-alpha);
	Matrix		m2;
	
	m2.x.x = m->x.x * ca + m->z.x * sa;
	m2.x.y = m->x.y * ca + m->z.y * sa;
	m2.x.z = m->x.z * ca + m->z.z * sa;
	
	m2.y = m->y;
	
	m2.z.x = -m->x.x * sa + m->z.x * ca;
	m2.z.y = -m->x.y * sa + m->z.y * ca;
	m2.z.z = -m->x.z * sa + m->z.z * ca;
	
	m2.w = m->w;
	
	*m = m2;
}

void Matrix_RotateAroundLocalAxisZ(MatrixPtr m, float alpha)
{
	float		ca = cos(-alpha),
				sa = sin(-alpha);
	Matrix		m2;
	
	m2.x.x = m->x.x * ca - m->y.x * sa;
	m2.x.y = m->x.y * ca - m->y.y * sa;
	m2.x.z = m->x.z * ca - m->y.z * sa;
	
	m2.y.x = m->x.x * sa + m->y.x * ca;
	m2.y.y = m->x.y * sa + m->y.y * ca;
	m2.y.z = m->x.z * sa + m->y.z * ca;
	
	m2.z = m->z;
	
	m2.w = m->w;
	
	*m = m2;
}

void Matrix_SetRotateAngleAndAxis(MatrixPtr m, VectorPtr axis, float angle)
{
	float			cs = cos(angle),
					sn = sin(angle);
	float			length = sqrt(axis->x * axis->x + axis->y * axis->y + axis->z * axis->z);
    float			invLength = 1.0 / length;
    float			x = axis->x * invLength,
    				y = axis->y * invLength,
    				z = axis->z * invLength;
    float			omc = 1.0 - cs;
    float			x2 = x * x,
    				y2 = y * y,
    				z2 = z * z;
    float			xy = x * y,
    				xz = x * z,
    				yz = y * z;
    float			snx = sn * x,
    				sny = sn * y,
    				snz = sn * z;
    
    m->x.x = 1.0 - omc * (y2 + z2);
    m->x.y = +snz + omc * xy;
    m->x.z = -sny + omc * xz;
    m->y.x = -snz + omc * xy;
    m->y.y = 1.0 - omc*(x2 + z2);
    m->y.z = +snx + omc * yz;
    m->z.x = +sny + omc * xz;
    m->z.y = -snx + omc * yz;
    m->z.z = 1.0 - omc * (x2 + y2);
    
    m->w.x = m->w.y = m->w.z = 0.0;
}

void Matrix_ExtractAngleAndAxis(MatrixPtr m, VectorPtr axis, float* angle)
{
	float			trace = m->x.x + m->y.y + m->z.z;
	float			cs = 0.5 * (trace - 1.0),
					length;
    float			epsilon = 1e-06;
    
    //Find angle
    if(-1.0 < cs) {
        if(cs < 1.0)
        *angle = acos(cs);
        else
        *angle = 0.0;
    }
    else
    *angle = kPi;

	//Find axis
	axis->x = m->y.z - m->z.y;
	axis->y = m->z.x - m->x.z;
	axis->z = m->x.y - m->y.x;
	length = sqrt(axis->x * axis->x + axis->y * axis->y + axis->z * axis->z);
	if(length > epsilon) {
		axis->x /= length;
		axis->y /= length;
		axis->z /= length;
	}
	else { // angle is 0 or pi
        if(*angle > 1.0) { // any number strictly between 0 and pi works
            float		test[3];
            
            // angle must be pi
			axis->x = sqrt(0.5 * (1.0 + m->x.x));
			axis->y = sqrt(0.5 * (1.0 + m->y.y));
			axis->z = sqrt(0.5 * (1.0 + m->z.z));

			// determine signs of axis components
			test[0] = m->x.x * axis->x + m->x.y * axis->y + m->x.z * axis->z - axis->x;
			test[1] = m->y.x * axis->x + m->y.y * axis->y + m->y.z * axis->z - axis->y;
			test[2] = m->z.x * axis->x + m->z.y * axis->y + m->z.z * axis->z - axis->z;
			length = test[0] * test[0] + test[1] * test[1] + test[2] * test[2];
			if(length < epsilon)
			return;

			axis->y = -axis->y;
			test[0] = m->x.x * axis->x + m->x.y * axis->y + m->x.z * axis->z - axis->x;
            test[1] = m->y.x * axis->x + m->y.y * axis->y + m->y.z * axis->z - axis->y;
            test[2] = m->z.x * axis->x + m->z.y * axis->y + m->z.z * axis->z - axis->z;
			length = test[0] * test[0] + test[1] * test[1] + test[2] * test[2];
			if(length < epsilon)
			return;

			axis->z = -axis->z;
			test[0] = m->x.x * axis->x + m->x.y * axis->y + m->x.z * axis->z - axis->x;
            test[1] = m->y.x * axis->x + m->y.y * axis->y + m->y.z * axis->z - axis->y;
            test[2] = m->z.x * axis->x + m->z.y * axis->y + m->z.z * axis->z - axis->z;
			length = test[0] * test[0] + test[1] * test[1] + test[2] * test[2];
           	if(length < epsilon)
			return;

			axis->y = -axis->y;
			test[0] = m->x.x * axis->x + m->x.y * axis->y + m->x.z * axis->z - axis->x;
            test[1] = m->y.x * axis->x + m->y.y * axis->y + m->y.z * axis->z - axis->y;
            test[2] = m->z.x * axis->x + m->z.y * axis->y + m->z.z * axis->z - axis->z;
			length = test[0] * test[0] + test[1] * test[1] + test[2] * test[2];
           	if(length < epsilon)
			return;
        }
        else {
			// Angle is zero, matrix is the identity, no unique axis, so
			// return (1,0,0) for as good a guess as any.
			axis->x = 1.0;
			axis->y = 0.0;
			axis->z = 0.0;
        }
	}
}

void Matrix_RotateAroundAxis(MatrixPtr m, VectorPtr axis, float angle)
{
	Matrix				r;
	
	Matrix_SetRotateAngleAndAxis(&r, axis, angle);
	
	Matrix_RotateVector(&r, &m->x, &m->x);
	Matrix_RotateVector(&r, &m->y, &m->y);
	Matrix_RotateVector(&r, &m->z, &m->z);
}

void Matrix_BuildRotation(float rX, float rY, float rZ, MatrixPtr matrix)
{
	Matrix				m;
	
	Matrix_SetRotateX(rX, matrix);
	
	Matrix_SetRotateY(rY, &m);
	Matrix_MultiplyByMatrix(&m, matrix, matrix);
	
	Matrix_SetRotateZ(rZ, &m);
	Matrix_MultiplyByMatrix(&m, matrix, matrix);
	
	matrix->w.x = matrix->w.y = matrix->w.z = 0.0;
}
