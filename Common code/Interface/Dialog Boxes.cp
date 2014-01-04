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

#define				kColorVeryVeryDarkGrey	30000
#define				kMaxPictBoxes			4
#define				kHandCursorID			-20709

#define				kMargin					3
#define				kBottomText				32000

//STRUCTURES LOCALES:

typedef struct {
	ControlHandle		control;
	PicHandle			picture;
	Boolean				interactive;
	Point				offset,
						offsetMax;
} PictBoxData;
typedef PictBoxData* PictBoxDataPtr;

//VARIABLES LOCALES:

static RGBColor		veryVeryDarkGrey = {kColorVeryVeryDarkGrey, kColorVeryVeryDarkGrey, kColorVeryVeryDarkGrey};
static PictBoxData	pictBoxDataList[kMaxPictBoxes];
static short		pixtBoxCount = 0;

//ROUTINES:

/*************************** Color Box ***********************************/

static void ColorBox_Draw(ControlHandle theControl, Boolean active)
{
	Rect		bounds;
	RGBColor	*color,
				oldColor;
	
	bounds = (**theControl).contrlRect;
	color = (RGBColor*) (**theControl).contrlRfCon;
	GetForeColor(&oldColor);
	
	RGBForeColor(&veryVeryDarkGrey);
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	OffsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	--bounds.right;
	--bounds.bottom;
	FrameRect(&bounds);
	
	if(active)
	ForeColor(blackColor);
	else
	ForeColor(whiteColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	RGBForeColor(color);
	InsetRect(&bounds, 1, 1);
	PaintRect(&bounds);
	
	RGBForeColor(&oldColor);
}

static pascal void ColorBox_DrawProc(ControlHandle theControl, short thePart)
{
	ColorBox_Draw(theControl, false);
}

static pascal ControlPartCode ColorBox_TrackProc(ControlHandle theControl, Point startPt, ControlActionUPP actionProc)
{
	Rect		bounds;
	Boolean		inButton,
				wasInButton = true;
	Point		mouse;
	
	bounds = (**theControl).contrlRect;
	InsetRect(&bounds, 3, 3);
	
	ColorBox_Draw(theControl, true);
	while(Button()) {
		GetMouse(&mouse);
		inButton = PtInRect(mouse, &bounds);
		if(inButton && !wasInButton) {
			ColorBox_Draw(theControl, true);
			wasInButton = inButton;
		}
		else if(!inButton && wasInButton) {
			ColorBox_Draw(theControl, false);
			wasInButton = inButton;
		}
	}
	
	if(inButton) {
		ColorBox_Draw(theControl, false);
		return kControlButtonPart;
	}
	
	return kControlNoPart;
}

static pascal ControlPartCode ColorBox_HitProc(ControlHandle theControl, Point startPt)
{
	Rect		bounds;
	
	bounds = (**theControl).contrlRect;
	InsetRect(&bounds, 3, 3);
	
	if(PtInRect(startPt, &bounds))
	return kControlButtonPart;
	
	return kControlNoPart;
}

void ColorBox_StartUp(DialogPtr dialog, short num, RGBColor* color)
{
	UniversalProcPtr	ColorBoxDrawRoutine = NewControlUserPaneDrawProc(ColorBox_DrawProc),
						ColorBoxHitRoutine = NewControlUserPaneHitTestProc(ColorBox_HitProc),
						ColorBoxTrackRoutine = NewControlUserPaneTrackingProc(ColorBox_TrackProc);
	ControlHandle		box;
	
	GetDialogItemAsControl(dialog, num, &box);
	SetControlReference(box, (long) color);
	SetControlData(box, 0, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr) &ColorBoxDrawRoutine);
	SetControlData(box, 0, kControlUserPaneHitTestProcTag, sizeof(ControlUserPaneHitTestUPP), (Ptr) &ColorBoxHitRoutine);
	SetControlData(box, 0, kControlUserPaneTrackingProcTag, sizeof(ControlUserPaneTrackingUPP), (Ptr) &ColorBoxTrackRoutine);
}

