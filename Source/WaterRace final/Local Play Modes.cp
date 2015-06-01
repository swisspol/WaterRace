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
#include					"Strings.h"
#include					"Data Files.h"
#include					"Interface.h"
#include					"Play Modes.h"

#include					"Terrain Utils.h"

//CONSTANTES:

#if __DEMO_MODE__
#define						kMaxLocalPlayers		3
#else
#define						kMaxLocalPlayers		4
#endif

//ROUTINES:

void PlayMode_GetName(OSType mode, Str31 name)
{
	switch(mode) {
		case kPilotType_Tournament: GetIndString(name, kModeStringID, 1); break;
		case kPilotType_TimeRace: GetIndString(name, kModeStringID, 2); break;
		case kPilotType_Duel: GetIndString(name, kModeStringID, 3); break;
		case kPilotType_FreePlay: GetIndString(name, kModeStringID, 4); break;
		case kPilotType_AddOns: GetIndString(name, kModeStringID, 5); break;
		default: name[0] = 0; break;
	}
}

OSType PlayMode_GetLeahAnim(OSType mode)
{
	switch(mode) {
		case kPilotType_Tournament: return kNewTournament; break;
		case kPilotType_TimeRace: return kNewTimeRace; break;
		case kPilotType_Duel: return kNewDuel; break;
		case kPilotType_FreePlay: return kNewFreePlay; break;
		case kPilotType_AddOns: return kNewAddOns; break;
		default: return kNoID; break;
	}
}

void PlayMode_GetDefaultBonus(OSType mode, short difficulty, long* points, long* numJokers)
{
	switch(mode) {
		
		case kPilotType_Tournament:
		*points = 2000 - difficulty * 500;
		*numJokers = 3 - difficulty;
		break;
		
		case kPilotType_TimeRace:
		case kPilotType_Duel:
		*points = 0;
		*numJokers = 3 - difficulty;
		break;
		
		case kPilotType_FreePlay:
		case kPilotType_AddOns:
		default:
		*points = 0;
		*numJokers = 0;
		break;
		
	}
}

void PlayMode_CheckIfPilotStuck(PilotPtr pilot, PlayerConfigurationPtr player)
{
	long						i;
	
	if((pilot->status != kStatus_Alive) && (pilot->status != kStatus_Challenge))
	return;
	
	if(pilot->status == kStatus_Alive) {
		switch(pilot->mode) {
			
			case kPilotType_Tournament:
			for(i = 0; i < coreData.terrainCount; ++i)
			if(PlayMode_GetLocationAccess(pilot, i) > 0)
			return;
			pilot->status = kStatus_Dead;
			break;
			
			case kPilotType_Duel:
			case kPilotType_TimeRace:
			if(!player->finished && (pilot->numJokers == 0))
			pilot->status = kStatus_Dead;
			break;
			
			/*case kPilotType_FreePlay:
			kPilotType_AddOns:
			
			break;*/
			
		}
	}
	else if(pilot->status == kStatus_Challenge) {
		switch(pilot->mode) {
			
			case kPilotType_Tournament:
#if !__LIMIT_CHALLENGE_TO_TOURNAMENT__
			case kPilotType_TimeRace:
			case kPilotType_Duel:
#endif
			if(PlayMode_GetChallengeLocationAccess(pilot) > 0)
			return;
			pilot->status = kStatus_Dead;
			break;
			
			/*case kPilotType_FreePlay:
			kPilotType_AddOns:
			
			break;*/
			
		}
	}
}

void PlayMode_CheckForChallenge(PilotPtr pilot, PlayerConfigurationPtr player)
{
	RacePtr						race;
	long						i;
	
	if(pilot->status != kStatus_Alive)
	return;
	if(pilot->mode != kPilotType_Tournament)
	return;
	if(!player->finished) //--??
	return;
	
	for(i = 0; i < coreData.terrainCount; ++i) {
		race = Pilot_GetRaceOnLocation(pilot, coreData.terrainList[i].ID);
		if((race == nil) || (race->rank != 1))
		return;
	}
	
	pilot->status = kStatus_Challenge;
}

