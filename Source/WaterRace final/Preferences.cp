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


#include				<QD3DAcceleration.h>

#include				"WaterRace.h"
#include				"Strings.h"
#include				"InputSprocket.h"
#include				"Preferences.h"
#include				"Data Files.h"
#include				"Interface.h"
#include				"ISpRoutines.h"

#include				"Textures.h"

#include				"Keys.h"
#include				"Dialog Utils.h"
#include				"Folder Utils.h"

#if __USE_AUDIO_CD_TRACKS__
#include					"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include					"QDesign Player.h"
#endif

//CONSTANTES:

#define					kScreenResType	 		'ScRs'
#define					kScreenResID			128

#define					kNumItems				5
#define					kTabItemID				3
#define					kHelpItemID				4
#define					kHelpMainResID			300

#define					kWaitDelay				0
#if __LANGUAGE__ == kLanguage_English
#define					kLicenseTextID			1000
#elif __LANGUAGE__ == kLanguage_French
#define					kLicenseTextID			1100
#elif __LANGUAGE__ == kLanguage_German
#define					kLicenseTextID			1200
#elif __LANGUAGE__ == kLanguage_Italian
#define					kLicenseTextID			1300
#elif
#error  __LANGUAGE__ undefined!
#endif

#define					kPrefDialogID			128

enum {
	k3DEngineDITLID = 200,
	kSoundDITLID,
	kControlDITLID,
	kNetworkDITLID
};

enum {
	k3DEngine = 1,
	kSound,
	kControl,
	kNetwork
};

enum {
	kMac_iMac = 1,
	kMac_iMac_RevBC,
	kMac_iMac_RevA,
	kMac_iBook = 5,
	kMac_iBook_Original,
	kMac_Yosemite = 8,
	kMac_PowerMacG3,
	kMac_PowerMacG3_Original,
	kMac_G4_AGP = 12,
	kMac_G4_PCI,
	kMac_PowerBookG3_FireWire = 15,
	kMac_PowerBookG3_Bronze,
	kMac_PowerBookG3,
	kMac_PowerBookG3_Original,
	k3Dfx_Voodoo_4500_5500 = 20,
	k3Dfx_Voodoo_2000_3000,
	kFormac_ProformanceIII_16_32 = 23,
	kFormac_ProformanceIII_8
};

//PROTOTYPES:

extern Boolean Display_Agreement(short textID, long waitingTime, Str255 okButton, Str255 cancelButton);

//VARIABLES LOCALES:

static long				gameStartTime;
static short			curPane = k3DEngine;

//VARIABLES:

Preferences				thePrefs;

//ROUTINES:

static long Auto_Config()
{
	DialogPtr				theDialog;
	short					itemHit,
							itemType;
	Rect					aRect;
	Handle					computerItem,
							okItem;
	long					value = -1;
	
	//Show dialog
	UseResFile(mainResFileID);
	theDialog = GetNewDialog(131, nil, (WindowPtr) -1);
	SetPort(theDialog);
	GetDialogItem(theDialog, 1, &itemType, &okItem, &aRect);
	GetDialogItem(theDialog, 3, &itemType, &computerItem, &aRect);
	HiliteControl((ControlHandle) okItem, 255);
	
	do {
		ModalDialog(nil, &itemHit);
		
		if(itemHit == 3) {
			if(GetControlValue((ControlHandle) computerItem) != 1) {
				HiliteControl((ControlHandle) okItem, 0);
				SetDialogDefaultItem(theDialog, 1);
			}
			else
			HiliteControl((ControlHandle) okItem, 255);
		}
	} while((itemHit != 1) && (itemHit != 2));
	
	if(itemHit == 1)
	value = GetControlValue((ControlHandle) computerItem) - 2;
	DisposeDialog(theDialog);
	
	return value;
}

