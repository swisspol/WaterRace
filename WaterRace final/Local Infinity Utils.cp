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
#include				"Pilots.h"
#include				"Game.h"
#include				"Data Files.h"
#include				"Play Modes.h"

#include				"Quaternion.h"
#include				"Vector.h"
#include				"Infinity Rendering.h"
#include				"MetaFile Utils.h"

#include				"Terrain Utils.h"
#include				"Enclosure Utils.h"

//CONSTANTES:

#define					kTerrainPreviewImageID		'Prev'

//ROUTINES:

static MegaObjectPtr GetMegaObjectFromID(OSType id)
{
	long				i;
	
	for(i = 0; i < modelCount; ++i)
	if(modelList[i].megaObject.object.id == id)
	return &modelList[i].megaObject;

	return nil;
}

SpecialItemPtr GetSpecialItemFromID(TerrainPtr terrain, OSType id)
{
	long				i;
	
	for(i = 0; i < terrain->specialItemCount; ++i)
	if(terrain->specialItemList[i].id == id)
	return &terrain->specialItemList[i];

	return nil;
}

inline TQATexture* GetTQATexturePtr(OSType texID, TQATexture* textureList[], OSType nameList[], long textureCount)
{
	long				i;
	
	for(i = 0; i < textureCount; ++i)
	if(nameList[i] == texID)
	return textureList[i];
	
	return nil;
}

static void Anim_UpdateLength(TerrainPtr terrain)
{
	long				length = 0,
						i,
						j;
	KeyFramePtr			timeEvent;
	
	for(i = 0; i < terrain->animCount; ++i) {
		timeEvent = terrain->animList[i]->frameList;
		for(j = 0; j < terrain->animList[i]->frameCount; ++j, ++timeEvent) {
			if(timeEvent->time > length)
			length = timeEvent->time;
		}
	}
	terrain->animLength = length;
}

inline void Get_CheckPosition(CheckPtr check, VectorPtr center)
{
	center->x = (check->rightMark.x + check->leftMark.x) / 2.0;
	center->y = (check->rightMark.y + check->leftMark.y) / 2.0;
	center->z = (check->rightMark.z + check->leftMark.z) / 2.0;
}

