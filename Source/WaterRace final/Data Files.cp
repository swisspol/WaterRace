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


#include				"WaterRace.h"
#include				"Data Files.h"

#include				"Folder Utils.h"
#include				"MetaFile Utils.h"

//VARIABLES:

FSSpec					dataFolder,
						shipsFolder,
						locationsFolder,
						charactersFolder,
						pilotsFolder,
						coreFolder;
#if !__DEMO_MODE__
FSSpec					addOnsFolder;
#endif
#if __USE_AUDIO_FILES__ || __USE_AUDIO_CD_TRACKS__
FSSpec					musicsFolder;
#endif
						
short					interfaceResFileID,
						soundResFileID;

DataSet					coreData,
						addOnData;
ShipPreloadedData		leahShipData;
TerrainPreloadedData	leahTerrainData,
						practiceTerrainData;
CharacterPreloadedData	leahCharacterData;

//ROUTINES:

OSErr Resolve_DataFolders()
{
	CInfoPBRec				cipbr;
	HFileInfo				*fpb = (HFileInfo*) &cipbr;
	OSErr					theError;
	Boolean					targetIsFolder,
							wasAliased;
	//Find data folder
	GetIndString(dataFolder.name, folderFileNameResID, 1);
	HGetVol(nil, &dataFolder.vRefNum, &dataFolder.parID);
	theError = ResolveAliasFile(&dataFolder, true, &targetIsFolder, &wasAliased);
	if(theError)
	return theError;
	if(!targetIsFolder)
	return kError_IncorrectAlias;
	
	//Get data folder parID
	fpb->ioVRefNum = dataFolder.vRefNum;
	fpb->ioNamePtr = dataFolder.name;
	fpb->ioDirID = dataFolder.parID;
	fpb->ioFDirIndex = 0;
	theError = PBGetCatInfo(&cipbr, false);
	if(theError)
	return theError;
	dataFolder.parID = fpb->ioDirID;
	
	//Get ships folder
	shipsFolder = dataFolder;
	GetIndString(shipsFolder.name, folderFileNameResID, folderShips);
	theError = ScanForFolderAlias(&shipsFolder);
	if(theError)
	return theError;
	
	//Get locations folder
	locationsFolder = dataFolder;
	GetIndString(locationsFolder.name, folderFileNameResID, folderLocations);
	theError = ScanForFolderAlias(&locationsFolder);
	if(theError)
	return theError;
	
	//Get characters folder
	charactersFolder = dataFolder;
	GetIndString(charactersFolder.name, folderFileNameResID, folderCharacters);
	theError = ScanForFolderAlias(&charactersFolder);
	if(theError)
	return theError;
	
#if __USE_AUDIO_FILES__
	//Get musics folder
	musicsFolder = dataFolder;
	GetIndString(musicsFolder.name, folderFileNameResID, folderMusics);
	theError = ScanForFolderAlias(&musicsFolder);
	if(theError)
	return theError;
#endif
	
#if !__DEMO_MODE__
	//Get add ons folder
	addOnsFolder = dataFolder;
	GetIndString(addOnsFolder.name, folderFileNameResID, folderAddOns);
	theError = ScanForFolderAlias(&addOnsFolder);
	if(theError)
	return theError;
#endif
	
	//Get pilots folder
	pilotsFolder = dataFolder;
	GetIndString(pilotsFolder.name, folderFileNameResID, folderPilots);
	theError = ScanForFolderAlias(&pilotsFolder);
	if(theError)
	return theError;
	
	//Get core folder
	coreFolder = dataFolder;
	GetIndString(coreFolder.name, folderFileNameResID, folderCore);
	theError = ScanForFolderAlias(&coreFolder);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr Open_DataFiles()
{
	Str63				fileName;
	
	//Open Interface file
	GetIndString(fileName, folderFileNameResID, fileInterface);
	interfaceResFileID = HOpenResFile(dataFolder.vRefNum, dataFolder.parID, fileName, fsRdPerm);
	if(interfaceResFileID == -1) {
		Wrapper_Error_Display(165, ResError(), nil, false);
		return ResError();
	}
	
	UseResFile(mainResFileID);
	
	//Open Sounds file
	GetIndString(fileName, folderFileNameResID, fileSounds);
	soundResFileID = HOpenResFile(dataFolder.vRefNum, dataFolder.parID, fileName, fsRdPerm);
	if(soundResFileID == -1) {
		Wrapper_Error_Display(165, ResError(), nil, false);
		return ResError();
	}
	
	UseResFile(mainResFileID);
	
	return noErr;
}

void Close_DataFiles()
{
	CloseResFile(interfaceResFileID);
	CloseResFile(soundResFileID);
	
	UseResFile(mainResFileID);
}

static OSErr Preload_Terrain(FSSpec* file, MetaFile_ReferencePtr reference, TerrainPreloadedDataPtr dataEntry)
{
	Data_TerrainDescriptionPtr	terrainDescription;
	OSErr						theError;
	Str63						name;
	OSType						id;
	long						i;
	OSType						dataType,
								dataID;
	Ptr							enablePreview,
								disablePreview;
	
	//Extract terrain id
	theError = MetaFile_Read_TerrainNameID_ByIndex(reference, name, &id, 1);
	if(theError)
	return theError;
	
	//Check for id conflict - Global
	if(id == kNoID)
	return kError_IDConflict;
	for(i = 0; i < coreData.terrainCount; ++i)
	if(coreData.terrainList[i].ID == id)
	return kError_IDConflict;
	for(i = 0; i < addOnData.terrainCount; ++i)
	if(addOnData.terrainList[i].ID == id)
	return kError_IDConflict;
	
#if __DEMO_MODE__
	//Make sure terrain file is one of the allowed ones
	GetEOF(reference->fileRefNum, &i);
	switch(id) {
		
		/*case 'Cayn':
		if(i != 4262074)
		return kError_CoreFileCorrupted;
		break;*/
		
		case 'Artc':
		if(i != 5478460)
		return kError_CoreFileCorrupted;
		break;
		
		case 'HlnG':
		if(i != 5670255)
		return kError_CoreFileCorrupted;
		break;
		
		default:
		return kError_CoreFileCorrupted;
		break;
		
	}
#endif
	
	//Extract terrain description - language dependant
	dataType = kSubTag_TerrainDescription;
#if __LANGUAGE__ == kLanguage_English
	dataID = kLanguageCode_English;
#elif __LANGUAGE__ == kLanguage_French
	dataID = kLanguageCode_French;
#elif __LANGUAGE__ == kLanguage_German
	dataID = kLanguageCode_German;
#elif __LANGUAGE__ == kLanguage_Italian
	dataID = kLanguageCode_Italian;
#elif
#error  __LANGUAGE__ undefined!
#endif
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &terrainDescription, &dataID);
	if(theError == kError_ItemNotFound)
	theError = MetaFile_Read_Data(reference, &dataType, 1, (Ptr*) &terrainDescription, &dataID);
	if(theError)
	return theError;
	
	//Extract terrain preview - enable
	dataType = kSubTag_Image;
	dataID = kEnablePreviewID;
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &enablePreview, &dataID);
	if(theError) {
		DisposePtr((Ptr) terrainDescription);
		return kError_TagMissing_Preview;
	}
		
	//Extract terrain preview - disable
	dataType = kSubTag_Image;
	dataID = kDisablePreviewID;
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &disablePreview, &dataID);
	if(theError) {
		DisposePtr((Ptr) enablePreview);
		DisposePtr((Ptr) terrainDescription);
		return kError_TagMissing_Preview;
	}
	
	//OK: append terrain to list
	dataEntry->fileSpec = *file;
	dataEntry->ID = id;
	dataEntry->description = *terrainDescription;
	theError = PtrToHand(enablePreview, (Handle*) &dataEntry->enablePreview, GetPtrSize((Ptr) enablePreview));
	if(theError)
	return theError;
	theError = PtrToHand(disablePreview, (Handle*) &dataEntry->disablePreview, GetPtrSize((Ptr) disablePreview));
	if(theError)
	return theError;
				
	//Clean up
	DisposePtr((Ptr) disablePreview);
	DisposePtr((Ptr) enablePreview);
	DisposePtr((Ptr) terrainDescription);
	
	return noErr;
}

