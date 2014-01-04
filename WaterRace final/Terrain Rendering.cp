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

#include				"WaterRace.h"
#include				"Game.h"
#include				"Preferences.h"

#include				"Clock.h"
#include				"Infinity Rendering.h"
#include				"Vector.h"

#include				"Cinematic Utils.h"
#include				"Clipping Utils.h"

//ROUTINES:

void SeaMesh_RenderModels(SeaMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	ModelReferencePtr	reference = mesh->referenceList;
	long				i,
						j,
						visible;
	ObjectPtr			object;
	Vector				refPos;
	
	for(i = 0; i < mesh->referenceCount; ++i, ++reference) {
		//Extract object
		object = (ObjectPtr) reference->id;
		
		//Test visibility
#if 0
		if(mesh->clipping > 0) {
			Matrix_TransformVector(&reference->pos, &object->sphereCenter, &refPos);
			visible = Sphere_Visible(negatedCamera, &refPos, object->sphereRadius * reference->scale, state);
			if(visible == kNotVisible)
			continue;
			if((visible > 0) && (reference->scale == 1.0)) //Forces per-shape clipping test
			visible = kUndetermined;
		}
		else
		visible = kNoClipping;
#else
		Matrix_TransformVector(&reference->pos, &object->sphereCenter, &refPos);
		visible = Sphere_Visible(negatedCamera, &refPos, object->sphereRadius * reference->scale, state);
		if(visible == kNotVisible)
		continue;
#endif
		
		//Do we need to update scripts?
		if(((ExtendedModelPtr) object)->hasScripts && (((ExtendedModelPtr) object)->lastScriptUpdateTime != state->frameTime)) {
			for(j = 0; j < ((MegaObjectPtr) object)->scriptCount; ++j)
			if(((MegaObjectPtr) object)->scriptList[j]->flags & kFlag_Running)
			Script_Run(((MegaObjectPtr) object)->scriptList[j], state->frameTime);
			((ExtendedModelPtr) object)->lastScriptUpdateTime = state->frameTime;
		}
		
#if __ENABLE_LOD__
		//Calculate distance to camera & Draw reference
		if(Square_Distance(cameraPos, reference->pos.w) > state->c) {
			for(j = 0; j < object->shapeCount; ++j)
			if(!(object->shapeList[j]->flags & kFlag_MayHide))
			Shape_CallRenderingProc(object->shapeList[j], state, &reference->pos, negatedCamera, cameraPos, object->shapeList, visible);
		}
		else {
#endif
			for(j = 0; j < object->shapeCount; ++j)
			Shape_CallRenderingProc(object->shapeList[j], state, &reference->pos, negatedCamera, cameraPos, object->shapeList, visible);
#if __ENABLE_LOD__
		}
#endif

#if __RENDER_MODEL_BBOX__
		{
			long							k,
											l;
			TQAVGouraud						vertices[kBBSize];
			CollisionTilePtr				tile;
			Boolean							found = false;
			CollisionBoxPtr					box;
			MegaObjectPtr					mega = (MegaObjectPtr) reference->id;
			
			if(reference->flags & kRefFlag_CollisionDetection) {
				//Find the main ship is in
				tile = &collisionMap[(long) ((-terrain_MinZ + shipList[0].megaObject.object.pos.w.z) / kCollisionMapResolution) * collisionMapWidth
			 		+ (long) ((-terrain_MinX + shipList[0].megaObject.object.pos.w.x) / kCollisionMapResolution)];
				
				for(l = 0; l < tile->referenceCount; ++l)
				if(tile->referenceList[l] == reference) {
					for(k = 0; k < kBBSize; ++k) {
						vertices[k].r = 0.0;
						vertices[k].g = 1.0;
						vertices[k].b = 1.0;
						vertices[k].a = 1.0;
					}
					found = true;
					break;
				}
				if(!found)
				for(k = 0; k < kBBSize; ++k) {
					vertices[k].r = 1.0;
					vertices[k].g = 1.0;
					vertices[k].b = 0.0;
					vertices[k].a = 1.0;
				}
				
				for(k = 0; k < mega->boxCount; ++k) {
					box = &mega->boxList[k];
				
					for(l = 0; l < kBBSize; l++) {
						Matrix_TransformVector(&reference->pos, &box->corners[l], (VectorPtr) &vertices[l]);
						Matrix_TransformVector(negatedCamera, (VectorPtr) &vertices[l], (VectorPtr) &vertices[l]);
					}
				
					Gouraud_DrawLine(state, &vertices[0], &vertices[1]);
					Gouraud_DrawLine(state, &vertices[1], &vertices[2]);
					Gouraud_DrawLine(state, &vertices[2], &vertices[3]);
					Gouraud_DrawLine(state, &vertices[3], &vertices[0]);
					Gouraud_DrawLine(state, &vertices[4], &vertices[5]);
					Gouraud_DrawLine(state, &vertices[5], &vertices[6]);
					Gouraud_DrawLine(state, &vertices[6], &vertices[7]);
					Gouraud_DrawLine(state, &vertices[7], &vertices[4]);
					Gouraud_DrawLine(state, &vertices[0], &vertices[4]);
					Gouraud_DrawLine(state, &vertices[1], &vertices[5]);
					Gouraud_DrawLine(state, &vertices[2], &vertices[6]);
					Gouraud_DrawLine(state, &vertices[3], &vertices[7]);
				}
			}
		}
#endif
	}
}

