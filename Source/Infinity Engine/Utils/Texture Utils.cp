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
#include			<PictUtils.h>
#include			<QuickTimeComponents.h>

#include			"Infinity Structures.h"
#include			"Infinity Error Codes.h"

#include			"Textures.h"
#include			"Texture Utils.h"

//CONSTANTES:

#define				kPICTFileOffset			512

#define				kTextureNewDialogID		9990

//PROTOTYPES:

void PictBox_Interactive_StartUp(DialogPtr dialog, short num, PicHandle picture);
void PictBox_Interactive_CleanUp();

//VARIABLES LOCALES:

static UniversalProcPtr	EventFilterRoutine = nil;
static short			lastCompressionSelected = 4,
						lastAlphaSelected = 0;

//ROUTINES:

void Texture_SetEventFilterRoutine(UniversalProcPtr newRoutine)
{
	EventFilterRoutine = newRoutine;
}

void Texture_BestDisplayRect(short width, short height, Rect* destRect)
{
	float				xRatio,
						yRatio;
						
	if((width <= destRect->right - destRect->left) && (height <= destRect->bottom - destRect->top)) {
		destRect->right = destRect->left + width;
		destRect->bottom = destRect->top + height;
	}
	else {
		xRatio = (float) width / (float) (destRect->right - destRect->left);
		yRatio = (float) height / (float) (destRect->bottom - destRect->top);
		
		if(xRatio > yRatio) {
			destRect->bottom = destRect->top + height / xRatio;
			destRect->right = destRect->left + width / xRatio;
		}
		else {
			destRect->bottom = destRect->top + height / yRatio;
			destRect->right = destRect->left + width / yRatio;
		}
	}
}

static OSErr ExtractAlphaLayer(PicHandle image, Handle* alphaLayer)
{
	Rect					pictRect;
	GWorldPtr				pictWorld = NULL;
	PixMapHandle			pictPix = NULL;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	OSErr					theError;
	unsigned char			*sourcePixel,
							*destPixel;
	Ptr						sourceBaseAddress;
	long					sourceRowBytes,
							i,
							j;
	
	//Gather PICT info
	pictRect = (**image).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);
	
	//Draw picture into 32Bits GWorld
	GetGWorld(&savePort, &saveDevice);
	theError = NewGWorld(&pictWorld, 32, &pictRect, NULL, NULL, NULL);
	if(theError)
	return theError;
	pictPix = GetGWorldPixMap(pictWorld);
	LockPixels(pictPix);
	SetGWorld(pictWorld, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	DrawPicture(image, &pictRect);
	SetGWorld(savePort, saveDevice);
	
	//Allocate memory
	*alphaLayer = NewHandle(pictRect.right * pictRect.bottom);
	if(*alphaLayer == nil) {
		DisposeGWorld(pictWorld);
		return MemError();
	}
	HLock(*alphaLayer);
	
	//Extract alpha layer
	destPixel = (unsigned char*) **alphaLayer;
	sourceRowBytes = (**pictPix).rowBytes & 0x3FFF;
	sourceBaseAddress = (**pictPix).baseAddr;
	for(i = 0; i < pictRect.bottom; ++i) {
		sourcePixel = (unsigned char*) sourceBaseAddress;
		for(j = 0; j < pictRect.right; ++j) {
			*destPixel = *sourcePixel;
			sourcePixel += 4;
			++destPixel;
		}
		sourceBaseAddress += sourceRowBytes;
	}
	
	//Clean up
	DisposeGWorld(pictWorld);
	
	return noErr;
}

static Boolean PictureIsPowerOfTwo(PicHandle image)
{
	short					width,
							height;
							
	width = (**image).picFrame.right - (**image).picFrame.left;
	height = (**image).picFrame.bottom - (**image).picFrame.top;
	
	while(width != 1) {
		if(width % 2)
		return false;
		width /= 2;
	}
	
	while(height != 1) {
		if(height % 2)
		return false;
		height /= 2;
	}
	
	return true;
}

