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


#include				"ScreenLib.h"
#include				"Dialog Utils.h"

//CONSTANTES PREPROCESSEUR:

#define	__REDEFINE_VALID_MODES__ 1
#define	__SELECT_BEST_REFRESH_RATE__	1

//CONSTANTES:

#define						kRefreshRatePickerDialogID	9970
#define						kDisplayPickerDialogID		9971

#define						kMaxDevices					8

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

typedef struct ScreenDesktop
{
	short					deviceIDCount,
							currentDevice,
							mainDevice;
	Rect					desktopRect;
	DisplayIDType			deviceIDList[kMaxDevices];
	Rect					deviceRectList[kMaxDevices];
};
typedef ScreenDesktop* ScreenDesktopPtr;

//ROUTINES:

Boolean SL_ModeAvailable(GDHandle device, VDSwitchInfoRec* switchInfo)
{
	Boolean				modeOK;
	unsigned long			flags;
	
	if(DMCheckDisplayMode(device, switchInfo->csData, switchInfo->csMode, &flags, 0, &modeOK))
	return false;
	
	return modeOK;
}

OSErr SL_Switch(GDHandle device, VDSwitchInfoRec* switchInfo)
{
	unsigned long	depthMode = switchInfo->csMode;
	
	return DMSetDisplayMode(device, switchInfo->csData, &depthMode, 0, nil);
}

OSErr SL_GetCurrent(GDHandle device, VDSwitchInfoRec* switchInfo)
{
	OSErr			theError;
	
	theError = DMGetDisplayMode(device, switchInfo);
	
	return theError;
}

static void Add_Secure_Suffix(Str63 name)
{
	++name[0];
	name[name[0]] = ' ';
	++name[0];
	name[name[0]] = '(';
	++name[0];
	name[name[0]] = '*';
	++name[0];
	name[name[0]] = ')';
}

