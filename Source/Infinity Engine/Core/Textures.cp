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


#include			<Rave.h>

#include			"Infinity Structures.h"
#include			"Infinity Error Codes.h"
#include			"Textures.h"

//CONSTANTES PREPROCESSEURS:

#define	__USE_BUFFERS__	1

//CONSTANTES:

#define				kTextureFile_Type		'PICT'
#define				kTextureFile_Creator	'ttxt'

#define				kPICTFileOffset			512
#define				kMaxTQAImages			16
#define				kMaskLimit				127

//LOCAL VARIABLES:

#if __USE_BUFFERS__
static GWorldPtr			_buffer32 = nil,
							_buffer16 = nil;
static Boolean				_bufferInited = false;
#endif

//ROUTINES:

//We assume we won't use textures bigger than 1024x1024 and won't mipmap textures bigger than 512x512
OSErr Texture_InitBuffers()
{
#if __USE_BUFFERS__
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	OSErr					theError;
	Rect					worldRect = {0,0,kMaxTextureSizeV,kMaxTextureSizeH};
	
	GetGWorld(&savePort, &saveDevice);
	
	//Create 32 Bits GWorld
	theError = NewGWorld(&_buffer32, 32, &worldRect, NULL, NULL, NULL);
	if(theError)
	return theError;
	LockPixels(GetGWorldPixMap(_buffer32));
	SetGWorld(_buffer32, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	SetGWorld(savePort, saveDevice);
	
	//Create 16 Bits GWorld
	theError = NewGWorld(&_buffer16, 16, &worldRect, NULL, NULL, NULL);
	if(theError) {
		Texture_DisposeBuffers();
		return theError;
	}
	LockPixels(GetGWorldPixMap(_buffer16));
	SetGWorld(_buffer16, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	SetGWorld(savePort, saveDevice);
	
	_bufferInited = true;
#endif
	
	return noErr;
}

void Texture_DisposeBuffers()
{
#if __USE_BUFFERS__
	if(_buffer32 != nil) {
		UnlockPixels(GetGWorldPixMap(_buffer32));
		DisposeGWorld(_buffer32);
		_buffer32 = nil;
	}
	if(_buffer16 != nil) {
		UnlockPixels(GetGWorldPixMap(_buffer16));
		DisposeGWorld(_buffer16);
		_buffer16 = nil;
	}
	_bufferInited = false;
#endif
}

OSErr Texture_Decompress(CompressedTexturePtr texturePtr, PixMapHandle destPixMap, Rect* destRect)
{
	Ptr						destBaseAddress;
	long					destRowBytes,
							hStep,
							vStep,
							i,
							j,
							width,
							height;
	unsigned char			*pixel8;
	unsigned short			*pixel16;
	
	if((texturePtr == nil) || (texturePtr->image == nil))
	return kError_FatalError;
	if(((**destPixMap).pixelSize != 16) && ((**destPixMap).pixelSize != 32))
	return kError_FatalError;
	
	//Draw texture image
	if(((**destPixMap).pixelSize == 16) && (texturePtr->depth > 16)) {
		if(texturePtr->flags & kFlag_Dithering)
		DrawPicture(texturePtr->image, destRect);
		else {
#if __USE_BUFFERS__
			CGrafPtr				savePort = NULL;
			GDHandle				saveDevice = NULL;
			
			GetGWorld(&savePort, &saveDevice);
			SetGWorld(_buffer32, NULL);
			DrawPicture(texturePtr->image, destRect);
			CopyBits(GWBitMapPtr(_buffer32), (BitMap*) *destPixMap, destRect, destRect, srcCopy, nil);
			SetGWorld(savePort, saveDevice);
#else
			GWorldPtr				pictWorld = NULL;
			PixMapHandle			pictPix = NULL;
			CGrafPtr				savePort = NULL;
			GDHandle				saveDevice = NULL;
			OSErr					theError;
			
			GetGWorld(&savePort, &saveDevice);
			theError = NewGWorld(&pictWorld, 32, destRect, NULL, NULL, NULL);
			if(theError)
			return theError;
			pictPix = GetGWorldPixMap(pictWorld);
			LockPixels(pictPix);
			SetGWorld(pictWorld, NULL);
			BackColor(whiteColor);
			ForeColor(blackColor);
			DrawPicture(texturePtr->image, destRect);
			CopyBits(GWBitMapPtr(pictWorld), (BitMap*) *destPixMap, destRect, destRect, srcCopy, nil);
			SetGWorld(savePort, saveDevice);
			DisposeGWorld(pictWorld);
#endif
		}
	}
	else
	DrawPicture(texturePtr->image, destRect);
	
	//Decompress alpha if necessary
	if(texturePtr->alphaDepth != kAlpha_None) {
		width = destRect->right - destRect->left;
		height = destRect->bottom - destRect->top;
		hStep = texturePtr->width / width;
		vStep = texturePtr->height / height;
		
		destRowBytes = (**destPixMap).rowBytes & 0x3FFF;
		
		if(texturePtr->alphaDepth == kAlpha_1Bit) {
			destBaseAddress = (**destPixMap).baseAddr + destRect->top * destRowBytes + destRect->left * 2;
		
			for(i = 0; i < height; ++i) {
				pixel16 = (unsigned short*) destBaseAddress;
				for(j = 0; j < width; ++j) {
					if(texturePtr->alphaLayerData[i * vStep * texturePtr->width + j * hStep] > kMaskLimit)
					*pixel16 |= 32768;
					++pixel16;
				}
				destBaseAddress += destRowBytes;
			}
		}
		else if(texturePtr->alphaDepth == kAlpha_8Bits) {
			destBaseAddress = (**destPixMap).baseAddr + destRect->top * destRowBytes + destRect->left * 4;
		
			for(i = 0; i < height; ++i) {
				pixel8 = (unsigned char*) destBaseAddress;
				for(j = 0; j < width; ++j) {
					*pixel8 = texturePtr->alphaLayerData[i * vStep * texturePtr->width + j * hStep];
					pixel8 += 4;
				}
				destBaseAddress += destRowBytes;
			}
		}
	}

	return QDError();
}

OSErr Texture_NewFromCompressedTexture(TQAEngine* engine, TQATexture** RAVETexture, CompressedTexturePtr texturePtr, long flags)
{
	Rect					pictRect;
	GWorldPtr				pictWorld = NULL;
	PixMapHandle			pictPix = NULL;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
#if !__USE_BUFFERS__
	OSErr					theError;
	Rect					worldRect;
#endif
	TQAImage				image[kMaxTQAImages];
	unsigned long			raveFlags;
	TQAError				theQAErr = kQANoErr;
	Boolean					mipmap = false;
	
	if((texturePtr == nil) || (texturePtr->image == nil))
	return kError_FatalError;
	if((texturePtr->width > kMaxTextureSizeH) || (texturePtr->height > kMaxTextureSizeV))
	return kError_FatalError;
	
	//Set up worldRect
	pictRect.left = 0;
	pictRect.top = 0;
	pictRect.right = texturePtr->width;
	pictRect.bottom = texturePtr->height;
		
	//Reduce picture if required
	if((flags & kTextureFlag_Reduce16X) && (texturePtr->width > kMinTextureReduce) && (texturePtr->height > kMinTextureReduce)) {
		pictRect.right /= 4;
		pictRect.bottom /= 4;
	}
	else if((flags & kTextureFlag_Reduce4X) && (texturePtr->width > kMinTextureReduce) && (texturePtr->height > kMinTextureReduce)) {
		pictRect.right /= 2;
		pictRect.bottom /= 2;
	}
	
	//3Dfx can't load textures bigger than 256x256
	if((flags & kTextureFlag_3DfxMode) && ((pictRect.right > kMax3DfxTextureSizeH) || (pictRect.bottom > kMax3DfxTextureSizeV))) {
		long		hRatio = pictRect.right / kMax3DfxTextureSizeH,
					vRatio = pictRect.bottom / kMax3DfxTextureSizeV;
		
		pictRect.left = 0;
		pictRect.top = 0;
		if(hRatio > vRatio) {
			pictRect.right = kMax3DfxTextureSizeH;
			pictRect.bottom = pictRect.bottom / hRatio;
		}
		else {
			pictRect.right = pictRect.right / vRatio;
			pictRect.bottom = kMax3DfxTextureSizeV;
		}
	}
	
	//Do we have to mipmap the texture?
	if(((flags & kTextureFlag_MipMap) && (texturePtr->flags & kFlag_MipMap)) || (flags & kTextureFlag_ForceMipMap))
	mipmap = true;
	if((pictRect.right > kMaxMipmapSizeH) || (pictRect.bottom > kMaxMipmapSizeV))
	mipmap = false;
	
#if __USE_BUFFERS__
	GetGWorld(&savePort, &saveDevice);
	if(texturePtr->alphaDepth == kAlpha_8Bits)
	pictWorld = _buffer32;
	else
	pictWorld = _buffer16;
	
	SetGWorld(pictWorld, NULL);
	pictPix = GetGWorldPixMap(pictWorld);
#else
	//Create GWorld and decompress texture into it
	GetGWorld(&savePort, &saveDevice);
	worldRect = pictRect;
	if(mipmap)
	worldRect.right *= 2;
	if(texturePtr->alphaDepth == kAlpha_8Bits)
	theError = NewGWorld(&pictWorld, 32, &worldRect, NULL, NULL, NULL);
	else
	theError = NewGWorld(&pictWorld, 16, &worldRect, NULL, NULL, NULL); //Textures are always loaded in VRAM in 16Bits!
	if(theError)
	return theError;
	pictPix = GetGWorldPixMap(pictWorld);
	
	LockPixels(pictPix);
	SetGWorld(pictWorld, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
#endif
	
	Texture_Decompress(texturePtr, pictPix, &pictRect);
	
	//Create Rave texture - Setup TQAImage
	image[0].width = pictRect.right;
	image[0].height = pictRect.bottom;
	image[0].rowBytes = (**pictPix).rowBytes & 0x3FFF;
	image[0].pixmap = GetPixBaseAddr(pictPix);
	if(mipmap) {
		long		index = 1;
		short		width = pictRect.right / 2,
					height = pictRect.bottom / 2,
					hPos = pictRect.right;
		Boolean		go = true;
		Rect		newPictRect;
		
		while(go) {
			newPictRect.left = hPos;
			newPictRect.top = 0;
			newPictRect.right = hPos + width;
			newPictRect.bottom = height;
			if(texturePtr->alphaDepth == kAlpha_None)
			CopyBits(GWBitMapPtr(pictWorld), GWBitMapPtr(pictWorld), &pictRect, &newPictRect, srcCopy, nil);
			else
			Texture_Decompress(texturePtr, pictPix, &newPictRect);
			
			image[index].width = width;
			image[index].height = height;
			image[index].rowBytes = (**pictPix).rowBytes & 0x3FFF;
			if(texturePtr->alphaDepth == kAlpha_8Bits)
			image[index].pixmap = GetPixBaseAddr(pictPix) + newPictRect.left * 4;
			else
			image[index].pixmap = GetPixBaseAddr(pictPix) + newPictRect.left * 2;
			
			hPos += width;
			width /= 2;
			height /= 2;
			
			if((width == 0) && (height == 0))
			go = false;
			else {
				if(height == 0)
				height = 1;
				if(width == 0)
				width = 1;
			}
			++index;
		}
	}
	SetGWorld(savePort, saveDevice);
	
	//Setup Rave flags
	raveFlags = kQATexture_None;
	if(flags & kTextureFlag_Compress)
	raveFlags |= kQATexture_HighCompression;
	else if(!(flags & kTextureFlag_LowQuality))
	raveFlags |= kQATexture_NoCompression;
	if(mipmap)
	raveFlags |= kQATexture_Mipmap;
	if(flags & kTextureFlag_ForceAGP) //ATI only
	raveFlags |= 3 << 28;
	
	//Create Rave texture
	switch(texturePtr->alphaDepth) {
		
		case kAlpha_None: theQAErr = QATextureNew(engine, raveFlags, kQAPixel_RGB16, image, RAVETexture); break;
		case kAlpha_1Bit: theQAErr = QATextureNew(engine, raveFlags, kQAPixel_ARGB16, image, RAVETexture); break;
		case kAlpha_8Bits: theQAErr = QATextureNew(engine, raveFlags, kQAPixel_ARGB32, image, RAVETexture); break;
		
	}
	
	//Force load into VRAM
	if(theQAErr == kQANoErr) {
		theQAErr = QATextureDetach(engine, *RAVETexture); 
		if(theQAErr != kQANoErr) {		
			if(*RAVETexture != nil)
			QATextureDelete(engine, *RAVETexture);
			*RAVETexture = nil;
		}
	}
	
	//Clean up
#if !__USE_BUFFERS__
	DisposeGWorld(pictWorld);
#endif
	
	return theQAErr;
}

OSErr Texture_NewARGB32BitmapFromPictResource(TQAEngine* engine, TQABitmap** RAVEBitmap, short pictID, long flags)
{
	TQAError		theQAErr = kQANoErr;
	TQAImage		image;
	PicHandle		picture = NULL;
	Rect			pictRect;
#if !__USE_BUFFERS__
	OSErr			theError;
	GWorldPtr		pictWorld = NULL;
#endif
	PixMapHandle	pictPix = NULL;
	CGrafPtr		savePort = NULL;
	GDHandle		saveDevice = NULL;
	
	//Get picture
	picture = (PicHandle) Get1Resource('PICT', pictID);
	if(picture == nil)
	return ResError();
	
#if __USE_BUFFERS__
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(_buffer32, NULL);
	pictPix = GetGWorldPixMap(_buffer32);
	DrawPicture(picture, &pictRect);
	SetGWorld(savePort, saveDevice);
#else
	//Create GWorld
	pictRect = (**picture).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);
	theError = NewGWorld(&pictWorld, 32, &pictRect, NULL, NULL, NULL);
	if(theError)
	return theError;
	pictPix = GetGWorldPixMap(pictWorld);
	LockPixels(pictPix);
	
	//Draw picture
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(pictWorld, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	DrawPicture(picture, &pictRect);
	SetGWorld(savePort, saveDevice);
#endif
	
	//Setup TQAImage
	image.width = pictRect.right;
	image.height = pictRect.bottom;
	image.rowBytes = (**pictPix).rowBytes & 0x3FFF;
	image.pixmap = GetPixBaseAddr(pictPix);
	
	//Create Rave texture
	if(flags & kTextureFlag_Compress)
	theQAErr = QABitmapNew(engine, kQABitmap_HighCompression, kQAPixel_ARGB32, &image, RAVEBitmap);
	else if(!(flags & kTextureFlag_LowQuality))
	theQAErr = QABitmapNew(engine, kQABitmap_NoCompression, kQAPixel_ARGB32, &image, RAVEBitmap);
	else
	theQAErr = QABitmapNew(engine, kQABitmap_None, kQAPixel_ARGB32, &image, RAVEBitmap);
	if(theQAErr == kQANoErr) {
		theQAErr = QABitmapDetach(engine, *RAVEBitmap);
		if(theQAErr != kQANoErr) {	
			if(*RAVEBitmap != nil)
			QABitmapDelete(engine, *RAVEBitmap);
			*RAVEBitmap = nil;
		}
	}
	
	//Clean up
	ReleaseResource((Handle) picture);
#if !__USE_BUFFERS__
	DisposeGWorld(pictWorld);
#endif
	
	return theQAErr;
}

OSErr Texture_NewARGB32TextureFromPictResource(TQAEngine* engine, TQATexture** RAVETexture, short pictID, long flags)
{
	TQAError		theQAErr = kQANoErr;
	TQAImage		image;
	PicHandle		picture = NULL;
	Rect			pictRect;
#if !__USE_BUFFERS__
	OSErr			theError;
	GWorldPtr		pictWorld = NULL;
#endif
	PixMapHandle	pictPix = NULL;
	CGrafPtr		savePort = NULL;
	GDHandle		saveDevice = NULL;
	unsigned long	raveFlags;
	
	//Get picture
	picture = (PicHandle) Get1Resource('PICT', pictID);
	if(picture == nil)
	return ResError();
	
	//Extract picture frame rect
	pictRect = (**picture).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);
	
	//Reduce picture if required
	if((flags & kTextureFlag_Reduce16X) && (pictRect.right > kMinTextureReduce) && (pictRect.bottom > kMinTextureReduce)) {
		pictRect.right /= 4;
		pictRect.bottom /= 4;
	}
	else if((flags & kTextureFlag_Reduce4X) && (pictRect.right > kMinTextureReduce) && (pictRect.bottom > kMinTextureReduce)) {
		pictRect.right /= 2;
		pictRect.bottom /= 2;
	}
	
	//3Dfx can't load textures bigger than 256x256
	if((flags & kTextureFlag_3DfxMode) && ((pictRect.right > kMax3DfxTextureSizeH) || (pictRect.bottom > kMax3DfxTextureSizeV))) {
		long		hRatio = pictRect.right / kMax3DfxTextureSizeH,
					vRatio = pictRect.bottom / kMax3DfxTextureSizeV;
		
		pictRect.left = 0;
		pictRect.top = 0;
		if(hRatio > vRatio) {
			pictRect.right = kMax3DfxTextureSizeH;
			pictRect.bottom = pictRect.bottom / hRatio;
		}
		else {
			pictRect.right = pictRect.right / vRatio;
			pictRect.bottom = kMax3DfxTextureSizeV;
		}
	}
	
#if __USE_BUFFERS__
	//Draw picture
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(_buffer32, NULL);
	pictPix = GetGWorldPixMap(_buffer32);
	DrawPicture(picture, &pictRect);
	SetGWorld(savePort, saveDevice);
#else
	//Create GWorld
	theError = NewGWorld(&pictWorld, 32, &pictRect, NULL, NULL, NULL);
	if(theError)
	return theError;
	pictPix = GetGWorldPixMap(pictWorld);
	LockPixels(pictPix);
	
	//Draw picture
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(pictWorld, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	DrawPicture(picture, &pictRect);
	SetGWorld(savePort, saveDevice);
#endif

	//Setup TQAImage
	image.width = pictRect.right;
	image.height = pictRect.bottom;
	image.rowBytes = (**pictPix).rowBytes & 0x3FFF;
	image.pixmap = GetPixBaseAddr(pictPix);
	
	//Setup Rave flags
	raveFlags = kQATexture_None;
	if(flags & kTextureFlag_Compress)
	raveFlags |= kQATexture_HighCompression;
	else if(!(flags & kTextureFlag_LowQuality))
	raveFlags |= kQATexture_NoCompression;
	if(flags & kTextureFlag_ForceAGP) //ATI only
	raveFlags |= 3 << 28;
	
	//Create Rave texture
	theQAErr = QATextureNew(engine, raveFlags, kQAPixel_ARGB32, &image, RAVETexture);
	if(theQAErr == kQANoErr) {
		theQAErr = QATextureDetach(engine, *RAVETexture); 
		if(theQAErr != kQANoErr) {	
			if(*RAVETexture != nil)
			QATextureDelete(engine, *RAVETexture);
			*RAVETexture = nil;
		}
	}
	
	//Clean up
	ReleaseResource((Handle) picture);
#if !__USE_BUFFERS__
	DisposeGWorld(pictWorld);
#endif
	
	return theQAErr;
}