void Mesh_RenderModels(TerrainMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	ModelReferencePtr	reference = mesh->referenceList;
	long				i,
						j,
						visible;
	ObjectPtr			object;
	Vector				refPos;
	
	if(mesh->flags & kFlag_UseMatrix)
	return;
	
	for(i = 0; i < mesh->referenceCount; ++i, ++reference) {
		//Extract object
		object = (ObjectPtr) reference->id;
		
		//Test visibility
#if 0
		if(mesh->clipping > 0) {
			Matrix_TransformVector(&reference->pos, &object->sphereCenter, &refPos);
			visible = Sphere_Visible(negatedCamera, &refPos, object->sphereRadius * reference->scale, state);
			if(visible == kNotVisible)
			continue;
			if((visible > 0) && (reference->scale == 1.0)) //Forces per-shape clipping test
			visible = kUndetermined;
		}
		else
		visible = kNoClipping;
#else
		Matrix_TransformVector(&reference->pos, &object->sphereCenter, &refPos);
		visible = Sphere_Visible(negatedCamera, &refPos, object->sphereRadius * reference->scale, state);
		if(visible == kNotVisible)
		continue;
#endif
		
		//Do we need to update scripts?
		if(((ExtendedModelPtr) object)->hasScripts && (((ExtendedModelPtr) object)->lastScriptUpdateTime != state->frameTime)) {
			for(j = 0; j < ((MegaObjectPtr) object)->scriptCount; ++j)
			if(((MegaObjectPtr) object)->scriptList[j]->flags & kFlag_Running)
			Script_Run(((MegaObjectPtr) object)->scriptList[j], state->frameTime);
			((ExtendedModelPtr) object)->lastScriptUpdateTime = state->frameTime;
		}
		
#if __ENABLE_LOD__
		//Calculate distance to camera & Draw reference
		if(Square_Distance(cameraPos, reference->pos.w) > state->c) {
			for(j = 0; j < object->shapeCount; ++j)
			if(!(object->shapeList[j]->flags & kFlag_MayHide))
			Shape_CallRenderingProc(object->shapeList[j], state, &reference->pos, negatedCamera, cameraPos, object->shapeList, visible);
		}
		else {
#endif
			for(j = 0; j < object->shapeCount; ++j)
			Shape_CallRenderingProc(object->shapeList[j], state, &reference->pos, negatedCamera, cameraPos, object->shapeList, visible);
#if __ENABLE_LOD__
		}
#endif

#if __RENDER_MODEL_BBOX__
		{
			long							k,
											l;
			TQAVGouraud						vertices[kBBSize];
			CollisionTilePtr				tile;
			Boolean							found = false;
			CollisionBoxPtr					box;
			MegaObjectPtr					mega = (MegaObjectPtr) reference->id;
			
			if(reference->flags & kRefFlag_CollisionDetection) {
				//Find the main ship is in
				tile = &collisionMap[(long) ((-terrain_MinZ + shipList[0].megaObject.object.pos.w.z) / kCollisionMapResolution) * collisionMapWidth
			 		+ (long) ((-terrain_MinX + shipList[0].megaObject.object.pos.w.x) / kCollisionMapResolution)];
				
				for(l = 0; l < tile->referenceCount; ++l)
				if(tile->referenceList[l] == reference) {
					for(k = 0; k < kBBSize; ++k) {
						vertices[k].r = 0.0;
						vertices[k].g = 1.0;
						vertices[k].b = 1.0;
						vertices[k].a = 1.0;
					}
					found = true;
					break;
				}
				if(!found)
				for(k = 0; k < kBBSize; ++k) {
					vertices[k].r = 1.0;
					vertices[k].g = 1.0;
					vertices[k].b = 0.0;
					vertices[k].a = 1.0;
				}
				
				for(k = 0; k < mega->boxCount; ++k) {
					box = &mega->boxList[k];
				
					for(l = 0; l < kBBSize; l++) {
						Matrix_TransformVector(&reference->pos, &box->corners[l], (VectorPtr) &vertices[l]);
						Matrix_TransformVector(negatedCamera, (VectorPtr) &vertices[l], (VectorPtr) &vertices[l]);
					}
				
					Gouraud_DrawLine(state, &vertices[0], &vertices[1]);
					Gouraud_DrawLine(state, &vertices[1], &vertices[2]);
					Gouraud_DrawLine(state, &vertices[2], &vertices[3]);
					Gouraud_DrawLine(state, &vertices[3], &vertices[0]);
					Gouraud_DrawLine(state, &vertices[4], &vertices[5]);
					Gouraud_DrawLine(state, &vertices[5], &vertices[6]);
					Gouraud_DrawLine(state, &vertices[6], &vertices[7]);
					Gouraud_DrawLine(state, &vertices[7], &vertices[4]);
					Gouraud_DrawLine(state, &vertices[0], &vertices[4]);
					Gouraud_DrawLine(state, &vertices[1], &vertices[5]);
					Gouraud_DrawLine(state, &vertices[2], &vertices[6]);
					Gouraud_DrawLine(state, &vertices[3], &vertices[7]);
				}
			}
		}
#endif
	}
}

