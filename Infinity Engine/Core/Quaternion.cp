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
#include					"Quaternion.h"

//CONSTANTES:

#define M_PI 3.14159265358979323846264338327950288419716939937510f

#define DELTA 1e-6		 // error tolerance

//ROUTINES:

/*SDOC***********************************************************************

	Name:		Quaternion_ToMat

	Action:	 Converts quaternion representation of a rotation to a matrix
			representation

	Params:	 QuaternionPtr (our quaternion), float (4x4 matrix)

	Returns:	nothing

	Comments: remember matrix (in OGL) is represented in COLUMN major form

***********************************************************************EDOC*/

void Quaternion_QuaternionToMatrix(QuaternionPtr quat, MatrixPtr m)
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = quat->x + quat->x; y2 = quat->y + quat->y; z2 = quat->z + quat->z;
	xx = quat->x * x2;	 xy = quat->x * y2;	 xz = quat->x * z2;
	yy = quat->y * y2;	 yz = quat->y * z2;	 zz = quat->z * z2;
	wx = quat->w * x2;	 wy = quat->w * y2;	 wz = quat->w * z2;

	m->x.x = 1.0 - (yy + zz);
	m->x.y = xy - wz;
	m->x.z = xz + wy;
	
	m->y.x = xy + wz;
	m->y.y = 1.0 - (xx + zz);
	m->y.z = yz - wx;
	
	m->z.x = xz - wy;
	m->z.y = yz + wx;
	m->z.z = 1.0 - (xx + yy);
	
#if 0
	m->w.x = 0.0;
	m->w.y = 0.0;
	m->w.z = 0.0;
#endif	
}


/*SDOC***********************************************************************

	Name:		EulerToQuat

	Action:	 Converts representation of a rotation from Euler angles to
			quaternion representation

	Params:	 float (roll), float (pitch), float (yaw), QuaternionPtr (quat)

	Returns:	nothing

	Comments: remember:	roll	- rotation around X axis
						pitch - rotation around Y axis
						yaw	 - rotation around Z axis
			
			rotations are performed in the following order:
					yaw -> pitch -> roll

			Qfinal = Qyaw Qpitch Qroll

***********************************************************************EDOC*/

void Quaternion_EulerToQuaternion(float roll, float pitch, float yaw, QuaternionPtr quat)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;

	cr = cos(roll/2);
	cp = cos(pitch/2);
	cy = cos(yaw/2);

	sr = sin(roll/2);
	sp = sin(pitch/2);
	sy = sin(yaw/2);
	
	cpcy = cp * cy;
	spsy = sp * sy;

	quat->w = cr * cpcy + sr * spsy;
	quat->x = sr * cpcy - cr * spsy;
	quat->y = cr * sp * cy + sr * cp * sy;
	quat->z = cr * cp * sy - sr * sp * cy;
}



/*SDOC***********************************************************************

	Name:		Quaternion_MatrixToQuaternion

	Action:	 Converts matrix representation of a rotation to a quaternion
			representation

	Params:	 float (matrix), QuaternionPtr (quat)

	Returns:	nothing

	Comments: remember matrix (in OGL) is represented in COLUMN major form

***********************************************************************EDOC*/

void Quaternion_MatrixToQuaternion(MatrixPtr matrix, QuaternionPtr quat)
{
	float			tr,	s;
	float			q[4];
	long			i, j, k;
	long			nxt[3] = {1, 2, 0};
	float			m[3][3];
	
	m[0][0] = matrix->x.x;
	m[0][1] = matrix->x.y;
	m[0][2] = matrix->x.z;
	m[1][0] = matrix->y.x;
	m[1][1] = matrix->y.y;
	m[1][2] = matrix->y.z;
	m[2][0] = matrix->z.x;
	m[2][1] = matrix->z.y;
	m[2][2] = matrix->z.z;
	
	tr = m[0][0] + m[1][1] + m[2][2];

	// check the diagonal
	if(tr > 0.0) {
		s = sqrt (tr + 1.0);

		quat->w = s / 2.0;
		s = 0.5 / s;
		quat->x = (m[1][2] - m[2][1]) * s;
		quat->y = (m[2][0] - m[0][2]) * s;
		quat->z = (m[0][1] - m[1][0]) * s;
	}
	else {		
		// diagonal is negative
		
		i = 0;
		if (m[1][1] > m[0][0])
		i = 1;
		if (m[2][2] > m[i][i])
		i = 2;
		j = nxt[i];
		k = nxt[j];

		s = sqrt ((m[i][i] - (m[j][j] + m[k][k])) + 1.0);
		q[i] = s * 0.5;
		if(s != 0.0)
		s = 0.5 / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;

		quat->x = q[0];
		quat->y = q[1];
		quat->z = q[2];
		quat->w = q[3];
	}
}


