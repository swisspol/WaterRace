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


#ifndef __WATERRACE_GAME__
#define __WATERRACE_GAME__

#include				"MetaFile Data Tags.h"
#include				"Infinity Rendering.h"

#include				"Physic Engine.h"

//CONSTANTES:

#define					kNetGameVersion			0x0100

#define					kMaxModels				128

#define					kShipScale				1.0

#define					kNoFlyThrough			(-1)

#define					kGameFXStartID			500

#define					kInfoTextDuration		(kTimeUnit * 4)

#define					kMaxReplayEvents		7500 //5mn - WARNING: max = 10mn
#define					kReplayEventDistance	4 //100/4=25Hz => 53Kb/s
#define					kMaxReplayTime			(kMaxReplayEvents * kReplayEventDistance)

#define					kShortMax				32000.0
#define					kPowerUnitToWatt		740.0

enum {
	kGameFX_GameStart = 0,
	kGameFX_Bonus,
	kGameFX_Turbo,
	kGameFX_Joker,
	kGameFX_Collision,
	kGameFX_Finish,
	kGameFX_CountDown,
	kGameFX_CheckPoint,
	KGameFX_NewLap,
	KGameFX_BestLap,
	kGameFX_FirstPlace,
	kGameFX_Error,
	kNbGameFX
};

//Max values
#define					kMaxPlayers				8
#define					kRaceCountDown			(kTimeUnit * 4)
#define					kMaxRaceTime			(kTimeUnit * 60 * 60) //1h
#define					kMaxPilots				64

//Special ship shapes
#define					kShapeID_PilotHead		'Head'
#define					kShapeID_PilotBody		'Body'
#define					kShapeID_Propeller_1	'Prp1'
#define					kShapeID_Propeller_2	'Prp2'
#define					kShapeID_Propeller_3	'Prp3'
#define					kShapeID_Propeller_4	'Prp4'
#define					kShapeID_Rudder_1		'Rdr1'
#define					kShapeID_Rudder_2		'Rdr2'
#define					kShapeID_TurboFlame_1	'Flme'
#define					kShapeID_TurboFlame_2	'Flm2'

//Special ship scripts
#define					kScriptID_Autorun		'play'
#define					kScriptID_Turbo			'TbOn'
#define					kScriptID_TurboReverse	'TbOf'

//Special sound IDs
#define					kSoundID_Engine			'Engn'
#define					kSoundID_Turbo			'Trbo'

//Special track IDs
#define					kTrackID_Easy			'Easy'
#define					kTrackID_Medium			'Medi'
#define					kTrackID_Hard			'Hard'

//Game modes
#define					kGameMode_Local			'Locl'
#define					kGameMode_Practice		'Prtc'
#define					kGameMode_Host			'Host'
#define					kGameMode_Join			'Join'
#define					kGameMode_BroadCastTV	'BdTV'
#define					kGameMode_Replay		'Rply'
#define					kGameMode_Demo			'Demo'

//Game sub-modes
#define					kGameSubMode_None					(-1)
#define					kGameSubMode_Net_SelectShip			'StSh'
#define					kGameSubMode_Net_ForceShip			'FcSh'
#define					kGameSubMode_Local_Tournament		kPilotType_Tournament
#define					kGameSubMode_Local_TimeRace			kPilotType_TimeRace
#define					kGameSubMode_Local_Duel				kPilotType_Duel
#define					kGameSubMode_Local_FreePlay			kPilotType_FreePlay
#define					kGameSubMode_Local_AddOns			kPilotType_AddOns

//Ship pilots
#define					kPilot_Local			'Lcal'
#define					kPilot_Bot				'Bot '
#define					kPilot_Remote			'Rmte'
#define					kPilot_RemoteOffLine	'RmOf'
#define					kPilot_Replay			'Rply'

//Difficulty
enum {
	kDifficulty_Easy = 1,
	kDifficulty_Medium,
	kDifficulty_Hard
};

