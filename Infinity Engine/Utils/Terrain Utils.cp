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
#include					"Infinity Terrain.h"
#include					"Infinity Error Codes.h"

#include					"Infinity Rendering.h"
#include					"Vector.h"

#include					"Infinity Utils.h"
#include					"Clipping Utils.h"

//CONSTANTES:

#define					kTextureVScale				(terrain->skyScale)
#define					kTileFactor					(terrain->numTiles)

//MACROS:

#define F(a) (1 - (1.0 / kTextureVScale - a / kTextureVScale))
#define G(a) (1 - (1.0 / 2.0 - a / 2.0))

//ROUTINES:

void ModelReference_UpdateMatrix(ModelReferencePtr reference)
{
	Vector			p;
	Matrix			m;
	
	p = reference->pos.w;
	Matrix_Clear(&reference->pos);
	if(reference->scale != 0.0)
	Matrix_ScaleLocal(&reference->pos, reference->scale, &reference->pos);
	
	Matrix_SetRotateX(reference->rotateX, &m);
	Matrix_MultiplyByMatrix(&m, &reference->pos, &reference->pos);
	
	Matrix_SetRotateY(reference->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &reference->pos, &reference->pos);
	
	Matrix_SetRotateZ(reference->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &reference->pos, &reference->pos);
	
	reference->pos.w = p;
}
	
void Anim_UpdateMatrix(ModelAnimationPtr animation)
{
	Vector			p;
	Matrix			m;
	
	p = animation->pos.w;
	
	Matrix_SetRotateX(animation->rotateX, &animation->pos);
	
	Matrix_SetRotateY(animation->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &animation->pos, &animation->pos);
	
	Matrix_SetRotateZ(animation->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &animation->pos, &animation->pos);
	
	animation->pos.w = p;
}

void Track_Dispose(RaceTrackPtr track)
{
	if(track == nil)
	return;
	
	DisposePtr((Ptr) track);
}

void Enclosure_Dispose(CollisionEnclosurePtr enclosure)
{
	if(enclosure == nil)
	return;
	
	DisposePtr((Ptr) enclosure);
}

void Mesh_Dispose(TerrainMeshPtr mesh)
{
	if(mesh == nil)
	return;
	
	DisposePtr((Ptr) mesh->pointList);
	DisposePtr((Ptr) mesh->normalList);
	DisposePtr((Ptr) mesh->lightningList);
	DisposePtr((Ptr) mesh->triangleList);
	
	DisposePtr((Ptr) mesh);
}

void SeaMesh_Dispose(SeaMeshPtr mesh)
{
	if(mesh == nil)
	return;
	
	DisposePtr((Ptr) mesh->pointList);
	DisposePtr((Ptr) mesh->lightningList);
	DisposePtr((Ptr) mesh->triangleList);
	
	DisposePtr((Ptr) mesh);
}

void AmbientSound_Dispose(AmbientSoundPtr sound)
{
	if(sound == nil)
	return;
	
	DisposePtr((Ptr) sound);
}

void Path_Dispose(ModelAnimationPtr path)
{
	if(path == nil)
	return;
	
	DisposePtr((Ptr) path);
}

void ItemList_Dispose(SpecialItemReferencePtr itemList)
{
	if(itemList == nil)
	return;
	
	DisposePtr((Ptr) itemList);
}

void Terrain_ReleaseData(TerrainPtr terrain)
{
	long			i;
	
	if(terrain == nil)
	return;
	
	for(i = 0; i < terrain->meshCount; ++i)
	Mesh_Dispose(terrain->meshList[i]);
	
	for(i = 0; i < terrain->seaCount; ++i)
	SeaMesh_Dispose(terrain->seaList[i]);
	
	for(i = 0; i < terrain->cameraCount; ++i)
	Camera_Dispose(terrain->cameraList[i]);
	terrain->cameraCount = 0;
	
	for(i = 0; i < terrain->soundCount; ++i)
	AmbientSound_Dispose(terrain->soundList[i]);
	terrain->soundCount = 0;
	
	for(i = 0; i < terrain->animCount; ++i)
	Path_Dispose(terrain->animList[i]);
	terrain->animCount = 0;
	
	for(i = 0; i < terrain->trackCount; ++i)
	Track_Dispose(terrain->trackList[i]);
	terrain->trackCount = 0;
	
	if(terrain->itemReferenceCount)
	ItemList_Dispose(terrain->itemReferenceList);
	terrain->itemReferenceCount = 0;
	
	for(i = 0; i < terrain->enclosureCount; ++i)
	Enclosure_Dispose(terrain->enclosureList[i]);
	terrain->enclosureCount = 0;
}

