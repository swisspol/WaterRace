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
#include			"Infinity Audio Engine.h"

//CONSTANTES:

#define				kFromStartTag				0
#define				kFromStartDepth				0

//ROUTINES:

//Utils:

static OSErr SetPos_IndexTag(MetaFile_ReferencePtr reference, OSType tag, long index, long fromTag, long minDepth)
{
	long				num = 0,
						i,
						count = reference->tagTable.tagCount;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[fromTag];
	
	for(i = fromTag; i < count; ++i, ++entry) {
		if(entry->depth < minDepth)
		return kError_ItemNotFound;
		if(entry->tag == tag) {
			++num;
			if(num == index) {
				reference->currentTag = i;
				SetFPos(reference->fileRefNum, fsFromStart, entry->dataPosition);
				return noErr;
			}
		}
	}
	
	return kError_ItemNotFound;
}

static OSErr SetPos_IDTag(MetaFile_ReferencePtr reference, OSType tag, OSType ID, long fromTag, long minDepth)
{
	long				i,
						count = reference->tagTable.tagCount;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[fromTag];
	
	for(i = fromTag; i < count; ++i, ++entry) {
		if(entry->depth < minDepth)
		return kError_ItemNotFound;
		if((entry->tag == tag) && (entry->ID == ID)) {
			reference->currentTag = i;
			SetFPos(reference->fileRefNum, fsFromStart, entry->dataPosition);
			return noErr;
		}
	}
	
	return kError_ItemNotFound;
}

static OSErr SetPos_IndexSubTag(MetaFile_ReferencePtr reference, OSType tag, OSType subTag, long index, long fromTag, long minDepth)
{
	long				num = 0,
						i,
						count = reference->tagTable.tagCount;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[fromTag];
	
	for(i = fromTag; i < count; ++i, ++entry) {
		if(entry->depth < minDepth)
		return kError_ItemNotFound;
		if((entry->tag == tag) && (entry->subTag == subTag)) {
			++num;
			if(num == index) {
				reference->currentTag = i;
				SetFPos(reference->fileRefNum, fsFromStart, entry->dataPosition);
				return noErr;
			}
		}
	}
	
	return kError_ItemNotFound;
}

static OSErr SetPos_IDSubTag(MetaFile_ReferencePtr reference, OSType tag, OSType subTag, OSType id, long fromTag, long minDepth)
{
	long				num = 0,
						i,
						count = reference->tagTable.tagCount;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[fromTag];
	
	for(i = fromTag; i < count; ++i, ++entry) {
		if(entry->depth < minDepth)
		return kError_ItemNotFound;
		if((entry->tag == tag) && (entry->subTag == subTag) && (entry->ID == id)) {
			reference->currentTag = i;
			SetFPos(reference->fileRefNum, fsFromStart, entry->dataPosition);
			return noErr;
		}
	}
	
	return kError_ItemNotFound;
}

static OSErr CountTags(MetaFile_ReferencePtr reference, OSType tag, long fromTag, long minDepth)
{
	long				num = 0,
						i,
						count = reference->tagTable.tagCount;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[fromTag];
	
	for(i = fromTag; i < count; ++i, ++entry) {
		if(entry->depth < minDepth)
		return num;
		if(entry->tag == tag)
		++num;
	}
	
	return num;
}

//Core:

long MetaFile_CountTags(MetaFile_ReferencePtr reference, OSType tag)
{
	return CountTags(reference, tag, kFromStartTag, kFromStartDepth);
}

long MetaFile_CountTags_InsideMegaObject(MetaFile_ReferencePtr reference, OSType modelID, OSType tag)
{
	//Find megaobject
	if(SetPos_IDTag(reference, kTag_MegaObject, modelID, kFromStartTag, kFromStartDepth))
	return 0; //-1
	
	return CountTags(reference, tag, reference->currentTag + 1, reference->tagTable.tagList[reference->currentTag].depth + 1);
}

long MetaFile_CountDataSubTags(MetaFile_ReferencePtr reference, OSType subTag)
{
	long				num = 0,
						i,
						count = reference->tagTable.tagCount;
	MetaFile_EntryPtr	entry = reference->tagTable.tagList;
	
	for(i = 0; i < count; ++i, ++entry)
	if((entry->tag == kTag_Data) && (entry->subTag == subTag))
	++num;
	
	return num;
}