void Render_SpecialItems(TerrainPtr terrain, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	long					i,
							j,
							visible;
	SpecialItemReferencePtr	item = terrain->itemReferenceList;
	ObjectPtr				object;
	Matrix					refPos;
	Vector					temp;
	float					amplitude,
							scaleX,
							scaleZ,
							time;
	
	amplitude = theTerrain.waveAmplitude;
	scaleX = theTerrain.waveScaleX;
	scaleZ = theTerrain.waveScaleZ;
	time = (float) state->frameTime / theTerrain.timeScale;
	
	Matrix_SetRotateY((float) state->frameTime * kPi / (kItem_RotateSpeed * kTimeUnit), &refPos);
	for(i = 0; i < terrain->itemReferenceCount; ++i, ++item) {
		object = (ObjectPtr) ((SpecialItemPtr) item->itemID)->modelID;
		
		//Test visibility
		Vector_Add(&item->position, &object->sphereCenter, &temp);
		visible = Sphere_Visible(negatedCamera, &temp, object->sphereRadius, state);
		if(visible == kNotVisible)
		continue;
		
		//Update reference height
		refPos.w.x = item->position.x;
		refPos.w.y = WaveHeight(time, item->position.x, item->position.z, scaleX, scaleZ, amplitude) + kItem_BonusHeight;
		refPos.w.z = item->position.z;
		
		//Calculate distance to camera & Draw reference
#if __ENABLE_LOD__
		if(Square_Distance(cameraPos, refPos.w) > state->c) {
			for(j = 0; j < object->shapeCount; ++j)
			if(!(object->shapeList[j]->flags & kFlag_MayHide))
			Shape_CallRenderingProc(object->shapeList[j], state, &refPos, negatedCamera, cameraPos, object->shapeList, visible);
		}
		else {
#endif
			for(j = 0; j < object->shapeCount; ++j)
			Shape_CallRenderingProc(object->shapeList[j], state, &refPos, negatedCamera, cameraPos, object->shapeList, visible);
#if __ENABLE_LOD__
		}
#endif
	}
}

