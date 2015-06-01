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


#include				<fp.h>

#include				"Infinity Structures.h"
#include				"Infinity Audio Engine.h"
#include				"Vector.h"
#include				"Matrix.h"
#include				"Infinity Error codes.h"
#include				"Infinity Rendering.h"

#include				"Clock.h"

//CONSTANTES PREPROCESSEURS:

#define	__REDUCE_VOLUME__	0
#define	__SAMPLE_CLIPPING__	1
#define	__TRUE_PANORAMIC__	1
#define	__QUADRATIC_PAN__	1
#define	__INTERPOLATE_VOLUMES__	0 //Produce sparks!
#define	__INTERPOLATED_44KHZ__	1
#define	__SET_HARDWARE_TO_44KHZ__	1
#define	__DOPPLER_EFFECT__	0
#define	__USE_DEFERRED_TASKS__	1 //Do not check for task completion before firing
#define	__USE_DOUBLE_BUFFERS__	0
#define	__SAVE_VOLATILE_FP_REGISTERS__	0
#define	__PREVENT_DEFERRED_LOOP__	1

//CONSTANTES:

#define					kMaxAudioReferences			128
#define					kSampleSize					2 //16Bits
#define					kBitsPerSample				8
#define					kInSampleRate				rate22050hz
#define					kInSampleRate2				rate22khz
#define					kOutSampleRate				rate44khz
#define					kInNumChannels				1
#define					kOutNumChannels				2

#define					kDefaultVolume				0xFF //max

#define					kInfinityAudioMode_FX					'Fx  '
#define					kInfinityAudioMode_Loop					'Loop'
#define					kInfinityAudioMode_Random				'Rndm'
#define					kInfinityAudioMode_AmbientLoop			'ALop'
#define					kInfinityAudioMode_AmbientRandomPlay	'ARnd'
#define					kInfinityAudioMode_AmbientFX			'AbFx'

#define					kSampleClippingHigh			32760.0 //32767
#define					kSampleClippingLow			-32760.0 //-32768

#define					kRandomMax					18000

#if __DOPPLER_EFFECT__
#define					kSpeedOfSound				343.0 //m.s-1
#define					kMaxSoundSpeed				(0.99 * kSpeedOfSound)
#endif

//STRUCTURES:

typedef struct AudioReference
{
	OSType				mode;
	
	float				maxDistance,
						minDistance;
						
	unsigned long		minTime,
						maxTime,
						playTime;
						
	float				volume,
						pitch,
						panoramic;
	MatrixPtr			matrix;
	VectorPtr			position;
	
	ExtSoundHeaderPtr	soundData;
	short*				samplePtr;
	float				currentSample,
						numSamples;
						
	float				leftVolume,
						rightVolume;
#if __INTERPOLATE_VOLUMES__
	float				lastLeftVolume,
						lastRightVolume;
#endif
#if __DOPPLER_EFFECT__
	float				dopplerPitch;
#endif
	Boolean				play;
	AudioID				id;
	OSType				clientID;
	Boolean				isStereo;
};
typedef AudioReference* AudioReferencePtr;

#if __SAVE_VOLATILE_FP_REGISTERS__
typedef struct FPURegs
{
	float	FPR00;
	float	FPR01;
	float	FPR02;
	float	FPR03;
	float	FPR04;
	float	FPR05;
	float	FPR06;
	float	FPR07;
	float	FPR08;
	float	FPR09;
	float	FPR10;
	float	FPR11;
	float	FPR12;
	float	FPR13;
	float	FPSCR;
} FPURegsRec,*FPURegsPtr,**FPURegsHdl;
#endif

//MACROS:

#define Concatenate(l, r) ((l << 16) + r)

//VARIABLES LOCALES:

static unsigned long		audioCount;
static AudioReference		audioList[kMaxAudioReferences];
static SndChannelPtr		audioChannel = nil;
static short				*audioBuffer1 = nil;
#if __USE_DOUBLE_BUFFERS__
static short				*audioBuffer2 = nil;
#endif
static ExtSoundHeader		audioBufferHeader1,
							audioBufferHeader2;
