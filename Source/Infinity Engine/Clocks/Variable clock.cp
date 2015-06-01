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

static unsigned long	clockRate = kTimeUnit;
static unsigned long	timeOffset = 0;

//ROUTINES:

unsigned long VClock_GetTime()
{
	return GetCurrentAbsoluteTime(clockRate) - timeOffset;
}

void VClock_SetTime(unsigned long time)
{
	timeOffset = GetCurrentAbsoluteTime(clockRate) - time;
}

void VClock_SetRate(long newRate)
{
	if(newRate)
	clockRate = newRate;
	else
	clockRate = kTimeUnit;
}

long VClock_GetRate()
{
	return clockRate;
}