void PlayMode_UpdatePilot(PilotPtr pilot, PlayerConfigurationPtr player)
{
	RacePtr						race;
	
	if((pilot->status != kStatus_Alive) && (pilot->status != kStatus_Challenge))
	return;
	
	if(pilot->status == kStatus_Alive) {
		switch(pilot->mode) {
			
			case kPilotType_Tournament:
			if(player->finished) {
				race = Pilot_GetRaceOnLocation(pilot, gameConfig.terrainID);
				if(race != nil) { //this is a replay
					if(player->score > race->score)
					pilot->points += player->score - race->score;
				}
				else //this is a first play
				pilot->points += player->score;
				if(player->hasJoker)
				++pilot->numJokers;
			}
			break;
			
			case kPilotType_TimeRace:
			if(player->finished) {
				++pilot->points;
				if(player->hasJoker)
				++pilot->numJokers;
				
				//Check for champion
				if(pilot->points >= coreData.terrainCount)
#if __LIMIT_CHALLENGE_TO_TOURNAMENT__
				pilot->status = kStatus_Champion;
#else
				pilot->status = kStatus_Challenge;
#endif
			}
#if 1
			else if(gameConfig.expiredTime == 0) { //Time has expired, but the player did not press Esc
				if(player->hasJoker)
				++pilot->numJokers;
			}
#endif
			break;
			
			case kPilotType_Duel:
			if(player->finished) {
				if(player->rank == 1)
				++pilot->points;
				if(player->hasJoker)
				++pilot->numJokers;
				
				//Check for champion
				if(pilot->points >= coreData.terrainCount)
#if __LIMIT_CHALLENGE_TO_TOURNAMENT__
				pilot->status = kStatus_Champion;
#else
				pilot->status = kStatus_Challenge;
#endif
			}
			break;
			
			/*case kPilotType_FreePlay:
			kPilotType_AddOns:
			
			break;*/
			
		}
	}
	else if(pilot->status == kStatus_Challenge) {
		switch(pilot->mode) {
			
			case kPilotType_Tournament:
			if(player->finished) {
				if(player->hasJoker)
				++pilot->numJokers;
				
				if(player->rank == 1)
				pilot->status = kStatus_Champion; //FIXME: play Leah anim
			}
			break;
			
#if !__LIMIT_CHALLENGE_TO_TOURNAMENT__
			case kPilotType_TimeRace:
			if(player->finished) {
				if(player->hasJoker)
				++pilot->numJokers;
				
				pilot->status = kStatus_Champion; //FIXME: play Leah anim
			}
			break;
			
			case kPilotType_Duel:
			if(player->finished) {
				if(player->hasJoker)
				++pilot->numJokers;
				
				if(player->rank == 1)
				pilot->status = kStatus_Champion; //FIXME: play Leah anim
			}
			break;
#endif
			
			/*case kPilotType_FreePlay:
			kPilotType_AddOns:
			
			break;*/
			
		}
	}
}

void PlayMode_GetStatus(PilotPtr pilot, Str31 status)
{
	switch(pilot->status) {
		
		case kStatus_Alive:
		switch(pilot->mode) {
			
			case kPilotType_Tournament:
			NumToString(pilot->points, status);
			status[++status[0]] = ' ';
			status[++status[0]] = 'p';
			status[++status[0]] = 't';
			status[++status[0]] = 's';
			break;
			
			case kPilotType_TimeRace:
			case kPilotType_Duel:
			BlockMove(kString_Interface_Level, status, sizeof(Str31));
			++status[0];
			FastNumToString1(pilot->points + 1, &status[status[0]]);
			break;
			
			case kPilotType_FreePlay:
			case kPilotType_AddOns:
			default:
			BlockMove(kString_Interface_NA, status, sizeof(Str31));
			break;
			
		}
		break;
		
		case kStatus_Dead:
		BlockMove(kString_Interface_GameOver, status, sizeof(Str31));
		break;
		
		case kStatus_Challenge:
		BlockMove(kString_Interface_Challenge, status, sizeof(Str31));
		break;
		
		case kStatus_Champion:
		BlockMove(kString_Interface_Champion, status, sizeof(Str31));
		break;
		
	}
}