static ExtSoundHeaderPtr	currentAudioHeader;
static Boolean				audioRunning;
static MatrixPtr			audioMatrix;
static AudioID				nextRef = 0;
static unsigned long		numBufferSamples;
#if __USE_DEFERRED_TASKS__ && __PREVENT_DEFERRED_LOOP__
static Boolean				deferredTaskFired;
#endif

#if __USE_DEFERRED_TASKS__
static DeferredTask			dtask;
#endif

#if __INTERPOLATED_44KHZ__
static float				lastLeftSum,
							lastRightSum;
#endif

static long					_audioSeed = 12345;

//ROUTINES:

#if __SAVE_VOLATILE_FP_REGISTERS__
asm void _SavePowerPCVolatileFPRs(FPURegsPtr pFPURegsPtr)
{
	stfd	fp0,FPURegs.FPR00(r3)
	stfd	fp1,FPURegs.FPR01(r3)
	stfd	fp2,FPURegs.FPR02(r3)
	stfd	fp3,FPURegs.FPR03(r3)
	stfd	fp4,FPURegs.FPR04(r3)
	stfd	fp5,FPURegs.FPR05(r3)
	stfd	fp6,FPURegs.FPR06(r3)
	stfd	fp7,FPURegs.FPR07(r3)
	stfd	fp8,FPURegs.FPR08(r3)
	stfd	fp9,FPURegs.FPR09(r3)
	stfd	fp10,FPURegs.FPR10(r3)
	stfd	fp11,FPURegs.FPR11(r3)
	stfd	fp12,FPURegs.FPR12(r3)
	stfd	fp13,FPURegs.FPR13(r3)

    mffs    fp0                                 // copy fpscr into fp0
    stfd    fp0,FPURegs.FPSCR(r3)               // save fpscr
	blr
}

asm void _RestorePowerPCVolatileFPRs(FPURegsPtr pFPURegsPtr)
{
	lfd	fp0, FPURegs.FPSCR(r3)            // load saved fpscr into fp0
	mtfsf	0xff, fp0                           // move it back into the fpscr

	lfd		fp0, FPURegs.FPR00(r3)
	lfd		fp1, FPURegs.FPR01(r3)
	lfd		fp2, FPURegs.FPR02(r3)
	lfd		fp3, FPURegs.FPR03(r3)
	lfd		fp4, FPURegs.FPR04(r3)
	lfd		fp5, FPURegs.FPR05(r3)
	lfd		fp6, FPURegs.FPR06(r3)
	lfd		fp7, FPURegs.FPR07(r3)
	lfd		fp8, FPURegs.FPR08(r3)
	lfd		fp9, FPURegs.FPR09(r3)
	lfd		fp10, FPURegs.FPR10(r3)
	lfd		fp11, FPURegs.FPR11(r3)
	lfd		fp12, FPURegs.FPR12(r3)
	lfd		fp13, FPURegs.FPR13(r3)
	blr
}
#endif

inline long RandomInteger()
{
	_audioSeed = (_audioSeed * 16807) % kRandomMax;
	
	return _audioSeed;
}

static void AudioCleanBuffer(short* buffer)
{
	long				i;
	
	for(i = 0; i < numBufferSamples * kOutNumChannels; ++i)
	*buffer++ = 0;
}