OSErr MetaFile_Read_Data(MetaFile_ReferencePtr reference, OSType* dataType, long index, Ptr* dataPtr, OSType* dataID)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry;
	
	//Find data
	if(*dataType == kSubTag_None) //Find any data
	theError = SetPos_IndexTag(reference, kTag_Data, index, kFromStartTag, kFromStartDepth);
	else { //Find data of a given type
		if(index == kMetaFile_UseID) //Find data of a given ID
		theError = SetPos_IDSubTag(reference, kTag_Data, *dataType, *dataID, kFromStartTag, kFromStartDepth);
		else //Any ID
		theError = SetPos_IndexSubTag(reference, kTag_Data, *dataType, index, kFromStartTag, kFromStartDepth);
	}
	if(theError)
	return theError;
	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*dataPtr = NewPtr(tagEntry->dataSize);
	if(*dataPtr == nil)
	return MemError();
	
	//Read data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *dataPtr);
	if(theError)
	return theError;
	
	//Copy type & ID
	*dataType = tagEntry->subTag;
	*dataID = tagEntry->ID;
	
	return noErr;
}

static OSErr Load_Texture(MetaFile_ReferencePtr reference, CompressedTexturePtr* texture)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*texture = (CompressedTexturePtr) NewPtr(tagEntry->dataSize);
	if(*texture == nil)
	return MemError();
	
	//Read texture data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *texture);
	if(theError)
	return theError;
	
	//Find texture image - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_TextureImage))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*texture)->image = (PicHandle) NewHandle(tagEntry->dataSize);
	if((*texture)->image == nil)
	return MemError();
	
	//Read image
	bytes = tagEntry->dataSize;
	HLock((Handle) (*texture)->image);
	theError = FSRead(destFileID, &bytes, *((*texture)->image));
	HUnlock((Handle) (*texture)->image);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_TextureSet(MetaFile_ReferencePtr reference, TextureSetPtr* set)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	long				i,
						searchTag = reference->currentTag + 1;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*set = (TextureSetPtr) NewPtr(sizeof(TextureSet));
	if(*set == nil)
	return MemError();
	
	//Read texture set data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *set);
	if(theError)
	return theError;
	
	//Load textures
	for(i = 0; i < (*set)->textureCount; ++i) {
		//Find texture
		theError = SetPos_IndexTag(reference, kTag_CompressedTexture, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Load texture
		theError = Load_Texture(reference, &((*set)->textureList[i]));
		if(theError)
		return theError;
	}
	
	return noErr;
}