void PlayMode_GetNext(OSType* mode)
{
#if __DEMO_MODE__
	switch(*mode) {
		case kPilotType_FreePlay: *mode = kPilotType_FreePlay; break;
	}
#else
	switch(*mode) {
		case kPilotType_FreePlay: *mode = kPilotType_TimeRace; break;
		case kPilotType_TimeRace: *mode = kPilotType_Duel; break;
		case kPilotType_Duel: *mode = kPilotType_Tournament; break;
		case kPilotType_Tournament: *mode = kPilotType_AddOns; break;
		case kPilotType_AddOns: *mode = kPilotType_FreePlay; break;
	}
#endif
}

OSErr PlayMode_Practice_SetUpPlayers(PilotPtr pilot, OSType localPlayerShipID)
{
	PlayerConfigurationPtr		player;
	CharacterPreloadedDataPtr	characterData;
	
	//Find local player character
	characterData = GetCharacterData_ByID(&coreData, pilot->character);
	if(characterData == nil)
	characterData = GetCharacterData_ByID(&addOnData, pilot->character);
	if(characterData == nil)
	return kError_CharacterNotFound;

	player = &gameConfig.playerList[0];
	player->characterID = pilot->character;
	player->shipID = localPlayerShipID;
	player->shipTextureID = characterData->description.textureSetID;
	player->shipPilotType = kPilot_Local;
	BlockMove(pilot->name, player->playerName, sizeof(Str31));
	player->playerDataPtr = (Ptr) pilot;
	gameConfig.playerCount = 1;
	
	return noErr;
}

static OSErr GetShip_RelatedToTerrain(OSType terrainID, OSType* shipID)
{
	long						i;
	
	//Look for the owner of this terrain
	for(i = 0; i < coreData.characterCount; ++i)
	if(coreData.characterList[i].description.terrainID == terrainID) {
		//Set ship ID
		*shipID = coreData.characterList[i].description.shipID;
		return noErr;
	}
	if(leahCharacterData.description.terrainID == terrainID) {
		//Set ship ID
		*shipID = leahCharacterData.description.shipID;
		return noErr;
	}
	
	return kError_CharacterNotFound;
}

static OSErr Local_SetUpPlayer(PilotPtr pilot, OSType terrainID, OSType localPlayerShipID, OSType* shipID, OSType* shipTextureID)
{
	CharacterPreloadedDataPtr	characterData;
	OSErr						theError;
	
	//Find local player character
	characterData = GetCharacterData_ByID(&coreData, pilot->character);
	if(characterData == nil)
	characterData = GetCharacterData_ByID(&addOnData, pilot->character);
	if(characterData == nil)
	return kError_CharacterNotFound;
	
	//Setup local player config
	switch(pilot->mode) {
		
		case kPilotType_Tournament:
		case kPilotType_AddOns:
		case kPilotType_FreePlay:
		case kPilotType_TimeRace:
		default:
		*shipID = localPlayerShipID;
		break;
		
		case kPilotType_Duel: //In this mode, the local player use the ship of the terrain's character
		theError = GetShip_RelatedToTerrain(terrainID, shipID);
		if(theError)
		return theError;
		break;
		
	}
	*shipTextureID = characterData->description.textureSetID;
	
	return noErr;
}

