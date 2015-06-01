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


#include						<Devices.h>
#include						<Errors.h>

#include						"Audio CD.h"

//CONSTANTES:

#define kCDDriverName			"\p.AppleCD"
#define	kIOVRefNumValue			0 //was 1

//Standard Driver Calls

enum {
	csEjectTheDisc		= 7,
	csDriveStatus
};

//Audio track Control Calls
enum {
	csReadTOC			= 100,
	csReadTheQSubcode,
	csReadHeader,
	csAudioTrackSearch,
	csAudioPlay,
	csAudioPause,
	csAudioStop,
	csAudioStatus,
	csAudioScan,
	csAudioControl,
	csReadMCN,
	csReadISRC,
	csReadAudioVolume,
	csGetSpindleSpeed,
	csSetSpindleSpeed,
	csReadAudio,
	csReadAllSubcodes,
	csSetTrackList = 122,
	csGetTrackList,
	csGetTrackIndex,
	csSetPlayMode,
	csGetPlayMode
};

//Special System Control Calls 
enum {
	csGoodBye			= -1
};

enum {kNormalMode = 0, kShuffleMode, kProgMode};

#define kAudioCDPlayMode_Mute				0x00
#define kAudioCDPlayMode_RightOnly			0x05
#define kAudioCDPlayMode_Stereo				0x09
#define kAudioCDPlayMode_LeftOnly			0x0A
#define kAudioCDPlayMode_Mono				0x0F

#define	kRepeatFlag							(1 << 8)

#define	kAddressModeTrack					2
#define	kAddressModePlayListIndex			3

//STRUCTURES:

#pragma pack(2)
typedef struct
{
	UInt16		track;
	UInt8		writeProtect;
	UInt8		discInPlace;
	UInt8		installed;
	UInt8		side;
	UInt32		qLink;
	UInt16		qType;
	UInt16		dQDrive;
	UInt16		dQRefNum;
	UInt16		dQFSID;
	UInt8		twoSideFormat;
	UInt8		needsFlush;
	UInt8		discErrs;
}
csParamDiscStatusRec;
#pragma pack()

//LOCAL VARIABLES:

static SInt16		gCDDriverRef = 0;

//ROUTINES:

static void ClearMemory(void* dest, unsigned long size)
{
	long			i;
	unsigned char*	ptr = (unsigned char*) dest;
	
	for(i = 0; i < size; ++i) {
		*ptr = 0x00;
		++ptr;
	}
}

static UInt32 NumToBCD(UInt16 theNum)
{
	UInt32	theBCD;

	theBCD = 0x0000;
	
	//¥	Get the digits
	theBCD |= theNum % 10;
	theNum /= 10;

	//¥	Get the tens
	theBCD |= (theNum % 10) << 4;
	theNum /= 10;
	
	//¥	Get the hundreds
	theBCD |= (theNum % 10) << 8;
	theNum /= 10;	

	//¥	Get the thousands
	theBCD |= (theNum % 10) << 12;
	
	return theBCD;
}

static SInt16 BCDToNum(UInt8 bcd)
{
	UInt8	place;
	SInt16	num = 0;
	
	place = bcd & 0x0F;									//¥	Get the first 10 bits
	num += place;
	
	place = (bcd >> 4) & 0x0F;
	num += (place * 10);
	
	return num;
}

//PUBLIC ROUTINES:

OSErr AudioCD_Init_UseAppleDriver()
{
	return MacOpenDriver(kCDDriverName, &gCDDriverRef);
}

OSErr AudioCD_Init_UseCDName(Str63 CDName)
{
	HVolumeParam			pb;
	Str63		 			ioName;
	OSErr					theError;
	
	//Check CD name
	ClearMemory(&pb, sizeof(HVolumeParam));
	pb.ioNamePtr = ioName;
	pb.ioVolIndex = 0;
	do {
		++pb.ioVolIndex;
		theError = PBHGetVInfoSync((HParamBlockRec*) &pb);
		if(theError)
		return theError;
		
		if(EqualString(CDName, ioName, true, true)) {
			gCDDriverRef = pb.ioVDRefNum;
			return noErr;
		}
	}
	while(1);
	
	return paramErr;
}

OSErr AudioCD_Init_Manually(short driverRefNum)
{
	gCDDriverRef = driverRefNum;
	
	return noErr;
}

OSErr AudioCD_SetVolume(UInt8 left, UInt8 right)
{
	ParamBlockRec	thePB;
	
	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioControl;
	thePB.cntrlParam.csParam[0] = (left << 8) | right;
	
	return PBControlSync(&thePB);
}

OSErr AudioCD_GetVolume(UInt8* left, UInt8* right)
{
	ParamBlockRec	thePB;
	OSErr			theError;
	
	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csReadAudioVolume;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
	
	*left = (thePB.cntrlParam.csParam[0] >> 8) & 0xFF;
	*right = thePB.cntrlParam.csParam[0] & 0xFF;
	
	return noErr;
}

OSErr AudioCD_Eject()
{
	ParamBlockRec	thePB;

	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csEjectTheDisc;

	return PBControlSync(&thePB);
}

short AudioCD_GetNumTracks()
{
	ParamBlockRec	thePB;
	
	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csReadTOC;
	thePB.cntrlParam.csParam[0] = 1;
	if(PBControlSync(&thePB) != noErr)
	return 0;
	
	return BCDToNum(thePB.cntrlParam.csParam[0] & 0xFF);
}

