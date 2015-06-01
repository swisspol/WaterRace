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

#define				kCursorAnimResType			'acur'
#define				kCursorAnimResID			9000
#define				kCursorFrequency			10

//STRUCTURES:

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct acurResource
{
	short			numCursors;
	short			frameCounter;
	CursHandle		cursorList[]; //Hi-word is cursor res ID
};
typedef acurResource* acurResourcePtr;
typedef acurResource** acurResourceHandle;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

//VARIABLES LOCALES:

static acurResourceHandle	cursors = nil;
static TMTask				cursorTask;
static Boolean				cursorActive;

//MACROS:

#define	Freq2Ms(f)	(1000 / (f))

//ROUTINES:

static pascal void Cursor_Callback(TMTaskPtr myTask)
{
	//Update cursor
	(**cursors).frameCounter += 1;
	if((**cursors).frameCounter > (**cursors).numCursors - 1)
	(**cursors).frameCounter = 0;
	SetCursor(*((**cursors).cursorList[(**cursors).frameCounter]));
	
	//Call back
	PrimeTime((QElemPtr) &cursorTask, Freq2Ms(kCursorFrequency));
}

OSErr SpinningCursor_Init()
{
	long			i;
	
	if(cursors != nil)
	return noErr;
	
	//Load resource
	cursors = (acurResourceHandle) GetResource(kCursorAnimResType, kCursorAnimResID);
	if(cursors == nil)
	return ResError();
	DetachResource((Handle) cursors);
	HLock((Handle) cursors);
	
	//Load cursors
	for(i = 0; i < (**cursors).numCursors; ++i) {
		(**cursors).cursorList[i] = GetCursor(HiWord((long) (**cursors).cursorList[i]));
		if((**cursors).cursorList[i] == nil)
		return ResError();
		DetachResource((Handle) (**cursors).cursorList[i]);
		HLock((Handle) (**cursors).cursorList[i]);
	}
	
	cursorActive = false;
	
	return noErr;
}

void SpinningCursor_Quit()
{
	long			i;
	
	if(cursors == nil)
	return;
	
	SpinningCursor_Stop();
	
	for(i = 0; i < (**cursors).numCursors; ++i)
	DisposeHandle((Handle) (**cursors).cursorList[i]);
	DisposeHandle((Handle) cursors);
	
	cursors = nil;
}


void SpinningCursor_Start()
{
	if(cursors == nil)
	return;
	if(cursorActive)
	return;
	
	InitCursor();
	
	//Start spinning
	(**cursors).frameCounter = 0;
	SetCursor(*((**cursors).cursorList[0]));
	cursorTask.tmAddr = NewTimerProc(Cursor_Callback);
	cursorTask.tmWakeUp = 0;
	cursorTask.tmReserved = 0;
	InsXTime((QElemPtr) &cursorTask);
	PrimeTime((QElemPtr) &cursorTask, Freq2Ms(kCursorFrequency));
	
	cursorActive = true;
}

void SpinningCursor_Stop()
{
	if(cursors == nil)
	return;
	if(!cursorActive)
	return;
	
	RmvTime((QElemPtr) &cursorTask);
	DisposeTimerUPP(cursorTask.tmAddr);
	InitCursor();
	
	cursorActive = false;
}

Boolean SpinningCursor_Active()
{
	return cursorActive;
}