static OSErr Local_SetUpBots(PilotPtr pilot, OSType terrainID, OSType localPlayerShipID)
{
	PlayerConfigurationPtr		player;
	long						i;
	OSErr						theError = noErr;
	CharacterPreloadedDataPtr	data;
	
#if __BOT_ONLY__
	{
		player = &gameConfig.playerList[gameConfig.playerCount];
		
		player->characterID = coreData.characterList[0].ID;
		player->shipID = localPlayerShipID;
		player->shipTextureID = coreData.characterList[0].description.textureSetID;
		player->shipPilotType = kPilot_Bot;
		CopyFirstName(coreData.characterList[0].description.realName, player->playerName);
		player->playerDataPtr = (Ptr) &coreData.characterList[0].description;
		
		++gameConfig.playerCount;
	}
#else
	switch(pilot->mode) {
		
		case kPilotType_Tournament:
		{
			if(pilot->status == kStatus_Challenge) {
				player = &gameConfig.playerList[gameConfig.playerCount];
				
				player->characterID = leahCharacterData.ID;
				player->shipID = leahCharacterData.description.shipID;
				player->shipTextureID = leahCharacterData.description.textureSetID;
				player->shipPilotType = kPilot_Bot;
				CopyFirstName(leahCharacterData.description.realName, player->playerName);
				player->playerDataPtr = (Ptr) &leahCharacterData.description;
				
				++gameConfig.playerCount;
			}
			else {
				for(i = 0; i < kMaxOpponents; ++i)
				if((gameConfig.playerCount < kMaxLocalPlayers) && (pilot->opponents[i] != pilot->character)) {
					player = &gameConfig.playerList[gameConfig.playerCount];
					data = GetCharacterData_ByID(&coreData, pilot->opponents[i]);
					
					player->characterID = data->ID;
					player->shipID = data->description.shipID;
					player->shipTextureID = data->description.textureSetID;
					player->shipPilotType = kPilot_Bot;
					CopyFirstName(data->description.realName, player->playerName);
					player->playerDataPtr = (Ptr) &data->description;
					
					++gameConfig.playerCount;
				}
			}
		}
		break;
		
		case kPilotType_TimeRace:
		default:
		;//Single player game
		break;
		
		case kPilotType_Duel:
		{
#if !__LIMIT_CHALLENGE_TO_TOURNAMENT__
			if(pilot->status == kStatus_Challenge) {
				player = &gameConfig.playerList[gameConfig.playerCount];
				
				player->characterID = leahCharacterData.ID;
				player->shipID = leahCharacterData.description.shipID;
				player->shipTextureID = leahCharacterData.description.textureSetID;
				player->shipPilotType = kPilot_Bot;
				CopyFirstName(leahCharacterData.description.realName, player->playerName);
				player->playerDataPtr = (Ptr) &leahCharacterData.description;
				
				++gameConfig.playerCount;
			}
			else {
#endif
				//Look for the owner of this terrain
				theError = kError_CharacterNotFound;
				for(i = 0; i < coreData.characterCount; ++i)
				if(coreData.characterList[i].description.terrainID == terrainID) {
					player = &gameConfig.playerList[gameConfig.playerCount];
					
					player->characterID = coreData.characterList[i].ID;
					player->shipID = coreData.characterList[i].description.shipID;
					player->shipTextureID = coreData.characterList[i].description.textureSetID;
					player->shipPilotType = kPilot_Bot;
					CopyFirstName(coreData.characterList[i].description.realName, player->playerName);
					player->playerDataPtr = (Ptr) &coreData.characterList[i].description;
					
					++gameConfig.playerCount;
					
					theError = noErr;
				}
#if !__LIMIT_CHALLENGE_TO_TOURNAMENT__
			}
#endif
		}
		break;
		
		case kPilotType_FreePlay:
		{
			for(i = 0; i < kMaxOpponents; ++i)
			if((gameConfig.playerCount < kMaxLocalPlayers) && (pilot->opponents[i] != pilot->character)) {
				player = &gameConfig.playerList[gameConfig.playerCount];
				data = GetCharacterData_ByID(&coreData, pilot->opponents[i]);
					
				player->characterID = data->ID;
				player->shipID = localPlayerShipID;
				player->shipTextureID = data->description.textureSetID;
				player->shipPilotType = kPilot_Bot;
				CopyFirstName(data->description.realName, player->playerName);
				player->playerDataPtr = (Ptr) &data->description;
				
				++gameConfig.playerCount;
			}
		}
		break;
		
		case kPilotType_AddOns:
		{
			for(i = 0; i < kMaxOpponents; ++i)
			if((gameConfig.playerCount < kMaxLocalPlayers) && (pilot->opponents[i] != pilot->character)) {
				player = &gameConfig.playerList[gameConfig.playerCount];
				data = GetCharacterData_ByID(&coreData, pilot->opponents[i]);
				
				player->characterID = data->ID;
				player->shipID = localPlayerShipID; //data->description.shipID;
				player->shipTextureID = data->description.textureSetID;
				player->shipPilotType = kPilot_Bot;
				CopyFirstName(data->description.realName, player->playerName);
				player->playerDataPtr = (Ptr) &data->description;
				
				++gameConfig.playerCount;
			}
		}
		break;
		
	}
#endif	
	return theError;
}