static OSErr Preload_Ship(FSSpec* file, MetaFile_ReferencePtr reference, ShipPreloadedDataPtr dataEntry)
{
	Data_ShipDescriptionPtr		shipDescription;
	OSErr						theError;
	Str63						name;
	OSType						id;
	long						i;
	OSType						dataType,
								dataID;
	Ptr							enablePreview,
								disablePreview;
	
	//Extract ship id
	theError = MetaFile_Read_MegaObjectNameID_ByIndex(reference, name, &id, 1);
	if(theError)
	return theError;
	
	//Check for id conflict - Global
	if(id == kNoID)
	return kError_IDConflict;
	for(i = 0; i < coreData.shipCount; ++i)
	if(coreData.shipList[i].ID == id)
	return kError_IDConflict;
	for(i = 0; i < addOnData.shipCount; ++i)
	if(addOnData.shipList[i].ID == id)
	return kError_IDConflict;

#if __DEMO_MODE__
	//Make sure terrain file is one of the allowed ones
	GetEOF(reference->fileRefNum, &i);
	switch(id) {
		
		/*case 'SpdB':
		if(i != 904942)
		return kError_CoreFileCorrupted;
		break;*/
		
		case 'Frm1':
		if(i != 712086)
		return kError_CoreFileCorrupted;
		break;
		
		case 'Hovr':
		if(i != 1024630)
		return kError_CoreFileCorrupted;
		break;
		
		case 'OfRc':
		if(i != 543880)
		return kError_CoreFileCorrupted;
		break;
		
		default:
		return kError_CoreFileCorrupted;
		break;
		
	}
#endif
	
	//Extract ship description - language dependant
	dataType = kSubTag_ShipDescription;
#if __LANGUAGE__ == kLanguage_English
	dataID = kLanguageCode_English;
#elif __LANGUAGE__ == kLanguage_French
	dataID = kLanguageCode_French;
#elif __LANGUAGE__ == kLanguage_German
	dataID = kLanguageCode_German;
#elif __LANGUAGE__ == kLanguage_Italian
	dataID = kLanguageCode_Italian;
#elif
#error  __LANGUAGE__ undefined!
#endif
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &shipDescription, &dataID);
	if(theError == kError_ItemNotFound)
	theError = MetaFile_Read_Data(reference, &dataType, 1, (Ptr*) &shipDescription, &dataID);
	if(theError)
	return theError;
	
	//Extract ship preview - enable
	dataType = kSubTag_Image;
	dataID = kEnablePreviewID;
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &enablePreview, &dataID);
	if(theError) {
		DisposePtr((Ptr) shipDescription);
		return kError_TagMissing_Preview;
	}
	
	//Extract ship preview - disable
	dataType = kSubTag_Image;
	dataID = kDisablePreviewID;
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &disablePreview, &dataID);
	if(theError) {
		DisposePtr((Ptr) enablePreview);
		DisposePtr((Ptr) shipDescription);
		return kError_TagMissing_Preview;
	}
	
	//OK: append ship to list
	dataEntry->fileSpec = *file;
	dataEntry->ID = id;
