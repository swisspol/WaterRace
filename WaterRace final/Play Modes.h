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


#ifndef __WATERRACE_PLAY_MODES__
#define __WATERRACE_PLAY_MODES__

//CONSTANTES:

enum {
	kAccess_Disabled = -3,
	kAccess_NeedJoker = -2,
	kAccess_NotEnoughPoints = -1,
	kAccess_OK = 1,
	kAccess_OK_Joker = 2
};

//PROTOTYPES:

//File: Local Play Modes.cp
void PlayMode_GetName(OSType mode, Str31 name);
OSType PlayMode_GetLeahAnim(OSType mode);
void PlayMode_GetDefaultBonus(OSType mode, short difficulty, long* points, long* numJokers);
void PlayMode_UpdatePilot(PilotPtr pilot, PlayerConfigurationPtr player);
void PlayMode_GetStatus(PilotPtr pilot, Str31 status);
void PlayMode_GetNext(OSType* mode);
OSErr PlayMode_Practice_SetUpPlayers(PilotPtr pilot, OSType localPlayerShipID);
OSErr PlayMode_Local_SetUpPlayers(PilotPtr pilot, OSType localPlayerShipID);
Boolean PlayMode_PointsAvailable(OSType mode);
DataSetPtr PlayMode_GetDataSet(OSType mode);
Boolean PlayMode_JokersAvailable(OSType mode);
short PlayMode_GetLocationAccess(PilotPtr pilot, long num);
short PlayMode_GetChallengeLocationAccess(PilotPtr pilot);
short PlayMode_GetShipAccess(PilotPtr pilot, long num);
short PlayMode_GetChallengeShipAccess(PilotPtr pilot);
void PlayMode_SortDataSet(OSType mode, short difficulty);
TerrainPreloadedDataPtr PlayMode_GetTerrain(OSType mode, OSType terrainID);
ShipPreloadedDataPtr PlayMode_GetShip(OSType mode, OSType shipID);
void PlayMode_FilterTerrainItems(OSType mode, TerrainPtr terrain, long visibilityMask);
Boolean PlayMode_GetHistoricEntry(PilotPtr pilot, long num, Str31 text1, Str31 text2, Str31 text3);
void PlayMode_CheckIfPilotStuck(PilotPtr pilot, PlayerConfigurationPtr player);
void PlayMode_CheckForChallenge(PilotPtr pilot, PlayerConfigurationPtr player);

#endif