OSErr SL_GetDeviceResolution(GDeviceInfoPtr g, short* width, short* height, short depth, VDSwitchInfoRec** switchInfo, Str255 message)
{
	long				i,
						j,
						k;
	ModeInfoPtr			m;
	DepthInfoPtr		d;
	short				reqWidth = *width,
						reqHeight = *height,
						perfectMatchCount = 0,
						possibleMatchCount = 0;
	Boolean				found = false;
	VDSwitchInfoRec		*perfectMatchList[kMaxPerfectMatchs],
						*possibleMatchList[kMaxPerfectMatchs];
	Str63				perfectNameList[kMaxPerfectMatchs],
						possibleNameList[kMaxPerfectMatchs];
	Boolean				perfectIsSecure[kMaxPerfectMatchs],
						possibleIsSecure[kMaxPerfectMatchs];
						
	*switchInfo = nil;
	*width = *height = 32000;
	
	for(i = 0; i < kMaxPerfectMatchs; ++i) {
		perfectIsSecure[i] = false;
		possibleIsSecure[i] = false;
	}
	
	for(j = 0; j < g->modeCount; ++j) {
		m = g->modeList[j];
		for(k = 0; k < m->depthCount; ++k) {
			d = &m->depthList[k];
			if(d->depth == depth) {
				
				if((m->resolution.right == reqWidth) && (m->resolution.bottom == reqHeight)) {
					perfectMatchList[perfectMatchCount] = &d->switchInfo;
					BlockMove(m->modeName, perfectNameList[perfectMatchCount], sizeof(Str63));
					if(d->valid)
					perfectIsSecure[perfectMatchCount] = true;
					++perfectMatchCount;
				}
				
				if((m->resolution.right >= reqWidth) && (m->resolution.bottom >= reqHeight)) {
					if((m->resolution.right < *width) || (m->resolution.bottom < *height)) {
						*width = m->resolution.right;
						*height = m->resolution.bottom;
						*switchInfo = &d->switchInfo;
						
						possibleMatchList[0] = &d->switchInfo;
						BlockMove(m->modeName, possibleNameList[0], sizeof(Str63));
						if(d->valid)
						possibleIsSecure[0] = true;
						possibleMatchCount = 1;
						
						found = true;
					}
					else if((m->resolution.right == *width) && (m->resolution.bottom == *height)) {
						possibleMatchList[possibleMatchCount] = &d->switchInfo;
						BlockMove(m->modeName, possibleNameList[possibleMatchCount], sizeof(Str63));
						if(d->valid)
						possibleIsSecure[possibleMatchCount] = true;
						++possibleMatchCount;
					}
				}
			}
		}
	}
	
	if(!found)
	return kError_NoMatchFound;
	
	//Ask user to select a refresh rate
	{
		DialogPtr			theDialog;
		GrafPtr				savePort;
		short				itemHit = 0;
		ControlHandle		control;
		ListHandle			screenList;
		Cell				theCell = {0,0};
		
		GetPort(&savePort);
		theDialog = GetNewDialog(kRefreshRatePickerDialogID, nil, (WindowPtr) -1);
		SetPort(theDialog);
		BackColor(whiteColor);
		
		if(message == nil)
		ParamText("\p", nil, nil, nil);
		else
		ParamText(message, nil, nil, nil);
		
		GetDialogItemAsControl(theDialog, 2, &control);
		GetListBoxListHandle(control, &screenList);
		LSetDrawingMode(true, screenList);
		if(perfectMatchCount) {
			LAddRow(perfectMatchCount, 1000, screenList);
			for(i = 0; i < perfectMatchCount; ++i) {
				theCell.v = i;
				if(perfectIsSecure[i]) {
#if __SELECT_BEST_REFRESH_RATE__
					itemHit = i;
#endif
					Add_Secure_Suffix(perfectNameList[i]);
				}
				LSetCell(&perfectNameList[i][1], perfectNameList[i][0], theCell, screenList);
			}
		}
		else {
			LAddRow(possibleMatchCount, 1000, screenList);
			for(i = 0; i < possibleMatchCount; ++i) {
				theCell.v = i;
				if(possibleIsSecure[i]) {
#if __SELECT_BEST_REFRESH_RATE__
					itemHit = i;
#endif
					Add_Secure_Suffix(possibleNameList[i]);
				}
				LSetCell(&possibleNameList[i][1], possibleNameList[i][0], theCell, screenList);
			}
		}
		theCell.v = itemHit;
		LSetSelect(true, theCell, screenList);
		
		SetDialogDefaultItem(theDialog, 1);
		FlushEvents(everyEvent, 0);
		
		do {
			ModalDialog(nil, &itemHit);
			
			theCell.h = 0;
			theCell.v = 0;
			if(!LGetSelect(true, &theCell, screenList)) {
				if(perfectMatchCount)
				theCell.v = perfectMatchCount - 1;
				else
				theCell.v = possibleMatchCount - 1;
				theCell.h = 0;
				LSetSelect(true, theCell, screenList);
			}
			
		} while(itemHit != 1);
		
		DisposeDialog(theDialog);
		SetPort(savePort);
		
		//Return selected frequency
		if(perfectMatchCount) {
			*width = reqWidth;
			*height = reqHeight;
			*switchInfo = perfectMatchList[theCell.v];
		}
		else
		*switchInfo = possibleMatchList[theCell.v];
	}
	
	return noErr;
}