void Sky_MapSkyTexture(ShapePtr shape, TerrainPtr terrain)
{
	long				x,
						y;
	TriFacePtr			t;
	float				u1, u2;
	float				v1, v2;
	long				p1, p2;
	long				q1, q2;
	
	t = shape->triangleList;
	
	if(terrain->skyFlags & kSkyFlag_Tile)
	for(y = 0; y < kLatBands - 1; y++){
		v1 = 1.0 - ((float) y) / kLatBands;
		v2 = 1.0 - ((float) y + 1) / kLatBands;
		for(x = 0; x < kLonBands; x++){
			u1 = ((float) x) / kLonBands;
			u2 = ((float) x + 1) / kLonBands;
			p2 = kLonBands + x + y * (kLonBands + 1);
			p1 = p2 - (kLonBands + 1);
			q2 = p2 + 1;
			q1 = p1 + 1;
			
			if(y == 0) {
				shape->pointList[x].u = u1 * kTileFactor;
				shape->pointList[x].v = G(1) * kTileFactor;
				shape->pointList[x].c = 1.0;
				
				shape->pointList[q2].u = u2 * kTileFactor;
				shape->pointList[q2].v = G(v2) * kTileFactor;
				shape->pointList[q2].c = 1.0;
				
				shape->pointList[p2].u = u1 * kTileFactor;
				shape->pointList[p2].v = G(v2) * kTileFactor;
				shape->pointList[p2].c = 1.0;
				
			} else {
				shape->pointList[p2].u = u1 * kTileFactor;
				shape->pointList[p2].v = G(v2) * kTileFactor;
				shape->pointList[p2].c = 1.0;
				
				shape->pointList[p1].u = u1 * kTileFactor;
				shape->pointList[p1].v = G(v1) * kTileFactor;
				shape->pointList[p1].c = 1.0;

				shape->pointList[q2].u = u2 * kTileFactor;
				shape->pointList[q2].v = G(v2) * kTileFactor;
				shape->pointList[q2].c = 1.0;
				
				shape->pointList[p1].u = u1 * kTileFactor;
				shape->pointList[p1].v = G(v1) * kTileFactor;
				shape->pointList[p1].c = 1.0;
				
				shape->pointList[q1].u = u2 * kTileFactor;
				shape->pointList[q1].v = G(v1) * kTileFactor;
				shape->pointList[q1].c = 1.0;

				shape->pointList[q2].u = u2 * kTileFactor;
				shape->pointList[q2].v = G(v2) * kTileFactor;
				shape->pointList[q2].c = 1.0;
				
			}
		}
	}
	else
	for(y = 0; y < kLatBands - 1; y++){
		v1 = 1.0 - ((float) y) / kLatBands;
		v2 = 1.0 - ((float) y + 1) / kLatBands;
		for(x = 0; x < kLonBands; x++){
			u1 = ((float) x) / kLonBands;
			u2 = ((float) x + 1) / kLonBands;
			p2 = kLonBands + x + y * (kLonBands + 1);
			p1 = p2 - (kLonBands + 1);
			q2 = p2 + 1;
			q1 = p1 + 1;
			
			if(y == 0) {
				shape->pointList[x].u = u1;
				shape->pointList[x].v = F(1);
				shape->pointList[x].c = 1.0;
				
				shape->pointList[q2].u = u2;
				shape->pointList[q2].v = F(v2);
				shape->pointList[q2].c = 1.0;
				
				shape->pointList[p2].u = u1;
				shape->pointList[p2].v = F(v2);
				shape->pointList[p2].c = 1.0;
				
			} else {
				shape->pointList[p2].u = u1;
				shape->pointList[p2].v = F(v2);
				shape->pointList[p2].c = 1.0;
				
				shape->pointList[p1].u = u1;
				shape->pointList[p1].v = F(v1);
				shape->pointList[p1].c = 1.0;

				shape->pointList[q2].u = u2;
				shape->pointList[q2].v = F(v2);
				shape->pointList[q2].c = 1.0;
				
				shape->pointList[p1].u = u1;
				shape->pointList[p1].v = F(v1);
				shape->pointList[p1].c = 1.0;
				
				shape->pointList[q1].u = u2;
				shape->pointList[q1].v = F(v1);
				shape->pointList[q1].c = 1.0;

				shape->pointList[q2].u = u2;
				shape->pointList[q2].v = F(v2);
				shape->pointList[q2].c = 1.0;
				
			}
		}
	}
}

