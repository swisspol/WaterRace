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


#ifndef __WATERRACE_PILOTS__
#define __WATERRACE_PILOTS__

//CONSTANTES:

#define				kPilot_Version					0x0104
#define				kPilotFileType					'Plot'

#define				kPilotType_Tournament			'Trnm'
#define				kPilotType_TimeRace				'TmRc'
#define				kPilotType_Duel					'Duel'
#define				kPilotType_FreePlay				'FrPl'
#define				kPilotType_AddOns				'AdOn'

#define				kMaxRaces						16
#define				kMaxOpponents					8

#define				kPilot_MaxNameSize				8

#define				kStatus_Alive					'Live'
#define				kStatus_Dead					'Dead'
#define				kStatus_Challenge				'Chlg'
#define				kStatus_Champion				'Chmp'

//STRUCTURES:

typedef struct Race_Definition
{
	OSType			location;
	unsigned long	totalTime,
					bestLapTime,
					leftTime;
	long			score,
					rank;
	float			maxSpeed,
					totalDistance;
};
typedef Race_Definition Race;
typedef Race_Definition* RacePtr;

typedef struct Pilot_Definition
{
	unsigned long	version,
					flags;
	
	//Pilot info
	Str31			name;
	OSType			character;
	OSType			mode;
	short			difficulty;
	unsigned long	creationDate,
					lastPlayDate,
					playTime;
					
	//Championship data
	OSType			status;
	long			points,
					numJokers;
	OSType			opponents[kMaxOpponents];
	
	//Private data
	unsigned long	refCon1,
					refCon2;
	
	//Some space
	unsigned char	unused[512];
	
	//Global data
	unsigned long	raceCount;
	Race_Definition	raceList[kMaxRaces];
};
typedef Pilot_Definition Pilot;
typedef Pilot_Definition* PilotPtr;

//ROUTINES:

//File: Pilots.cp	
void Pilot_New(PilotPtr pilot, Str31 name, OSType character, OSType mode, short difficulty);
OSErr Pilot_Save(PilotPtr pilot);
OSErr Pilot_Load(Str63 name, PilotPtr pilot);
OSErr Pilot_Delete(Str63 name);
void Pilot_Update(PilotPtr pilot);
RacePtr Pilot_GetRaceOnLocation(PilotPtr pilot, OSType locationID);

#endif

