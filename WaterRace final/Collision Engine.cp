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
#include				"Drivers.h"

#include				"Vector.h"
#include				"Infinity Audio Engine.h"

#include				"Collision Engine.h"

#include				"Terrain Utils.h"

//MACROS:

#define Square_Norm3(a, b, c) ((a) * (a) + (b) * (b) + (c) * (c))
#define Square_Norm2(a, b) ((a) * (a) + (b) * (b))
#define	AbsDotProduct(v1, v2) fAbs((v1)->x * (v2)->x + (v1)->y * (v2)->y + (v1)->z * (v2)->z)

//CONSTANTES:

#define	kNumBBoxTris		12
static TriFace				triangleList[kNumBBoxTris] = {{0,1,2},{2,3,0},{4,5,6},{6,7,4},
														{0,1,5},{5,4,0},{1,2,6},{6,5,1},
														{2,3,7},{7,6,2},{3,0,4},{4,7,3}};
//ROUTINES:

void Check_BonusCollision_Local(ExtendedShipPtr ship, MatrixPtr inverseShipMatrix)
{
	long					i;
	ObjectPtr				bonusObject,
							shipObject = &ship->megaObject.object;
	SpecialItemReferencePtr	item = theTerrain.itemReferenceList;
	Vector					bonusPos;
	SpecialItemPtr			parent;
	
	for(i = 0; i < theTerrain.itemReferenceCount; ++i, ++item) {
		//Extract bonus parent
		parent = (SpecialItemPtr) item->itemID;
		
		//Extract bonus model
		bonusObject = (ObjectPtr) parent->modelID;
		
		//Compare distance between ship & bonus - not correct: we should consider real-sphere centers
		if(Square_Norm3(shipObject->pos.w.x - item->position.x, shipObject->pos.w.y - item->position.y, shipObject->pos.w.z - item->position.z) > Square(shipObject->sphereRadius + bonusObject->sphereRadius))
		continue;
		
		//Transform bonus into ship world
		Matrix_TransformVector(inverseShipMatrix, &item->position, &bonusPos);
		
		//Compare bonus bounding sphere to ship bounding box
		if((bonusPos.x /*+ bonusObject->sphereRadius*/ > shipObject->boundingBox[0].x) && (bonusPos.x /*- bonusObject->sphereRadius*/ < shipObject->boundingBox[1].x)
			&& (bonusPos.y + bonusObject->sphereRadius > shipObject->boundingBox[0].y) && (bonusPos.y - bonusObject->sphereRadius < shipObject->boundingBox[4].y)
			&& (bonusPos.z /*+ bonusObject->sphereRadius*/ > shipObject->boundingBox[0].z) && (bonusPos.z /*- bonusObject->sphereRadius*/ < shipObject->boundingBox[2].z)) {
			
			//Add bonus score
			switch(parent->itemType) {
				
				case kItem_Bonus:
				ship->player->score += parent->itemValue;
				if(cameraTargetShip == localShip)
				InfinityAudio_InstallSound_FX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Bonus], kFXVolume, kInfinityAudio_CenteredPanoramic, kInfinityAudio_NormalPitch, nil);
				break;
				
				case kItem_Turbo:
				ship->player->turboCount += 1;
				if(cameraTargetShip == localShip)
				InfinityAudio_InstallSound_FX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Turbo], kFXVolume, kInfinityAudio_CenteredPanoramic, kInfinityAudio_NormalPitch, nil);
				break;
				
				case kItem_Joker:
				ship->player->hasJoker = true;
				if(cameraTargetShip == localShip)
				InfinityAudio_InstallSound_FX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Joker], kFXVolume, kInfinityAudio_CenteredPanoramic, kInfinityAudio_NormalPitch, nil);
				break;
				
			}
			
			//Remove bonus from terrain
			theTerrain.itemReferenceList[i] = theTerrain.itemReferenceList[theTerrain.itemReferenceCount - 1];
			
			--theTerrain.itemReferenceCount;
			if(theTerrain.itemReferenceCount == 0) {
				ItemList_Dispose(theTerrain.itemReferenceList);
				return;
			}
			--i;
			--item;
		}
	}
}