OSErr MetaFile_Read_TextureSet_ByIndex(MetaFile_ReferencePtr reference, TextureSetPtr* set, OSType modelID, long index)
{
	OSErr				theError;
		
	//Find megaobject
	theError = SetPos_IDTag(reference, kTag_MegaObject, modelID, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	//Find texture set inside current megaobject
	theError = SetPos_IndexTag(reference, kTag_TextureSet, index, reference->currentTag + 1, reference->tagTable.tagList[reference->currentTag].depth + 1);
	if(theError)
	return theError;
	
	return Load_TextureSet(reference, set);
}

OSErr MetaFile_Read_TextureSet_ByID(MetaFile_ReferencePtr reference, TextureSetPtr* set, OSType modelID, OSType setID)
{
	OSErr				theError;
		
	//Find megaobject
	theError = SetPos_IDTag(reference, kTag_MegaObject, modelID, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	//Find texture set inside current megaobject
	theError = SetPos_IDTag(reference, kTag_TextureSet, setID, reference->currentTag + 1, reference->tagTable.tagList[reference->currentTag].depth + 1);
	if(theError)
	return theError;
	
	return Load_TextureSet(reference, set);
}

OSErr MetaFile_Read_ExtractTextureSet(MetaFile_ReferencePtr reference, TextureSetPtr* set, MegaObjectPtr mega)
{
	OSErr				theError;
	long				i,
						j,
						searchTag,
						minDepth;
	Boolean				found;
	OSType				name;
	
	//Find model texture list
	theError = SetPos_IndexTag(reference, kFakeTag_ModelTextureList, 1, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	searchTag = reference->currentTag + 1;
	minDepth = reference->tagTable.tagList[reference->currentTag].depth + 1;
	
	//Allocate memory
	*set = (TextureSetPtr) NewPtrClear(sizeof(TextureSet));
	if(*set == nil)
	return MemError();
	
	//Setup texture set
	BlockMove("\pExtracted texture set", (*set)->name, sizeof(Str63));
	(*set)->id = kNoID;
	(*set)->flags = 0;
	(*set)->textureCount = 0;
	
	for(i = 0; i < mega->object.shapeCount; ++i) {
		//Get texture name
		name = mega->object.shapeList[i]->texture;
		
		//Update set texture table
		(*set)->textureTable[i] = name;
		
		//Check if this shape has a texture
		if(name == kNoTexture)
		continue;
		
		//Check if this texture is already loaded
		found = false;
		for(j = 0; j < (*set)->textureCount; ++j)
			if((*set)->textureList[j]->name == name)
			found = true;
		if(found)
		continue;
		
		//Look for the texture
		theError = SetPos_IDTag(reference, kTag_CompressedTexture, name, searchTag, minDepth);
		if(theError)
		return theError;
		
		//Load texture
		theError = Load_Texture(reference, &((*set)->textureList[(*set)->textureCount]));
		if(theError)
		return theError;
		
		(*set)->textureCount += 1;
	}
	
	return noErr;
}

static OSErr Load_CameraPosition(MetaFile_ReferencePtr reference, CameraPositionPtr camera)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Read camera position data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, camera);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Read_CameraPosition_ByIndex(MetaFile_ReferencePtr reference, CameraPositionPtr camera, long index)
{
	OSErr				theError;
		
	//Find camera position
	theError = SetPos_IndexTag(reference, kTag_CameraPosition, index, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	return Load_CameraPosition(reference, camera);
}

static OSErr Load_View(MetaFile_ReferencePtr reference, ViewPtr* view)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*view = (ViewPtr) NewPtr(sizeof(View));
	if(*view == nil)
	return MemError();
	
	//Read camera position data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *view);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Read_View_ByIndex(MetaFile_ReferencePtr reference, ViewPtr* view, long index)
{
	OSErr				theError;
		
	//Find camera position
	theError = SetPos_IndexTag(reference, kTag_ViewSetup, index, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	return Load_View(reference, view);
}

OSErr MetaFile_Read_GetModelDefaultTextureSet(MetaFile_ReferencePtr reference, TextureSetPtr* set, MegaObjectPtr mega)
{
	long			setCount;
	OSErr			theError;
	
	//Count texture sets
	setCount = MetaFile_CountTags_InsideMegaObject(reference, mega->object.id, kTag_TextureSet);
	
	//Does the megaobject contains texture sets?
	if(setCount > 0) {
		//Load the first one
		theError = MetaFile_Read_TextureSet_ByIndex(reference, set, mega->object.id, 1);
	}
	else {
		//Create a new one
		theError = MetaFile_Read_ExtractTextureSet(reference, set, mega);
	}
	
	return theError;
}

static OSErr Load_Shape(MetaFile_ReferencePtr reference, ShapePtr* shape)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*shape = (ShapePtr) NewPtr(sizeof(Shape));
	if(*shape == nil)
	return MemError();
	
	//Read shape data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *shape);
	if(theError)
	return theError;
	
	//Find point list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_PointList)) {
		(*shape)->pointList = nil; //return kError_ItemNotFound;
		--tagEntry;
	}
	else {
		reference->currentTag += 1;
		SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
		
		//Allocate memory
		(*shape)->pointList = (VertexPtr) NewPtr(tagEntry->dataSize);
		if((*shape)->pointList == nil)
		return MemError();
		
		//Read points
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, (*shape)->pointList);
		if(theError)
		return theError;
	}
	
	//Find triangle list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_TriangleList)) {
		(*shape)->triangleList = nil; //return kError_ItemNotFound;
		--tagEntry;
	}
	else {
		reference->currentTag += 1;
		SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
		
		//Allocate memory
		(*shape)->triangleList = (TriFacePtr) NewPtr(tagEntry->dataSize);
		if((*shape)->triangleList == nil)
		return MemError();
		
		//Read triangles
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, (*shape)->triangleList);
		if(theError)
		return theError;
	}
	
	//Check for normals
	if((*shape)->normalMode != kNoNormals) {
		//Find normal list - we suppose it is the next tag entry
		++tagEntry;
		if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_NormalList))
		return kError_ItemNotFound;
		reference->currentTag += 1;
		SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
		
		//Allocate memory
		(*shape)->normalList = (VectorPtr) NewPtr(tagEntry->dataSize);
		if((*shape)->normalList == nil)
		return MemError();
	
		//Read normals
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, (*shape)->normalList);
		if(theError)
		return theError;
	}
	else
	(*shape)->normalList = nil;
	
	//Find custom data - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_ShapeData)) {
		(*shape)->dataPtr = nil; //return kError_ItemNotFound;
		--tagEntry;
	}
	else {
		reference->currentTag += 1;
		SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
		
		//Allocate memory
		(*shape)->dataPtr = NewPtr(tagEntry->dataSize);
		if((*shape)->dataPtr == nil)
		return MemError();
		
		//Read triangles
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, (*shape)->dataPtr);
		if(theError)
		return theError;
	}
	
	return noErr;
}

