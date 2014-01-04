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


#include			<AIFF.h>
#include			<fp.h>

#include			"QDesign Player.h"

//CONSTANTES PREPROCESSEURS:

#define	__PREVENT_DEFERRED_LOOP__	1

//CONSTANTES:

#define				kInputBufferSize			32768
#define				kOutputBufferMargin			256

#define				kMaxValue					65536
#define				kBitsPerByte				8

#define				kChunkBufferSize 			128
#define				kChunkHeaderSize			8

#define				kFORM						(1<<0)
#define				kFormatVersion				(1<<1)
#define				kCommon						(1<<2)
#define				kSoundData					(1<<3)

//STRUCTURES:

typedef union {
	ChunkHeader					generic;
	ContainerChunk				container;
	FormatVersionChunk			formatVersion;
	CommonChunk					common;
	ExtCommonChunk				extCommon;
	SoundDataChunk				soundData;
	MarkerChunk					marker;
	InstrumentChunk				instrument;
	MIDIDataChunk				midiData;
	AudioRecordingChunk			audioRecording;
	CommentsChunk				comments;
	TextChunk					text;
} ChunkTemplate, *ChunkTemplatePtr;

typedef struct {
	long			atomSize;			// how big this structure is (big endian)
	long			atomType;			// atom type
	char			waveData[28];
} AtomQDMCWaveFormatEx;

typedef struct {
	AudioFormatAtom				formatData;
	AtomQDMCWaveFormatEx		endianData;
	AudioTerminatorAtom			terminatorData;
} AudioCompressionAtom, *AudioCompressionAtomPtr, **AudioCompressionAtomHandle;

typedef struct {
	short 							numChannels;
	short 							sampleSize;
	UnsignedFixed 					sampleRate;
	long 							numFrames;
	extended80 						AIFFSampleRate;
} SoundInfo_Definition;
typedef SoundInfo_Definition SoundInfo;
typedef SoundInfo_Definition* SoundInfoPtr;

//VARIABLES LOCALES:

static SoundConverter			sc = nil;
static DeferredTask				dtask;
static Boolean					audioRunning;
static unsigned long			loopPoint;
SndChannelPtr					QDChannel = nil;
static Ptr						audioBuffer1 = nil,
								audioBuffer2 = nil,
								currentBuffer;
static ExtSoundHeader			audioBufferHeader;
static Ptr						QDesignBuffer;
static unsigned long			QDesignBufferSize;
static unsigned long			inputFrames,
								inputBytes,
								outputBytes,
								totalInputBytes;
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
							
	if(totalInputBytes + inputBytes > QDesignBufferSize)
	totalInputBytes = loopPoint;
	
	inputPtr = QDesignBuffer + totalInputBytes;
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

static Fixed ASoundLongDoubleToFix(long double theLD)
{
	unsigned long	theResult		= 0;
	unsigned short	theSignificant	= 0,
					theFraction		= 0;

	if(theLD < kMaxValue) {
		theSignificant = theLD;
		theFraction = theLD - theSignificant;
		if (theFraction > kMaxValue) {
			/* Won't be able to convert */
			theSignificant	= 0;
			theFraction		= 0;
		}
	}

	theResult |= theSignificant;
	theResult = theResult << (sizeof (unsigned short) * kBitsPerByte);
	theResult |= theFraction;

	return theResult;
}

