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


#include			"NewList.h"

//CONSTANTES:

#define				kScrollBarWidth					16

#define				kColorLightGrey					57000
#define				kColorDarkGrey					50000
#define				kColorVeryDarkGrey				40000
#define				kColorVeryVeryDarkGrey			30000

#define				kScrollingDelay					5

//PRIVATE VARIABLES:

static RGBColor		veryVeryDarkGrey = {kColorVeryVeryDarkGrey, kColorVeryVeryDarkGrey, kColorVeryVeryDarkGrey},
					veryDarkGrey = {kColorVeryDarkGrey, kColorVeryDarkGrey, kColorVeryDarkGrey},
					darkGrey = {kColorDarkGrey, kColorDarkGrey, kColorDarkGrey},
					lightGrey = {kColorLightGrey, kColorLightGrey, kColorLightGrey};

//PRIVATE ROUTINES:

static void Row_Draw(NewListPtr listPtr, Row rowNum)
{
	Rect			rowRect;
	
	if((rowNum < listPtr->firstRow) || (rowNum >= listPtr->firstRow + listPtr->rowVisible))
	return;
	
	rowRect.left = listPtr->viewRect.left;
	rowRect.top = listPtr->viewRect.top + (rowNum - listPtr->firstRow) * kNewList_RowHeight;
	rowRect.right = listPtr->viewRect.right - 1;
	rowRect.bottom = rowRect.top + kNewList_RowHeight - 1;
	
	if(rowNum != listPtr->selectedRow) {
		if(rowNum - listPtr->firstRow > 0)
		ForeColor(whiteColor);
		else
		RGBForeColor(&darkGrey);
		MoveTo(rowRect.left, rowRect.top);
		LineTo(rowRect.right, rowRect.top);
	
		RGBForeColor(&darkGrey);
		MoveTo(rowRect.left, rowRect.bottom);
		LineTo(rowRect.right, rowRect.bottom);
		
		RGBForeColor(&lightGrey);
		++rowRect.top;
	}
	else {
		if(rowNum - listPtr->firstRow > 0) {
			RGBForeColor(&veryDarkGrey);
			MoveTo(rowRect.left, rowRect.top);
			LineTo(rowRect.right, rowRect.top);
		}
		
		RGBForeColor(&darkGrey);
		++rowRect.top;
	}
	++rowRect.right;
	PaintRect(&rowRect);
	
	ForeColor(blackColor);
	MoveTo(rowRect.left + 12, rowRect.top + 11);
	DrawString(listPtr->rowList[rowNum]);
}

inline void Row_Copy(NewListPtr listPtr, Row source, Row dest)
{
	BlockMove(listPtr->rowList[source], listPtr->rowList[dest], listPtr->rowList[source][0] + 1);
}

inline void Row_Clear(NewListPtr listPtr, Row source)
{
	listPtr->rowList[source][0] = 0;
}

static pascal void LiveActionProc(ControlHandle control, SInt16 part)
{
	SInt16					startValue;
	NewListHandle			list;
	Boolean					update = false;
	
	startValue = GetControlValue(control);
	switch(part) {
		
		case kControlUpButtonPart:
		if(startValue > GetControlMinimum(control)) {
			SetControlValue(control, startValue - 1);
			update = true;
		}
		break;
		
		case kControlDownButtonPart:
		if(startValue < GetControlMaximum(control)) {
			SetControlValue(control, startValue + 1);
			update = true;
		}
		break;
		
		case kControlPageUpPart:
		SetControlValue(control,GetControlMinimum(control));
		update = true;
		break;
		
		case kControlPageDownPart:
		SetControlValue(control,GetControlMaximum(control));
		update = true;
		break;
		
		case kControlIndicatorPart:
		update = true;
		break;
		
	}
	
	if(update) {
		list = (NewListHandle) GetControlReference(control);
		(**list).firstRow = GetControlValue(control);
		NewList_Update(list);
	}
}

//PUBLIC ROUTINES:

NewListHandle NewList_New(Rect* viewRect, short maxNumRows, WindowPtr window, Boolean drawIt, Boolean scrollBar)
{
	NewListHandle			list;
	NewListPtr				listPtr;
	Rect					scrollRect;
	ControlActionUPP		LiveActionRoutine = NewControlActionProc(LiveActionProc);
	
	list = (NewListHandle) NewHandle(sizeof(NewList_Definition) + maxNumRows * sizeof(Str31));
	if(list == nil)
	return nil;
	HLock((Handle) list);
	listPtr = *list;
	
	listPtr->rowVisible = (viewRect->bottom - viewRect->top) / kNewList_RowHeight;
	listPtr->window = window;
	listPtr->viewRect = *viewRect;
	listPtr->viewRect.bottom = listPtr->viewRect.top + kNewList_RowHeight * listPtr->rowVisible;
	listPtr->active = drawIt;
	listPtr->maxRowCount = maxNumRows;
	listPtr->rowCount = 0;
	listPtr->selectedRow = kNewList_NoRowSelected;
	listPtr->firstRow = 0;
	listPtr->lastClickTime = 0;
	
	if(scrollBar) {
		SetRect(&scrollRect, listPtr->viewRect.right, listPtr->viewRect.top - 1, listPtr->viewRect.right + kScrollBarWidth, listPtr->viewRect.bottom + 1);
		listPtr->vScroll = NewControl(window, &scrollRect, "\p", drawIt, 0, 0, 0, kControlScrollBarLiveProc, nil);
		SetControlAction(listPtr->vScroll, LiveActionRoutine);
		SetControlReference(listPtr->vScroll, (long) list);
	}
	else
	listPtr->vScroll = nil;
	
	//NewList_DoDraw(drawIt, list);
	NewList_Update(list);
	
	return list;
}

void NewList_Dispose(NewListHandle list)
{
	NewListPtr		listPtr = *list;
	
	if(list != nil) {
		if(listPtr->vScroll != nil)
		DisposeControl(listPtr->vScroll);
		DisposeHandle((Handle) list);
	}
}

Row NewList_Delete(short count, Row rowNum, NewListHandle list)
{
	NewListPtr		listPtr = *list;
	long			i,
					j;
	
	if(count == 0) {
		//Delete all rows
		listPtr->rowCount = 0;
		listPtr->selectedRow = kNewList_NoRowSelected;
		listPtr->firstRow = 0;
	}
	else {
		for(i = 0; i < count; ++i) {
			for(j = rowNum + 1; j < listPtr->rowCount; ++j)
			Row_Copy(listPtr, j, j - 1);
			--listPtr->rowCount;
		}
		
		if(listPtr->selectedRow >= rowNum + count)
		listPtr->selectedRow -= count;
	}
	
	if(listPtr->selectedRow > listPtr->rowCount - 1)
	listPtr->selectedRow = listPtr->rowCount - 1;
	
	if(listPtr->vScroll != nil) {
		if(listPtr->rowCount - listPtr->rowVisible > 0)
		SetControlMaximum(listPtr->vScroll, listPtr->rowCount - listPtr->rowVisible);
		else
		SetControlMaximum(listPtr->vScroll, 0);
	}
	if((listPtr->rowCount >= listPtr->rowVisible) && (listPtr->firstRow + listPtr->rowVisible > listPtr->rowCount)) {
		listPtr->firstRow = listPtr->rowCount - listPtr->rowVisible;
		SetControlValue(listPtr->vScroll, listPtr->firstRow);
	}
	
	if(listPtr->rowCount <= listPtr->rowVisible)
	listPtr->firstRow = 0;
	else if(listPtr->firstRow + listPtr->rowVisible > listPtr->rowCount)
	listPtr->firstRow = listPtr->rowCount - listPtr->rowVisible;
	
	NewList_Update(list);
	
	return listPtr->selectedRow;
}