static pascal void AudioFillBuffer(long bufferPtr)
{
#if __USE_DEFERRED_TASKS__ && __PREVENT_DEFERRED_LOOP__
	deferredTaskFired = true;
#endif
#if __SAVE_VOLATILE_FP_REGISTERS__
	FPURegsRec			tFPURegsRec;
	_SavePowerPCVolatileFPRs(&tFPURegsRec);
	{
#endif
	long				i,
						j;
	short*				dest = (short*) bufferPtr;
	float				leftSum,
						rightSum,
						volume,
						distance;
	AudioReferencePtr	reference;
	Vector				localPosition;
	Matrix				negatedAudioMatrix;
	unsigned long		audioTime = RClock_GetAbsoluteTime();
	short				*leftSample,
						*rightSample;
						
	//Compute volumes & panoramics
	Matrix_Negate(audioMatrix, &negatedAudioMatrix);
	reference = audioList;
	for(j = 0; j < audioCount; ++j, ++reference) {
		if((reference->mode == kInfinityAudioMode_FX) || (reference->mode == kInfinityAudioMode_Loop) || (reference->mode == kInfinityAudioMode_Random)) {
			if((reference->mode == kInfinityAudioMode_Random) && (reference->playTime > audioTime)) {
				reference->play = false;
				continue;
			}
			reference->play = true;
			
			//Compute volume & panoramic
#if __TRUE_PANORAMIC__
			if(reference->panoramic >= 0.0) {
				reference->rightVolume = reference->volume;
				reference->leftVolume = (1.0 - reference->panoramic) * reference->volume;
			}
			else {
				reference->rightVolume = (1.0 + reference->panoramic) * reference->volume;
				reference->leftVolume = reference->volume;
			}
#else
			reference->rightVolume = (0.5 + reference->panoramic) * reference->volume;
			reference->leftVolume = (0.5 - reference->panoramic) * reference->volume;
#endif		
		}
		else {
			if((reference->mode == kInfinityAudioMode_AmbientRandomPlay) && (reference->playTime > audioTime)) {
				reference->play = false;
#if __INTERPOLATE_VOLUMES__
				reference->lastLeftVolume = 0.0;
				reference->lastRightVolume = 0.0;
#endif
				continue;
			}
			reference->play = true;
			
			//Transform sound in sound space
			if(reference->matrix != nil) {
				Matrix_TransformVector(reference->matrix, reference->position, &localPosition);
				Matrix_TransformVector(&negatedAudioMatrix, &localPosition, &localPosition);
			}
			else
			Matrix_TransformVector(&negatedAudioMatrix, reference->position, &localPosition);
			distance = FVector_Length(localPosition);
			if(distance > reference->maxDistance) {
				reference->leftVolume = 0.0;
				reference->rightVolume = 0.0;
				continue;
			}
			
			//Compute volume
			if(distance < reference->minDistance)
			volume = reference->volume;
			else
			volume = (1.0 - (distance - reference->minDistance) / (reference->maxDistance - reference->minDistance)) * reference->volume;
			
			//Compute panoramic
			localPosition.x /= -distance; //!zero divide! //Switch left-right
			
#if __QUADRATIC_PAN__
			if(localPosition.x < 0.0)
			localPosition.x *= -localPosition.x;
			else
			localPosition.x *= localPosition.x;
#endif
			
#if __TRUE_PANORAMIC__
			if(localPosition.x >= 0.0) {
				reference->rightVolume = volume;
				reference->leftVolume = (1.0 - localPosition.x) * volume;
			}
			else {
				reference->rightVolume = (1.0 + localPosition.x) * volume;
				reference->leftVolume = volume;
			}
#else
			reference->rightVolume = (0.5 + localPosition.x / 2.0) * volume;
			reference->leftVolume = (0.5 - localPosition.x / 2.0) * volume;
#endif
		}
	}
	
	for(i = 0; i < numBufferSamples / 2; ++i) {
		//Compute sample
		leftSum = 0.0;
		rightSum = 0.0;
		reference = audioList;
		for(j = 0; j < audioCount; ++j, ++reference) {
			if(!reference->play)
			continue;
			
			if((reference->leftVolume != 0.0) || (reference->rightVolume != 0.0)) {
				if(reference->isStereo) {
					leftSample = reference->samplePtr + (long) reference->currentSample * 2;
					rightSample = leftSample + 1;
				}
				else {
					leftSample = reference->samplePtr + (long) reference->currentSample;
					rightSample = leftSample;
				}
				
#if __INTERPOLATE_VOLUMES__
				float leftVolume = reference->lastLeftVolume + (reference->leftVolume - reference->lastLeftVolume) / (float) (numBufferSamples / 2) * (float) i;
				float rightVolume = reference->lastRightVolume + (reference->rightVolume - reference->lastRightVolume) / (float) (numBufferSamples / 2) * (float) i;
				
				leftSum += (float) *leftSample * leftVolume;
				rightSum += (float) *rightSample * rightVolume;
#else
				leftSum += (float) *leftSample * reference->leftVolume;
				rightSum += (float) *rightSample * reference->rightVolume;
#endif
			}
			
			reference->currentSample += reference->pitch;
			
			if(reference->currentSample >= reference->numSamples) {
				if((reference->mode == kInfinityAudioMode_AmbientLoop) || (reference->mode == kInfinityAudioMode_Loop))
				reference->currentSample = 0.0;
				else if((reference->mode == kInfinityAudioMode_AmbientRandomPlay) || (reference->mode == kInfinityAudioMode_Random)) {
					reference->playTime = audioTime + reference->minTime + RandomInteger() % reference->maxTime;
					reference->currentSample = 0.0;
					reference->play = false;
				}
				else {
					--audioCount;
					audioList[j] = audioList[audioCount];
					
					//Fix
					--j;
					--reference;
				}
			}
		}

#if __REDUCE_VOLUME__
		leftSum /= 2.0;
		rightSum /= 2.0;
#endif

#if __SAMPLE_CLIPPING__
		if(leftSum > kSampleClippingHigh)
		leftSum = kSampleClippingHigh;
		else if(leftSum < kSampleClippingLow)
		leftSum = kSampleClippingLow;
		
		if(rightSum > kSampleClippingHigh)
		rightSum = kSampleClippingHigh;
		else if(rightSum < kSampleClippingLow)
		rightSum = kSampleClippingLow;
#endif
	
#if __INTERPOLATED_44KHZ__
		//Copy left sample
		*dest++ = (lastLeftSum + leftSum) / 2.0;
		//Copy right sample
		*dest++ = (lastRightSum + rightSum) / 2.0;
		//Copy left sample
		*dest++ = leftSum;
		//Copy right sample
		*dest++ = rightSum;
		
		lastLeftSum = leftSum;
		lastRightSum = rightSum;
#else
		//Copy left sample
		*dest++ = leftSum;
		//Copy right sample
		*dest++ = rightSum;
		//Copy left sample
		*dest++ = leftSum;
		//Copy right sample
		*dest++ = rightSum;
#endif
	}
	
#if __INTERPOLATE_VOLUMES__
	reference = audioList;
	for(j = 0; j < audioCount; ++j, ++reference) {
		reference->lastLeftVolume = reference->leftVolume;
		reference->lastRightVolume = reference->rightVolume;
	}
#endif

#if __SAVE_VOLATILE_FP_REGISTERS__
	}
	_RestorePowerPCVolatileFPRs(&tFPURegsRec);
#endif
}

