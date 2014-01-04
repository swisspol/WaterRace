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


#include						<Traps.h>

//CONSTANTES:

#define _ControlStripDispatch 0xAAF2

//VARIABLES:

static RgnHandle			mBarRgn;
static unsigned short		mBarHeight;
static Boolean				controlStripVisible;

//ROUTINES:

static Boolean HasControlStrip()
{
	short			err;
	long			response;
	
	err = Gestalt(gestaltControlStripAttr, &response);
	if(err)
	return false;
	
	return (response & (1 << gestaltControlStripExists));
}

static UniversalProcPtr gControlStripTrapUPP = (RoutineDescriptorPtr) kUnresolvedCFragSymbolAddress;
static UniversalProcPtr gUnimplementedUPP = (RoutineDescriptorPtr) kUnresolvedCFragSymbolAddress;

static pascal Boolean SBIsControlStripVisible()
// TWOWORDINLINE(0x7000, 0xAAF2);
//	MOVEQ	#$00,D0
//	_ControlStripDispatch
{
	long	private_result = 0L;

	if ((Ptr) gUnimplementedUPP == (Ptr) kUnresolvedCFragSymbolAddress)
		gUnimplementedUPP = GetToolboxTrapAddress(_Unimplemented);

	if ((Ptr) gControlStripTrapUPP == (Ptr) kUnresolvedCFragSymbolAddress)
		gControlStripTrapUPP = GetToolboxTrapAddress(_ControlStripDispatch);

	if ((Ptr) gControlStripTrapUPP != (Ptr) gUnimplementedUPP)
	{
		private_result = CallUniversalProc(gControlStripTrapUPP,
			kD0DispatchedPascalStackBased |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kTwoByteCode) |
			RESULT_SIZE(SIZE_CODE(sizeof(Boolean))),
			0x0000);	// selector
	}
	return (Boolean) private_result;
}

static pascal void SBShowHideControlStrip(Boolean showIt)
// THREEWORDINLINE(0x303C, 0x0101, 0xAAF2);
//	MOVE.W	#$0101,D0
//	_ControlStripDispatch
{
	if ((Ptr) gUnimplementedUPP == (Ptr) kUnresolvedCFragSymbolAddress)
		gUnimplementedUPP = GetToolboxTrapAddress(_Unimplemented);

	if ((Ptr) gControlStripTrapUPP == (Ptr) kUnresolvedCFragSymbolAddress)
		gControlStripTrapUPP = GetToolboxTrapAddress(_ControlStripDispatch);

	if ((Ptr) gControlStripTrapUPP != (Ptr) gUnimplementedUPP)
	{
		CallUniversalProc(gControlStripTrapUPP,
			kD0DispatchedPascalStackBased |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kTwoByteCode) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Boolean))),
			0x0101,		// selector
			showIt);	// paramter(s)
	}
}

static void SH_ForceUpdate(RgnHandle rgn)
{
	WindowRef		wpFirst = LMGetWindowList();
	
	PaintBehind(wpFirst, rgn);
	CalcVisBehind(wpFirst, rgn);
}

static void GetMBarRgn(RgnHandle mBarRgn)
{
	Rect			mBarRect;

	mBarRect = qd.screenBits.bounds;
	mBarRect.bottom = mBarRect.top + GetMBarHeight();
	RectRgn(mBarRgn, &mBarRect);
}

void Hide_MenuBar()
{
	RgnHandle		GrayRgn = LMGetGrayRgn();
	
	mBarHeight = GetMBarHeight();
	mBarRgn = NewRgn();
	GetMBarRgn(mBarRgn);
	LMSetMBarHeight(0);
	UnionRgn(GrayRgn,mBarRgn,GrayRgn);
	SH_ForceUpdate(mBarRgn);
	
	if(HasControlStrip()) {
		controlStripVisible = SBIsControlStripVisible();
		if(controlStripVisible)
		SBShowHideControlStrip(false);
	}
}

void Show_MenuBar()
{
	RgnHandle		GrayRgn = LMGetGrayRgn();
	
	LMSetMBarHeight(mBarHeight);
	DiffRgn(GrayRgn, mBarRgn, GrayRgn);
	DisposeRgn(mBarRgn);
	DrawMenuBar();
	
	if(HasControlStrip() && controlStripVisible)
	SBShowHideControlStrip(true);
}