float Mesh_IsClicked(StatePtr state, TerrainMeshPtr mesh, MatrixPtr negatedCamera, Point whereMouse, VectorPtr mousePos)
{
	long				x;
	Vector				v1,
						v2,
						mouse,
						n,
						verts[3];
	Matrix				r1;
	TriFacePtr			tri;
	float				pixelConversion = state->pixelConversion,
						iw;
	VectorPtr			vg0,
						vg1,
						vg2;
	float				curDepth = kHugeDistance,
						depth;
						
	mouse.x = whereMouse.h;
	mouse.y = whereMouse.v;
	mouse.z = 0.0;
	
	//Calculate mesh-to-camera matrix
	if(mesh->flags & kFlag_UseMatrix)
	Matrix_Cat(&mesh->pos, negatedCamera, &r1);
	else
	r1 = *negatedCamera;
	
	//Transform points
	for(x = 0; x < mesh->pointCount; x++)
	Matrix_TransformVector(&r1, &mesh->pointList[x].point, (VectorPtr) &vGouraudBuffer[x]);
	
	tri = mesh->triangleList;
	for(x = 0; x < mesh->triangleCount; x++) {
		//Map to VectorPtr
		vg0 = (VectorPtr) &vGouraudBuffer[tri->corner[0]];
		vg1 = (VectorPtr) &vGouraudBuffer[tri->corner[1]];
		vg2 = (VectorPtr) &vGouraudBuffer[tri->corner[2]];
		
		//Remove backfacing triangles
		if(mesh->flags & kFlag_RemoveBF) {
			Vector_Subtract(vg1, vg0, &v1);
			Vector_Subtract(vg2, vg0, &v2);
			Vector_CrossProduct(&v2, &v1, &n);
			if(Vector_DotProduct(vg0, &n) > 0) {
				++tri;
				continue;
			}
		}
		
		//Remove invisible triangles
		if((vg0->z < state->d) || (vg1->z < state->d) || (vg2->z < state->d)) {
			++tri;
			continue;
		}
		if((vg0->z > kMaxViewDistance) || (vg1->z > kMaxViewDistance) || (vg2->z > kMaxViewDistance)) {
			++tri;
			continue;
		}
		
		//Project points
		iw = 1.0 / vg0->z;
		verts[0].x = vg0->x * iw * pixelConversion + state->projectionOffset_X;
		verts[0].y = vg0->y * iw * pixelConversion + state->projectionOffset_Y;
		verts[0].z = vg0->z;
		iw = 1.0 / vg1->z;
		verts[1].x = vg1->x * iw * pixelConversion + state->projectionOffset_X;
		verts[1].y = vg1->y * iw * pixelConversion + state->projectionOffset_Y;
		verts[1].z = vg1->z;
		iw = 1.0 / vg2->z;
		verts[2].x = vg2->x * iw * pixelConversion + state->projectionOffset_X;
		verts[2].y = vg2->y * iw * pixelConversion + state->projectionOffset_Y;
		verts[2].z = vg2->z;
		
		//Is click inside the face?
		depth = ClickInFace(&mouse, verts);
		if((depth > 0.0) && (depth <= curDepth)) {
			*mousePos = mesh->pointList[tri->corner[0]].point;
			Vector_Add(mousePos, &mesh->pointList[tri->corner[1]].point, mousePos);
			Vector_Add(mousePos, &mesh->pointList[tri->corner[2]].point, mousePos);
			Vector_Multiply(1.0 / 3.0, mousePos, mousePos);
			
			curDepth = depth;
		}
		
		++tri;
	}
	
	//We found a face
	if(curDepth < kHugeDistance)
	return curDepth;
	
	return -1.0;
}

