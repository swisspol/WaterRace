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


#ifndef __INFINITY_CLOCK__
#define __INFINITY_CLOCK__

//CONSTANTES PREPROCESSEURS:

#define	__USE_UPTIME__	0
#define	__SECURE_UPTIME__	1

//CONSTANTES:

#define					kMilliPerSecond			1000
#define					kMicroPerSecond			1000000
#define					kNanoPerSecond			1000000000

//INLINE ROUTINES:

#if __USE_UPTIME__
#ifdef __DRIVERSERVICES__
inline unsigned long GetCurrentAbsoluteTime(long timeUnit)
{
#if __SECURE_UPTIME__
	if((RoutineDescriptorPtr) UpTime != (RoutineDescriptorPtr) kUnresolvedCFragSymbolAddress) {
#endif
	AbsoluteTime at = UpTime();
	UnsignedWide ns = AbsoluteToNanoseconds(at);
	
	return ((unsigned long) (*(unsigned long long *) &ns * timeUnit / kNanoPerSecond));
#if __SECURE_UPTIME__
	}
	else {
		UnsignedWide	currentTime;
		
		Microseconds(&currentTime);
		return ((unsigned long) (*(unsigned long long *) &currentTime * timeUnit / kMicroPerSecond));
	}
#endif
}
#endif
#else
#ifdef __OPENTRANSPORT__
inline unsigned long GetCurrentAbsoluteTime(long timeUnit)
{
	OTTimeStamp		currentTime;
	
	OTGetTimeStamp(&currentTime);
	
	return OTTimeStampInMilliseconds(&currentTime) * timeUnit / kMilliPerSecond;
}
#endif
#endif

//ROUTINES:

//File: Relative Clock.cp
unsigned long RClock_GetTime();
unsigned long RClock_GetAbsoluteTime();
void RClock_SetTime(unsigned long time);
void RClock_Delay(unsigned long delay);

//File: Variable Clock.cp
unsigned long VClock_GetTime();
void VClock_SetTime(unsigned long time);
void VClock_SetRate(long newRate);
long VClock_GetRate();

//File: Step Clock.cp
unsigned long StepClock_GetTime();
unsigned long StepClock_UpdateTime();
void StepClock_SetTime(unsigned long time);
void StepClock_SetStep(unsigned long step);

#endif
