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


#include			"WaterRace.h"
#include			"Strings.h"
#include			"Game.h"
#include			"Preferences.h"
#include			"Textures.h"
#include			"Infinity Audio Engine.h"

#include			"Dialog Utils.h"
#include			"Folder Utils.h"
#include			"Screen Capture.h"
#include			"Sound Utils.h"

#include			"Data Files.h"

#if __USE_AUDIO_CD_TRACKS__
#include			"Audio CD.h"
#endif

//CONSTANTES:

#define				kEnterDelay			10

#define				kSfxVolume			1.0

//MACROS:

#define MacOSTextAliasingAvailable() ((RoutineDescriptorPtr) IsAntiAliasedTextEnabled != (RoutineDescriptorPtr) kUnresolvedCFragSymbolAddress)

#define Mask16(s, m) ( ((((s) & 0x001F) * (m) / 255) & 0x001F) \
						+ ((((s) & 0x03E0) * (m) / 255) & 0x03E0) \
						+ ((((s) & 0x7C00) * (m) / 255) & 0x7C00) )

//VARIABLES:

static Boolean		textAliasingEnabled;
static short		textAliasingSize;
static char			decimalChar = '.';
static Handle		sfxSounds[kNbSFX];

#if __USE_AUDIO_CD_TRACKS__
static Str31		audioCDTranslationTable[] = {
						"\p<data>",
						"\pWaterRock'in Race",
						"\pWang the One",
						"\pCote D'Azur",
						"\pVenezia",
						"\pNY'Masta' Race",
						"\pScottix",
						"\pCowboy",
						"\pArmy",
						"\pRastaTouch",
						"\pRussian",
						"\pWaterfunk'83",
						"\pFeel da Powa'Race",
						"\p"
					};
#endif

//PROTOTYPES:

//File: Interface/Error system.cp
void Error_Display(short actionID, OSErr errorID, ModalFilterProcPtr eventFilterProc, Boolean fatal);

//ROUTINES:

#if 0
pascal Boolean DialogEventFilterProc(DialogPtr theDialog, EventRecord *theEvent, DialogItemIndex *itemHit)
{
	Rect					theRect;
	GrafPtr					savePort;
	
	if(theEvent->what == updateEvt) {
		if((WindowPtr) theEvent->message != mainWin)
		return false;
		
		GetPort(&savePort);
		SetPort(mainWin);
		BeginUpdate(mainWin);
			SetRect(&theRect, 0, 0, kInterface_Width, kInterface_Height);
			CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &theRect, &theRect, srcCopy, nil);
		EndUpdate(mainWin);
		SetPort(savePort);
		return true;
	}
	
	if((theEvent->what == keyDown) || (theEvent->what == autoKey)) {
		short theKey = (theEvent->message & keyCodeMask) >> 8;
	
		//User pressed the Return or Enter key
		if((theKey == 0x24) || (theKey == 0x4C) || (theKey == 0x34)) {
			*itemHit = 1;
			Press_Button(theDialog, 1);
			return true;
		}/* else if(theKey == 0x35) {
			*itemHit = 2;
			Press_Button(theDialog, 2);
			return true;
		}
		char theChar = (theEvent->message & charCodeMask);
		if((theChar == '.') && (theEvent->modifiers & cmdKey)) {
			*itemHit = 2;
			Press_Button(theDialog, 2);
			return true;
		}*/
	}
	
	return false;
}
#endif

void Wrapper_Error_Display(short actionID, OSErr errorID, ModalFilterProcPtr eventFilterProc, Boolean fatal)
{
	short					oldResFile;
	Boolean					cursorSpinning = SpinningCursor_Active();
	
	//Restore screen gamma & show cursor
#if __SCREEN_FADES__
	if(mainGamma)
	SetMonitorState(mainGamma);
#endif
	SetGWorld(nil, GetMainDevice());
	
	oldResFile = CurResFile();
	UseResFile(mainResFileID);
	
	if(cursorSpinning)
	SpinningCursor_Stop();
	
	InitCursor();
	Error_Display(actionID, errorID, nil, fatal);
	
	if(cursorSpinning)
	SpinningCursor_Start();
	
	//Clean up...
	UseResFile(oldResFile);
	//HideCursor();
}

