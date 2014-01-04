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


//CONSTANTES:

#define			kError_ActionDataResType		'ErAc'
#define			kError_CodeDataResType			'ErCd'

#define			kError_ActionDataResID			150
#define			kError_CodeDataResID_Local		150

#define			kError_CodeDataResID_GlobalSystem		128
#define			kError_CodeDataResID_GlobalInfinity		129

//ROUTINES:

static Boolean Error_ParseErrorActionData(Ptr dataPtr, short actionID, OSErr errorID, Str255 actionText, Str255 solutionText)
{
	long					numEntries = *((short*) dataPtr),
							numSolutions;
	Ptr						thePtr = dataPtr + sizeof(short);
	
	//Parse error data
	while(numEntries--) {
		//Check action ID
		if(*((short*) thePtr) == actionID) {
			//Find action text
			thePtr += sizeof(short);
			//Copy text
			if(actionText[0] == 0)
			BlockMove(thePtr, actionText, sizeof(Str255));
			thePtr += *((unsigned char*) thePtr) + 1;
			//Look for solution if any
			numSolutions = *((short*) thePtr);
			thePtr += sizeof(short);
			while(numSolutions--) {
				//Check error ID
				if(*((OSErr*) thePtr) == errorID) {
					//Find solution text
					thePtr += sizeof(OSErr);
					//Copy text
					if(solutionText[0] == 0)
					BlockMove(thePtr, solutionText, sizeof(Str255));
					
					return true;
				}
				//Skip text
				thePtr += sizeof(OSErr);
				thePtr += *((unsigned char*) thePtr) + 1;
			}
			
			return true;
		}
		
		//Skip action text
		thePtr += sizeof(short);
		thePtr += *((unsigned char*) thePtr) + 1;
		
		//Skip solution text
		numSolutions = *((short*) thePtr);
		thePtr += sizeof(short);
		while(numSolutions--) {
			//Skip error ID
			thePtr += sizeof(OSErr);
			//Skip text
			thePtr += *((unsigned char*) thePtr) + 1;
		}
	}
	
	return false;
}

static Boolean Get_ActionText(short actionID, OSErr errorID, Str255 actionText, Str255 solutionText)
{
	Handle					data;
	Boolean					found;
	
	//Search local
	data = GetResource(kError_ActionDataResType, kError_ActionDataResID);
	if(ResError())
	return false;
	DetachResource(data);
	HLock(data);
	found = Error_ParseErrorActionData(*data, actionID, errorID, actionText, solutionText);
	DisposeHandle(data);
	
	return found;
}

static Boolean Error_ParseErrorCodeData(Ptr dataPtr, OSErr errorID, Str255 errorText, Str255 solutionText)
{
	long					numEntries = *((short*) dataPtr);
	Ptr						thePtr = dataPtr + sizeof(short);
	
	//Parse error data
	while(numEntries--) {
		//Check error ID
		if(*((OSErr*) thePtr) == errorID) {
			//Find error text
			thePtr += sizeof(short);
			//Copy text
			if(errorText[0] == 0)
			BlockMove(thePtr, errorText, sizeof(Str255));
			thePtr += *((unsigned char*) thePtr) + 1;
			//Copy text
			if(solutionText[0] == 0)
			BlockMove(thePtr, solutionText, sizeof(Str255));
			
			return true;
		}
		
		//Skip error text
		thePtr += sizeof(short);
		thePtr += *((unsigned char*) thePtr) + 1;
		
		//Skip solution text
		thePtr += *((unsigned char*) thePtr) + 1;
	}
	
	return false;
}

static Boolean Get_ErrorText(OSErr errorID, Str255 errorText, Str255 solutionText)
{
	Handle					data;
	Boolean					found;
	
	//Search local
	data = GetResource(kError_CodeDataResType, kError_CodeDataResID_Local);
	if(ResError())
	return false;
	DetachResource(data);
	HLock(data);
	found = Error_ParseErrorCodeData(*data, errorID, errorText, solutionText);
	DisposeHandle(data);
	if(found)
	return true;
	
	//Search global - SYSTEM
	data = GetResource(kError_CodeDataResType, kError_CodeDataResID_GlobalSystem);
	if(ResError())
	return false;
	DetachResource(data);
	HLock(data);
	found = Error_ParseErrorCodeData(*data, errorID, errorText, solutionText);
	DisposeHandle(data);
	
	//Search global - INFINITY
	data = GetResource(kError_CodeDataResType, kError_CodeDataResID_GlobalInfinity);
	if(ResError())
	return false;
	DetachResource(data);
	HLock(data);
	found = Error_ParseErrorCodeData(*data, errorID, errorText, solutionText);
	DisposeHandle(data);
	
	return found;
}

void Error_Display(short actionID, OSErr errorID, ModalFilterProcPtr eventFilterProc, Boolean fatal)
{
	AlertStdAlertParamRec	params;
	short					outItemHit;
	Str255					actionText,
							errorText,
							solutionText;
	Str31					errorCode;
	
	//Setup dialog params
	params.movable = true;
	if(eventFilterProc != nil)
	params.filterProc = NewModalFilterProc(eventFilterProc);
	else
	params.filterProc = NewModalFilterProc(eventFilterProc);
	
	//Setp buttons
	params.helpButton = false;
	if(fatal)
	params.defaultText = "\pQuit";
	else
	params.defaultText = (StringPtr) kAlertDefaultOKText;
	params.cancelText = nil;
	params.otherText = nil;
	params.defaultButton = kAlertStdAlertOKButton;
	params.cancelButton = 0;
	
	//Setup window position
	params.position = kWindowDefaultPosition;
	
	//Find texts
	actionText[0] = 0;
	errorText[0] = 0;
	solutionText[0] = 0;
	
	Get_ActionText(actionID, errorID, actionText, solutionText);
	NumToString(errorID, errorCode);
	Get_ErrorText(errorID, errorText, solutionText);
	
	if(actionText[0] == 0)
	BlockMove("\pAn error occured.", actionText, sizeof(Str255));
	if(errorText[0] == 0)
	BlockMove("\pUnknown error.", errorText, sizeof(Str255));
	
	//Setup text #1
	++actionText[0];
	actionText[actionText[0]] = '\r';
	++actionText[0];
	actionText[actionText[0]] = '(';
	BlockMove(&errorCode[1], &actionText[actionText[0] + 1], errorCode[0]);
	actionText[0] += errorCode[0];
	++actionText[0];
	actionText[actionText[0]] = ':';
	++actionText[0];
	actionText[actionText[0]] = ' ';
	BlockMove(&errorText[1], &actionText[actionText[0] + 1], errorText[0]);
	actionText[0] += errorText[0];
	++actionText[0];
	actionText[actionText[0]] = ')';
	
	//Display error dialog
	InitCursor();
	if(solutionText[0])
	StandardAlert(kAlertStopAlert, actionText, solutionText, &params, &outItemHit);
	else
	StandardAlert(kAlertStopAlert, actionText, nil, &params, &outItemHit);
	
	if(fatal)
	ExitToShell();
}
