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


//STRUCTURES:

typedef struct Request_Definition {
	GDHandle				displayDevice;
	DisplayIDType			displayID;
	short					reqWidth,
							reqHeight,
							reqDepth,
							width,
							height;
	VDSwitchInfoRec*		switchInfo;
	Boolean					matchFound,
							possibleFound;
};
typedef Request_Definition Request;
typedef Request_Definition* RequestPtr;

//VARIABLES:

static Handle			theDisplayState = nil;

//ROUTINES:

OSErr ScreenDevice_Init()
{
	return DMBeginConfigureDisplays(&theDisplayState);
}

OSErr ScreenDevice_Quit()
{
	OSErr			theError = noErr;
	
	if(theDisplayState != nil)
	theError = DMEndConfigureDisplays(theDisplayState);
	
#if 0
	if(theDisplayState != nil)
	DisposeHandle(theDisplayState);
#endif
	
	return theError;
}

OSErr ScreenDevice_GetResolution(GDHandle device, VDSwitchInfoRec* switchInfo)
{
	return DMGetDisplayMode(device, switchInfo);
}

OSErr ScreenDevice_SetResolution(GDHandle device, VDSwitchInfoRec* switchInfo)
{
	unsigned long	depthMode = switchInfo->csMode;
	
#if 1
	return DMSetDisplayMode(device, switchInfo->csData, &depthMode, (unsigned long) switchInfo, theDisplayState);
#else
	return DMSetDisplayMode(device, switchInfo->csData, &depthMode, 0, theDisplayState);
#endif
}

static pascal void ModeListIteratorProc(long* ioUserData, DMListIndexType inItemIndex, DMDisplayModeListEntryPtr inDisplayModeInfo)
{
	unsigned long			theDepthCount,
							theIndex;
	RequestPtr				request = (RequestPtr) ioUserData;
	VPBlockPtr				modeInfo;
	
	//Is this mode a possible resolution?
	modeInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[0].depthVPBlock;
	if((modeInfo->vpBounds.right - modeInfo->vpBounds.left) < request->reqWidth)
	return;
	if((modeInfo->vpBounds.bottom - modeInfo->vpBounds.top) < request->reqHeight)
	return;
	
	//Does this mode have the depth we are looking for?
	//Get number of depths in this mode
	theDepthCount = inDisplayModeInfo->displayModeDepthBlockInfo->depthBlockCount;
	
	//Scan depths
	for(theIndex = 0; theIndex < theDepthCount; ++theIndex) {
		VPBlockPtr				modeInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthVPBlock;
		VDSwitchInfoRec*		switchInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthSwitchInfo;
		unsigned long			theSwitchFlags;
		Boolean					modeIsValid;
		
		//Gather mode info
		modeInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthVPBlock;
		if(modeInfo->vpPixelSize == request->reqDepth) {
			//ask the Display Manager if the mode is valid
			DMCheckDisplayMode(request->displayDevice, switchInfo->csData, switchInfo->csMode, &theSwitchFlags, 0, &modeIsValid);
#if 1
			if(theSwitchFlags & (1 << kNoSwitchConfirmBit))
			modeIsValid = true;
			else
			modeIsValid = false;
#endif

			//This mode is valid
			if(modeIsValid) {
				if(((modeInfo->vpBounds.right - modeInfo->vpBounds.left) == request->reqWidth) && ((modeInfo->vpBounds.bottom - modeInfo->vpBounds.top) == request->reqHeight)) {
					BlockMove(switchInfo, request->switchInfo, sizeof(VDSwitchInfoRec));
					request->matchFound = true;
					break;
				}
				else if(((modeInfo->vpBounds.right - modeInfo->vpBounds.left) < request->width) && ((modeInfo->vpBounds.bottom - modeInfo->vpBounds.top) < request->height)) {
					BlockMove(switchInfo, request->switchInfo, sizeof(VDSwitchInfoRec));
					request->width = modeInfo->vpBounds.right - modeInfo->vpBounds.left;
					request->height = modeInfo->vpBounds.bottom - modeInfo->vpBounds.top;
					request->possibleFound = true;
				}
			}
		}
	}
}

