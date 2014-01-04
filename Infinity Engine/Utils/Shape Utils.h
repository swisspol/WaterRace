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


#ifndef __INFINITY_SHAPE_UTILS__
#define __INFINITY_SHAPE_UTILS__

//ROUTINES:

//File: Shape Utils.cp
ShapePtr Shape_New();
ShapePtr Shape_New_Sprite(OSType textureID);
ShapePtr Shape_New_Light();
ShapePtr Shape_New_ParticleSystem(OSType textureID, long numParticles);
OSErr Shape_ParticleUpdateNumber(ShapePtr shape, long newNumber, MatrixPtr shapeMatrix);
ShapePtr Shape_New_Lens(OSType textureID);
ShapePtr Shape_New_Audio(Handle sound);

void Shape_UpdateMatrix(ShapePtr shape);
void Shape_CalculateBounding(ShapePtr shape);
void Shape_LinkMatrixByID(ObjectPtr object, ShapePtr shape, MatrixPtr m);
void Shape_LinkMatrixByID_WithScale(ObjectPtr object, ShapePtr shape, MatrixPtr m, float* s);
void Shape_Scale(ShapePtr shape, float factor);
float Shape_IsClicked(StatePtr state, ShapePtr shape, MatrixPtr shapeMatrix, MatrixPtr negatedCamera, Point whereMouse);
float FXShape_IsClicked(StatePtr state, ShapePtr shape, MatrixPtr shapeMatrix, MatrixPtr negatedCamera, Point whereMouse);
void Shape_ScaleBox(ShapePtr shape, float size);
void Shape_ApplyMatrix(ShapePtr shape);
void Shape_MoveOrigin(ShapePtr shape, float moveX, float moveY, float moveZ);
void Shape_Center(ShapePtr shape);
void Shape_ReCenterOrigin(ShapePtr shape);
ShapePtr Shape_Copy(ShapePtr shape);
void Shape_PointAt(ShapePtr source, VectorPtr target);

OSErr Shape_LoadFromResource(ShapePtr* shape, short resID);
OSErr Shape_SaveInResource(ShapePtr shape, short resID);

//File: Special Shapes.cp
ShapePtr Shape_NewSky(float radius);
ShapePtr Shape_NewPlane(float sizeH, float sizeV, float resolutionH, float resolutionV, VectorPtr color1, VectorPtr color2);

#endif