Row NewList_Add(short count, Row rowNum, NewListHandle list)
{
	NewListPtr		listPtr = *list;
	long			i,
					j;
	
	if(rowNum > listPtr->rowCount - 1) {
		rowNum = listPtr->rowCount;
		for(i = 0; i < count; ++i)
		Row_Clear(listPtr, rowNum + i);
	}
	else {
		for(i = listPtr->rowCount + count - 1, j = 0; i >= 0; --i, ++j) {
			Row_Copy(listPtr, rowNum + j, i);
			Row_Clear(listPtr, rowNum + j);
		}
	}
	listPtr->rowCount += count;
	
	if(listPtr->vScroll != nil) {
		if(listPtr->rowCount - listPtr->rowVisible > 0)
		SetControlMaximum(listPtr->vScroll, listPtr->rowCount - listPtr->rowVisible);
		else
		SetControlMaximum(listPtr->vScroll, 0);
	}
	
	listPtr->selectedRow = rowNum;
	if(NewList_DisplayRow(rowNum, list))
	NewList_Update(list);
	
	return rowNum;
}

void NewList_Draw(Row rowNum, NewListHandle list)
{
	GrafPtr			savePort;
	NewListPtr		listPtr = *list;
	RGBColor		oldColor;
	RgnHandle		oldClipRgn;
	
	if(!listPtr->active)
	return;
	
	GetPort(&savePort);
	SetPort(listPtr->window);
	GetForeColor(&oldColor);
	
	oldClipRgn = NewRgn();
	GetClip(oldClipRgn);
	ClipRect(&listPtr->viewRect);
	
	Row_Draw(listPtr, rowNum);
	
	RGBForeColor(&oldColor);
	SetClip(oldClipRgn);
	SetPort(savePort);
	DisposeRgn(oldClipRgn);
}

void NewList_Update(NewListHandle list)
{
	GrafPtr			savePort;
	NewListPtr		listPtr = *list;
	long			i;
	RGBColor		oldColor;
	RgnHandle		oldClipRgn;
	
	if(!listPtr->active)
	return;
	
	GetPort(&savePort);
	SetPort(listPtr->window);
	GetForeColor(&oldColor);
	
	oldClipRgn = NewRgn();
	GetClip(oldClipRgn);
	ClipRect(&listPtr->viewRect);
	
	RGBForeColor(&lightGrey);
	PaintRect(&listPtr->viewRect);
	
	for(i = 0; i < listPtr->rowVisible; ++i)
	if(listPtr->firstRow + i < listPtr->rowCount)
	Row_Draw(listPtr, listPtr->firstRow + i);
	
	if((listPtr->rowCount > 0) && (listPtr->rowCount < listPtr->rowVisible)) {
		i = listPtr->viewRect.top + listPtr->rowCount * kNewList_RowHeight;
		ForeColor(whiteColor);
		MoveTo(listPtr->viewRect.left, i);
		LineTo(listPtr->viewRect.right - 1, i);
	}
	
	if(listPtr->vScroll != nil)
	Draw1Control(listPtr->vScroll);
	
	RGBForeColor(&oldColor);
	SetClip(oldClipRgn);
	SetPort(savePort);
	DisposeRgn(oldClipRgn);
}

void NewList_Select(Row rowNum, NewListHandle list)
{
	GrafPtr			savePort;
	NewListPtr		listPtr = *list;
	RGBColor		oldColor;
	Row				oldSelectedRow;
	RgnHandle		oldClipRgn;
	
	oldSelectedRow = listPtr->selectedRow;
	listPtr->selectedRow = rowNum;
	
	if(!listPtr->active)
	return;
	
	if(NewList_DisplayRow(rowNum, list)) {
		GetPort(&savePort);
		SetPort(listPtr->window);
		GetForeColor(&oldColor);
	
		oldClipRgn = NewRgn();
		GetClip(oldClipRgn);
		ClipRect(&listPtr->viewRect);
		
		if(oldSelectedRow != kNewList_NoRowSelected)
		Row_Draw(listPtr, oldSelectedRow);
		Row_Draw(listPtr, listPtr->selectedRow);
		
		RGBForeColor(&oldColor);
		SetClip(oldClipRgn);
		SetPort(savePort);
		DisposeRgn(oldClipRgn);
	}
}