OSErr ScreenDevice_LookForResolution(GDHandle device, short width, short height, short depth, VDSwitchInfoRec* switchInfo)
{
	DMDisplayModeListIteratorUPP	theUPP;
	DMListType						theDMList;
	unsigned long					index,
									numModes = 0;
	OSErr							theError;
	DisplayIDType					theDisplayID;
	Request							theRequest;
	
	theError = DMGetDisplayIDByGDevice(device, &theDisplayID, false);
	if(theError)
	return theError;
	
	theRequest.displayDevice = device;
	theRequest.displayID = theDisplayID;
	theRequest.reqWidth = width;
	theRequest.reqHeight = height;
	theRequest.reqDepth = depth;
	theRequest.width = 32000;
	theRequest.height = 32000;
	theRequest.switchInfo = switchInfo;
	theRequest.matchFound = false;
	theRequest.possibleFound = false;
	
	theUPP = NewDMDisplayModeListIteratorProc(ModeListIteratorProc);
	theError = DMNewDisplayModeList(theDisplayID, 0, 0, (UInt32*) &numModes, &theDMList);
	if(theError)
	return theError;
	
	for(index = 0; index < numModes; ++index) {
		DMGetIndexedDisplayModeFromList(theDMList, index, 0, theUPP, (void*) &theRequest);
		if(theRequest.matchFound)
		break;
	}
	
	DMDisposeList(theDMList);
	DisposeRoutineDescriptor(theUPP);
	
	if(!theRequest.possibleFound && !theRequest.matchFound)
	return -43;
	
	return noErr;
}

static pascal void MaxModeListIteratorProc(long* ioUserData, DMListIndexType inItemIndex, DMDisplayModeListEntryPtr inDisplayModeInfo)
{
	unsigned long			theDepthCount,
							theIndex;
	RequestPtr				request = (RequestPtr) ioUserData;
	VPBlockPtr				modeInfo;
	
	//Is this mode smaller than what we already have?
	modeInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[0].depthVPBlock;
	if((modeInfo->vpBounds.right - modeInfo->vpBounds.left) <= request->width)
	return;
	if((modeInfo->vpBounds.bottom - modeInfo->vpBounds.top) <= request->height)
	return;
	
	//Does this mode have the depth we are looking for?
	//Get number of depths in this mode
	theDepthCount = inDisplayModeInfo->displayModeDepthBlockInfo->depthBlockCount;
	
	//Scan depths
	for(theIndex = 0; theIndex < theDepthCount; ++theIndex) {
		VPBlockPtr				modeInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthVPBlock;
		VDSwitchInfoRec*		switchInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthSwitchInfo;
		unsigned long			theSwitchFlags;
		Boolean					modeIsValid;
		
		//Gather mode info
		modeInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthVPBlock;
		if(modeInfo->vpPixelSize == request->reqDepth) {
			//ask the Display Manager if the mode is valid
			DMCheckDisplayMode(request->displayDevice, switchInfo->csData, switchInfo->csMode, &theSwitchFlags, 0, &modeIsValid);
#if __REDEFINE_VALID_MODES__
			if(theSwitchFlags & (1 << kNoSwitchConfirmBit))
			modeIsValid = true;
			else
			modeIsValid = false;
#endif

			//This mode is valid
			if(modeIsValid) {
				if(((modeInfo->vpBounds.right - modeInfo->vpBounds.left) > request->width) && ((modeInfo->vpBounds.bottom - modeInfo->vpBounds.top) > request->height)) {
					request->width = modeInfo->vpBounds.right - modeInfo->vpBounds.left;
					request->height = modeInfo->vpBounds.bottom - modeInfo->vpBounds.top;
				}
			}
		}
	}
}

OSErr SL_GetMaxResolution(GDHandle device, short* width, short* height, short depth)
{
	DMDisplayModeListIteratorUPP	theUPP;
	DMListType						theDMList;
	unsigned long					index,
									numModes = 0;
	OSErr							theError;
	DisplayIDType					theDisplayID;
	Request							theRequest;
	
	theError = DMGetDisplayIDByGDevice(device, &theDisplayID, false);
	if(theError)
	return theError;
	
	theRequest.displayDevice = device;
	theRequest.displayID = theDisplayID;
	theRequest.reqDepth = depth;
	theRequest.width = 0;
	theRequest.height = 0;
	
	theUPP = NewDMDisplayModeListIteratorProc(MaxModeListIteratorProc);
	theError = DMNewDisplayModeList(theDisplayID, 0, 0, (UInt32*) &numModes, &theDMList);
	if(theError)
	return theError;
	
	for(index = 0; index < numModes; ++index)
	DMGetIndexedDisplayModeFromList(theDMList, index, 0, theUPP, (void*) &theRequest);
	
	DMDisposeList(theDMList);
	DisposeRoutineDescriptor(theUPP);
	
	*width = theRequest.width;
	*height = theRequest.height;
	
	return noErr;
}