static OSErr Texture_New(PicHandle image, OSType name, CompressedTexturePtr* texturePtr, Boolean conversion, short defaultAlphaMode)
{
	OSErr					theError;
	PictInfo				info;
	Handle					alphaHandle = nil;
	CompressedTexturePtr	texture;
	PicHandle				newImage;
	ComponentInstance		ci;
	SCSpatialSettings		compressionSettings;
	ControlHandle			items[5];
	long					i;
	GrafPtr					savePort;
	short					itemType;
	DialogPtr				theDialog;
	Str31					width,
							height,
							sourceSize,
							destSize;
	short					alphaMode;
	Boolean					dontRecompress = false;
					
	if(image == nil)
	return kError_FatalError;
	
	if(!PictureIsPowerOfTwo(image))
	return kError_ImageIsNotPowerOfTwo;
	
	//Gather some info
	theError = GetPictInfo(image, &info, 0, 0, 0, 0);
	if(theError)
	return theError;
	
	//Connect to image compressor component
	ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
	if(ci == nil)
	return paramErr;
	theError = SCDefaultPictHandleSettings(ci, image, false);
	if(theError)
	return kError_QTImageCompressorError;
	theError = SCSetTestImagePictHandle(ci, image, &((**image).picFrame), scPreferScalingAndCropping);
	if(theError)
	return kError_QTImageCompressorError;
	theError = SCGetInfo(ci, scSpatialSettingsType, &compressionSettings);
	if(theError)
	return kError_QTImageCompressorError;
	switch(lastCompressionSelected) {
	
		case 1:
		dontRecompress = true;
		break;
		
		case 3:
		compressionSettings.codecType = kJPEGCodecType;
		compressionSettings.depth = 24;
		compressionSettings.spatialQuality = codecNormalQuality;
		break;
		
		case 4:
		compressionSettings.codecType = kJPEGCodecType;
		compressionSettings.depth = 24;
		compressionSettings.spatialQuality = codecHighQuality;
		break;
		
		case 5:
		compressionSettings.codecType = kJPEGCodecType;
		compressionSettings.depth = 24;
		compressionSettings.spatialQuality = codecLosslessQuality;
		break;
		
		case 6:
		compressionSettings.codecType = kRawCodecType;
		compressionSettings.depth = 16;
		compressionSettings.spatialQuality = codecLosslessQuality;
		break;
		
		case 8:
		compressionSettings.codecType = kJPEGCodecType;
		compressionSettings.depth = 24;
		compressionSettings.spatialQuality = codecHighQuality;
		//theError = SCRequestImageSettings(ci);
		break;
		
	}
	theError = SCSetInfo(ci, scSpatialSettingsType, &compressionSettings);
	if(theError)
	return kError_QTImageCompressorError;
	
	//Compress image
	if(dontRecompress) {
		newImage = image;
		theError = HandToHand((Handle*) &newImage);
	}
	else {
		newImage = (PicHandle) NewHandle(0);
		theError = SCCompressPicture(ci, image, newImage);
	}
	if(theError) {
		CloseComponent(ci);
		if(theError == scUserCancelled)
		return kError_UserCancel;
		else
		return kError_QTImageCompressorError;
	}
	
	//Display dialog
	GetPort(&savePort);
	theDialog = GetNewDialog(kTextureNewDialogID, nil, (WindowPtr) -1);
	SetPort(theDialog);
	for(i = 0; i < 5; ++i)
	GetDialogItemAsControl(theDialog, 3 + i, &items[i]);
	
	PictBox_Interactive_StartUp(theDialog, 8, image);
	PictBox_Interactive_StartUp(theDialog, 9, newImage);
	
	//Setup items
	SetControlValue(items[0], lastCompressionSelected);
	
	width[0] = 4;
	BlockMove(&name, &width[1], 4);
	SetDialogItemText((Handle) items[1], width);
	
	SetControlValue(items[2 + lastAlphaSelected], true);
	if(info.depth != 32) {
		HiliteControl(items[3], 255);
		HiliteControl(items[4], 255);
		SetControlValue(items[2], true);
	}
	else
	SetControlValue(items[2 + lastAlphaSelected], true);
	
	if(GetHandleSize((Handle) image) < 1024)
	BlockMove("\p< 1", sourceSize, sizeof(Str31));
	else
	NumToString(GetHandleSize((Handle) image) / 1024, sourceSize);
	if(GetHandleSize((Handle) newImage) < 1024)
	BlockMove("\p< 1", destSize, sizeof(Str31));
	else
	NumToString(GetHandleSize((Handle) newImage) / 1024, destSize);
	NumToString((**image).picFrame.right - (**image).picFrame.left, width);
	NumToString((**image).picFrame.bottom - (**image).picFrame.top, height);
	ParamText(sourceSize, destSize, width, height);
	
	if(conversion) {
		ControlHandle			item;
		
		GetDialogItemAsControl(theDialog, 2, &item);
		HiliteControl(item, 255);
		GetDialogItemAsControl(theDialog, 4, &item);
		HiliteControl(item, 255);
		
		SetControlValue(items[2], false);
		SetControlValue(items[2 + defaultAlphaMode], true);
		HiliteControl(items[2], 255);
		HiliteControl(items[3], 255);
		HiliteControl(items[4], 255);
	}
	
	DrawDialog(theDialog);
	SelectDialogItemText(theDialog, 4, 0, 32000);
	SetDialogDefaultItem(theDialog, 1);
	
	//Loop...
	do {
		ModalDialog(EventFilterRoutine, &itemType);
		switch(itemType) {
			
			case 3:
			theError = noErr;
			i = GetControlValue(items[0]);
			dontRecompress = false;
			switch(i) {
			
				case 1:
				dontRecompress = true;
				break;
				
				case 3:
				compressionSettings.codecType = kJPEGCodecType;
				compressionSettings.depth = 24;
				compressionSettings.spatialQuality = codecNormalQuality;
				theError = SCSetInfo(ci, scSpatialSettingsType, &compressionSettings);
				break;
				
				case 4:
				compressionSettings.codecType = kJPEGCodecType;
				compressionSettings.depth = 24;
				compressionSettings.spatialQuality = codecHighQuality;
				theError = SCSetInfo(ci, scSpatialSettingsType, &compressionSettings);
				break;
				
				case 5:
				compressionSettings.codecType = kJPEGCodecType;
				compressionSettings.depth = 24;
				compressionSettings.spatialQuality = codecLosslessQuality;
				theError = SCSetInfo(ci, scSpatialSettingsType, &compressionSettings);
				break;
				
				case 6:
				compressionSettings.codecType = kRawCodecType;
				compressionSettings.depth = 16;
				compressionSettings.spatialQuality = codecLosslessQuality;
				theError = SCSetInfo(ci, scSpatialSettingsType, &compressionSettings);
				break;
				
				case 8:
				theError = SCRequestImageSettings(ci);
				break;
				
			}
			if(dontRecompress) {
				DisposeHandle((Handle) newImage);
				newImage = image;
				theError = HandToHand((Handle*) &newImage);
			}
			else {
				if(theError == noErr)
				theError = SCCompressPicture(ci, image, newImage);
			}
			if(GetHandleSize((Handle) newImage) < 1024)
			BlockMove("\p< 1", destSize, sizeof(Str31));
			else
			NumToString(GetHandleSize((Handle) newImage) / 1024, destSize);
			ParamText(sourceSize, destSize, width, height);
			DrawDialog(theDialog);
			break;
			
			case 5:
			case 6:
			case 7:
			SetControlValue(items[2], false);
			SetControlValue(items[3], false);
			SetControlValue(items[4], false);
			SetControlValue(items[itemType - 3], true);
			break;
			
		}
	} while((itemType != 1) && (itemType != 2));
	theError = SCGetInfo(ci, scSpatialSettingsType, &compressionSettings);
	if(theError)
	return kError_QTImageCompressorError;
	CloseComponent(ci);
	
	if((itemType == 2) && !conversion) {
		DisposeDialog(theDialog);
		PictBox_Interactive_CleanUp();
		SetPort(savePort);
		DisposeHandle((Handle) newImage);
		return kError_UserCancel;
	}
	
	GetDialogItemText((Handle) items[1], width);
	BlockMove(&width[1], &name, 4);
	
	if(GetControlValue(items[3]))
	alphaMode = kAlpha_1Bit;
	else if(GetControlValue(items[4]))
	alphaMode = kAlpha_8Bits;
	else
	alphaMode = kAlpha_None;
	
	//Use the alpha layer
	if(alphaMode != kAlpha_None) {
		//Extract alpha layer
		theError = ExtractAlphaLayer(image, &alphaHandle);
		if(theError)
		return theError;
	}
	
	//Allocate memory
	if(alphaHandle != nil)
	*texturePtr = (CompressedTexturePtr) NewPtr(sizeof(CompressedTexture) + GetHandleSize(alphaHandle));
	else
	*texturePtr = (CompressedTexturePtr) NewPtr(sizeof(CompressedTexture));
	if(*texturePtr == nil) {
		if(alphaHandle != nil)
		DisposeHandle(alphaHandle);
		return MemError();
	}
	texture = *texturePtr;
	
	//Create texture
	texture->name = name;
	texture->width = (**image).picFrame.right - (**image).picFrame.left;
	texture->height = (**image).picFrame.bottom - (**image).picFrame.top;
	if(dontRecompress)
	texture->depth = info.depth;
	else
	texture->depth = compressionSettings.depth; //final depth
	if(((texture->depth == 24) || (texture->depth == 32)) && (alphaMode != kAlpha_8Bits))
	texture->flags = kFlag_Dithering;
	else
	texture->flags = 0;
	texture->image = newImage;
	if(alphaMode == kAlpha_8Bits)
	texture->uncompressedSize = texture->width * texture->height * 4;
	else
	texture->uncompressedSize = texture->width * texture->height * 2;
	texture->alphaDepth = alphaMode;
	if(alphaHandle != nil)
	BlockMove(*alphaHandle, texture->alphaLayerData, GetHandleSize(alphaHandle));
	
	//Save last values
	if(GetControlValue(items[3]))
	lastAlphaSelected = 1;
	else if(GetControlValue(items[4]))
	lastAlphaSelected = 2;
	else
	lastAlphaSelected = 0;
	lastCompressionSelected = GetControlValue(items[0]);
	
	//Clean up
	if(alphaHandle != nil)
	DisposeHandle(alphaHandle);
	DisposeDialog(theDialog);
	PictBox_Interactive_CleanUp();
	SetPort(savePort);
	
	return noErr;
}

