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


#ifndef __INFINITY_QUATERNION__
#define __INFINITY_QUATERNION__

//ROUTINES:

void Quaternion_QuaternionToMatrix(QuaternionPtr quat, MatrixPtr m);
void Quaternion_MatrixToQuaternion(MatrixPtr matrix, QuaternionPtr quat);
void Quaternion_EulerToQuaternion(float roll, float pitch, float yaw, QuaternionPtr quat);
void Quaternion_SlerpInterpolation(QuaternionPtr from, QuaternionPtr to, float t, QuaternionPtr res);
void Quaternion_LerpInterpolation(QuaternionPtr from, QuaternionPtr to, float t, QuaternionPtr res);
void Quaternion_Normalize(QuaternionPtr quat);
void Quaternion_GetValue(QuaternionPtr quat, float* x, float* y, float* z, float* radians);
void Quaternion_SetValue(QuaternionPtr quat, float x, float y, float z, float angle);
void Quaternion_ScaleAngle(QuaternionPtr  quat, float scale);
void Quaternion_Inverse(QuaternionPtr quat);
void Quaternion_SetFromAx(float x1, float y1, float z1, float x2,float y2, float z2, QuaternionPtr quat);
void Quaternion_Multiply(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res);
void Quaternion_Add(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res);
void Quaternion_Substract(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res);
void Quaternion_Divide(QuaternionPtr q1, QuaternionPtr q2, QuaternionPtr res);
void Quaternion_Copy(QuaternionPtr q1, QuaternionPtr q2);
void Quaternion_Square(QuaternionPtr q1, QuaternionPtr res);
void Quaternion_Sqrt(QuaternionPtr q1, QuaternionPtr res);
float Quaternion_Dot(QuaternionPtr q1, QuaternionPtr q2);
float Quaternion_Length(QuaternionPtr q1);
void Quaternion_Negate(QuaternionPtr q1, QuaternionPtr q2);
void Quaternion_Exp(QuaternionPtr q1, QuaternionPtr q2);
void Quaternion_Log(QuaternionPtr q1, QuaternionPtr q2);
void Quaternion_LnDif(QuaternionPtr a, QuaternionPtr b, QuaternionPtr res);

#endif
