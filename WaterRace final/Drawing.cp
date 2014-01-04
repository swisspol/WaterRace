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
#include				"Strings.h"
#include				"Drivers.h"
#include				"Preferences.h"
#include				"Data Files.h"
#include				"Interface.h"
#include				"Play Modes.h"

#include				"Vector.h"
#include				"Infinity Rendering.h"
#include				"Infinity Post FX.h"

#include				"Infinity Utils.h"
#include				"Cinematic Utils.h"
#include				"Model Animation Utils.h"
#include				"Terrain Utils.h"
#include				"Clipping Utils.h"
#include				"Projection Utils.h"

#include				"Numeric Utils.h"

//CONSTANTES:

#define					kMap_Size					150.0
#define					kMap_Top					100.0
#define					kMap_Left					(thePrefs.renderWidth - kMap_Size)
#define					kMap_Bottom					(kMap_Top + kMap_Size)
#define					kMap_Right					(kMap_Left + kMap_Size)
#define					kMap_Depth					0.001
#define					kMapCrossSize				4

#define					kColor_Legend_r				1.0
#define					kColor_Legend_g				1.0
#define					kColor_Legend_b				0.5

#define					kColor_Info_r				1.0
#define					kColor_Info_g				0.5
#define					kColor_Info_b				0.0

#define					kColor_Points_r				1.0
#define					kColor_Points_g				1.0
#define					kColor_Points_b				0.0

#define					kColor_Instruction_r		1.0
#define					kColor_Instruction_g		1.0
#define					kColor_Instruction_b		1.0

#define					kColor_Ready_r				1.0
#define					kColor_Ready_g				0.5
#define					kColor_Ready_b				0.0

#define					kTab						16
#define					kTab2						25

#if __RENDER_CHECK_POINTS__
#define					kCheckPointHeight			4.0
#endif

//VARIABLES LOCALES:

static Boolean			startFXPlayed;
static long				lastCountDownDigit;

//ROUTINES:

static void SeaMesh_AnimateModels(SeaMeshPtr mesh)
{
	long				i;
	float				amplitude,
						scaleX,
						scaleZ;
	float				time;
	ModelReferencePtr	reference = mesh->referenceList;
	float				dif;
	
	time = (float) worldState->frameTime / theTerrain.timeScale;
	amplitude = theTerrain.waveAmplitude;
	scaleX = theTerrain.waveScaleX;
	scaleZ = theTerrain.waveScaleZ;
	
	for(i = 0; i < mesh->referenceCount; ++i, ++reference)
	if(reference->flags & kRefFlag_Float) {
		reference->pos.w.y = WaveHeight(time, reference->pos.w.x, reference->pos.w.z, scaleX, scaleZ, amplitude);
		if(reference->flags & kRefFlag_Roll) {
			dif = WaveHeight(time, reference->pos.w.x + 1.0, reference->pos.w.z, scaleX, scaleZ, amplitude) - reference->pos.w.y;
			reference->rotateZ = atan(dif) * reference->inertia;
			
			dif = WaveHeight(time, reference->pos.w.x, reference->pos.w.z + 1.0, scaleX, scaleZ, amplitude) - reference->pos.w.y;
			reference->rotateX = atan(dif) * reference->inertia; //pour être correct, il faut un signe moins ici!
			
			ModelReference_UpdateMatrix(reference);
		}
	}
}

