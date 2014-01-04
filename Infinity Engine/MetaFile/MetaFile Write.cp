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
#include			"Infinity Terrain.h"
#include			"MetaFile Structures.h"
#include			"Infinity Error Codes.h"
#include			"MetaFile Utils.h"

//ROUTINES:

//Utils:

inline void Add_TableEntry(MetaFile_TablePtr table, long depth, short version, OSType tag, OSType subTag, OSType ID, long dataSize, long dataPosition)
{
	MetaFile_EntryPtr		entry;
	
	entry = &(table->tagList[table->tagCount]);
	
	entry->depth = depth;
	entry->flags = 0;
	entry->version = version;
	entry->tag = tag;
	entry->subTag = subTag;
	entry->ID = ID;
	entry->dataSize = dataSize;
	entry->dataPosition = dataPosition;
	
	table->tagCount += 1;
}

inline long GetFilePos(short fileID)
{
	long				filePos;
	
	GetFPos(fileID, &filePos);
				
	return filePos;
}

static OSErr Space_Remove(short fileID, long start, long size)
{
	Ptr					buffer;
	long				copySize,
						fileSize;
	OSErr				theError;
	
	//Compute copy size
	GetEOF(fileID, &fileSize);
	copySize = fileSize - (start + size);
	
	//Allocate memory
	buffer = NewPtr(copySize);
	if(buffer == nil)
	return MemError();
	
	//Load data
	SetFPos(fileID, fsFromStart, start + size);
	theError = FSRead(fileID, &copySize, buffer);
	if(theError) {
		DisposePtr(buffer);
		return theError;
	}
	
	//Dump data
	SetFPos(fileID, fsFromStart, start);
	theError = FSWrite(fileID, &copySize, buffer);
	if(theError) {
		DisposePtr(buffer);
		return theError;
	}
	
	//Clean up
	DisposePtr(buffer);
	
	//Correct EOF
	fileSize -= size;
	SetEOF(fileID, fileSize);
	
	return noErr;
}

//Core:

OSErr MetaFile_Write_Data(MetaFile_ReferencePtr reference, OSType dataType, Ptr dataPtr, long dataSize, OSType ID, short version)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write object data
	bytes = dataSize;
	Add_TableEntry(table, currentDepth, version, kTag_Data, dataType, ID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, dataPtr);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_Texture(MetaFile_ReferencePtr reference, CompressedTexturePtr texture)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write texture data
	bytes = GetPtrSize((Ptr) texture);
	Add_TableEntry(table, currentDepth, kVersion_Texture, kTag_CompressedTexture, kSubTag_None, texture->name, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, texture);
	if(theError)
	return theError;
	
	//Write image
	bytes = GetHandleSize((Handle) texture->image);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_TextureImage, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	HLock((Handle) texture->image);
	theError = FSWrite(destFileID, &bytes, *(texture->image));
	HUnlock((Handle) texture->image);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_TextureSet(MetaFile_ReferencePtr reference, TextureSetPtr set)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	long				i;
	
	//Write texture set data
	bytes = sizeof(TextureSet);
	Add_TableEntry(table, currentDepth, kVersion_TextureSet, kTag_TextureSet, kSubTag_None, set->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, set);
	if(theError)
	return theError;
	
	MetaFile_Write_IncreaseDepth(reference);
		
	for(i = 0; i < set->textureCount; ++i) {
		theError = MetaFile_Write_Texture(reference, set->textureList[i]);
		if(theError)
		return theError;
	}
	
	MetaFile_Write_DecreaseDepth(reference);
	
	return noErr;
}

OSErr MetaFile_Write_Shape(MetaFile_ReferencePtr reference, ShapePtr shape)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write shape data
	bytes = sizeof(Shape);
	Add_TableEntry(table, currentDepth, kVersion_Shape, kTag_Shape, kSubTag_None, shape->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, shape);
	if(theError)
	return theError;
	
	//Write points if any
	if(shape->pointList != nil) {
		bytes = GetPtrSize((Ptr) shape->pointList);
		Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_PointList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
		theError = FSWrite(destFileID, &bytes, shape->pointList);
		if(theError)
		return theError;
	}
	
	//Write triangles if any
	if(shape->triangleList != nil) {
		bytes = GetPtrSize((Ptr) shape->triangleList);
		Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_TriangleList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
		theError = FSWrite(destFileID, &bytes, shape->triangleList);
		if(theError)
		return theError;
	}
	
	//Write normals if any
	if(shape->normalMode != kNoNormals) {
		bytes = GetPtrSize((Ptr) shape->normalList);
		Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_NormalList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
		theError = FSWrite(destFileID, &bytes, shape->normalList);
		if(theError)
		return theError;
	}
	
	//Write custom data if any
	if(shape->dataPtr != nil) {
		bytes = GetPtrSize((Ptr) shape->dataPtr);
		Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_ShapeData, kMetaFile_NoID, bytes, GetFilePos(destFileID));
		theError = FSWrite(destFileID, &bytes, shape->dataPtr);
		if(theError)
		return theError;
	}
	
	return noErr;
}