static pascal void AudioCallBack(SndChannelPtr channel, SndCommand* command)
{
	SndCommand			theCommand;
	
	//Check if we should stop processing sound
	if(!audioRunning)
	return;
	
	//Install current buffer
	theCommand.cmd		= bufferCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) currentAudioHeader;
	SndDoCommand(channel, &theCommand, true);
	
#if __USE_DOUBLE_BUFFERS__
	//Fill other buffer
	if(currentAudioHeader == &audioBufferHeader1) {
#if __USE_DEFERRED_TASKS__
		dtask.dtParam = (long) audioBuffer2;
#else
		AudioFillBuffer(audioBuffer2);
#endif
		currentAudioHeader = &audioBufferHeader2;
	}
	else {
#if __USE_DEFERRED_TASKS__
		dtask.dtParam = (long) audioBuffer1;
#else
		AudioFillBuffer(audioBuffer1);
#endif
		currentAudioHeader = &audioBufferHeader1;
	}
#if __USE_DEFERRED_TASKS__
	DTInstall((DeferredTaskPtr) &dtask);
#endif
#else
#if __USE_DEFERRED_TASKS__
#if __PREVENT_DEFERRED_LOOP__
	if(deferredTaskFired) {
#endif
	dtask.dtParam = (long) audioBuffer1;
	deferredTaskFired = false;
	DTInstall((DeferredTaskPtr) &dtask);
#if __PREVENT_DEFERRED_LOOP__
	}
	/*else
	AudioCleanBuffer(audioBuffer1);*/