//Network status
enum {
	kStatus_OffLine = 0,
	kStatus_OnLine,
	kStatus_Locked
};

//Collision map
#define					kCollisionMapResolution	15.0
#define					kCollisionMapTolerance	5.0
#define					kMaxReferences			64

//Wake effect
#define					kWakeNumSprites			3
#define					kWakeHeight				theTerrain.waveAmplitude

//MACROS:

#define	WaveHeight(t, x, z, sx, sz, a) ((TCos((z) / sz + DegreesToRadians((t))) + TSin((x) / sx + DegreesToRadians((t)))) / 2.0 * (a))

//STRUCTURES:

typedef struct Pilot Pilot;
typedef Pilot* PilotPtr;

typedef struct ReplayEvent
{
	unsigned short		timeStamp;
	short				motorPower;
	short				mouseH; //Conversion needed
	Boolean				turboState;
	Vector				position;
	Quaternion			orientation;
};
typedef ReplayEvent* ReplayEventPtr;

typedef struct PlayerConfiguration_Definition
{
	//Player info - Must be set before new game
	OSType					characterID,
							shipID,
							shipTextureID,
							shipPilotType;
	Str31					playerName;
	Ptr						playerDataPtr; //pointer to Pilot (if local player) or CharacterDescription (if bot) or nil (if network player)
	
	//Player race info - Set by the game when the race is done - READ ONLY
	long					score,
							lapCount,
							rank;
	short					turboCount;
	Boolean					hasJoker;
	unsigned long			bestLapTime,
							lastLapTime,
							finishTime;
	float					speed,
							maxSpeed,
							totalDistance;
	
	//Game specific data - Real-time data used by the game
	ExtendedShipPtr			playerShip;
	Str31					infoText;
	unsigned long			infoMaxTime;
	Str31					networkText;
	Boolean					finished;
};
typedef PlayerConfiguration_Definition PlayerConfiguration;
typedef PlayerConfiguration_Definition* PlayerConfigurationPtr;

typedef struct GameConfiguration_Definition
{
	//Game info - Must be set before new game
	short					version;
	OSType					gameMode,
							gameSubMode;
	OSType					gameMode_save,
							gameSubMode_save;
	OSType					terrainID;
	short					terrainDifficulty;
	
	//Race info - Set by the game when the race is done - READ ONLY
	unsigned long			startTime,
							globalBestLapTime,
							raceLength;
							
	//Temp game info
	unsigned long			expiredTime; //Used in "Time race" mode only
	
	//Player info - Must be set before new game
	long					playerCount;
	PlayerConfiguration		playerList[kMaxPlayers];
};
typedef GameConfiguration_Definition GameConfiguration;
typedef GameConfiguration_Definition* GameConfigurationPtr;

typedef struct ExtendedShip_Definition
{
	//3D geometry
	MegaObject				megaObject;
	ShapePtr				headShape,
							bodyShape,
							rudderShape_1,
							rudderShape_2,
							propellerShape_1,
							propellerShape_2,
							propellerShape_3,
							propellerShape_4,
							flameShape_1,
							flameShape_2;
	
	//Animation
	ScriptPtr				turbo,
							reverseTurbo,
							autoRun,
							currentScript;
	
	//Sounds
	ExtSoundHeaderPtr		engineSound,
							turboSound;
	long					engineSoundRef;
	
	//Wake effect
	Vector					wakePositions[kWakeNumSprites],
							wakeDirections[kWakeNumSprites];
	unsigned long			wakeTime[kWakeNumSprites];
	Vector					wakeLastPosition;
	
	//Driver
	PlayerConfigurationPtr	player;
	ProcPtr					DriverShipRoutine;
	float					mouseH,
							mouseV;
	Boolean					turboButton,
							turboState,
							turboFlame;
	unsigned long			turboStartTime;
	long					nextDoor,
							lastCheckPoint,
							nextCheckPoint;
	unsigned long			checkPointTime;
	Vector					lastPos;
	Boolean					inCheckPoint;
	unsigned long			motorStartTime;
#if __RENDER_BOT_TARGET__
	Vector					botTarget;
#endif

	//Network
	short					shipNum;
	
	//Physics
	Data_ShipPhysicParameterPtr	params;
	Dynamics				dynamics;
	
	//Terrain collision
	float					center1,
							center2,
							radius;
	
#if __RECORD_RACE__
	//Recording data
	unsigned long			eventCount,
							lastEventNum;
	ReplayEventPtr			eventBuffer;
#endif

#if __RENDER_SHIP_COLLISION_NORMAL__
	Vector					normal;
#endif
};
typedef ExtendedShip_Definition ExtendedShip;
typedef ExtendedShip_Definition* ExtendedShipPtr;