static OSErr Load_Script(MetaFile_ReferencePtr reference, ScriptPtr* script)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	long				i,
						searchTag = reference->currentTag;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*script = (ScriptPtr) NewPtr(sizeof(Script));
	if(*script == nil)
	return MemError();
	
	//Read script data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *script);
	if(theError)
	return theError;
	
	//Read animations
	for(i = 0; i < (*script)->animationCount; ++i) {
		//Find animation - we suppose it is the next tag entry
		++tagEntry;
		if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_Animation))
		return kError_ItemNotFound;
		reference->currentTag += 1;
		SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
		//Allocate memory
		(*script)->animationList[i] = (AnimationPtr) NewPtr(tagEntry->dataSize);
		if((*script)->animationList[i] == nil)
		return MemError();
	
		//Read animation
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, (*script)->animationList[i]);
		if(theError)
		return theError;
	}
	
	return noErr;
}

static OSErr Load_Skeleton(MetaFile_ReferencePtr reference, SkeletonPtr* skeleton)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*skeleton = (SkeletonPtr) NewPtr(tagEntry->dataSize);
	if(*skeleton == nil)
	return MemError();
	
	//Read skeleton data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *skeleton);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_Camera(MetaFile_ReferencePtr reference, CameraPtr* camera)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*camera = (CameraPtr) NewPtr(sizeof(Camera));
	if(*camera == nil)
	return MemError();
	
	//Read camera data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *camera);
	if(theError)
	return theError;
	
	//Check version
	if(tagEntry->version == 0x0100)
	(*camera)->visibility = kVisible_Easy | kVisible_Medium | kVisible_Hard;
	
	return noErr;
}

typedef struct {
	Object			object;
	
	long			skeletonCount;
	long			scriptCount;
	long			cameraCount;
} MetaFile_3DModel;
typedef MetaFile_3DModel* MetaFile_3DModelPtr;

typedef struct {
	Vector			center;
	Vector			xVector,
					yVector,
					zVector;
	
	long			unused;
} OLD_CollisionBox;