void Mesh_CalculateBounding(TerrainPtr terrain, TerrainMeshPtr mesh, Boolean isTerrainMesh)
{
	long			i;
	float			minX = kHugeDistance,
					minY = kHugeDistance,
					minZ = kHugeDistance,
					maxX = -kHugeDistance,
					maxY = -kHugeDistance,
					maxZ = -kHugeDistance;
	
	for(i = 0; i < mesh->pointCount; ++i) {
		
		if(mesh->pointList[i].point.x < minX)
		minX = mesh->pointList[i].point.x;
		if(mesh->pointList[i].point.x > maxX)
		maxX = mesh->pointList[i].point.x;
		
		if(mesh->pointList[i].point.y < minY)
		minY = mesh->pointList[i].point.y;
		if(mesh->pointList[i].point.y > maxY)
		maxY = mesh->pointList[i].point.y;
		
		if(mesh->pointList[i].point.z < minZ)
		minZ = mesh->pointList[i].point.z;
		if(mesh->pointList[i].point.z > maxZ)
		maxZ = mesh->pointList[i].point.z;
		
	}
	if(isTerrainMesh == false) {
		minY = -terrain->waveAmplitude;
		maxY = terrain->waveAmplitude;
	}
	
	mesh->boundingBox[0].x = minX;
	mesh->boundingBox[0].y = minY;
	mesh->boundingBox[0].z = minZ;
	
	mesh->boundingBox[1].x = maxX;
	mesh->boundingBox[1].y = minY;
	mesh->boundingBox[1].z = minZ;
	
	mesh->boundingBox[2].x = maxX;
	mesh->boundingBox[2].y = minY;
	mesh->boundingBox[2].z = maxZ;
	
	mesh->boundingBox[3].x = minX;
	mesh->boundingBox[3].y = minY;
	mesh->boundingBox[3].z = maxZ;
	
	mesh->boundingBox[4].x = minX;
	mesh->boundingBox[4].y = maxY;
	mesh->boundingBox[4].z = minZ;
	
	mesh->boundingBox[5].x = maxX;
	mesh->boundingBox[5].y = maxY;
	mesh->boundingBox[5].z = minZ;
	
	mesh->boundingBox[6].x = maxX;
	mesh->boundingBox[6].y = maxY;
	mesh->boundingBox[6].z = maxZ;
	
	mesh->boundingBox[7].x = minX;
	mesh->boundingBox[7].y = maxY;
	mesh->boundingBox[7].z = maxZ;
}

void Mesh_UpdateMatrix(TerrainMeshPtr mesh)
{
	Vector			p;
	Matrix			m;
	
	p = mesh->pos.w;
	Matrix_Clear(&mesh->pos);
	
	mesh->pos.x.x = mesh->scaleX;
	mesh->pos.y.y = mesh->scaleY;
	mesh->pos.z.z = mesh->scaleZ;
	
	Matrix_SetRotateX(mesh->rotateX, &m);
	Matrix_MultiplyByMatrix(&m, &mesh->pos, &mesh->pos);
	
	Matrix_SetRotateY(mesh->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &mesh->pos, &mesh->pos);
	
	Matrix_SetRotateZ(mesh->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &mesh->pos, &mesh->pos);
	
	mesh->pos.w = p;
}

