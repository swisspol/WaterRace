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


#include					"Infinity Structures.h"
#include					"Infinity Terrain.h"
#include					"MetaFile Structures.h"
#include					"MetaFile Utils.h"

//CONSTANTES:

#define				kModelChooserDialogID		9980

//ROUTINES:

static pascal OSStatus GetListBoxListHandle(ControlHandle control, ListHandle* list)
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

long MetaFile_PickModelIndex(MetaFile_ReferencePtr reference, UniversalProcPtr filterProc)
{
	GrafPtr				savePort;
	short				itemHit;
	ControlHandle		control;
	ListHandle			screenList;
	Cell				theCell;
	long				modelCount,
						i;
	Str63				name;
	DialogPtr			theDialog;
	OSType				id;
	
	modelCount = MetaFile_CountTags(reference, kTag_MegaObject);
	if(modelCount <= 1)
	return modelCount;
	ParamText("\pmodel", nil, nil, nil);
	
	GetPort(&savePort);
	theDialog = GetNewDialog(kModelChooserDialogID, nil, (WindowPtr) -1);
	SetPort(theDialog);
	
	GetDialogItemAsControl(theDialog, 2, &control);
	GetListBoxListHandle(control, &screenList);
	LSetDrawingMode(true, screenList);
	LAddRow(modelCount, 1000, screenList);
	
	theCell.h = 0;
	for(i = 0; i < modelCount; ++i) {
		//Extract model name
		MetaFile_Read_MegaObjectNameID_ByIndex(reference, name, &id, i + 1);
		
		//Add name to list
		theCell.v = i;
		LSetCell(&name[1], name[0], theCell, screenList);
	}
	theCell.v = 0;
	LSetSelect(true, theCell, screenList);
	
	SetDialogDefaultItem(theDialog, 1);
	FlushEvents(everyEvent, 0);
	
	InitCursor();
	do {
		ModalDialog(filterProc, &itemHit);
		
		theCell.h = 0;
		theCell.v = 0;
		if(!LGetSelect(true, &theCell, screenList)) {
			theCell.h = 0;
			theCell.v = modelCount - 1;
			LSetSelect(true, theCell, screenList);
		}
	} while(itemHit != 1);
	
	DisposeDialog(theDialog);
	SetPort(savePort);
	
	return theCell.v + 1;
}

long MetaFile_PickModelTextureSetIndex(MetaFile_ReferencePtr reference, OSType modelID, UniversalProcPtr filterProc)
{
	GrafPtr				savePort;
	short				itemHit;
	ControlHandle		control;
	ListHandle			screenList;
	Cell				theCell;
	long				setCount,
						i;
	Str63				name;
	DialogPtr			theDialog;
	
	setCount = MetaFile_CountTags_InsideMegaObject(reference, modelID, kTag_TextureSet);
	if(setCount <= 1)
	return setCount;
	
	GetPort(&savePort);
	theDialog = GetNewDialog(kModelChooserDialogID, nil, (WindowPtr) -1);
	SetPort(theDialog);
	ParamText("\ptexture set", nil, nil, nil);
	
	GetDialogItemAsControl(theDialog, 2, &control);
	GetListBoxListHandle(control, &screenList);
	LSetDrawingMode(true, screenList);
	LAddRow(setCount, 1000, screenList);
	
	theCell.h = 0;
	for(i = 0; i < setCount; ++i) {
		//Extract texture set name
		MetaFile_Read_TextureSetName_ByIndex(reference, modelID, name, i + 1);
		
		//Add name to list
		theCell.v = i;
		LSetCell(&name[1], name[0], theCell, screenList);
	}
	theCell.v = 0;
	LSetSelect(true, theCell, screenList);
	
	SetDialogDefaultItem(theDialog, 1);
	FlushEvents(everyEvent, 0);
	
	InitCursor();
	do {
		ModalDialog(filterProc, &itemHit);
		
		theCell.h = 0;
		theCell.v = 0;
		if(!LGetSelect(true, &theCell, screenList)) {
			theCell.h = 0;
			theCell.v = setCount - 1;
			LSetSelect(true, theCell, screenList);
		}
	} while(itemHit != 1);
	
	DisposeDialog(theDialog);
	SetPort(savePort);
	
	return theCell.v + 1;
}