OSErr Preference_Read()
{
	OSErr		theError;
	short		fileID,
				volumeID;
	long		bytesNumber,
				prefFolderNum;
	Str63		prefName;
	long		computer;
				
	//Update running time
	gameStartTime = TickCount();
	
	theError = Find_FrenchTouchPreferencesFolder(&volumeID, &prefFolderNum);
	if(theError)
	goto New;
	
	GetIndString(prefName, 128, 2);
	theError = HOpen(volumeID, prefFolderNum, prefName, fsRdPerm, &fileID);
	if(theError)
	goto New;
	SetFPos(fileID, 1, 0);
	bytesNumber = sizeof(Preferences);
	theError = FSRead(fileID, &bytesNumber, &thePrefs);
	FSClose(fileID);
	
	if(thePrefs.version != kPref_Version)
	goto New;
	
	return noErr;
	
	New:
#if __LANGUAGE__ == kLanguage_English
	if(Display_Agreement(kLicenseTextID, kWaitDelay, "\pAgree", "\pDisagree") == false)
#elif __LANGUAGE__ == kLanguage_French
	if(Display_Agreement(kLicenseTextID, kWaitDelay, "\pAccepter", "\pRefuser") == false)
#elif __LANGUAGE__ == kLanguage_German
	if(Display_Agreement(kLicenseTextID, kWaitDelay, "\pAkzeptieren", "\pAblehnen") == false)
#elif __LANGUAGE__ == kLanguage_Italian
	if(Display_Agreement(kLicenseTextID, kWaitDelay, "\pAccetto", "\pNon Accetto") == false)
#else
#error  __LANGUAGE__ undefined!
#endif
	return kError_LicenseNotAccepted;
	
	computer = Auto_Config();
	if(computer < 0)
	return kError_UserCancel;
	
	thePrefs.version = kPref_Version;
	thePrefs.raveEngineName[0] = 0;
	
	switch(computer) {
		
		case kMac_iMac: //Rage 128 AGP 8Mb
		case kMac_PowerBookG3_FireWire: //(*)
		case kMac_iBook:
#if 1
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagTriFiltering 
			 + kPref_EngineFlagVideoFX + kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap;
#else
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagTriFiltering 
			+ kPref_EngineFlag32BitRendering + kPref_EngineFlagVideoFX + kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_ForceMipMap;
#endif
		break;
		
		case kMac_PowerMacG3_Original: //Rage Pro 2/4/6Mb (*)
		case kMac_iMac_RevBC: //Rage Pro 6Mb (*)
		thePrefs.renderWidth = 640;
		thePrefs.renderHeight = 480;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagVideoFX;
		thePrefs.textureFlags = kTextureFlag_Reduce4X;
		break;
		
		case kMac_iBook_Original: //Rage Mobility AGP 4Mb
		thePrefs.renderWidth = 640;
		thePrefs.renderHeight = 480;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagVideoFX;
		thePrefs.textureFlags = kTextureFlag_Reduce4X;
		break;
		
		case kMac_PowerBookG3_Bronze: //Rage LT Pro 8Mb
		thePrefs.renderWidth = 640;
		thePrefs.renderHeight = 480;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagVideoFX;
		thePrefs.textureFlags = 0;
		break;
		
		case kMac_PowerBookG3: //Rage LT Pro 4Mb
		thePrefs.renderWidth = 640;
		thePrefs.renderHeight = 480;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagVideoFX;
		thePrefs.textureFlags = kTextureFlag_Reduce16X;
		break;
		
		case kMac_PowerBookG3_Original: //Rage LT 4Mb (*)
		case kMac_iMac_RevA: //Rage IIc 2/4/6Mb
		case kMac_PowerMacG3: //Rage II+ 2/4/6Mb
		thePrefs.renderWidth = 600;
		thePrefs.renderHeight = 400;
		thePrefs.engineFlags = kPref_EngineFlagLowResolution;
		thePrefs.textureFlags = kTextureFlag_Reduce16X;
		break;
		
		case kMac_G4_AGP: //Rage 128 Pro 16Mb
		case kMac_G4_PCI:
#if 1
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagTriFiltering 
			+ kPref_EngineFlag32BitRendering + kPref_EngineFlagVideoFX + kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap;
#else
		thePrefs.renderWidth = 1024;
		thePrefs.renderHeight = 768;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagTriFiltering 
			+ kPref_EngineFlag32BitRendering + kPref_EngineFlagVideoFX + kPref_EngineFlag32BitZBuffer
			+ kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap;
#endif
		break;
		
		case kMac_Yosemite: //Rage 128 16Mb (*)
#if 1
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagTriFiltering 
			+ kPref_EngineFlag32BitRendering + kPref_EngineFlagVideoFX + kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap;
#else
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagTriFiltering 
			+ kPref_EngineFlag32BitRendering + kPref_EngineFlagVideoFX + kPref_EngineFlag32BitZBuffer
			+ kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap;
#endif
		break;
		
		case k3Dfx_Voodoo_2000_3000: //16Mb
#if 1
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagUseSoftware + kPref_EngineFlagFiltering + kPref_EngineFlagVideoFX + kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap + kTextureFlag_3DfxMode;
#else
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlag32BitRendering
			+ kPref_EngineFlagVideoFX + kPref_EngineFlag32BitZBuffer;
		thePrefs.textureFlags = kTextureFlag_MipMap + kTextureFlag_3DfxMode;
#endif
		break;
		
		case k3Dfx_Voodoo_4500_5500: //32Mb
		thePrefs.renderWidth = 800;
		thePrefs.renderHeight = 600;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlag32BitRendering
			+ kPref_EngineFlagVideoFX + kPref_EngineFlag32BitZBuffer + kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap;
		break;
		
		case kFormac_ProformanceIII_16_32: //16+ Mb (*)
		thePrefs.renderWidth = 1024;
		thePrefs.renderHeight = 768;
		thePrefs.engineFlags = kPref_EngineFlagFiltering + kPref_EngineFlagTriFiltering 
			+ kPref_EngineFlag32BitRendering + kPref_EngineFlagVideoFX + kPref_EngineFlag32BitZBuffer
			+ kPref_EngineFlagFurtherClipping;
		thePrefs.textureFlags = kTextureFlag_MipMap;
		break;
		
		case kFormac_ProformanceIII_8: //8Mb
		thePrefs.renderWidth = 832;
		thePrefs.renderHeight = 624;
		thePrefs.engineFlags = kPref_EngineFlagFiltering  + kPref_EngineFlagVideoFX;
		thePrefs.textureFlags = 0;
		break;
		
	}
	
	thePrefs.soundFlags = kPref_SoundFlagGameMusic;
	thePrefs.controlFlags = 0;
	thePrefs.screenID = kInvalidDisplayID;
	thePrefs.interfaceScreen.csMode = 0;
	thePrefs.gameScreen.csMode = 0;
	thePrefs.musicVolume = 8 * 256 / 10;
	thePrefs.soundFXVolume = 4 * 256 / 10;
	BlockMove(kString_Preferences_Name, thePrefs.playerName, sizeof(Str63));
	BlockMove(kString_Preferences_Location, thePrefs.playerLocation, sizeof(Str63));
	BlockMove(kString_Preferences_Computer, thePrefs.hostName, sizeof(Str63));
	thePrefs.hostPassword[0] = 0;
	BlockMove(kString_Preferences_Welcome, thePrefs.hostMessage, sizeof(Str255));
	thePrefs.networkFlags = kPref_NetworkFlagBroadCast;
	thePrefs.runningTime = 0;
	thePrefs.lastNetworkServerIP[0] = 0;
	thePrefs.lastPort = 0;
	
	return noErr;
}

