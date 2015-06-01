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


#ifndef __WATERRACE_PREFERENCES__
#define __WATERRACE_PREFERENCES__

//CONSTANTES:

#define				kPref_Version					0x0400

#define				kPref_EngineFlagUseSoftware		(1 << 0)
#define				kPref_EngineFlagLowResolution	(1 << 1)
#define				kPref_EngineFlagFiltering		(1 << 16)
#define				kPref_EngineFlagVideoFX			(1 << 17)
#define				kPref_EngineFlag32BitRendering	(1 << 18)
#define				kPref_EngineFlag32BitZBuffer	(1 << 19)
#define				kPref_EngineFlagFurtherClipping	(1 << 20)
#define				kPref_EngineFlagTriFiltering	(1 << 21)

#define				kPref_SoundFlagGameMusic		(1 << 0)

#define				kPref_NetworkFlagBroadCast		(1 << 0)

#define				kPref_ControlFlagMouseOn		(1 << 0)

#define				kNbHighScores					10

//STRUCTURES:

typedef struct Preferences_Definition
{
	unsigned long	version;
	unsigned long	textureFlags,
					engineFlags,
					soundFlags,
					controlFlags;
	Str255			raveEngineName;
	
	DisplayIDType	screenID;
	VDSwitchInfoRec	interfaceScreen,
					gameScreen;
	short			gameWidth,
					gameHeight,
					gameDepth;
					
	short			renderWidth,
					renderHeight;
	short			musicVolume,
					soundFXVolume;
	
	short			unused; //ambientVolume
					
	Str31			playerName,
					playerLocation,
					hostName,
					hostPassword;
	Str255			hostMessage;
	unsigned long	networkFlags;
	short			unused2;
	
	unsigned long	runningTime;
	
	Str31			lastNetworkServerIP;
	unsigned short	lastPort;
};
typedef Preferences_Definition Preferences;
typedef Preferences_Definition* PreferencesPtr;

//VARIABLES:
extern Preferences			thePrefs;

//ROUTINES:

//File: Preferences.cp	
OSErr Preference_Read();
void Preference_Write();
void Preferences_Set(PreferencesPtr prefs);

#endif