/*SDOC***********************************************************************

	Name:		Quaternion_Slerp

	Action:	Smoothly (spherically, shortest path on a quaternion sphere) 
			interpolates between two UNIT quaternion positions

	Params:	 GLQUAT (first and second quaternion), float (interpolation
			parameter [0..1]), Quaternion (resulting quaternion; inbetween)

	Returns:	nothing

	Comments: Most of this code is optimized for speed and not for readability

			As t goes from 0 to 1, qt goes from p to q.
		slerp(p,q,t) = (p*sin((1-t)*omega) + q*sin(t*omega)) / sin(omega)

***********************************************************************EDOC*/

void Quaternion_SlerpInterpolation(QuaternionPtr from, QuaternionPtr to, float t, QuaternionPtr res)
{
	float					to1[4];
	double					omega, cosom, sinom;
	double					scale0, scale1;

	// calc cosine
	cosom = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;
	
	// adjust signs (if necessary)
	if(cosom < 0.0) {
		cosom = -cosom;
		to1[0] = - to->x;
		to1[1] = - to->y;
		to1[2] = - to->z;
		to1[3] = - to->w;
	}
	else {
		to1[0] = to->x;
		to1[1] = to->y;
		to1[2] = to->z;
		to1[3] = to->w;
	}

	// calculate coefficients
	if((1.0 - cosom) > DELTA) {
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0 - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;
	}
	else {				
		// "from" and "to" quaternions are very close 
		//	... so we can do a linear interpolation
		scale0 = 1.0 - t;
		scale1 = t;
	}

	// calculate final values
	res->x = scale0 * from->x + scale1 * to1[0];
	res->y = scale0 * from->y + scale1 * to1[1];
	res->z = scale0 * from->z + scale1 * to1[2];
	res->w = scale0 * from->w + scale1 * to1[3];
}



/*SDOC***********************************************************************

	Name:		Quaternion_Lerp

	Action:	 Linearly interpolates between two quaternion positions

	Params:	 GLQUAT (first and second quaternion), float (interpolation
			parameter [0..1]), Quaternion (resulting quaternion; inbetween)

	Returns:	nothing

	Comments: fast but not as nearly as smooth as Slerp

***********************************************************************EDOC*/

void Quaternion_LerpInterpolation(QuaternionPtr from, QuaternionPtr to, float t, QuaternionPtr res)
{
	float					to1[4];
	double					cosom;
	double					scale0, scale1;

	// calc cosine
	cosom = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;

	// adjust signs (if necessary)
	if(cosom < 0.0) {
		to1[0] = - to->x;
		to1[1] = - to->y;
		to1[2] = - to->z;
		to1[3] = - to->w;
	}
	else {
		to1[0] = to->x;
		to1[1] = to->y;
		to1[2] = to->z;
		to1[3] = to->w;
	}

 	// interpolate linearly
	scale0 = 1.0 - t;
	scale1 = t;
 
	// calculate final values
	res->x = scale0 * from->x + scale1 * to1[0];
	res->y = scale0 * from->y + scale1 * to1[1];
	res->z = scale0 * from->z + scale1 * to1[2];
	res->w = scale0 * from->w + scale1 * to1[3];
}



/*SDOC***********************************************************************

	Name:		Quaternion_Normalize

	Action:	 Normalizes quaternion (i.e. w^2 + x^2 + y^2 + z^2 = 1)

	Params:	 QuaternionPtr (quaternion)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Normalize(QuaternionPtr quat)
{
	float		dist,
				square;

	square = quat->x * quat->x + quat->y * quat->y + quat->z * quat->z + quat->w * quat->w;
	if(square > 0.0)
	dist = (float) (1.0 / sqrt(square));
	else
	dist = 1;

	quat->x *= dist;
	quat->y *= dist;
	quat->z *= dist;
	quat->w *= dist;
}

/*SDOC***********************************************************************

	Name:		Quaternion_GetValue

	Action:	 Disassembles quaternion to an axis and an angle

	Params:	 QuaternionPtr (quaternion), float* (x, y, z - axis), float (angle)

	Returns:	nothing

	Comments: NOTE: vector has been split into x, y, z so that you do not have
			to change your vector library (i.e. greater portability)

			NOTE2: angle is in RADIANS

***********************************************************************EDOC*/