#endif
#else
	AudioFillBuffer(audioBuffer1); //We need to dop a sound buffer
#endif
#endif
	
	//Install callback
	theCommand.cmd		= callBackCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	SndDoCommand(channel, &theCommand, true);
}

OSErr InfinityAudio_Init(MatrixPtr matrix)
{
	SndCommand			theCommand;
	UniversalProcPtr	routine = NewSndCallBackUPP(AudioCallBack);
	OSErr				theError;
	SoundComponentData	outputFormat;
	
	if(audioChannel != nil)
	return kError_FatalError;
	
#if __SET_HARDWARE_TO_44KHZ__
	//Set output rate to 44Khz
	SetSoundOutputInfo(nil, siSampleRate, (void*) rate44khz);
#endif
	
	//Gather hardware buffer size
	if(GetSoundOutputInfo(nil, siHardwareFormat, &outputFormat) == noErr)
	numBufferSamples = outputFormat.sampleCount;
	else
	numBufferSamples = 4096; //Assume largest value
	
	//Allocate buffers
	audioBuffer1 = (short*) NewPtr(sizeof(short) * numBufferSamples * kOutNumChannels);
	if(audioBuffer1 == nil)
	return MemError();
#if __USE_DOUBLE_BUFFERS__
	audioBuffer2 = (short*) NewPtr(sizeof(short) * numBufferSamples * kOutNumChannels);
	if(audioBuffer2 == nil)
	return MemError();
#endif
	
	//Create a sound channel
	theError = SndNewChannel(&audioChannel, sampledSynth, initStereo, routine);
	if(theError)
	return theError;
	
	//Prepare audio buffer #1
	AudioCleanBuffer(audioBuffer1);
	audioBufferHeader1.samplePtr = (Ptr) audioBuffer1;
	audioBufferHeader1.numChannels = kOutNumChannels;
	audioBufferHeader1.sampleRate = kOutSampleRate;
	audioBufferHeader1.loopStart = 0;
	audioBufferHeader1.loopEnd = 0;
	audioBufferHeader1.encode = extSH;
	audioBufferHeader1.baseFrequency = kMiddleC;
	audioBufferHeader1.numFrames = numBufferSamples;
	//audioBufferHeader1.AIFFSampleRate = ?
	audioBufferHeader1.markerChunk = nil;
	audioBufferHeader1.instrumentChunks = nil;
	audioBufferHeader1.AESRecording = nil;
	audioBufferHeader1.sampleSize = kSampleSize * kBitsPerSample;
	audioBufferHeader1.futureUse1 = 0;
	audioBufferHeader1.futureUse2 = 0;
	audioBufferHeader1.futureUse3 = 0;
	audioBufferHeader1.futureUse4 = 0;
	
#if __USE_DOUBLE_BUFFERS__
	//Prepare audio buffer #2
	AudioCleanBuffer(audioBuffer2);
	audioBufferHeader2.samplePtr = (Ptr) audioBuffer2;
	audioBufferHeader2.numChannels = kOutNumChannels;
	audioBufferHeader2.sampleRate = kOutSampleRate;
	audioBufferHeader2.loopStart = 0;
	audioBufferHeader2.loopEnd = 0;
	audioBufferHeader2.encode = extSH;
	audioBufferHeader2.baseFrequency = kMiddleC;
	audioBufferHeader2.numFrames = numBufferSamples;
	//audioBufferHeader2.AIFFSampleRate = ?
	audioBufferHeader2.markerChunk = nil;
	audioBufferHeader2.instrumentChunks = nil;
	audioBufferHeader2.AESRecording = nil;
	audioBufferHeader2.sampleSize = kSampleSize * kBitsPerSample;
	audioBufferHeader2.futureUse1 = 0;
	audioBufferHeader2.futureUse2 = 0;
	audioBufferHeader2.futureUse3 = 0;
	audioBufferHeader2.futureUse4 = 0;
#endif
	
	//Prepare channel
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) &audioBufferHeader1;
	SndDoImmediate(audioChannel, &theCommand);
	
	//Reset params
	audioCount = 0;
	audioRunning = false;
	
	//Set volume & matrix
	InfinityAudio_Volume(kDefaultVolume);
	InfinityAudio_SetMatrix(matrix);
	