void Mesh_ReCenterOrigin(TerrainMeshPtr mesh)
{
	float			xOffset,
					yOffset,
					zOffset;
	long			i;
	float			minX = kHugeDistance,
					minY = kHugeDistance,
					minZ = kHugeDistance,
					maxX = -kHugeDistance,
					maxY = -kHugeDistance,
					maxZ = -kHugeDistance;
	
	for(i = 0; i < mesh->pointCount; ++i) {
		
		if(mesh->pointList[i].point.x < minX)
		minX = mesh->pointList[i].point.x;
		else if(mesh->pointList[i].point.x > maxX)
		maxX = mesh->pointList[i].point.x;
		
		if(mesh->pointList[i].point.y < minY)
		minY = mesh->pointList[i].point.y;
		else if(mesh->pointList[i].point.y > maxY)
		maxY = mesh->pointList[i].point.y;
		
		if(mesh->pointList[i].point.z < minZ)
		minZ = mesh->pointList[i].point.z;
		else if(mesh->pointList[i].point.z > maxZ)
		maxZ = mesh->pointList[i].point.z;
		
	}
	
	xOffset = (minX + maxX) / 2;
	yOffset = (minY + maxY) / 2;
	zOffset = (minZ + maxZ) / 2;
	
	for(i = 0; i < mesh->pointCount; ++i) {
		mesh->pointList[i].point.x -= xOffset;
		mesh->pointList[i].point.y -= yOffset;
		mesh->pointList[i].point.z -= zOffset;
	}
	Mesh_CalculateBounding(nil, mesh, true);
	
	mesh->pos.w.x = xOffset * mesh->scaleX;
	mesh->pos.w.y = yOffset * mesh->scaleY;
	mesh->pos.w.z = zOffset * mesh->scaleZ;
	
	for(i = 0; i < mesh->referenceCount; ++i) {
		mesh->referenceList[i].pos.w.x -= xOffset * mesh->scaleX;
		mesh->referenceList[i].pos.w.y -= yOffset * mesh->scaleY;
		mesh->referenceList[i].pos.w.z -= zOffset * mesh->scaleZ;
	}
}

void Mesh_ApplyMatrix(TerrainMeshPtr mesh)
{
	long				i;
	float				oldScale;
	
	if(!(mesh->flags & kFlag_UseMatrix))
	return;
	
	for(i = 0; i < mesh->pointCount; i++)
	Matrix_TransformVector(&mesh->pos, &mesh->pointList[i].point, &mesh->pointList[i].point);
	
	mesh->scaleX = 1.0;
	mesh->scaleY = 1.0;
	mesh->scaleZ = 1.0;
	Mesh_UpdateMatrix(mesh);
	for(i = 0; i < mesh->pointCount; i++) {
		Matrix_RotateVector(&mesh->pos, &mesh->normalList[i], &mesh->normalList[i]);
		Vector_Normalize(&mesh->normalList[i], &mesh->normalList[i]);
	}
	
#if 0
	for(i = 0; i < mesh->referenceCount; ++i)
	Matrix_TransformVector(&mesh->pos, &mesh->referenceList[i].pos.w,  &mesh->referenceList[i].pos.w);
#else
	for(i = 0; i < mesh->referenceCount; ++i) {
		oldScale = mesh->referenceList[i].scale;
		mesh->referenceList[i].scale = 1.0;
		ModelReference_UpdateMatrix(&mesh->referenceList[i]);
		
		Matrix_Cat(&mesh->referenceList[i].pos, &mesh->pos, &mesh->referenceList[i].pos);
		Matrix_ExtractAngles(&mesh->referenceList[i].pos, &mesh->referenceList[i].rotateX, &mesh->referenceList[i].rotateY, &mesh->referenceList[i].rotateZ);
		
		mesh->referenceList[i].scale = oldScale;
		ModelReference_UpdateMatrix(&mesh->referenceList[i]);
	}
#endif
	
	mesh->rotateX = 0.0;
	mesh->rotateY = 0.0;
	mesh->rotateZ = 0.0;
	Matrix_Clear(&mesh->pos);
	mesh->flags &= ~kFlag_UseMatrix;
	Mesh_CalculateBounding(nil, mesh, true);
}

