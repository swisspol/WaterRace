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


#include			<Navigation.h>

#include			"Navigation Services.h"

#include			"Infinity Structures.h"
#include			"MetaFile Structures.h"
#include			"MetaFile Utils.h"

//ROUTINES:

static pascal void NavEventProc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	//WindowPtr			window = (WindowPtr) callBackParms->eventData.event->message;
	
	switch(callBackSelector) {
		
		case kNavCBEvent:
		switch(callBackParms->eventData.eventDataParms.event->what) {
			
			case updateEvt:
			if(callBackUD != nil)
			(*((HandleUpdateEvent_Proc) callBackUD))();
			break;
			
		}
		break;
		
	}
}

void NavigationPutFile(Str255 prompt, Str255 defaultName, StandardFileReply* reply, Str63 appName, OSType fileType, OSType fileCreator, HandleUpdateEvent_Proc updateProc)
{
	OSErr				theErr = noErr;
	NavReplyRecord		theReply;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP = NewNavEventProc(NavEventProc);
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	
	GetGWorld(&savePort, &saveDevice);
	
	theErr = NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.dialogOptionFlags -= kNavDontAddTranslateItems;
	dialogOptions.dialogOptionFlags += kNavNoTypePopup;
	//dialogOptions.preferenceKey = kSavePrefKey;
	BlockMove(appName, dialogOptions.clientName, sizeof(Str63));
	BlockMove(defaultName, dialogOptions.savedFileName, sizeof(Str255));
	if(prompt != nil)
	BlockMove(prompt, dialogOptions.message, sizeof(Str255));
	
	theErr = NavPutFile(NULL, &theReply, &dialogOptions, eventUPP, fileType, fileCreator, (NavCallBackUserData) updateProc);
	if(theReply.validRecord && theErr == noErr) {
		FSSpec	finalFSSpec;	
		AEDesc 	resultDesc;	
		resultDesc.dataHandle = 0L;
		
		if((theErr = AEGetNthDesc(&(theReply.selection),1,typeFSS,NULL,&resultDesc)) == noErr) {
			BlockMove(*resultDesc.dataHandle,&finalFSSpec,sizeof(FSSpec));
			
			reply->sfGood = true;
			reply->sfReplacing = theReply.replacing;
			BlockMove(&finalFSSpec, &reply->sfFile, sizeof(FSSpec));
			reply->sfScript = theReply.keyScript;
			
			AEDisposeDesc(&resultDesc);
		}
		theErr = NavDisposeReply(&theReply);
	}
	else
	reply->sfGood = false;
	
	SetGWorld(savePort, saveDevice);
}

void NavigationGetFile(short numTypes, ConstSFTypeListPtr typeList, StandardFileReply* reply, Str63 appName, OSType appCreator, Boolean displayPreview, HandleUpdateEvent_Proc updateProc)
{
	NavReplyRecord		theReply;
	NavDialogOptions	dialogOptions;
	OSErr				theErr = noErr;
	NavTypeListHandle	openList = NULL;
	long				count = 0;
	NavEventUPP			eventUPP = NewNavEventProc(NavEventProc);
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	
	GetGWorld(&savePort, &saveDevice);
	
	theErr = NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.dialogOptionFlags -= kNavDontAddTranslateItems;
	dialogOptions.dialogOptionFlags += kNavNoTypePopup;
	if(!displayPreview)
	dialogOptions.dialogOptionFlags -= kNavAllowPreviews;
	//dialogOptions.preferenceKey = kSavePrefKey;
	BlockMove(appName, dialogOptions.clientName, sizeof(Str63));
	
	openList = (NavTypeListHandle) NewHandleClear(sizeof(NavTypeList) + numTypes * sizeof(OSType));
	(**openList).componentSignature = appCreator;
	(**openList).reserved = 0;
	(**openList).osTypeCount = numTypes;
	for(long i = 0; i < numTypes; ++i)
	(**openList).osType[i] = typeList[i];
	
	theErr = NavGetFile(nil, &theReply, &dialogOptions, eventUPP, nil, nil, (NavTypeListHandle) openList, (NavCallBackUserData) updateProc);
	
	if(theReply.validRecord && theErr == noErr) {
		FSSpec	finalFSSpec;	
		AEDesc 	resultDesc;
		FInfo	fileInfo;

		resultDesc.dataHandle = 0L;
		if((theErr = AEGetNthDesc(&(theReply.selection),1,typeFSS,NULL,&resultDesc)) == noErr) {
			BlockMoveData(*resultDesc.dataHandle,&finalFSSpec,sizeof(FSSpec));
				
			reply->sfGood = true;
			if(FSpGetFInfo(&finalFSSpec, &fileInfo))
			reply->sfGood = false;
			else
			reply->sfType = fileInfo.fdType;
			BlockMove(&finalFSSpec, &reply->sfFile, sizeof(FSSpec));
			reply->sfScript = theReply.keyScript;
			
			theErr = AEDisposeDesc(&resultDesc);
		}
	}
	else
	reply->sfGood = false;

	theErr = NavDisposeReply(&theReply);
	DisposeHandle((Handle) openList);

	SetGWorld(savePort, saveDevice);
}