/*************************** PICT Box Clip ***********************************/

static pascal void PictBox_DrawClipProc(ControlHandle theControl, short thePart)
{
	Rect		bounds;
	RGBColor	oldColor;
	PicHandle	picture;
	RgnHandle	saveClipping;
	
	bounds = (**theControl).contrlRect;
	picture = (PicHandle) (**theControl).contrlRfCon;
	GetForeColor(&oldColor);
	
	RGBForeColor(&veryVeryDarkGrey);
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	OffsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	--bounds.right;
	--bounds.bottom;
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	InsetRect(&bounds, 1, 1);
	
	
	saveClipping = NewRgn();
	GetClip(saveClipping);
	
	ClipRect(&bounds);
	
	bounds.right = bounds.left + (**picture).picFrame.right - (**picture).picFrame.left;
	bounds.bottom = bounds.top + (**picture).picFrame.bottom - (**picture).picFrame.top;
	DrawPicture(picture, &bounds);
	
	SetClip(saveClipping);
	DisposeRgn(saveClipping);
	
	RGBForeColor(&oldColor);
}

void PictBox_Clip_StartUp(DialogPtr dialog, short num, PicHandle picture)
{
	UniversalProcPtr	PictBoxDrawRoutine = NewControlUserPaneDrawProc(PictBox_DrawClipProc);
	ControlHandle		box;
	Rect*				destRect;
	short				width,
						height;
	
	GetDialogItemAsControl(dialog, num, &box);
	SetControlReference(box, (long) picture);
	SetControlData(box, 0, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr) &PictBoxDrawRoutine);
	
	//Calculate box size				
	destRect = &((**box).contrlRect);					
	width = (**picture).picFrame.right - (**picture).picFrame.left;
	height = (**picture).picFrame.bottom - (**picture).picFrame.top;
	
	if(width <= destRect->right - destRect->left)
	destRect->right = destRect->left + width;
	if(height <= destRect->bottom - destRect->top)
	destRect->bottom = destRect->top + height;
	
	destRect->right += 7;
	destRect->bottom += 7;
}

/*************************** PICT Box (schrink mode) ***********************************/

static pascal void PictBox_DrawSchrinkProc(ControlHandle theControl, short thePart)
{
	Rect		bounds;
	RGBColor	oldColor;
	PicHandle	picture;
	
	bounds = (**theControl).contrlRect;
	picture = (PicHandle) (**theControl).contrlRfCon;
	GetForeColor(&oldColor);
	
	RGBForeColor(&veryVeryDarkGrey);
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	OffsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	--bounds.right;
	--bounds.bottom;
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	InsetRect(&bounds, 1, 1);
	DrawPicture(picture, &bounds);
	RGBForeColor(&oldColor);
}

void PictBox_Schrink_StartUp(DialogPtr dialog, short num, PicHandle picture)
{
	UniversalProcPtr	PictBoxDrawRoutine = NewControlUserPaneDrawProc(PictBox_DrawSchrinkProc);
	ControlHandle		box;
	float				xRatio,
						yRatio;
	Rect*				destRect;
	short				width,
						height;
	
	GetDialogItemAsControl(dialog, num, &box);
	SetControlReference(box, (long) picture);
	SetControlData(box, 0, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr) &PictBoxDrawRoutine);
	
	//Calculate box size				
	destRect = &((**box).contrlRect);					
	width = (**picture).picFrame.right - (**picture).picFrame.left;
	height = (**picture).picFrame.bottom - (**picture).picFrame.top;
	
	if((width <= destRect->right - destRect->left) && (height <= destRect->bottom - destRect->top)) {
		destRect->right = destRect->left + width;
		destRect->bottom = destRect->top + height;
	}
	else {
		xRatio = (float) width / (float) (destRect->right - destRect->left);
		yRatio = (float) height / (float) (destRect->bottom - destRect->top);
		
		if(xRatio > yRatio) {
			destRect->bottom = destRect->top + height / xRatio;
			destRect->right = destRect->left + width / xRatio;
		}
		else {
			destRect->bottom = destRect->top + height / yRatio;
			destRect->right = destRect->left + width / yRatio;
		}
	}
	destRect->right += 7;
	destRect->bottom += 7;
}