short AudioCD_GetCurrentTrack()
{
	ParamBlockRec	thePB;
	
	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csReadTheQSubcode;
	thePB.cntrlParam.csParam[0] = 1;
	if(PBControlSync(&thePB) != noErr)
	return 0;
	
	return BCDToNum(thePB.cntrlParam.csParam[0] & 0xFF);
}

OSErr AudioCD_Stop()
{
	ParamBlockRec		thePB;

	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioStop;
	
	return PBControlSync(&thePB);
}

OSErr AudioCD_Pause()
{
	ParamBlockRec		thePB;

	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioPause;
	thePB.cntrlParam.csParam[0] = 1;
	thePB.cntrlParam.csParam[1] = 1;
	
	return PBControlSync(&thePB);
}

OSErr AudioCD_Resume()
{
	ParamBlockRec		thePB;

	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioPause;
	thePB.cntrlParam.csParam[0] = 0;
	
	return PBControlSync(&thePB);
}

Boolean AudioCD_IsPlaying()
{
	ParamBlockRec	thePB;
	
	if(!gCDDriverRef)
	return false;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioStatus;
	if(PBControlSync(&thePB) != noErr)
	return false;
	
	if((thePB.cntrlParam.csParam[0] >> 8) == 0)
	return true;

	return false;
}

#if 0
Boolean AudioCD_IsCDInserted()
{
	ParamBlockRec	thePB;
	
	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csDriveStatus;
	if(PBStatusSync(&thePB) != noErr)
	return false;
	
	return (thePB.cntrlParam.csParam[1] & 0xFF);
}
#else
Boolean AudioCD_IsCDInserted()
{
	ParamBlockRec			thePB;
	csParamDiscStatusRec*	statusRec;
	
	if(!gCDDriverRef)
	return paramErr;
	
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csDriveStatus;
	if(PBStatusSync(&thePB) != noErr)
	return false;
	
	//Check if CD is inserted
	statusRec = (csParamDiscStatusRec*) &(thePB.cntrlParam.csParam[0]);
	return (statusRec->discInPlace == 1);
}
#endif

OSErr AudioCD_PlayTrack(UInt8 trackNumber)
{
	ParamBlockRec	thePB;
	OSErr			theError;
	UInt8			buffer[1];
	
	if(!gCDDriverRef)
	return paramErr;
	
#if 0
	//Set mode to "normal"
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csSetPlayMode;
	thePB.cntrlParam.csParam[0] = kNormalMode;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
	
	//Mark stop address
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioPlay;
	thePB.cntrlParam.csParam[0] = kAddressModeTrack;
	thePB.cntrlParam.csParam[1] = 0;
	thePB.cntrlParam.csParam[2] = LoWord(NumToBCD(trackNumber));
	thePB.cntrlParam.csParam[3] = 1;
	thePB.cntrlParam.csParam[4] = kAudioCDPlayMode_Stereo;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
	
	//Mark start address
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioPlay;
	thePB.cntrlParam.csParam[0] = kAddressModeTrack;
	thePB.cntrlParam.csParam[1] = 0;
	thePB.cntrlParam.csParam[2] = LoWord(NumToBCD(trackNumber));
	thePB.cntrlParam.csParam[3] = 0;
	thePB.cntrlParam.csParam[4] = kAudioCDPlayMode_Stereo;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
#else
	//Set mode to "program + repeat"
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csSetPlayMode;
	thePB.cntrlParam.csParam[0] = kProgMode | kRepeatFlag;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
	
	//Position optical pick up
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioTrackSearch;
	thePB.cntrlParam.csParam[0] = kAddressModeTrack;
	thePB.cntrlParam.csParam[1] = 0;
	thePB.cntrlParam.csParam[2] = LoWord(NumToBCD(trackNumber));
	thePB.cntrlParam.csParam[3] = 0;
	thePB.cntrlParam.csParam[4] = kAudioCDPlayMode_Stereo;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
	
	//Set track list
	buffer[0] = trackNumber;
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csSetTrackList;
	thePB.cntrlParam.csParam[0] = 1;
	*((long*) &thePB.cntrlParam.csParam[1]) = (long) buffer;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
	
	//Mark stop address
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioPlay;
	thePB.cntrlParam.csParam[0] = kAddressModePlayListIndex;
	thePB.cntrlParam.csParam[1] = 0;
	thePB.cntrlParam.csParam[2] = 0;
	thePB.cntrlParam.csParam[3] = 1;
	thePB.cntrlParam.csParam[4] = kAudioCDPlayMode_Stereo;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
	
	//Mark start address
	ClearMemory(&thePB, sizeof(ParamBlockRec));
	thePB.cntrlParam.ioVRefNum = kIOVRefNumValue;
	thePB.cntrlParam.ioCRefNum = gCDDriverRef;
	thePB.cntrlParam.csCode = csAudioPlay;
	thePB.cntrlParam.csParam[0] = kAddressModePlayListIndex;
	thePB.cntrlParam.csParam[1] = 0;
	thePB.cntrlParam.csParam[2] = 0;
	thePB.cntrlParam.csParam[3] = 0;
	thePB.cntrlParam.csParam[4] = kAudioCDPlayMode_Stereo;
	theError = PBControlSync(&thePB);
	if(theError)
	return theError;
#endif
	
	return noErr;
}
