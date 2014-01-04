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


#ifndef __INFINITY_TEXTURES__
#define __INFINITY_TEXTURES__

//Texture creation flags
#define				kTextureFlag_3DfxMode		(1 << 0)
#define				kTextureFlag_Compress		(1 << 1)
#define				kTextureFlag_Reduce4X		(1 << 2)
#define				kTextureFlag_Reduce16X		(1 << 3)
#define				kTextureFlag_MipMap			(1 << 4)
#define				kTextureFlag_ForceMipMap	(1 << 5)
#define				kTextureFlag_ForceAGP		(1 << 6) //ATI only
#define				kTextureFlag_LowQuality		(1 << 7) //ATI only

//MACROS:

#define	GWBitMapPtr(w) &(((GrafPtr)(w))->portBits)

//ROUTINES:

//File: Textures.cp
OSErr Texture_InitBuffers();
void Texture_DisposeBuffers();

OSErr Texture_Decompress(CompressedTexturePtr texturePtr, PixMapHandle destPixMap, Rect* destRect);
OSErr Texture_NewFromCompressedTexture(TQAEngine* engine, TQATexture** RAVETexture, CompressedTexturePtr texturePtr, long flags);
OSErr Texture_NewARGB32BitmapFromPictResource(TQAEngine* engine, TQABitmap** RAVEBitmap, short pictID, long flags);
OSErr Texture_NewARGB32TextureFromPictResource(TQAEngine* engine, TQATexture** RAVETexture, short pictID, long flags);

#endif
