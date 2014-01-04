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


#include					"WaterRace.h"
#include					"Data Files.h"
#include					"Interface.h"
#include					"Play Modes.h"

#if	__CD_PROTECTION__
#include				"CD Protection.h"
#endif

//CONSTANTES:

#define	kMagicRefCon		0xC267AF31

//ROUTINES:

#if __PREVENT_PILOT_HACKING__
inline long GetFileHardID(FSSpec* file)
{
	CInfoPBRec			info;
	
	info.hFileInfo.ioCompletion = nil;
	info.hFileInfo.ioNamePtr = file->name;
	info.hFileInfo.ioVRefNum = file->vRefNum;
	info.hFileInfo.ioFDirIndex = 0;
	info.hFileInfo.ioDirID = file->parID;
	if(PBGetCatInfoSync(&info) != noErr)
	return 0;
	
	return info.hFileInfo.ioDirID;
}

inline unsigned long GetPilotCheckSum(PilotPtr pilot)
{
	unsigned long		refCon1,
						refCon2;
	unsigned long		sum = 0;
	long				i;
	unsigned char*		ptr;
					
	refCon1 = pilot->refCon1;
	refCon2 = pilot->refCon2;
	pilot->refCon1 = 0;
	pilot->refCon2 = 0;
	
	ptr = (unsigned char*) pilot;
	for(i = 0; i < sizeof(Pilot); ++i, ++ptr)
	sum += *ptr;
	
	pilot->refCon1 = refCon1;
	pilot->refCon2 = refCon2;
	
	return sum;
}
#endif

void Pilot_New(PilotPtr pilot, Str31 name, OSType character, OSType mode, short difficulty)
{
	long				i,
						source,
						dest;
	OSType				temp;
	
	pilot->version = kPilot_Version;
	pilot->flags = 0;
	
	BlockMove(name, pilot->name, sizeof(Str31));
	pilot->character = character;
	pilot->mode = mode;
	pilot->difficulty = difficulty;
	GetDateTime(&pilot->creationDate);
	pilot->lastPlayDate = pilot->creationDate;
	pilot->playTime = 0;
	
	pilot->status = kStatus_Alive;
	PlayMode_GetDefaultBonus(mode, difficulty, &pilot->points, &pilot->numJokers);
	
	for(i = 0; i < coreData.characterCount; ++i)
	pilot->opponents[i] = coreData.characterList[i].ID;
#if !__DEMO_MODE__
	GetDateTime((unsigned long*) &qd.randSeed);
	for(i = 0; i < 64; ++i) {
		source = (Random() + 32767) % coreData.characterCount;
		dest = (Random() + 32767) % coreData.characterCount;
		temp = pilot->opponents[source];
		pilot->opponents[source] = pilot->opponents[dest];
		pilot->opponents[dest] = temp;
	}
#endif
	
	pilot->raceCount = 0;
}

OSErr Pilot_Save(PilotPtr pilot)
{
	OSErr		theError;
	short		fileID;
	FSSpec		destFile = pilotsFolder;
	long		bytesNumber;
	
	GetDateTime(&pilot->lastPlayDate);
#if __PREVENT_PILOT_HACKING__
	Boolean		newFile = false;
#endif
	
	BlockMove(pilot->name, destFile.name, sizeof(Str31));
	
	theError = FSpOpenDF(&destFile, fsRdWrPerm, &fileID);
	if(theError) {
		theError = FSpCreate(&destFile, kCreatorType, kPilotFileType, smSystemScript);
		if(theError)
		return theError;
		theError = FSpOpenDF(&destFile, fsRdWrPerm, &fileID);
		if(theError)
		return theError;
#if __PREVENT_PILOT_HACKING__
		newFile = true;
#endif
	}
	
	SetFPos(fileID, fsFromStart, 0);
	bytesNumber = sizeof(Pilot);
#if __PREVENT_PILOT_HACKING__
	if(newFile)
	pilot->refCon1 = GetFileHardID(&destFile);
	pilot->refCon2 = GetPilotCheckSum(pilot);
#else
	pilot->refCon1 = kMagicRefCon;
	pilot->refCon2 = kMagicRefCon;
#endif

#if __CD_PROTECTION__
	if(!networkOnly && !XCheckForCD("\pWaterRace -", 13, 60, 37))
	theError = noErr;
	else
#endif
	
	theError = FSWrite(fileID, &bytesNumber, pilot);
	FSClose(fileID);
	
	return theError;
}

