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


#ifndef __INFINITY_CINEMATIC_UTILS__
#define __INFINITY_CINEMATIC_UTILS__

//CONSTANTES:

#define			kCinematic_AudioClient			'Cmtc'

//INLINE ROUTINES:

inline ScriptPtr Script_GetPtrFromID(MegaObjectPtr mega, OSType ID)
{
	long				i;
	
	for(i = 0; i < mega->scriptCount; ++i)
	if(mega->scriptList[i]->id == ID)
	return mega->scriptList[i];
	
	return nil;
}

//ROUTINES:

void Script_Start(ScriptPtr script, unsigned long absoluteTime);
void Script_Stop(ScriptPtr script);
void Script_Run(ScriptPtr script, unsigned long absoluteTime);
void Script_DisplayTime(ScriptPtr script, unsigned long localTime);

void Script_AudioEnable(long soundCount, ExtSoundHeaderPtr* soundList, OSType* soundIDList);
void Script_AudioDisable();

#endif