static pascal void ModeListIteratorProc(void* ioUserData, DMListIndexType inItemIndex, DMDisplayModeListEntryPtr inDisplayModeInfo)
{
	GDHandle				displayDevice = nil;
	DisplayIDType			displayID = (DisplayIDType) ((long*) ioUserData)[0];
	GDeviceInfoPtr			info = (GDeviceInfoPtr) ((long*) ioUserData)[1];
	ModeInfoPtr				modeInfoPtr;
	DepthInfoPtr			depthInfoPtr;
	unsigned long			theDepthCount,
							theIndex,
							finalDepthCount = 0;
	
	theDepthCount = inDisplayModeInfo->displayModeDepthBlockInfo->depthBlockCount;
	
	modeInfoPtr = (ModeInfoPtr) NewPtr(sizeof(ModeInfo) + theDepthCount * sizeof(DepthInfo));
	if(modeInfoPtr == nil)
	return;
	info->modeList[info->modeCount] = modeInfoPtr;
	++info->modeCount;
	
	BlockMove(inDisplayModeInfo->displayModeName, modeInfoPtr->modeName, inDisplayModeInfo->displayModeName[0] + 1);
	
	//Scan depths
	for(theIndex = 0; theIndex < theDepthCount; ++theIndex) {
		VDSwitchInfoRec*		switchInfo;
		unsigned long			theSwitchFlags;
		VPBlockPtr				modeInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthVPBlock;
		
		//We are interested only in 16 & 32 Bits depth
		if((modeInfo->vpPixelSize != 16) && (modeInfo->vpPixelSize != 32))
		continue;
		
		//Fill ModeInfo structure
		modeInfoPtr->resolution.left = modeInfo->vpBounds.left;
		modeInfoPtr->resolution.right = modeInfo->vpBounds.right;
		modeInfoPtr->resolution.top = modeInfo->vpBounds.top;
		modeInfoPtr->resolution.bottom = modeInfo->vpBounds.bottom;
		modeInfoPtr->hRes = modeInfo->vpHRes;
		modeInfoPtr->vRes = modeInfo->vpVRes;
		
		//Fill DepthInfo structure
		depthInfoPtr = &modeInfoPtr->depthList[finalDepthCount];
		//Set depth
		depthInfoPtr->depth = modeInfo->vpPixelSize;
		//Copy switchInfo data
		switchInfo = inDisplayModeInfo->displayModeDepthBlockInfo->depthVPBlock[theIndex].depthSwitchInfo;
		BlockMove(switchInfo, &depthInfoPtr->switchInfo, sizeof(VDSwitchInfoRec));

		// ask the Display Manager if the mode is valid
		DMGetGDeviceByDisplayID(displayID, &displayDevice, false);
		DMCheckDisplayMode(displayDevice, switchInfo->csData, switchInfo->csMode, &theSwitchFlags, 0, &depthInfoPtr->valid);
#if __REDEFINE_VALID_MODES__
		if(theSwitchFlags & (1 << kNoSwitchConfirmBit))
		depthInfoPtr->valid = true;
		else
		depthInfoPtr->valid = false;
#endif
		++finalDepthCount;
	}
	
	SetPtrSize((Ptr) modeInfoPtr, sizeof(ModeInfo) + finalDepthCount * sizeof(DepthInfo));
	modeInfoPtr->depthCount = finalDepthCount;
}

static OSErr Scan_DisplayModes(DisplayIDType displayID, GDeviceInfoPtr info)
{
	DMDisplayModeListIteratorUPP	theUPP;
	DMListType						theDMList;
	unsigned long					index,
									numModes = 0;
	OSErr							theError;
	long							data[2];
	
	theUPP = NewDMDisplayModeListIteratorProc(ModeListIteratorProc);
	theError = DMNewDisplayModeList(displayID, 0, 0, (UInt32*) &numModes, &theDMList);
	if(theError)
	return theError;
	
	info->modeCount = 0;
	data[0] = (long) displayID;
	data[1] = (long) info;
	
	for(index = 0; index < numModes; ++index)
	DMGetIndexedDisplayModeFromList(theDMList, index, 0, theUPP, (void*) data);	
	
	DMDisposeList(theDMList);
	DisposeRoutineDescriptor(theUPP);
	
	return noErr;
}

OSErr SL_ScanGDevice(GDHandle theGDevice, GDeviceInfoPtr* theInfo)
{
	OSErr				theError;
	DisplayIDType		displayID;
		
	*theInfo = (GDeviceInfoPtr) NewPtr(sizeof(GDeviceInfo));
	if(theInfo == nil)
	return MemError();
	
	theError = DMGetDisplayIDByGDevice(theGDevice, &displayID, false);
	if(theError)
	return theError;

	//Fill GDeviceInfo structure
	(*theInfo)->gDevice = theGDevice;
	(*theInfo)->id = displayID;
	
	//Scan for GDevice's modes
	theError = Scan_DisplayModes(displayID, *theInfo);
	if(theError)
	return theError;
	
	return noErr;
}

OSErr SL_DisposeGDeviceInfo(GDeviceInfoPtr info)
{
	long				i;
	
	if(info == nil)
	return noErr;
	
	for(i = 0; i < info->modeCount; ++i)
	DisposePtr((Ptr) info->modeList[i]);
	
	DisposePtr((Ptr) info);
	
	return noErr;
}