#if 0
	dataEntry->description = *shipDescription;
#else
	BlockMoveData(shipDescription, &dataEntry->description, sizeof(Data_ShipDescription));
#endif
	theError = PtrToHand(enablePreview, (Handle*) &dataEntry->enablePreview, GetPtrSize((Ptr) enablePreview));
	if(theError)
	return theError;
	theError = PtrToHand(disablePreview, (Handle*) &dataEntry->disablePreview, GetPtrSize((Ptr) disablePreview));
	if(theError)
	return theError;
				
	//Clean up
	DisposePtr((Ptr) disablePreview);
	DisposePtr((Ptr) enablePreview);
	DisposePtr((Ptr) shipDescription);
	
	return noErr;
}

static OSErr Preload_Character(FSSpec* file, MetaFile_ReferencePtr reference, CharacterPreloadedDataPtr dataEntry)
{
	Data_CharacterDescriptionPtr	characterDescription;
	OSErr						theError;
	Str63						name;
	OSType						id;
	long						i;
	OSType						dataType,
								dataID;
	Ptr							enablePreview,
								disablePreview;
	
	//Extract character id
	theError = MetaFile_Read_MegaObjectNameID_ByIndex(reference, name, &id, 1);
	if(theError)
	return theError;
	
	//Check for id conflict - Global
	if(id == kNoID)
	return kError_IDConflict;
	for(i = 0; i < coreData.characterCount; ++i)
	if(coreData.characterList[i].ID == id)
	return kError_IDConflict;
	
#if __DEMO_MODE__
	//Make sure character file is one of the allowed ones
	if((id != 'BbSk') && (id != 'SgKg') && (id != 'EwGr') && (id != 'Leah'))
	return kError_CoreFileCorrupted;
#endif
	
	//Extract character description - language dependant
	dataType = kSubTag_CharacterDescription;
#if __LANGUAGE__ == kLanguage_English
	dataID = kLanguageCode_English;
#elif __LANGUAGE__ == kLanguage_French
	dataID = kLanguageCode_French;
#elif __LANGUAGE__ == kLanguage_German
	dataID = kLanguageCode_German;
#elif __LANGUAGE__ == kLanguage_Italian
	dataID = kLanguageCode_Italian;
#elif
#error  __LANGUAGE__ undefined!
#endif
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &characterDescription, &dataID);
	if(theError == kError_ItemNotFound)
	theError = MetaFile_Read_Data(reference, &dataType, 1, (Ptr*) &characterDescription, &dataID);
	if(theError)
	return theError;
	
	//Extract character preview - enable
	dataType = kSubTag_Image;
	dataID = kEnablePreviewID;
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &enablePreview, &dataID);
	if(theError) {
		DisposePtr((Ptr) characterDescription);
		return kError_TagMissing_Preview;
	}
		
	//Extract character preview - disable
	dataType = kSubTag_Image;
	dataID = kDisablePreviewID;
	theError = MetaFile_Read_Data(reference, &dataType, kMetaFile_UseID, (Ptr*) &disablePreview, &dataID);
	if(theError) {
		DisposePtr((Ptr) enablePreview);
		DisposePtr((Ptr) characterDescription);
		return kError_TagMissing_Preview;
	}
	
	//OK: append character to list
	dataEntry->fileSpec = *file;
	dataEntry->ID = id;
	dataEntry->description = *characterDescription;
	theError = PtrToHand(enablePreview, (Handle*) &dataEntry->enablePreview, GetPtrSize((Ptr) enablePreview));
	if(theError)
	return theError;
	theError = PtrToHand(disablePreview, (Handle*) &dataEntry->disablePreview, GetPtrSize((Ptr) disablePreview));
	if(theError)
	return theError;
	
	//Clean up
	DisposePtr((Ptr) enablePreview);
	DisposePtr((Ptr) disablePreview);
	DisposePtr((Ptr) characterDescription);
	
	return noErr;
}

