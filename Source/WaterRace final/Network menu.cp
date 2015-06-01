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


#include					"WaterRace.h"
#include					"Strings.h"
#include					"Data Files.h"
#include					"Interface.h"
#include					"Preferences.h"
#include					"Network Engine.h"

#include					"Clock.h"

#include					"MetaFile Utils.h"

#if __USE_AUDIO_CD_TRACKS__
#include					"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include					"QDesign Player.h"
#endif

#include					"Keys.h"
#include					"Dialog Utils.h"

//CONSTANTES:

#define						kMainDialogID		1000

#define						kTerminalSize		16384

enum {kMode_NotConnected, kMode_Host, kMode_Join};

//ROUTINES:

static void Terminal_AppendText(Ptr terminal, Str255 text)
{
	BlockMove(&text[1], &terminal[*((short*) terminal) + 2], text[0]);
	*((short*) terminal) += text[0];
}

static OSErr GetTextureSetName(FSSpec* file, short num, Str31 name, OSType* ID)
{
	MetaFile_ReferencePtr	reference;
	OSErr					theError;
	long					index = 0,
							i;
	TextureSetPtr			set;
	
	theError = MetaFile_Read_OpenFile(file, &reference);
	if(theError)
	return theError;
	
	//Low-level scan for texture set tags
	for(i = 0; i < reference->tagTable.tagCount; ++i)
	if(reference->tagTable.tagList[i].tag == kTag_TextureSet) {
		if(index == num) {
			theError = MetaFile_LowLevelRead_DataPtr(reference, i, (Ptr*) &set);
			if(theError == noErr) {
				BlockMove(set->name, name, sizeof(Str31));
				*ID = set->id;
				DisposePtr((Ptr) set);
			}
			break;
		}
		++index;
	}
	
	MetaFile_Read_CloseFile(reference);
	
	return noErr;
}

static void SetDialogItemText(DialogPtr dialog, short itemNum, Str255 text)
{
	Handle				item;
	short				itemType;
	Rect				itemRect;
	
	GetDialogItem(dialog, itemNum, &itemType, &item, &itemRect);
	SetDialogItemText(item, text);
}