TerrainMeshPtr Mesh_Copy(TerrainMeshPtr mesh)
{
	TerrainMeshPtr				copy;
	
	copy = (TerrainMeshPtr) NewPtr(sizeof(TerrainMesh));
	if(copy == nil)
	return nil;
	BlockMove(mesh, copy, sizeof(TerrainMesh));
	BlockMove(" - copy", &copy->name[copy->name[0] + 1], 7);
	copy->name[0] += 7;
	
	copy->pointList = (VertexPtr) NewPtr(GetPtrSize((Ptr) mesh->pointList));
	if(copy->pointList == nil)
	return nil;
	BlockMove(mesh->pointList, copy->pointList, GetPtrSize((Ptr) mesh->pointList));
	
	copy->lightningList = (LightningPtr) NewPtr(GetPtrSize((Ptr) mesh->lightningList));
	if(copy->lightningList == nil)
	return nil;
	BlockMove(mesh->lightningList, copy->lightningList, GetPtrSize((Ptr) mesh->lightningList));
	
	copy->triangleList = (TriFacePtr) NewPtr(GetPtrSize((Ptr) mesh->triangleList));
	if(copy->triangleList == nil)
	return nil;
	BlockMove(mesh->triangleList, copy->triangleList, GetPtrSize((Ptr) mesh->triangleList));
	
	copy->normalList = (VectorPtr) NewPtr(GetPtrSize((Ptr) mesh->normalList));
	if(copy->normalList == nil)
	return nil;
	BlockMove(mesh->normalList, copy->normalList, GetPtrSize((Ptr) mesh->normalList));
	
	copy->referenceCount = 0;
	
	return copy;
}

#if 0
SeaMeshPtr SeaMesh_Copy(SeaMeshPtr mesh)
{
	SeaMeshPtr				copy;
	
	copy = (SeaMeshPtr) NewPtr(sizeof(SeaMesh));
	if(copy == nil)
	return nil;
	BlockMove(mesh, copy, sizeof(SeaMesh));
	BlockMove(" - copy", &copy->name[copy->name[0] + 1], 7);
	copy->name[0] += 7;
	
	copy->pointList = (VertexPtr) NewPtr(GetPtrSize((Ptr) mesh->pointList));
	if(copy->pointList == nil)
	return nil;
	BlockMove(mesh->pointList, copy->pointList, GetPtrSize((Ptr) mesh->pointList));
	
	copy->lightningList = (LightningPtr) NewPtr(GetPtrSize((Ptr) mesh->lightningList));
	if(copy->lightningList == nil)
	return nil;
	BlockMove(mesh->lightningList, copy->lightningList, GetPtrSize((Ptr) mesh->lightningList));
	
	copy->triangleList = (TriFacePtr) NewPtr(GetPtrSize((Ptr) mesh->triangleList));
	if(copy->triangleList == nil)
	return nil;
	BlockMove(mesh->triangleList, copy->triangleList, GetPtrSize((Ptr) mesh->triangleList));
	
	copy->referenceCount = 0;
	
	return copy;
}
#endif

TerrainMeshPtr Mesh_GetCurrent(TerrainPtr terrain, VectorPtr position)
{
	TerrainMeshPtr			mesh,
							currentMesh = nil;
	long					i;
	float					x = position->x,
							z = position->z,
							minArea = kHugeDistance * kHugeDistance,
							area;
							
	for(i = 0; i < terrain->meshCount; ++i) {
		mesh = terrain->meshList[i];
		if((x > mesh->boundingBox[0].x) && (x < mesh->boundingBox[1].x) 
			&& (z > mesh->boundingBox[0].z) && (z < mesh->boundingBox[2].z)) {
			area = (mesh->boundingBox[1].x - mesh->boundingBox[0].x) * (mesh->boundingBox[2].z - mesh->boundingBox[0].z);
			if(area < minArea) {
				currentMesh = mesh;
				minArea = area;
			}
		}
	}
	
	return currentMesh;
}
