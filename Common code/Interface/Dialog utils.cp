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


#include				"Numeric Utils.h"

//CONSTANTES:

#define					kInsertChar					'^'
#define					kEnterDelay					10

//ROUTINES:

void InsertDialogItemText(DialogPtr dialog, short itemNum, Str255 addText)
{
	Handle				item;
	Str255				text;
	long				i,
						insert = -1;
	short				itemType;
	Rect				itemRect;
	
	GetDialogItem(dialog, itemNum, &itemType, &item, &itemRect);
	
	GetDialogItemText(item, text);
	for(i = 1; i <= text[0]; ++i)
	if(text[i] == kInsertChar)
	insert = i;
	if(insert == -1)
	return;
	
	BlockMove(&text[insert + 1], &text[insert + addText[0]], text[0] - insert);
	BlockMove(&addText[1], &text[insert], addText[0]);
	text[0] += addText[0] - 1;
	
	SetDialogItemText(item, text);
}

void SetDialogItemFloat(Handle itemHandle, float num)
{
	Str31			text;
	
	FloatToString(num, text);
	SetDialogItemText(itemHandle, text);
}

void GetDialogItemFloat(Handle itemHandle, float* num)
{
	Str31			text;
	
	GetDialogItemText(itemHandle, text);
	StringToFloat(text, num);
}

void SetDialogItemNum(Handle itemHandle, long num)
{
	Str31			text;
	
	text[0] = Integer2Text(&text[1], num);
	SetDialogItemText(itemHandle, text);
}

void GetDialogItemNum(Handle itemHandle, long* num)
{
	Str31			text;
	
	GetDialogItemText(itemHandle, text);
	*num = Text2Integer(&text[1], text[0]);
}

void SetDialogItemTime(Handle itemHandle, long time)
{
	Str31			text;
	
	TimeToString(time, text);
	SetDialogItemText(itemHandle, text);
}

void GetDialogItemTime(Handle itemHandle, long* time)
{
	Str31			text;
	
	GetDialogItemText(itemHandle, text);
	StringToTime(text, time);
}

void Press_Button(DialogPtr theDialog, short itemNum)
{
	ControlHandle		buttonHandle;
	unsigned long		ticks;
	
	GetDialogItemAsControl(theDialog, itemNum, &buttonHandle);
	HiliteControl(buttonHandle, kControlButtonPart);
	Delay(kEnterDelay, &ticks);
	HiliteControl(buttonHandle, kControlNoPart);
}

void SetDialogItemID(Handle itemHandle, OSType id)
{
	Str31			text;
	
	IDToString(id, text);
	SetDialogItemText(itemHandle, text);
}

void GetDialogItemID(Handle itemHandle, OSType* id)
{
	Str31			text;
	
	GetDialogItemText(itemHandle, text);
	StringToID(text, id);
}

pascal OSStatus GetListBoxListHandle(ControlHandle control, ListHandle* list)
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( list == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlListBoxListHandleTag, sizeof( ListHandle ),
			 (Ptr)list, &actualSize );
		
	return err;
}

Point GetWindowPosition(WindowPtr window)
{
	GrafPtr				savePort;
	Point				thePoint = {0,0};
	
	GetPort(&savePort);
	SetPort(window);
	LocalToGlobal(&thePoint);
	SetPort(savePort);
	
	return thePoint;
}

Boolean MakeSure_WindowIsOnScreen(WindowPtr window)
{
	RgnHandle		desktopRgn = LMGetGrayRgn();
	Rect			windowRect, //(**(((WindowPeek) window)->contRgn)).rgnBBox, //Not valid is window hidden
					desktopRect;
	Point			windowPos = GetWindowPosition(window);
	
	//Extract window rect
	windowRect.left = windowPos.h;
	windowRect.right = windowPos.h + window->portRect.right - window->portRect.left;
	windowRect.top = windowPos.v;
	windowRect.bottom = windowPos.v + window->portRect.bottom - window->portRect.top;
	
	//Check if window is in desktop area
	if(RectInRgn(&windowRect, desktopRgn))
	return true;
	
	//Move window inside main device's desktop
	desktopRect = (**GetMainDevice()).gdRect;
	windowPos.h = windowRect.left;
	windowPos.v = windowRect.top;
	
	if(windowRect.left > desktopRect.right)
	windowPos.h = desktopRect.right - (windowRect.right - windowRect.left);
	else if(windowRect.right < desktopRect.left)
	windowPos.h = desktopRect.left;
	
	if(windowRect.bottom > desktopRect.bottom)
	windowPos.v = desktopRect.bottom - (windowRect.bottom - windowRect.top);
	else if(windowRect.top < desktopRect.top)
	windowPos.v = desktopRect.top;
	
	MoveWindow(window, windowPos.h, windowPos.v, false);
	
	return false;
}

void MenuSafeString(Str255 in, Str255 out)
{
	long				i;
	
	out[0] = 0;
	for(i = 1; i <= in[0]; ++i)
	if((in[i] != ';') && (in[i] != '\r') && (in[i] != '^') && (in[i] != '!') && (in[i] != '<')
		 && (in[i] != '/') && (in[i] != '(') && (in[i] != '-')) {
		++out[0];
		out[out[0]] = in[i];
	}
}

//AppendDialogItemList Glue

#define RESULT_OFFSET(type) ((sizeof(type) == 1) ? 3 : ((sizeof(type) == 2) ? 1 : 0))

pascal OSErr AppendDialogItemList(DialogPtr dialog, short ditlID, DITLMethod method)
{
	long	private_result;
	
	private_result = CallUniversalProc((UniversalProcPtr) GetToolTrapAddress(0xAA68),
		kD0DispatchedPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kTwoByteCode)
		 | DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(dialog)))
		 | DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ditlID)))
		 | DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(method))),
		0x0412,
		dialog,
		ditlID,
		method );
		
	return *(((OSErr*)&private_result) + RESULT_OFFSET(OSErr));
}
