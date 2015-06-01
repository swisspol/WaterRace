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


#ifndef __INFINITY_TEXTURES_UTILS__
#define __INFINITY_TEXTURES_UTILS__

//CONSTANTES:

#define				kTextureFile_Type			'PICT'
#define				kTextureFile_Creator		'ttxt'

//ROUTINES:

//File: Texture utils.cp
void Texture_SetEventFilterRoutine(UniversalProcPtr newRoutine);
void Texture_BestDisplayRect(short width, short height, Rect* destRect);
OSErr Texture_NewFromFile(FSSpec* sourceFile, CompressedTexturePtr* texturePtr, Boolean QTImport);
OSErr Texture_NewFromClipboard(CompressedTexturePtr* texturePtr);
OSErr Texture_NewFromPicHandle(PicHandle picture, OSType name, CompressedTexturePtr* texturePtr, short alphaMode);
OSErr Texture_Draw(CompressedTexturePtr texturePtr, Rect* destRect);
OSErr Texture_ExportToClipboard(CompressedTexturePtr texturePtr);
OSErr Texture_ExportToPICTFile(FSSpec* destFile, CompressedTexturePtr texturePtr);
OSErr Texture_Copy(CompressedTexturePtr sourceTexture, CompressedTexturePtr* newTexture);

#endif
