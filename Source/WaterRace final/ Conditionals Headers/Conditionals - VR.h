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


#ifndef __WATERRACE_CONDITIONALS_VR__
#define __WATERRACE_CONDITIONALS_VR__

#include	"Conditionals.h"


/********* PATCHED CONDITIONALS ***********/

#undef __SCREEN_FADES__


/********** OPTIONNAL CONDITIONALS **********/

//Display FPS
#define	__DISPLAY_FPS__	1

//Look for a "WaterRace.ini" file at startup
#define	__USE_INI__	1

//VR Mode
#define	__VR_MODE__	1

#endif
