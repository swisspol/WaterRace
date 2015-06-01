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


#include			"Infinity Structures.h"
#include			"Matrix.h"

//ROUTINES:

CameraPtr Camera_New(float zoom)
{
	CameraPtr		camera;
	
	camera = (CameraPtr) NewPtrClear(sizeof(Camera));
	
	BlockMove("\pUntitled", camera->name, sizeof(Str31));
	camera->visibility = 0;
	camera->flags = 0;
	camera->id = kNoID;
	Matrix_Clear(&camera->pos);
	camera->roll = 0.0;
	camera->pitch = 0.0;
	camera->yaw = 0.0;
	camera->zoom = zoom;
	
	return camera;
}

void Camera_UpdateMatrix(CameraPtr camera)
{
	Vector			p;
	Matrix			m;
	
	p = camera->pos.w;
	
	Matrix_SetRotateX(camera->roll, &camera->pos);
	
	Matrix_SetRotateY(camera->pitch, &m);
	Matrix_MultiplyByMatrix(&camera->pos, &m, &camera->pos);
	
	Matrix_SetRotateZ(camera->yaw, &m);
	Matrix_MultiplyByMatrix(&camera->pos, &m, &camera->pos);
	
	camera->pos.w = p;
}

void CameraState_UpdateMatrix(CameraStatePtr camera)
{
	Vector			p;
	Matrix			m;
	
	p = camera->camera.w;
	
	Matrix_SetRotateX(camera->roll, &camera->camera);
	
	Matrix_SetRotateY(camera->pitch, &m);
	Matrix_MultiplyByMatrix(&camera->camera, &m, &camera->camera);
	
	Matrix_SetRotateZ(camera->yaw, &m);
	Matrix_MultiplyByMatrix(&camera->camera, &m, &camera->camera);
	
	camera->camera.w = p;
}
