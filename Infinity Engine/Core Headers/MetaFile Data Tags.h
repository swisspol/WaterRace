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


#ifndef __INFINITY_METAFILE_DATA_TAGS__
#define __INFINITY_METAFILE_DATA_TAGS__

//CONSTANTES:

//Generic Data subtags
#define					kSubTag_Sound					'snd '
#define					kSubTag_CompressedSound			'csnd'
#define					kSubTag_Image					'imag'
#define					kSubTag_Copyright				'copy'
#define					kSubTag_LockedCopyright			'lcpy'
#define					kSubTag_SubTitleBank			'stbk'
#define					kSubTag_Version					'vers'

//WaterRace Data subtags
#define					kSubTag_ShipDescription			'shds'
#define					kSubTag_TerrainDescription		'trds'
#define					kSubTag_CharacterDescription	'chds'
#define					kSubTag_ShipPhysicParameters	'shpp'

//Other Data subtags
#define					kSubTag_JavaScript				'jvst'

//Language codes
#define					kLanguageCode_English			'Engl'
#define					kLanguageCode_French			'Frch'
#define					kLanguageCode_German			'Grmn'
#define					kLanguageCode_Italian			'Ital'

//Data constants
#define					kMax_SubTitleChars				128
#define					kMax_DescriptionChars			256

//STRUCTURES CONSTANTES:

#define					kPhysicParameters_Version		0x0200

enum {kWaterPropellor = 0, kAirPropellor, kWaterJet};
enum {kFixedPropulsion = 0, kDirectionalPropulsion};
enum {kFixedTurbo = 0, kDirectionalTurbo};
enum {kNoRudder = 0, kRudder};

#define					kBotFlag_UseTurboInAir			(1 << 0)
#define					kBotFlag_WatchForTurboCollision	(1 << 2)

//DATA STRUCTURES:

typedef struct {
	Str255				copyright;
} Data_Copyright;
typedef Data_Copyright* Data_CopyrightPtr;

typedef	unsigned char SubTitle[kMax_SubTitleChars];

typedef struct {
	unsigned short		version,
						subVersion;
	Str63				note;
	long				unused[4];
} Data_Version;
typedef Data_Version* Data_VersionPtr;

typedef struct {
	unsigned long		subTitleCount;
	SubTitle			subTitleList[];
} Data_SubTitles;
typedef Data_SubTitles* Data_SubTitlesPtr;

typedef ExtSoundHeader* Data_SoundPtr;

typedef struct {
	Str31			realName;
	unsigned char	description[kMax_DescriptionChars],
					notes[kMax_DescriptionChars];
	unsigned char	unused1[5];
	long			points[3];
	
	OSType			languageCode;
	
	unsigned char	unused2[500];
} Data_ShipDescription;
typedef Data_ShipDescription* Data_ShipDescriptionPtr;

typedef struct {
	Str31			realName;
	unsigned char	description[kMax_DescriptionChars],
					notes[kMax_DescriptionChars];
	unsigned char	unused1[258];
	Str63			musicName;
	long			points[3],
					parTime[3];
	short			location_H,
					location_V;
	
	short			maxNetworkPlayers;
	
	OSType			languageCode;
	
	unsigned char	unused2[490];
} Data_TerrainDescription;
typedef Data_TerrainDescription* Data_TerrainDescriptionPtr;

typedef struct {
	Str31			realName;
	unsigned char	description[kMax_DescriptionChars];
	unsigned char	unused1[256];
	unsigned char	age;
	
	OSType			terrainID,
					shipID,
					textureSetID;
	Str63			musicName;
	Str31			origin;
	
	OSType			languageCode;
	
	unsigned char	unused2[476];
} Data_CharacterDescription;
typedef Data_CharacterDescription* Data_CharacterDescriptionPtr;

typedef struct {
	unsigned short		version;
	
	long				propulsionType,
						propulsionSteering,
						turboSteering,
						rudderType;
	
	float				area_frontal,
						area_lateral,
						area_floor;
	float				Cx_H,
						Cx_L,
						Cx_V,
						Cw,
						Cf;
	
	float				archimedeCorrectionFactor_front,
						archimedeCorrectionFactor_back;
	
	float				area_rotationnal;
	float				Cx_R;
	
	float				mass;
	float				inertia_roll,
						inertia_pitch,
						inertia_yaw;
	
	float				power_Max,
						power_Turbo,
						power_MaxReverse,
						power_ChangePerSec,
						power_idle;
						
	Vector				propulsion_position;
	float				propulsion_MaxRotate,
						propulsion_Angle;
	
	Vector				turbo_position;
	float				turbo_MaxRotate,
						turbo_Angle;
						
	unsigned long		flags;
	
	Vector				propulsion_fakePosition;
	Vector				turbo_fakePosition;
	
	Vector				rudder_position;
	float				rudder_width,
						rudder_MaxRotate;
	
	Vector				rudder_fakePosition;
	float				rudder_height;
	
	float				stabilityCorrection;
	
	long				turbo_duration;
	
	float				power_MaxSoundPitchRatio,
						turbo_SoundPitch;
	
	float				wakeHalfWidth,
						wakeMinDistance,
						wakeLength,
						wakeShipInWaterOffset,
						wakePositionOffset;
	unsigned long		wakeLifeTime;
	
	unsigned long		bot_flags;
	
	float				bot_Turn_Target,
						bot_Turn_TargetToleranceAngle, //convert to cos
						bot_Turn_MinTargetAngle, //convert to cos
						bot_Turn_Max,
						bot_Turn_Opposite,
						bot_Turn_MaxRotationSpeed;
						
	float				bot_Power_Max,
						bot_Power_Min,
						bot_Power_LowPowerTurnAngle, //convert to cos
						bot_Power_InAir;
						
	float				bot_Turbo_TargetToleranceAngle, //convert to cos
						bot_Turbo_MinDistance,
						bot_Turbo_MaxAngle, //convert to cos
						bot_Turbo_CollisionDistance,
						bot_Turbo_CollisionAngle; //convert to cos
	
	float				bot_Anticipation;
	
	float				autoPilotThrottle;
	
	float				power_MinSoundPitchRatio,
						power_SoundVolume;
	
	unsigned char		unused[376];
} Data_ShipPhysicParameter;
typedef Data_ShipPhysicParameter* Data_ShipPhysicParameterPtr;

#endif
