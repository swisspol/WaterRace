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


#ifndef __AUDIO_CD__
#define __AUDIO_CD__

//ROUTINES:

OSErr AudioCD_Init_UseAppleDriver();
OSErr AudioCD_Init_UseCDName(Str63 CDName);
OSErr AudioCD_Init_Manually(short driverRefNum);

OSErr AudioCD_SetVolume(UInt8 left, UInt8 right);
OSErr AudioCD_GetVolume(UInt8* left, UInt8* right);
OSErr AudioCD_Eject();
short AudioCD_GetNumTracks();
short AudioCD_GetCurrentTrack();
OSErr AudioCD_Stop();
OSErr AudioCD_Pause();
OSErr AudioCD_Resume();
Boolean AudioCD_IsPlaying();
Boolean AudioCD_IsCDInserted();
OSErr AudioCD_PlayTrack(UInt8 trackNumber);

#endif