void Preference_Write()
{
	OSErr		theError;
	short		fileID,
				volumeID;
	long		bytesNumber,
				prefFolderNum;
	Str63		prefName,
				folderName;
	
	theError = Find_FrenchTouchPreferencesFolder(&volumeID, &prefFolderNum);
	if(theError) {
		theError = FindFolder(kOnSystemDisk, 'pref', kDontCreateFolder, &volumeID, &prefFolderNum);
		if(theError)
		return;
		GetIndString(folderName, 128, 1);
		DirCreate(volumeID, prefFolderNum, folderName, &prefFolderNum);
	}
	
	GetIndString(prefName, 128, 2);
	theError = HOpen(volumeID, prefFolderNum, prefName, fsRdWrPerm, &fileID);
	if(theError) {
		theError = HCreate(volumeID, prefFolderNum, prefName, kCreatorType, 'pref');
		if(theError)
		return;
		theError = HOpen(volumeID, prefFolderNum, prefName, fsRdWrPerm, &fileID);
		if(theError)
		return;
	}
	
	//Update running time
	thePrefs.runningTime += (TickCount() - gameStartTime) / 60;
	
	SetFPos(fileID, 1, 0);
	bytesNumber = sizeof(Preferences);
	theError = FSWrite(fileID, &bytesNumber, &thePrefs);
	FSClose(fileID);
}

//Infinity Engine pane

