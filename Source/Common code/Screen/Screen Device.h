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


#ifndef __SCREEN__
#define __SCREEN__

//ROUTINES:

OSErr ScreenDevice_Init();
OSErr ScreenDevice_Quit();
OSErr ScreenDevice_GetResolution(GDHandle device, VDSwitchInfoRec* switchInfo);
OSErr ScreenDevice_SetResolution(GDHandle device, VDSwitchInfoRec* switchInfo);
OSErr ScreenDevice_LookForResolution(GDHandle device, short width, short height, short depth, VDSwitchInfoRec* switchInfo);
OSErr SL_GetMaxResolution(GDHandle device, short* width, short* height, short depth);

#endif