static pascal Boolean CustomPreviewProc(NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	GrafPtr				savePort;
	AEDesc				resultDesc;
	FSSpec				previewFileSpec;
	FInfo				fileInfo;
	PicHandle			thePic = nil;
	Rect				destRect;

	if(AECoerceDesc((AEDesc*) callBackParms->eventData.eventDataParms.param, typeFSS, &resultDesc) == noErr) {
		BlockMoveData(*resultDesc.dataHandle, &previewFileSpec, sizeof(FSSpec));
		AEDisposeDesc(&resultDesc);
		if(FSpGetFInfo(&previewFileSpec, &fileInfo))
		return false;
		
		if(fileInfo.fdType == kMetaFileType) {
			GetPort(&savePort);
			SetPort(callBackParms->window);
			
			thePic = MetaFile_ExtractPreview(&previewFileSpec);
			if(thePic != nil) {
				if((callBackParms->previewRect.right - callBackParms->previewRect.left) >= kMetaFile_PreviewH) {
					destRect.left = callBackParms->previewRect.left + ((callBackParms->previewRect.right - callBackParms->previewRect.left) - kMetaFile_PreviewH) / 2;
					destRect.top = callBackParms->previewRect.top + ((callBackParms->previewRect.bottom - callBackParms->previewRect.top) - kMetaFile_PreviewV) / 2;
					
					destRect.right = destRect.left + kMetaFile_PreviewH;
					destRect.bottom = destRect.top + kMetaFile_PreviewV;
				}
				
				BackColor(whiteColor);
				ForeColor(blackColor);
				DrawPicture(thePic, &destRect);
				DisposeHandle((Handle) thePic);
				
				InsetRect(&destRect, -1, -1);
				FrameRect(&destRect);
			}
			
			SetPort(savePort);
			return true;
		}
	}
	
	return false;
}

void NavigationGetMetaFile(short numTypes, ConstSFTypeListPtr typeList, StandardFileReply* reply, Str63 appName, OSType appCreator, HandleUpdateEvent_Proc updateProc)
{
	NavReplyRecord		theReply;
	NavDialogOptions	dialogOptions;
	OSErr				theErr = noErr;
	NavTypeListHandle	openList = NULL;
	long				count = 0;
	NavEventUPP			eventUPP = NewNavEventProc(NavEventProc);
	NavPreviewUPP		previewUPP = NewNavPreviewProc(CustomPreviewProc);
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	
	GetGWorld(&savePort, &saveDevice);
	
	theErr = NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.dialogOptionFlags -= kNavDontAddTranslateItems;
	dialogOptions.dialogOptionFlags += kNavNoTypePopup;
	//dialogOptions.preferenceKey = kSavePrefKey;
	BlockMove(appName, dialogOptions.clientName, sizeof(Str63));
	
	openList = (NavTypeListHandle) NewHandleClear(sizeof(NavTypeList) + numTypes * sizeof(OSType));
	HLock((Handle) openList);
	(**openList).componentSignature = appCreator;
	(**openList).reserved = 0;
	(**openList).osTypeCount = numTypes;
	for(long i = 0; i < numTypes; ++i)
	(**openList).osType[i] = typeList[i];
	
	theErr = NavGetFile(nil, &theReply, &dialogOptions, eventUPP, previewUPP, nil, (NavTypeListHandle) openList, (NavCallBackUserData) updateProc);
	
	if(theReply.validRecord && theErr == noErr) {
		FSSpec	finalFSSpec;	
		AEDesc 	resultDesc;
		FInfo	fileInfo;

		resultDesc.dataHandle = 0L;
		if((theErr = AEGetNthDesc(&(theReply.selection),1,typeFSS,NULL,&resultDesc)) == noErr) {
			BlockMoveData(*resultDesc.dataHandle,&finalFSSpec,sizeof(FSSpec));
				
			reply->sfGood = true;
			if(FSpGetFInfo(&finalFSSpec, &fileInfo))
			reply->sfGood = false;
			else
			reply->sfType = fileInfo.fdType;
			BlockMove(&finalFSSpec, &reply->sfFile, sizeof(FSSpec));
			reply->sfScript = theReply.keyScript;
			
			theErr = AEDisposeDesc(&resultDesc);
		}
	}
	else
	reply->sfGood = false;

	theErr = NavDisposeReply(&theReply);
	DisposeHandle((Handle) openList);

	SetGWorld(savePort, saveDevice);
}
