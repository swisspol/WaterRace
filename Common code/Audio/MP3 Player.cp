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


#include			"MP3 Player.h"

//CONSTANTES PREPROCESSEURS:

#define	__PREVENT_DEFERRED_LOOP__	1

//CONSTANTES:

#define				kInputBufferSize			32768
#define				kOutputBufferMargin			256

#define				kMaxValue					65536
#define				kBitsPerByte				8

//STRUCTURES:

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct {
	long	atomSize;			// how big this structure is (big endian)
	long	atomType;			// atom type
	char	waveData[28];
} AtomQDMCWaveFormatEx;

typedef struct {
	AudioFormatAtom			formatData;
	AtomQDMCWaveFormatEx	endianData;
	AudioTerminatorAtom		terminatorData;
} AudioCompressionAtom, *AudioCompressionAtomPtr, **AudioCompressionAtomHandle;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

//VARIABLES LOCALES:

static SoundConverter			sc = nil;
static DeferredTask				dtask;
static Boolean					audioRunning;
static unsigned long			loopPoint;
SndChannelPtr					MP3Channel = nil;
static Ptr						audioBuffer1 = nil,
								audioBuffer2 = nil,
								currentBuffer;
static ExtSoundHeader			audioBufferHeader;
static Ptr						MP3Buffer;
static unsigned long			MP3BufferSize;
static unsigned long			inputFrames,
								inputBytes,
								outputBytes,
								totalInputBytes;
static AudioCompressionAtomPtr	theDecompressionAtom = nil;
#if __PREVENT_DEFERRED_LOOP__
static Boolean					deferredTaskFired;
#endif

//MACROS:

#define Short2Long(v) ((v << 16) + v)

//ROUTINES:

static void AudioCleanBuffer(short* buffer)
{
	long				i;
	
	for(i = 0; i < outputBytes / 2; ++i)
	*buffer++ = 0;
}

static void AudioFillBuffer(short* buffer)
{
#if __PREVENT_DEFERRED_LOOP__
	deferredTaskFired = true;
#endif
	Ptr						inputPtr;
	unsigned long			outputFramesLocal,
							outputBytesLocal;
							
	if(totalInputBytes + inputBytes > MP3BufferSize)
	totalInputBytes = loopPoint;
	
	inputPtr = MP3Buffer + totalInputBytes;
	SoundConverterConvertBuffer(sc, inputPtr, inputFrames, buffer, &outputFramesLocal, &outputBytesLocal);
	totalInputBytes += inputBytes;
	
	audioBufferHeader.numFrames = outputFramesLocal;
}

static pascal void AudioCallBack(SndChannelPtr channel, SndCommand command)
{
	SndCommand			theCommand;
	
	//Check if we should stop processing sound
	if(!audioRunning)
	return;
	
	//Install current buffer
	audioBufferHeader.samplePtr = currentBuffer;
	theCommand.cmd		= bufferCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) &audioBufferHeader;
	SndDoCommand(channel, &theCommand, true);
	
#if __PREVENT_DEFERRED_LOOP__
	if(deferredTaskFired) {
#endif
	if(currentBuffer == audioBuffer1)
	currentBuffer = audioBuffer2;
	else
	currentBuffer = audioBuffer1;
	dtask.dtParam = (long) currentBuffer;
#if __PREVENT_DEFERRED_LOOP__
	deferredTaskFired = false;
#endif
	DTInstall((DeferredTaskPtr) &dtask);
#if __PREVENT_DEFERRED_LOOP__
	}
#endif
	
	//Install callback
	theCommand.cmd		= callBackCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	SndDoCommand(channel, &theCommand, true);
}

