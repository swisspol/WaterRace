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


#ifndef __NEWLIST__
#define __NEWLIST__

//CONSTANTES:

#define				kNewList_NoRowSelected			-1
#define				kNewList_RowHeight				16

//STRUCTURES:

typedef struct {
	WindowPtr			window;
	Rect				viewRect;
	
	Boolean				active;
	ControlHandle		vScroll;
	long				lastClickTime;
	
	short				maxRowCount,
						rowCount,
						selectedRow,
						firstRow,
						rowVisible;
	Str31				rowList[];
} NewList_Definition;
typedef NewList_Definition NewList;
typedef NewList_Definition* NewListPtr;
typedef NewList_Definition** NewListHandle;

typedef short Row;
typedef Row* RowPtr;

//MACROS:

#define GetRow(l) ((**(l)).selectedRow)

//ROUTINES:

NewListHandle NewList_New(Rect* viewRect, short maxNumRows, WindowPtr window, Boolean drawIt, Boolean scrollBar);
void NewList_Dispose(NewListHandle list);
Row NewList_Delete(short count, Row rowNum, NewListHandle list);
Row NewList_Add(short count, Row rowNum, NewListHandle list);
void NewList_Draw(Row rowNum, NewListHandle list);
void NewList_Update(NewListHandle list);
void NewList_Select(Row rowNum, NewListHandle list);
Row NewList_GetSelect(NewListHandle list);
void NewList_SetDrawingMode(Boolean drawIt, NewListHandle list);
void NewList_SetRow(void* dataPtr, short dataSize, Row rowNum, NewListHandle list);
Boolean NewList_Click(Point localPoint, NewListHandle list);
Boolean NewList_DisplayRow(Row rowNum, NewListHandle list);

#endif