static void PaneDisplay_3DEngine(DialogPtr dialog, short offset, short* maxItem, PreferencesPtr prefs)
{
	ControlHandle		items[13];
	long				i;
	Handle				screenResList;
	Point				*resolution;
	short				nbRes,
						maxWidth,
						maxHeight,
						depth;
	MenuHandle			theMenu,
						theMenu2;
	TQAEngine			*engine;
	TQADevice			raveDevice;
	Str255				name;
	short				engineID = -1,
						count = 0;
	
	//Gather rave engine list
	theMenu = GetMenu(128);
	raveDevice.deviceType = kQADeviceGDevice;
	raveDevice.device.gDevice = mainGDHandle;
	QAEngineEnable(kQAVendor_Apple, kQAEngine_AppleHW); //Enable Apple accelerator card
	engine = QADeviceGetFirstEngine(&raveDevice);
	while(engine != NULL)
	{
		QAEngineGestalt(engine, kQAGestalt_ASCIINameLength, &i);
		QAEngineGestalt(engine, kQAGestalt_ASCIIName, &name[1]);
		name[0] = i;
		
		if(prefs->raveEngineName[0] != 0) {
			if(EqualString(prefs->raveEngineName, name, false, true))
			engineID = count;
		}
		
		InsertMenuItem(theMenu, name, 100);
		++count;
		engine = QADeviceGetNextEngine(&raveDevice, engine);
	}
	
	//Get screen resolutions list
	screenResList = Get1Resource(kScreenResType, kScreenResID);
	if(screenResList == nil)
	return;
	DetachResource(screenResList);
	HLock(screenResList);
	nbRes = *((short*) *screenResList);
	resolution = (Point*) (*screenResList + 2);
	
	//Get max resolution
	if(thePrefs.engineFlags & kPref_EngineFlag32BitRendering)
	depth = 32;
	else
	depth = kInterface_Depth;
#if !__VR_MODE__
	if(SL_GetMaxResolution(mainGDHandle, &maxWidth, &maxHeight, depth) == noErr) {
		theMenu2 = GetMenu(129);
		for(i = 0; i < nbRes; ++i)
		if((resolution[i].v > maxWidth) || (resolution[i].h > maxHeight))
		DisableItem(theMenu2, i + 1);
	}
#endif
	
	//Append DITL pane
	AppendDialogItemList(dialog, k3DEngineDITLID, overlayDITL);
	*maxItem = offset + 13;
	
	//Get items
	for(i = 0; i < 13; ++i)
	GetDialogItemAsControl(dialog, offset + 1 + i, &items[i]);
	
	//Set items
	if(prefs->raveEngineName[0] == 0)
	SetControlValue(items[0], 1);
	else {
		if(engineID != -1)
		SetControlValue(items[0], 3 + engineID);
		else
		prefs->raveEngineName[0] = 0;
	}
	if(prefs->engineFlags & kPref_EngineFlagUseSoftware)
	SetControlValue(items[1], 1);
	
	for(i = 0; i < nbRes; ++i)
	if((resolution[i].v == prefs->renderWidth) && (resolution[i].h == prefs->renderHeight))
	SetControlValue(items[2], i + 1);
	if(prefs->engineFlags & kPref_EngineFlagLowResolution)
	SetControlValue(items[3], 1);
	
	if(prefs->textureFlags & kTextureFlag_Reduce4X)
	SetControlValue(items[4], 2);
	else if(prefs->textureFlags & kTextureFlag_Reduce16X)
	SetControlValue(items[4], 3);
	else if(prefs->textureFlags & kTextureFlag_Compress)
	SetControlValue(items[4], 4);
	
	if(prefs->textureFlags & (kTextureFlag_Compress + kTextureFlag_Reduce4X + kTextureFlag_Reduce16X)) {
		SetControlValue(items[5], 2);
		HiliteControl(items[5], 255);
	}
	else {
		if(prefs->textureFlags & kTextureFlag_MipMap)
		SetControlValue(items[5], 2);
		else if(prefs->textureFlags & kTextureFlag_ForceMipMap)
		SetControlValue(items[5], 3);
	}
	
	if(prefs->engineFlags & kPref_EngineFlagFiltering)
	SetControlValue(items[6], 1);
	else
	HiliteControl(items[11], 255);
	if(prefs->engineFlags & kPref_EngineFlagVideoFX)
	SetControlValue(items[7], 1);
	if(prefs->engineFlags & kPref_EngineFlag32BitRendering)
	SetControlValue(items[8], 1);
	if(prefs->engineFlags & kPref_EngineFlag32BitZBuffer)
	SetControlValue(items[9], 1);
	if(prefs->engineFlags & kPref_EngineFlagFurtherClipping)
	SetControlValue(items[10], 1);
	if(prefs->engineFlags & kPref_EngineFlagTriFiltering)
	SetControlValue(items[11], 1);
	if(prefs->textureFlags & kTextureFlag_3DfxMode)
	SetControlValue(items[12], 1);
	
	DisposeHandle(screenResList);
}