#if __USE_DEFERRED_TASKS__
	//Setup deferred task params
	dtask.qType = dtQType;
	dtask.dtFlags = 0;
	dtask.dtAddr = NewDeferredTaskProc(AudioFillBuffer);
	dtask.dtReserved = 0;
#endif
	
	return noErr;
}

void InfinityAudio_Start()
{
	SndCommand		theCommand;
	
	if((audioChannel == nil) || audioRunning)
	return;
	
	//Fill buffer #1 and set it as buffer to play
	AudioFillBuffer((long) audioBuffer1);
	currentAudioHeader = &audioBufferHeader1;
	
#if __INTERPOLATED_44KHZ__
	lastLeftSum = 0.0;
	lastRightSum = 0.0;
#endif

	//Call call back
	audioRunning = true;
#if __USE_DEFERRED_TASKS__ && __PREVENT_DEFERRED_LOOP__
	deferredTaskFired = true;
#endif
	theCommand.cmd		= nullCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	AudioCallBack(audioChannel, &theCommand);
}

void InfinityAudio_CleanUp(OSType clientID)
{
	long				i;
	
	if(audioChannel == nil)
	return;
	
	if(clientID == kAnyClient) {
		audioCount = 0;
		AudioCleanBuffer(audioBuffer1);
#if __USE_DOUBLE_BUFFERS__
		AudioCleanBuffer(audioBuffer2);
#endif
	}
	else {
		for(i = 0; i < audioCount; ++i)
		if(audioList[i].clientID == clientID) {
			--audioCount;
			audioList[i] = audioList[audioCount];
			--i;
		}
	}
}

void InfinityAudio_Stop()
{
	if((audioChannel == nil) || !audioRunning)
	return;
	
	audioRunning = false;
	AudioCleanBuffer(audioBuffer1);
#if __USE_DOUBLE_BUFFERS__
	AudioCleanBuffer(audioBuffer2);
#endif
}

void InfinityAudio_Quit()
{
	if(audioChannel == nil)
	return;
	
	InfinityAudio_Stop();
	SndDisposeChannel(audioChannel, true);
	audioChannel = nil;
	DisposePtr((Ptr) audioBuffer1);
	audioBuffer1 = nil;
#if __USE_DOUBLE_BUFFERS__
	DisposePtr((Ptr) audioBuffer2);
	audioBuffer2 = nil;
#endif
}

void InfinityAudio_Volume(short volume)
{
	SndCommand		theCommand;
	
	if(audioChannel == nil)
	return;
	
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= Concatenate(volume, volume);
	SndDoCommand(audioChannel, &theCommand, true);
}

void InfinityAudio_SetMatrix(MatrixPtr matrix)
{
	audioMatrix = matrix;
}

