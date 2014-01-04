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


#ifndef __INFINITY_METAFILE_UTILS__
#define __INFINITY_METAFILE_UTILS__

#include				"Infinity Terrain.h"

//ROUTINES:

//File: MetaFile Write.cp
OSErr MetaFile_Write_Shape(MetaFile_ReferencePtr reference, ShapePtr shape);
OSErr MetaFile_Write_Texture(MetaFile_ReferencePtr reference, CompressedTexturePtr texture);
OSErr MetaFile_Write_Script(MetaFile_ReferencePtr reference, ScriptPtr script);
OSErr MetaFile_Write_Skeleton(MetaFile_ReferencePtr reference, SkeletonPtr skeleton);
OSErr MetaFile_Write_Camera(MetaFile_ReferencePtr reference, CameraPtr camera);

OSErr MetaFile_LowLevelWrite_DataPtr(MetaFile_ReferencePtr reference, long tagNum, Ptr data);
OSErr MetaFile_Write_Delete(MetaFile_ReferencePtr reference, long tagNum);
void MetaFile_Write_IncreaseDepth(MetaFile_ReferencePtr reference);
void MetaFile_Write_DecreaseDepth(MetaFile_ReferencePtr reference);

OSErr MetaFile_Write_Data(MetaFile_ReferencePtr reference, OSType dataType, Ptr dataPtr, long dataSize, OSType ID, short version);
OSErr MetaFile_Write_TextureSet(MetaFile_ReferencePtr reference, TextureSetPtr set);
OSErr MetaFile_Write_MegaObject(MetaFile_ReferencePtr reference, MegaObjectPtr mega);
OSErr MetaFile_Write_TerrainMesh(MetaFile_ReferencePtr reference, TerrainMeshPtr mesh);
OSErr MetaFile_Write_SeaMesh(MetaFile_ReferencePtr reference, SeaMeshPtr mesh);
OSErr MetaFile_Write_AmbientSound(MetaFile_ReferencePtr reference, AmbientSoundPtr sound);
OSErr MetaFile_Write_ModelAnimation(MetaFile_ReferencePtr reference, ModelAnimationPtr path);
OSErr MetaFile_Write_Track(MetaFile_ReferencePtr reference, RaceTrackPtr track);
OSErr MetaFile_Write_Enclosure(MetaFile_ReferencePtr reference, CollisionEnclosurePtr enclosure);
OSErr MetaFile_Write_Terrain(MetaFile_ReferencePtr reference, TerrainPtr terrain);
OSErr MetaFile_Write_TextureList(MetaFile_ReferencePtr reference, CompressedTexturePtr textureList[], long textureCount, OSType tag);
OSErr MetaFile_Write_View(MetaFile_ReferencePtr reference, ViewPtr view);
OSErr MetaFile_Write_CameraPosition(MetaFile_ReferencePtr reference, CameraPositionPtr camera);
OSErr MetaFile_Write_Preview(MetaFile_ReferencePtr reference, PicHandle previewPic);
OSErr MetaFile_Write_CameraLayout(MetaFile_ReferencePtr reference, CameraStatePtr localCamera, CameraStatePtr topCamera, CameraStatePtr frontCamera, CameraStatePtr rightCamera, CameraStatePtr worldState);

OSErr MetaFile_NewFile(FSSpec* destFile, MetaFile_ReferencePtr* newReference);
OSErr MetaFile_Write_OpenFile(FSSpec* destFile, MetaFile_ReferencePtr* newReference);
OSErr MetaFile_Write_CloseFile(MetaFile_ReferencePtr reference, OSType creatorCode);

//File: MetaFile Read.cp
long MetaFile_CountTags(MetaFile_ReferencePtr reference, OSType tag);
long MetaFile_CountTags_InsideMegaObject(MetaFile_ReferencePtr reference, OSType modelID, OSType tag);
long MetaFile_CountDataSubTags(MetaFile_ReferencePtr reference, OSType subTag);

OSErr MetaFile_LowLevelRead_DataPtr(MetaFile_ReferencePtr reference, long tagNum, Ptr* data);
OSErr MetaFile_LowLevelRead_DataHandle(MetaFile_ReferencePtr reference, long tagNum, Handle* data);

OSErr MetaFile_Read_Data(MetaFile_ReferencePtr reference, OSType* dataType, long index, Ptr* dataPtr, OSType* dataID);
OSErr MetaFile_Read_TextureList(MetaFile_ReferencePtr reference, CompressedTexturePtr textureList[], long* textureCount, OSType tag);
OSErr MetaFile_Read_TextureSet_ByIndex(MetaFile_ReferencePtr reference, TextureSetPtr* set, OSType modelID, long index);
OSErr MetaFile_Read_TextureSet_ByID(MetaFile_ReferencePtr reference, TextureSetPtr* set, OSType modelID, OSType setID);
OSErr MetaFile_Read_ExtractTextureSet(MetaFile_ReferencePtr reference, TextureSetPtr* set, MegaObjectPtr mega);
OSErr MetaFile_Read_GetModelDefaultTextureSet(MetaFile_ReferencePtr reference, TextureSetPtr* set, MegaObjectPtr mega);
OSErr MetaFile_Read_Terrain(MetaFile_ReferencePtr reference, TerrainPtr terrain, long skipFlags);
OSErr MetaFile_Read_MegaObject_ByIndex(MetaFile_ReferencePtr reference, MegaObjectPtr mega, long index, long skipFlags);
OSErr MetaFile_Read_MegaObjectNameID_ByIndex(MetaFile_ReferencePtr reference, Str63 name, OSType* id, long index);
OSErr MetaFile_Read_TerrainNameID_ByIndex(MetaFile_ReferencePtr reference, Str63 name, OSType* id, long index);
OSErr MetaFile_Read_TextureSetName_ByIndex(MetaFile_ReferencePtr reference, OSType modelID, Str63 name, long index);
OSErr MetaFile_Read_Preview(MetaFile_ReferencePtr reference, PicHandle* previewPic);
OSErr MetaFile_Read_CameraLayout(MetaFile_ReferencePtr reference, CameraStatePtr localCamera, CameraStatePtr topCamera, CameraStatePtr frontCamera, CameraStatePtr rightCamera, CameraStatePtr worldState);
OSErr MetaFile_Read_CameraPosition_ByIndex(MetaFile_ReferencePtr reference, CameraPositionPtr camera, long index);
OSErr MetaFile_Read_View_ByIndex(MetaFile_ReferencePtr reference, ViewPtr* view, long index);

OSErr MetaFile_Read_OpenFile(FSSpec* destFile, MetaFile_ReferencePtr* newReference);
OSErr MetaFile_Read_CloseFile(MetaFile_ReferencePtr reference);
PicHandle MetaFile_ExtractPreview(FSSpec* theFile);

//File: MetaFile Utils.cp
long MetaFile_PickModelIndex(MetaFile_ReferencePtr reference, UniversalProcPtr filterProc);
long MetaFile_PickModelTextureSetIndex(MetaFile_ReferencePtr reference, OSType modelID, UniversalProcPtr filterProc);

#endif