static OSErr PreloadMP3File(FSSpec *inMP3file, AudioFormatAtomPtr *outAudioAtom, CmpSoundHeaderPtr outSoundHeader)
{
	Movie theMovie;
	Track theTrack;
	Media theMedia;
	short theRefNum;
	short theResID = 0;	// we want the first movie
	Boolean wasChanged;
	
	OSErr err = noErr;
	
	// open the movie file
	err = OpenMovieFile(inMP3file, &theRefNum, fsRdPerm);
	if(err)
	return err;

	// instantiate the movie
	err = NewMovieFromFile(&theMovie, theRefNum, &theResID, NULL, newMovieActive, &wasChanged);
	CloseMovieFile(theRefNum);
	if(err)
	return err;
	theRefNum = 0;
		
	// get the first sound track
	theTrack = GetMovieIndTrackType(theMovie, 1, SoundMediaType, movieTrackMediaType);
	if(theTrack != NULL) {
		// get the sound track media
		theMedia = GetTrackMedia(theTrack);
		if(theMedia != NULL) {			
			Size size;
			Handle extension;
			
			// Version 1 of this record includes four extra fields to store information about compression ratios. It also defines
			// how other extensions are added to the SoundDescription record.
			// All other additions to the SoundDescription record are made using QT atoms. That means one or more
			// atoms can be appended to the end of the SoundDescription record using the standard [size, type]
			// mechanism used throughout the QuickTime movie resource architecture.
			// http://developer.apple.com/techpubs/quicktime/qtdevdocs/RM/frameset.htm
			SoundDescriptionV1Handle sourceSoundDescription = (SoundDescriptionV1Handle)NewHandle(0);
			
			// get the description of the sample data
			GetMediaSampleDescription(theMedia, 1, (SampleDescriptionHandle)sourceSoundDescription);
			err = GetMoviesError();

			extension = NewHandle(0);
			
			// get the "magic" decompression atom
			// This extension to the SoundDescription information stores data specific to a given audio decompressor.
			// Some audio decompression algorithms require a set of out-of-stream values to configure the decompressor
			// which are stored in a siDecompressionParams atom. The contents of the siDecompressionParams atom are dependent
			// on the audio decompressor.
			err = GetSoundDescriptionExtension((SoundDescriptionHandle)sourceSoundDescription, &extension, siDecompressionParams);
			
			if(noErr == err) {
				size = GetHandleSize(extension);
				HLock(extension);
				*outAudioAtom = (AudioFormatAtom*)NewPtr(size);
				err = MemError();
				// copy the atom data to our buffer...
				BlockMoveData(*extension, *outAudioAtom, size);
				HUnlock(extension);
			} else
			err = noErr; // if it doesn't have an atom, that's ok
			
			
			// set up our sound header
			outSoundHeader->format = (*sourceSoundDescription)->desc.dataFormat;
			outSoundHeader->numChannels = (*sourceSoundDescription)->desc.numChannels;
			outSoundHeader->sampleSize = (*sourceSoundDescription)->desc.sampleSize;
			outSoundHeader->sampleRate = (*sourceSoundDescription)->desc.sampleRate;
			
			DisposeHandle(extension);
			DisposeHandle((Handle) sourceSoundDescription);
		}
	}
	
	return err;
}

