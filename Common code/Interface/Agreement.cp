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


#include				"Dialog Utils.h"

//CONSTANTES:

#define				kAgreementDialogID			9999
#define				kMove						10
#define				kScroll_Delay				2

enum {kItem_Agree = 1, kItem_Disagree, kItem_Scroll, kItem_Text, kItem_Wait, kItemNum};

//ROUTINES:

Boolean Display_Agreement(short textID, long waitingTime, Str255 okButton, Str255 cancelButton)
{
	DialogPtr			theDialog;
	TEHandle			theTEHandle;
	GrafPtr				savePort;
	Handle				textHandle;
	StScrpHandle		stylHandle;
	short				curPos = 0,
						itemType,
						scrollHit;
	long				textLength = 1000;
	ControlHandle		scrollBar,
						control,
						okItem,
						cancelItem;
	Rect				realRect,
						frameRect,
						displayRect;
	Handle				textItem,
						waitItem;
	unsigned long		startTime,
						lastTime = -1,
						wait;
	EventRecord			theEvent;
	Boolean				loop = true,
						agree = false;
	Str31				tempString;
	short				theKey;
	
	//GetDialog
	GetPort(&savePort);
	theDialog = GetNewDialog(kAgreementDialogID, nil, (WindowPtr) -1);
	SetPort(theDialog);
	GetDialogItem(theDialog, kItem_Agree, &itemType, (Handle*) &okItem, &realRect);
	GetDialogItem(theDialog, kItem_Disagree, &itemType, (Handle*) &cancelItem, &realRect);
	
	if(okButton != nil)
	SetControlTitle(okItem, okButton);
	else
	SetControlTitle(okItem, "\pAgree");
	
	if(cancelButton != nil)
	SetControlTitle(cancelItem, cancelButton);
	else
	SetControlTitle(cancelItem, "\pDisagree");
	
	//Get waiting box and disable buttons
	if(waitingTime) {
		GetDialogItem(theDialog, kItem_Wait, &itemType, &waitItem, &realRect);
		HiliteControl(okItem, 255);
		startTime = TickCount();
	}
	
	//Get scroll bar
	GetDialogItem(theDialog, kItem_Scroll, &itemType, (Handle*) &scrollBar, &realRect);
	
	//Get text box position
	GetDialogItem(theDialog, kItem_Text, &itemType, &textItem, &frameRect);
	displayRect.top = frameRect.top + 1;
	displayRect.left = frameRect.left + 1;
	displayRect.bottom = frameRect.bottom - 1;
	displayRect.right = frameRect.right - 1;
	frameRect.right += 1;
	SetRect(&realRect, 10, 0, displayRect.right - displayRect.left - 10, 10000);
	
	//Create text box
	theTEHandle = TEStyleNew(&realRect, &displayRect);
	if(theTEHandle == nil)
	return false;
	textHandle = Get1Resource('TEXT', textID);
	if(textHandle == nil)
	return false;
	HLock(textHandle);
	stylHandle = (StScrpHandle) Get1Resource('styl', textID);
	if(stylHandle == nil)
	return false;
	GetResInfo((Handle) stylHandle, &itemType, (unsigned long*) &textLength, tempString);
	StringToNum(tempString, &textLength);
	HLock((Handle) stylHandle);
	TEStyleInsert(*textHandle, GetHandleSize(textHandle), stylHandle, theTEHandle);
	
	//Update
	SetControlMaximum(scrollBar, textLength);
	ShowWindow(theDialog);
	if(waitingTime == 0)
	SetDialogDefaultItem(theDialog, 1);
	
	while(loop) {
		//Update waiting time
		if(waitingTime) {
			if((TickCount() - startTime) / 60 > waitingTime) {
				HiliteControl(okItem, 0);
				waitingTime = 0;
				SetDialogItemText(waitItem, "\p");
			}
			else if((TickCount() - startTime) / 60 != lastTime) {
				Str31			time,
								timeString = "\pSeconds to wait: ";
								
				NumToString(waitingTime - (TickCount() - startTime) / 60, time);
				BlockMove(&time[1], &timeString[timeString[0] + 1], time[0]);
				timeString[0] += time[0];
				SetDialogItemText(waitItem, timeString);
				lastTime = (TickCount() - startTime) / 60;
			}
		}
		
		//Handle dialog events
		if(!WaitNextEvent(everyEvent & ~highLevelEventMask, &theEvent, (long) 20, 0) && !IsDialogEvent(&theEvent))
		continue;
		
		switch(theEvent.what) {
		
			case mouseDown:
			{
				short					whereClick;
				WindowPtr				whichWin;
				
				whereClick = FindWindow(theEvent.where, &whichWin);
				if(whereClick == inContent) {
					GlobalToLocal(&theEvent.where);
					if(!FindControl(theEvent.where, whichWin, &control))
					break;
					if(control == scrollBar) {
						scrollHit = TestControl(scrollBar, theEvent.where);
						switch(scrollHit) {
							
							case kControlIndicatorPart:
							TrackControl(scrollBar, theEvent.where, nil);
							TEScroll(0, curPos - GetControlValue(scrollBar), theTEHandle);
							curPos = GetControlValue(scrollBar);
							break;
							
							case kControlUpButtonPart:
							HiliteControl(scrollBar, kControlUpButtonPart);
							while(Button()) {
								if(curPos > kMove) {
									curPos -= kMove;
									TEScroll(0, kMove, theTEHandle);
									FrameRect(&frameRect);
									SetControlValue(scrollBar, curPos);
								}
								else if(curPos > 0) {
									curPos -= 1;
									TEScroll(0, 1, theTEHandle);
									FrameRect(&frameRect);
									SetControlValue(scrollBar, curPos);
								}
								Delay(kScroll_Delay, &wait);
							}
							HiliteControl(scrollBar, 0);
							break;
							
							case kControlDownButtonPart:
							HiliteControl(scrollBar, kControlDownButtonPart);
							while(Button()) {
								if(curPos < textLength - kMove) {
									curPos += kMove;
									TEScroll(0, -kMove, theTEHandle);
									FrameRect(&frameRect);
									SetControlValue(scrollBar, curPos);
								}
								else if(curPos < textLength) {
									curPos += 1;
									TEScroll(0, -1, theTEHandle);
									FrameRect(&frameRect);
									SetControlValue(scrollBar, curPos);
								}
								Delay(kScroll_Delay, &wait);
							}
							HiliteControl(scrollBar, 0);
							break;
							
							case kControlPageUpPart:
							TEScroll(0, curPos, theTEHandle);
							FrameRect(&frameRect);
							curPos = 0;
							SetControlValue(scrollBar, curPos);
							break;
							
							case kControlPageDownPart:
							TEScroll(0, curPos - textLength, theTEHandle);
							FrameRect(&frameRect);
							curPos = textLength;
							SetControlValue(scrollBar, curPos);
							break;
							
						}
					}
					else {
						if(TrackControl(control, theEvent.where, nil)) {
							loop = false;
							if(control == okItem)
							agree = true;
						}
					}
				}
				else
				SysBeep(0);
			}
			break;
			
			case updateEvt:
			if((WindowPtr) theEvent.message != theDialog)
			break;
			BeginUpdate(theDialog);
			DrawDialog(theDialog);
			TEUpdate(&displayRect, theTEHandle);
			FrameRect(&frameRect);
			EndUpdate(theDialog);
			break;
			
			case keyDown:
			case autoKey:
			if(waitingTime)
			break;
			theKey = (theEvent.message & keyCodeMask) >> 8;
			
			if((theKey == 0x24) || (theKey == 0x4C) || (theKey == 0x34)) {
				Press_Button(theDialog, 1);
				loop = false;
				agree = true;
			} else if(theKey == 0x35) {
				Press_Button(theDialog, 2);
				loop = false;
			}
			break;
		
		}
	}
	
	TEDispose(theTEHandle);
	HUnlock(textHandle);
	ReleaseResource(textHandle);
	HUnlock((Handle) stylHandle);
	ReleaseResource((Handle) stylHandle);
	DisposeDialog(theDialog);
	SetPort(savePort);
	
	return agree;
}
