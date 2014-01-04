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


#ifndef __WATERRACE_CONDITIONALS__
#define __WATERRACE_CONDITIONALS__

/*********** PRECOMPILED HEADERS ************/

#ifdef __cplusplus
#include	<MacHeadersPPC++>
#endif


/*********** CONSTANTS ************/

//Language constants
#define	kLanguage_English					1
#define	kLanguage_French					2
#define	kLanguage_German					3
#define	kLanguage_Italian					4

//Unit system constants
#define	kUnit_Meters						1
#define	kUnit_Miles							2


/********** INTERNATIONAL OPTIONS ********/

//Language for text and so on
#define	__LANGUAGE__						kLanguage_English

//Unit system
#define	__UNIT_SYSTEM__						kUnit_Meters


/*********** RENDERING OPTIONS ************/

//Render checkpoints on terrain
#define	__RENDER_CHECK_POINTS__	1

//Use Level of Detail for model rendering
//#define	__ENABLE_LOD__	1


/*********** HUD OPTIONS ************/

//Render map in the ship HUD
#define	__RENDER_MAP__	1

//Rotate the map according to ship direction
#define	__ROTATING_MAP__	1

//Draw a one pixel black drop shadow behind the map - works if __ROTATING_MAP__ is 1
//#define	__DROP_SHADOW_MAP__	1

//Simplify map to check points only
//#define	__RENDER_ONLY_CHECKPOINTS_IN_MAP__	1

//Render ship name in the HUD
//#define	__RENDER_SHIP_NAME__	1

//Render camera names in the HUD - causes translation problems
//#define	__RENDER_CAMERA_NAMES__	1

//Flashing turbo in the HUD
#define	__FLASHING_TURBO__	1


/********** GAME OPTIONS ******************/

//Enable the use of the mouse for ship control
//#define	__ENABLE_MOUSE_CONTROL__	1

//Enable the use of axis based devices (joysticks...) for ship control
#define	__ENABLE_AXIS_CONTROL__	1

//Display "CHECK POINT" when ship passes a check point
#define	__DISPLAY_CHECK_POINT__	1

//Display "MISSED CHECK POINT" when ship misses a check point
#define	__DISPLAY_MISSED_CHECK_POINT__	1

//Display "WRONG DIRECTION" when ship passes check point in wrong direction
#define	__DISPLAY_WRONG_DIRECTION__	1

//Adjust local player starting position according to difficulty level
#define	__ADJUST_LOCAL_PLAYER_POSITION__	1

//Limit race time
#define	__LIMIT_RACE_TIME__	1

//Display score screen with characters
#define	__DISPLAY_SCORE_SCREEN__	1

//TV Camera follows automatically first ship
#define	__TV_TARGET_FIRST__	1

//Autopilot ships when they have finished the race
#define	__AUTOPILOT__	1

//No collision car occur with ships which have finished the race or are offline
//#define	__NO_COLLISION_WITH_FINISHED__	1

//Limit Leah's level access to tournament mode only
#define	__LIMIT_CHALLENGE_TO_TOURNAMENT__	1

//Disable GBoat access for human player
#define	__DISABLE_GBOAT_ACCESS__	1


/********** BOTS OPTIONS ******************/

//Add bots to local games
#define	__ENABLE_BOTS__	1

//Bots can pickup turbos - if set to 1, bot begin with all turbos picked up
//#define	__BOTS_CAN_PICKUP_TURBOS__	1

//Adjust bots number of turbos according to difficulty level - __BOTS_CAN_PICKUP_TURBOS__ must be 1
#define	__BOTS_ADJUST_TURBOS__	1

//Bots detect missed check points
#define	__BOTS_DETECT_MISSED_CHECK_POINTS__	1

//Bots use improved turbo use detection
#define	__BOTS_IMPROVED_TURBOS__	1

//Use turbo depending on where local player is
#define	__BOTS_SMART_TURBOS__	1

//Adjust bots trajectory according to difficulty level
#define	__BOTS_ADJUST_TRAJECTORY__	1

//Allow bots to finish race after local player has finished
#define	__BOTS_FINISH_RACE__	1