OSErr Texture_NewFromFile(FSSpec* sourceFile, CompressedTexturePtr* texturePtr, Boolean QTImport)
{
	short					fileID;
	long					picSize;
	OSErr					theError;
	PicHandle				picture;
	OSType					name;
	GraphicsImportComponent	ci = nil;
	
	if(QTImport) {
		theError = GetGraphicsImporterForFile(sourceFile, &ci);
		if(theError)
		return theError;
		theError = GraphicsImportGetAsPicture(ci, &picture);
		if(theError)
		return theError;
		CloseComponent(ci);
	}
	else {
		theError = FSpOpenDF(sourceFile, fsRdPerm, &fileID);
		if(theError)
		return theError;
		GetEOF(fileID, &picSize);
		picSize -= kPICTFileOffset;
		picture = (PicHandle) NewHandle(picSize);
		if(picture == nil)
		return MemError();
		HLock((Handle) picture);
		SetFPos(fileID, fsFromStart, kPICTFileOffset);
		FSRead(fileID, &picSize, *picture);
		FSClose(fileID);
	}
	
	//Create texture name
	BlockMove(&sourceFile->name[1], &name, 4);
	
	//Create texture
	theError = Texture_New(picture, name, texturePtr, false, 0);
	
	//Clean up
	DisposeHandle((Handle) picture);
	
	return theError;
}