#if __3DFX_SCREENSHOTS__

static PixMapHandle		destPixMapHandle;
static long				destSizeV,
						destSizeH;
						
static void Buffer_CompositeMethod_BufferCopy(const TQADrawContext* drawContext, const TQADevice* buffer, const TQARect* dirtyRect, void* refCon)
{
	PixMapPtr			thePixMapPtr;
	Ptr					destBaseAddress,
						sourceBaseAddress;
	long				sourceRowBytes,
						destRowBytes,
						height;
	TQADeviceMemory*	memoryDevice;
	double				*source,
						*dest;
	unsigned long		copyWidth;
												
	//Set up pointers
	thePixMapPtr = *destPixMapHandle;
	destRowBytes = thePixMapPtr->rowBytes & 0x3FFF;
	destBaseAddress = thePixMapPtr->baseAddr;
	
	if(buffer->deviceType != kQADeviceMemory)
	return;
	memoryDevice = (TQADeviceMemory*) &buffer->device.memoryDevice;
	sourceRowBytes = memoryDevice->rowBytes;
	sourceBaseAddress = (Ptr) memoryDevice->baseAddr;
	
	//Copy data
	copyWidth = destSizeH * (thePixMapPtr->pixelSize / 8) / 8;
	height = destSizeV;
	do {
		source = (double*) sourceBaseAddress;
		dest = (double*) destBaseAddress;
		for(unsigned long i = 0; i < copyWidth; ++i)
		*dest++ = *source++;
		
		sourceBaseAddress += sourceRowBytes;
		destBaseAddress += destRowBytes;
	} while(--height);
}

PicHandle Capture_3DfxVRAM()
{
	PicHandle			thePic = nil;
	GWorldPtr			oldGWorld;
	GDHandle			oldGDHandle;
	GWorldPtr			tempGWorld;
	Rect				tempRect;
	TQANoticeMethod		saveNoticeMethod,
						noticeMethod;
	void*				refCon;
	
	//Create temp GWorld
	GetGWorld(&oldGWorld, &oldGDHandle);
	SetRect(&tempRect, 0, 0, thePrefs.renderWidth, thePrefs.renderHeight);
	if(thePrefs.engineFlags & kPref_EngineFlag32BitRendering) {
		if(NewGWorld(&tempGWorld, 32, &tempRect, NULL, NULL, useTempMem))
		return nil;
	}
	else {
		if(NewGWorld(&tempGWorld, 16, &tempRect, NULL, NULL, useTempMem))
		return nil;
	}
	SetGWorld(tempGWorld, nil);
	LockPixels(GetGWorldPixMap(tempGWorld));
	BackColor(whiteColor);
	ForeColor(blackColor);
	
	//Setup params
	destPixMapHandle = tempGWorld->portPixMap;
	destSizeH = thePrefs.renderWidth;
	destSizeV = thePrefs.renderHeight;
	
	//Patch 2D composite method
	QAGetNoticeMethod(worldState->drawContext, kQAMethod_ImageBuffer2DComposite, &saveNoticeMethod, &refCon);
	noticeMethod.bufferNoticeMethod = (TQABufferNoticeMethod) Buffer_CompositeMethod_BufferCopy;
	QASetNoticeMethod(worldState->drawContext, kQAMethod_ImageBuffer2DComposite, noticeMethod, nil);
	
	//Render a frame
#if 0
	Draw_Frame(worldState);
#else
	QARenderStart(worldState->drawContext, NULL, NULL);
	QARenderEnd(worldState->drawContext, NULL);
#endif
	
	//Restore 2D composite method
	QASetNoticeMethod(worldState->drawContext, kQAMethod_ImageBuffer2DComposite, saveNoticeMethod, nil);
	
	//Create picture
	thePic = OpenPicture(&tempRect);
	CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(tempGWorld), &tempRect, &tempRect, srcCopy, nil);
	if(QDError()) {
		ClosePicture();
		DisposeHandle((Handle) thePic);
		thePic = nil;
		SysBeep(0);
	}
	else
	ClosePicture();
	
	//Clean up
	UnlockPixels(GetGWorldPixMap(tempGWorld));
	DisposeGWorld(tempGWorld);
	SetGWorld(oldGWorld, oldGDHandle);
	
	if(GetHandleSize((Handle) thePic) <= sizeof(Picture)) {
		DisposeHandle((Handle) thePic);
		thePic = nil;
		SysBeep(0);
	}
	
	return thePic;
}
#endif