typedef struct ExtendedModel_Definition
{
	MegaObject				megaObject;
	Boolean					hasScripts;
	unsigned long			lastScriptUpdateTime;
};
typedef ExtendedModel_Definition ExtendedModel;
typedef ExtendedModel_Definition* ExtendedModelPtr;

typedef struct {
	long				referenceCount;
	ModelReferencePtr	referenceList[kMaxReferences];
} CollisionTile;
typedef CollisionTile* CollisionTilePtr;

typedef struct {
	Boolean				isCheckPoint;
	long				nextCheckPoint;
	
	Vector				rightMark,
						leftMark,
						middle;
						
	float				squareDoorWidth,
						doorWidth;
	Matrix				doorMatrix,
						negatedDoorMatrix;
						
	float				doorAngle,
						distanceToNextDoor;
	Boolean				turningRight;
} RaceDoor;
typedef RaceDoor* RaceDoorPtr;

typedef struct {
	long				lapNumber;
	long				maxDoorsBetweenCheckPoints;
	
	long				doorCount;
	RaceDoor			doorList[];
} RaceDescription;
typedef RaceDescription* RaceDescriptionPtr;

//VARIABLES:

extern StatePtr					worldState;
extern CameraState				worldCamera;
extern ModelAnimationPtr		flyThroughCameraAnimation;

extern Terrain					theTerrain;
extern ShapePtr					skyShape;
extern RaceDescriptionPtr		theRace;
extern long						modelCount;
extern ExtendedModel			modelList[kMaxModels];
extern TextureMemory			videoMemory;
extern TQATexture				*turboTexture,
								*jokerTexture,
								*wakeTexture;
extern Boolean					videoFX;

extern long						shipCount;
extern ExtendedShip				shipList[kMaxPlayers];
extern ShapePtr					arrowShape;
#if __RENDER_CHECK_POINTS__
extern ShapePtr					checkPoint_left,
								checkPoint_middle,
								checkPoint_right;
extern ShapePtr					start_left,
								start_middle,
								start_right;
#endif

extern float					terrain_MinX,
								terrain_MaxX,
								terrain_MinZ,
								terrain_MaxZ;
extern float					border_MinX,
								border_MaxX,
								border_MinZ,
								border_MaxZ;
extern CollisionTilePtr			collisionMap;
extern long						collisionMapWidth,
								collisionMapHeight;

extern ExtendedShipPtr			localShip;
extern GameConfiguration		gameConfig;
extern Handle					gameFXSounds[kNbGameFX];
extern Boolean					run,
								forceQuit;

//INLINE ROUTINES:

#define	Digit2Char(d) ('0' + (d))

inline void FastNumToString1(long num, unsigned char* text)
{
	*text = Digit2Char(num);
}

inline void FastNumToString2(long num, unsigned char* text)
{
	if(num > 9) {
		*text++ = Digit2Char(num / 10);
		*text = Digit2Char(num % 10);
	}
	else {
		*text++ = '0';
		*text = Digit2Char(num);
	}
}