OSErr Texture_NewFromClipboard(CompressedTexturePtr* texturePtr)
{
	long				i;
	OSErr				theError;
	PicHandle			picture;
	OSType				name;
	
	//Get texture image
	picture = (PicHandle) NewHandle(0);
	theError = GetScrap((Handle) picture, kTextureFile_Type, &i);
	if(theError < 0)
	return theError;
	
	//Create texture name
	name = 'Clip';
	
	//Create texture
	theError = Texture_New(picture, name, texturePtr, false, 0);
	
	//Clean up
	DisposeHandle((Handle) picture);
	
	return theError;
}

OSErr Texture_NewFromPicHandle(PicHandle picture, OSType name, CompressedTexturePtr* texturePtr, short alphaMode)
{
	OSErr				theError;
	
	//Create texture
	theError = Texture_New(picture, name, texturePtr, true, alphaMode);
	
	//Clean up
	DisposeHandle((Handle) picture);
	
	return theError;
}

OSErr Texture_Draw(CompressedTexturePtr texturePtr, Rect* destRect)
{
	if((texturePtr == nil) || (texturePtr->image == nil))
	return kError_FatalError;
	
	DrawPicture(texturePtr->image, destRect);

	return QDError();
}

static OSErr Texture_CreateUncompressedPICT(CompressedTexturePtr texturePtr, PicHandle* image)
{
	Rect					pictRect;
	GWorldPtr				pictWorld = NULL;
	PixMapHandle			pictPix = NULL;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	OSErr					theError;
	Ptr						destBaseAddress;
	long					destRowBytes,
							i,
							j,
							width,
							height;
	unsigned char			*pixel8;
	unsigned long			oldSize;
	
	//Create uncompressed PICT
	GetGWorld(&savePort, &saveDevice);
	SetRect(&pictRect, 0, 0, texturePtr->width, texturePtr->height);
	if(texturePtr->alphaDepth == kAlpha_None)
	theError = NewGWorld(&pictWorld, texturePtr->depth, &pictRect, NULL, NULL, NULL);
	else
	theError = NewGWorld(&pictWorld, 32, &pictRect, NULL, NULL, NULL);
	if(theError)
	return theError;
	pictPix = GetGWorldPixMap(pictWorld);
	LockPixels(pictPix);
	SetGWorld(pictWorld, NULL);
	BackColor(whiteColor);
	ForeColor(blackColor);
	DrawPicture(texturePtr->image, &pictRect);
	if(QDError()) {
		SetGWorld(savePort, saveDevice);
		DisposeGWorld(pictWorld);
		return QDError();
	}
	
	if(texturePtr->alphaDepth != kAlpha_None) {
		width = texturePtr->width;
		height = texturePtr->height;
		
		destRowBytes = (**pictPix).rowBytes & 0x3FFF;
		destBaseAddress = (**pictPix).baseAddr;
		for(i = 0; i < height; ++i) {
			pixel8 = (unsigned char*) destBaseAddress;
			for(j = 0; j < width; ++j) {
				*pixel8 = texturePtr->alphaLayerData[i * texturePtr->width + j];
				pixel8 += 4;
			}
			destBaseAddress += destRowBytes;
		}
		
		(**pictPix).cmpCount = 4; //Hack to preserve alpha layer!
	}
	
	*image = OpenPicture(&pictRect);
	if(QDError()) {
		SetGWorld(savePort, saveDevice);
		DisposeGWorld(pictWorld);
		return QDError();
	}
	oldSize = GetHandleSize((Handle) *image);
	CopyBits(GWBitMapPtr(pictWorld), GWBitMapPtr(pictWorld), &pictRect, &pictRect, srcCopy, nil);
	if(QDError()) {
		SetGWorld(savePort, saveDevice);
		DisposeGWorld(pictWorld);
		return QDError();
	}
	ClosePicture();
	if(QDError()) {
		SetGWorld(savePort, saveDevice);
		DisposeGWorld(pictWorld);
		return QDError();
	}
	SetGWorld(savePort, saveDevice);
	DisposeGWorld(pictWorld);
	
	//Check for QD weird bug
	if(GetHandleSize((Handle) *image) <= oldSize) {
		DisposeHandle((Handle) *image);
		return kError_TextureDecompression;
	}
	
	return noErr;
}