#if __BOTS_CAN_PICKUP_TURBOS__
void Check_BonusCollision_Bot(ExtendedShipPtr ship, MatrixPtr inverseShipMatrix)
{
	long					i;
	ObjectPtr				bonusObject,
							shipObject = &ship->megaObject.object;
	SpecialItemReferencePtr	item = theTerrain.itemReferenceList;
	Vector					bonusPos;
	SpecialItemPtr			parent;
	
	for(i = 0; i < theTerrain.itemReferenceCount; ++i, ++item) {
		//Extract bonus parent
		parent = (SpecialItemPtr) item->itemID;
		
		//Bot are only allowed to pick up turbos
		if(parent->itemType != kItem_Turbo)
		continue;
		
		//Extract bonus model
		bonusObject = (ObjectPtr) parent->modelID;
		
		//Compare distance between ship & bonus - not correct: we should consider real-sphere centers
		if(Square_Norm3(shipObject->pos.w.x - item->position.x, shipObject->pos.w.y - item->position.y, shipObject->pos.w.z - item->position.z) > Square(shipObject->sphereRadius + bonusObject->sphereRadius))
		continue;
		
		//Transform bonus into ship world
		Matrix_TransformVector(inverseShipMatrix, &item->position, &bonusPos);
		
		//Compare bonus bounding sphere to ship bounding box
		if((bonusPos.x /*+ bonusObject->sphereRadius*/ > shipObject->boundingBox[0].x) && (bonusPos.x /*- bonusObject->sphereRadius*/ < shipObject->boundingBox[1].x)
			&& (bonusPos.y + bonusObject->sphereRadius > shipObject->boundingBox[0].y) && (bonusPos.y - bonusObject->sphereRadius < shipObject->boundingBox[4].y)
			&& (bonusPos.z /*+ bonusObject->sphereRadius*/ > shipObject->boundingBox[0].z) && (bonusPos.z /*- bonusObject->sphereRadius*/ < shipObject->boundingBox[2].z)) {
			
			if(cameraTargetShip == ship)
			InfinityAudio_InstallSound_FX(kCreatorType, (ExtSoundHeaderPtr) *gameFXSounds[kGameFX_Turbo], kFXVolume, kInfinityAudio_CenteredPanoramic, kInfinityAudio_NormalPitch, nil);
			
			//Add turbo
			ship->player->turboCount += 1;
			
			//Remove bonus from terrain
			theTerrain.itemReferenceList[i] = theTerrain.itemReferenceList[theTerrain.itemReferenceCount - 1];
			
			--theTerrain.itemReferenceCount;
			if(theTerrain.itemReferenceCount == 0) {
				ItemList_Dispose(theTerrain.itemReferenceList);
				return;
			}
			--i;
			--item;
		}
	}
}
#endif

inline Boolean Box_Intersect(VectorPtr axis, VectorPtr sourceX, VectorPtr sourceY, VectorPtr sourceZ, VectorPtr destX, VectorPtr destY, VectorPtr destZ, VectorPtr direction)
{
	float				sourceR = 0.0,
						destR = 0.0;
						
	//Project source edges on the axis
	sourceR += AbsDotProduct(sourceX, axis);
	sourceR += AbsDotProduct(sourceY, axis);
	sourceR += AbsDotProduct(sourceZ, axis);
	sourceR /= 2.0;
	
	//Project dest edges on the axis
	destR += AbsDotProduct(destX, axis);
	destR += AbsDotProduct(destY, axis);
	destR += AbsDotProduct(destZ, axis);
	destR /= 2.0;
	
	//Check overlapping
	return (AbsDotProduct(direction, axis) < sourceR + destR);
}

inline void Subtract(VectorPtr v1, VectorPtr v2, VectorPtr v3)
{
	v3->x = v1->x - v2->x;
	v3->y = v1->y - v2->y;
	v3->z = v1->z - v2->z;
}

