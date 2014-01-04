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


#include				<OpenTransport.h>

#include				"Infinity Structures.h"
#include				"Clock.h"

//VARIABLES LOCALES:

static unsigned long	stepTime = 0,
						timeStep = 4; //1/25th of a second

//ROUTINES:

unsigned long StepClock_GetTime()
{
	return stepTime;
}

unsigned long StepClock_UpdateTime()
{
	stepTime += timeStep;

	return stepTime;
}

void StepClock_SetTime(unsigned long time)
{
	stepTime = time;
}

void StepClock_SetStep(unsigned long step)
{
	timeStep = step;
}
