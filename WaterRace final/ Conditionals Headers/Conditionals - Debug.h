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


#ifndef __WATERRACE_CONDITIONALS_DEBUG__
#define __WATERRACE_CONDITIONALS_DEBUG__

#include	"Conditionals.h"


/********* PATCHED CONDITIONALS ***********/

#undef __SCREEN_FADES__


/********** OPTIONNAL CONDITIONALS **********/

//Render the physic volumes of the ships
#define	__RENDER_SHIP_PHYSIC_VOLUMES__	1

//Render the track on the terrain
#define	__RENDER_TRACK__	1

//Render ship bot target
#define	__RENDER_BOT_TARGET__	1

//Render ship bounding boxes
#define	__RENDER_SHIP_BBOX__	1

//Render model bounding boxes according to collision map
#define	__RENDER_MODEL_BBOX__	1

//Render collision enclosures
#define	__RENDER_ENCLOSURE__	1

//Render the normal used to compute collision movement
//#define	__RENDER_SHIP_COLLISION_NORMAL__	1

//Profile with InstrumentationLib
//#define	__PROFILE__	1

//Patch preferences settings (1=1024x768, 2=640x480, 3=512x384, 4=600x400 low res)
//#define	__PATCH_PREFERENCES__				1

//Display FPS
#define	__DISPLAY_FPS__	1

//One lap races
//#define	__ONE_LAP__	1

//Enable play on all locations and ships in browser
//#define	__ACCESS_ALL_LEVELS__	1

//Look for a "WaterRace.ini" file at startup
#define	__USE_INI__	1


/********** HEADERS **********/

#ifdef __cplusplus
#if __PROFILE__
#include				<InstrumentationMacros.h>
#endif
#endif

#endif