OSErr Take_ScreenShot()
{
	Rect					copyRect,
							finalRect;
	PicHandle				pic;
	FSSpec					file = {0, 0, kString_Misc_ScreenshotFileName};
	Str31					timeStr;
	RGBColor				white = {0xFFFF, 0xFFFF, 0xFFFF};
	
#if __3DFX_SCREENSHOTS__
	if(Is3DfxKnownEngine(worldState->engine))
	pic = Capture_3DfxVRAM();
	else {
#else
	//No screenshot if in 3Dfx mode
	if(Is3DfxKnownEngine(worldState->engine))
	return kError_ScreenshotNotAllowed;
#endif

	SetRect(&copyRect, 0, 0, thePrefs.renderWidth, thePrefs.renderHeight);
	OffsetRect(&copyRect, (screenResolution.h - thePrefs.renderWidth) / 2 + (**mainGDHandle).gdRect.left - (**LMGetGrayRgn()).rgnBBox.left, 
		(screenResolution.v - thePrefs.renderHeight) / 2 + (**mainGDHandle).gdRect.top - (**LMGetGrayRgn()).rgnBBox.top);
	SetRect(&finalRect, -1, -1, -1, -1);
	pic = Capture_VRAM(mainGDHandle, &copyRect, &finalRect);
#if __3DFX_SCREENSHOTS__
	}
#endif
	if(!pic)
	return kError_CantCaptureVRAM;
	
	NumToString(TickCount() & 0xFFFF, timeStr);
	BlockMove(&timeStr[1], &file.name[file.name[0] + 1], timeStr[0]);
	file.name[0] += timeStr[0];
#if __MOVIE__
	extern FSSpec			destFolder;
	
	file.parID = destFolder.parID;
	file.vRefNum = destFolder.vRefNum;
#else
	file.parID = fsRtDirID;
	file.vRefNum = GetStartUpVolume();
#endif
	
#if !__MOVIE__
	//Play a sound
	Sound_PlaySFx(kSFxSnapShot);
#endif
	
#if __SCREEN_FADES__
	//Do a flash!
	Fade1ToColor(mainGDHandle, &white, 1, linearFade, nil, 0);
	Set1MonitorState(mainGDHandle, mainGamma);
#endif

	Capture_WritePICTFile(pic, &file);
	DisposeHandle((Handle) pic);
	
	return noErr;
}

void MacOSTextAliasing_Disable()
{
	if(!MacOSTextAliasingAvailable())
	return;
	
	textAliasingEnabled = IsAntiAliasedTextEnabled(&textAliasingSize);
	if(textAliasingEnabled)
	SetAntiAliasedTextEnabled(false, textAliasingSize);
}

void MacOSTextAliasing_Restore()
{
	if(!MacOSTextAliasingAvailable())
	return;
	
	if(textAliasingEnabled)
	SetAntiAliasedTextEnabled(true, textAliasingSize);
}

Boolean MacOSTextAliasing_IsEnabled()
{
	if(!MacOSTextAliasingAvailable())
	return false;
	
	textAliasingEnabled = IsAntiAliasedTextEnabled(&textAliasingSize);
	return textAliasingEnabled;
}