OSErr PlayMode_Local_SetUpPlayers(PilotPtr pilot, OSType localPlayerShipID)
{
	OSErr						theError;
	PlayerConfigurationPtr		player;
	
	if((pilot->status == kStatus_Dead) || (pilot->status == kStatus_Champion))
	return kError_FatalError;
	
	//Setup local player
	player = &gameConfig.playerList[0];
	player->characterID = pilot->character;
	theError = Local_SetUpPlayer(pilot, gameConfig.terrainID, localPlayerShipID, &player->shipID, &player->shipTextureID);
	if(theError)
	return theError;
	player->shipPilotType = kPilot_Local;
	BlockMove(pilot->name, player->playerName, sizeof(Str31));
	player->playerDataPtr = (Ptr) pilot;
	gameConfig.playerCount = 1;
	
#if __ENABLE_BOTS__
	//Setup bots ships & look
	theError = Local_SetUpBots(pilot, gameConfig.terrainID, localPlayerShipID);
	if(theError)
	return theError;
#endif
	
	return noErr;
}

Boolean PlayMode_PointsAvailable(OSType mode)
{
	switch(mode) {
		
		case kPilotType_Tournament:
		return true;
		break;
		
		case kPilotType_TimeRace:
		case kPilotType_Duel:
		case kPilotType_FreePlay:
		case kPilotType_AddOns:
		return false;
		break;
		
	}
	
	return false;
}

Boolean PlayMode_JokersAvailable(OSType mode)
{
	switch(mode) {
		
		case kPilotType_Tournament:
		case kPilotType_TimeRace:
		case kPilotType_Duel:
		return true;
		break;
		
		case kPilotType_FreePlay:
		case kPilotType_AddOns:
		return false;
		break;
		
	}
	
	return false;
}

DataSetPtr PlayMode_GetDataSet(OSType mode)
{
	switch(mode) {
		
		case kPilotType_Tournament:
		case kPilotType_TimeRace:
		case kPilotType_Duel:
		case kPilotType_FreePlay:
		return &coreData;
		break;
		
		case kPilotType_AddOns:
		return &addOnData;
		break;
		
	}
	
	return nil;
}

short PlayMode_GetLocationAccess(PilotPtr pilot, long num)
{
#if __ACCESS_ALL_LEVELS__
	return kAccess_OK;
#else
	if(pilot->status != kStatus_Alive)
	return kAccess_Disabled;
	
	switch(pilot->mode) {
		
		case kPilotType_Tournament:
		{
			if(coreData.terrainList[num].description.points[pilot->difficulty - 1] > pilot->points)
			return kAccess_NotEnoughPoints;
			
			if(Pilot_GetRaceOnLocation(pilot, coreData.terrainList[num].ID) != nil) {
				if(pilot->numJokers <= 0)
				return kAccess_NeedJoker;
				else
				return kAccess_OK_Joker;
			}
			else
			return kAccess_OK;
		}
		break;
		
		case kPilotType_TimeRace:
		case kPilotType_Duel:
		{
			if(num == pilot->points) {
				if(Pilot_GetRaceOnLocation(pilot, coreData.terrainList[num].ID) != nil) {
					if(pilot->numJokers <= 0)
					return kAccess_NeedJoker;
					else
					return kAccess_OK_Joker;
				}
				else
				return kAccess_OK;
			}
			else
			return kAccess_Disabled;
		}
		break;
		
		case kPilotType_FreePlay:
		case kPilotType_AddOns:
		return kAccess_OK;
		break;
		
	}
#endif

	return kAccess_OK;
}

short PlayMode_GetChallengeLocationAccess(PilotPtr pilot)
{
	if(pilot->status != kStatus_Challenge)
	return kAccess_Disabled;
	
	switch(pilot->mode) {
		
		case kPilotType_Tournament:
#if !__LIMIT_CHALLENGE_TO_TOURNAMENT__
		case kPilotType_TimeRace:
		case kPilotType_Duel:
#endif
		{
			if(Pilot_GetRaceOnLocation(pilot, leahTerrainData.ID) != nil) {
				if(pilot->numJokers <= 0)
				return kAccess_NeedJoker;
				else
				return kAccess_OK_Joker;
			}
			else
			return kAccess_OK;
		}
		break;
		
#if __LIMIT_CHALLENGE_TO_TOURNAMENT__
		case kPilotType_TimeRace:
		case kPilotType_Duel:
#endif
		case kPilotType_FreePlay:
		case kPilotType_AddOns:
		return kAccess_Disabled;
		break;
		
	}
	
	return kAccess_Disabled;
}