OSErr NetworkMenu_Display()
{
	DialogPtr				theDialog,
							whichDialog;
	EventRecord				theEvent;
	WindowPtr				whichWin;
	short					itemHit;
	ControlHandle			items[13];
	OSErr					theError;
	ControlHandle			cancelButton,
							okButton;
	short					currentLocation = 0,
							currentDifficulty = kDifficulty_Easy,
							currentShip = 0,
							currentTextureSet = 0;
	OSType					currentShipID,
							currentLocationID,
							currentTextureSetID;
	FSSpec					currentShipSpec;
	Str31					text;
	short					mode = kMode_NotConnected;
	Ptr						terminal;
	Boolean					connected = false,
							online = false,
							gameStarted = false;
	OSType					event;
	long					eventData;
	long					i;
	VDSwitchInfoRec			interfaceSwitchInfo;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	
	GetGWorld(&savePort, &saveDevice);
	
	//Allocate terminal memory
	terminal = NewPtr(kTerminalSize + sizeof(short));
	if(terminal == nil)
	return MemError();

#if __SCREEN_FADES__
	//Fade in...
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
	theDialog = GetNewDialog(kMainDialogID, nil, (WindowPtr) -1);
	SetPort(theDialog);
	TextFont(3);
	TextFace(0);
	TextSize(9);
	ForeColor(blackColor);
	
	//Setup terminal
	*((short*) terminal) = 0;
	TextBox_StartUp(theDialog, 20, terminal);
	
	GetDialogItemAsControl(theDialog, 1, &okButton);
	GetDialogItemAsControl(theDialog, 2, &cancelButton);
	for(i = 0; i < 13; ++i)
	GetDialogItemAsControl(theDialog, 3 + i, &items[i]);
	
	SetControlValue(items[0], 1);
#if __CD_PROTECTION__ && __NETWORK_ONLY_IF_CD_FAILURE__
	if(networkOnly) {
		HiliteControl(items[0], 255);
		HiliteControl(items[1], 255);
	}
#endif
#if __NETWORK_CLIENT_MODE__
	HiliteControl(items[0], 255);
	HiliteControl(items[1], 255);
#endif
	SetDialogItemText((Handle) items[2], "\p128.128.128.128");
	if(thePrefs.lastNetworkServerIP[0])
	SetDialogItemText((Handle) items[2], thePrefs.lastNetworkServerIP);
	if(thePrefs.lastPort)
	SetDialogItemNum((Handle) items[3], thePrefs.lastPort);
	
	HiliteControl(items[5], 255);
	HiliteControl(items[6], 255);
	HiliteControl(items[7], 255);
	HiliteControl(items[8], 255);
	
	UseResFile(interfaceResFileID);
	
	SetDialogItemText(theDialog, 16, coreData.terrainList[currentLocation].description.realName);
	GetIndString(text, kDifficultyStringID, currentDifficulty);
	SetDialogItemText(theDialog, 17, text);
	SetDialogItemText(theDialog, 18, coreData.shipList[currentShip].description.realName);
	GetTextureSetName(&coreData.shipList[currentShip].fileSpec, currentTextureSet, text, &currentTextureSetID);
	SetDialogItemText(theDialog, 19, text);
	currentShipID = coreData.shipList[currentShip].ID;
	currentShipSpec = coreData.shipList[currentShip].fileSpec;
	currentLocationID = coreData.terrainList[currentLocation].ID;
	
	DrawDialog(theDialog);
	SelectDialogItemText(theDialog, 5, 0, 32000);
	SetDialogDefaultItem(theDialog, 1);
	
#if __SCREEN_FADES__
	//Fade out...
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	InitCursor();
	
	//Run
	FlushEvents(everyEvent, 0);
	do {
		//If we are connected to the server, check for events
		if((mode == kMode_Join) && connected && Client_GetNextEvent(&event, &eventData))
		switch(event) {
			
			case kNetEvent_Client_ServerDisconnection:
			{
				Terminal_AppendText(terminal, kString_Network_Disconnected);
				switch(eventData) {
					case kNetError_Client_BadVersion: Terminal_AppendText(terminal, kString_Network_Reason1); break;
					case kNetError_Client_WrongPassword: Terminal_AppendText(terminal, kString_Network_Reason2); break;
					case kNetError_Client_ServerShutDown: Terminal_AppendText(terminal, kString_Network_Reason3); break;
					case kNetError_Client_GameStarted: Terminal_AppendText(terminal, kString_Network_Reason4); break;
					default: Terminal_AppendText(terminal, kString_Network_Reason5); break;
				}
				DrawDialog(theDialog);
				connected = false;
			}
			break;
			
			case kNetEvent_Client_ReceivedServerInfo:
			{
				ServerInfoPtr			info = (ServerInfoPtr) eventData;
				
				Terminal_AppendText(terminal, kString_Network_Client1);
				Terminal_AppendText(terminal, info->serverName);
				Terminal_AppendText(terminal, kString_Network_Client2);
				Terminal_AppendText(terminal, info->serverLocation);
				Terminal_AppendText(terminal, kString_Network_Client3);
				Terminal_AppendText(terminal, info->serverOS);
				Terminal_AppendText(terminal, kString_Network_Client4);
				Terminal_AppendText(terminal, info->serverProcessor);
				Terminal_AppendText(terminal, kString_Network_Client2);
				Terminal_AppendText(terminal, kString_Network_Client2);
				if(info->serverWelcomeMessage[0]) {
					Terminal_AppendText(terminal, info->serverWelcomeMessage);
					Terminal_AppendText(terminal, kString_Network_Client2);
				}
				Terminal_AppendText(terminal, kString_Network_Client5);
				Terminal_AppendText(terminal, info->terrainName);
				Terminal_AppendText(terminal, "\p (");
				GetIndString(text, kDifficultyStringID, info->terrainDifficulty);
				Terminal_AppendText(terminal, text);
				Terminal_AppendText(terminal, "\p)");
				Terminal_AppendText(terminal, kString_Network_Client6);
				Terminal_AppendText(terminal, kString_Network_Client7);
				DrawDialog(theDialog);
			}
			break;
			
			case kNetEvent_Client_ReceivedGameInfo:
			{
				Terminal_AppendText(terminal, kString_Network_Client8);
				DrawDialog(theDialog);
				gameStarted = true;
			}
			break;
			
		}
		
		//If we are online, check for client events
		if((mode == kMode_Host) && online && Server_GetNextEvent(&event, &eventData)) {
			switch(event) {
				
				case kNetEvent_Server_ServerDisconnection:
				{
					Terminal_AppendText(terminal, kString_Network_Server1);
					DrawDialog(theDialog);
					online = false;
					HiliteControl(okButton, 255);
				}
				break;
				
				case kNetEvent_Server_ClientConnection:
				{
					ClientInfoPtr			info = (ClientInfoPtr) eventData;
				
					Terminal_AppendText(terminal, kString_Network_Server2);
					Terminal_AppendText(terminal, info->playerName);
					Terminal_AppendText(terminal, kString_Network_Server3);
					Terminal_AppendText(terminal,info->playerLocation);
					Terminal_AppendText(terminal, "\p (");
					Server_GetClientIP(info, text);
					Terminal_AppendText(terminal, text);
					Terminal_AppendText(terminal, "\p)\r");
					DrawDialog(theDialog);
				}
				break;
				
				case kNetEvent_Server_ClientDisconnection:
				{
					ClientInfoPtr			info = (ClientInfoPtr) eventData;
				
					Terminal_AppendText(terminal, info->playerName);
					Terminal_AppendText(terminal, kString_Network_Server4);
					DrawDialog(theDialog);
				}
				break;
				
			}
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
			unsigned char	theKey = (theEvent.message & keyCodeMask) >> 8,
							theChar = theEvent.message & charCodeMask;
			
			if((theKey == keyReturn) || (theKey == keyEnter) || (theKey == keyEnterPB)) {
				if(mode != kMode_Join) {
					Press_Button(theDialog, 1);
					itemHit = 1;
				}
			}
			if((theKey == keyEscape) || ((theEvent.modifiers & cmdKey) && (theChar == '.'))) {
				Press_Button(theDialog, 2);
				itemHit = 2;
			}
		}
		
		//Handle mouse events
		if(itemHit == -1)
			if(!(IsDialogEvent(&theEvent) && DialogSelect(&theEvent, &whichDialog, &itemHit) && (whichDialog == theDialog)))
			continue;
		
		switch(itemHit) {
			
			case 1:
			if(mode == kMode_NotConnected) {
				if(GetControlValue(items[0])) { //Join
					GetDialogItemText((Handle) items[2], thePrefs.lastNetworkServerIP);
					GetDialogItemNum((Handle) items[3], &i);
					thePrefs.lastPort = i;
					GetDialogItemText((Handle) items[4], text);
					
					HiliteControl(okButton, 255);
					HiliteControl(cancelButton, 255);
					Terminal_AppendText(terminal, kString_Network_Connecting1);
					Terminal_AppendText(terminal, thePrefs.lastNetworkServerIP);
					Terminal_AppendText(terminal, kString_Network_Connecting2);
					
					//Attempt to connect
					theError = Client_Connect(thePrefs.lastNetworkServerIP, thePrefs.lastPort, text, thePrefs.playerName, thePrefs.playerLocation, currentShipID, currentTextureSetID);
					if(theError) {
						Terminal_AppendText(terminal, kString_Network_Connecting3);
						switch(theError) {
							case kNetError_Client_BadVersion: Terminal_AppendText(terminal, kString_Network_Reason1); break;
							case kNetError_Client_WrongPassword: Terminal_AppendText(terminal, kString_Network_Reason2); break;
							case kNetError_Client_ServerShutDown: Terminal_AppendText(terminal, kString_Network_Reason3); break;
							case kNetError_Client_GameStarted: Terminal_AppendText(terminal, kString_Network_Reason4); break;
							default:
							Terminal_AppendText(terminal, kString_Network_Connecting4);
							Terminal_AppendText(terminal, kString_Network_Connecting7);
							NumToString(theError, text);
							Terminal_AppendText(terminal, text);
							Terminal_AppendText(terminal, "\p)\r\r");
							break;
						}
						HiliteControl(okButton, 0);
					}
					else {
						Terminal_AppendText(terminal, kString_Network_Connecting5);
						Terminal_AppendText(terminal, kString_Network_Connecting6);
						connected = true;
						mode = kMode_Join;
						
						for(i = 0; i < 13; ++i)
						HiliteControl(items[i], 255);
					}
					
					HiliteControl(cancelButton, 0);
					DrawDialog(theDialog);
				}
				else { //Host
					GetDialogItemNum((Handle) items[3], &i);
					thePrefs.lastPort = i;
					
					HiliteControl(okButton, 255);
					HiliteControl(cancelButton, 255);
					Terminal_AppendText(terminal, kString_Network_Hosting1);
					Network_GetComputerIPString(text);
					Terminal_AppendText(terminal, text);
					Terminal_AppendText(terminal, "\p...\r");
					
					//Attempt to start server
					theError = Server_Start(thePrefs.lastPort, thePrefs.hostName, thePrefs.playerLocation, thePrefs.hostPassword, thePrefs.hostMessage, currentLocationID, currentDifficulty);
					if(theError) {
						Terminal_AppendText(terminal, kString_Network_Hosting2);
						NumToString(theError, text);
						Terminal_AppendText(terminal, text);
						Terminal_AppendText(terminal, "\p)\r\r");
					}
					else {
						Terminal_AppendText(terminal, kString_Network_Hosting3);
						Terminal_AppendText(terminal, kString_Network_Hosting4);
						online = true;
						mode = kMode_Host;
						
						for(i = 0; i < 13; ++i)
						HiliteControl(items[i], 255);
#if 1
						theError = Server_ConnectLocalPlayer(thePrefs.playerName, thePrefs.playerLocation, currentShipID, currentTextureSetID);
						if(theError)
						Terminal_AppendText(terminal, kString_Network_Hosting5);
#endif
						
#if __ENABLE_BOTS__
						//Server_ConnectLocalBot('Sara');
#endif
					}
					
					HiliteControl(okButton, 0);
					HiliteControl(cancelButton, 0);
					DrawDialog(theDialog);
				}
			}
			else if(mode == kMode_Host) {
				gameStarted = true;
			}
			break;
			
			case 3:
			SetControlValue(items[0], 1);
			SetControlValue(items[1], 0);
			SetDialogItemText((Handle) items[2], thePrefs.lastNetworkServerIP);
			HiliteControl(items[2], 0);
			SetDialogItemText(theDialog, 7, "\p");
			HiliteControl(items[4], 0);
			HiliteControl(items[5], 255);
			HiliteControl(items[6], 255);
			HiliteControl(items[7], 255);
			HiliteControl(items[8], 255);
			SelectDialogItemText(theDialog, 5, 0, 32000);
			break;
			
			case 4:
			SetControlValue(items[0], 0);
			SetControlValue(items[1], 1);
			Network_GetComputerIPString(text);
			SetDialogItemText((Handle) items[2], text);
			HiliteControl(items[2], 255);
			SetDialogItemText(theDialog, 7, thePrefs.hostPassword);
			HiliteControl(items[4], 255);
			HiliteControl(items[5], 0);
			HiliteControl(items[6], 0);
			HiliteControl(items[7], 0);
			HiliteControl(items[8], 0);
			break;
			
			case 8:
			--currentLocation;
			if(currentLocation < 0)
			currentLocation = 0;
			currentLocationID = coreData.terrainList[currentLocation].ID;
			SetDialogItemText(theDialog, 16, coreData.terrainList[currentLocation].description.realName);
			DrawDialog(theDialog);
			break;
			
			case 9:
			++currentLocation;
			if(currentLocation > coreData.terrainCount)
			currentLocation = coreData.terrainCount;
			if(currentLocation == coreData.terrainCount) {
				currentLocationID = leahTerrainData.ID;
				SetDialogItemText(theDialog, 16, leahTerrainData.description.realName);
			}
			else {
				currentLocationID = coreData.terrainList[currentLocation].ID;
				SetDialogItemText(theDialog, 16, coreData.terrainList[currentLocation].description.realName);
			}
			DrawDialog(theDialog);
			break;
			
			case 10:
			--currentDifficulty;
			if(currentDifficulty < kDifficulty_Easy)
			currentDifficulty = kDifficulty_Easy;
			GetIndString(text, kDifficultyStringID, currentDifficulty);
			SetDialogItemText(theDialog, 17, text);
			DrawDialog(theDialog);
			break;
			
			case 11:
			++currentDifficulty;
			if(currentDifficulty > kDifficulty_Hard)
			currentDifficulty = kDifficulty_Hard;
			GetIndString(text, kDifficultyStringID, currentDifficulty);
			SetDialogItemText(theDialog, 17, text);
			DrawDialog(theDialog);
			break;
			
			case 12:
			--currentShip;
			if(currentShip < 0)
			currentShip = 0;
			currentTextureSet = 0;
			currentShipSpec = coreData.shipList[currentShip].fileSpec;
			currentShipID = coreData.shipList[currentShip].ID;
			SetDialogItemText(theDialog, 18, coreData.shipList[currentShip].description.realName);
			GetTextureSetName(&currentShipSpec, currentTextureSet, text, &currentTextureSetID);
			SetDialogItemText(theDialog, 19, text);
			DrawDialog(theDialog);
			break;
			
			case 13:
			++currentShip;
			if(currentShip > coreData.shipCount)
			currentShip = coreData.shipCount;
			currentTextureSet = 0;
			if(currentShip == coreData.shipCount) {
				currentShipSpec = leahShipData.fileSpec;
				currentShipID = leahShipData.ID;
				SetDialogItemText(theDialog, 18, leahShipData.description.realName);
			}
			else {
				currentShipSpec = coreData.shipList[currentShip].fileSpec;
				currentShipID = coreData.shipList[currentShip].ID;
				SetDialogItemText(theDialog, 18, coreData.shipList[currentShip].description.realName);
			}
			GetTextureSetName(&currentShipSpec, currentTextureSet, text, &currentTextureSetID);
			SetDialogItemText(theDialog, 19, text);
			DrawDialog(theDialog);
			break;
			
			case 14:
			--currentTextureSet;
			if(currentTextureSet < 0)
			currentTextureSet = 0;
			GetTextureSetName(&currentShipSpec, currentTextureSet, text, &currentTextureSetID);
			SetDialogItemText(theDialog, 19, text);
			DrawDialog(theDialog);
			break;
			
			case 15:
			++currentTextureSet;
			if(currentTextureSet > 7)
			currentTextureSet = 7;
			GetTextureSetName(&currentShipSpec, currentTextureSet, text, &currentTextureSetID);
			SetDialogItemText(theDialog, 19, text);
			DrawDialog(theDialog);
			break;
			
		}
	} while((itemHit != 2) && !gameStarted);
	
	DisposeDialog(theDialog);
	DisposePtr(terminal);
	
	//Start configuring displays
	DMBeginConfigureDisplays(&theDisplayState);
	
	//Restore screen state
	SL_Switch(mainGDHandle, &interfaceSwitchInfo);
	
	SetGWorld(savePort, saveDevice);
	
	if(itemHit == 2) {
		switch(mode) {
			
			case kMode_Host:
			Server_ShutDown();
			break;
			
			case kMode_Join:
			Client_Disconnect();
			break;
			
		}
		HideCursor();
		return kError_UserCancel;
	}
	
	HideCursor();
	
	if(mode == kMode_Host)
	return Server_SetupGame();
	
	return Client_SetupGame();
}