inline void FastNumToString3(long num, unsigned char* text)
{
	if(num > 99) {
		*text++ = Digit2Char(num / 100);
		num %= 100;
		*text++ = Digit2Char(num / 10);
		*text = Digit2Char(num % 10);
	}
	else if(num > 9) {
		*text++ = '0';
		*text++ = Digit2Char(num / 10);
		*text = Digit2Char(num % 10);
	}
	else {
		*text++ = '0';
		*text++ = '0';
		*text = Digit2Char(num);
	}
}

inline unsigned char FastNumToString4(long num, unsigned char* text)
{
	if(num > 999) {
		*text++ = Digit2Char(num / 1000);
		num %= 1000;
		*text++ = Digit2Char(num / 100);
		num %= 100;
		*text++ = Digit2Char(num / 10);
		*text = Digit2Char(num % 10);
		return 4;
	}
	else if(num > 99) {
		*text++ = Digit2Char(num / 100);
		num %= 100;
		*text++ = Digit2Char(num / 10);
		*text = Digit2Char(num % 10);
		return 3;
	}
	else if(num > 9) {
		*text++ = Digit2Char(num / 10);
		*text = Digit2Char(num % 10);
		return 2;
	}
	else {
		*text = Digit2Char(num);
		return 1;
	}
}

inline void RaceTimeToString(unsigned long time, Str31 text)
{
	if(time / (60 * kTimeUnit) < 10) {
		text[0] = 7;
	
		//Minutes
		FastNumToString1(time / (60 * kTimeUnit), &text[1]);
		text[2] = ':';
		time %= (60 * kTimeUnit);
		
		//Seconds
		FastNumToString2(time / kTimeUnit, &text[3]);
		text[5] = '\'';
		time %= kTimeUnit;
		
		//Hundreds
		FastNumToString2(time, &text[6]);
	}
	else {
		text[0] = 8;
	
		//Minutes
		FastNumToString2(time / (60 * kTimeUnit), &text[1]);
		text[3] = ':';
		time %= (60 * kTimeUnit);
		
		//Seconds
		FastNumToString2(time / kTimeUnit, &text[4]);
		text[6] = '\'';
		time %= kTimeUnit;
		
		//Hundreds
		FastNumToString2(time, &text[7]);
	}
}

//PROTOTYPES:

//File: Game.cp
OSErr Game_New();
OSErr Game_SetupLocal(OSType gameMode, PilotPtr pilot, OSType terrainID, OSType shipID);
void Game_UpdateRank();

//File: Drawing.cp
void Draw_Frame(StatePtr localState);
void GameOverText(StatePtr state);

//File: World.cp
OSErr World_Load(StatePtr state, FSSpec* terrainFile, long level, OSType flyThroughID);
void World_InstallAudio();
void World_Prepare();
OSErr World_InstallShip(FSSpec* shipFile, PlayerConfigurationPtr pilotConfig, VectorPtr position, float rotateY);
void World_Unload();
void World_UpdateSea();

//File: Terrain rendering.cp
void SeaMesh_RenderModels(SeaMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos);
void Mesh_RenderModels(TerrainMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos);
void Render_SpecialItems(TerrainPtr terrain, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos);

//File: Local Infinity Utils.cp
void Terrain_Resolve(TerrainPtr terrain, TextureMemoryPtr vram, long visibilityMask, OSType flyThroughID);
OSErr Terrain_ExtractPreview(FSSpec* theFile, PicHandle* daPic);
SpecialItemPtr GetSpecialItemFromID(TerrainPtr terrain, OSType id);

//File: Utils.cp
void Clear_Player(PlayerConfigurationPtr player);
void CopyFirstName(Str31 source, Str31 dest);

//File: Replay.cp
void Replay_Reset();
void Replay_Init();
void Replay_Quit();
OSErr Replay_Save(FSSpec* destFile);
OSErr Replay_SetupGame(FSSpec* file);

#endif