void Quaternion_GetValue(QuaternionPtr quat, float* x, float* y, float* z, float* radians)
{
	float			len;
	float			tx,
					ty,
					tz;

	//cache variables
	tx = quat->x;
	ty = quat->y;
	tz = quat->z;
	
	len = tx * tx + ty * ty + tz * tz;
	if(len > DELTA) {
		*x = tx * (1.0f / len);
		*y = ty * (1.0f / len);
		*z = tz * (1.0f / len);
			*radians = (float)(2.0 * acos(quat->w));
	}
	else {
		*x = 0.0;
		*y = 0.0;
		*z = 1.0;
		*radians = 0.0;
	}
}


/*SDOC***********************************************************************

	Name:		Quaternion_SetValue

	Action:	 Assembles quaternion from an axis and an angle

	Params:	 QuaternionPtr (quaternion), float (x, y, z - axis), float (angle)

	Returns:	nothing

	Comments: NOTE: vector has been split into x, y, z so that you do not have
			to change your vector library (i.e. greater portability)

			NOTE2: angle has to be in RADIANS

***********************************************************************EDOC*/

void Quaternion_SetValue(QuaternionPtr quat, float x, float y, float z, float angle)
{
	float temp, dist;

	// normalize
	temp = x*x + y*y + z*z;

	dist = (float)(1.0 / sqrt(temp));

	x *= dist;
	y *= dist;
	z *= dist;

	quat->x = x;
	quat->y = y;
	quat->z = z;
	quat->w = (float) cos(angle / 2.0f);
}



/*SDOC***********************************************************************

	Name:		Quaternion_ScaleAngle

	Action:	 Scales the rotation angle of a quaternion

	Params:	 QuaternionPtr (quaternion), float (scale value)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_ScaleAngle(QuaternionPtr	quat, float scale)
{
	float x, y, z;	// axis
	float angle;		// and angle

	Quaternion_GetValue(quat, &x, &y, &z, &angle);
	Quaternion_SetValue(quat, x, y, z, (angle * scale));
}



/*SDOC***********************************************************************

	Name:		Quaternion_Inverse

	Action:	 Inverts quaternion's rotation ( q^(-1) )

	Params:	 QuaternionPtr (quaternion)

	Returns:	nothing

	Comments: none
Returns the inverse of the quaternion (1/q).	check conjugate
***********************************************************************EDOC*/

void Quaternion_Inverse(QuaternionPtr quat)
{
	float norm, invNorm;

	norm = quat->x * quat->x + quat->y * quat->y + quat->z * quat->z + quat->w * quat->w;
	
	invNorm = (float) (1.0 / norm);
	
	quat->x = -quat->x * invNorm;
	quat->y = -quat->y * invNorm;
	quat->z = -quat->z * invNorm;
	quat->w =	quat->w * invNorm;
}


/*SDOC***********************************************************************

	Name:		Quaternion_SetFromAx

	Action:	 Constructs quaternion to rotate from one direction vector to 
			another

	Params:	 float (x1, y1, z1 - from vector), 
			float (x2, y2, z2 - to vector), QuaternionPtr (resulting quaternion)

	Returns:	nothing

	Comments: Two vectors have to be UNIT vectors (so make sure you normalize
			them before calling this function
			Some parts are heavily optimized so readability is not so great :(
***********************************************************************EDOC*/

void Quaternion_SetFromAx(float x1, float y1, float z1, float x2,float y2, float z2, QuaternionPtr quat)
{
	float	tx, ty, tz, temp, dist;
	float	cost, len, ss;

	// get dot product of two vectors
	cost = x1 * x2 + y1 * y2 + z1 * z2;

	// check if parallel
	if(cost > 0.99999f) {
		quat->x = quat->y = quat->z = 0.0f;
		quat->w = 1.0f;
		return;
	}
	else if(cost < -0.99999f) {		// check if opposite
		// check if we can use cross product of from vector with [1, 0, 0]
		tx = 0.0;
		ty = x1;
		tz = -y1;
	
		len = sqrt(ty*ty + tz*tz);
		if(len < DELTA) {
			// nope! we need cross product of from vector with [0, 1, 0]
			tx = -z1;
			ty = 0.0;
			tz = x1;
		}
	
		// normalize
		temp = tx*tx + ty*ty + tz*tz;
	
		dist = (float)(1.0 / sqrt(temp));

		tx *= dist;
		ty *= dist;
		tz *= dist;
		
		quat->x = tx;
		quat->y = ty;
		quat->z = tz;
		quat->w = 0.0;
	
		return;
	}

	// ... else we can just cross two vectors

	tx = y1 * z2 - z1 * y2;
	ty = z1 * x2 - x1 * z2;
	tz = x1 * y2 - y1 * x2;

	temp = tx*tx + ty*ty + tz*tz;

	dist = (float)(1.0 / sqrt(temp));

	tx *= dist;
	ty *= dist;
	tz *= dist;


	// we have to use half-angle formulae (sin^2 t = ( 1 - cos (2t) ) /2)
	
	ss = (float)sqrt(0.5f * (1.0f - cost));

	tx *= ss;
	ty *= ss;
	tz *= ss;

	// scale the axis to get the normalized quaternion
	quat->x = tx;
	quat->y = ty;
	quat->z = tz;

	// cos^2 t = ( 1 + cos (2t) ) / 2
	// w part is cosine of half the rotation angle
	quat->w = (float)sqrt(0.5f * (1.0f + cost));
}




