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


#ifndef __NAVIGATION_SERVICES__
#define __NAVIGATION_SERVICES__

//PROTOTYPES:

typedef void (*HandleUpdateEvent_Proc)();

//PROTOTYPES:

void NavigationPutFile(Str255 prompt, Str255 defaultName, StandardFileReply* reply, Str63 appName, OSType fileType, OSType fileCreator, HandleUpdateEvent_Proc updateProc);
void NavigationGetFile(short numTypes, ConstSFTypeListPtr typeList, StandardFileReply* reply, Str63 appName, OSType appCreator, Boolean displayPreview, HandleUpdateEvent_Proc updateProc);

PicHandle Object_ExtractPreview(FSSpec* theFile);
void NavigationGetMetaFile(short numTypes, ConstSFTypeListPtr typeList, StandardFileReply* reply, Str63 appName, OSType appCreator, HandleUpdateEvent_Proc updateProc);

#endif