OSErr MetaFile_Write_Script(MetaFile_ReferencePtr reference, ScriptPtr script)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	long				i;
	
	//Write script data
	bytes = sizeof(Script);
	Add_TableEntry(table, currentDepth, kVersion_Script, kTag_Script, kSubTag_None, script->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, script);
	if(theError)
	return theError;
	
	//Write animations
	for(i = 0; i < script->animationCount; ++i) {
		bytes = GetPtrSize((Ptr) script->animationList[i]);
		Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_Animation, kMetaFile_NoID, bytes, GetFilePos(destFileID));
		theError = FSWrite(destFileID, &bytes, script->animationList[i]);
		if(theError)
		return theError;
	}
	
	return noErr;
}

OSErr MetaFile_Write_Skeleton(MetaFile_ReferencePtr reference, SkeletonPtr skeleton)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write skeleton data
	bytes = GetPtrSize((Ptr) skeleton);
	Add_TableEntry(table, currentDepth, kVersion_Skeleton, kTag_Skeleton, kSubTag_None, skeleton->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, skeleton);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_Camera(MetaFile_ReferencePtr reference, CameraPtr camera)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write camera data
	bytes = sizeof(Camera);
	Add_TableEntry(table, currentDepth, kVersion_Camera, kTag_Camera, kSubTag_None, camera->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, camera);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_MegaObject(MetaFile_ReferencePtr reference, MegaObjectPtr mega)
{
	MetaFile_TablePtr	table = &reference->tagTable;
	long				i;
	OSErr				theError;
	long				bytes;
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	
	//Write 3D model data
	bytes = sizeof(MegaObject);
	Add_TableEntry(table, currentDepth, kVersion_MegaObject, kTag_MegaObject, kSubTag_None, mega->object.id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mega);
	if(theError)
	return theError;
	
	MetaFile_Write_IncreaseDepth(reference);
	
	//Write shapes
	Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_ModelGeometry, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
	MetaFile_Write_IncreaseDepth(reference);
	for(i = 0; i < mega->object.shapeCount; ++i) {
		theError = MetaFile_Write_Shape(reference, mega->object.shapeList[i]);
		if(theError)
		return theError;
	}
	MetaFile_Write_DecreaseDepth(reference);
	
	//Write skeletons
	if(mega->skeletonCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_ModelSkeleton, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < mega->skeletonCount; ++i) {
			theError = MetaFile_Write_Skeleton(reference, mega->skeletonList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	//Write scripts
	if(mega->scriptCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_ModelAnimation, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < mega->scriptCount; ++i) {
			theError = MetaFile_Write_Script(reference, mega->scriptList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	//Write cameras
	if(mega->cameraCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_ModelCameras, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < mega->cameraCount; ++i) {
			theError = MetaFile_Write_Camera(reference, mega->cameraList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	MetaFile_Write_DecreaseDepth(reference);
	
	return noErr;
}

OSErr MetaFile_Write_TerrainMesh(MetaFile_ReferencePtr reference, TerrainMeshPtr mesh)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write mesh data
	bytes = sizeof(TerrainMesh);
	Add_TableEntry(table, currentDepth, kVersion_TerrainMesh, kTag_TerrainMesh, kSubTag_None, mesh->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh);
	if(theError)
	return theError;
	
	//Write points
	bytes = GetPtrSize((Ptr) mesh->pointList);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_PointList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh->pointList);
	if(theError)
	return theError;
	
	//Write lightning
	bytes = GetPtrSize((Ptr) mesh->lightningList);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_LightningList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh->lightningList);
	if(theError)
	return theError;
	
	//Write triangles
	bytes = GetPtrSize((Ptr) mesh->triangleList);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_TriangleList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh->triangleList);
	if(theError)
	return theError;
	
	//Write normals
	bytes = GetPtrSize((Ptr) mesh->normalList);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_NormalList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh->normalList);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_SeaMesh(MetaFile_ReferencePtr reference, SeaMeshPtr mesh)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write mesh data
	bytes = sizeof(SeaMesh);
	Add_TableEntry(table, currentDepth, kVersion_SeaMesh, kTag_SeaMesh, kSubTag_None, mesh->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh);
	if(theError)
	return theError;
	
	//Write points
	bytes = GetPtrSize((Ptr) mesh->pointList);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_PointList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh->pointList);
	if(theError)
	return theError;
	
	//Write lightning
	bytes = GetPtrSize((Ptr) mesh->lightningList);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_LightningList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh->lightningList);
	if(theError)
	return theError;
	
	//Write triangles
	bytes = GetPtrSize((Ptr) mesh->triangleList);
	Add_TableEntry(table, currentDepth + 1, kMetaFile_NoVersion, kTag_IncompleteData, kSubTag_TriangleList, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, mesh->triangleList);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_AmbientSound(MetaFile_ReferencePtr reference, AmbientSoundPtr sound)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write camera data
	bytes = GetPtrSize((Ptr) sound);
	Add_TableEntry(table, currentDepth, kVersion_AmbientSound, kTag_AmbientSound, kSubTag_None, sound->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, sound);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_ModelAnimation(MetaFile_ReferencePtr reference, ModelAnimationPtr path)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write path data
	bytes = GetPtrSize((Ptr) path);
	Add_TableEntry(table, currentDepth, kVersion_ModelAnimation, kTag_ModelAnimation, kSubTag_None, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, path);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_Track(MetaFile_ReferencePtr reference, RaceTrackPtr track)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write track data
	bytes = GetPtrSize((Ptr) track);
	Add_TableEntry(table, currentDepth, kVersion_Track, kTag_Track, kSubTag_None, track->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, track);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_Enclosure(MetaFile_ReferencePtr reference, CollisionEnclosurePtr enclosure)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write track data
	bytes = GetPtrSize((Ptr) enclosure);
	Add_TableEntry(table, currentDepth, kVersion_Enclosure, kTag_Enclosure, kSubTag_None, kNoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, enclosure);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_ItemList(MetaFile_ReferencePtr reference, SpecialItemReferencePtr itemList)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write item list data
	bytes = GetPtrSize((Ptr) itemList);
	Add_TableEntry(table, currentDepth, kVersion_ItemList, kTag_ItemList, kSubTag_None, kNoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, itemList);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_Terrain(MetaFile_ReferencePtr reference, TerrainPtr terrain)
{
	MetaFile_TablePtr	table = &reference->tagTable;
	OSErr				theError;
	long				bytes,
						i;
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	
	//Write terrain data
	bytes = sizeof(Terrain);
	Add_TableEntry(table, currentDepth, kVersion_Terrain, kTag_Terrain, kSubTag_None, terrain->id, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, terrain);
	if(theError)
	return theError;
	
	MetaFile_Write_IncreaseDepth(reference);
	
	//Write terrain meshes
	Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_TerrainGeometry, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
	MetaFile_Write_IncreaseDepth(reference);
	for(i = 0; i < terrain->meshCount; ++i) {
		theError = MetaFile_Write_TerrainMesh(reference, terrain->meshList[i]);
		if(theError)
		return theError;
	}
	for(i = 0; i < terrain->seaCount; ++i) {
		theError = MetaFile_Write_SeaMesh(reference, terrain->seaList[i]);
		if(theError)
		return theError;
	}
	MetaFile_Write_DecreaseDepth(reference);
	
	//Write camera list
	if(terrain->cameraCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_TerrainCameras, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < terrain->cameraCount; ++i) {
			theError = MetaFile_Write_Camera(reference, terrain->cameraList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	//Write ambient sound list
	if(terrain->soundCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_TerrainSounds, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < terrain->soundCount; ++i) {
			theError = MetaFile_Write_AmbientSound(reference, terrain->soundList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	//Write animation list
	if(terrain->animCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_TerrainAnimations, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < terrain->animCount; ++i) {
			theError = MetaFile_Write_ModelAnimation(reference, terrain->animList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	//Write track list
	if(terrain->trackCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_TerrainTracks, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < terrain->trackCount; ++i) {
			theError = MetaFile_Write_Track(reference, terrain->trackList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	//Write special items list
	if(terrain->itemReferenceCount) {
		theError = MetaFile_Write_ItemList(reference, terrain->itemReferenceList);
		if(theError)
		return theError;
	}
	
	//Write enclosure list
	if(terrain->enclosureCount) {
		Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, kFakeTag_TerrainEnclosures, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
		MetaFile_Write_IncreaseDepth(reference);
		for(i = 0; i < terrain->enclosureCount; ++i) {
			theError = MetaFile_Write_Enclosure(reference, terrain->enclosureList[i]);
			if(theError)
			return theError;
		}
		MetaFile_Write_DecreaseDepth(reference);
	}
	
	MetaFile_Write_DecreaseDepth(reference);
	
	return noErr;
}

OSErr MetaFile_Write_TextureList(MetaFile_ReferencePtr reference, CompressedTexturePtr textureList[], long textureCount, OSType tag)
{
	MetaFile_TablePtr	table = &reference->tagTable;
	OSErr				theError;
	long				i;
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	
	Add_TableEntry(table, reference->currentDepth, kMetaFile_NoVersion, tag, kSubTag_None, kMetaFile_NoID, kMetaFile_NoData, kMetaFile_NoData);
	MetaFile_Write_IncreaseDepth(reference);
	
	for(i = 0; i < textureCount; ++i) {
		theError = MetaFile_Write_Texture(reference, textureList[i]);
		if(theError)
		return theError;
	}
	
	MetaFile_Write_DecreaseDepth(reference);
	
	return noErr;
}

OSErr MetaFile_Write_View(MetaFile_ReferencePtr reference, ViewPtr view)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write view data
	bytes = sizeof(View);
	Add_TableEntry(table, currentDepth, kVersion_View, kTag_ViewSetup, kSubTag_None, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, view);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_CameraPosition(MetaFile_ReferencePtr reference, CameraPositionPtr camera)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write camera position data
	bytes = sizeof(CameraPosition);
	Add_TableEntry(table, currentDepth, kVersion_CameraPosition, kTag_CameraPosition, kSubTag_None, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	theError = FSWrite(destFileID, &bytes, camera);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_Preview(MetaFile_ReferencePtr reference, PicHandle previewPic)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write preview data
	bytes = GetHandleSize((Handle) previewPic);
	Add_TableEntry(table, currentDepth, kMetaFile_NoVersion, kTag_Preview, kSubTag_None, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	HLock((Handle) previewPic);
	theError = FSWrite(destFileID, &bytes, *previewPic);
	HUnlock((Handle) previewPic);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_CameraLayout(MetaFile_ReferencePtr reference, CameraStatePtr localCamera, CameraStatePtr topCamera, CameraStatePtr frontCamera, CameraStatePtr rightCamera, CameraStatePtr worldState)
{
	short				destFileID = reference->fileRefNum,
						currentDepth = reference->currentDepth;
	MetaFile_TablePtr	table = &reference->tagTable;
	long				bytes;
	OSErr				theError;
	
	//Write camera data
	bytes = 5 * sizeof(CameraState);
	Add_TableEntry(table, currentDepth, kVersion_CameraLayout, kTag_CameraLayout, kSubTag_None, kMetaFile_NoID, bytes, GetFilePos(destFileID));
	
	bytes = sizeof(CameraState);
	theError = FSWrite(destFileID, &bytes, localCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSWrite(destFileID, &bytes, topCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSWrite(destFileID, &bytes, frontCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSWrite(destFileID, &bytes, rightCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSWrite(destFileID, &bytes, worldState);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_LowLevelWrite_DataPtr(MetaFile_ReferencePtr reference, long tagNum, Ptr data)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[tagNum];
	
	//Find data
	reference->currentTag = tagNum;
	SetFPos(reference->fileRefNum, fsFromStart, entry->dataPosition);
	
	//Read data
	bytes = entry->dataSize;
	theError = FSWrite(destFileID, &bytes, data);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_Delete(MetaFile_ReferencePtr reference, long tagNum)
{
	OSErr				theError;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[tagNum];
	long				i;
	
	//Remove unused space
	theError = Space_Remove(reference->fileRefNum, entry->dataPosition, entry->dataSize);
	if(theError)
	return theError;
	
	//Correct table entries
	for(i = tagNum + 1; i < reference->tagTable.tagCount; ++i)
	reference->tagTable.tagList[i].dataPosition -= entry->dataSize;
	
	//Remove entry
	BlockMove(&reference->tagTable.tagList[tagNum + 1], &reference->tagTable.tagList[tagNum], (reference->tagTable.tagCount - tagNum) * sizeof(MetaFile_Entry));
	reference->tagTable.tagCount -= 1;
	
	return noErr;
}

OSErr MetaFile_NewFile(FSSpec* destFile, MetaFile_ReferencePtr* newReference)
{
	OSErr					theError;
	short					destFileID;
	MetaFile_ReferencePtr	reference;
	
	//Create file
	theError = FSpCreate(destFile, kMetaFileCreator, kMetaFileType, smSystemScript);
	if(theError)
	return theError;
	theError = FSpOpenDF(destFile, fsRdWrPerm, &destFileID);
	if(theError)
	return theError;
	
	//Skip header size
	SetEOF(destFileID, sizeof(MetaFile_Header));
	SetFPos(destFileID, fsFromStart, sizeof(MetaFile_Header));
	
	//Create reference
	reference = (MetaFile_ReferencePtr) NewPtr(sizeof(MetaFile_Reference) + kMetaFile_MaxTagEntries * sizeof(MetaFile_Entry));
	if(reference == nil)
	return MemError();
	
	//Setup reference
	reference->fileRefNum = destFileID;
	reference->currentDepth = 0;
	reference->tagTable.tagCount = 0;
	*newReference = reference;
	
	return noErr;
}

OSErr MetaFile_Write_OpenFile(FSSpec* destFile, MetaFile_ReferencePtr* newReference)
{
	OSErr					theError;
	short					destFileID;
	MetaFile_ReferencePtr	reference;
	MetaFile_Header			header;
	long					bytes;
	Boolean					targetIsFolder,
							wasAliased;
	FSSpec					realFile;
	
	//Resolve alias
	realFile = *destFile;
	theError = ResolveAliasFile(&realFile, true, &targetIsFolder, &wasAliased);
	if(theError)
	return theError;
	
	//Open file
	theError = FSpOpenDF(&realFile, fsRdWrPerm, &destFileID);
	if(theError)
	return theError;
	
	//Read header
	bytes = sizeof(MetaFile_Header);
	theError = FSRead(destFileID, &bytes, &header);
	if(theError)
	return theError;
	
	//Check version
	if(header.version > kMetaFile_Version) {
		FSClose(destFileID);
		return kError_BadVersion;
	}
	
	//Create reference
	reference = (MetaFile_ReferencePtr) NewPtr(sizeof(MetaFile_Reference) + kMetaFile_MaxTagEntries * sizeof(MetaFile_Entry));
	if(reference == nil)
	return MemError();
	
	//Read tag table
	SetFPos(destFileID, fsFromStart, header.tagTablePosition);
	bytes = header.tagTableSize;
	theError = FSRead(destFileID, &bytes, &reference->tagTable);
	if(theError)
	return theError;
	
	//Setup reference
	reference->fileRefNum = destFileID;
	reference->currentDepth = 0;
	reference->currentTag = 0;
	*newReference = reference;
	
	//Remove tagTable
	SetFPos(destFileID, fsFromStart, header.tagTablePosition);
	theError = SetEOF(destFileID, header.tagTablePosition);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Write_CloseFile(MetaFile_ReferencePtr reference, OSType creatorCode)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	long				tableSize,
						position;
	MetaFile_Header		header;
	
	//Compute table size
	tableSize = sizeof(MetaFile_Table) + reference->tagTable.tagCount * sizeof(MetaFile_Entry);
	
	//Setup header
	GetEOF(destFileID, &position);
	header.creatorCode = creatorCode;
	header.version = kMetaFile_Version;
	header.flags = 0;
	header.tagTablePosition = position;
	header.tagTableSize = tableSize;
	
	//Write table
	SetFPos(destFileID, fsFromStart, position);
	bytes = tableSize;
	theError = FSWrite(destFileID, &bytes, &reference->tagTable);
	if(theError)
	return theError;
	
	//Write file header
	SetFPos(destFileID, fsFromStart, 0);
	bytes = sizeof(MetaFile_Header);
	theError = FSWrite(destFileID, &bytes, &header);
	if(theError)
	return theError;
	
	//Clean up
	FSClose(destFileID);
	DisposePtr((Ptr) reference);
	
	return noErr;
}

void MetaFile_Write_IncreaseDepth(MetaFile_ReferencePtr reference)
{
	reference->currentDepth += 1;
}

void MetaFile_Write_DecreaseDepth(MetaFile_ReferencePtr reference)
{
	reference->currentDepth -= 1;
}