static OSErr Analyze_AIFFFile(short refNum, SoundInfoPtr soundInfo, long* dataOffset, long *dataLength, Ptr waveData)
{
	char					chunkBuffer[kChunkBufferSize];
	OSErr					err = noErr;
	unsigned long			type;
	ChunkTemplatePtr		chunkTemplate = nil;
	long					filePosition = 0,
							byteCount = 0;
	unsigned short			chunkFlags = 0;
	
	do {
		SetFPos(refNum, fsFromStart, filePosition);
		byteCount = kChunkBufferSize;
		err = FSRead(refNum, &byteCount, chunkBuffer);
		if(err == eofErr) {
			err = noErr;
			break;
		}
			
		chunkTemplate = (ChunkTemplatePtr) chunkBuffer;

		err = badFileFormat;
		switch(chunkTemplate->generic.ckID) {
			
			case FORMID:
			if((chunkFlags & kFORM) == false) {
				chunkFlags |= kFORM;
				filePosition += sizeof (ContainerChunk);		// Can't use ckSize because it's the size of the file, not this header
				
				//theSoundInfo->bytesTotal = chunkTemplate->container.ckSize;
				type = chunkTemplate->container.formType; //AIFF or AIFC file?
				err = noErr;
			}
			break;
			
			case FormatVersionID:
			if((chunkFlags & kFormatVersion) == false) {
				chunkFlags |= kFormatVersion;
				filePosition += chunkTemplate->formatVersion.ckSize + kChunkHeaderSize;
				
				if(chunkTemplate->formatVersion.timestamp != AIFCVersion1)
				err = badFileFormat;
				else
				err = noErr;
			}
			break;
			
			case CommonID:
			if((chunkFlags & kCommon) == false) {
				chunkFlags |= kCommon;
				filePosition += chunkTemplate->common.ckSize + kChunkHeaderSize;

				long double		tempLD	= 0.0;
				x80told(&(chunkTemplate->common.sampleRate), &tempLD);

				soundInfo->sampleRate = ASoundLongDoubleToFix(tempLD);
				soundInfo->numChannels = chunkTemplate->common.numChannels;
				soundInfo->numFrames = chunkTemplate->common.numSampleFrames;
				soundInfo->AIFFSampleRate = chunkTemplate->common.sampleRate;
				soundInfo->sampleSize = chunkTemplate->common.sampleSize;
				
				if(type == AIFCID) {
					//Sound data is compressed!
					if(chunkTemplate->extCommon.compressionType != 'QDMC')
					err = paramErr;
					else
					err = noErr;
				}
				else
				err = paramErr; //Sound data is not compressed!
			}
			break;
			
			case SoundDataID:
			if((chunkFlags & kSoundData) == false) {
				*dataOffset = filePosition + 16;
				*dataLength = chunkTemplate->soundData.ckSize - kChunkHeaderSize;
				err = noErr;
				
				chunkFlags |= kSoundData;
				filePosition += chunkTemplate->soundData.ckSize + kChunkHeaderSize;
			}
			break;
			
			case 'wave': //QDesign data
			filePosition += chunkTemplate->generic.ckSize + kChunkHeaderSize;
			
			BlockMoveData((Ptr) (&(chunkTemplate->generic)) + kChunkHeaderSize, waveData, chunkTemplate->generic.ckSize);
			err = noErr;
			break;
			
			default:
			filePosition += chunkTemplate->generic.ckSize + kChunkHeaderSize;
			err = noErr;
			break;
			
		}
		
		if(err)
		break;
	} while(true);

	return err;
}

OSErr QDPlayer_Start(FSSpec* fileSpec)
{
	OSErr					theError;
	short					fileID;
	long					dataOffset,
							dataLength;
	SoundInfo				info;
	AudioCompressionAtom	decomAtom;
	SoundComponentData		inputFormat,
							outputFormat;
	SndCommand				theCommand;
	UniversalProcPtr		routine = NewSndCallBackUPP(AudioCallBack);
	
	if(QDChannel != nil)
	QDPlayer_Stop();
	
	//Open and load AIFF file
	theError = FSpOpenDF(fileSpec, fsRdPerm, &fileID);
	if(theError)
	return theError;
	theError = Analyze_AIFFFile(fileID, &info, &dataOffset, &dataLength, (Ptr) &(decomAtom.formatData));
	if(theError)
	return theError;

	//Allocate memory
	QDesignBuffer = NewPtr(dataLength);
	if(MemError())
	return MemError();
	QDesignBufferSize = dataLength;
	SetFPos(fileID, fsFromStart, dataOffset);
	theError = FSRead(fileID, &dataLength, QDesignBuffer);
	if(theError)
	return theError;
	FSClose(fileID);
	
	//Setup sound converter input format
	inputFormat.flags = 0;
	inputFormat.format = 'QDMC';
	inputFormat.numChannels = info.numChannels;
	inputFormat.sampleSize = info.sampleSize;
	inputFormat.sampleRate = info.sampleRate;
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
	theError = SoundConverterSetInfo(sc, siDecompressionParams, &decomAtom);
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
	theError = SndNewChannel(&QDChannel, sampledSynth, initStereo, routine);
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
	SndDoImmediate(QDChannel, &theCommand);
	
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
	AudioCallBack(QDChannel, theCommand);
	
	return noErr;
}

OSErr QDPlayer_Stop()
{
	if(QDChannel == nil)
	return paramErr;
	
	audioRunning = false;
	AudioCleanBuffer((short*) audioBuffer1);
	AudioCleanBuffer((short*) audioBuffer2);
	
	SndDisposeChannel(QDChannel, true);
	QDChannel = nil;
	DisposePtr(audioBuffer1);
	DisposePtr(audioBuffer2);
	DisposePtr(QDesignBuffer);
	audioBuffer1 = nil;
	audioBuffer2 = nil;
	QDesignBuffer = nil;
	SoundConverterClose(sc);
	sc = nil;

	return noErr;
}

void QDPlayer_Volume(short volume)
{
	SndCommand		theCommand;
	
	if(QDChannel == nil)
	return;
	
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= Short2Long(volume);
	SndDoImmediate(QDChannel, &theCommand);
}