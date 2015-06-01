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


#include				"Sound Utils.h"

//ROUTINES:

OSErr Convert_Sound(Handle theSound, Boolean correctLoop)
{
	OSErr				theError;
	SoundHeaderPtr		stdHeader;
	ExtSoundHeaderPtr	extHeader;
	SoundComponentData	inputSndInfo;
	unsigned long		numFrames,
						dataOffset;
	long				offset;
	
	//Gather sound info
	if(correctLoop) {
		theError = ParseSndHeader((SndListHandle) theSound, &inputSndInfo, &numFrames, &dataOffset);
		if(theError)
		correctLoop = false;
	}
	
	GetSoundHeaderOffset((SndListHandle) theSound, &offset);
	BlockMove(*theSound + offset, *theSound, GetHandleSize(theSound) - offset);
	SetHandleSize(theSound, GetHandleSize(theSound) - offset);
	stdHeader = (SoundHeaderPtr) *theSound;
	
	//Check sound header type
	if(stdHeader->encode != extSH) {
		DisposeHandle(theSound);
		return paramErr;
	}
	
	//Check sound sample size
	extHeader = (ExtSoundHeaderPtr) *theSound;
	if(extHeader->sampleSize != 16) {
		DisposeHandle(theSound);
		return paramErr;
	}
	
	//Correct loop if required
	if(correctLoop) {	
		stdHeader->loopStart		= 0;
		stdHeader->loopEnd			= inputSndInfo.sampleCount;
	}
	
	return noErr;
}

OSErr Sound_ResLoad(short resID, Boolean correctLoop, Handle* sound)
{
	Handle				theSound;
	OSErr				theError;
	
	//Load sound resource
	theSound = Get1Resource(kSoundResType, resID);
	if(theSound == nil) {
		*sound = nil;
		return ResError();
	}
	DetachResource(theSound);
	HLock(theSound);
	
	//Correct sound
	theError = Convert_Sound(theSound, correctLoop);
	if(theError) {
		*sound = nil;
		return theError;
	}
	
	//Copy handle
	*sound = theSound;
	
	return noErr;
}

OSErr Sound_Load(FSSpec* file, Boolean correctLoop, Handle* sound)
{
	Handle				theSound;
	short				fileID;
	OSErr				theError;
	
	//Load sound from file
	fileID = FSpOpenResFile(file, fsRdPerm);
	if(fileID == -1)
	return ResError();
	UseResFile(fileID);
	theSound = Get1IndResource(kSoundResType, 1);
	if(theSound == nil)
	return ResError();
	DetachResource(theSound);
	HLock(theSound);
	CloseResFile(fileID);
	
	//Correct sound
	theError = Convert_Sound(theSound, correctLoop);
	if(theError) {
		*sound = nil;
		return theError;
	}
	
	//Copy handle
	*sound = theSound;
	
	return noErr;
}

OSErr Sound_Play(Ptr soundHeaderPtr, Boolean loop)
{
	SndCommand			theCommand;
	SCStatus			status;
	SndChannelPtr		channel = nil;
	OSErr				theError;
	
	theError = SndNewChannel(&channel, sampledSynth, initStereo, nil);
	if(theError)
	return theError;
		
	if(loop) {
		theCommand.cmd		= soundCmd;
		theCommand.param1	= 0;
		theCommand.param2	= (long) soundHeaderPtr;
		SndDoImmediate(channel, &theCommand);
		
		theCommand.cmd		= freqCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 60;
		SndDoImmediate(channel, &theCommand);
	}
	else {
		theCommand.cmd		= bufferCmd;
		theCommand.param1	= 0;
		theCommand.param2	= (long) soundHeaderPtr;
		SndDoImmediate(channel, &theCommand);
	}
	
	do {
		SndChannelStatus(channel, sizeof(SCStatus), &status);
	} while(!Button() && status.scChannelBusy);
	
	theCommand.cmd		= quietCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	SndDoImmediate(channel, &theCommand);
	
	SndDisposeChannel(channel, true);
	
	return noErr;
}