static OSErr Load_MegaObject(MetaFile_ReferencePtr reference, MegaObjectPtr mega, long skipFlags)
{
	OSErr				theError;
	long				i,
						searchTag = reference->currentTag + 1;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	long				bytes;
	short				destFileID = reference->fileRefNum;
	MetaFile_3DModel	model;
	OLD_CollisionBox	boxes[kMega_MaxCollisionBoxes];
	
	//Check version
	if(tagEntry->version == 0x0100) {
		//OLD FORLMAT: Read megaobject data directly
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, mega);
		if(theError)
		return theError;
		
		//Update data
		mega->boxCount = 0;
	}
	else if(tagEntry->version == 0x0200) {
		//NEW FORMAT: Read model 3D
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, &model);
		if(theError)
		return theError;
		
		//Setup MegaObject structure
		mega->object = model.object;
		mega->skeletonCount = model.skeletonCount;
		mega->scriptCount = model.scriptCount;
		mega->cameraCount = model.cameraCount;
		
		//Update data
		mega->boxCount = 0;
	}
	else {
		//NEW FORMAT: Read megaobject data directly
		bytes = tagEntry->dataSize;
		theError = FSRead(destFileID, &bytes, mega);
		if(theError)
		return theError;
	}
	
	//Convert collision boxes
	if(tagEntry->version == 0x0300) {
		if(mega->boxCount > kMega_MaxCollisionBoxes)
		mega->boxCount = kMega_MaxCollisionBoxes;
		BlockMove(&mega->boxList, boxes, sizeof(boxes));
		for(i = 0; i < mega->boxCount; ++i) {
			mega->boxList[i].center = boxes[i].center;
			mega->boxList[i].xVector = boxes[i].xVector;
			mega->boxList[i].yVector = boxes[i].yVector;
			mega->boxList[i].zVector = boxes[i].zVector;
		}
	}
	
	//Read shapes
	for(i = 0; i < mega->object.shapeCount; ++i) {
		//Find shape
		theError = SetPos_IndexTag(reference, kTag_Shape, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read shape
		theError = Load_Shape(reference, &mega->object.shapeList[i]);
		if(theError)
		return theError;
	}
	
	//Read skeletons
	if(skipFlags & kMetaFile_SkipSkeletons)
	mega->skeletonCount = 0;
	else
	for(i = 0; i < mega->skeletonCount; ++i) {
		//Find skeleton
		theError = SetPos_IndexTag(reference, kTag_Skeleton, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read skeleton
		theError = Load_Skeleton(reference, &mega->skeletonList[i]);
		if(theError)
		return theError;
	}
	
	//Read scripts
	if(skipFlags & kMetaFile_SkipScripts)
	mega->scriptCount = 0;
	else
	for(i = 0; i < mega->scriptCount; ++i) {
		//Find script
		theError = SetPos_IndexTag(reference, kTag_Script, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read script
		theError = Load_Script(reference, &mega->scriptList[i]);
		if(theError)
		return theError;
	}
	
	//Read cameras
	if(skipFlags & kMetaFile_SkipCameras)
	mega->cameraCount = 0;
	else
	for(i = 0; i < mega->cameraCount; ++i) {
		//Find camera
		theError = SetPos_IndexTag(reference, kTag_Camera, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read camera
		theError = Load_Camera(reference, &mega->cameraList[i]);
		if(theError)
		return theError;
	}
	
	return noErr;
}

OSErr MetaFile_Read_MegaObject_ByIndex(MetaFile_ReferencePtr reference, MegaObjectPtr mega, long index, long skipFlags)
{
	OSErr				theError;
	
	//Find data
	theError = SetPos_IndexTag(reference, kTag_MegaObject, index, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	return Load_MegaObject(reference, mega, skipFlags);
}

OSErr MetaFile_Read_TextureList(MetaFile_ReferencePtr reference, CompressedTexturePtr textureList[], long* textureCount, OSType tag)
{
	OSErr				theError;
	long				i,
						searchTag,
						minDepth;
	short				destFileID = reference->fileRefNum;
	
	//Find texture list
	theError = SetPos_IndexTag(reference, tag, 1, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	searchTag = reference->currentTag + 1;
	minDepth = reference->tagTable.tagList[reference->currentTag].depth + 1;
	
	//Count textures
	*textureCount = CountTags(reference, kTag_CompressedTexture, searchTag, minDepth);
	
	//Load textures
	for(i = 0; i < *textureCount; ++i) {
		//Find texture
		theError = SetPos_IndexTag(reference, kTag_CompressedTexture, 1 + i, searchTag, minDepth);
		if(theError)
		return theError;
		
		//Read texture
		theError = Load_Texture(reference, &textureList[i]);
		if(theError)
		return theError;
	}
	
	return noErr;
}

static OSErr Load_TerrainMesh(MetaFile_ReferencePtr reference, TerrainMeshPtr* mesh)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*mesh = (TerrainMeshPtr) NewPtr(sizeof(TerrainMesh));
	if(*mesh == nil)
	return MemError();
	
	//Read mesh data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *mesh);
	if(theError)
	return theError;
	
	//Find point list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_PointList))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*mesh)->pointList = (VertexPtr) NewPtr(tagEntry->dataSize);
	if((*mesh)->pointList == nil)
	return MemError();
	
	//Read points
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, (*mesh)->pointList);
	if(theError)
	return theError;
	
	//Find lightning list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_LightningList))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*mesh)->lightningList = (LightningPtr) NewPtr(tagEntry->dataSize);
	if((*mesh)->lightningList == nil)
	return MemError();
	
	//Read lightning
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, (*mesh)->lightningList);
	if(theError)
	return theError;
	
	//Find triangle list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_TriangleList))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*mesh)->triangleList = (TriFacePtr) NewPtr(tagEntry->dataSize);
	if((*mesh)->triangleList == nil)
	return MemError();
	
	//Read triangles
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, (*mesh)->triangleList);
	if(theError)
	return theError;
	
	//Find normal list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_NormalList))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*mesh)->normalList = (VectorPtr) NewPtr(tagEntry->dataSize);
	if((*mesh)->normalList == nil)
	return MemError();

	//Read normals
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, (*mesh)->normalList);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_SeaMesh(MetaFile_ReferencePtr reference, SeaMeshPtr* mesh)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*mesh = (SeaMeshPtr) NewPtr(sizeof(SeaMesh));
	if(*mesh == nil)
	return MemError();
	
	//Read mesh data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *mesh);
	if(theError)
	return theError;
	
	//Find point list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_PointList))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*mesh)->pointList = (VertexPtr) NewPtr(tagEntry->dataSize);
	if((*mesh)->pointList == nil)
	return MemError();
	
	//Read points
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, (*mesh)->pointList);
	if(theError)
	return theError;
	
	//Find lightning list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_LightningList))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*mesh)->lightningList = (LightningPtr) NewPtr(tagEntry->dataSize);
	if((*mesh)->lightningList == nil)
	return MemError();
	
	//Read lightning
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, (*mesh)->lightningList);
	if(theError)
	return theError;
	
	//Find triangle list - we suppose it is the next tag entry
	++tagEntry;
	if((tagEntry->tag != kTag_IncompleteData) || (tagEntry->subTag != kSubTag_TriangleList))
	return kError_ItemNotFound;
	reference->currentTag += 1;
	SetFPos(destFileID, fsFromStart, tagEntry->dataPosition);
	
	//Allocate memory
	(*mesh)->triangleList = (TriFacePtr) NewPtr(tagEntry->dataSize);
	if((*mesh)->triangleList == nil)
	return MemError();
	
	//Read triangles
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, (*mesh)->triangleList);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_AmbientSound(MetaFile_ReferencePtr reference, AmbientSoundPtr* sound)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*sound = (AmbientSoundPtr) NewPtr(tagEntry->dataSize);
	if(*sound == nil)
	return MemError();
	
	//Read sound data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *sound);
	if(theError)
	return theError;
	
	return noErr;
} 