/*SDOC***********************************************************************

	Name:		Quaternion_Mul

	Action:	 Multiplies two quaternions

	Params:	 Quaternion ( q1 * q2 = res)

	Returns:	nothing

	Comments: NOTE: multiplication is not commutative

***********************************************************************EDOC*/

void Quaternion_Multiply(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res)
{
	res->x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
	res->y = q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
	res->z = q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;
	res->w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;

	// make sure the resulting quaternion is a unit quat.
	Quaternion_Normalize(res);
}


/*SDOC***********************************************************************

	Name:		Quaternion_Add

	Action:	 Adds two quaternions

	Params:	 QuaternionPtr (q1 + q2 = res)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/
void Quaternion_Add(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res)
{
	res->x = q1->x + q2->x;
	res->y = q1->y + q2->y;
	res->z = q1->z + q2->z;
	res->w = q1->w + q2->w;

	// make sure the resulting quaternion is a unit quat.
	Quaternion_Normalize(res);
}


/*SDOC***********************************************************************

	Name:		Quaternion_Sub

	Action:	 Subtracts two quaternions

	Params:	 QuaternionPtr (q1 - q2 = res)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Substract(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res)
{
	res->x = q1->x - q2->x;
	res->y = q1->y - q2->y;
	res->z = q1->z - q2->z;
	res->w = q1->w - q2->w;

	// make sure the resulting quaternion is a unit quat.
	Quaternion_Normalize(res);
}


/*SDOC***********************************************************************

	Name:		Quaternion_Div

	Action:	 Divide two quaternions

	Params:	 QuaternionPtr (q1 / q2 = res)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Divide(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res)
{
	Quaternion q, r, s;

	Quaternion_Copy(q2, &q);

	// invert vector
	q.x = -q.x;
	q.y = -q.y;
	q.z = -q.z;
	
	Quaternion_Multiply(q1, &q, &r);
	Quaternion_Multiply(&q, &q, &s);

	res->x = r.x / s.w;
	res->y = r.y / s.w;
	res->z = r.z / s.w;
	res->w = r.w / s.w;
}


/*SDOC***********************************************************************

	Name:		Quaternion_Copy

	Action:	 copies q1 into q2

	Params:	 QuaternionPtr (q1 and q2)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Copy(QuaternionPtr q1, QuaternionPtr q2)
{
	q2->x = q1->x;
	q2->y = q1->y;
	q2->z = q1->z;
	q2->w = q1->w;
}


/*SDOC***********************************************************************

	Name:		Quaternion_Square

	Action:	 Square quaternion

	Params:	 QuaternionPtr (q1 * q1 = res)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Square(QuaternionPtr q1, QuaternionPtr res)
{
	float	tt;

	tt = 2 * q1->w;
	res->x = tt * q1->x;
	res->y = tt * q1->y;
	res->z = tt * q1->z;
	res->w = (q1->w * q1->w - q1->x * q1->x - q1->y * q1->y - q1->z * q1->z);
}


/*SDOC***********************************************************************

	Name:		Quaternion_Sqrt

	Action:	 Find square root of a quaternion

	Params:	 QuaternionPtr (sqrt(q1) = res)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Sqrt(QuaternionPtr q1, QuaternionPtr res)
{
	float	length, m, r1, r2;
	Quaternion r;

	length = sqrt (q1->w * q1->w + q1->x * q1->x + q1->y * q1->y);
	if(length != 0.0) 
	length = 1.0 / length; 
	else
	length = 1.0;

	r.x = q1->x * length;
	r.y = q1->z * length;
	r.z = 0.0f;
	r.w = q1->w * length;

	m = 1.0 / sqrt (r.w * r.w + r.x * r.x);
	r1 = sqrt ((1.0 + r.y) * 0.5);
	r2 = sqrt ((1.0 - r.y) * 0.5);

	res->x = sqrt (length) * r2 * r.x * m;
	res->y = sqrt (length) * r1;
	res->z = q1->z;
	res->w = sqrt (length) * r1 * r.w * m;
}


/*SDOC***********************************************************************

	Name:		Quaternion_Dot

	Action:	 Computes the dot product of two unit quaternions

	Params:	 Quaternion (first and second quaternion)

	Returns:	(float) Dot product

	Comments: Quaternion has to be normalized (i.e. it's a unit quaternion)

***********************************************************************EDOC*/

