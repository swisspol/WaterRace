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


#include				"Infinity Error Codes.h"

//ROUTINES:

OSErr ScanForFolder(short volumeID, long parentFolderID, Str63 folderName, long* folderID)
{
	short			idx;
	CInfoPBRec		cipbr;
	HFileInfo		*fpb = (HFileInfo*) &cipbr;
	DirInfo			*dpb = (DirInfo*) &cipbr;
	Str63			name;
	
	fpb->ioVRefNum = volumeID;
	fpb->ioNamePtr = name;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = parentFolderID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		if(fpb->ioFlAttrib & 16 && EqualString(name, folderName, false, false)) {
			*folderID = dpb->ioDrDirID;
			return noErr;
		}
	}
	
	return kError_FolderNotFound;
}

OSErr ScanForFolderAlias(FSSpec* folder)
{
	short			idx;
	CInfoPBRec		cipbr,
					cipbr2;
	HFileInfo		*fpb = (HFileInfo*) &cipbr,
					*fpb2 = (HFileInfo*) &cipbr2;
	Boolean			targetIsFolder,
					wasAliased;
	FSSpec			destFolder;
	OSErr			theError;
						
	destFolder.vRefNum = folder->vRefNum;
	destFolder.parID = folder->parID;
	fpb->ioVRefNum = folder->vRefNum;
	fpb->ioNamePtr = destFolder.name;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = folder->parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		if(EqualString(destFolder.name, folder->name, false, false)) {
			theError = ResolveAliasFile(&destFolder, true, &targetIsFolder, &wasAliased);
			if(theError)
			return theError;
		
			if(targetIsFolder) {
				fpb2->ioVRefNum = destFolder.vRefNum;
				fpb2->ioNamePtr = destFolder.name;
				fpb2->ioDirID = destFolder.parID;
				fpb2->ioFDirIndex = 0;
				theError = PBGetCatInfo(&cipbr2, false);
				if(theError)
				return theError;
				
				destFolder.parID = fpb2->ioDirID;
				*folder = destFolder;
				
				return noErr;
			}
		}
	}
	
	return kError_FolderNotFound;
}

OSErr Find_FrenchTouchPreferencesFolder(short* volumeID, long* folderNum)
{
	OSErr		theError;
	Str63		folderName;
	long		prefFolderNum;
	
	theError = FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder, volumeID, &prefFolderNum);
	if(theError)
	return theError;
	
	GetIndString(folderName, 128, 1);
	theError = ScanForFolder(*volumeID, prefFolderNum, folderName, folderNum);
	if(theError)
	return theError;
	
	return noErr;
}

short GetStartUpVolume()
{
	Str63			name;
	ParamBlockRec	volPB;
	
	volPB.volumeParam.ioNamePtr = name;
	volPB.volumeParam.ioVRefNum = 0;
	volPB.volumeParam.ioVolIndex = 1;
	
	PBGetVInfo(&volPB, false);

	return volPB.volumeParam.ioVRefNum;
}

void SetDefaultDirectory(FSSpec* spec)
{
	LMSetCurDirStore(spec->parID);
	LMSetSFSaveDisk(-spec->vRefNum);
}