static OSErr LowLevel_SoundInstall(OSType clientID, OSType mode, ExtSoundHeaderPtr soundHeader, MatrixPtr matrix, VectorPtr position, float minDistance, float maxDistance, 
	float volume, float pitch, float panoramic, unsigned long minTime, unsigned long maxTime, float timeOffset, AudioIDPtr id)
{
	AudioReferencePtr			reference;
	
	//Check if audio is on
	if(audioChannel == nil)
	return kError_FatalError;
	
	//Check sound format
	if(soundHeader->encode != extSH)
	return kError_SoundFormatNotSupported;
	if(((soundHeader->sampleRate >> 16) != (kInSampleRate >> 16)) && ((soundHeader->sampleRate >> 16) != (kInSampleRate2 >> 16)))
	return kError_SoundSampleRateNotSupported;
	if((soundHeader->numChannels != 1) && (soundHeader->numChannels != 2))
	return kError_SoundFormatNotSupported;
	if(soundHeader->sampleSize != kSampleSize * kBitsPerSample)
	return kError_SoundSampleSizeNotSupported;
	
	//Check if we have too many items already
	if(audioCount >= kMaxAudioReferences)
	return kError_TooManySounds;
	
	reference = &audioList[audioCount];
	reference->mode = mode;
	reference->maxDistance = maxDistance;
	reference->minDistance = minDistance;
	reference->minTime = minTime;
	reference->maxTime = maxTime;
	reference->playTime = RClock_GetAbsoluteTime() + minTime + RandomInteger() % maxTime;
	reference->volume = volume;
	reference->pitch = pitch;
	
#if __QUADRATIC_PAN__
	if(panoramic < 0.0)
	panoramic *= -panoramic;
	else
	panoramic *= panoramic;
#endif

#if __TRUE_PANORAMIC__
	reference->panoramic = panoramic;
#else
	reference->panoramic = panoramic / 2.0;
#endif

	reference->matrix = matrix;
	reference->position = position;
	reference->soundData = soundHeader;
	if(soundHeader->samplePtr == nil)
	reference->samplePtr = (short*) &(soundHeader->sampleArea[0]);
	else
	reference->samplePtr = (short*) soundHeader->samplePtr;
	reference->currentSample = timeOffset * (float) (kInSampleRate >> 16);
	reference->numSamples = soundHeader->numFrames;
	
#if __INTERPOLATE_VOLUMES__
	if((mode == kInfinityAudioMode_FX) || (mode == kInfinityAudioMode_Random)) {
		reference->lastLeftVolume = volume;
		reference->lastRightVolume = volume;
	}
	else {
		reference->lastLeftVolume = 0.0;
		reference->lastRightVolume = 0.0;
	}
#endif

	reference->id = nextRef++;
	reference->clientID = clientID;
	if(soundHeader->numChannels == 2)
	reference->isStereo = true;
	else
	reference->isStereo = false;
	
	++audioCount;
	
	if(id != nil)
	*id = reference->id;
	
	return noErr;
}

OSErr InfinityAudio_InstallSound_FX(OSType clientID, ExtSoundHeaderPtr soundHeader, float volume, float panoramic, float pitch, AudioIDPtr id)
{
	return LowLevel_SoundInstall(clientID, kInfinityAudioMode_FX, soundHeader, nil, nil, 0.0, 0.0, volume, pitch, panoramic, 0, 0, 0.0, id);
}

OSErr InfinityAudio_InstallSound_Loop(OSType clientID, ExtSoundHeaderPtr soundHeader, float volume, float panoramic, float pitch, AudioIDPtr id)
{
	return LowLevel_SoundInstall(clientID, kInfinityAudioMode_Loop, soundHeader, nil, nil, 0.0, 0.0, volume, pitch, panoramic, 0, 0, 0.0, id);
}

OSErr InfinityAudio_InstallSound_Random(OSType clientID, ExtSoundHeaderPtr soundHeader, float volume, float panoramic, float pitch, unsigned long minTime, unsigned long maxTime, AudioIDPtr id)
{
	return LowLevel_SoundInstall(clientID, kInfinityAudioMode_Random, soundHeader, nil, nil, 0.0, 0.0, volume, pitch, panoramic, minTime, maxTime, 0.0, id);
}

