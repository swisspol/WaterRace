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


#ifndef __INFINITY_POST_FX__
#define __INFINITY_POST_FX__

//CONSTANTES:

enum {
	kTextMode_Left = 0,
	kTextMode_Centered,
	kTextMode_Right,
	kTextMode_Fake_NewLine = -1,
	kTextMode_Space = -2
};

enum {
	kTextSize_Small = 1,
	kTextSize_Big
};

//PROTOTYPES:

//File: Infinity Post FX Lens flare.cp
OSErr PostFX_LensFlare_Init(StatePtr state, long textureFlags);
void PostFX_LensFlare_Quit(StatePtr state);
void PostFX_LensFlare_Display(StatePtr state, float skyRadius, MatrixPtr negatedCamera, VectorPtr cameraPos);

//File: Infinity Post FX Moon.cp
OSErr PostFX_Moon_Init(StatePtr state, long textureFlags);
void PostFX_Moon_Quit(StatePtr state);
void PostFX_Moon_Display(StatePtr state, float skyRadius, MatrixPtr negatedCamera, VectorPtr cameraPos);

//File: Infinity Post FX Snow.cp
OSErr PostFX_Snow_Init(StatePtr state, long textureFlags);
void PostFX_Snow_Quit(StatePtr state);
void PostFX_Snow_Display(StatePtr state, MatrixPtr camera, MatrixPtr negatedCamera);

//File: Infinity Post FX Rain.cp
OSErr PostFX_Rain_Init(StatePtr state, long textureFlags);
void PostFX_Rain_Quit(StatePtr state);
void PostFX_Rain_Display(StatePtr state, MatrixPtr camera, MatrixPtr negatedCamera);

//File: Infinity Post FX Text.cp
OSErr PostFX_Text_Init(StatePtr state, long textureFlags, long textSize);
void PostFX_Text_Quit(StatePtr state);
void PostFX_Text_Display(StatePtr state, long length, unsigned char* text, VectorPtr position, long mode, float r, float g, float b);

#endif