static void PaneDoClick_3DEngine(DialogPtr dialog, short itemNum, short offset, PreferencesPtr prefs, Handle helpItem)
{
	ControlHandle		control,
						item;
	long				i;
	
	//Get control
	if(GetDialogItemAsControl(dialog, itemNum, &control))
	return;
	
	switch(itemNum - offset) {
		
		case 2:
		case 4:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		i = GetControlValue(control);
		SetControlValue(control, !i);
		if(itemNum - offset == 7) {
			GetDialogItemAsControl(dialog, offset + 12, &item);
			if(i) {
				SetControlValue(item, 0);
				HiliteControl(item, 255);
			}
			else
			HiliteControl(item, 0);
		}
		break;
		
		case 5:
		i = GetControlValue(control);
		GetDialogItemAsControl(dialog, offset + 6, &item);
		if(i == 1)
		HiliteControl(item, 0);
		else {
			SetControlValue(item, 1);
			HiliteControl(item, 255);
		}
		break;
		
	}
}

static void PaneHide_3DEngine(DialogPtr dialog, short offset, PreferencesPtr prefs)
{
	Handle				screenResList;
	Point				*resolution;
	short				nbRes;
	ControlHandle		items[13];
	long				i,
						k;
	TQAEngine			*engine;
	TQADevice			raveDevice;
	
	//Get screen resolutions list
	screenResList = Get1Resource(kScreenResType, kScreenResID);
	if(screenResList == nil)
	return;
	DetachResource(screenResList);
	HLock(screenResList);
	nbRes = *((short*) *screenResList);
	resolution = (Point*) (*screenResList + 2);
	
	//Get items
	for(i = 0; i < 13; ++i)
	GetDialogItemAsControl(dialog, offset + 1 + i, &items[i]);
	
	//Save values
	i = GetControlValue(items[0]);
	if(i == 1)
	prefs->raveEngineName[0] = 0;
	else {
		raveDevice.deviceType = kQADeviceGDevice;
		raveDevice.device.gDevice = mainGDHandle;
		QAEngineEnable(kQAVendor_Apple, kQAEngine_AppleHW); //Enable Apple accelerator card
		engine = QADeviceGetFirstEngine(&raveDevice);
		for(k = 0; k < i - 3; ++k)
		if(engine != nil)
		engine = QADeviceGetNextEngine(&raveDevice, engine);
		
		QAEngineGestalt(engine, kQAGestalt_ASCIINameLength, &k);
		QAEngineGestalt(engine, kQAGestalt_ASCIIName, &prefs->raveEngineName[1]);
		prefs->raveEngineName[0] = k;
	}
	
	prefs->textureFlags = 0;
	prefs->engineFlags = 0;
	
	if(GetControlValue(items[1]))
	prefs->engineFlags |= kPref_EngineFlagUseSoftware;
	
	i = GetControlValue(items[2]);
	prefs->renderWidth = resolution[i - 1].v;
	prefs->renderHeight = resolution[i - 1].h;
	if(GetControlValue(items[3]))
	prefs->engineFlags |= kPref_EngineFlagLowResolution;
	
	switch(GetControlValue(items[4])) {
		case 2: prefs->textureFlags |= kTextureFlag_Reduce4X; break;
		case 3: prefs->textureFlags |= kTextureFlag_Reduce16X; break;
		case 4: prefs->textureFlags |= kTextureFlag_Compress; break;
	}
	
	switch(GetControlValue(items[5])) {
		case 2: prefs->textureFlags |= kTextureFlag_MipMap; break;
		case 3: prefs->textureFlags |= kTextureFlag_ForceMipMap; break;
	}
	
	if(GetControlValue(items[6]))
	prefs->engineFlags |= kPref_EngineFlagFiltering;
	if(GetControlValue(items[7]))
	prefs->engineFlags |= kPref_EngineFlagVideoFX;
	if(GetControlValue(items[8]))
	prefs->engineFlags |= kPref_EngineFlag32BitRendering;
	if(GetControlValue(items[9]))
	prefs->engineFlags |= kPref_EngineFlag32BitZBuffer;
	if(GetControlValue(items[10]))
	prefs->engineFlags |= kPref_EngineFlagFurtherClipping;
	if(GetControlValue(items[11]))
	prefs->engineFlags |= kPref_EngineFlagTriFiltering;
	if(GetControlValue(items[12]))
	prefs->textureFlags |= kTextureFlag_3DfxMode;
	
	//Remove DITL pane
	ShortenDITL(dialog, CountDITL(dialog) - offset);
	
	//Clean up
	DisposeHandle(screenResList);
}

//Sound pane