static void Anim_Render(TerrainPtr terrain, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	long				i,
						j,
						visible;
	ObjectPtr			object;
	Vector				refPos;
	ModelAnimationPtr	animation;
	
	for(i = 0; i < terrain->animCount; ++i) {
		animation = terrain->animList[i];
		
		if(animation->modelID == kFlyThroughID)
		continue;
		
		if(!animation->running && (animation->flags & kAnimFlag_AutoRemove))
		continue;
		
		object = (ObjectPtr) animation->modelID;
		
		//Test visibility
		Matrix_TransformVector(&animation->pos, &object->sphereCenter, &refPos);
		visible = Sphere_Visible(negatedCamera, &refPos, object->sphereRadius, state);
		if(visible == kNotVisible)
		continue;
		if(visible > 0) //Forces per-shape clipping test
		visible = kUndetermined;
			
		//Do we need to update scripts?
		if(((ExtendedModelPtr) object)->hasScripts && (((ExtendedModelPtr) object)->lastScriptUpdateTime != state->frameTime)) {
			for(j = 0; j < ((MegaObjectPtr) object)->scriptCount; ++j)
			if(((MegaObjectPtr) object)->scriptList[j]->flags & kFlag_Running)
			Script_Run(((MegaObjectPtr) object)->scriptList[j], state->frameTime);
			((ExtendedModelPtr) object)->lastScriptUpdateTime = state->frameTime;
		}
		
#if __ENABLE_LOD__
		//Calculate distance to camera & Draw reference
		if(Square_Distance(cameraPos, animation->pos.w) > state->c) {
			for(j = 0; j < object->shapeCount; ++j)
			if(!(object->shapeList[j]->flags & kFlag_MayHide))
			Shape_CallRenderingProc(object->shapeList[j], state, &animation->pos, negatedCamera, cameraPos, object->shapeList, visible);
		}
		else {
#endif
			for(j = 0; j < object->shapeCount; ++j)
			Shape_CallRenderingProc(object->shapeList[j], state, &animation->pos, negatedCamera, cameraPos, object->shapeList, visible);
#if __ENABLE_LOD__
		}
#endif

#if __RENDER_MODEL_BOX__
		{
			long							k,
											l;
			TQAVGouraud						vertices[kBBSize];
			CollisionBoxPtr					box;
			MegaObjectPtr					mega = (MegaObjectPtr) (ObjectPtr) animation->modelID;
			
			if(animation->flags & kAnimFlag_CollisionDetection) {
				for(k = 0; k < kBBSize; ++k) {
					vertices[k].r = 0.0;
					vertices[k].g = 1.0;
					vertices[k].b = 1.0;
					vertices[k].a = 1.0;
				}
				
				for(k = 0; k < mega->boxCount; ++k) {
					box = &mega->boxList[k];
				
					vertices[0].x = box->center.x - box->xVector.x / 2.0 - box->yVector.x / 2.0 - box->zVector.x / 2.0;
					vertices[0].y = box->center.y - box->xVector.y / 2.0 - box->yVector.y / 2.0 - box->zVector.y / 2.0;
					vertices[0].z = box->center.z - box->xVector.z / 2.0 - box->yVector.z / 2.0 - box->zVector.z / 2.0;
					
					vertices[1].x = box->center.x + box->xVector.x / 2.0 - box->yVector.x / 2.0 - box->zVector.x / 2.0;
					vertices[1].y = box->center.y + box->xVector.y / 2.0 - box->yVector.y / 2.0 - box->zVector.y / 2.0;
					vertices[1].z = box->center.z + box->xVector.z / 2.0 - box->yVector.z / 2.0 - box->zVector.z / 2.0;
					
					vertices[2].x = box->center.x + box->xVector.x / 2.0 - box->yVector.x / 2.0 + box->zVector.x / 2.0;
					vertices[2].y = box->center.y + box->xVector.y / 2.0 - box->yVector.y / 2.0 + box->zVector.y / 2.0;
					vertices[2].z = box->center.z + box->xVector.z / 2.0 - box->yVector.z / 2.0 + box->zVector.z / 2.0;
					
					vertices[3].x = box->center.x - box->xVector.x / 2.0 - box->yVector.x / 2.0 + box->zVector.x / 2.0;
					vertices[3].y = box->center.y - box->xVector.y / 2.0 - box->yVector.y / 2.0 + box->zVector.y / 2.0;
					vertices[3].z = box->center.z - box->xVector.z / 2.0 - box->yVector.z / 2.0 + box->zVector.z / 2.0;
					
					vertices[4].x = box->center.x - box->xVector.x / 2.0 + box->yVector.x / 2.0 - box->zVector.x / 2.0;
					vertices[4].y = box->center.y - box->xVector.y / 2.0 + box->yVector.y / 2.0 - box->zVector.y / 2.0;
					vertices[4].z = box->center.z - box->xVector.z / 2.0 + box->yVector.z / 2.0 - box->zVector.z / 2.0;
					
					vertices[5].x = box->center.x + box->xVector.x / 2.0 + box->yVector.x / 2.0 - box->zVector.x / 2.0;
					vertices[5].y = box->center.y + box->xVector.y / 2.0 + box->yVector.y / 2.0 - box->zVector.y / 2.0;
					vertices[5].z = box->center.z + box->xVector.z / 2.0 + box->yVector.z / 2.0 - box->zVector.z / 2.0;
					
					vertices[6].x = box->center.x + box->xVector.x / 2.0 + box->yVector.x / 2.0 + box->zVector.x / 2.0;
					vertices[6].y = box->center.y + box->xVector.y / 2.0 + box->yVector.y / 2.0 + box->zVector.y / 2.0;
					vertices[6].z = box->center.z + box->xVector.z / 2.0 + box->yVector.z / 2.0 + box->zVector.z / 2.0;
					
					vertices[7].x = box->center.x - box->xVector.x / 2.0 + box->yVector.x / 2.0 + box->zVector.x / 2.0;
					vertices[7].y = box->center.y - box->xVector.y / 2.0 + box->yVector.y / 2.0 + box->zVector.y / 2.0;
					vertices[7].z = box->center.z - box->xVector.z / 2.0 + box->yVector.z / 2.0 + box->zVector.z / 2.0;
				
					for(l = 0; l < kBBSize; l++) {
						Matrix_TransformVector(&animation->pos, (VectorPtr) &vertices[l], (VectorPtr) &vertices[l]);
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

static void Draw_ShipWake(StatePtr state, MatrixPtr negatedCamera, ExtendedShipPtr ship)
{
	TQAIndexedTriangle	*trianglePtr;
	TQAVTexture			*texturePtr;
	long				triangleCount,
						triangleCount2,
						verticeCount;
	long				i;
	Matrix				r1;
	float				waveHeight;
	long				spriteCount = 0;
	Data_ShipPhysicParameterPtr	params = ship->params;
	
	//Is this ship visible?
	Matrix_Cat(&ship->megaObject.object.pos, negatedCamera, &r1);
	if(Box_Visible(&r1, ship->megaObject.object.boundingBox, state) == kNotVisible)
	return;
		
	//Do we need to create a new wake sprite?
	waveHeight = WaveHeight((float) state->frameTime / theTerrain.timeScale, ship->megaObject.object.pos.w.x, ship->megaObject.object.pos.w.z, theTerrain.waveScaleX, theTerrain.waveScaleZ, theTerrain.waveAmplitude);
	if((ship->megaObject.object.pos.w.y + params->wakeShipInWaterOffset <= waveHeight) && (ship->player->speed > 6.0) && (FVector_Distance(ship->wakeLastPosition, ship->megaObject.object.pos.w) > params->wakeMinDistance)) {
		unsigned long		minTime = state->frameTime;
		long				daMin = 0;
	
		//Find an available sprite slot
		for(i = 0; i < kWakeNumSprites; ++i)
		if(ship->wakeTime[i] < minTime) {
			daMin = i;
			minTime = ship->wakeTime[i];
		}
		
		//Create new sprite
		Vector_MultiplyAdd(params->wakePositionOffset, &ship->megaObject.object.pos.z, &ship->megaObject.object.pos.w, &ship->wakePositions[daMin]);
		ship->wakeDirections[daMin].x = ship->dynamics.velocityG.x / ship->player->speed;
		ship->wakeDirections[daMin].z = ship->dynamics.velocityG.z / ship->player->speed;
		ship->wakeTime[daMin] = state->frameTime;
		ship->wakeLastPosition = ship->megaObject.object.pos.w;
	}
	
	//Set texture
	if(wakeTexture != state->lastTexture) {
		QASetPtr(state->drawContext, kQATag_Texture, wakeTexture);
		state->lastTexture = wakeTexture;
	}
	
	//Set texture method
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate);
	
	//Set transparency mode
	QASetInt(state->drawContext, kQATag_Blend, kQABlend_PreMultiply);
	
	//Create vertice buffer and trianglesBuffer buffer
	verticeCount = 0;
	triangleCount = 0;
	triangleCount2 = 0;
	
	//Fill vertice buffer
	texturePtr = vTextureBuffer;
	
	for(i = 0; i < kWakeNumSprites; ++i) {
		if(state->frameTime >= ship->wakeTime[i] + params->wakeLifeTime)
		continue;
		
		float width = params->wakeHalfWidth - params->wakeHalfWidth * (float) (state->frameTime - ship->wakeTime[i]) / (float) params->wakeLifeTime;
		
		//Point #0
		texturePtr->x = ship->wakePositions[i].x - ship->wakeDirections[i].z * width;
		texturePtr->y = kWakeHeight;
		texturePtr->z = ship->wakePositions[i].z + ship->wakeDirections[i].x * width;
		Matrix_TransformVector(negatedCamera, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
		texturePtr->uOverW = 0.0;
		texturePtr->vOverW = 1.0;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
		texturePtr->a = 1.0;
		++texturePtr;
		
		//Point #1
		texturePtr->x = ship->wakePositions[i].x + ship->wakeDirections[i].z * width;
		texturePtr->y = kWakeHeight;
		texturePtr->z = ship->wakePositions[i].z - ship->wakeDirections[i].x * width;
		Matrix_TransformVector(negatedCamera, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
		texturePtr->uOverW = 1.0;
		texturePtr->vOverW = 1.0;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
		texturePtr->a = 1.0;
		++texturePtr;
		
		//Point #2
		texturePtr->x = ship->wakePositions[i].x - ship->wakeDirections[i].x * params->wakeLength + ship->wakeDirections[i].z * width;
		texturePtr->y = kWakeHeight;
		texturePtr->z = ship->wakePositions[i].z - ship->wakeDirections[i].z * params->wakeLength - ship->wakeDirections[i].x * width;
		Matrix_TransformVector(negatedCamera, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
		texturePtr->uOverW = 1.0;
		texturePtr->vOverW = 0.0;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
		texturePtr->a = 1.0;
		++texturePtr;
		
		//Point #3
		texturePtr->x = ship->wakePositions[i].x - ship->wakeDirections[i].x * params->wakeLength - ship->wakeDirections[i].z * width;
		texturePtr->y = kWakeHeight;
		texturePtr->z = ship->wakePositions[i].z - ship->wakeDirections[i].z * params->wakeLength + ship->wakeDirections[i].x * width;
		Matrix_TransformVector(negatedCamera, (VectorPtr) texturePtr, (VectorPtr) texturePtr);
		texturePtr->uOverW = 0.0;
		texturePtr->vOverW = 0.0;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
		texturePtr->a = 1.0;
		++texturePtr;
		
		verticeCount += 4;
		++spriteCount;
	}
	
	//Fill trianglesBuffer buffer #1
	for(i = 0; i < spriteCount; ++i) {
		ClipTriangleTexture_Z(state, vTextureBuffer, &verticeCount, trianglesBuffer, &triangleCount, 4 * i + 0, 4 * i + 1, 4 * i + 2);
		ClipTriangleTexture_Z(state, vTextureBuffer, &verticeCount, trianglesBuffer, &triangleCount, 4 * i + 2, 4 * i + 3, 4 * i + 0);
	}
	
	//Project vTextureBuffer
	Project_TextureBuffer(state, verticeCount);
	
	//Clip trianglesBuffer and fill triangle buffer #2
	trianglePtr = trianglesBuffer;
	for(i = 0; i < triangleCount; ++i) {
		ClipTriangleTexture_XY(state, vTextureBuffer, &verticeCount, trianglesBuffer2, &triangleCount2, trianglePtr->vertices[0], trianglePtr->vertices[1], trianglePtr->vertices[2]);
		++trianglePtr;
	}
		
	//Draw trianglesBuffer
	QASubmitVerticesTexture(state->drawContext, verticeCount, vTextureBuffer);
	QADrawTriMeshTexture(state->drawContext, triangleCount2, trianglesBuffer2);
	
	//Restore transparency mode
	QASetInt(state->drawContext, kQATag_Blend, kQABlend_Interpolate);
	
	//Flush!
	//QAFlush(state->drawContext);
}

static void Ships_Render(StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	long				i,
						j,
						visible;
	ObjectPtr			object;
	Vector				refPos;
	
	for(i = 0; i < shipCount; ++i) {
		object = &shipList[i].megaObject.object;
		
		//Test visibility
		Matrix_TransformVector(&object->pos, &object->sphereCenter, &refPos);
		visible = Sphere_Visible(negatedCamera, &refPos, object->sphereRadius * object->scale, state);
		if(visible == kNotVisible)
		continue;
		
		/*Matrix		r1;
		Matrix_Cat(&object->pos, negatedCamera, &r1);
		visible = ModelBBox_Visible(&r1, object->boundingBox, state, cameraPos);
		if(visible == kNotVisible)
		continue;*/
		
		for(j = 0; j < object->shapeCount; ++j) {
			//Don't draw head shape if we are in head cam mode
			if((cameraMode == kCameraMode_Head) && (cameraTargetShip == &shipList[i]) && (object->shapeList[j] == shipList[i].headShape))
			continue;
			//Don't draw turbo flame if we don't need it
			if(!shipList[i].turboFlame && ((object->shapeList[j] == shipList[i].flameShape_1) || (object->shapeList[j] == shipList[i].flameShape_2)))
			continue;
			//Render shape
#if 1
			if((&shipList[i] == cameraTargetShip) && ((cameraMode == kCameraMode_Head) || (cameraMode == kCameraMode_OnBoard)))
			Shape_CallRenderingProc(object->shapeList[j], state, &object->pos, negatedCamera, cameraPos, object->shapeList, kUndetermined);
			else
#endif
			Shape_CallRenderingProc(object->shapeList[j], state, &object->pos, negatedCamera, cameraPos, object->shapeList, visible);
		}
		
#if __RENDER_SHIP_COLLISION_NORMAL__
		{
			TQAVGouraud						vertices[2];
			long							k;
			
			vertices[0].x = object->pos.w.x;
			vertices[0].y = 4.0;
			vertices[0].z = object->pos.w.z;
			Vector_MultiplyAdd(4.0, &shipList[i].normal, (VectorPtr) &vertices[0], (VectorPtr) &vertices[1]);
			Matrix_TransformVector(negatedCamera, (VectorPtr) &vertices[0], (VectorPtr) &vertices[0]);
			Matrix_TransformVector(negatedCamera, (VectorPtr) &vertices[1], (VectorPtr) &vertices[1]);
			for(k = 0; k < 2; k++) {
				vertices[k].r = 1.0;
				vertices[k].g = 0.0;
				vertices[k].b = 0.0;
				vertices[k].a = 1.0;
			}	
			Gouraud_DrawLine(state, &vertices[0], &vertices[1]);	
		}
#endif
	
#if __RENDER_SHIP_PHYSIC_VOLUMES__
		{
			long							k,
											l;
			TQAVGouraud						vertices[kBBSize];
			DynamicsPtr			 			dynamics = &shipList[i].dynamics;
			
			for(l = 0; l < kVolumeCount; ++l) {
				for(k = 0; k < kBBSize; k++) {
					Matrix_TransformVector(&object->pos, &dynamics->volumeList[l].boundingBox[k], (VectorPtr) &vertices[k]);
					Matrix_TransformVector(negatedCamera, (VectorPtr) &vertices[k], (VectorPtr) &vertices[k]);
					vertices[k].r = 1.0;
					vertices[k].g = 0.0;
					vertices[k].b = 0.0;
					vertices[k].a = 1.0;
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
#endif

#if __RENDER_SHIP_BBOX__
		{
			long							k,
											l;
			TQAVGouraud						vertices[kBBSize];
			CollisionBoxPtr					box;
			
			for(k = 0; k < kBBSize; k++) {
				vertices[k].r = 1.0;
				vertices[k].g = 1.0;
				vertices[k].b = 0.0;
				vertices[k].a = 1.0;
			}
			
			for(k = 0; k < shipList[i].megaObject.boxCount; ++k) {
				box = &shipList[i].megaObject.boxList[k];
			
				vertices[0].x = box->center.x - box->xVector.x / 2.0 - box->yVector.x / 2.0 - box->zVector.x / 2.0;
				vertices[0].y = box->center.y - box->xVector.y / 2.0 - box->yVector.y / 2.0 - box->zVector.y / 2.0;
				vertices[0].z = box->center.z - box->xVector.z / 2.0 - box->yVector.z / 2.0 - box->zVector.z / 2.0;
				
				vertices[1].x = box->center.x + box->xVector.x / 2.0 - box->yVector.x / 2.0 - box->zVector.x / 2.0;
				vertices[1].y = box->center.y + box->xVector.y / 2.0 - box->yVector.y / 2.0 - box->zVector.y / 2.0;
				vertices[1].z = box->center.z + box->xVector.z / 2.0 - box->yVector.z / 2.0 - box->zVector.z / 2.0;
				
				vertices[2].x = box->center.x + box->xVector.x / 2.0 - box->yVector.x / 2.0 + box->zVector.x / 2.0;
				vertices[2].y = box->center.y + box->xVector.y / 2.0 - box->yVector.y / 2.0 + box->zVector.y / 2.0;
				vertices[2].z = box->center.z + box->xVector.z / 2.0 - box->yVector.z / 2.0 + box->zVector.z / 2.0;
				
				vertices[3].x = box->center.x - box->xVector.x / 2.0 - box->yVector.x / 2.0 + box->zVector.x / 2.0;
				vertices[3].y = box->center.y - box->xVector.y / 2.0 - box->yVector.y / 2.0 + box->zVector.y / 2.0;
				vertices[3].z = box->center.z - box->xVector.z / 2.0 - box->yVector.z / 2.0 + box->zVector.z / 2.0;
				
				vertices[4].x = box->center.x - box->xVector.x / 2.0 + box->yVector.x / 2.0 - box->zVector.x / 2.0;
				vertices[4].y = box->center.y - box->xVector.y / 2.0 + box->yVector.y / 2.0 - box->zVector.y / 2.0;
				vertices[4].z = box->center.z - box->xVector.z / 2.0 + box->yVector.z / 2.0 - box->zVector.z / 2.0;
				
				vertices[5].x = box->center.x + box->xVector.x / 2.0 + box->yVector.x / 2.0 - box->zVector.x / 2.0;
				vertices[5].y = box->center.y + box->xVector.y / 2.0 + box->yVector.y / 2.0 - box->zVector.y / 2.0;
				vertices[5].z = box->center.z + box->xVector.z / 2.0 + box->yVector.z / 2.0 - box->zVector.z / 2.0;
				
				vertices[6].x = box->center.x + box->xVector.x / 2.0 + box->yVector.x / 2.0 + box->zVector.x / 2.0;
				vertices[6].y = box->center.y + box->xVector.y / 2.0 + box->yVector.y / 2.0 + box->zVector.y / 2.0;
				vertices[6].z = box->center.z + box->xVector.z / 2.0 + box->yVector.z / 2.0 + box->zVector.z / 2.0;
				
				vertices[7].x = box->center.x - box->xVector.x / 2.0 + box->yVector.x / 2.0 + box->zVector.x / 2.0;
				vertices[7].y = box->center.y - box->xVector.y / 2.0 + box->yVector.y / 2.0 + box->zVector.y / 2.0;
				vertices[7].z = box->center.z - box->xVector.z / 2.0 + box->yVector.z / 2.0 + box->zVector.z / 2.0;
			
				for(l = 0; l < kBBSize; l++) {
					Matrix_TransformVector(&object->pos, (VectorPtr) &vertices[l], (VectorPtr) &vertices[l]);
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
#endif

#if __RENDER_BOT_TARGET__
		{
			VectorPtr		target;
			TQAVGouraud		v1, v2;
			Vector			p1, p2;
			Vector			pos = {0.0, 2.0, 0.0};
			
			v1.r = 1.0;
			v1.g = 1.0;
			v1.b = 1.0;
			v1.a = 1.0;
			Matrix_TransformVector(&object->pos, &pos, &p1);
			Matrix_TransformVector(negatedCamera, &p1, (VectorPtr) &v1);
			
			if(shipList[i].player->shipPilotType == kPilot_Bot)
			target = &shipList[i].botTarget;
			else
			target = &theRace->doorList[shipList[i].nextCheckPoint].middle;
			
			v2.r = 1.0;
			v2.g = 0.0;
			v2.b = 1.0;
			v2.a = 1.0;
			p2.x = target->x;
			p2.y = p1.y;
			p2.z = target->z;
			Matrix_TransformVector(negatedCamera, &p2, (VectorPtr) &v2);
			Gouraud_DrawLine(state, &v1, &v2);
			v1.r = 1.0;
			v1.g = 0.0;
			v1.b = 1.0;
			v1.a = 1.0;
			p1.x = target->x;
			p1.y = 0.0;
			p1.z = target->z;
			Matrix_TransformVector(negatedCamera, &p1, (VectorPtr) &v1);
			Gouraud_DrawLine(state, &v1, &v2);
		}
#endif
	}
}

#if __RENDER_TRACK__
inline void Fill_TQAVGouraud_Color(VectorPtr source, TQAVGouraud* dest, float r, float g, float b)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
	dest->r = r;
	dest->g = g;
	dest->b = b;
	dest->a = 1.0;
}

static void Draw_Track(StatePtr state, RaceDescriptionPtr track, MatrixPtr negatedCamera)
{
	long				x;
	Vector				lastLeft,
						nextLeft,
						lastRight,
						nextRight;
	TQAVGouraud			verts[2];
	float				r = 1.0,
						g = 0.0,
						b = 0.0;
	Matrix				pos;
	
	Matrix_Clear(&pos);
	
	//Draw first check point
	Matrix_TransformVector(negatedCamera, &track->doorList[0].rightMark, &lastLeft);
	Matrix_TransformVector(negatedCamera, &track->doorList[0].leftMark, &lastRight);
	Fill_TQAVGouraud_Color(&lastLeft, &verts[0], r, g, b);
	Fill_TQAVGouraud_Color(&lastRight, &verts[1], r, g, b);
	//if((verts[0].z < kMaxViewDistance) && (verts[1].z < kMaxViewDistance))
	Gouraud_DrawLine(state, &verts[0], &verts[1]);

	//Draw path
	for(x = 1; x < track->doorCount; ++x) {
		//Get matrix
		Matrix_TransformVector(negatedCamera, &track->doorList[x].rightMark, &nextLeft);
		Matrix_TransformVector(negatedCamera, &track->doorList[x].leftMark, &nextRight);
	
		//Draw left line
		Fill_TQAVGouraud_Color(&lastLeft, &verts[0], r, g, b);
		Fill_TQAVGouraud_Color(&nextLeft, &verts[1], r, g, b);
		//if((verts[0].z < kMaxViewDistance) && (verts[1].z < kMaxViewDistance))
		Gouraud_DrawLine(state, &verts[0], &verts[1]);
		
		//Draw right line
		Fill_TQAVGouraud_Color(&lastRight, &verts[0], r, g, b);
		Fill_TQAVGouraud_Color(&nextRight, &verts[1], r, g, b);
		//if((verts[0].z < kMaxViewDistance) && (verts[1].z < kMaxViewDistance))
		Gouraud_DrawLine(state, &verts[0], &verts[1]);
		
		//Draw door
		Fill_TQAVGouraud_Color(&nextLeft, &verts[0], r, g, b);
		Fill_TQAVGouraud_Color(&nextRight, &verts[1], r, g, b);
		//if((verts[0].z < kMaxViewDistance) && (verts[1].z < kMaxViewDistance))
		Gouraud_DrawLine(state, &verts[0], &verts[1]);
		
		lastLeft = nextLeft;
		lastRight = nextRight;
	}
}
#endif

#if __RENDER_CHECK_POINTS__
static void CheckPoints_Render(RaceDescriptionPtr race, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	Matrix				pos;
	RaceDoorPtr			door = &race->doorList[0];
	long				num = door->nextCheckPoint;
	
	pos.y.x = 0.0;
	pos.y.y = 1.0;
	pos.y.z = 0.0;
	pos.w.y = kCheckPointHeight;
	
	//Build door matrix
	pos.x = door->doorMatrix.x;
	pos.z = door->doorMatrix.z;
	
	//Draw left
	pos.w.x = door->leftMark.x;
	pos.w.z = door->leftMark.z;
	Shape_CallRenderingProc(start_left, state, &pos, negatedCamera, cameraPos, nil, kUndetermined);
	
	//Draw right
	pos.w.x = door->rightMark.x;
	pos.w.z = door->rightMark.z;
	Shape_CallRenderingProc(start_right, state, &pos, negatedCamera, cameraPos, nil, kUndetermined);
	
	//Draw middle
	pos.w.x = door->middle.x;
	pos.w.z = door->middle.z;
	Vector_Multiply(door->doorWidth, &pos.x, &pos.x);
	Shape_CallRenderingProc(start_middle, state, &pos, negatedCamera, cameraPos, nil, kUndetermined);

	while(num > 0) {
		door = &race->doorList[num];
		
		//Build door matrix
		pos.x = door->doorMatrix.x;
		pos.z = door->doorMatrix.z;
		
		//Draw left
		pos.w.x = door->leftMark.x;
		pos.w.z = door->leftMark.z;
		Shape_CallRenderingProc(checkPoint_left, state, &pos, negatedCamera, cameraPos, nil, kUndetermined);
		
		//Draw right
		pos.w.x = door->rightMark.x;
		pos.w.z = door->rightMark.z;
		Shape_CallRenderingProc(checkPoint_right, state, &pos, negatedCamera, cameraPos, nil, kUndetermined);
		
		//Draw middle
		pos.w.x = door->middle.x;
		pos.w.z = door->middle.z;
		Vector_Multiply(door->doorWidth, &pos.x, &pos.x);
		Shape_CallRenderingProc(checkPoint_middle, state, &pos, negatedCamera, cameraPos, nil, kUndetermined);
		
		num = door->nextCheckPoint;
	}
}
#endif

static void Draw_Arrow(RaceDescriptionPtr race, long targetCheck, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	Matrix			pos;
	float			move = 5.0,
					move2 = 2.0;
	Vector			direction;
	RaceDoorPtr		door = &race->doorList[targetCheck];
	
	//Calculate direction to door
	direction.x = door->middle.x - cameraPos->x;
	direction.y = 0.0;
	direction.z = door->middle.z - cameraPos->z;
	
	//If we are in the correct direction and if we're too close, get next direction
	if((direction.x * direction.x + direction.z * direction.z) < door->squareDoorWidth) {
		Draw_Arrow(race, race->doorList[targetCheck].nextCheckPoint, state, negatedCamera, cameraPos);
		return;
	}
	
	//Setup arrow matrix
	Vector_Normalize(&direction, &pos.z);
	pos.y.x = 0.0;
	pos.y.y = 1.0;
	pos.y.z = 0.0;
	Vector_CrossProduct(&pos.y, &pos.z, &pos.x);
	pos.w.x = worldCamera.camera.w.x + worldCamera.camera.z.x * move + worldCamera.camera.y.x * move2;
	pos.w.y = worldCamera.camera.w.y + worldCamera.camera.z.y * move + worldCamera.camera.y.y * move2;
	pos.w.z = worldCamera.camera.w.z + worldCamera.camera.z.z * move + worldCamera.camera.y.z * move2;
	
	//Render arrow
	Shape_CallRenderingProc(arrowShape, state, &pos, negatedCamera, cameraPos, nil, kNoClipping);
}

#if __RENDER_MAP__
#if __ROTATING_MAP__
static void Draw_Map(RaceDescriptionPtr race, StatePtr state, MatrixPtr negatedCamera)
{
	long			i;
	RaceDoorPtr 	door;
	TQAVGouraud		v1,
					v2;
	float			scale;
	Matrix			m,
					n;
	Vector			temp;
	
	//Compute map scale
	if(terrain_MaxX - terrain_MinX > terrain_MaxZ - border_MinZ)
	scale = -(terrain_MaxX - terrain_MinX) / kMap_Size;
	else
	scale = -(terrain_MaxZ - terrain_MinZ) / kMap_Size;
	
	//Compute world-to-map matrix
	m = cameraTargetShip->megaObject.object.pos;
	m.x.y = 0.0;
	Vector_Normalize(&m.x, &m.x);
	m.y.x = 0.0;
	m.y.y = 1.0;
	m.y.z = 0.0;
	m.z.y = 0.0;
	Vector_Normalize(&m.z, &m.z);
	m.w.y = 0.0;
	Matrix_Negate(&m, &n);
	
	//Preset vertices
	v1.z = kMap_Depth; v1.invW = 1.0;
	v2.z = kMap_Depth; v2.invW = 1.0;
	v1.a = 1.0;
	v2.a = 1.0;

#if __DROP_SHADOW_MAP__
	//Preset colors
	v1.r = v1.g = v1.b = 0.0;
	v2.r = v2.g = v2.b = 0.0;
	
	//Draw track
	door = race->doorList;
	Matrix_TransformVector(&n, &door->middle, &temp);
	v1.x = temp.x / scale + kMap_Left + kMap_Size / 2.0 + 1.0;
	v1.y = temp.z / scale + kMap_Top + kMap_Size / 2.0 + 1.0;
	++door;
	for(i = 1; i < race->doorCount; ++i, ++door) {
#if __RENDER_ONLY_CHECKPOINTS_IN_MAP__
		if(!door->isCheckPoint)
		continue;
#endif	
		Matrix_TransformVector(&n, &door->middle, &temp);
		v2.x = temp.x / scale + kMap_Left + kMap_Size / 2.0 + 1.0;
		v2.y = temp.z / scale + kMap_Top + kMap_Size / 2.0 + 1.0;
		QADrawLine(state->drawContext, &v1, &v2);
		
		v1.x = v2.x;
		v1.y = v2.y;
	}
	Matrix_TransformVector(&n, &race->doorList[0].middle, &temp);
	v2.x = temp.x / scale + kMap_Left + kMap_Size / 2.0 + 1.0;
	v2.y = temp.z / scale + kMap_Top + kMap_Size / 2.0 + 1.0;
	QADrawLine(state->drawContext, &v1, &v2);
#if __RENDER_ONLY_CHECKPOINTS_IN_MAP__
	door = &race->doorList[race->doorCount - 1];
	if(!door->isCheckPoint) {
		Matrix_TransformVector(&n, &door->middle, &temp);
		v2.x = temp.x / scale + kMap_Left + kMap_Size / 2.0 + 1.0;
		v2.y = temp.z / scale + kMap_Top + kMap_Size / 2.0 + 1.0;
		QADrawLine(state->drawContext, &v1, &v2);
	}
#endif

	//Draw camera target ship position
	v1.x = (long) (kMap_Left + kMap_Size / 2.0) - kMapCrossSize + 1.0;
	v1.y = (long) (kMap_Top + kMap_Size / 2.0) + 1.0;
	v2.x = v1.x + 2 * kMapCrossSize + 1;
	v2.y = v1.y;
	QADrawLine(state->drawContext, &v1, &v2);
	v1.x = v1.x + kMapCrossSize;
	v1.y = v1.y - kMapCrossSize;
	v2.x = v1.x;
	v2.y = v1.y + 2 * kMapCrossSize + 1;
	QADrawLine(state->drawContext, &v1, &v2);
	
	//Other ship positions
	for(i = 0; i < shipCount; ++i)
	if(&shipList[i] != cameraTargetShip) {
		Matrix_TransformVector(&n, &shipList[i].megaObject.object.pos.w, &temp);
		v1.x = (long) (temp.x / scale + kMap_Left + kMap_Size / 2.0) - kMapCrossSize + 1.0;
		v1.y = (long) (temp.z / scale + kMap_Top + kMap_Size / 2.0) + 1.0;
		v2.x = v1.x + 2 * kMapCrossSize + 1;
		v2.y = v1.y;
		QADrawLine(state->drawContext, &v1, &v2);
		v1.x = v1.x + kMapCrossSize;
		v1.y = v1.y - kMapCrossSize;
		v2.x = v1.x;
		v2.y = v1.y + 2 * kMapCrossSize + 1;
		QADrawLine(state->drawContext, &v1, &v2);
	}
#endif
	
	//Preset colors
	v1.r = v1.g = v1.b = 1.0;
	v2.r = v2.g = v2.b = 1.0;
	
	//Draw track
	door = race->doorList;
	Matrix_TransformVector(&n, &door->middle, &temp);
	v1.x = temp.x / scale + kMap_Left + kMap_Size / 2.0;
	v1.y = temp.z / scale + kMap_Top + kMap_Size / 2.0;
	++door;
	for(i = 1; i < race->doorCount; ++i, ++door) {
#if __RENDER_ONLY_CHECKPOINTS_IN_MAP__
		if(!door->isCheckPoint)
		continue;
#endif	
		Matrix_TransformVector(&n, &door->middle, &temp);
		v2.x = temp.x / scale + kMap_Left + kMap_Size / 2.0;
		v2.y = temp.z / scale + kMap_Top + kMap_Size / 2.0;
		QADrawLine(state->drawContext, &v1, &v2);
		
		v1.x = v2.x;
		v1.y = v2.y;
	}
	Matrix_TransformVector(&n, &race->doorList[0].middle, &temp);
	v2.x = temp.x / scale + kMap_Left + kMap_Size / 2.0;
	v2.y = temp.z / scale + kMap_Top + kMap_Size / 2.0;
	QADrawLine(state->drawContext, &v1, &v2);
#if __RENDER_ONLY_CHECKPOINTS_IN_MAP__
	door = &race->doorList[race->doorCount - 1];
	if(!door->isCheckPoint) {
		Matrix_TransformVector(&n, &door->middle, &temp);
		v2.x = temp.x / scale + kMap_Left + kMap_Size / 2.0;
		v2.y = temp.z / scale + kMap_Top + kMap_Size / 2.0;
		QADrawLine(state->drawContext, &v1, &v2);
	}
#endif

	//Draw camera target ship position (Red cross)
	v1.g = v1.b = 0.0;
	v2.g = v2.b = 0.0;
	v1.x = (long) (kMap_Left + kMap_Size / 2.0) - kMapCrossSize;
	v1.y = (long) (kMap_Top + kMap_Size / 2.0);
	v2.x = v1.x + 2 * kMapCrossSize + 1;
	v2.y = v1.y;
	QADrawLine(state->drawContext, &v1, &v2);
	v1.x = v1.x + kMapCrossSize;
	v1.y = v1.y - kMapCrossSize;
	v2.x = v1.x;
	v2.y = v1.y + 2 * kMapCrossSize + 1;
	QADrawLine(state->drawContext, &v1, &v2);
	
	//Other ship positions (Green crosses)
	v1.r = v2.r = 0.0;
	v1.g = v1.g = 1.0;
	for(i = 0; i < shipCount; ++i)
	if(&shipList[i] != cameraTargetShip) {
		Matrix_TransformVector(&n, &shipList[i].megaObject.object.pos.w, &temp);
		v1.x = (long) (temp.x / scale + kMap_Left + kMap_Size / 2.0) - kMapCrossSize;
		v1.y = (long) (temp.z / scale + kMap_Top + kMap_Size / 2.0);
		v2.x = v1.x + 2 * kMapCrossSize + 1;
		v2.y = v1.y;
		QADrawLine(state->drawContext, &v1, &v2);
		v1.x = v1.x + kMapCrossSize;
		v1.y = v1.y - kMapCrossSize;
		v2.x = v1.x;
		v2.y = v1.y + 2 * kMapCrossSize + 1;
		QADrawLine(state->drawContext, &v1, &v2);
	}
}
#else
static void Draw_Map(RaceDescriptionPtr race, StatePtr state, MatrixPtr negatedCamera)
{
	long			i;
	RaceDoorPtr 	door = race->doorList;
	TQAVGouraud		v1,
					v2;
	float			scale;
	
	if(terrain_MaxX - terrain_MinX > terrain_MaxZ - border_MinZ)
	scale = (terrain_MaxX - terrain_MinX) / kMap_Size;
	else
	scale = (terrain_MaxZ - terrain_MinZ) / kMap_Size;
	
	//Draw track
	v1.z = kMap_Depth; v1.invW = 1.0;
	v2.z = kMap_Depth; v2.invW = 1.0;
	v1.r = v1.g = v1.b = v1.a = 1.0;
	v2.r = v2.g = v2.b = v2.a = 1.0;
	v1.x = door->middle.x / scale + kMap_Left + kMap_Size / 2.0;
	v1.y = door->middle.z / scale + kMap_Top + kMap_Size / 2.0;
	++door;
	for(i = 1; i < race->doorCount; ++i, ++door) {
#if __RENDER_ONLY_CHECKPOINTS_IN_MAP__
		if(!door->isCheckPoint)
		continue;
#endif	
		v2.x = door->middle.x / scale + kMap_Left + kMap_Size / 2.0;
		v2.y = door->middle.z / scale + kMap_Top + kMap_Size / 2.0;
		QADrawLine(state->drawContext, &v1, &v2);
		
		v1.x = v2.x;
		v1.y = v2.y;
	}
	v2.x = race->doorList[0].middle.x / scale + kMap_Left + kMap_Size / 2.0;
	v2.y = race->doorList[0].middle.z / scale + kMap_Top + kMap_Size / 2.0;
	QADrawLine(state->drawContext, &v1, &v2);
#if __RENDER_ONLY_CHECKPOINTS_IN_MAP__
	door = &race->doorList[race->doorCount - 1];
	if(!(check->flags & kCheckFlag_IsCheckPoint)) {
		v2.x = check->middle.x / scale + kMap_Left + kMap_Size / 2.0;
		v2.y = check->middle.z / scale + kMap_Top + kMap_Size / 2.0;
		QADrawLine(state->drawContext, &v1, &v2);
	}
#endif

	//Draw camera target ship position (Red cross)
	v1.g = v1.b = 0.0;
	v2.g = v2.b = 0.0;
	v1.x = (long) (cameraTargetShip->megaObject.object.pos.w.x / scale + kMap_Left + kMap_Size / 2.0) - kMapCrossSize;
	v1.y = (long) (cameraTargetShip->megaObject.object.pos.w.z / scale + kMap_Top + kMap_Size / 2.0);
	v2.x = v1.x + 2 * kMapCrossSize + 1;
	v2.y = v1.y;
	QADrawLine(state->drawContext, &v1, &v2);
	v1.x = v1.x + kMapCrossSize;
	v1.y = v1.y - kMapCrossSize;
	v2.x = v1.x;
	v2.y = v1.y + 2 * kMapCrossSize + 1;
	QADrawLine(state->drawContext, &v1, &v2);
	
	//Other ship positions (Green crosses)
	v1.r = v2.r = 0.0;
	v1.g = v1.g = 1.0;
	for(i = 0; i < shipCount; ++i)
	if(&shipList[i] != cameraTargetShip) {
		v1.x = (long) (shipList[i].megaObject.object.pos.w.x / scale + kMap_Left + kMap_Size / 2.0) - kMapCrossSize;
		v1.y = (long) (shipList[i].megaObject.object.pos.w.z / scale + kMap_Top + kMap_Size / 2.0);
		v2.x = v1.x + 2 * kMapCrossSize + 1;
		v2.y = v1.y;
		QADrawLine(state->drawContext, &v1, &v2);
		v1.x = v1.x + kMapCrossSize;
		v1.y = v1.y - kMapCrossSize;
		v2.x = v1.x;
		v2.y = v1.y + 2 * kMapCrossSize + 1;
		QADrawLine(state->drawContext, &v1, &v2);
	}
}
#endif
#endif

#if __RENDER_ENCLOSURE__
#define			kEnclosureNormalLength		4.0
#define			kEnclosureHeight			1.0

inline void Fill_TQAVGouraud_Yellow(VectorPtr source, TQAVGouraud* dest)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
	dest->r = 1.0;
	dest->g = 1.0;
	dest->b = 0.0;
	dest->a = 1.0;
}

inline void Fill_TQAVGouraud_Blue(VectorPtr source, TQAVGouraud* dest)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
	dest->r = 0.0;
	dest->g = 0.0;
	dest->b = 1.0;
	dest->a = 1.0;
}

static void Draw_Enclosure(StatePtr state, CollisionEnclosurePtr enclosure, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	long				i;
	CollisionWallPtr	wall = enclosure->wallList;
	Vector				left,
						right;
	TQAVGouraud			verts[2];
	Vector				xDir,
						yDir = {0.0,1.0,0.0},
						zDir;
	Boolean				contact = false;
	
	{
		float					x,
								z;
		MatrixPtr				matrix = &shipList[0].megaObject.object.pos;
		
		x = matrix->x.x * shipList[0].megaObject.object.sphereCenter.x + matrix->y.x * shipList[0].megaObject.object.sphereCenter.y + matrix->z.x * shipList[0].megaObject.object.sphereCenter.z + matrix->w.x;
		z = matrix->x.z * shipList[0].megaObject.object.sphereCenter.x + matrix->y.z * shipList[0].megaObject.object.sphereCenter.y + matrix->z.z * shipList[0].megaObject.object.sphereCenter.z + matrix->w.z;
		
		if((x > enclosure->bounding_min_x - shipList[0].megaObject.object.sphereRadius) && (x < enclosure->bounding_max_x + shipList[0].megaObject.object.sphereRadius)
			&& (z > enclosure->bounding_min_z - shipList[0].megaObject.object.sphereRadius) && (z < enclosure->bounding_max_z + shipList[0].megaObject.object.sphereRadius))
		contact = true;
	}
	
	for(i = 0; i < enclosure->wallCount; ++i, ++wall) {
		//Draw segment
		wall->left.y = kEnclosureHeight;
		Matrix_TransformVector(negatedCamera, &wall->left, &left);
		wall->right.y = kEnclosureHeight;
		Matrix_TransformVector(negatedCamera, &wall->right, &right);
		if(contact) {
			Fill_TQAVGouraud_Yellow(&left, &verts[0]);
			Fill_TQAVGouraud_Yellow(&right, &verts[1]);
		}
		else {
			Fill_TQAVGouraud_Blue(&left, &verts[0]);
			Fill_TQAVGouraud_Blue(&right, &verts[1]);
		}
		Gouraud_DrawLine(state, &verts[0], &verts[1]);
		
		//Compute normal
		left.x = (wall->left.x + wall->right.x) / 2.0;
		left.y = kEnclosureHeight;
		left.z = (wall->left.z + wall->right.z) / 2.0;
		Vector_Subtract(&wall->right, &wall->left, &xDir);
		Vector_CrossProduct(&yDir, &xDir, &zDir);
		Vector_Normalize(&zDir, &zDir);
		right.x = left.x + zDir.x * kEnclosureNormalLength;
		right.y = left.y + zDir.y * kEnclosureNormalLength;
		right.z = left.z + zDir.z * kEnclosureNormalLength;
		
		//Draw normal
		Matrix_TransformVector(negatedCamera, &left, &left);
		Matrix_TransformVector(negatedCamera, &right, &right);
		if(contact) {
			Fill_TQAVGouraud_Yellow(&left, &verts[0]);
			Fill_TQAVGouraud_Yellow(&right, &verts[1]);
		}
		else {
			Fill_TQAVGouraud_Blue(&left, &verts[0]);
			Fill_TQAVGouraud_Blue(&right, &verts[1]);
		}
		Gouraud_DrawLine(state, &verts[0], &verts[1]);
	}
}
#endif

void Draw_Frame(StatePtr localState)
{
	Matrix				skyPos,
						negatedCamera;
	long				i;
	TerrainMeshPtr		currentMesh;
	
	//Update camera
	Matrix_Negate(&worldCamera.camera, &negatedCamera);
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:QARenderStart");
#endif
	QARenderStart(localState->drawContext, NULL, NULL);
#if __PROFILE__
		LOG_EXIT; }
#endif
	
	//Disable fog
	if((theTerrain.fogMode != kFog_None) && (theTerrain.flags & kTerrainFlag_NoFogOnSky) && videoFX)
	QASetInt(localState->drawContext, kQATag_FogMode, kQAFogMode_None);
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:Sky");
#endif
	//Update sky position & render sky
	Matrix_SetRotateY(theTerrain.skyRotation, &skyPos);
	skyPos.w.x = worldCamera.camera.w.x;
	skyPos.w.y = theTerrain.skyHeight;
	skyPos.w.z = worldCamera.camera.w.z;
	Terrain_RenderSky(skyShape, localState, &skyPos, &negatedCamera);
#if __PROFILE__
		LOG_EXIT; }
#endif
	
	//Enable fog
	if((theTerrain.fogMode != kFog_None) && (theTerrain.flags & kTerrainFlag_NoFogOnSky) && videoFX) {
		if(theTerrain.fogMode == kFog_Linear)
		QASetInt(localState->drawContext, kQATag_FogMode, kQAFogMode_Linear);
		else
		QASetInt(localState->drawContext, kQATag_FogMode, kQAFogMode_Exponential);
	}
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:Ground");
#endif
	for(i = 0; i < theTerrain.meshCount; ++i) {
		//Test visibility
		theTerrain.meshList[i]->clipping = Box_Visible(&negatedCamera, theTerrain.meshList[i]->boundingBox, localState);
		if(theTerrain.meshList[i]->clipping == kNotVisible)
		continue;
		
		//Draw mesh
		Terrain_RenderMesh_Textured(theTerrain.meshList[i], localState, &negatedCamera, &worldCamera.camera.w);
	}
#if __PROFILE__
		LOG_EXIT; }
#endif
		
	//Flush! - in case of transparent triangles in the terrain mesh
	QAFlush(localState->drawContext);
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:Sea");
#endif
	for(i = 0; i < theTerrain.seaCount; ++i) {
		//Test visibility
		theTerrain.seaList[i]->clipping = Box_Visible(&negatedCamera, theTerrain.seaList[i]->boundingBox, localState);
		if(theTerrain.seaList[i]->clipping == kNotVisible)
		continue;
		
		//Update sea mesh geometry
		{
			float			time = (float) localState->frameTime / theTerrain.timeScale,
							amplitude = theTerrain.waveAmplitude,
							scaleX = theTerrain.waveScaleX,
							scaleZ = theTerrain.waveScaleZ;
			unsigned long	j;
			char			*vx, *vy, *vz;
			
			vx = (char*) &theTerrain.seaList[i]->pointList[0].point.x;
			vy = (char*) &theTerrain.seaList[i]->pointList[0].point.y;
			vz = (char*) &theTerrain.seaList[i]->pointList[0].point.z;
			
			for(j = 0; j < theTerrain.seaList[i]->pointCount; ++j, vx += sizeof(Vertex), vy += sizeof(Vertex), vz += sizeof(Vertex))
			*((float*) vy) = WaveHeight(time, *((float*) vx), *((float*) vz), scaleX, scaleZ, amplitude);
		}
		
		//Draw mesh
		Terrain_RenderMesh_Textured((TerrainMeshPtr) theTerrain.seaList[i], localState, &negatedCamera, &worldCamera.camera.w);
	}
#if __PROFILE__
		LOG_EXIT; }
#endif
	
#if __RENDER_TRACK__
	Draw_Track(localState, theRace, &negatedCamera);
#endif

#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:Sea Models");
#endif
	//Draw sea mesh models
	for(i = 0; i < theTerrain.seaCount; ++i)
	if((theTerrain.seaList[i]->clipping >= 0) && (theTerrain.seaList[i]->referenceCount)) {
		SeaMesh_AnimateModels(theTerrain.seaList[i]);
		SeaMesh_RenderModels(theTerrain.seaList[i], localState, &negatedCamera, &worldCamera.camera.w);
	}
#if __PROFILE__
		LOG_EXIT; }
#endif
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:Ships");
#endif
	if(shipCount)
	Ships_Render(localState, &negatedCamera, &worldCamera.camera.w);
#if __PROFILE__
		LOG_EXIT; }
#endif
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:Ground Models");
#endif
	//Draw mesh models
	for(i = 0; i < theTerrain.meshCount; ++i)
	if((theTerrain.meshList[i]->clipping >= 0) && (theTerrain.meshList[i]->referenceCount))
	Mesh_RenderModels(theTerrain.meshList[i], localState, &negatedCamera, &worldCamera.camera.w);
#if __PROFILE__
		LOG_EXIT; }
#endif
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:Animation & Bonus");
#endif
	Anim_Render(&theTerrain, localState, &negatedCamera, &worldCamera.camera.w);

	if(theTerrain.itemReferenceCount)
	Render_SpecialItems(&theTerrain, localState, &negatedCamera, &worldCamera.camera.w);
#if __PROFILE__
		LOG_EXIT; }
#endif
	
#if __RENDER_CHECK_POINTS__
	if((gameConfig.gameMode != kGameMode_Demo) && (gameConfig.gameMode != kGameMode_Replay))
	CheckPoints_Render(theRace, localState, &negatedCamera, &worldCamera.camera.w);
#endif

#if __RENDER_ENCLOSURE__
	for(i = 0; i < theTerrain.enclosureCount; ++i)
	Draw_Enclosure(localState, theTerrain.enclosureList[i], &negatedCamera, &worldCamera.camera.w);
#endif

#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:FX");
#endif
	//Draw FX
	QAFlush(localState->drawContext);
	if(videoFX) {
		//Disable fog
		if(theTerrain.fogMode != kFog_None)
		QASetInt(localState->drawContext, kQATag_FogMode, kQAFogMode_None);
	
		//Draw wake effect
		for(i = 0; i < shipCount; ++i)
		Draw_ShipWake(localState, &negatedCamera, &shipList[i]);

		if(theTerrain.skyFlags & (kSkyFlag_RenderRain + kSkyFlag_RenderSnow + kSkyFlag_RenderMoon + kSkyFlag_RenderLF)) {
			currentMesh = Mesh_GetCurrent(&theTerrain, &worldCamera.camera.w);
			if((currentMesh == nil) || ((currentMesh != nil) && !(currentMesh->flags & kFlag_NoPostFX))) {
				//Render FX
				if(theTerrain.skyFlags & kSkyFlag_RenderRain)
				PostFX_Rain_Display(localState, &worldCamera.camera, &negatedCamera);
				if(theTerrain.skyFlags & kSkyFlag_RenderSnow)
				PostFX_Snow_Display(localState, &worldCamera.camera, &negatedCamera);
				if(theTerrain.skyFlags & kSkyFlag_RenderMoon)
				PostFX_Moon_Display(localState, theTerrain.skyRadius, &negatedCamera, &worldCamera.camera.w);
				if(theTerrain.skyFlags & kSkyFlag_RenderLF)
				PostFX_LensFlare_Display(localState, theTerrain.skyRadius, &negatedCamera, &worldCamera.camera.w);
			}	
		}
		
		//Enable fog
		if(theTerrain.fogMode == kFog_Linear)
		QASetInt(localState->drawContext, kQATag_FogMode, kQAFogMode_Linear);
		else if(theTerrain.fogMode == kFog_Exponential)
		QASetInt(localState->drawContext, kQATag_FogMode, kQAFogMode_Exponential);
		else if(theTerrain.fogMode == kFog_ExponentialSquared)
		QASetInt(localState->drawContext, kQATag_FogMode, kQAFogMode_ExponentialSquared);
	}
#if __PROFILE__
		LOG_EXIT; }
#endif
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:HUD");
#endif
	//Draw display
	QAFlush(localState->drawContext);
	QASetInt(localState->drawContext, kQATag_ZFunction, kQAZFunction_None);
	if(((cameraMode == kCameraMode_Follow) || (cameraMode == kCameraMode_Head) || (cameraMode == kCameraMode_OnBoard)) && (gameConfig.gameMode != kGameMode_Replay) && !cameraTargetShip->player->finished) {
		Draw_Arrow(theRace, cameraTargetShip->nextCheckPoint, localState, &negatedCamera, &worldCamera.camera.w);
#if __RENDER_MAP__
		if(!(thePrefs.engineFlags & kPref_EngineFlagLowResolution))
		Draw_Map(theRace, localState, &negatedCamera);
#endif
	}
	if(cameraTextDriver != nil)
	cameraTextDriver(localState);
	if(localState->frameTime < gameConfig.startTime) {
		Vector				position;
		unsigned char		text[kString_Driver_Ready_L];
		long				digit = (gameConfig.startTime - localState->frameTime) / kTimeUnit + 1;
		
		BlockMove(kString_Driver_Ready, text, kString_Driver_Ready_L - 1);
		text[kString_Driver_Ready_L - 1] = Digit2Char(digit);
		position.x = thePrefs.renderWidth / 2;
		position.y = thePrefs.renderHeight / 2;
		PostFX_Text_Display(localState, kString_Driver_Ready_L, text, &position, kTextMode_Centered, kColor_Ready_r, kColor_Ready_g, kColor_Ready_b);
		
		if(digit != lastCountDownDigit) {
			Sound_PlaySFxHandle(gameFXSounds[kGameFX_CountDown]);
			lastCountDownDigit = digit;
		}
		
		if(gameConfig.startTime - localState->frameTime < 10) {
			if(!startFXPlayed) {
				Sound_PlaySFxHandle(gameFXSounds[kGameFX_GameStart]);
				startFXPlayed = true;
			}
		}
		else
		startFXPlayed = false;
	}
	if((gameConfig.gameMode == kGameMode_Local) && (gameConfig.gameSubMode == kGameSubMode_Local_TimeRace) && ((gameConfig.expiredTime - localState->frameTime) <= 5 * kTimeUnit)) {
		Vector				position;
		unsigned char		text[kString_Driver_Left_L];
		long				digit = (gameConfig.expiredTime - localState->frameTime) / kTimeUnit + 1;
		
		BlockMove(kString_Driver_Left, text, kString_Driver_Left_L - 1);
		text[kString_Driver_Left_L - 1] = Digit2Char(digit);
		position.x = thePrefs.renderWidth / 2;
		position.y = thePrefs.renderHeight / 2;
		PostFX_Text_Display(localState, kString_Driver_Left_L, text, &position, kTextMode_Centered, kColor_Ready_r, kColor_Ready_g, kColor_Ready_b);
		
		if(digit != lastCountDownDigit) {
			Sound_PlaySFxHandle(gameFXSounds[kGameFX_CountDown]);
			lastCountDownDigit = digit;
		}
	}
	QASetInt(localState->drawContext, kQATag_ZFunction, kQAZFunction_LT);
#if __PROFILE__
		LOG_EXIT; }
#endif
	
#if __PROFILE__
		{ TRACE_SETUP; LOG_ENTRY("Render:QARenderEnd");
#endif
	QARenderEnd(localState->drawContext, NULL);
#if __PROFILE__
		LOG_EXIT; }
#endif
}

static void HiliteScreen(StatePtr state, float brightness)
{
	long				i;
	TQAVGouraud			verts[4];
	long				saveBlend;
						
	//Disable fog & modify blend
	if(theTerrain.fogMode != kFog_None)
	QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_None);
	saveBlend = QAGetInt(state->drawContext, kQATag_Blend);
	QASetInt(state->drawContext, kQATag_Blend, kQABlend_Interpolate);
		
	//We suppose screen H > screen V - Draw a white rectangle
	for(i = 0; i < 4; ++i) {
		verts[i].r = verts[i].g = verts[i].b = 1.0;
		verts[i].a = brightness;
		verts[i].z = 0.00001;
		verts[i].invW = 1.0;
	}
	verts[0].x = boundrySlop; verts[0].y = boundrySlop;
	verts[1].x = state->viewWidth - boundrySlop; verts[1].y = boundrySlop;
	verts[2].x = state->viewWidth - boundrySlop; verts[2].y = state->viewHeight - boundrySlop;
	verts[3].x = boundrySlop; verts[3].y = state->viewHeight - boundrySlop;
	QADrawTriGouraud(state->drawContext, &verts[0], &verts[1], &verts[2], 0);
	QADrawTriGouraud(state->drawContext, &verts[2], &verts[3], &verts[0], 0);
	
	//Enable fog & restore blend
	if(theTerrain.fogMode == kFog_Linear)
	QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_Linear);
	else if(theTerrain.fogMode == kFog_Exponential)
	QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_Exponential);
	else if(theTerrain.fogMode == kFog_ExponentialSquared)
	QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_ExponentialSquared);
	QASetInt(state->drawContext, kQATag_Blend, saveBlend);
}

