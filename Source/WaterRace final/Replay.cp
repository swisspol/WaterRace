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
#include				"Game.h"

//CONSTANTES:

#define				kReplayVersion			0x0100

//STRUCTURES:

typedef struct Replay_ShipParameters
{
	OSType			shipID,
					shipTextureID;
	Str31			playerName;
	
	unsigned long	bestLapTime,
					finishTime;
	float			maxSpeed,
					totalDistance;
					
	long			eventCount;
	
	long			unused[4];
};

typedef struct Replay_GameParameters
{
	short			version;
	unsigned long	flags;
	long			eventBufferSize;
	
	long			unused[12];
	
	long			raceStartTime,
					globalBestLapTime,
					raceLength;
	
	OSType			terrainID;
	short			terrainDifficulty;
	long			playerCount;
	Replay_ShipParameters	playerList[kMaxPlayers];
};

//VARIABLES LOCALES:

static Replay_GameParameters	replayParams;

//ROUTINES:

void Replay_Reset()
{
	long			i;
	
	//Reset counters
	for(i = 0; i < shipCount; ++i)
	shipList[i].lastEventNum = 0;
}

void Replay_Init()
{
	long			i;
	
	//Allocate memory - Check for nil at run-time
	for(i = 0; i < shipCount; ++i)
	shipList[i].eventBuffer = (ReplayEventPtr) NewPtr(sizeof(ReplayEvent) * kMaxReplayEvents);
	
	//Reset counters
	for(i = 0; i < shipCount; ++i)
	shipList[i].eventCount = 0;
}

void Replay_Quit()
{
	long			i;
	
	for(i = 0; i < shipCount; ++i)
	if(shipList[i].eventBuffer != nil)
	DisposePtr((Ptr) shipList[i].eventBuffer);
}

OSErr Replay_Save(FSSpec* destFile)
{
	long					i;
	OSErr					theError;
	short					fileID;
	long					bytesNumber;
	
	//Create file
	theError = FSpCreate(destFile, kCreatorType, kReplayFileType, smSystemScript);
	if(theError)
	return theError;
	
	//Open file
	theError = FSpOpenDF(destFile, fsRdWrPerm, &fileID);
	if(theError)
	return theError;
	SetFPos(fileID, fsFromStart, 0);
	
	//Setup replayParams structure
	replayParams.version = kReplayVersion;
	replayParams.flags = 0;
	replayParams.eventBufferSize = sizeof(ReplayEvent) * kMaxReplayEvents;
	replayParams.raceStartTime = gameConfig.startTime;
	replayParams.globalBestLapTime = gameConfig.globalBestLapTime;
	replayParams.raceLength = gameConfig.raceLength;
	replayParams.terrainID = gameConfig.terrainID;
	replayParams.terrainDifficulty = gameConfig.terrainDifficulty;
	replayParams.playerCount = gameConfig.playerCount;
	for(i = 0; i < gameConfig.playerCount; ++i) {
		replayParams.playerList[i].shipID = gameConfig.playerList[i].shipID;
		replayParams.playerList[i].shipTextureID = gameConfig.playerList[i].shipTextureID;
		BlockMove(gameConfig.playerList[i].playerName, replayParams.playerList[i].playerName, sizeof(Str31));
		replayParams.playerList[i].bestLapTime = gameConfig.playerList[i].bestLapTime;
		replayParams.playerList[i].finishTime = gameConfig.playerList[i].finishTime;
		replayParams.playerList[i].maxSpeed = gameConfig.playerList[i].maxSpeed;
		replayParams.playerList[i].totalDistance = gameConfig.playerList[i].totalDistance;
		
		replayParams.playerList[i].eventCount = shipList[i].eventCount;
	}
	
	//Write replayParams
	bytesNumber = sizeof(Replay_GameParameters);
	theError = FSWrite(fileID, &bytesNumber, &replayParams);
	if(theError)
	return theError;
	
	//Write replay event buffers
	for(i = 0; i < gameConfig.playerCount; ++i) {
		bytesNumber = replayParams.eventBufferSize;
		theError = FSWrite(fileID, &bytesNumber, shipList[i].eventBuffer);
		if(theError)
		return theError;
	}
	
	//Clean up
	FSClose(fileID);
	
	return noErr;
}

OSErr Replay_SetupGame(FSSpec* file)
{
	OSErr						theError;
	short						fileID;
	long						bytesNumber;
	long						i;
	
	//Open file
	theError = FSpOpenDF(file, fsRdPerm, &fileID);
	if(theError)
	return theError;
	SetFPos(fileID, fsFromStart, 0);
	
	//Read replayParams
	bytesNumber = sizeof(Replay_GameParameters);
	theError = FSRead(fileID, &bytesNumber, &replayParams);
	if(theError)
	return theError;
	
	//Check version
	if(replayParams.version != kReplayVersion) {
		FSClose(fileID);
		return kError_BadVersion;
	}
	
	//Load replay event buffers
	for(i = 0; i < replayParams.playerCount; ++i) {
		shipList[i].eventBuffer = (ReplayEventPtr) NewPtr(replayParams.eventBufferSize);
		if(shipList[i].eventBuffer == nil) {
			FSClose(fileID);
			return MemError();
		}
		bytesNumber = replayParams.eventBufferSize;
		theError = FSRead(fileID, &bytesNumber, shipList[i].eventBuffer);
		if(theError) {
			FSClose(fileID);
			return theError;
		}
	}
	
	//Setup game info
	gameConfig.version = kNetGameVersion;
	gameConfig.gameMode = kGameMode_Replay;
	gameConfig.gameSubMode = kGameSubMode_None;
	gameConfig.gameMode_save = gameConfig.gameMode;
	gameConfig.gameSubMode_save = gameConfig.gameSubMode;
	gameConfig.terrainID = replayParams.terrainID;
	gameConfig.terrainDifficulty = replayParams.terrainDifficulty;
	gameConfig.playerCount = replayParams.playerCount;
	
	gameConfig.startTime = replayParams.raceStartTime;
	gameConfig.globalBestLapTime = replayParams.globalBestLapTime;
	gameConfig.raceLength = replayParams.raceLength;
	
	//Clear players
	for(i = 0; i < kMaxPlayers; ++i)
	Clear_Player(&gameConfig.playerList[i]);
	
	//Setup player infos
	for(i = 0; i < replayParams.playerCount; ++i) {
		gameConfig.playerList[i].characterID = kNoID;
		gameConfig.playerList[i].shipID = replayParams.playerList[i].shipID;
		gameConfig.playerList[i].shipTextureID = replayParams.playerList[i].shipTextureID;
		gameConfig.playerList[i].shipPilotType = kPilot_Replay;
		BlockMove(replayParams.playerList[i].playerName, gameConfig.playerList[i].playerName, sizeof(Str31));
		gameConfig.playerList[i].playerDataPtr = nil;
		
		gameConfig.playerList[i].bestLapTime = replayParams.playerList[i].bestLapTime;
		gameConfig.playerList[i].finishTime = replayParams.playerList[i].finishTime;
		gameConfig.playerList[i].maxSpeed = replayParams.playerList[i].maxSpeed;
		gameConfig.playerList[i].totalDistance = replayParams.playerList[i].totalDistance;
		
		shipList[i].eventCount = replayParams.playerList[i].eventCount;
	}

	//Clean up
	FSClose(fileID);
	
	//Reset counters
	Replay_Reset();
	
	return noErr;
}