static Boolean BBox_Collision(MatrixPtr sourceMatrix, Vector source[kBBSize], MatrixPtr destMatrix, Vector dest[kBBSize])
{
	Vector				axis;
	Vector				sourceCenter, sourceX, sourceY, sourceZ;
	Vector				destCenter, destX, destY, destZ;
	Vector				direction;
	
	//Compute bounding boxes edges
	Subtract(&source[1], &source[0], &sourceX);
	Subtract(&source[4], &source[0], &sourceY);
	Subtract(&source[3], &source[0], &sourceZ);
	
	Subtract(&dest[1], &dest[0], &destX);
	Subtract(&dest[4], &dest[0], &destY);
	Subtract(&dest[3], &dest[0], &destZ);
	
	//Compute bounding boxes center
	sourceCenter.x = source[0].x + sourceX.x / 2.0 + sourceY.x / 2.0 + sourceZ.x / 2.0;
	sourceCenter.y = source[0].y + sourceX.y / 2.0 + sourceY.y / 2.0 + sourceZ.y / 2.0;
	sourceCenter.z = source[0].z + sourceX.z / 2.0 + sourceY.z / 2.0 + sourceZ.z / 2.0;
				
	destCenter.x = dest[0].x + destX.x / 2.0 + destY.x / 2.0 + destZ.x / 2.0;
	destCenter.y = dest[0].y + destX.y / 2.0 + destY.y / 2.0 + destZ.y / 2.0;
	destCenter.z = dest[0].z + destX.z / 2.0 + destY.z / 2.0 + destZ.z / 2.0;
				
	//Compute bounding boxes distance
	Subtract(&destCenter, &sourceCenter, &direction);
	
	//Check 3 axis from source
	if(!Box_Intersect(&sourceMatrix->x, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	if(!Box_Intersect(&sourceMatrix->y, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	if(!Box_Intersect(&sourceMatrix->z, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	
	//Check 3 axis from dest
	if(!Box_Intersect(&destMatrix->x, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	if(!Box_Intersect(&destMatrix->y, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	if(!Box_Intersect(&destMatrix->z, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	
	//Check 9 axis from source & dest
	Vector_CrossProduct(&sourceMatrix->x, &destMatrix->x, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	Vector_CrossProduct(&sourceMatrix->x, &destMatrix->y, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	Vector_CrossProduct(&sourceMatrix->x, &destMatrix->z, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	
	Vector_CrossProduct(&sourceMatrix->y, &destMatrix->x, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	Vector_CrossProduct(&sourceMatrix->y, &destMatrix->y, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	Vector_CrossProduct(&sourceMatrix->y, &destMatrix->z, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	
	Vector_CrossProduct(&sourceMatrix->z, &destMatrix->x, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	Vector_CrossProduct(&sourceMatrix->z, &destMatrix->y, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	Vector_CrossProduct(&sourceMatrix->z, &destMatrix->z, &axis);
	if(!Box_Intersect(&axis, &sourceX, &sourceY, &sourceZ, &destX, &destY, &destZ, &direction))
	return false;
	
	return true;
}

Boolean Box_Collision(VectorPtr sourceC, VectorPtr sourceX, VectorPtr sourceY, VectorPtr sourceZ, VectorPtr destC, VectorPtr destX, VectorPtr destY, VectorPtr destZ)
{
	Vector				axis,
						dir;
	
	//Compute bounding boxes distance
	Subtract(destC, sourceC, &dir);
	
	//Check 3 axis from source
	if(!Box_Intersect(sourceX, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	if(!Box_Intersect(sourceY, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	if(!Box_Intersect(sourceZ, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	
	//Check 3 axis from dest
	if(!Box_Intersect(destX, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	if(!Box_Intersect(destY, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	if(!Box_Intersect(destZ, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	
	//Check 9 axis from source & dest
	Vector_CrossProduct(sourceX, destX, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	Vector_CrossProduct(sourceX, destY, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	Vector_CrossProduct(sourceX, destZ, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	
	Vector_CrossProduct(sourceY, destX, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	Vector_CrossProduct(sourceY, destY, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	Vector_CrossProduct(sourceY, destZ, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	
	Vector_CrossProduct(sourceZ, destX, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	Vector_CrossProduct(sourceZ, destY, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	Vector_CrossProduct(sourceZ, destZ, &axis);
	if(!Box_Intersect(&axis, sourceX, sourceY, sourceZ, destX, destY, destZ, &dir))
	return false;
	
	return true;
}

Boolean Object_Collision(MegaObjectPtr source, MatrixPtr sourceMatrix, float sourceScale, MegaObjectPtr dest, MatrixPtr destMatrix, float destScale)
{
	//Step 1: sphere intersection
	{
		Vector					sourceCenter,
								destCenter;
	
		Matrix_TransformVector(sourceMatrix, &source->object.sphereCenter, &sourceCenter);
		Matrix_TransformVector(destMatrix, &dest->object.sphereCenter, &destCenter);
		if(Square_Norm3(sourceCenter.x - destCenter.x, sourceCenter.y - destCenter.y, sourceCenter.z - destCenter.z) > Square(source->object.sphereRadius * sourceScale + dest->object.sphereRadius * destScale))
		return false;
	}
	
	//Step 2: BBox intersection
	{
		long					i,
								j;
		Vector					sourceC,
								sourceX,
								sourceY,
								sourceZ;
		Vector					destC,
								destX,
								destY,
								destZ;
		
		for(i = 0; i < source->boxCount; ++i) {
			//Transform box
			Matrix_TransformVector(sourceMatrix, &source->boxList[i].center, &sourceC);
			Matrix_RotateVector(sourceMatrix, &source->boxList[i].xVector, &sourceX);
			Matrix_RotateVector(sourceMatrix, &source->boxList[i].yVector, &sourceY);
			Matrix_RotateVector(sourceMatrix, &source->boxList[i].zVector, &sourceZ);
			
			for(j = 0; j < dest->boxCount; ++j) {
				//Transform box
				Matrix_TransformVector(destMatrix, &dest->boxList[j].center, &destC);
				Matrix_RotateVector(destMatrix, &dest->boxList[j].xVector, &destX);
				Matrix_RotateVector(destMatrix, &dest->boxList[j].yVector, &destY);
				Matrix_RotateVector(destMatrix, &dest->boxList[j].zVector, &destZ);
				
				//Check collision
				if(Box_Collision(&sourceC, &sourceX, &sourceY, &sourceZ, &destC, &destX, &destY, &destZ))
				return true;
			}
		}
	}
	
	return false;
}

Boolean Object_Collision_WithNormal(MegaObjectPtr source, MatrixPtr sourceMatrix, float sourceScale, MegaObjectPtr dest, MatrixPtr destMatrix, float destScale, VectorPtr normal)
{
	//Step 1: sphere intersection
	{
		Vector					sourceCenter,
								destCenter;
	
		Matrix_TransformVector(sourceMatrix, &source->object.sphereCenter, &sourceCenter);
		Matrix_TransformVector(destMatrix, &dest->object.sphereCenter, &destCenter);
		if(Square_Norm3(sourceCenter.x - destCenter.x, sourceCenter.y - destCenter.y, sourceCenter.z - destCenter.z) > Square(source->object.sphereRadius * sourceScale + dest->object.sphereRadius * destScale))
		return false;
	}
	
	//Step 2: BBox intersection
	{
		long					i,
								j;
		Vector					sourceC,
								sourceX,
								sourceY,
								sourceZ;
		Vector					destC,
								destX,
								destY,
								destZ;
		Boolean					colliding = false;
		
		Vector_Clear(normal);
		for(i = 0; i < source->boxCount; ++i) {
			//Transform box
			Matrix_TransformVector(sourceMatrix, &source->boxList[i].center, &sourceC);
			Matrix_RotateVector(sourceMatrix, &source->boxList[i].xVector, &sourceX);
			Matrix_RotateVector(sourceMatrix, &source->boxList[i].yVector, &sourceY);
			Matrix_RotateVector(sourceMatrix, &source->boxList[i].zVector, &sourceZ);
			
			for(j = 0; j < dest->boxCount; ++j) {
				//Transform box
				Matrix_TransformVector(destMatrix, &dest->boxList[j].center, &destC);
				Matrix_RotateVector(destMatrix, &dest->boxList[j].xVector, &destX);
				Matrix_RotateVector(destMatrix, &dest->boxList[j].yVector, &destY);
				Matrix_RotateVector(destMatrix, &dest->boxList[j].zVector, &destZ);
				
				//Check collision
				if(Box_Collision(&sourceC, &sourceX, &sourceY, &sourceZ, &destC, &destX, &destY, &destZ)) {
					Matrix			m,
									n;
					float			lx,
									lz;
					long			k;
					Vector			temp;
					
					//Compute dest box size
					lx = dest->boxList[j].halfSize_x * destScale * destScale;
					//ly = dest->boxList[j].halfSize_y * destScale * destScale;
					lz = dest->boxList[j].halfSize_z * destScale * destScale;
					
					//Compute dest box matrix in global space
					Matrix_Cat(&dest->boxList[j].matrix, destMatrix, &m);
					Matrix_Negate(&m, &n);
					
					for(k = 0; k < kBBSize; ++k) {
						//Transform source box corner into dest box space
						Matrix_TransformVector(sourceMatrix, &source->boxList[i].corners[k], &temp);
						Matrix_TransformVector(&n, &temp, &temp);
					
						//Check corner position and update collision normal
						if(temp.x < -lx) {
							normal->x += -m.x.x;
							normal->y += -m.x.y;
							normal->z += -m.x.z;
						}
						else if(temp.x > lx) {
							normal->x += m.x.x;
							normal->y += m.x.y;
							normal->z += m.x.z;
						}
						/*if(temp.y < -ly) {
							normal->x += -m.y.x;
							normal->y += -m.y.y;
							normal->z += -m.y.z;
						}
						else if(temp.y > ly) {
							normal->x += m.y.x;
							normal->y += m.y.y;
							normal->z += m.y.z;
						}*/
						if(temp.z < -lz) {
							normal->x += -m.z.x;
							normal->y += -m.z.y;
							normal->z += -m.z.z;
						}
						else if(temp.z > lz) {
							normal->x += m.z.x;
							normal->y += m.z.y;
							normal->z += m.z.z;
						}
					}
					
					colliding = true;
				}
			}
		}
		
		if(colliding) {
			Vector_Normalize(normal, normal);
			return true;
		}
	}
	
	return false;
}

inline Boolean WallSphere_Collision(float x, float y, float z, float radius, CollisionWallPtr wall)
{
	float			xx, zz;
	
	//Transform sphere
	xx = wall->inverseMatrix.x.x * x + wall->inverseMatrix.y.x * y + wall->inverseMatrix.z.x * z + wall->inverseMatrix.w.x;
	zz = wall->inverseMatrix.x.z * x + wall->inverseMatrix.y.z * y + wall->inverseMatrix.z.z * z + wall->inverseMatrix.w.z;
	
	//Check if sphere close to wall
	if(!((zz < radius) && (zz > -radius)
		&& (xx < radius + wall->width) && (xx > -radius)))
	return false;
	
	//Check if sphere close to left
	if((xx < 0.0) && (Square_Norm2(xx, zz) >= Square(radius)))
	return false;
		
	//Check if sphere close to right
	if((xx > wall->width) && (Square_Norm2(xx - wall->width, zz) >= Square(radius)))
	return false;
	
	return true;
}

Boolean ShipEnclosure_Collision(ExtendedShipPtr ship, CollisionEnclosurePtr enclosure)
{
	//Step 1: ship object sphere / enclosure intersection
	{
		float					x,
								z;
		MatrixPtr				matrix = &ship->megaObject.object.pos;
		
		x = matrix->x.x * ship->megaObject.object.sphereCenter.x + matrix->y.x * ship->megaObject.object.sphereCenter.y + matrix->z.x * ship->megaObject.object.sphereCenter.z + matrix->w.x;
		z = matrix->x.z * ship->megaObject.object.sphereCenter.x + matrix->y.z * ship->megaObject.object.sphereCenter.y + matrix->z.z * ship->megaObject.object.sphereCenter.z + matrix->w.z;
		
		if(!((x > enclosure->bounding_min_x - ship->megaObject.object.sphereRadius) && (x < enclosure->bounding_max_x + ship->megaObject.object.sphereRadius)
			&& (z > enclosure->bounding_min_z - ship->megaObject.object.sphereRadius) && (z < enclosure->bounding_max_z + ship->megaObject.object.sphereRadius)))
		return false;
	}
	
	//Step 2: ship special spheres / walls intersection
	{
		float					x1, y1, z1,
								x2, y2, z2;
		long					i;
		MatrixPtr				matrix = &ship->megaObject.object.pos;					
		
		//Transform sphere #1
		x1 = matrix->z.x * ship->center1 + matrix->w.x;
		y1 = matrix->z.y * ship->center1 + matrix->w.y;
		z1 = matrix->z.z * ship->center1 + matrix->w.z;
		
		//Transform sphere #2
		x2 = matrix->z.x * ship->center2 + matrix->w.x;
		y2 = matrix->z.y * ship->center2 + matrix->w.y;
		z2 = matrix->z.z * ship->center2 + matrix->w.z;
		
		//Test against all walls
		for(i = 0; i < enclosure->wallCount; ++i) {
			//Test sphere #1
			if(WallSphere_Collision(x1, y1, z1, ship->radius, &enclosure->wallList[i]))
			return true;
			
			//Test sphere #2
			if(WallSphere_Collision(x2, y2, z2, ship->radius, &enclosure->wallList[i]))
			return true;
		}
	}
	
	return false;
}

Boolean ShipEnclosure_Collision_WithNormal(ExtendedShipPtr ship, CollisionEnclosurePtr enclosure, VectorPtr normal)
{
	//Step 1: ship object sphere / enclosure intersection
	{
		float					x,
								z;
		MatrixPtr				matrix = &ship->megaObject.object.pos;
		
		x = matrix->x.x * ship->megaObject.object.sphereCenter.x + matrix->y.x * ship->megaObject.object.sphereCenter.y + matrix->z.x * ship->megaObject.object.sphereCenter.z + matrix->w.x;
		z = matrix->x.z * ship->megaObject.object.sphereCenter.x + matrix->y.z * ship->megaObject.object.sphereCenter.y + matrix->z.z * ship->megaObject.object.sphereCenter.z + matrix->w.z;
		
		if(!((x > enclosure->bounding_min_x - ship->megaObject.object.sphereRadius) && (x < enclosure->bounding_max_x + ship->megaObject.object.sphereRadius)
			&& (z > enclosure->bounding_min_z - ship->megaObject.object.sphereRadius) && (z < enclosure->bounding_max_z + ship->megaObject.object.sphereRadius)))
		return false;
	}
	
	//Step 2: ship special spheres / walls intersection
	{
		float					x1, y1, z1,
								x2, y2, z2;
		long					i;
		MatrixPtr				matrix = &ship->megaObject.object.pos;					
		Boolean					colliding = false;
		
		Vector_Clear(normal);
		
		//Transform sphere #1
		x1 = matrix->z.x * ship->center1 + matrix->w.x;
		y1 = matrix->z.y * ship->center1 + matrix->w.y;
		z1 = matrix->z.z * ship->center1 + matrix->w.z;
		
		//Transform sphere #2
		x2 = matrix->z.x * ship->center2 + matrix->w.x;
		y2 = matrix->z.y * ship->center2 + matrix->w.y;
		z2 = matrix->z.z * ship->center2 + matrix->w.z;
		
		//Test against all walls
		for(i = 0; i < enclosure->wallCount; ++i) {
			//Test sphere #1
			if(WallSphere_Collision(x1, y1, z1, ship->radius, &enclosure->wallList[i])) {
				normal->x += enclosure->wallList[i].inverseMatrix.x.z;
				normal->y += enclosure->wallList[i].inverseMatrix.y.z;
				normal->z += enclosure->wallList[i].inverseMatrix.z.z;
				colliding = true;
			}
			
			//Test sphere #2
			if(WallSphere_Collision(x2, y2, z2, ship->radius, &enclosure->wallList[i])) {
				normal->x += enclosure->wallList[i].inverseMatrix.x.z;
				normal->y += enclosure->wallList[i].inverseMatrix.y.z;
				normal->z += enclosure->wallList[i].inverseMatrix.z.z;
				colliding = true;
			}
		}
		
		if(colliding) {
			Vector_Normalize(normal, normal);
			return true;
		}
	}
	
	return false;
}