static pascal void LookModeListIteratorProc(void* ioUserData, DMListIndexType inItemIndex, DMDisplayModeListEntryPtr inDisplayModeInfo)
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
#if __REDEFINE_VALID_MODES__
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
				}
				else if(!request->matchFound && ((modeInfo->vpBounds.right - modeInfo->vpBounds.left) < request->width) && ((modeInfo->vpBounds.bottom - modeInfo->vpBounds.top) < request->height)) {
					BlockMove(switchInfo, request->switchInfo, sizeof(VDSwitchInfoRec));
					request->width = modeInfo->vpBounds.right - modeInfo->vpBounds.left;
					request->height = modeInfo->vpBounds.bottom - modeInfo->vpBounds.top;
					request->possibleFound = true;
				}
			}
		}
	}
}

OSErr SL_LookForResolution(GDHandle device, short* width, short* height, short depth, VDSwitchInfoRec* switchInfo)
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
	theRequest.reqWidth = *width;
	theRequest.reqHeight = *height;
	theRequest.reqDepth = depth;
	theRequest.width = 32000;
	theRequest.height = 32000;
	theRequest.switchInfo = switchInfo;
	theRequest.matchFound = false;
	theRequest.possibleFound = false;
	
	theUPP = NewDMDisplayModeListIteratorProc(LookModeListIteratorProc);
	theError = DMNewDisplayModeList(theDisplayID, 0, 0, (UInt32*) &numModes, &theDMList);
	if(theError)
	return theError;
	
	for(index = 0; index < numModes; ++index) {
		DMGetIndexedDisplayModeFromList(theDMList, index, 0, theUPP, (void*) &theRequest);
#if !__SELECT_BEST_REFRESH_RATE__
		if(theRequest.matchFound)
		break;
#endif
	}
	
	DMDisposeList(theDMList);
	DisposeRoutineDescriptor(theUPP);
	
	if(!theRequest.matchFound && theRequest.possibleFound) {
		*width = theRequest.width;
		*height = theRequest.height;
	}
	
	if(!theRequest.possibleFound && !theRequest.matchFound) {
		*width = 0;
		*height = 0;
		return kError_NoMatchFound;
	}
	
	return noErr;
}

static pascal void MaxModeListIteratorProc(void* ioUserData, DMListIndexType inItemIndex, DMDisplayModeListEntryPtr inDisplayModeInfo)
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

static void DesktopBox_Draw(ControlHandle theControl)
{
	ScreenDesktopPtr		desktop = (ScreenDesktopPtr) (**theControl).contrlRfCon;
	RGBColor				oldColor;
	long					i;
	RGBColor				color = {0x8888, 0x8888, 0x8888};
	Rect					displayR,
							focusR,
							iconR;
	
	GetForeColor(&oldColor);
	
	//Draw the displays
	for(i = 0; i < desktop->deviceIDCount; ++i) {
		displayR = desktop->deviceRectList[i];
		
		//Paint display background
		RGBForeColor(&color);
		PaintRoundRect(&displayR, 8, 8);
		
		//Draw the menu bar for the main display
		if(i == desktop->mainDevice) {
			RgnHandle		saveRgn = NewRgn();
			RgnHandle		rgn = NewRgn();
			
			OpenRgn();
			FrameRoundRect(&displayR, 8, 8);
			CloseRgn(rgn);
			GetClip(saveRgn);
			SetClip(rgn);
			DisposeRgn(rgn);
			Rect menuR = displayR;
			menuR.bottom = menuR.top + 6;	//Fake the menu bar height
			ForeColor(whiteColor);
			PaintRect(&menuR);
			SetClip(saveRgn);
			DisposeRgn(rgn);
		}
		
		//Frame the display
		ForeColor(blackColor);
		FrameRoundRect(&displayR, 8, 8);
	}
	
	//Draw a focus around the selected display
	focusR = desktop->deviceRectList[desktop->currentDevice];
	InsetRect(&focusR, 4, 4);
	DrawThemeFocusRect(&focusR, true);
	
	//Draw an icon inside the selected display
	iconR.left = ((focusR.right + focusR.left) / 2) - 16;
	iconR.top = ((focusR.bottom + focusR.top) / 2) - 16;
	iconR.right = iconR.left + 32;
	iconR.bottom = iconR.top + 32;
	PlotIconID(&iconR, kAlignNone, kTransformNone, 300);
	
	RGBForeColor(&oldColor);
}

