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


#include <DriverGestalt.h>

enum {
	kCSDriveStatus			= 8,
	kCSReadTOC				= 100
};

enum {
	kCDTypeUnknown			= 0x0000,
	kCDTypeHighSierra		= 0x4242,
	kCDTypeISO9660			= 0x4147,
	kCDTypeAudio			= 0x4A48
};

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

#define			kAppleCDDriverName			"\p.AppleCD"
#define			kIOVRefNumValue				0

inline void ClearMemory(void* ptr, long size)
{
	do {
		*((char*) ptr)++ = 0x00;
	} while(--size);
}

inline SInt16 BCDToNum(UInt8 bcd)
{
	UInt8	place;
	SInt16	num = 0;
	
	place = bcd & 0x0F;
	num += place;
	
	place = (bcd >> 4) & 0x0F;
	num += (place * 10);
	
	return num;
}

inline Boolean CheckForCD(Str31 CDName, short trackNumber, SInt16 min, SInt16 sec)
{
	SInt16					driverRefNum = -1;
	ParamBlockRec			params;
	csParamDiscStatusRec	*statusRec;
	Str255		 			ioName;
	HVolumeParam			pb;
	
	{
		long				response;
	
		enum {
		  gestaltMacOSCompatibilityBoxAttr = FOUR_CHAR_CODE('bbox'), /* Classic presence and features */
		  gestaltMacOSCompatibilityBoxPresent = 0, /* True if running under the Classic */
		  gestaltMacOSCompatibilityBoxHasSerial = 1, /* True if Classic serial support is implemented. */
		  gestaltMacOSCompatibilityBoxless = 2  /* True if we're Boxless (screen shared with Carbon/Cocoa) */
		};

		//Are we running under MacOS X Classic?
		if((Gestalt(gestaltMacOSCompatibilityBoxAttr, &response) == noErr) && (response & (1 << gestaltMacOSCompatibilityBoxPresent)))
		return true;
	}
	
	//Open connection with Apple CD driver
	if(MacOpenDriver(kAppleCDDriverName, &driverRefNum) != noErr) {
		//We suppose the CD does not use the standard Apple CD driver
		//so let's find the driver by volume name!
		
		HVolumeParam			pb;
		Str63		 			ioName;
		
		ClearMemory(&pb, sizeof(HVolumeParam));
		pb.ioNamePtr = ioName;
		pb.ioVolIndex = 0;
		do {
			++pb.ioVolIndex;
			pb.ioVRefNum = 0;
			if(PBHGetVInfoSync((HParamBlockRec*) &pb) != noErr)
			return false;
			
			ioName[0] = CDName[0]; //Fix!
			if(EqualString(CDName, ioName, true, true)) {
				driverRefNum = pb.ioVDRefNum;
				break;
			}
		}
		while(1);
	}

	ClearMemory(&params, sizeof(ParamBlockRec));
	statusRec = (csParamDiscStatusRec*) &(params.cntrlParam.csParam[0]);
	params.cntrlParam.ioVRefNum = kIOVRefNumValue;
	params.cntrlParam.ioCRefNum = driverRefNum;
	params.cntrlParam.csCode = kCSDriveStatus;
	if(PBStatusSync(&params) != noErr)
	return false;
	
	//Check if CD is inserted
	if(statusRec->discInPlace != 1)
	return false;
	
	//Check CD name
	ClearMemory(&pb, sizeof(HVolumeParam));
	pb.ioNamePtr = ioName;
	pb.ioVolIndex = 0;
	do {
		++pb.ioVolIndex;
		pb.ioVRefNum = 0;
		if(PBHGetVInfoSync((HParamBlockRec*) &pb) != noErr)
		return false;
	}
	while((pb.ioVDRefNum != driverRefNum) || (pb.ioVFSID == kCDTypeAudio)); //Skip audio partition on MacOS 9.x
	ioName[0] = CDName[0]; //Fix!
	if(!EqualString(CDName, ioName, true, true))
	return false;
	
	//Check length of CD (data + audio)
	ClearMemory(&params, sizeof(ParamBlockRec));
	params.cntrlParam.ioVRefNum = kIOVRefNumValue;
	params.cntrlParam.ioCRefNum = driverRefNum;
	params.cntrlParam.csCode = kCSReadTOC;
	params.cntrlParam.csParam[0] = 2;
	if(PBControlSync(&params) != noErr)
	return false;
	if(BCDToNum((params.cntrlParam.csParam[0] >> 8) & 0xFF) != min)
	return false;
	if(BCDToNum(params.cntrlParam.csParam[0] & 0xFF) != sec)
	return false;
	
	//Check number of audio tracks
	ClearMemory(&params, sizeof(ParamBlockRec));
	params.cntrlParam.ioVRefNum = kIOVRefNumValue;
	params.cntrlParam.ioCRefNum = driverRefNum;
	params.cntrlParam.csCode = kCSReadTOC;
	params.cntrlParam.csParam[0] = 1;
	if(PBControlSync(&params) != noErr)
	return false;
	if(BCDToNum(params.cntrlParam.csParam[0] & 0xFF) != trackNumber)
	return false;
	
	return true;
}