OSErr MP3Player_Start(FSSpec* fileSpec)
{
	OSErr					theError;
	short					fileID;
	long					dataLength;
	SoundComponentData		inputFormat,
							outputFormat;
	SndCommand				theCommand;
	UniversalProcPtr		routine = NewSndCallBackUPP(AudioCallBack);
	CmpSoundHeader			mySndHeader;
	
	if(MP3Channel != nil)
	MP3Player_Stop();
	
	//Open and load MP3 file
	theError = PreloadMP3File(fileSpec, (AudioFormatAtomPtr*) &theDecompressionAtom, &mySndHeader);
	if(theError)
	return theError;

	//Open MP3 file
	theError = FSpOpenDF(fileSpec, fsRdPerm, &fileID);
	if(theError)
	return theError;
	
	//Allocate memory
	GetEOF(fileID, &dataLength);
	MP3Buffer = NewPtr(dataLength);
	if(MemError())
	return MemError();
	MP3BufferSize = dataLength;
	SetFPos(fileID, fsFromStart, 0);
	theError = FSRead(fileID, &dataLength, MP3Buffer);
	if(theError)
	return theError;
	FSClose(fileID);
	
	//Setup sound converter input format
	inputFormat.flags = 0;
	inputFormat.format = mySndHeader.format;
	inputFormat.numChannels = mySndHeader.numChannels;
	inputFormat.sampleSize = mySndHeader.sampleSize;
	inputFormat.sampleRate = mySndHeader.sampleRate;
	inputFormat.sampleCount = 0;
	inputFormat.buffer = nil;
	inputFormat.reserved = 0;

	//Setup sound converter output format
	outputFormat.flags = 0;
	outputFormat.format = kSoundNotCompressed;
	outputFormat.numChannels = inputFormat.numChannels;
	outputFormat.sampleSize = inputFormat.sampleSize;
	outputFormat.sampleRate = inputFormat.sampleRate;
	outputFormat.sampleCount = 0;
	outputFormat.buffer = nil;
	outputFormat.reserved = 0;
	
	//Open sound converter
	theError = SoundConverterOpen(&inputFormat, &outputFormat, &sc);
	if(theError)
	return theError;
	theError = SoundConverterSetInfo(sc, siDecompressionParams, theDecompressionAtom);
	if(theError)
	return theError;
	theError = SoundConverterGetBufferSizes(sc, kInputBufferSize, &inputFrames, &inputBytes, &outputBytes);
	if(theError)
	return theError;
	theError = SoundConverterBeginConversion(sc);
	if(theError)
	return theError;
	
	//Allocate buffers
	audioBuffer1 = NewPtr(outputBytes + kOutputBufferMargin);
	if(audioBuffer1 == nil)
	return MemError();
	audioBuffer2 = NewPtr(outputBytes + kOutputBufferMargin);
	if(audioBuffer2 == nil)
	return MemError();
	
	//Create sound channel
	theError = SndNewChannel(&MP3Channel, sampledSynth, initStereo, routine);
	if(theError)
	return theError;
	
	//Prepare audio buffer #1
	AudioCleanBuffer((short*) audioBuffer1);
	AudioCleanBuffer((short*) audioBuffer2);
	//audioBufferHeader.samplePtr = (Ptr) audioBuffer1;
	audioBufferHeader.numChannels = inputFormat.numChannels;
	audioBufferHeader.sampleRate = inputFormat.sampleRate;
	audioBufferHeader.loopStart = 0;
	audioBufferHeader.loopEnd = 0;
	audioBufferHeader.encode = extSH;
	audioBufferHeader.baseFrequency = kMiddleC;
	audioBufferHeader.numFrames = outputBytes / (inputFormat.numChannels * inputFormat.sampleSize / kBitsPerByte);
	//audioBufferHeader1.AIFFSampleRate = ?
	audioBufferHeader.markerChunk = nil;
	audioBufferHeader.instrumentChunks = nil;
	audioBufferHeader.AESRecording = nil;
	audioBufferHeader.sampleSize = inputFormat.sampleSize;
	audioBufferHeader.futureUse1 = 0;
	audioBufferHeader.futureUse2 = 0;
	audioBufferHeader.futureUse3 = 0;
	audioBufferHeader.futureUse4 = 0;
	
	//Prepare channel
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) &audioBufferHeader;
	SndDoImmediate(MP3Channel, &theCommand);
	
	//Setup deferred task params
	dtask.qType = dtQType;
	dtask.dtFlags = 0;
	dtask.dtAddr = NewDeferredTaskProc(AudioFillBuffer);
	dtask.dtReserved = 0;
	
	//Fill buffer and set it as buffer to play
	totalInputBytes = 0;
	AudioFillBuffer((short*) audioBuffer1);
	currentBuffer = audioBuffer1;
	
	//Call call back
	audioRunning = true;
#if __PREVENT_DEFERRED_LOOP__
	deferredTaskFired = true;
#endif
	loopPoint = 0;
	theCommand.cmd		= nullCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	AudioCallBack(MP3Channel, theCommand);
	
	return noErr;
}

OSErr MP3Player_Stop()
{
	if(MP3Channel == nil)
	return paramErr;
	
	audioRunning = false;
	AudioCleanBuffer((short*) audioBuffer1);
	AudioCleanBuffer((short*) audioBuffer2);
	
	SndDisposeChannel(MP3Channel, true);
	MP3Channel = nil;
	DisposePtr(audioBuffer1);
	DisposePtr(audioBuffer2);
	DisposePtr(MP3Buffer);
	audioBuffer1 = nil;
	audioBuffer2 = nil;
	MP3Buffer = nil;
	SoundConverterClose(sc);
	sc = nil;
	DisposePtr((Ptr) theDecompressionAtom);
	theDecompressionAtom = nil;
	
	return noErr;
}

void MP3Player_Volume(short volume)
{
	SndCommand		theCommand;
	
	if(MP3Channel == nil)
	return;
	
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= Short2Long(volume);
	SndDoImmediate(MP3Channel, &theCommand);
}