static void PaneDisplay_Sound(DialogPtr dialog, short offset, short* maxItem, PreferencesPtr prefs)
{
	ControlHandle		items[3];
	long				i;
	
	//Append DITL pane
	AppendDialogItemList(dialog, kSoundDITLID, overlayDITL);
	*maxItem = offset + 3;
	
	//Get items
	for(i = 0; i < 3; ++i)
	GetDialogItemAsControl(dialog, offset + 1 + i, &items[i]);
	
	//Set items
	SetControlValue(items[0], prefs->musicVolume);
	SetControlValue(items[1], prefs->soundFXVolume);
	if(prefs->soundFlags & kPref_SoundFlagGameMusic)
	SetControlValue(items[2], 1);
	
#if 1
	for(i = 0; i < 3; ++i)
	Draw1Control(items[i]);
#endif
}

static void PaneDoClick_Sound(DialogPtr dialog, short itemNum, short offset, PreferencesPtr prefs, Handle helpItem)
{
	ControlHandle		control;
	long				i;
	
	//Get control
	if(GetDialogItemAsControl(dialog, itemNum, &control))
	return;
	
	switch(itemNum - offset) {
		
		case 3:
		i = GetControlValue(control);
		SetControlValue(control, !i);
		break;
		
	}
}

static void PaneHide_Sound(DialogPtr dialog, short offset, PreferencesPtr prefs)
{
	ControlHandle		items[3];
	long				i;
	
	//Get items
	for(i = 0; i < 3; ++i)
	GetDialogItemAsControl(dialog, offset + 1 + i, &items[i]);
	
	//Save values
	prefs->musicVolume = GetControlValue(items[0]);
	prefs->soundFXVolume = GetControlValue(items[1]);
	prefs->soundFlags = 0;
	if(GetControlValue(items[2]))
	prefs->soundFlags |= kPref_SoundFlagGameMusic;
	
	//Remove DITL pane
	ShortenDITL(dialog, CountDITL(dialog) - offset);
}

//Control pane

static void PaneDisplay_Control(DialogPtr dialog, short offset, short* maxItem, PreferencesPtr prefs)
{
	//Append DITL pane
	AppendDialogItemList(dialog, kControlDITLID, overlayDITL);
	*maxItem = offset + 1;
}

static Boolean EventHandlerProc(EventRecord *inEvent)
{
	return false;
}
	
static void PaneDoClick_Control(DialogPtr dialog, short itemNum, short offset, PreferencesPtr prefs, Handle helpItem)
{
	OSErr				theError;
	
	switch(itemNum - offset) {
		
		case 1:
		theError = ISpConfigure((ISpEventProcPtr) EventHandlerProc);
		if(theError)
		Wrapper_Error_Display(145, theError, nil, false);
		break;
		
	}
}

static void PaneHide_Control(DialogPtr dialog, short offset, PreferencesPtr prefs)
{
	//Remove DITL pane
	ShortenDITL(dialog, CountDITL(dialog) - offset);
}

//Network pane

static void PaneDisplay_Network(DialogPtr dialog, short offset, short* maxItem, PreferencesPtr prefs)
{
	ControlHandle		items[5];
	long				i;
	
	//Append DITL pane
	AppendDialogItemList(dialog, kNetworkDITLID, overlayDITL);
	*maxItem = offset + 5;
	
	//Get items
	for(i = 0; i < 5; ++i)
	GetDialogItemAsControl(dialog, offset + 1 + i, &items[i]);
	
	//Set items
	SetDialogItemText((Handle) items[0], prefs->playerName);
	SetDialogItemText((Handle) items[1], prefs->playerLocation);
	SetDialogItemText((Handle) items[2], prefs->hostName);
	SetDialogItemText((Handle) items[3], prefs->hostPassword);
	SetDialogItemText((Handle) items[4], prefs->hostMessage);
	
	SelectDialogItemText(dialog, offset + 1, 0, 32000);
	
#if 1
	for(i = 0; i < 5; ++i)
	Draw1Control(items[i]);
#endif
}

static void PaneDoClick_Network(DialogPtr dialog, short itemNum, short offset, PreferencesPtr prefs, Handle helpItem)
{
	ControlHandle		control;
	
	//Get control
	if(GetDialogItemAsControl(dialog, itemNum, &control))
	return;
	
	switch(itemNum - offset) {
		
		;
		
	}
}