static OSErr Preload_Folder(FSSpec* folder, DataSetPtr dataSet, Boolean officialOnly)
{
	FSSpec					theFile;
	CInfoPBRec				cipbr;
	HFileInfo				*fpb = (HFileInfo*) &cipbr;
	DirInfo					*dpb = (DirInfo*) &cipbr;
	long					idx;
	OSErr					theError;
	MetaFile_ReferencePtr	reference;
	
	theFile.vRefNum = folder->vRefNum;
	theFile.parID = folder->parID;
	fpb->ioVRefNum = folder->vRefNum;
	fpb->ioNamePtr = theFile.name;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = folder->parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		//If this is a folder, do a recursive scan
		if(fpb->ioFlAttrib & 16)
		Preload_Folder(&theFile, dataSet, officialOnly);
		
		//If this not a MetaFile, skip it
		if(fpb->ioFlFndrInfo.fdType != kMetaFileType)
		continue;
		
#if __ENABLE_DATAFILE_SKIPPING__
		//If this MetaFile begins with "_", skip it
		if(theFile.name[1] == kSkipChar)
		continue;
#endif
		
		//Open MetaFile
		theError = MetaFile_Read_OpenFile(&theFile, &reference);
		if(theError)
		return theError;

#if __PREVENT_FILE_HACKING__
		if(officialOnly) {
			//This metafile MUST BE an WR official data file
			if(MetaFile_CountDataSubTags(reference, kSubTag_Version) <= 0) {
				ParamText(nil, nil, nil, theFile.name);
				Wrapper_Error_Display(172, kError_AddOnFileInCoreFolder, nil, false);
				MetaFile_Read_CloseFile(reference);
				continue;
			}
		}
		else {
			//This metafile MUST NOT BE an WR official data file
			if(MetaFile_CountDataSubTags(reference, kSubTag_Version) > 0) {
				ParamText(nil, nil, nil, theFile.name);
				Wrapper_Error_Display(172, kError_CoreFileInAddOnFolder, nil, false);
				MetaFile_Read_CloseFile(reference);
				continue;
			}
		}
#endif

		//Find type of data the MetaFile contains
		if(MetaFile_CountDataSubTags(reference, kSubTag_TerrainDescription) > 0) {
			//It's a terrain
			if(dataSet->terrainCount < kMaxNbTerrains) {
				theError = Preload_Terrain(&theFile, reference, &dataSet->terrainList[dataSet->terrainCount]);
				if(theError) {
					ParamText(nil, nil, nil, theFile.name);
					Wrapper_Error_Display(166, theError, nil, false);
				}
				else
				dataSet->terrainCount += 1;
			}
		}
		else if(MetaFile_CountDataSubTags(reference, kSubTag_ShipDescription) > 0) {
			//It's a ship
			if(dataSet->shipCount < kMaxNbShips) {
				theError = Preload_Ship(&theFile, reference, &dataSet->shipList[dataSet->shipCount]);
				if(theError) {
					ParamText(nil, nil, nil, theFile.name);
					Wrapper_Error_Display(167, theError, nil, false);
				}
				else
				dataSet->shipCount += 1;
			}
		}
		else if(MetaFile_CountDataSubTags(reference, kSubTag_CharacterDescription) > 0) {
			if(officialOnly) {
				//It's a character
				if(dataSet->characterCount < kMaxNbCharacters) {
					theError = Preload_Character(&theFile, reference, &dataSet->characterList[dataSet->characterCount]);
					if(theError) {
						ParamText(nil, nil, nil, theFile.name);
						Wrapper_Error_Display(168, theError, nil, false);
					}
					else
					dataSet->characterCount += 1;
				}
			}
			else {
				//It's a character in the Add ons folder - DO NOT LOAD
				ParamText(nil, nil, nil, theFile.name);
				Wrapper_Error_Display(168, kError_AddOnCharacter, nil, false);
			}
		}
		else {
			ParamText(nil, nil, nil, theFile.name);
			Wrapper_Error_Display(169, kError_UnknownFileType, nil, false);
		}
		
		//Close MetaFile
		MetaFile_Read_CloseFile(reference);
	}
	
	return noErr;
}