OSErr Texture_ExportToClipboard(CompressedTexturePtr texturePtr)
{
	PicHandle				pic;
	OSErr					theError;
	
	if((texturePtr == nil) || (texturePtr->image == nil))
	return kError_FatalError;
	
	//Create PICT
	if(texturePtr->alphaDepth != kAlpha_None) {
		theError = Texture_CreateUncompressedPICT(texturePtr, &pic);
		if(theError)
		return theError;
	}
	else {
		pic = texturePtr->image;
		theError = HandToHand((Handle*) &pic);
		if(theError)
		return theError;
	}
	
	//Copy to clipboard
	HLock((Handle) pic);
	ZeroScrap();
	theError = PutScrap(GetHandleSize((Handle) pic), kTextureFile_Type, *pic);
	
	//Clean up
	DisposeHandle((Handle) pic);
	
	return theError;
}

OSErr Texture_ExportToPICTFile(FSSpec* destFile, CompressedTexturePtr texturePtr)
{
	PicHandle				pic;
	OSErr					theError;
	short					destFileID;
	long					bytes;
	
	if((texturePtr == nil) || (texturePtr->image == nil))
	return kError_FatalError;
	
	//Create PICT
	if(texturePtr->alphaDepth != kAlpha_None) {
		theError = Texture_CreateUncompressedPICT(texturePtr, &pic);
		if(theError)
		return theError;
	}
	else {
		pic = texturePtr->image;
		theError = HandToHand((Handle*) &pic);
		if(theError)
		return theError;
	}
	
	//Create file
	theError = FSpCreate(destFile, kTextureFile_Creator, kTextureFile_Type, smSystemScript);
	if(theError)
	return theError;
	
	//Write PICT
	theError = FSpOpenDF(destFile, fsRdWrPerm, &destFileID);
	if(theError)
	return theError;
	SetEOF(destFileID, kPICTFileOffset);
	SetFPos(destFileID, fsFromStart, kPICTFileOffset);
	bytes = GetHandleSize((Handle) pic);
	HLock((Handle) pic);
	FSWrite(destFileID, &bytes, *pic);
	HUnlock((Handle) pic);
	
	//Clean up
	FSClose(destFileID);
	DisposeHandle((Handle) pic);
	
	return noErr;
}

OSErr Texture_Copy(CompressedTexturePtr sourceTexture, CompressedTexturePtr* newTexture)
{
	if((sourceTexture == nil) || (sourceTexture->image == nil))
	return kError_FatalError;
	
	*newTexture = (CompressedTexturePtr) NewPtr(GetPtrSize((Ptr) sourceTexture));
	if(*newTexture == nil)
	return MemError();
	BlockMove(sourceTexture, *newTexture, GetPtrSize((Ptr) sourceTexture));
	
	(*newTexture)->image = (PicHandle) NewHandle(GetHandleSize((Handle) sourceTexture->image));
	if((*newTexture)->image == nil)
	return MemError();
	HLock((Handle) sourceTexture->image);
	HLock((Handle) (*newTexture)->image);
	BlockMove(*(sourceTexture->image), *((*newTexture)->image), GetHandleSize((Handle) sourceTexture->image));
	HUnlock((Handle) sourceTexture->image);
	HUnlock((Handle) (*newTexture)->image);
	
	return noErr;
}
