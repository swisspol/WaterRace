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


#ifndef __DIALOG__
#define __DIALOG__

//ROUTINES:

//File: Dialog Utils.cp
void InsertDialogItemText(DialogPtr dialog, short itemNum, Str255 addText);
void SetDialogItemFloat(Handle itemHandle, float num);
void GetDialogItemFloat(Handle itemHandle, float* num);
void SetDialogItemNum(Handle itemHandle, long num);
void GetDialogItemNum(Handle itemHandle, long* num);
void SetDialogItemTime(Handle itemHandle, long time);
void GetDialogItemTime(Handle itemHandle, long* time);
void Press_Button(DialogPtr theDialog, short itemNum);
void SetDialogItemID(Handle itemHandle, OSType id);
void GetDialogItemID(Handle itemHandle, OSType* id);
pascal OSStatus GetListBoxListHandle(ControlHandle control, ListHandle* list);
Point GetWindowPosition(WindowPtr window);
Boolean MakeSure_WindowIsOnScreen(WindowPtr window);
pascal OSErr AppendDialogItemList(DialogPtr dialog, short ditlID, DITLMethod method);

//File: Dialog Boxes.cp
void ColorBox_StartUp(DialogPtr dialog, short num, RGBColor* color);
void PictBox_Clip_StartUp(DialogPtr dialog, short num, PicHandle picture);
void PictBox_Schrink_StartUp(DialogPtr dialog, short num, PicHandle picture);
void PictBox_Interactive_StartUp(DialogPtr dialog, short num, PicHandle picture);
void PictBox_Interactive_CleanUp();
void TextBox_StartUp(DialogPtr dialog, short num, Ptr text);
void MenuSafeString(Str255 in, Str255 out);

//File: Spinning cursor.cp
OSErr SpinningCursor_Init();
void SpinningCursor_Quit();
void SpinningCursor_Start();
void SpinningCursor_Stop();
Boolean SpinningCursor_Active();

//File: Error system.cp
void Error_Display(short actionID, OSErr errorID, ModalFilterProcPtr eventFilterProc, Boolean fatal);

#endif