short PlayMode_GetShipAccess(PilotPtr pilot, long num)
{
#if __ACCESS_ALL_LEVELS__
	return kAccess_OK;
#else
	
	switch(pilot->mode) {
		
		case kPilotType_Tournament:
		if((pilot->status != kStatus_Alive) && (pilot->status != kStatus_Challenge))
		return kAccess_Disabled;
		else {
			if(coreData.shipList[num].description.points[pilot->difficulty - 1] > pilot->points)
			return kAccess_NotEnoughPoints;
			else
			return kAccess_OK;
		}
		break;
		
		case kPilotType_TimeRace:
		if((pilot->status != kStatus_Alive) && (pilot->status != kStatus_Challenge))
		return kAccess_Disabled;
		else {
			if(num <= pilot->points)
			return kAccess_OK;
			else
			return kAccess_Disabled;
		}
		break;
		
		case kPilotType_Duel:
		if(pilot->status != kStatus_Alive)
		return kAccess_Disabled;
		else {
			OSType					shipID;
			
			if(GetShip_RelatedToTerrain(coreData.terrainList[pilot->points].ID, &shipID) != noErr)
			return kAccess_Disabled;
			
			if(shipID == coreData.shipList[num].ID)
			return kAccess_OK;
			else
			return kAccess_Disabled;
		}
		break;
		
		case kPilotType_FreePlay:
		case kPilotType_AddOns:
		if(pilot->status != kStatus_Alive)
		return kAccess_Disabled;
		else
		return kAccess_OK;
		break;
		
	}
#endif

	return kAccess_OK;
}

short PlayMode_GetChallengeShipAccess(PilotPtr pilot)
{
#if !__DISABLE_GBOAT_ACCESS__
	if(pilot->status != kStatus_Challenge)
	return kAccess_Disabled;
	
	switch(pilot->mode) {
		
#if !__LIMIT_CHALLENGE_TO_TOURNAMENT__
		case kPilotType_TimeRace:
		case kPilotType_Duel:
#endif
		case kPilotType_Tournament:
		return kAccess_OK;
		break;
		
#if __LIMIT_CHALLENGE_TO_TOURNAMENT__
		case kPilotType_TimeRace:
		case kPilotType_Duel:
#endif
		case kPilotType_FreePlay:
		case kPilotType_AddOns:
		return kAccess_Disabled;
		break;
		
	}
#endif
	
	return kAccess_Disabled;
}

void PlayMode_SortDataSet(OSType mode, short difficulty)
{
	long					i,
							j;
	TerrainPreloadedData	tempTerrainData;
	ShipPreloadedData		tempShipData;
	
	--difficulty;
	
	//Sort terrains by number of points - easiest first
	for(i = 0; i < coreData.terrainCount - 1; ++i)
	for(j = 0; j < coreData.terrainCount - i - 1; ++j)
	if(coreData.terrainList[j + 1].description.points[difficulty] < coreData.terrainList[j].description.points[difficulty]) {
		tempTerrainData = coreData.terrainList[j];
		coreData.terrainList[j] = coreData.terrainList[j + 1];
		coreData.terrainList[j + 1] = tempTerrainData;
	}
	
	//Sort ships by number of points - easiest first
	for(i = 0; i < coreData.shipCount - 1; ++i)
	for(j = 0; j < coreData.shipCount - i - 1; ++j)
	if(coreData.shipList[j + 1].description.points[difficulty] < coreData.shipList[j].description.points[difficulty]) {
		tempShipData = coreData.shipList[j];
		coreData.shipList[j] = coreData.shipList[j + 1];
		coreData.shipList[j + 1] = tempShipData;
	}
}