static OSErr Preload_Core()
{
	OSErr					theError;
	FSSpec					demoFile;
	MetaFile_ReferencePtr	reference;
	
	//Get core folder location
	demoFile.vRefNum = coreFolder.vRefNum;
	demoFile.parID = coreFolder.parID;
	
#if !__DEMO_MODE__
	//Preload Practice Zone
	{
		//Get file
		GetIndString(demoFile.name, folderFileNameResID, filePractice);
	
		//Open MetaFile
		theError = MetaFile_Read_OpenFile(&demoFile, &reference);
		if(theError)
		return theError;
		
#if __PREVENT_FILE_HACKING__
		//This metafile MUST BE an WR official data file
		if(MetaFile_CountDataSubTags(reference, kSubTag_Version) <= 0) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(172, kError_CoreFileCorrupted, nil, false);
			MetaFile_Read_CloseFile(reference);
			return kError_CoreFileCorrupted;
		}
#endif

		//It's a terrain
		theError = Preload_Terrain(&demoFile, reference, &practiceTerrainData);
		if(theError) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(166, theError, nil, false);
		}
		
		//Close MetaFile
		MetaFile_Read_CloseFile(reference);
	}
#endif
	
	//Preload Leah
	{
		//Get file
		GetIndString(demoFile.name, folderFileNameResID, leahFileName);
	
		//Open MetaFile
		theError = MetaFile_Read_OpenFile(&demoFile, &reference);
		if(theError)
		return theError;
		
#if __PREVENT_FILE_HACKING__
		//This metafile MUST BE an WR official data file
		if(MetaFile_CountDataSubTags(reference, kSubTag_Version) <= 0) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(172, kError_CoreFileCorrupted, nil, false);
			MetaFile_Read_CloseFile(reference);
			return kError_CoreFileCorrupted;
		}
#endif

		//It's a character
		theError = Preload_Character(&demoFile, reference, &leahCharacterData);
		if(theError) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(168, theError, nil, false);
		}
		
		//Close MetaFile
		MetaFile_Read_CloseFile(reference);
	}
	
#if !__DEMO_MODE__
	//Preload Leah's ship
	{
		//Get file
		GetIndString(demoFile.name, folderFileNameResID, leahShipName);
	
		//Open MetaFile
		theError = MetaFile_Read_OpenFile(&demoFile, &reference);
		if(theError)
		return theError;
		
#if __PREVENT_FILE_HACKING__
		//This metafile MUST BE an WR official data file
		if(MetaFile_CountDataSubTags(reference, kSubTag_Version) <= 0) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(172, kError_CoreFileCorrupted, nil, false);
			MetaFile_Read_CloseFile(reference);
			return kError_CoreFileCorrupted;
		}
