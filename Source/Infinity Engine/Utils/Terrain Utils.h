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


#ifndef __INFINITY_TERRAIN_UTILS__
#define __INFINITY_TERRAIN_UTILS__

//PROTOTYPES:

//File: Terrain Utils.cp
void ModelReference_UpdateMatrix(ModelReferencePtr reference);
void Anim_UpdateMatrix(ModelAnimationPtr animation);

void Mesh_Dispose(TerrainMeshPtr mesh);
void SeaMesh_Dispose(SeaMeshPtr mesh);
void AmbientSound_Dispose(AmbientSoundPtr sound);
void Track_Dispose(RaceTrackPtr track);
void Enclosure_Dispose(CollisionEnclosurePtr enclosure);
void ItemList_Dispose(SpecialItemReferencePtr itemList);
void Path_Dispose(ModelAnimationPtr path);
void Terrain_ReleaseData(TerrainPtr terrain);
void Sky_MapSkyTexture(ShapePtr shape, TerrainPtr terrain);
float Mesh_IsClicked(StatePtr state, TerrainMeshPtr mesh, MatrixPtr negatedCamera, Point whereMouse, VectorPtr mousePos);
void Mesh_CalculateBounding(TerrainPtr terrain, TerrainMeshPtr mesh, Boolean isTerrainMesh);
void Mesh_UpdateMatrix(TerrainMeshPtr mesh);
void Mesh_ReCenterOrigin(TerrainMeshPtr mesh);
void Mesh_ApplyMatrix(TerrainMeshPtr mesh);
TerrainMeshPtr Mesh_Copy(TerrainMeshPtr mesh);
TerrainMeshPtr Mesh_GetCurrent(TerrainPtr terrain, VectorPtr position);

#endif
