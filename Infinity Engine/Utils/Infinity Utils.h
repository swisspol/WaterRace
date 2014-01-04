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


#ifndef __INFINITY_UTILS__
#define __INFINITY_UTILS__

#include				"Matrix.h"

//INLINE ROUTINES:

inline long Shape_GetNumFromID(ObjectPtr object, OSType ID)
{
	long				i;
	
	for(i = 0; i < object->shapeCount; ++i)
	if(object->shapeList[i]->id == ID)
	return i;
	
	return kNoLink;
}

inline ShapePtr Shape_GetPtrFromID(ObjectPtr object, OSType ID)
{
	long				i;
	
	for(i = 0; i < object->shapeCount; ++i)
	if(object->shapeList[i]->id == ID)
	return object->shapeList[i];
	
	return nil;
}

inline void Shape_LinkMatrixByParent(ShapePtr shape, MatrixPtr m)
{
	//Get parent matrix - max depth = 4
	if(shape->parent != nil) {
		if(shape->parent->parent != nil) {
			if(shape->parent->parent->parent != nil) {
				if(shape->parent->parent->parent->parent != nil)
				Matrix_Cat(&shape->parent->parent->parent->parent->pos, m, m);
				Matrix_Cat(&shape->parent->parent->parent->pos, m, m);
			}
			Matrix_Cat(&shape->parent->parent->pos, m, m);
		}
		Matrix_Cat(&shape->parent->pos, m, m);
	}
}

//PROTOTYPES:

//File: Infinity Utils.cp
long Is3DfxKnownEngine(TQAEngine* theEngine);
Boolean IsATIKnownEngine(TQAEngine* theEngine);
OSErr IsAppleSoftwareEngine(TQAEngine* theEngine);
OSErr Get_RaveEngineByName(GDHandle screenDevice, Str255 name, TQAEngine** theEngine);
OSErr Get_BestRaveEngine(GDHandle screenDevice, TQAEngine** theEngine);
OSErr Get_SoftwareEngine(TQAEngine** theEngine);
void SetUpBBox_Homogene(VectorPtr center, float size, Vector bBox[kBBSize]);
void SetUpBBox_Corners(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, Vector bBox[kBBSize]);
Boolean PointOnSide(VectorPtr p, VectorPtr v1, VectorPtr v2, VectorPtr v3);
float ClickInFace(VectorPtr mouse, Vector vertices[3]);

//File: Memory Utils.cp
void Texture_Dispose(CompressedTexturePtr texturePtr);
void TextureSet_Dispose(TextureSetPtr set);
void Script_Dispose(ScriptPtr script);
void Shape_Dispose(ShapePtr shape);
void Camera_Dispose(CameraPtr camera);
void Skeleton_Dispose(SkeletonPtr skeleton);
void MegaObject_ReleaseData(MegaObjectPtr mega);

#endif