static pascal void DesktopBox_DrawProc(ControlHandle theControl, short thePart)
{
	DesktopBox_Draw(theControl);
}

static pascal ControlPartCode DesktopBox_HitProc(ControlHandle theControl, Point startPt)
{
	long					i;
	ScreenDesktopPtr		desktop = (ScreenDesktopPtr) (**theControl).contrlRfCon;
	
	for(i = 0; i < desktop->deviceIDCount; ++i)
	if(PtInRect(startPt, &desktop->deviceRectList[i])) {
		if(desktop->currentDevice != i) {
			desktop->currentDevice = i;
			DesktopBox_Draw(theControl);
		}
		return kControlButtonPart;
	}
	
	return kControlNoPart;
}

DisplayIDType SL_PickUpScreen(Str255 message)
{
	GDHandle				theGDevice;
	ScreenDesktop			desktop;
	
	desktop.deviceIDCount = 0;
	desktop.currentDevice = 0;
	SetRect(&desktop.desktopRect, 0, 0, 0, 0);
	
	theGDevice = DMGetFirstScreenDevice(true);
	while(theGDevice) {
		DMGetDisplayIDByGDevice(theGDevice, &desktop.deviceIDList[desktop.deviceIDCount], true); //On error return main screen
		desktop.deviceRectList[desktop.deviceIDCount] = (**theGDevice).gdRect;
		UnionRect(&(**theGDevice).gdRect, &desktop.desktopRect, &desktop.desktopRect);
		
		if(theGDevice == GetMainDevice())
		desktop.mainDevice = desktop.deviceIDCount;
		
		++desktop.deviceIDCount;
		theGDevice = DMGetNextScreenDevice(theGDevice, true);
	}
	
	if(desktop.deviceIDCount == 1)
	return desktop.deviceIDList[0];
	
	//We have more than one device available
	{
		DialogPtr			theDialog;
		GrafPtr				savePort;
		UniversalProcPtr	DesktopBoxDrawRoutine = NewControlUserPaneDrawProc(DesktopBox_DrawProc),
							DesktopBoxHitRoutine = NewControlUserPaneHitTestProc(DesktopBox_HitProc);
		ControlHandle		box;
		short				itemHit;
		Rect				itemRect,
							dstR;
		short				itemW,
							itemH,
							srcW,
							srcH,
							dstW,
							dstH;
		long				i;
							
		GetPort(&savePort);
		theDialog = GetNewDialog(kDisplayPickerDialogID, nil, (WindowPtr) -1);
		SetPort(theDialog);
		
		if(message == nil)
		ParamText("\p", nil, nil, nil);
		else
		ParamText(message, nil, nil, nil);
		
		GetDialogItemAsControl(theDialog, 2, &box);
		
		itemRect = (**box).contrlRect;
		itemW = itemRect.right - itemRect.left;
		itemH = itemRect.bottom - itemRect.top;
		srcW = desktop.desktopRect.right - desktop.desktopRect.left;
		srcH = desktop.desktopRect.bottom - desktop.desktopRect.top;
		dstW = itemW;
		dstH = (float) itemW * ((float) srcH / (float) srcW);
		if(dstH > itemH) {
			dstW = (float) itemH * ((float) dstW / (float) dstH);
			dstH = itemH;
		}
		dstR.left = dstR.top = 0;
		dstR.right = dstW;
		dstR.bottom = dstH;
		OffsetRect(&dstR, itemRect.left + ((itemW - dstW) / 2), itemRect.top + ((itemH - dstH) / 2));
		for(i = 0; i < desktop.deviceIDCount; ++i)
		MapRect(&desktop.deviceRectList[i], &desktop.desktopRect, &dstR);
		
		SetControlReference(box, (long) &desktop);
		SetControlData(box, 0, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr) &DesktopBoxDrawRoutine);
		SetControlData(box, 0, kControlUserPaneHitTestProcTag, sizeof(ControlUserPaneHitTestUPP), (Ptr) &DesktopBoxHitRoutine);
			
		SetDialogDefaultItem(theDialog, 1);
		FlushEvents(everyEvent, 0);
		
		do {
			ModalDialog(nil, &itemHit);
		} while(itemHit != 1);
		
		DisposeDialog(theDialog);
		SetPort(savePort);
	}
	
	return desktop.deviceIDList[desktop.currentDevice];
}