void CopyBits_16WithAlpha(PixMap *alphaMap, PixMap *srcMap, PixMap *destMap, Rect* alphaRect, Rect *sourceRect, Rect *destRect)
{
	Ptr					alphaLineStart,
						sourceLineStart,
						destLineStart;
	unsigned long		alphaRowBytes,
						sourceRowBytes,
						destRowBytes,
						width,
						height,
						i,
						j;
	unsigned short		*sourcePixel,
						*destPixel;
	unsigned char		*alpha;
	
	alphaRowBytes = alphaMap->rowBytes & 0x3FFF;
	alphaLineStart = alphaMap->baseAddr + alphaRect->top * alphaRowBytes + alphaRect->left;
	sourceRowBytes = srcMap->rowBytes & 0x3FFF;
	sourceLineStart = srcMap->baseAddr + sourceRect->top * sourceRowBytes + sourceRect->left * 2;
	destRowBytes = destMap->rowBytes & 0x3FFF;
	destLineStart = destMap->baseAddr + destRect->top * destRowBytes + destRect->left * 2;
	
	width = sourceRect->right - sourceRect->left;
	height = sourceRect->bottom - sourceRect->top;
	for(i = 0; i < height; ++i) {
		alpha = (unsigned char*) alphaLineStart;
		sourcePixel = (unsigned short*) sourceLineStart;
		destPixel = (unsigned short*) destLineStart;
		for(j = 0; j < width; ++j, ++alpha, ++sourcePixel, ++destPixel) {
			if(*alpha == 0xFF)
			*destPixel = *sourcePixel;
			else if(*alpha == 0x00)
			;
			else
			*destPixel = Mask16(*sourcePixel, *alpha) + Mask16(*destPixel, 0xFF - *alpha);
		}
		alphaLineStart += alphaRowBytes;
		sourceLineStart += sourceRowBytes;
		destLineStart += destRowBytes;
	}
}

void Clear_Player(PlayerConfigurationPtr player)
{
	player->playerDataPtr = nil;
	player->playerShip = nil;
	player->score = 0;
	player->lapCount = 1;
	player->rank = 0;
	player->turboCount = 1;
	player->hasJoker = false;
	
	player->bestLapTime = kMaxRaceTime * 2;
	player->lastLapTime = 0;
	player->finishTime = 0;
	player->maxSpeed = 0.0;
	player->totalDistance = 0.0;
	
	player->infoText[0] = 0;
	player->networkText[0] = 0;
	player->finished = false;
}

OSErr Sound_InitSFx()
{
	long				i;
	
	UseResFile(soundResFileID);
	for(i = 0; i < kNbSFX; ++i) {
		if(Sound_ResLoad(kSFxStartID + i, false, &sfxSounds[i]) != noErr)
		return kError_CantLoadSound;
	}
	
	return noErr;
}

void Sound_QuitSFx()
{
	long			i;
	
	for(i = 0; i < kNbSFX; ++i)
	DisposeHandle(sfxSounds[i]);
}

void Sound_PlaySFx(short SFxID)
{
	InfinityAudio_InstallSound_FX(kCreatorType, (ExtSoundHeaderPtr) *sfxSounds[SFxID], kSfxVolume, kInfinityAudio_CenteredPanoramic, kInfinityAudio_NormalPitch, nil);
}

void Sound_PlaySFxHandle(Handle sound)
{
	InfinityAudio_InstallSound_FX(kCreatorType, (ExtSoundHeaderPtr) *sound, kSfxVolume, kInfinityAudio_CenteredPanoramic, kInfinityAudio_NormalPitch, nil);
}

void CopyFirstName(Str31 source, Str31 dest)
{
	unsigned char		*p1 = &source[1],
						*p2 = &dest[1];
	
	dest[0] = 0;
	while((*p1 != ' ') && (dest[0] < source[0])) {
		*p2 = *p1;
		++dest[0];
		++p2;
		++p1;
	}
}

#if __USE_AUDIO_CD_TRACKS__
OSErr AudioCD_PlayAudioTrack_Wrapper(FSSpec* file)
{
	long			index = 0;
	
	if(!audioCD)
	return noErr;
	
	//We have the WaterRace audio CD
	if(audioCD > 0)
	do {
		if(EqualString(file->name, audioCDTranslationTable[index], false, false))
		return AudioCD_PlayTrack(index + 1);
		
		++index;
	} while(audioCDTranslationTable[index][0]);
	
	//Any audio CD
	if(audioCD < 0)
	return AudioCD_PlayTrack((Random() + 32767) % AudioCD_GetNumTracks() + 1);
	
	return paramErr;
}
#endif