void GameOverText(StatePtr state)
{
	Vector			position;
	float			lineHeight;
	Str31			text;
	Boolean			displayPoints = false;
	
	QAFlush(state->drawContext);
	HiliteScreen(state, 0.8);
	QAFlush(state->drawContext);
	
	//Do we need to display points
	if((gameConfig.gameMode_save == kGameMode_Local) && PlayMode_PointsAvailable(gameConfig.gameSubMode_save))
	displayPoints = true;
	
	//Display instructions
	position.y = 0.0;
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	lineHeight = position.y;
	
	position.x = thePrefs.renderWidth - 10.0;
	position.y = thePrefs.renderHeight - 2 * lineHeight;
	PostFX_Text_Display(state, kString_WhiteScreen_Escape_1_L, (unsigned char*) kString_WhiteScreen_Escape_1, &position, kTextMode_Right, kColor_Instruction_r, kColor_Instruction_g, kColor_Instruction_b);
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	position.x = thePrefs.renderWidth - 10.0;
	PostFX_Text_Display(state, kString_WhiteScreen_Escape_2_L, (unsigned char*) kString_WhiteScreen_Escape_2, &position, kTextMode_Right, kColor_Instruction_r, kColor_Instruction_g, kColor_Instruction_b);
	
#if __RECORD_RACE__
	position.x = 10.0;
	position.y = thePrefs.renderHeight - 2 * lineHeight;
	PostFX_Text_Display(state, kString_WhiteScreen_Replay_1_L, (unsigned char*) kString_WhiteScreen_Replay_1, &position, kTextMode_Left, kColor_Instruction_r, kColor_Instruction_g, kColor_Instruction_b);
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	position.x = 10.0;
	PostFX_Text_Display(state, kString_WhiteScreen_Replay_2_L, (unsigned char*) kString_WhiteScreen_Replay_2, &position, kTextMode_Left, kColor_Instruction_r, kColor_Instruction_g, kColor_Instruction_b);
#endif

	if(thePrefs.engineFlags & kPref_EngineFlagLowResolution)
	return;
	
	if(!localShip) { //We are replaying a race or blind hosting
		//Display total time
		position.y = 10.0;
		position.x = 10.0;
		PostFX_Text_Display(state, kString_WhiteScreen_RaceTime_L, (unsigned char*) kString_WhiteScreen_RaceTime, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
		position.x = 10.0;
		PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
		RaceTimeToString(gameConfig.raceLength, text); //localShip->player->finishTime
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
		
		return;
	}
	
	//Display title
	position.y = 10.0;
	position.x = thePrefs.renderWidth / 2;
	PostFX_Text_Display(state, kString_WhiteScreen_Results_L, (unsigned char*) kString_WhiteScreen_Results, &position, kTextMode_Centered, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
	
	//Skip 2 lines
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	//Display rank
	if(!((gameConfig.gameMode_save == kGameMode_Local) && (gameConfig.gameSubMode_save == kGameSubMode_Local_TimeRace))) {
		position.x = 10.0;
		PostFX_Text_Display(state, kString_WhiteScreen_Rank_L, (unsigned char*) kString_WhiteScreen_Rank, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
		position.x = 10.0;
		PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
		text[0] = 3;
		FastNumToString1(localShip->player->rank, &text[1]);
		text[2] = '/';
		FastNumToString1(shipCount, &text[3]);
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
		if(displayPoints && (localShip->player->rank <= 2)) {
			position.x = 10.0;
			PostFX_Text_Display(state, kTab2, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
			text[0] = 0;
			text[++text[0]] = ' ';
			text[++text[0]] = '(';
			text[++text[0]] = '+';
			if(localShip->player->rank == 1)
			FastNumToString3(400, &text[text[0] + 1]);
			else if(localShip->player->rank == 2)
			FastNumToString3(300, &text[text[0] + 1]);
			text[0] += 3;
			text[++text[0]] = ' ';
			text[++text[0]] = 'P';
			text[++text[0]] = 'T';
			text[++text[0]] = 'S';
			text[++text[0]] = ')';
			PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Points_r, kColor_Points_g, kColor_Points_b);
		}
		PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	}
	
	//Display bonus points
	if(displayPoints) {
		position.x = 10.0;
		PostFX_Text_Display(state, kString_WhiteScreen_Bonus_L, (unsigned char*) kString_WhiteScreen_Bonus, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
		position.x = 10.0;
		PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
		text[0] = FastNumToString4(localShip->player->score, &text[1]);
		text[++text[0]] = ' ';
		text[++text[0]] = 'P';
		text[++text[0]] = 'T';
		text[++text[0]] = 'S';
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
	}
	
	//Display player best lap time
	if(localShip->player->bestLapTime < kMaxRaceTime) {
		PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
		position.x = 10.0;
		PostFX_Text_Display(state, kString_WhiteScreen_BestLap_L, (unsigned char*) kString_WhiteScreen_BestLap, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
		position.x = 10.0;
		PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
		RaceTimeToString(localShip->player->bestLapTime, text);
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
		if(displayPoints && (localShip->player->bestLapTime <= gameConfig.globalBestLapTime)) {
			position.x = 10.0;
			PostFX_Text_Display(state, kTab2, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
			text[0] = 0;
			text[++text[0]] = ' ';
			text[++text[0]] = '(';
			text[++text[0]] = '+';
			FastNumToString3(100, &text[text[0] + 1]);
			text[0] += 3;
			text[++text[0]] = ' ';
			text[++text[0]] = 'P';
			text[++text[0]] = 'T';
			text[++text[0]] = 'S';
			text[++text[0]] = ')';
			PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Points_r, kColor_Points_g, kColor_Points_b);
		}
	}
	
	//Display time left
	if((gameConfig.gameMode_save == kGameMode_Local) && (gameConfig.gameSubMode_save == kGameSubMode_Local_TimeRace)) {
		PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
		position.x = 10.0;
		PostFX_Text_Display(state, kString_WhiteScreen_TimeLeft_L, (unsigned char*) kString_WhiteScreen_TimeLeft, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
		position.x = 10.0;
		PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
		RaceTimeToString(gameConfig.expiredTime, text);
		PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
	}
	
	//Skip 2 lines
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	
	//Display total time
	position.x = 10.0;
	PostFX_Text_Display(state, kString_WhiteScreen_RaceTime_L, (unsigned char*) kString_WhiteScreen_RaceTime, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
	position.x = 10.0;
	PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
	RaceTimeToString(gameConfig.raceLength, text); //localShip->player->finishTime
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
	
	//Display max speed
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	position.x = 10.0;
	PostFX_Text_Display(state, kString_WhiteScreen_MaxSpeed_L, (unsigned char*) kString_WhiteScreen_MaxSpeed, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
	position.x = 10.0;
	PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
	text[0] = FastNumToString4(UnitToSpeed(localShip->player->maxSpeed), &text[1]);
	text[++text[0]] = ' ';
#if __UNIT_SYSTEM__ == kUnit_Meters
	text[++text[0]] = 'k';
	text[++text[0]] = 'm';
	text[++text[0]] = '/';
	text[++text[0]] = 'h';
#elif __UNIT_SYSTEM__ == kUnit_Miles
	text[++text[0]] = 'm';
	text[++text[0]] = 'p';
	text[++text[0]] = 'h';
#endif
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
	
	//Display total distance
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	position.x = 10.0;
	PostFX_Text_Display(state, kString_WhiteScreen_Distance_L, (unsigned char*) kString_WhiteScreen_Distance, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
	position.x = 10.0;
	PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
#if __UNIT_SYSTEM__ == kUnit_Meters
	FloatToString(UnitToDistance((float) localShip->player->totalDistance), text);
#elif __UNIT_SYSTEM__ == kUnit_Miles
	FloatToString(UnitToDistance((float) localShip->player->totalDistance), text);
#endif
#if __UNIT_SYSTEM__ == kUnit_Meters
	text[++text[0]] = ' ';
	text[++text[0]] = 'k';
	text[++text[0]] = 'm';
#elif __UNIT_SYSTEM__ == kUnit_Miles
	text[++text[0]] = ' ';
	text[++text[0]] = 'm';
#endif
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
	
	//Display average speed
	PostFX_Text_Display(state, 0, nil, &position, kTextMode_Fake_NewLine, 0, 0, 0);
	position.x = 10.0;
	PostFX_Text_Display(state, kString_WhiteScreen_AvSpeed_L, (unsigned char*) kString_WhiteScreen_AvSpeed, &position, kTextMode_Left, kColor_Legend_r, kColor_Legend_g, kColor_Legend_b);
	position.x = 10.0;
	PostFX_Text_Display(state, kTab, nil, &position, kTextMode_Space, 0.0, 0.0, 0.0);
	text[0] = FastNumToString4(UnitToSpeed(kTimeUnit * localShip->player->totalDistance / (float) localShip->player->finishTime), &text[1]);
	text[++text[0]] = ' ';
#if __UNIT_SYSTEM__ == kUnit_Meters
	text[++text[0]] = 'k';
	text[++text[0]] = 'm';
	text[++text[0]] = '/';
	text[++text[0]] = 'h';
#elif __UNIT_SYSTEM__ == kUnit_Miles
	text[++text[0]] = 'm';
	text[++text[0]] = 'p';
	text[++text[0]] = 'h';
#endif
	PostFX_Text_Display(state, text[0], &text[1], &position, kTextMode_Left, kColor_Info_r, kColor_Info_g, kColor_Info_b);
}