Row NewList_GetSelect(NewListHandle list)
{
	return (**list).selectedRow;
}

void NewList_SetDrawingMode(Boolean drawIt, NewListHandle list)
{
	NewListPtr		listPtr = *list;
	
	listPtr->active = drawIt;
	
	if(listPtr->vScroll != nil) {
		if(drawIt)
		ShowControl(listPtr->vScroll);
		else
		HideControl(listPtr->vScroll);
	}
}

void NewList_SetRow(void* dataPtr, short dataSize, Row rowNum, NewListHandle list)
{
	NewListPtr		listPtr = *list;
	
	listPtr->rowList[rowNum][0] = dataSize;
	BlockMove(dataPtr, &listPtr->rowList[rowNum][1], dataSize);
	
	NewList_Draw(rowNum, list);
}

Boolean NewList_Click(Point localPoint, NewListHandle list)
{
	NewListPtr		listPtr = *list;
	Row				clickRow,
					oldRow;
	long			tick;
	unsigned long	delay;
	
	if(listPtr->rowCount == 0)
	return false;
	
	if(PtInRect(localPoint, &listPtr->viewRect)) {
		oldRow = listPtr->selectedRow;
		while(Button()) {
			if(PtInRect(localPoint, &listPtr->viewRect)) {
				clickRow = (localPoint.v - listPtr->viewRect.top) / kNewList_RowHeight + listPtr->firstRow;
				if(clickRow > listPtr->rowCount - 1)
				clickRow = listPtr->rowCount - 1;
				if(clickRow != listPtr->selectedRow)
				NewList_Select(clickRow, list);
			}
			else if((localPoint.h >= listPtr->viewRect.left) && (localPoint.h < listPtr->viewRect.right)) {
				if((localPoint.v < listPtr->viewRect.top) && (listPtr->firstRow > 0)) {
					--listPtr->firstRow;
					listPtr->selectedRow = listPtr->firstRow;
					SetControlValue(listPtr->vScroll, listPtr->firstRow);
					NewList_Update(list);
					Delay(kScrollingDelay, &delay);
				}
				else if((localPoint.v > listPtr->viewRect.bottom) && (listPtr->firstRow < listPtr->rowCount - listPtr->rowVisible)) {
					++listPtr->firstRow;
					listPtr->selectedRow = listPtr->firstRow + listPtr->rowVisible - 1;
					SetControlValue(listPtr->vScroll, listPtr->firstRow);
					NewList_Update(list);
					Delay(kScrollingDelay, &delay);
				}
			}
			GetMouse(&localPoint);
		}
		
		tick = TickCount();
		if((oldRow == listPtr->selectedRow) && (tick - listPtr->lastClickTime < GetDblTime())) {
			listPtr->lastClickTime = tick;
			return true;
		}
		else
		listPtr->lastClickTime = tick;
	}
	else if(listPtr->vScroll != nil)
	HandleControlClick(listPtr->vScroll, localPoint, nil, (ControlActionUPP) -1);
	
	return false;
}

Boolean NewList_DisplayRow(Row rowNum, NewListHandle list)
{
	NewListPtr		listPtr = *list;
	
	if((rowNum >= listPtr->firstRow) && (rowNum < listPtr->firstRow + listPtr->rowVisible))
	return true;
	
	if(rowNum < listPtr->firstRow)
	listPtr->firstRow = rowNum;	
	else
	listPtr->firstRow = rowNum - listPtr->rowVisible + 1;
	
	if(listPtr->vScroll != nil)
	SetControlValue(listPtr->vScroll, listPtr->firstRow);
	NewList_Update(list);
	
	return false;
}