float Quaternion_Dot(QuaternionPtr q1, QuaternionPtr q2)
{
	return (float) (q1->w * q2->w + q1->x * q2->x + q1->y * q2->y+q1->z*q2->z);
}


/*SDOC***********************************************************************

	Name:		Quaternion_Length

	Action:	 Calculates the length of a quaternion

	Params:	 QuaternionPtr (quaternion)

	Returns:	float (length)

	Comments: none

***********************************************************************EDOC*/

float Quaternion_Length(QuaternionPtr q1)
{
	return sqrt(q1->w * q1->w + q1->x * q1->x + q1->y * q1->y + q1->z * q1->z);
}


/*SDOC***********************************************************************

	Name:		Quaternion_Negate

	Action:	 Negates vector part of a quaternion

	Params:	 Quaternion (source and destination quaternion)

	Returns:	nothing

	Comments: Source quaternion does NOT have to be normalized 

***********************************************************************EDOC*/

void Quaternion_Negate(QuaternionPtr q1, QuaternionPtr q2)
{
	Quaternion_Copy(q1, q2);

	Quaternion_Normalize(q2);
	q2->x = -q2->x;
	q2->y = -q2->y;
	q2->z = -q2->z;
}

/*SDOC***********************************************************************

	Name:		Quaternion_Exp

	Action:	 Calculates exponent of a quaternion

	Params:	 QuaternionPtr (Source and destination quaternion)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Exp(QuaternionPtr q1, QuaternionPtr q2)
{
	float	len1, len2;

	len1 = (float) sqrt (q1->x * q1->x + q1->y * q1->y + q1->z * q1->z);
	if(len1 > 0.0) 
	len2 = (float) sin(len1) / len1; 
	else 
	len2 = 1.0;

	q2->x = q1->x * len2;
	q2->y = q1->y * len2;
	q2->z = q1->z * len2;
	q2->w = cos (len1);
}


/*SDOC***********************************************************************

	Name:		Quaternion_Log

	Action:	 Calculates natural logarithm of a quaternion

	Params:	 QuaternionPtr (Source and destination quaternion)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_Log(QuaternionPtr q1, QuaternionPtr q2)
{
	float	length;

	length = sqrt(q1->x * q1->x + q1->y * q1->y + q1->z * q1->z);

	//make sure we do not divide by 0
	if(q1->w != 0.0) 
	length = atan(length / q1->w); 
	else
	length = (float) M_PI/2;

	q2->w = 0.0f;
	q2->x = q1->x * length;
	q2->y = q1->y * length;
	q2->z = q1->z * length;
}

/*SDOC***********************************************************************

	Name:		Quaternion_LnDif

	Action:	 Computes the "natural log difference" of two quaternions,
			q1 and q2 as	ln(qinv(q1)*q2)

	Params:	 QuaternionPtr (Source quaternions	and a destination quaternion)

	Returns:	nothing

	Comments: none

***********************************************************************EDOC*/

void Quaternion_LnDif(QuaternionPtr a, QuaternionPtr b, QuaternionPtr res)
{

	Quaternion inv, dif, temp;
	float	len, len1, s;

	Quaternion_Copy(a, &inv);
	Quaternion_Inverse(&inv);
	Quaternion_Multiply(&inv, b, &dif);
	len = sqrt(dif.x*dif.x + dif.y*dif.y + dif.z*dif.z);
	s = Quaternion_Dot(a, b);
	if(s != 0.0)
	len1 = atan (len / s);
	else
	len1 = M_PI/2;
	if(len != 0.0)
	len1 /= len;
	temp.w = 0.0;
	temp.x = dif.x * len1;
	temp.y = dif.y * len1;
	temp.z = dif.z * len1;
	Quaternion_Copy(&temp, res);
}