TerrainPreloadedDataPtr PlayMode_GetTerrain(OSType mode, OSType terrainID)
{
	switch(mode) {
		
		case kPilotType_Tournament:
		case kPilotType_TimeRace:
		case kPilotType_Duel:
		case kPilotType_FreePlay:
		return GetTerrainData_ByID(&coreData, terrainID);
		break;
		
		case kPilotType_AddOns:
		return GetTerrainData_ByID(&addOnData, terrainID);
		break;
		
	}
	
	return nil;
}

ShipPreloadedDataPtr PlayMode_GetShip(OSType mode, OSType shipID)
{
	ShipPreloadedDataPtr		data;
	
	switch(mode) {
		
		case kPilotType_Tournament:
		case kPilotType_TimeRace:
		case kPilotType_Duel:
		case kPilotType_FreePlay:
		return GetShipData_ByID(&coreData, shipID);
		break;
		
		case kPilotType_AddOns:
		data = GetShipData_ByID(&addOnData, shipID);
		if(data == nil)
		data = GetShipData_ByID(&coreData, shipID);
		return data;
		break;
		
	}
	
	return nil;
}

void PlayMode_FilterTerrainItems(OSType mode, TerrainPtr terrain, long visibilityMask)
{
	long				i,
						j;
	
	switch(mode) {
			
		case kPilotType_Tournament: //Keep everything
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
		
		case kPilotType_TimeRace: //Keep jokers and turbos
		case kPilotType_Duel:
		{
			for(i = 0; i < terrain->itemReferenceCount; ++i)
			if(!(terrain->itemReferenceList[i].visibility & visibilityMask) || (GetSpecialItemFromID(terrain, terrain->itemReferenceList[i].itemID)->itemType == kItem_Bonus)) {
				for(j = i + 1; j < terrain->itemReferenceCount; ++j)
				terrain->itemReferenceList[j - 1] = terrain->itemReferenceList[j];
				--terrain->itemReferenceCount;
				--i;
			}
			if(terrain->itemReferenceCount == 0)
			ItemList_Dispose(terrain->itemReferenceList);
		}
		break;
		
		case kPilotType_AddOns: //Keep turbos
		case kPilotType_FreePlay:
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
		
	}
}

Boolean PlayMode_GetHistoricEntry(PilotPtr pilot, long num, Str31 text1, Str31 text2, Str31 text3)
{
	RacePtr					race;
	TerrainPreloadedDataPtr	terrainData;
	
	if(num < 0) {
		BlockMove(kString_Interface_RaceLocation, text1, sizeof(Str31));
		switch(pilot->mode) {
			
			case kPilotType_Tournament:
			BlockMove(kString_Interface_Rank, text2, sizeof(Str31));
			BlockMove(kString_Interface_Score, text3, sizeof(Str31));
			break;
			
			case kPilotType_TimeRace:
			BlockMove(kString_Interface_TimeLeft, text2, sizeof(Str31));
			text3[0] = 0;
			break;
			
			case kPilotType_Duel:
			case kPilotType_FreePlay:
			case kPilotType_AddOns:
			BlockMove(kString_Interface_Rank, text2, sizeof(Str31));
			text3[0] = 0;
			break;
			
		}
	}
	else {
		race = &pilot->raceList[num];
		terrainData = GetTerrainData_ByID(PlayMode_GetDataSet(pilot->mode), race->location);
		if(terrainData == nil)
		return false;
		
		BlockMove(terrainData->description.realName, text1, sizeof(Str31));
		switch(pilot->mode) {
			
			case kPilotType_Tournament:
			if(race->rank == -1)
			BlockMove(kString_Interface_NA, text2, sizeof(Str31));
			else
			NumToString(race->rank, text2);
			NumToString(race->score, text3);
			break;
			
			case kPilotType_TimeRace:
			RaceTimeToString(race->leftTime, text2);
			text3[0] = 0;
			break;
			
			case kPilotType_Duel:
			case kPilotType_FreePlay:
			case kPilotType_AddOns:
			if(race->rank == -1)
			BlockMove(kString_Interface_NA, text2, sizeof(Str31));
			else
			NumToString(race->rank, text2);
			text3[0] = 0;
			break;
			
		}
	}
	
	return true;
}