static OSErr Load_ModelAnimation(MetaFile_ReferencePtr reference, ModelAnimationPtr* path)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*path = (ModelAnimationPtr) NewPtr(tagEntry->dataSize);
	if(*path == nil)
	return MemError();
	
	//Read path data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *path);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_Track(MetaFile_ReferencePtr reference, RaceTrackPtr* track)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*track = (RaceTrackPtr) NewPtr(tagEntry->dataSize);
	if(*track == nil)
	return MemError();
	
	//Read track data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *track);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_Enclosure(MetaFile_ReferencePtr reference, CollisionEnclosurePtr* enclosure)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*enclosure = (CollisionEnclosurePtr) NewPtr(tagEntry->dataSize);
	if(*enclosure == nil)
	return MemError();
	
	//Read track data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *enclosure);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_ItemList(MetaFile_ReferencePtr reference, SpecialItemReferencePtr* itemList)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*itemList = (SpecialItemReferencePtr) NewPtr(tagEntry->dataSize);
	if(*itemList == nil)
	return MemError();
	
	//Read track data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, *itemList);
	if(theError)
	return theError;
	
	return noErr;
}

static OSErr Load_Terrain(MetaFile_ReferencePtr reference, TerrainPtr terrain, long skipFlags)
{
	OSErr				theError;
	long				bytes,
						i,
						searchTag = reference->currentTag + 1;
	short				destFileID = reference->fileRefNum;
	MetaFile_EntryPtr	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Read terrain data
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, terrain);
	if(theError)
	return theError;
	
	//Check version
	if(tagEntry->version > kVersion_Terrain)
	return kError_BadVersion;
	if(tagEntry->version <= 0x0100) {
		terrain->trackCount = 0;
	}
	if(tagEntry->version <= 0x0101) {
		terrain->itemReferenceCount = 0;
		terrain->specialItemCount = 0;
	}
	if(tagEntry->version <= 0x0102) {
		for(i = 0; i < terrain->sourceCount; ++i)
		if(terrain->sourceList[i].sourceType == kSource_Sound) {
			terrain->sourceList[i].volume = kInfinityAudio_MaxVolume;
			terrain->sourceList[i].pitch = kInfinityAudio_NormalPitch;
		}
	}
	if(tagEntry->version <= 0x0103) {
		terrain->enclosureCount = 0;
	}
	
	//Read terrain meshes
	for(i = 0; i < terrain->meshCount; ++i) {
		//Find mesh
		theError = SetPos_IndexTag(reference, kTag_TerrainMesh, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read mesh
		theError = Load_TerrainMesh(reference, &terrain->meshList[i]);
		if(theError)
		return theError;
	}
	for(i = 0; i < terrain->seaCount; ++i) {
		//Find mesh
		theError = SetPos_IndexTag(reference, kTag_SeaMesh, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read sea mesh
		theError = Load_SeaMesh(reference, &terrain->seaList[i]);
		if(theError)
		return theError;
	}
	
	//Read camera list
	if(skipFlags & kMetaFile_SkipCameras)
	terrain->cameraCount = 0;
	else
	for(i = 0; i < terrain->cameraCount; ++i) {
		//Find camera
		theError = SetPos_IndexTag(reference, kTag_Camera, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read camera
		theError = Load_Camera(reference, &terrain->cameraList[i]);
		if(theError)
		return theError;
	}
	
	//Read ambient sound list
	if(skipFlags & kMetaFile_SkipAmbientSounds)
	terrain->soundCount = 0;
	else
	for(i = 0; i < terrain->soundCount; ++i) {
		//Find sound
		theError = SetPos_IndexTag(reference, kTag_AmbientSound, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read sound
		theError = Load_AmbientSound(reference, &terrain->soundList[i]);
		if(theError)
		return theError;
	}
	
	//Read animation list
	if(skipFlags & kMetaFile_SkipAnimations)
	terrain->animCount = 0;
	else
	for(i = 0; i < terrain->animCount; ++i) {
		//Find path
		theError = SetPos_IndexTag(reference, kTag_ModelAnimation, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read path
		theError = Load_ModelAnimation(reference, &terrain->animList[i]);
		if(theError)
		return theError;
	}
	
	//Read track list
	if(skipFlags & kMetaFile_SkipTracks)
	terrain->trackCount = 0;
	else
	for(i = 0; i < terrain->trackCount; ++i) {
		//Find track
		theError = SetPos_IndexTag(reference, kTag_Track, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read track
		theError = Load_Track(reference, &terrain->trackList[i]);
		if(theError)
		return theError;
	}
	
	//Read special items list
	if(skipFlags & kMetaFile_SkipSpecialItems)
	terrain->itemReferenceCount = 0;
	else if(terrain->itemReferenceCount) {
		//Find item list
		theError = SetPos_IndexTag(reference, kTag_ItemList, 1, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read item list
		theError = Load_ItemList(reference, &terrain->itemReferenceList);
		if(theError)
		return theError;
	}
	
	//Read track list
	if(skipFlags & kMetaFile_SkipEnclosures)
	terrain->enclosureCount = 0;
	else
	for(i = 0; i < terrain->enclosureCount; ++i) {
		//Find track
		theError = SetPos_IndexTag(reference, kTag_Enclosure, 1 + i, searchTag, kFromStartDepth);
		if(theError)
		return theError;
		
		//Read track
		theError = Load_Enclosure(reference, &terrain->enclosureList[i]);
		if(theError)
		return theError;
	}
	
	return noErr;
}

OSErr MetaFile_Read_Terrain(MetaFile_ReferencePtr reference, TerrainPtr terrain, long skipFlags)
{
	OSErr				theError;
	
	//Find data
	theError = SetPos_IndexTag(reference, kTag_Terrain, 1, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	return Load_Terrain(reference, terrain, skipFlags);
}

OSErr MetaFile_Read_MegaObjectNameID_ByIndex(MetaFile_ReferencePtr reference, Str63 name, OSType* id, long index)
{
	OSErr				theError;
	MegaObject			tempObject;
	long				bytes;
	MetaFile_EntryPtr	tagEntry;
	short				destFileID = reference->fileRefNum;
	
	//Find data
	theError = SetPos_IndexTag(reference, kTag_MegaObject, index, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	//Read megaobject data
	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, &tempObject);
	if(theError)
	return theError;
	
	BlockMove(tempObject.object.name, name, sizeof(Str63));
	*id = tempObject.object.id;
	
	return noErr;
}

OSErr MetaFile_Read_TerrainNameID_ByIndex(MetaFile_ReferencePtr reference, Str63 name, OSType* id, long index)
{
	OSErr				theError;
	Terrain				tempTerrain;
	long				bytes;
	MetaFile_EntryPtr	tagEntry;
	short				destFileID = reference->fileRefNum;
	
	//Find data
	theError = SetPos_IndexTag(reference, kTag_Terrain, index, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	//Read terrain data
	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, &tempTerrain);
	if(theError)
	return theError;
	
	BlockMove(tempTerrain.name, name, sizeof(Str63));
	*id = tempTerrain.id;
	
	return noErr;
}

OSErr MetaFile_Read_TextureSetName_ByIndex(MetaFile_ReferencePtr reference, OSType modelID, Str63 name, long index)
{
	OSErr				theError;
	TextureSet			tempSet;
	long				bytes;
	MetaFile_EntryPtr	tagEntry;
	short				destFileID = reference->fileRefNum;
	
	//Find megaobject
	theError = SetPos_IDTag(reference, kTag_MegaObject, modelID, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	
	//Find texture set inside current megaobject
	theError = SetPos_IndexTag(reference, kTag_TextureSet, index, reference->currentTag + 1, reference->tagTable.tagList[reference->currentTag].depth + 1);
	if(theError)
	return theError;
	
	//Read texture set data
	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	bytes = tagEntry->dataSize;
	theError = FSRead(destFileID, &bytes, &tempSet);
	if(theError)
	return theError;
	
	BlockMove(tempSet.name, name, sizeof(Str63));
	
	return noErr;
}

OSErr MetaFile_Read_Preview(MetaFile_ReferencePtr reference, PicHandle* previewPic)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry;
	
	//Find data
	theError = SetPos_IndexTag(reference, kTag_Preview, 1, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Allocate memory
	*previewPic = (PicHandle) NewHandle(tagEntry->dataSize);
	if(*previewPic == nil)
	return MemError();
	
	//Read preview data
	bytes = tagEntry->dataSize;
	HLock((Handle) *previewPic);
	theError = FSRead(destFileID, &bytes, **previewPic);
	HUnlock((Handle) *previewPic);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Read_CameraLayout(MetaFile_ReferencePtr reference, CameraStatePtr localCamera, CameraStatePtr topCamera, CameraStatePtr frontCamera, CameraStatePtr rightCamera, CameraStatePtr worldState)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	tagEntry;
	
	//Find camera layout data
	theError = SetPos_IndexTag(reference, kTag_CameraLayout, 1, kFromStartTag, kFromStartDepth);
	if(theError)
	return theError;
	tagEntry = &reference->tagTable.tagList[reference->currentTag];
	
	//Check size
	if(tagEntry->dataSize != 5 * sizeof(CameraState))
	return kError_BadVersion;
	
	//Read camera data
	bytes = sizeof(CameraState);
	theError = FSRead(destFileID, &bytes, localCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSRead(destFileID, &bytes, topCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSRead(destFileID, &bytes, frontCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSRead(destFileID, &bytes, rightCamera);
	if(theError)
	return theError;
	
	bytes = sizeof(CameraState);
	theError = FSRead(destFileID, &bytes, worldState);
	if(theError)
	return theError;
	
	return noErr;
}

#if 0
OSErr MetaFile_LowLevelRead_DataPtr(MetaFile_ReferencePtr reference, long tagNum, Ptr data)
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
	theError = FSRead(destFileID, &bytes, data);
	if(theError)
	return theError;
	
	return noErr;
}
#else
OSErr MetaFile_LowLevelRead_DataPtr(MetaFile_ReferencePtr reference, long tagNum, Ptr* data)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[tagNum];
	
	//Find data
	reference->currentTag = tagNum;
	SetFPos(reference->fileRefNum, fsFromStart, entry->dataPosition);
	
	//Allocate memory
	*data = NewPtr(entry->dataSize);
	if(*data == nil)
	return MemError();
	
	//Read data
	bytes = entry->dataSize;
	theError = FSRead(destFileID, &bytes, *data);
	if(theError)
	return theError;
	
	return noErr;
}
#endif

OSErr MetaFile_LowLevelRead_DataHandle(MetaFile_ReferencePtr reference, long tagNum, Handle* data)
{
	short				destFileID = reference->fileRefNum;
	long				bytes;
	OSErr				theError;
	MetaFile_EntryPtr	entry = &reference->tagTable.tagList[tagNum];
	
	//Find data
	reference->currentTag = tagNum;
	SetFPos(reference->fileRefNum, fsFromStart, entry->dataPosition);
	
	//Allocate memory
	*data = NewHandle(entry->dataSize);
	if(*data == nil)
	return MemError();
	
	//Read data
	bytes = entry->dataSize;
	HLock(*data);
	theError = FSRead(destFileID, &bytes, **data);
	HUnlock(*data);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr MetaFile_Read_OpenFile(FSSpec* destFile, MetaFile_ReferencePtr* newReference)
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
	theError = FSpOpenDF(&realFile, fsRdPerm, &destFileID);
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
	reference = (MetaFile_ReferencePtr) NewPtr(sizeof(MetaFile_Reference) + header.tagTableSize);
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
	
	//Reset to beginning of data
	SetFPos(destFileID, fsFromStart, sizeof(MetaFile_Header));
	
	return noErr;
}

OSErr MetaFile_Read_CloseFile(MetaFile_ReferencePtr reference)
{
	short				destFileID = reference->fileRefNum;
	
	//Clean up
	FSClose(destFileID);
	DisposePtr((Ptr) reference);
	
	return noErr;
}

PicHandle MetaFile_ExtractPreview(FSSpec* theFile)
{
	MetaFile_ReferencePtr	reference;
	OSErr					theError;
	PicHandle				thePic;
	
	theError = MetaFile_Read_OpenFile(theFile, &reference);
	if(theError)
	return nil;
	
	theError = MetaFile_Read_Preview(reference, &thePic);
	MetaFile_Read_CloseFile(reference);
	if(theError)
	return nil;
	
	return thePic;
}