OSErr Pilot_Load(Str63 name, PilotPtr pilot)
{
	OSErr		theError;
	short		fileID;
	FSSpec		destFile = pilotsFolder;
	long		bytesNumber;
	
	BlockMove(name, destFile.name, sizeof(Str63));
	theError = FSpOpenDF(&destFile, fsRdPerm, &fileID);
	if(theError)
	return theError;
	SetFPos(fileID, fsFromStart, 0);
	bytesNumber = sizeof(Pilot);
	theError = FSRead(fileID, &bytesNumber, pilot);
	FSClose(fileID);
	
	if(pilot->version != kPilot_Version)
	return kError_BadVersion;
	
#if __PREVENT_PILOT_HACKING__
	if((pilot->refCon1 != kMagicRefCon) && (pilot->refCon1 != GetFileHardID(&destFile)))
	return kError_PilotFileCorrupted;
	if((pilot->refCon2 != kMagicRefCon) && (pilot->refCon2 != GetPilotCheckSum(pilot)))
	return kError_PilotFileCorrupted;
#endif
	
	return theError;
}

#if __MOVE_PILOT_TO_TRASH__
#if 0
OSErr Pilot_Delete(Str63 name)
{
	OSErr		theError;
	FSSpec		sourceFile = pilotsFolder,
				destFile;
	
	BlockMove(name, sourceFile.name, sizeof(Str63));
	destFile.name[0] = 0;
	theError = FindFolder(pilotsFolder.vRefNum, kTrashFolderType, kCreateFolder, &destFile.vRefNum, &destFile.parID);
	if(theError)
	return theError;
	
	return FSpCatMove(&sourceFile, &destFile);
}
#else
OSErr Pilot_Delete(Str63 name)
{
	OSErr		theError;
	FSSpec		sourceFile = pilotsFolder,
				destFile;
#if __PREVENT_PILOT_HACKING__
	Pilot		temp;
	short		fileID;
	long		bytesNumber;
#endif
		
	BlockMove(name, sourceFile.name, sizeof(Str63));
	BlockMove(name, destFile.name, sizeof(Str63));
	
	theError = FindFolder(pilotsFolder.vRefNum, kTrashFolderType, kCreateFolder, &destFile.vRefNum, &destFile.parID);
	if(theError)
	return theError;
	theError = FSpCreate(&destFile, kCreatorType, kPilotFileType, smSystemScript);
	if(theError)
	return theError;
	
	theError = FSpExchangeFiles(&sourceFile, &destFile);
	if(theError)
	return theError;
	
	theError = FSpDelete(&sourceFile);
	if(theError)
	return theError;
	
#if __PREVENT_PILOT_HACKING__	
	theError = FSpOpenDF(&destFile, fsRdWrPerm, &fileID);
	if(theError == noErr) {
		SetFPos(fileID, fsFromStart, 0);
		bytesNumber = sizeof(Pilot);
		FSRead(fileID, &bytesNumber, &temp);
		
		temp.refCon1 = GetFileHardID(&destFile);
		temp.refCon2 = GetPilotCheckSum(&temp);
	
		SetFPos(fileID, fsFromStart, 0);
		bytesNumber = sizeof(Pilot);
		FSWrite(fileID, &bytesNumber, &temp);
		FSClose(fileID);
	}
#endif
	
	return noErr;
}
#endif
#else
OSErr Pilot_Delete(Str63 name)
{
	FSSpec		destFile = pilotsFolder;
	
	BlockMove(name, destFile.name, sizeof(Str63));
	
	return FSpDelete(&destFile);
}
#endif

RacePtr Pilot_GetRaceOnLocation(PilotPtr pilot, OSType locationID)
{
	long						i;
								
	
	for(i = 0; i < pilot->raceCount; ++i)
	if(pilot->raceList[i].location == locationID)
	return &pilot->raceList[i];
	
	return nil;
}

void Pilot_Update(PilotPtr pilot)
{
	RacePtr						race;
	PlayerConfigurationPtr		player = localShip->player;
	
	//Find the local player
	if(player == nil)
	return;
	
	//Update pilot score
	PlayMode_UpdatePilot(pilot, player);
	
	//Save race info
	race = Pilot_GetRaceOnLocation(pilot, gameConfig.terrainID);
	if(race == nil) {
		if(pilot->raceCount == kMaxRaces)
		return;
		
		race = &pilot->raceList[pilot->raceCount];
		++pilot->raceCount;
	}
	race->location = gameConfig.terrainID;
	if(player->finished) {
		race->totalTime = player->finishTime;
		race->bestLapTime = player->bestLapTime;
		race->leftTime = gameConfig.expiredTime;
		race->score = player->score;
		race->rank = player->rank;
	}
	else {
		race->totalTime = 0;
		race->bestLapTime = 0;
		race->leftTime = 0;
		race->score = 0;
		race->rank = -1;
	}
	race->maxSpeed = player->maxSpeed;
	race->totalDistance = player->totalDistance;
	
	//Update pilot play time
	GetDateTime(&pilot->lastPlayDate);
	pilot->playTime += gameConfig.raceLength / kTimeUnit;
	
	//Check if pilot is challenger
	PlayMode_CheckForChallenge(pilot, player);
	
	//Check if pilot is stuck
	PlayMode_CheckIfPilotStuck(pilot, player);
}
