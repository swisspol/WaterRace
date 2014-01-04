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


#include					"Infinity Structures.h"

//ROUTINES:

void Texture_Dispose(CompressedTexturePtr texturePtr)
{
	if(texturePtr == nil)
	return;
	
	if(texturePtr->image != nil)
	DisposeHandle((Handle) texturePtr->image);
	DisposePtr((Ptr) texturePtr);
}

void TextureSet_Dispose(TextureSetPtr set)
{
	long				i;
	
	if(set == nil)
	return;
	
	for(i = 0; i < set->textureCount; ++i)
	Texture_Dispose(set->textureList[i]);
	
	DisposePtr((Ptr) set);
}

void Script_Dispose(ScriptPtr script)
{
	long			i;
	
	if(script == nil)
	return;
	
	if(script->flags & kFlag_Running)
	script->flags &= ~kFlag_Running; //Script_Stop(script, true);
	
	for(i = 0; i < script->animationCount; ++i)
	DisposePtr((Ptr) script->animationList[i]);
	
	DisposePtr((Ptr) script);
}

void Shape_Dispose(ShapePtr shape)
{
	if(shape == nil)
	return;
	
	if(shape->pointList != nil)
	DisposePtr((Ptr) shape->pointList);
	if(shape->triangleList != nil)
	DisposePtr((Ptr) shape->triangleList);
	if(shape->normalList != nil)
	DisposePtr((Ptr) shape->normalList);
	if(shape->dataPtr != nil)
	DisposePtr((Ptr) shape->dataPtr);
	
	DisposePtr((Ptr) shape);
}

void Camera_Dispose(CameraPtr camera)
{
	if(camera == nil)
	return;
	
	DisposePtr((Ptr) camera);
}

void Skeleton_Dispose(SkeletonPtr skeleton)
{
	if(skeleton == nil)
	return;
	
	DisposePtr((Ptr) skeleton);
}

void MegaObject_ReleaseData(MegaObjectPtr mega)
{
	long			i;
	
	if(mega == nil)
	return;
	
	for(i = 0; i < mega->object.shapeCount; ++i)
	Shape_Dispose(mega->object.shapeList[i]);
	
	for(i = 0; i < mega->skeletonCount; ++i)
	Skeleton_Dispose(mega->skeletonList[i]);
	mega->skeletonCount = 0;
	
	for(i = 0; i < mega->scriptCount; ++i)
	Script_Dispose(mega->scriptList[i]);
	mega->scriptCount = 0;
	
	for(i = 0; i < mega->cameraCount; ++i)
	Camera_Dispose(mega->cameraList[i]);
	mega->cameraCount = 0;
}