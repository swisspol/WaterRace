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


#ifndef __INFINITY_MATRIX__
#define __INFINITY_MATRIX__

//PROTOTYPES:

void Matrix_Clear(MatrixPtr m1);
void Matrix_Cat(MatrixPtr m1, MatrixPtr m2, MatrixPtr m3);
void Matrix_MultiplyByMatrix(MatrixPtr m1, MatrixPtr m2, MatrixPtr m3);
void Matrix_TransformVector(MatrixPtr m1, Vector* v1, Vector* v2);
void Matrix_RotateVector(MatrixPtr m1, Vector* v1, Vector* v2);
void Matrix_SetRotateX(float theta, MatrixPtr m1);
void Matrix_SetRotateY(float theta, MatrixPtr m1);
void Matrix_SetRotateZ(float theta, MatrixPtr m1);
void Matrix_Negate(MatrixPtr m1, MatrixPtr m2);
void Matrix_ScaleLocal(MatrixPtr m1, float scaleFactor, MatrixPtr m2);

void Matrix_ExtractAngles(MatrixPtr m, float* rotate_Y , float* rotate_Z, float* rotate_X);

void Matrix_RotateAroundGlobalAxisX(MatrixPtr m, float alpha);
void Matrix_RotateAroundGlobalAxisY(MatrixPtr m, float alpha);
void Matrix_RotateAroundGlobalAxisZ(MatrixPtr m, float alpha);
void Matrix_RotateAroundLocalAxisX(MatrixPtr m, float alpha);
void Matrix_RotateAroundLocalAxisY(MatrixPtr m, float alpha);
void Matrix_RotateAroundLocalAxisZ(MatrixPtr m, float alpha);

void Matrix_SetRotateAngleAndAxis(MatrixPtr m, VectorPtr axis, float angle);
void Matrix_ExtractAngleAndAxis(MatrixPtr m, VectorPtr axis, float* angle);
void Matrix_RotateAroundAxis(MatrixPtr m, VectorPtr axis, float angle);

void Matrix_BuildRotation(float rX, float rY, float rZ, MatrixPtr matrix);

#endif