#endif

		//It's a ship
		theError = Preload_Ship(&demoFile, reference, &leahShipData);
		if(theError) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(167, theError, nil, false);
		}
		
		//Close MetaFile
		MetaFile_Read_CloseFile(reference);
	}
	
	//Preload Leah's terrain
	{
		//Get file
		GetIndString(demoFile.name, folderFileNameResID, leahTerrainName);
	
		//Open MetaFile
		theError = MetaFile_Read_OpenFile(&demoFile, &reference);
		if(theError)
		return theError;
		
#if __PREVENT_FILE_HACKING__
		//This metafile MUST BE an WR official data file
		if(MetaFile_CountDataSubTags(reference, kSubTag_Version) <= 0) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(172, kError_CoreFileCorrupted, nil, false);
			MetaFile_Read_CloseFile(reference);
			return kError_CoreFileCorrupted;
		}
#endif

		//It's a terrain
		theError = Preload_Terrain(&demoFile, reference, &leahTerrainData);
		if(theError) {
			ParamText(nil, nil, nil, demoFile.name);
			Wrapper_Error_Display(166, theError, nil, false);
		}
		
		//Close MetaFile
		MetaFile_Read_CloseFile(reference);
	}
#endif
	
	return noErr;
}

OSErr Preload_DataFolders()
{
	OSErr					theError;
								
	//Reset
	coreData.shipCount = 0;
	coreData.terrainCount = 0;
	coreData.characterCount = 0;
	addOnData.shipCount = 0;
	addOnData.terrainCount = 0;
	addOnData.characterCount = 0;
	
	//Load core data files
	theError = Preload_Core();
	if(theError)
	return theError;
	
	//Scan terrain folder
	theError = Preload_Folder(&locationsFolder, &coreData, true);
	if(theError)
	return theError;
	
	//Scan ship folder
	theError = Preload_Folder(&shipsFolder, &coreData, true);
	if(theError)
	return theError;
	
	//Scan character folder
	theError = Preload_Folder(&charactersFolder, &coreData, true);
	if(theError)
	return theError;
	
#if !__DEMO_MODE__
	//Scan add-ons folder
	theError = Preload_Folder(&addOnsFolder, &addOnData, false);
	if(theError)
	return theError;
#endif
	
	return noErr; //vŽrifer la cohŽrence avant de retourner
}

static void Release_DataSetMemory(DataSetPtr dataSet)
{
	long				i;
	
	for(i = 0; i < dataSet->shipCount; ++i) {
		DisposeHandle((Handle) dataSet->shipList[i].enablePreview);
		DisposeHandle((Handle) dataSet->shipList[i].disablePreview);
	}
	for(i = 0; i < dataSet->terrainCount; ++i) {
		DisposeHandle((Handle) dataSet->terrainList[i].enablePreview);
		DisposeHandle((Handle) dataSet->terrainList[i].disablePreview);
	}
	for(i = 0; i < dataSet->characterCount; ++i) {
		DisposeHandle((Handle) dataSet->characterList[i].enablePreview);
		DisposeHandle((Handle) dataSet->characterList[i].disablePreview);
	}
}

void Preload_ReleaseMemory()
{
	Release_DataSetMemory(&coreData);
	Release_DataSetMemory(&addOnData);
	
	DisposeHandle((Handle) leahShipData.enablePreview);
	DisposeHandle((Handle) leahShipData.disablePreview);
	DisposeHandle((Handle) leahTerrainData.enablePreview);
	DisposeHandle((Handle) leahTerrainData.disablePreview);
}

TerrainPreloadedDataPtr GetTerrainData_ByID(DataSetPtr dataSet, OSType ID)
{
	long				i;
	
	for(i = 0; i < dataSet->terrainCount; ++i)
	if(dataSet->terrainList[i].ID == ID)
	return &dataSet->terrainList[i];
	
	if(ID == leahTerrainData.ID)
	return &leahTerrainData;
	
	return nil;
}

ShipPreloadedDataPtr GetShipData_ByID(DataSetPtr dataSet, OSType ID)
{
	long				i;
	
	for(i = 0; i < dataSet->shipCount; ++i)
	if(dataSet->shipList[i].ID == ID)
	return &dataSet->shipList[i];
	
	if(ID == leahShipData.ID)
	return &leahShipData;
	
	return nil;
}

CharacterPreloadedDataPtr GetCharacterData_ByID(DataSetPtr dataSet, OSType ID)
{
	long				i;
	
	for(i = 0; i < dataSet->characterCount; ++i)
	if(dataSet->characterList[i].ID == ID)
	return &dataSet->characterList[i];
	
	return nil;
}
