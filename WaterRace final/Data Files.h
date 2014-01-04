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


#ifndef __WATERRACE_DATA_FILES__
#define __WATERRACE_DATA_FILES__

#include				"MetaFile Data Tags.h"

//CONSTANTES:

#if __DEMO_MODE__
#define					kMaxNbShips				3
#define					kMaxNbTerrains			2
#define					kMaxNbCharacters		3
#else
#define					kMaxNbShips				8
#define					kMaxNbTerrains			kMaxNbShips
#define					kMaxNbCharacters		kMaxNbShips

#endif

#define					kEnablePreviewID		'Enbl'
#define					kDisablePreviewID		'Dsbl'
#define					kPreviewID				'Prvw'

//STRUCTURES:

typedef struct {
	FSSpec					fileSpec;
	OSType					ID;
	Data_ShipDescription	description;
	PicHandle				enablePreview,
							disablePreview;
} ShipPreloadedData;
typedef ShipPreloadedData* ShipPreloadedDataPtr;

typedef struct {
	FSSpec					fileSpec;
	OSType					ID;
	Data_TerrainDescription	description;
	PicHandle				enablePreview,
							disablePreview;
} TerrainPreloadedData;
typedef TerrainPreloadedData* TerrainPreloadedDataPtr;

typedef struct {
	FSSpec					fileSpec;
	OSType					ID;
	Data_CharacterDescription	description;
	PicHandle				enablePreview,
							disablePreview;
} CharacterPreloadedData;
typedef CharacterPreloadedData* CharacterPreloadedDataPtr;

typedef struct {
	long					shipCount;
	ShipPreloadedData		shipList[kMaxNbShips];
	long					terrainCount;
	TerrainPreloadedData	terrainList[kMaxNbTerrains];
	long					characterCount;
	CharacterPreloadedData	characterList[kMaxNbCharacters];
} DataSet;
typedef DataSet* DataSetPtr;

//VARIABLES:
							
extern FSSpec				dataFolder,
							shipsFolder,
							locationsFolder,
							charactersFolder,
							pilotsFolder,
							coreFolder;
#if !__DEMO_MODE__
extern FSSpec				addOnsFolder;
#endif
#if __USE_AUDIO_FILES__ || __USE_AUDIO_CD_TRACKS__
extern FSSpec				musicsFolder;
#endif
							
extern short				mainResFileID,
							interfaceResFileID,
							soundResFileID;
							
extern DataSet				coreData,
							addOnData;
extern ShipPreloadedData		leahShipData;
extern TerrainPreloadedData		leahTerrainData,
								practiceTerrainData;
extern CharacterPreloadedData	leahCharacterData;

//ROUTINES:

//Fichier: Data Files.cp
OSErr Resolve_DataFolders();
OSErr Open_DataFiles();
void Close_DataFiles();
OSErr Preload_DataFolders();
void Preload_ReleaseMemory();

TerrainPreloadedDataPtr GetTerrainData_ByID(DataSetPtr dataSet, OSType ID);
ShipPreloadedDataPtr GetShipData_ByID(DataSetPtr dataSet, OSType ID);
CharacterPreloadedDataPtr GetCharacterData_ByID(DataSetPtr dataSet, OSType ID);

#endif
