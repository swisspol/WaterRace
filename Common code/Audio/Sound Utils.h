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


#ifndef __SOUND_UTILS__
#define __SOUND_UTILS__

//CONSTANTES:

#define				kSoundFileType			'sfil'
#define				kSoundResType			'snd '

//ROUTINES:

//File: Sound Utils.cp
OSErr Sound_ResLoad(short resID, Boolean correctLoop, Handle* sound);
OSErr Sound_Load(FSSpec* file, Boolean correctLoop, Handle* sound);
OSErr Sound_Play(Ptr soundHeaderPtr, Boolean loop);

#endif