static void PaneHide_Network(DialogPtr dialog, short offset, PreferencesPtr prefs)
{
	ControlHandle		items[5];
	long				i;
	
	//Get items
	for(i = 0; i < 5; ++i)
	GetDialogItemAsControl(dialog, offset + 1 + i, &items[i]);
	
	//Save values
	GetDialogItemText((Handle) items[0], prefs->playerName);
	if(prefs->playerName[0] > kPilot_MaxNameSize)
	prefs->playerName[0] = kPilot_MaxNameSize;
	GetDialogItemText((Handle) items[1], prefs->playerLocation);
	GetDialogItemText((Handle) items[2], prefs->hostName);
	GetDialogItemText((Handle) items[3], prefs->hostPassword);
	GetDialogItemText((Handle) items[4], prefs->hostMessage);
	prefs->networkFlags = 0;
	
	//Remove DITL pane
	ShortenDITL(dialog, CountDITL(dialog) - offset);
}

//Main:

void Preferences_Set(PreferencesPtr prefs)
{
	DialogPtr				theDialog,
							whichDialog;
	short					itemType,
							itemHit = 0,
							itemOver;
	Rect					aRect;
	Handle					tabItem,
							helpItem;
	long					i;
	Str255					helpText;
	short					maxItem,
							lastItem = -1;
	Preferences				tempPrefs;
	RgnHandle				helpRegion = NewRgn();
	EventRecord				theEvent;
	Point					mouse;
	Handle					tempHandle;
	WindowPtr				whichWin;
	VDSwitchInfoRec			interfaceSwitchInfo;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	
	GetGWorld(&savePort, &saveDevice);
	
	//Copy preferences
	BlockMove(prefs, &tempPrefs, sizeof(Preferences));
	
	//Fade in...
#if __SCREEN_FADES__
#if __USE_AUDIO_CD_TRACKS__
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
	AudioCD_SetVolume(thePrefs.musicVolume / 3, thePrefs.musicVolume / 3);
#elif __USE_AUDIO_FILES__
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, QDChannel, thePrefs.musicVolume / 3);
#else
	Fade1ToBlack(mainGDHandle, kGammaSpeed, quadraticFade, nil, 0);
#endif
#endif
	
	//Save current display state
	SL_GetCurrent(mainGDHandle, &interfaceSwitchInfo);
	
	//Restore screen state
	SL_Switch(mainGDHandle, &oldScreenSwitchInfo);
	
	//End configuring displays
	DMEndConfigureDisplays(theDisplayState);
	
	//Set up screen
	SetPort(mainWin);
	PaintRect(&(mainWin->portRect));
	
	//Show dialog
	UseResFile(mainResFileID);
	theDialog = GetNewDialog(kPrefDialogID, nil, (WindowPtr) -1);
	SetPort(theDialog);
	GetDialogItem(theDialog, kTabItemID, &itemType, &tabItem, &aRect);
	GetDialogItem(theDialog, kHelpItemID, &itemType, &helpItem, &aRect);
	RectRgn(helpRegion, &aRect);
	
	//Display tab
	SetControlValue((ControlHandle) tabItem, curPane);
	switch(curPane) {
		
		case k3DEngine:
		PaneDisplay_3DEngine(theDialog, kNumItems, &maxItem, &tempPrefs);
		break;
		
		case kSound:
		PaneDisplay_Sound(theDialog, kNumItems, &maxItem, &tempPrefs);
		break;
		
		case kControl:
		PaneDisplay_Control(theDialog, kNumItems, &maxItem, &tempPrefs);
		break;
		
		case kNetwork:
		PaneDisplay_Network(theDialog, kNumItems, &maxItem, &tempPrefs);
		break;
		
	}
	SetDialogDefaultItem(theDialog, 1);
	
	GetIndString(helpText, kHelpMainResID, 1);
	SetDialogItemText(helpItem, helpText);
	DrawDialog(theDialog);
	