//Bots wait for local player
#define	__BOTS_ADAPTATIVE_AI__	1


/********* PHYSIC ENGINE OPTIONS ************/

//Use trigonometric Look Up Tables for computations
#define	__USE_TRIG_TABLES__	1

//Limit ship rotation speed
#define	__LIMIT_ROTATION_SPEED__	1

//Use sliding collision response for ship collision
#define	__SLIDING_SHIP_COLLISIONS__	1

//Rebound if hard collision between ships
//#define	__REBOUND_SHIP_COLLISIONS__	1

//Try to preserve ship speed during collision
#define	__IMPROVED_SHIP_COLLISIONS__	1

//Check if collision occurs in network physic
//#define	__CHECK_NETWORK_SHIP_COLLISION__	1

//Rotate rudders smoothly
#define	__SLOW_RUDDERS__	1

//Stop turbo if reverse engine
#define	__STOP_TURBO_ON_REVERSE__	1


/*********** NETWORKING OPTIONS *************/

//Server sends clock time
#define	__SYNCHRONIZE_NETWORK_GAME__	1

//When clients receive timestamp for server, set client clock only if offset significant
//#define	__CLOCK_TOLERANCE__		1

//Force the use of TCP/IP default packet size
//#define	__USE_SEGMENT_SIZE_PACKETS__	1


/*********** REPLAY OPTIONS ******************/

//Enable race recording and replay
#define	__RECORD_RACE__		1

//Watch for Apple High-Level Events to open replay files at startup
#define	__REPLAY_AWARE__	1

//When replaying, allow slow motion and fast forward
#define	__VARIABLE_SPEED_REPLAY__	1


/********** INTERFACE OPTIONS **************/

//Show a spinning cursor instead of hiding the cursor
#define	__SHOW_SPINNING_CURSOR__	1

//Completely delete the default name in New pilot screen when user presses a key
#define	__REPLACE_NAME__	1


/********** DATA SYSTEM OPTIONS **************/

//Skip data files that begin with "_"
#define	__ENABLE_DATAFILE_SKIPPING__	1

//Watch for official WR files
#define	__PREVENT_FILE_HACKING__	1

//Watch for modified pilot files
#define	__PREVENT_PILOT_HACKING__	1

//Move pilot files to trash instead of deleting them
#define	__MOVE_PILOT_TO_TRASH__		0


/*********** DEMO OPTIONS ******************/

//Fade in / fade out before demo starts
//#define	__FADE_BEFORE_DEMO__	1

//Render demo in 32 Bits - Warning: mess up the screen
//#define	__32_BITS_RENDERING__	1

//Display fly-through demo
//#define	__PLAY_FLY_THROUGH__	1

//Click on mouse to stop demo
#define	__MOUSE_STOPS_DEMO__	1

//Use keyboard to stop demo
#define	__KEYBOARD_STOPS_DEMO__	1

//Play FT jingle while displaying FT logo
#define	__PLAY_JINGLE__	1

/*********** SYSTEM OPTIONS ******************/

//If required track not found, use first one available
#define	__DEFAULT_TRACK__	1

//Auto-reset screen settings when force quit
#define	__SECURE_CRASH__	1

//Enable fade in /fade out
#define	__SCREEN_FADES__	1

//Fade to black
#define	__FADE_ALL_SCREENS_ON_STARTUP__	0

//Fade to a white screen after race results
#define	__FADE_TO_WHITE_AFTER_RACE__	1

//Force quit on serious error
#define	__FORCE_QUIT_ON_ERROR__	1

//Check for Rave 1.6
#define	__CHECK_FOR_RAVE_1_6__	1

//Check for minimal ATI Drivers
#define	__CHECK_FOR_ATI_DRIVERS__	1

//Check for minimal 3Dfx Drivers
#define	__CHECK_FOR_3Dfx_DRIVERS__	1

//Display a loading screen on 3Dfx cards
#define	__DISPLAY_3DFX_LOADING_SCREEN__	1

//Enable MacOS X Classic Workarounds
#define	__MACOS_X_CLASSIC__	1

#endif