/*************************** PICT Box Interactive ***********************************/

static pascal void PictBox_DrawInteractive(ControlHandle theControl, Boolean active)
{
	Rect				bounds;
	RGBColor			oldColor;
	RgnHandle			saveClipping;
	PictBoxDataPtr		data;
	PicHandle			picture;
	
	data = (PictBoxDataPtr) GetControlReference(theControl);
	picture = data->picture;
	
	bounds = (**theControl).contrlRect;
	GetForeColor(&oldColor);
	
	RGBForeColor(&veryVeryDarkGrey);
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	OffsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	--bounds.right;
	--bounds.bottom;
	FrameRect(&bounds);
	
	if(active)
	ForeColor(blackColor);
	else
	ForeColor(whiteColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	InsetRect(&bounds, 1, 1);
	
	saveClipping = NewRgn();
	GetClip(saveClipping);
	
	ClipRect(&bounds);
	
	bounds.right = bounds.left + (**picture).picFrame.right - (**picture).picFrame.left;
	bounds.bottom = bounds.top + (**picture).picFrame.bottom - (**picture).picFrame.top;
	OffsetRect(&bounds, -data->offset.h, -data->offset.v);
	DrawPicture(picture, &bounds);
	
	SetClip(saveClipping);
	DisposeRgn(saveClipping);
	
	RGBForeColor(&oldColor);
}

static pascal void PictBox_DrawInteractiveProc(ControlHandle theControl, short thePart)
{
	PictBox_DrawInteractive(theControl, false);
}

static pascal ControlPartCode PictBox_TrackProc(ControlHandle theControl, Point startPt, ControlActionUPP actionProc)
{
	Point				mouse,
						startMouse,
						oldOffset;
	PictBoxDataPtr		data;
	CursHandle			handCursor;
	long				i;
	
	GetMouse(&startMouse);
	data = (PictBoxDataPtr) GetControlReference(theControl);
	oldOffset = data->offset;
	handCursor = GetCursor(kHandCursorID);
	HLock((Handle) handCursor);
	SetCursor(*handCursor);
	
	while(Button()) {
		GetMouse(&mouse);
		
		if(data->offsetMax.h > 0) {
			data->offset.h = oldOffset.h - (mouse.h - startMouse.h);
			if(data->offset.h > data->offsetMax.h)
			data->offset.h = data->offsetMax.h;
			else if(data->offset.h < 0)
			data->offset.h = 0;
		}
		if(data->offsetMax.v > 0) {
			data->offset.v = oldOffset.v - (mouse.v - startMouse.v);
			if(data->offset.v > data->offsetMax.v)
			data->offset.v = data->offsetMax.v;
			else if(data->offset.v < 0)
			data->offset.v = 0;
		}
		
		for(i = 0; i < pixtBoxCount; ++i) {
			pictBoxDataList[i].offset = data->offset;
			PictBox_DrawInteractive(pictBoxDataList[i].control, true);
		}
	}
	InitCursor();
	HUnlock((Handle) handCursor);
	ReleaseResource((Handle) handCursor);
	
	for(i = 0; i < pixtBoxCount; ++i)
	PictBox_DrawInteractive(pictBoxDataList[i].control, false);
	
	return kControlNoPart;
}

static pascal ControlPartCode PictBox_HitProc(ControlHandle theControl, Point startPt)
{
	Rect		bounds;
	
	bounds = (**theControl).contrlRect;
	InsetRect(&bounds, 3, 3);
	
	if(PtInRect(startPt, &bounds))
	return kControlButtonPart;
	
	return kControlNoPart;
}

void PictBox_Interactive_StartUp(DialogPtr dialog, short num, PicHandle picture)
{
	UniversalProcPtr	PictBoxDrawRoutine = NewControlUserPaneDrawProc(PictBox_DrawInteractiveProc),
						PictBoxHitRoutine = NewControlUserPaneHitTestProc(PictBox_HitProc),
						PictBoxTrackRoutine = NewControlUserPaneTrackingProc(PictBox_TrackProc);
	ControlHandle		box;
	Rect*				destRect;
	short				width,
						height;
	PictBoxDataPtr		data;
	
	if(pixtBoxCount == kMaxPictBoxes)
	return;
	data = &pictBoxDataList[pixtBoxCount];
	++pixtBoxCount;
	
	GetDialogItemAsControl(dialog, num, &box);
	SetControlReference(box, (long) data);
	SetControlData(box, 0, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr) &PictBoxDrawRoutine);
	
	//Calculate box size				
	destRect = &((**box).contrlRect);					
	width = (**picture).picFrame.right - (**picture).picFrame.left;
	height = (**picture).picFrame.bottom - (**picture).picFrame.top;
	
	data->control = box;
	data->picture = picture;
	if((width > destRect->right - destRect->left) || (height > destRect->bottom - destRect->top)) {
		data->interactive = true;
		SetControlData(box, 0, kControlUserPaneHitTestProcTag, sizeof(ControlUserPaneHitTestUPP), (Ptr) &PictBoxHitRoutine);
		SetControlData(box, 0, kControlUserPaneTrackingProcTag, sizeof(ControlUserPaneTrackingUPP), (Ptr) &PictBoxTrackRoutine);
	}
	else
	data->interactive = false;
	data->offset.h = 0;
	data->offset.v = 0;
	data->offsetMax.h = width - (destRect->right - destRect->left);
	data->offsetMax.v = height - (destRect->bottom - destRect->top);
	
	if(width <= destRect->right - destRect->left)
	destRect->right = destRect->left + width;
	if(height <= destRect->bottom - destRect->top)
	destRect->bottom = destRect->top + height;
	
	destRect->right += 7;
	destRect->bottom += 7;
}