#if __SCREEN_FADES__
	//Fade out...
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	InitCursor();
	
	//Run
	FlushEvents(everyEvent, 0);
	do {
		GetMouse(&mouse);
		itemOver = 0;
		for(i = 1; i <= maxItem; ++i) {
			GetDialogItem(theDialog, i, &itemType, &tempHandle, &aRect);
			if(PtInRect(mouse, &aRect))
			itemOver = i;
		}
		if(itemOver != lastItem) {
			//Set help text
			if(itemOver == 0)
			GetIndString(helpText, kHelpMainResID, 1);
			else if(itemOver <= kNumItems)
			GetIndString(helpText, kHelpMainResID, 1 + itemOver);
			else
			GetIndString(helpText, kHelpMainResID + curPane, itemOver - kNumItems);
			
			//Display text
			SetDialogItemText(helpItem, helpText);
			UpdateControls(theDialog, helpRegion);
			lastItem = itemOver;
		}
		
		//Get dialog events
		if(WaitNextEvent(everyEvent, &theEvent, 20, nil) == false)
		continue;
		
		//Handle dummy update events
		if((theEvent.what == updateEvt) && ((WindowPtr) theEvent.message != theDialog)) {
			BeginUpdate((WindowPtr) theEvent.message);
			EndUpdate((WindowPtr) theEvent.message);
			continue;
		}
		
		//Handle drag window events
		if((theEvent.what == mouseDown) && (FindWindow(theEvent.where, &whichWin) == inDrag)) {
			DragWindow(whichWin, theEvent.where, &(**LMGetGrayRgn()).rgnBBox);
			continue;
		}
		
		//Map special keys to some dialog buttons
		itemHit = -1;
		if(theEvent.what == keyDown) {
			unsigned char theKey = (theEvent.message & keyCodeMask) >> 8;
			
			if((theKey == keyReturn) || (theKey == keyEnter) || (theKey == keyEnterPB)) {
				Press_Button(theDialog, 1);
				itemHit = 1;
			}
			if(theKey == keyEscape) {
				Press_Button(theDialog, 2);
				itemHit = 2;
			}
		}
		
		if(itemHit == -1)
			if(!(IsDialogEvent(&theEvent) && DialogSelect(&theEvent, &whichDialog, &itemHit) && (whichDialog == theDialog)))
			continue;
		
		if(itemHit > kNumItems)
		switch(curPane) {
			
			case k3DEngine:
			PaneDoClick_3DEngine(theDialog, itemHit, kNumItems, &tempPrefs, helpItem);
			break;
			
			case kSound:
			PaneDoClick_Sound(theDialog, itemHit, kNumItems, &tempPrefs, helpItem);
			break;
			
			case kControl:
			PaneDoClick_Control(theDialog, itemHit, kNumItems, &tempPrefs, helpItem);
			break;
			
			case kNetwork:
			PaneDoClick_Network(theDialog, itemHit, kNumItems, &tempPrefs, helpItem);
			break;
			
		}
		else if(itemHit == kTabItemID) {
			//Tab switch
			i = GetControlValue((ControlHandle) tabItem);
			if(i != curPane) {
				switch(curPane) {
					
					case k3DEngine:
					PaneHide_3DEngine(theDialog, kNumItems, &tempPrefs);
					break;
					
					case kSound:
					PaneHide_Sound(theDialog, kNumItems, &tempPrefs);
					break;
					
					case kControl:
					PaneHide_Control(theDialog, kNumItems, &tempPrefs);
					break;
					
					case kNetwork:
					PaneHide_Network(theDialog, kNumItems, &tempPrefs);
					break;
					
				}
				curPane = i;
				switch(curPane) {
					
					case k3DEngine:
					PaneDisplay_3DEngine(theDialog, kNumItems, &maxItem, &tempPrefs);
					break;
					
					case kSound:
					PaneDisplay_Sound(theDialog, kNumItems, &maxItem, &tempPrefs);
					break;
					
					case kControl:
					PaneDisplay_Control(theDialog, kNumItems, &maxItem, &tempPrefs);
					break;
					
					case kNetwork:
					PaneDisplay_Network(theDialog, kNumItems, &maxItem, &tempPrefs);
					break;
					
				}
			}
	
			//Display tab help
			GetIndString(helpText, kHelpMainResID, 2);
			SetDialogItemText(helpItem, helpText);
		}
		
	} while((itemHit != 1) && (itemHit != 2));
	switch(curPane) {
		
		case k3DEngine:
		PaneHide_3DEngine(theDialog, kNumItems, &tempPrefs);
		break;
		
		case kSound:
		PaneHide_Sound(theDialog, kNumItems, &tempPrefs);
		break;
		
		case kControl:
		PaneHide_Control(theDialog, kNumItems, &tempPrefs);
		break;
		
		case kNetwork:
		PaneHide_Network(theDialog, kNumItems, &tempPrefs);
		break;
		
	}
				
	//The user clicked OK => save new prefs
	if(itemHit == 1)
	BlockMove(&tempPrefs, prefs, sizeof(Preferences));
	
	//Clean up
	DisposeDialog(theDialog);
	DisposeRgn(helpRegion);
	HideCursor();
	
	//Start configuring displays
	DMBeginConfigureDisplays(&theDisplayState);
	
	//Restore screen state
	SL_Switch(mainGDHandle, &interfaceSwitchInfo);
	
	SetGWorld(savePort, saveDevice);
}