inline short XCheckForCD(Str31 CDName, short trackNumber, SInt16 min, SInt16 sec)
{
	DrvQElPtr			dqPtr;
	OSErr				err;
	ParamBlockRec		params;
	Str63				name;
	DriverGestaltParam	pb;
	
	//Scan driver queue
	dqPtr = (DrvQElPtr) GetDrvQHdr()->qHead;
	while(dqPtr != NULL) {
		//Make sure this driver has mounted a CD-ROM
		pb.ioVRefNum = dqPtr->dQDrive;
		pb.ioCRefNum = dqPtr->dQRefNum;
		pb.csCode = kDriverGestaltCode;
		pb.driverGestaltSelector = kdgDeviceType;
		err = PBStatusSync((ParmBlkPtr) &pb);
		if((err == noErr) && (pb.driverGestaltResponse == kdgCDType)) {
			//Make sure CD-ROM name is correct
			params.volumeParam.ioCompletion = nil;
			params.volumeParam.ioVolIndex = 0;
			params.volumeParam.ioNamePtr = name;
			params.volumeParam.ioVRefNum = dqPtr->dQDrive;
			err = PBGetVInfoSync(&params);
			name[0] = CDName[0]; //Patch!
			if((err == noErr) && EqualString(CDName, name, true, true)) {
				//Make sure CD-ROM length is correct (data + audio)
				params.cntrlParam.ioCompletion = nil;
				params.cntrlParam.ioVRefNum = dqPtr->dQDrive;
				params.cntrlParam.ioCRefNum = dqPtr->dQRefNum;
				params.cntrlParam.csCode = kCSReadTOC;
				params.cntrlParam.csParam[0] = 2;
				err = PBControlSync(&params);
				if((err == noErr) && (BCDToNum((params.cntrlParam.csParam[0] >> 8) & 0xFF) == min)
					&& (BCDToNum(params.cntrlParam.csParam[0] & 0xFF) == sec)) {
					//Make sure number of audio tracks is correct
					params.cntrlParam.ioCompletion = nil;
					params.cntrlParam.ioVRefNum = dqPtr->dQDrive;
					params.cntrlParam.ioCRefNum = dqPtr->dQRefNum;
					params.cntrlParam.csCode = kCSReadTOC;
					params.cntrlParam.csParam[0] = 1;
					err = PBControlSync(&params);
					if((err == noErr) && (BCDToNum(params.cntrlParam.csParam[0] & 0xFF) == trackNumber)) {
						return dqPtr->dQRefNum;
					}
				}
			}
		}
		
		//Get next driver
		dqPtr = (DrvQEl *) dqPtr->qLink;
	}
	
	return 0;
}