void Terrain_Resolve(TerrainPtr terrain, TextureMemoryPtr vram, long visibilityMask, OSType flyThroughID)
{
	long					i,
							j,
							k;
	ModelAnimationPtr		animation;
	KeyFramePtr				timeEvent;
								
	//Remove animation according to visibility - KEEP FLY-THROUGH
	for(i = 0; i < terrain->animCount; ++i) {
		if(((terrain->animList[i]->modelID == kFlyThroughID) && (terrain->animList[i]->flyID != flyThroughID))
		 || !(terrain->animList[i]->visibility & visibilityMask)) {
			Path_Dispose(terrain->animList[i]);
			for(j = i + 1; j < terrain->animCount; ++j)
			terrain->animList[j - 1] = terrain->animList[j];
			--terrain->animCount;
			--i;
		}
	}
	 
	Anim_UpdateLength(terrain);
	
	//Remove cameras according to visibility
	for(i = 0; i < terrain->cameraCount; ++i)
	if(!(terrain->cameraList[i]->visibility & visibilityMask)) {
		Camera_Dispose(terrain->cameraList[i]);
		for(j = i + 1; j < terrain->cameraCount; ++j)
		terrain->cameraList[j - 1] = terrain->cameraList[j];
		--terrain->cameraCount;
		--i;
	}
	
	//Remove sound sources according to visibility
	for(i = 0; i < terrain->sourceCount; ++i)
	if(!(terrain->sourceList[i].visibility & visibilityMask)) {
		for(j = i + 1; j < terrain->sourceCount; ++j)
		terrain->sourceList[j - 1] = terrain->sourceList[j];
		--terrain->sourceCount;
		--i;
	}
	
	//Resolve terrain meshes references according to visibility
	for(i = 0; i < terrain->meshCount; ++i)
	for(j = 0; j < terrain->meshList[i]->referenceCount; ++j)
	if(!(terrain->meshList[i]->referenceList[j].visibility & visibilityMask)) {
		for(k = j + 1; k < terrain->meshList[i]->referenceCount; ++k)
		terrain->meshList[i]->referenceList[k - 1] = terrain->meshList[i]->referenceList[k];
		--terrain->meshList[i]->referenceCount;
		--j;
	}
	
	//Resolve sea meshes references according to visibility
	for(i = 0; i < terrain->seaCount; ++i)
	for(j = 0; j < terrain->seaList[i]->referenceCount; ++j)
	if(!(terrain->seaList[i]->referenceList[j].visibility & visibilityMask)) {
		for(k = j + 1; k < terrain->seaList[i]->referenceCount; ++k)
		terrain->seaList[i]->referenceList[k - 1] = terrain->seaList[i]->referenceList[k];
		--terrain->seaList[i]->referenceCount;
		--j;
	}
		
	//Resolve terrain meshes textures
	for(i = 0; i < terrain->meshCount; ++i)
	terrain->meshList[i]->texturePtr = GetTQATexturePtr(terrain->meshList[i]->texture, vram->textureList, vram->nameList, vram->textureCount);
	
	//Resolve sea meshes textures
	for(i = 0; i < terrain->seaCount; ++i)
	terrain->seaList[i]->texturePtr = GetTQATexturePtr(terrain->seaList[i]->texture, vram->textureList, vram->nameList, vram->textureCount);
	
	//Resolve terrain meshes references
	for(i = 0; i < terrain->meshCount; ++i)
	for(j = 0; j < terrain->meshList[i]->referenceCount; ++j)
	terrain->meshList[i]->referenceList[j].id = (unsigned long) GetMegaObjectFromID(terrain->meshList[i]->referenceList[j].id);
	
	//Resolve sea meshes references
	for(i = 0; i < terrain->seaCount; ++i)
	for(j = 0; j < terrain->seaList[i]->referenceCount; ++j)
	terrain->seaList[i]->referenceList[j].id = (unsigned long) GetMegaObjectFromID(terrain->seaList[i]->referenceList[j].id);
	
	//Resolve ambient sounds
	for(i = 0; i < terrain->sourceCount; ++i)
	if(terrain->sourceList[i].sourceType == kSource_Sound) {
		for(j = 0; j < terrain->soundCount; ++j)
		if(terrain->soundList[j]->id == terrain->sourceList[i].soundID)
		terrain->sourceList[i].soundID = (unsigned long) terrain->soundList[j];
	}
	
	//Resolve animation models
	for(i = 0; i < terrain->animCount; ++i)
	if(terrain->animList[i]->modelID != kFlyThroughID)
	terrain->animList[i]->modelID = (unsigned long) GetMegaObjectFromID(terrain->animList[i]->modelID);
	
	//Precalculate animation quaternions
	for(i = 0; i < terrain->animCount; ++i) {
		animation = terrain->animList[i];
		timeEvent = animation->frameList;
		for(j = 0; j < animation->frameCount; ++j, ++timeEvent)
		Quaternion_EulerToQuaternion(-timeEvent->rotateX, -timeEvent->rotateY, -timeEvent->rotateZ, (QuaternionPtr) &timeEvent->rotateX);
	}
	
	//Resolve bonus models references according to visibility
	if(terrain->itemReferenceCount)
	switch(gameConfig.gameMode) {
		
		case kGameMode_Local:
		PlayMode_FilterTerrainItems(gameConfig.gameSubMode, terrain, visibilityMask);
		break;
		
		case kGameMode_Practice:
		{
			for(i = 0; i < terrain->itemReferenceCount; ++i)
			if(!(terrain->itemReferenceList[i].visibility & visibilityMask)) {
				for(j = i + 1; j < terrain->itemReferenceCount; ++j)
				terrain->itemReferenceList[j - 1] = terrain->itemReferenceList[j];
				--terrain->itemReferenceCount;
				--i;
			}
			if(terrain->itemReferenceCount == 0)
			ItemList_Dispose(terrain->itemReferenceList);
		}
		break;
		
		case kGameMode_Join:
		case kGameMode_Host:
		{
			for(i = 0; i < terrain->itemReferenceCount; ++i)
			if(!(terrain->itemReferenceList[i].visibility & visibilityMask) || (GetSpecialItemFromID(terrain, terrain->itemReferenceList[i].itemID)->itemType != kItem_Turbo)) {
				for(j = i + 1; j < terrain->itemReferenceCount; ++j)
				terrain->itemReferenceList[j - 1] = terrain->itemReferenceList[j];
				--terrain->itemReferenceCount;
				--i;
			}
			if(terrain->itemReferenceCount == 0)
			ItemList_Dispose(terrain->itemReferenceList);
		}
		break;
		
		case kGameMode_BroadCastTV:
		case kGameMode_Replay:
		case kGameMode_Demo:
		{
			terrain->itemReferenceCount = 0;
			ItemList_Dispose(terrain->itemReferenceList);
		}
		break;
		
	}
	
	//Resolve bonus models
	for(i = 0; i < terrain->specialItemCount; ++i)
	terrain->specialItemList[i].modelID = (unsigned long) GetMegaObjectFromID(terrain->specialItemList[i].modelID);
	
	//Resolve bonus models references
	for(i = 0; i < terrain->itemReferenceCount; ++i)
	terrain->itemReferenceList[i].itemID = (unsigned long) GetSpecialItemFromID(terrain, terrain->itemReferenceList[i].itemID);
	
	//Prepare enclosure collision detection
	for(i = 0; i < terrain->enclosureCount; ++i)
	Enclosure_PreProcess(terrain->enclosureList[i]);
}

OSErr Terrain_ExtractPreview(FSSpec* theFile, PicHandle* daPic)
{
	MetaFile_ReferencePtr		reference;
	OSType						dataType = kSubTag_Image,
								dataID = kTerrainPreviewImageID;
	Ptr							previewData;
	OSErr						theError;
	
	//Open MetaFile
	theError = MetaFile_Read_OpenFile(theFile, &reference);
	if(theError)
	return theError;
	
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, &previewData, &dataID);
	if(theError == noErr) {
		theError = PtrToHand(previewData, (Handle*) daPic, GetPtrSize(previewData));
		DisposePtr(previewData);
	}
	
	//Close MetaFile
	MetaFile_Read_CloseFile(reference);
	
	return theError;
}