void PictBox_Interactive_CleanUp()
{
	pixtBoxCount = 0;
}

/*************************** Text Box ***********************************/

static pascal void TextBox_DrawProc(ControlHandle theControl, short thePart)
{
	Rect		bounds;
	Ptr			textPtr;
	short		textSize;
	RGBColor	oldColor;
	TEHandle	theTEHandle;
	Rect		destRect;
	
	bounds = (**theControl).contrlRect;
	textPtr = (Ptr) (**theControl).contrlRfCon + 2;
	textSize = *((short*) (**theControl).contrlRfCon);
	
	GetForeColor(&oldColor);
	
	RGBForeColor(&veryVeryDarkGrey);
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	OffsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	--bounds.right;
	--bounds.bottom;
	FrameRect(&bounds);
	
	ForeColor(whiteColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	ForeColor(blackColor);
	InsetRect(&bounds, 1, 1);
	FrameRect(&bounds);
	
	RGBForeColor(&oldColor);
	
	InsetRect(&bounds, kMargin + 1, kMargin + 1);
	destRect.top = bounds.top;
	destRect.left = bounds.left;
	destRect.bottom = kBottomText;
	destRect.right = bounds.right;
	theTEHandle = TENew(&destRect, &bounds);
	TESetText(textPtr, textSize, theTEHandle);
	TEPinScroll(0, -kBottomText, theTEHandle);
	TEUpdate(&bounds, theTEHandle);
	TEDispose(theTEHandle);
}

void TextBox_StartUp(DialogPtr dialog, short num, Ptr text)
{
	UniversalProcPtr	TextBoxDrawRoutine = NewControlUserPaneDrawProc(TextBox_DrawProc);
	ControlHandle		box;
	
	GetDialogItemAsControl(dialog, num, &box);
	SetControlReference(box, (long) text);
	SetControlData(box, 0, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr) &TextBoxDrawRoutine);
}