OSErr InfinityAudio_InstallSound_AmbientFX(OSType clientID, ExtSoundHeaderPtr soundHeader, MatrixPtr matrix, VectorPtr position, float minDistance, float maxDistance, 
	float volume, float pitch, AudioIDPtr id)
{
	return LowLevel_SoundInstall(clientID, kInfinityAudioMode_AmbientFX, soundHeader, matrix, position, minDistance, maxDistance, volume, pitch, 0.0, 0, 0, 0.0, id);
}

OSErr InfinityAudio_InstallSound_AmbientLoop(OSType clientID, ExtSoundHeaderPtr soundHeader, MatrixPtr matrix, VectorPtr position, float minDistance, float maxDistance, 
	float volume, float pitch, AudioIDPtr id)
{
	return LowLevel_SoundInstall(clientID, kInfinityAudioMode_AmbientLoop, soundHeader, matrix, position, minDistance, maxDistance, volume, pitch, 0.0, 0, 0, 0.0, id);
}

OSErr InfinityAudio_InstallSound_AmbientRandom(OSType clientID, ExtSoundHeaderPtr soundHeader, MatrixPtr matrix, VectorPtr position, float minDistance, float maxDistance, 
	float volume, float pitch, unsigned long minTime, unsigned long maxTime, AudioIDPtr id)
{
	return LowLevel_SoundInstall(clientID, kInfinityAudioMode_AmbientRandomPlay, soundHeader, matrix, position, minDistance, maxDistance, volume, pitch, 0.0, minTime, maxTime, 0.0, id);
}

OSErr InfinityAudio_InstallSound_CinematicFX(OSType clientID, ExtSoundHeaderPtr soundHeader, unsigned long timeOffset)
{
	return LowLevel_SoundInstall(clientID, kInfinityAudioMode_FX, soundHeader, nil, nil, 0.0, 0.0, kInfinityAudio_MaxVolume, kInfinityAudio_NormalPitch, kInfinityAudio_CenteredPanoramic, 0, 0, (float) timeOffset / (float) kTimeUnit, nil);
}

void InfinityAudio_SetSoundParams(OSType clientID, AudioID id, float volume, float pitch)
{
	AudioReferencePtr		reference = audioList;
	long					i;

	for(i = 0; i < audioCount; ++i, ++reference)
	if(((clientID == kAnyClient) || (reference->clientID == clientID)) && (reference->id == id)) {
		reference->volume = volume;
		reference->pitch = pitch;
		return;
	}
}

void InfinityAudio_UninstallSound(OSType clientID, AudioID id)
{
	AudioReferencePtr		reference = audioList;
	long					i;

	for(i = 0; i < audioCount; ++i, ++reference)
	if(((clientID == kAnyClient) || (reference->clientID == clientID)) && (reference->id == id)) {
		--audioCount;
		audioList[i] = audioList[audioCount];
		return;
	}
}

OSErr InfinityAudio_InstallObject(OSType clientID, ObjectPtr object, MatrixPtr globalPos)
{
	ShapeData_AudioPtr	data;
	long				k;
	OSErr				theError;
	
	//Scan for audio shaders
	for(k = 0; k < object->shapeCount; ++k)
	if(object->shapeList[k]->shading == kShader_Audio) {
		if(object->shapeList[k]->dataPtr == nil)
		return kError_FatalError;
		data = (ShapeData_AudioPtr) object->shapeList[k]->dataPtr;
		
		
		if(data->flags & kFlag_RandomAudioPlay)
		theError = LowLevel_SoundInstall(clientID, kInfinityAudioMode_AmbientRandomPlay, &data->soundHeader, globalPos, 
			&data->position, data->minDistance, data->maxDistance, data->volume, data->pitch, 0.0, 
			data->minTime, data->maxTime, 0.0, nil);
		else
		theError = LowLevel_SoundInstall(clientID, kInfinityAudioMode_AmbientLoop, &data->soundHeader, globalPos,
			&data->position, data->minDistance, data->maxDistance, data->volume, data->pitch, 0.0,
			0, 0, 0.0, nil);
		if(theError)
		return theError;
	}
	
	return noErr;
}
