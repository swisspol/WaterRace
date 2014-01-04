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


#ifndef __INFINITY_PLAYER__
#define __INFINITY_PLAYER__

#ifdef __cplusplus
extern "C" {
#endif

//CONSTANTES:

#define					kPlayer_DefaultAnimationID			'None'
#define					kPlayer_DefaultTextureSet			'None'

#define					kPlayer_UnlimitedFPS				(-1)
#define					kPlayer_DefaultMaxFPS				50

#define					kPlayerFlag_StopOnMouse				(1 << 0)
#define					kPlayerFlag_StopOnKeyBoard			(1 << 1)
#define					kPlayerFlag_DisplayLastFrameOnStop	(1 << 16)

//ROUTINES:

Boolean InfinityPlayer_RaveAvailable();
OSErr InfinityPlayer_Init(Boolean enableAudio);
void InfinityPlayer_Quit();
OSErr InfinityPlayer_GetBestRaveEngine(GDHandle screenDevice, TQAEngine** theEngine);

OSErr InfinityPlayer_LoadAnimation(GDHandle screenDevice, TQAEngine* theEngine, FSSpec* animationFile, OSType textureSetID,
	short positionH, short positionV, short renderWidth, short renderHeight, unsigned long contextFlags, 
	unsigned long textureFlags, PixMapHandle backgroundPixMap, short bkgLeft, short bkgTop, Boolean useBitMap);
	
void InfinityPlayer_SetClearColor(float r, float g, float b);
void InfinityPlayer_SetModelMatrix(float x, float y, float z, float rx, float ry, float rz, float scale);
void InfinityPlayer_SetCamera(float pos_X, float pos_Y, float pos_Z, float roll, float pitch, float yaw, float zoom);
void InfinityPlayer_SetLight(float ambient, float sun, float sun_Rx, float sun_Rz);
void InfinityPlayer_SetClipping(float hither, float yon);
OSErr InfinityPlayer_UnloadAnimation();

void InfinityPlayer_EraseBackground();
OSErr InfinityPlayer_PlayAnimation(OSType animationID);
void InfinityPlayer_StopAnimation();
void InfinityPlayer_RenderFrame();
Boolean InfinityPlayer_RunAsync(float maxFPS);
Boolean InfinityPlayer_RunSync(long flags, float maxFPS);
Boolean InfinityPlayer_ScriptPlaying();
unsigned long InfinityPlayer_GetCurrentAnimationLength();
void InfinityPlayer_SetCurrentAnimationTime(unsigned long localTime);
void InfinityPlayer_MaxFPS(float maxFPS);
MegaObjectPtr InfinityPlayer_ReturnMegaObject();

#ifdef __cplusplus
}
#endif

#endif
