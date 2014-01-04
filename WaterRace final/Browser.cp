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
#include					"Play Modes.h"
#include					"Preferences.h"
#include					"Drivers.h"

#include					"Clock.h"
#include					"Model Animation Utils.h"
#include					"Infinity Player.h"
#include					"Cinematic Utils.h"

#if __SHOW_SPINNING_CURSOR__
#include					"Dialog Utils.h"
#endif

#if __USE_AUDIO_CD_TRACKS__
#include					"Audio CD.h"
#elif __USE_AUDIO_FILES__
#include					"QDesign Player.h"
#endif

#include					"Keys.h"

//CONSTANTES:

#define						kNbTabs					3
#define						kTabOffset				2
#define						kTabStartID				2030
#define						kBackgroundID			2000
#define						kButtonResID			150

#define						kFontDefID				133

#define						kBackStartID			2050
#define						kWaitPicID				2060
#define						kPreviewDisabledPicID	2042
#define						kPracticeDisabledPicID	2045
#define						kRaceDoneEnabledPicID	2070
#define						kRaceDoneDisabledPicID	2071

enum {kBrowserButton_OK = 0, kBrowserButton_Back, kBrowserButton_Report, kBrowserButton_Locations, kBrowserButton_Engines, kBrowserButton_Preview, kBrowserButton_Practice, kBrowserNbButtons};
enum {kPage_Report = 0, kPage_Locations, kPage_Ships, kNbPages};
enum {kBack_Normal = 0, kBack_Selected, kBack_Click, kNbBackImages};

#define						kLocation_X				78
#define						kLocation_Y				134
#define						kLocation_SpaceX		108
#define						kLocation_SpaceY		82
#define						kLocation_MaxPreviews	9
#define						kImageOffset_X			2
#define						kImageOffset_Y			2

#define						kDelay_Turbo			4
#define						kDelay_NoTurbo			5
#define						kShipPreviewScale		0.8

#if __DEMO_MODE__
#define						kFakeTerrainPreviewsID	2001
#define						kFakeShipPreviewsID		2002
#endif
	
//VARIABLES:

static short				currentPage,
							currentShip,
							currentLocation;
static PicHandle			tabPic[kNbTabs],
							backPic[kNbBackImages],
							waitPic,
							previewDisabledPic,
							practiceDisabledPic,
							raceDoneEnabledPic,
							raceDoneDisabledPic;
static Rect					contentRect = {128,74,380,402},
							textRect = {175,430,360,598},
							titleRect = {128,430,148,598};
static Boolean				previewEnabled,
							praticeEnabled;
static short				lastMouseShip,
							lastMouseLocation;
#if __DEMO_MODE__
static PicHandle			fakeTerrainPreviewsPic,
							fakeShipPreviewsPic;
#endif

//ROUTINES:

static void Browser_DrawCurrentTab()
{
	Rect				buttonRect;
	long				i;
	
	//Draw tab
	for(i = 0; i < kNbTabs; ++i) {
		buttonRect = (**buttonsPic[kTabOffset + i][0]).picFrame;
		OffsetRect(&buttonRect, (**buttonsData).buttonList[kTabOffset + i].where.h - buttonRect.left, (**buttonsData).buttonList[kTabOffset + i].where.v - buttonRect.top);
		if(i == currentPage)
		DrawPicture(tabPic[currentPage], &buttonRect);
		else
		CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &buttonRect, &buttonRect, srcCopy, nil);
	}
	
	//Draw preview button
	buttonRect = (**buttonsPic[kBrowserButton_Preview][0]).picFrame;
	OffsetRect(&buttonRect, (**buttonsData).buttonList[kBrowserButton_Preview].where.h - buttonRect.left, (**buttonsData).buttonList[kBrowserButton_Preview].where.v - buttonRect.top);
	if(previewEnabled)
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &buttonRect, &buttonRect, srcCopy, nil);
	else
	DrawPicture(previewDisabledPic, &buttonRect);
	
	//Draw practice button
	buttonRect = (**buttonsPic[kBrowserButton_Practice][0]).picFrame;
	OffsetRect(&buttonRect, (**buttonsData).buttonList[kBrowserButton_Practice].where.h - buttonRect.left, (**buttonsData).buttonList[kBrowserButton_Practice].where.v - buttonRect.top);
	if(praticeEnabled)
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &buttonRect, &buttonRect, srcCopy, nil);
	else
	DrawPicture(practiceDisabledPic, &buttonRect);
}
static void Browser_DrawPilotInfo(PilotPtr pilot)
{
	Point				destPoint;
	PixMapPtr			pixmap = *GetGWorldPixMap(bufferGWorld);
	Str63				text;
	
	//Draw score
	PlayMode_GetStatus(pilot, text);
	TextEngine_Settings_Text(specs[2].fontNum, specs[2].fontSize, specs[2].fontStyle);
	destPoint.h = 514;
	destPoint.v = 98;
	TextEngine_DrawLine_Right((Ptr) &text[1], text[0], specs[2].fontColor, pixmap, destPoint);
	
	//Draw jokers
	if(PlayMode_JokersAvailable(pilot->mode)) {
		NumToString(pilot->numJokers, &text[1]);
		text[0] = text[1] + 1;
		text[1] = 'x';
		TextEngine_Settings_Text(specs[2].fontNum, specs[2].fontSize, specs[2].fontStyle);
		destPoint.h = 542;
		destPoint.v = 98;
		TextEngine_DrawLine_Left((Ptr) &text[1], text[0], specs[2].fontColor, pixmap, destPoint);
	}
}

static OSErr Display_Locations(PilotPtr pilot, DataSetPtr dataSet)
{
	long				i;
	Rect				theRect,
						imageRect;
	PicHandle			daPic;
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	Point				destPoint;
	PixMapPtr			pixmap = *GetGWorldPixMap(bufferGWorld);
	Data_TerrainDescriptionPtr	data;
	
	//Copy background
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &contentRect, &contentRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &titleRect, &titleRect, srcCopy, nil);
	
#if __DEMO_MODE__
	//Draw fake previews
	DrawPicture(fakeTerrainPreviewsPic, &contentRect);
#endif

	//Draw previews
	theRect.left = kLocation_X;
	theRect.top = kLocation_Y;
	for(i = 0; i < dataSet->terrainCount; ++i) {
		if(i == currentLocation)
		daPic = backPic[kBack_Selected];
		else
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &theRect);
		
		if(PlayMode_GetLocationAccess(pilot, i) > 0)
		daPic = dataSet->terrainList[i].enablePreview;
		else
		daPic = dataSet->terrainList[i].disablePreview;
		imageRect.left = theRect.left + kImageOffset_X;
		imageRect.top = theRect.top + kImageOffset_X;
		imageRect.right = theRect.left + kImageOffset_Y + (**daPic).picFrame.right - (**daPic).picFrame.left;
		imageRect.bottom = theRect.top + kImageOffset_Y + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &imageRect);
		
		if(Pilot_GetRaceOnLocation(pilot, dataSet->terrainList[i].ID) != nil) {
			if(PlayMode_GetLocationAccess(pilot, i) > 0)
			daPic = raceDoneEnabledPic;
			else
			daPic = raceDoneDisabledPic;
			
			imageRect.right = theRect.left + kImageOffset_Y + (**daPic).picFrame.right - (**daPic).picFrame.left;
			imageRect.bottom = theRect.top + kImageOffset_Y + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
			DrawPicture(daPic, &imageRect);
		}
		
		theRect.left += kLocation_SpaceX;
		if(theRect.left > 300) {
			theRect.left = kLocation_X;
			theRect.top += kLocation_SpaceY;
		}
	}
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion)) {
		if(i == currentLocation)
		daPic = backPic[kBack_Selected];
		else
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &theRect);
		
		if(PlayMode_GetChallengeLocationAccess(pilot) > 0)
		daPic = leahTerrainData.enablePreview;
		else
		daPic = leahTerrainData.disablePreview;
		imageRect.left = theRect.left + kImageOffset_X;
		imageRect.top = theRect.top + kImageOffset_X;
		imageRect.right = theRect.left + kImageOffset_Y + (**daPic).picFrame.right - (**daPic).picFrame.left;
		imageRect.bottom = theRect.top + kImageOffset_Y + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &imageRect);
		
		if(Pilot_GetRaceOnLocation(pilot, leahTerrainData.ID) != nil) {
			if(PlayMode_GetChallengeLocationAccess(pilot) > 0)
			daPic = raceDoneEnabledPic;
			else
			daPic = raceDoneDisabledPic;
			
			imageRect.right = theRect.left + kImageOffset_Y + (**daPic).picFrame.right - (**daPic).picFrame.left;
			imageRect.bottom = theRect.top + kImageOffset_Y + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
			DrawPicture(daPic, &imageRect);
		}
	}
		
	//Display location data
	if(currentLocation != -1) {
		if(currentLocation < dataSet->terrainCount)
		data = &dataSet->terrainList[currentLocation].description;
		else
		data = &leahTerrainData.description;
		
		//Draw location name
		TextEngine_Settings_Text(specs[0].fontNum, specs[0].fontSize, specs[0].fontStyle);
		destPoint.h = 502;
		destPoint.v = 128;
		TextEngine_DrawLine_Centered((Ptr) &data->realName[1], data->realName[0], specs[0].fontColor, pixmap, destPoint);
	}
	
	//Copy back
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &contentRect, &contentRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &titleRect, &titleRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
	
	return noErr;
}

static void MouseOver_Locations(Point mouse, PilotPtr pilot, DataSetPtr dataSet)
{
	long				i;
	Rect				theRect;
	PicHandle			daPic;
	short				newLocation = -1;
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	Point				destPoint;
	PixMapPtr			pixmap = *GetGWorldPixMap(bufferGWorld);
	Data_TerrainDescriptionPtr	data;
	Str31				text = kString_Interface_RequiredP;
	short				access;
	
	//Find location under mouse
	theRect.left = kLocation_X;
	theRect.top = kLocation_Y;
	for(i = 0; i < dataSet->terrainCount; ++i) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect)) {
			newLocation = i;
			break;
		}
		
		theRect.left += kLocation_SpaceX;
		if(theRect.left > 300) {
			theRect.left = kLocation_X;
			theRect.top += kLocation_SpaceY;
		}
	}
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion)) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect))
		newLocation = i;
	}
	
	if(newLocation == lastMouseLocation)
	return;
	
	//Copy background
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &textRect, &textRect, srcCopy, nil);
	
	if(newLocation != -1) {
		//Get location data
		if(newLocation < dataSet->terrainCount) {
			data = &dataSet->terrainList[newLocation].description;
			access = PlayMode_GetLocationAccess(pilot, newLocation);
		}
		else {
			data = &leahTerrainData.description;
			access = PlayMode_GetChallengeLocationAccess(pilot);
		}
		
		//Display description
		TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
		destPoint.h = 435;
		destPoint.v = 184;
		TextEngine_DrawLine_Left((Ptr) &data->realName[1], data->realName[0], specs[0].fontColor, pixmap, destPoint);
		destPoint.v += TextEngine_GetInterline();
		destPoint.v += TextEngine_GetInterline();
		
		if(access == kAccess_OK)
		TextEngine_DrawText_Left((Ptr) &data->description[1], data->description[0], specs[1].fontColor, pixmap, destPoint);
		else if(access == kAccess_OK_Joker) {
			TextEngine_DrawText_Left((Ptr) kString_Interface_JokerAccess, kString_Interface_JokerAccess_L, specs[0].fontColor, pixmap, destPoint);
			destPoint.v += TextEngine_GetInterline();
#if 0
			destPoint.v += TextEngine_GetInterline();
			TextEngine_DrawText_Left((Ptr) &data->description[1], data->description[0], specs[1].fontColor, pixmap, destPoint);
#else
			TextEngine_DrawText_Left((Ptr) kString_Interface_JokerRedo, kString_Interface_JokerRedo_L, specs[0].fontColor, pixmap, destPoint);
#endif
		}
		else if(access == kAccess_NotEnoughPoints) {
			TextEngine_DrawLine_Left((Ptr) kString_Interface_AccessDenied, kString_Interface_AccessDenied_L, specs[0].fontColor, pixmap, destPoint);
			destPoint.v += TextEngine_GetInterline();
			i = text[0] + 1;
			NumToString(data->points[pilot->difficulty - 1], &text[i]);
			text[0] += text[i] + 1;
			text[i] = ' ';
			TextEngine_DrawLine_Left((Ptr) &text[1], text[0], specs[0].fontColor, pixmap, destPoint);
		}
		else if(access == kAccess_NeedJoker) {
			TextEngine_DrawLine_Left((Ptr) kString_Interface_AccessDenied, kString_Interface_AccessDenied_L, specs[0].fontColor, pixmap, destPoint);
			destPoint.v += TextEngine_GetInterline();
			TextEngine_DrawText_Left((Ptr) kString_Interface_JokerNeed, kString_Interface_JokerNeed_L, specs[0].fontColor, pixmap, destPoint);
		}
		else if(access == kAccess_Disabled)
		TextEngine_DrawLine_Left((Ptr) kString_Interface_AccessDenied, kString_Interface_AccessDenied_L, specs[0].fontColor, pixmap, destPoint);
	}
	
	//Copy back
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &textRect, &textRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
	
	lastMouseLocation = newLocation;
}

static void HandleMouse_Locations(Point mouse, PilotPtr pilot, DataSetPtr dataSet)
{
	long				i;
	Rect				theRect;
	PicHandle			daPic;
	
	theRect.left = kLocation_X;
	theRect.top = kLocation_Y;
	for(i = 0; i < dataSet->terrainCount; ++i) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect) && (PlayMode_GetLocationAccess(pilot, i) > 0)) {
			currentLocation = i;
			Display_Locations(pilot, dataSet);
			while(Button())
			;
			return;
		}
		
		theRect.left += kLocation_SpaceX;
		if(theRect.left > 300) {
			theRect.left = kLocation_X;
			theRect.top += kLocation_SpaceY;
		}
	}
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion)) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect) && (PlayMode_GetChallengeLocationAccess(pilot) > 0)) {
			currentLocation = i;
			Display_Locations(pilot, dataSet);
			while(Button())
			;
			return;
		}
	}
}

static OSErr Display_Ships(PilotPtr pilot, DataSetPtr dataSet)
{
	long				i;
	Rect				theRect,
						imageRect;
	PicHandle			daPic;
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	Data_ShipDescriptionPtr	data;
	Point				destPoint;
	PixMapPtr			pixmap = *GetGWorldPixMap(bufferGWorld);
	
	//Copy background
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &contentRect, &contentRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &titleRect, &titleRect, srcCopy, nil);
	
#if __DEMO_MODE__
	//Draw fake previews
	DrawPicture(fakeShipPreviewsPic, &contentRect);
#endif

	//Draw previews
	theRect.left = kLocation_X;
	theRect.top = kLocation_Y;
	for(i = 0; i < dataSet->shipCount; ++i) {
		if(i == currentShip)
		daPic = backPic[kBack_Selected];
		else
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &theRect);
		
		if(PlayMode_GetShipAccess(pilot, i) > 0)
		daPic = dataSet->shipList[i].enablePreview;
		else
		daPic = dataSet->shipList[i].disablePreview;
		imageRect.left = theRect.left + kImageOffset_X;
		imageRect.top = theRect.top + kImageOffset_X;
		imageRect.right = theRect.left + kImageOffset_Y + (**daPic).picFrame.right - (**daPic).picFrame.left;
		imageRect.bottom = theRect.top + kImageOffset_Y + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &imageRect);
		
		theRect.left += kLocation_SpaceX;
		if(theRect.left > 300) {
			theRect.left = kLocation_X;
			theRect.top += kLocation_SpaceY;
		}
	}
#if !__DISABLE_GBOAT_ACCESS__
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion)) {
		if(i == currentShip)
		daPic = backPic[kBack_Selected];
		else
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &theRect);
		
		if(PlayMode_GetChallengeShipAccess(pilot) > 0)
		daPic = leahShipData.enablePreview;
		else
		daPic = leahShipData.disablePreview;
		imageRect.left = theRect.left + kImageOffset_X;
		imageRect.top = theRect.top + kImageOffset_X;
		imageRect.right = theRect.left + kImageOffset_Y + (**daPic).picFrame.right - (**daPic).picFrame.left;
		imageRect.bottom = theRect.top + kImageOffset_Y + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		DrawPicture(daPic, &imageRect);
	}
#endif
	
	//Display location data
	if(currentShip != -1) {
		if(currentShip < dataSet->shipCount)
		data = &dataSet->shipList[currentShip].description;
		else
		data = &leahShipData.description;
		
		//Draw ship name
		TextEngine_Settings_Text(specs[0].fontNum, specs[0].fontSize, specs[0].fontStyle);
		destPoint.h = 502;
		destPoint.v = 128;
		TextEngine_DrawLine_Centered((Ptr) &data->realName[1], data->realName[0], specs[0].fontColor, pixmap, destPoint);
	}
		
	//Copy back
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &contentRect, &contentRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &titleRect, &titleRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
	
	return noErr;
}

static void MouseOver_Ships(Point mouse, PilotPtr pilot, DataSetPtr dataSet)
{
	long				i;
	Rect				theRect;
	PicHandle			daPic;
	short				newShip = -1;
	CGrafPtr			savePort = NULL;
	GDHandle			saveDevice = NULL;
	Point				destPoint;
	PixMapPtr			pixmap = *GetGWorldPixMap(bufferGWorld);
	Data_ShipDescriptionPtr	data;
	Str31				text = kString_Interface_RequiredP;
	short				access;
	
	//Find location under mouse
	theRect.left = kLocation_X;
	theRect.top = kLocation_Y;
	for(i = 0; i < dataSet->shipCount; ++i) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect)) {
			newShip = i;
			break;
		}
		
		theRect.left += kLocation_SpaceX;
		if(theRect.left > 300) {
			theRect.left = kLocation_X;
			theRect.top += kLocation_SpaceY;
		}
	}
#if !__DISABLE_GBOAT_ACCESS__
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion)) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect))
		newShip = i;
	}
#endif
	
	if(newShip == lastMouseShip)
	return;
	
	//Copy background
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &textRect, &textRect, srcCopy, nil);
	
	if(newShip != -1) {
		//Get ship data
		if(newShip < dataSet->shipCount) {
			data = &dataSet->shipList[newShip].description;
			access = PlayMode_GetShipAccess(pilot, newShip);
		}
		else {
			data = &leahShipData.description;
			access = PlayMode_GetChallengeShipAccess(pilot);
		}
		
		//Display description
		TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
		destPoint.h = 435;
		destPoint.v = 184;
		TextEngine_DrawLine_Left((Ptr) &data->realName[1], data->realName[0], specs[0].fontColor, pixmap, destPoint);
		destPoint.v += TextEngine_GetInterline();
		destPoint.v += TextEngine_GetInterline();

		if(access > 0)
		TextEngine_DrawText_Left((Ptr) &data->description[1], data->description[0], specs[1].fontColor, pixmap, destPoint);
		else if(access == kAccess_NotEnoughPoints) {
			TextEngine_DrawLine_Left((Ptr) kString_Interface_AccessDenied, kString_Interface_AccessDenied_L, specs[0].fontColor, pixmap, destPoint);
			destPoint.v += TextEngine_GetInterline();
			i = text[0] + 1;
			NumToString(data->points[pilot->difficulty - 1], &text[i]);
			text[0] += text[i] + 1;
			text[i] = ' ';
			TextEngine_DrawLine_Left((Ptr) &text[1], text[0], specs[0].fontColor, pixmap, destPoint);
		}
		else if(access == kAccess_Disabled)
		TextEngine_DrawLine_Left((Ptr) kString_Interface_AccessDenied, kString_Interface_AccessDenied_L, specs[0].fontColor, pixmap, destPoint);
	}
	
	//Copy back
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &textRect, &textRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
	
	lastMouseShip = newShip;
}

static void HandleMouse_Ships(Point mouse, PilotPtr pilot, DataSetPtr dataSet)
{
	long				i;
	Rect				theRect;
	PicHandle			daPic;
	
	theRect.left = kLocation_X;
	theRect.top = kLocation_Y;
	for(i = 0; i < dataSet->shipCount; ++i) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect) && (PlayMode_GetShipAccess(pilot, i) > 0)) {
			currentShip = i;
			Display_Ships(pilot, dataSet);
			while(Button())
			;
			return;
		}
		
		theRect.left += kLocation_SpaceX;
		if(theRect.left > 300) {
			theRect.left = kLocation_X;
			theRect.top += kLocation_SpaceY;
		}
	}
#if !__DISABLE_GBOAT_ACCESS__
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion)) {
		daPic = backPic[kBack_Normal];
		theRect.right = theRect.left + (**daPic).picFrame.right - (**daPic).picFrame.left;
		theRect.bottom = theRect.top + (**daPic).picFrame.bottom - (**daPic).picFrame.top;
		
		if(PtInRect(mouse, &theRect) && (PlayMode_GetChallengeShipAccess(pilot) > 0)) {
			currentShip = i;
			Display_Ships(pilot, dataSet);
			while(Button())
			;
			return;
		}
	}
#endif
}

static OSErr Display_Report(PilotPtr pilot, DataSetPtr dataSet)
{
	Rect					imageRect;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PixMapPtr				pixmap = *GetGWorldPixMap(bufferGWorld);
	CharacterPreloadedDataPtr	characterData;
	Point					destPoint;
	long					i;
	Str31					text1,
							text2,
							text3;
	Data_ShipDescriptionPtr	dataShip = &dataSet->shipList[currentShip].description;
	long					count = 0;
	
	//Find local player character
	characterData = GetCharacterData_ByID(&coreData, pilot->character);
	if(characterData == nil)
	characterData = GetCharacterData_ByID(&addOnData, pilot->character);
	if(characterData == nil)
	return kError_CharacterNotFound;
	
	//Copy background
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &contentRect, &contentRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &textRect, &textRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &titleRect, &titleRect, srcCopy, nil);
	
	//Draw character name
	TextEngine_Settings_Text(specs[0].fontNum, specs[0].fontSize, specs[0].fontStyle);
	destPoint.h = 502;
	destPoint.v = 128;
	TextEngine_DrawLine_Centered((Ptr) &characterData->description.realName[1], characterData->description.realName[0], specs[0].fontColor, pixmap, destPoint);
	
	//Display picture
	imageRect = (**characterData->enablePreview).picFrame;
	OffsetRect(&imageRect, 78 - imageRect.left, 132 - imageRect.top);
	DrawPicture(characterData->enablePreview, &imageRect);
	
	//Display description
	TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
	destPoint.h = 170;
	destPoint.v = 132;
	TextEngine_DrawText_Left((Ptr) &characterData->description.description[1], characterData->description.description[0], specs[1].fontColor, pixmap, destPoint);
	
	//Display last 8 races
	PlayMode_GetHistoricEntry(pilot, -1, text1, text2, text3);
	destPoint.v = 230;
	destPoint.h = 80;
	TextEngine_DrawLine_Left((Ptr) &text1[1], text1[0], specs[0].fontColor, pixmap, destPoint);
	destPoint.h = 200;
	TextEngine_DrawLine_Left((Ptr) &text2[1], text2[0], specs[0].fontColor, pixmap, destPoint);
	destPoint.h = 300;
	TextEngine_DrawLine_Left((Ptr) &text3[1], text3[0], specs[0].fontColor, pixmap, destPoint);
	
	destPoint.v = 250;
	for(i = 0; i < iMin(8, pilot->raceCount); ++i)
	if(PlayMode_GetHistoricEntry(pilot, i, text1, text2, text3)) {
		destPoint.h = 80;
		TextEngine_DrawLine_Left((Ptr) &text1[1], text1[0], specs[1].fontColor, pixmap, destPoint);
		destPoint.h = 200;
		TextEngine_DrawLine_Left((Ptr) &text2[1], text2[0], specs[1].fontColor, pixmap, destPoint);
		destPoint.h = 300;
		TextEngine_DrawLine_Left((Ptr) &text3[1], text3[0], specs[1].fontColor, pixmap, destPoint);
		
		destPoint.v += 16;
	}
	
#if 0
	//Draw current race settings
	if(currentLocation != -1) {
		dataLocation = &dataSet->terrainList[currentLocation].description;
		
		TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
		destPoint.h = 435;
		destPoint.v = 184;
		TextEngine_DrawLine_Left((Ptr) "Next race settings:", 19, specs[0].fontColor, pixmap, destPoint);
		destPoint.v += TextEngine_GetInterline();
		destPoint.v += TextEngine_GetInterline();
		TextEngine_DrawLine_Left((Ptr) "Location:", 9, specs[0].fontColor, pixmap, destPoint);
		destPoint.v += TextEngine_GetInterline();
		TextEngine_DrawLine_Left((Ptr) &dataLocation->realName[1], dataLocation->realName[0], specs[1].fontColor, pixmap, destPoint);
		destPoint.v += TextEngine_GetInterline();
		destPoint.v += TextEngine_GetInterline();
		TextEngine_DrawLine_Left((Ptr) "Engine:", 7, specs[0].fontColor, pixmap, destPoint);
		destPoint.v += TextEngine_GetInterline();
		TextEngine_DrawLine_Left((Ptr) &dataShip->realName[1], dataShip->realName[0], specs[1].fontColor, pixmap, destPoint);
	}
	else {
		TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
		destPoint.h = 435;
		destPoint.v = 184;
		TextEngine_DrawLine_Left((Ptr) "GAME OVER!", 10, specs[0].fontColor, pixmap, destPoint);
	}
#endif
	
	//Copy back
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &contentRect, &contentRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &textRect, &textRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &titleRect, &titleRect, srcCopy, nil);
	
	SetGWorld(savePort, saveDevice);
	
	return noErr;
}

#if 0
static Boolean CheckForUserStop()
{
	KeyMap				theKeys;
	
	if(Button())
	return true;
	
	GetKeys(theKeys);
	theKeys[1] &= ~(1 << 1); //Mask the Caps Lock key
	if(theKeys[0] || theKeys[1] || theKeys[2] || theKeys[3])
	return true;
	
	return false;
}
#else
#define	CheckForUserStop()	Button()
#endif

static OSErr Display_TerrainPreview(PilotPtr pilot, DataSetPtr dataSet)
{
	Rect				imageRect;
	Point				size,
						offset,
						destPoint;
	OSErr				theError;
	PixMapPtr			pixmap = *GetGWorldPixMap(bufferGWorld);
	TerrainPreloadedDataPtr	terrainData;
	
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion))
	terrainData = &leahTerrainData;
	else
	terrainData = &dataSet->terrainList[currentLocation];
	
	//Copy background
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &contentRect, &contentRect, srcCopy, nil);
	
	//Display description
	TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
	destPoint.h = 435;
	destPoint.v = 184;
	TextEngine_DrawLine_Left((Ptr) &terrainData->description.realName[1], terrainData->description.realName[0], specs[0].fontColor, pixmap, destPoint);
	destPoint.v += TextEngine_GetInterline();
	destPoint.v += TextEngine_GetInterline();
	TextEngine_DrawText_Left((Ptr) &terrainData->description.notes[1], terrainData->description.notes[0], specs[1].fontColor, pixmap, destPoint);	
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &textRect, &textRect, srcCopy, nil);
	
	//Start music
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_Stop();
#elif __USE_AUDIO_FILES__
	QDPlayer_Stop();
#endif
	GetIndString(musicsFolder.name, MusicFileNameResID, browserMusic);
#if __USE_AUDIO_CD_TRACKS__
	theError = AudioCD_PlayAudioTrack_Wrapper(&musicsFolder);
#elif __USE_AUDIO_FILES__
	theError = QDPlayer_Start(&musicsFolder);
#endif
	if(theError)
	Wrapper_Error_Display(143, theError, nil, false);
#endif
	
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#else
	HideCursor();
#endif
	
	size.h = 312;
	size.v = 178;
	offset.h = mainWinOffset.h + 80;
	offset.v = mainWinOffset.v + 165;
	
	imageRect.left = 78;
	imageRect.top = 132;
	imageRect.right = imageRect.left + (**waitPic).picFrame.right - (**waitPic).picFrame.left;
	imageRect.bottom = imageRect.top + (**waitPic).picFrame.bottom - (**waitPic).picFrame.top;
	DrawPicture(waitPic, &imageRect);
	
	//Setup fake game mode
	gameConfig.gameMode = kGameMode_Demo;
	gameConfig.gameSubMode = kGameSubMode_None;
	gameConfig.gameMode_save = gameConfig.gameMode;
	gameConfig.gameSubMode_save = gameConfig.gameSubMode;
	gameConfig.startTime = 0;
	
	//Init engine
	theError = Infinity_GameInit(Get_InterfaceEngine(), &worldState, size, offset);
	if(theError) {
#if __SHOW_SPINNING_CURSOR__
		SpinningCursor_Stop();
#endif
		Wrapper_Error_Display(146, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	theError = World_Load(worldState, &terrainData->fileSpec, pilot->difficulty, kNoID);
	if(theError) {
#if __SHOW_SPINNING_CURSOR__
		SpinningCursor_Stop();
#endif
		Wrapper_Error_Display(147, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	RClock_SetTime(0);
	World_InstallAudio();
	World_Prepare();
	
	//Setup camera
	DriverCamera_FlyThrough_Init();
	
	//ACTION!
	while(!CheckForUserStop()) {
		//Get time
		worldState->lastFrameTime = worldState->frameTime;
		worldState->frameTime = RClock_GetTime();
		
		//Run scripts
		Anim_Run(&theTerrain, worldState->frameTime);
		
		//Animate sea
		World_UpdateSea();
		
		//Drive camera
		DriverCamera_FlyThrough_Callback();
		
		Draw_Frame(worldState);
	}
	
	//Clean up
	World_Unload();
	
	//Wait for mouse up
	while(Button())
	;

#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
#else
	InitCursor();
#endif
	
#if __USE_AUDIO_CD_TRACKS__ || __USE_AUDIO_FILES__
	//Restore normal music
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_Stop();
#elif __USE_AUDIO_FILES__
	QDPlayer_Stop();
#endif
	GetIndString(musicsFolder.name, MusicFileNameResID, interfaceMusic);
#if __USE_AUDIO_CD_TRACKS__
	theError = AudioCD_PlayAudioTrack_Wrapper(&musicsFolder);
#elif __USE_AUDIO_FILES__
	theError = QDPlayer_Start(&musicsFolder);
#endif
	if(theError)
	Wrapper_Error_Display(143, theError, nil, false);
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_SetVolume(thePrefs.musicVolume / 3, thePrefs.musicVolume / 3);
#elif __USE_AUDIO_FILES__
	QDPlayer_Volume(thePrefs.musicVolume / 3);
#endif
#endif
	
	return noErr;
}

static OSErr Display_ShipPreview(PilotPtr pilot, DataSetPtr dataSet)
{
	Rect					imageRect;
	OSErr					theError;
	PixMapPtr				pixmap = *GetGWorldPixMap(bufferGWorld);
	ShipPreloadedDataPtr	shipData;
	MegaObjectPtr			megaObject;
	long					i;
	unsigned long			startTime;
	CharacterPreloadedDataPtr	characterData;
	Point					destPoint;
	
#if !__DISABLE_GBOAT_ACCESS__
	if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion))
	shipData = &leahShipData;
	else
#endif
	shipData = &dataSet->shipList[currentShip];
	
	//Find local player character
	characterData = GetCharacterData_ByID(&coreData, pilot->character);
	if(characterData == nil)
	characterData = GetCharacterData_ByID(&addOnData, pilot->character);
	if(characterData == nil)
	return kError_CharacterNotFound;
	
	//Copy background
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(bufferGWorld), &contentRect, &contentRect, srcCopy, nil);
	CopyBits(GWBitMapPtr(bufferGWorld2), GWBitMapPtr(mainWin), &contentRect, &contentRect, srcCopy, nil);
	
	imageRect.left = 78;
	imageRect.top = 132;
	imageRect.right = imageRect.left + (**waitPic).picFrame.right - (**waitPic).picFrame.left;
	imageRect.bottom = imageRect.top + (**waitPic).picFrame.bottom - (**waitPic).picFrame.top;
	DrawPicture(waitPic, &imageRect);
	
	//Display description
	TextEngine_Settings_Text(specs[1].fontNum, specs[1].fontSize, specs[1].fontStyle);
	destPoint.h = 435;
	destPoint.v = 184;
	TextEngine_DrawLine_Left((Ptr) &shipData->description.realName[1], shipData->description.realName[0], specs[0].fontColor, pixmap, destPoint);
	destPoint.v += TextEngine_GetInterline();
	destPoint.v += TextEngine_GetInterline();
	TextEngine_DrawText_Left((Ptr) &shipData->description.notes[1], shipData->description.notes[0], specs[1].fontColor, pixmap, destPoint);	
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &textRect, &textRect, srcCopy, nil);
	
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Start();
#else
	HideCursor();
#endif
	
	//Init engine
	theError = Interface_LoadAnimationWrapper(&shipData->fileSpec, characterData->description.textureSetID, 80, 165, 312, 178, kCameraShipPreviewID);
	if(theError) {
#if __SHOW_SPINNING_CURSOR__
		SpinningCursor_Stop();
#endif
		Wrapper_Error_Display(148, theError, nil, __FORCE_QUIT_ON_ERROR__);
		return theError;
	}
	
	//Remove flame shapes
	megaObject = InfinityPlayer_ReturnMegaObject();
	for(i = 0; i < megaObject->object.shapeCount; ++i)
	if((megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_1) || (megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_2))
	Shape_DisableRendering(megaObject->object.shapeList[i]);
	
	//ACTION!
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_SetVolume(thePrefs.musicVolume, thePrefs.musicVolume);
#elif __USE_AUDIO_FILES__
	QDPlayer_Volume(thePrefs.musicVolume);
#endif
	RClock_SetTime(0);
	
	//Display turbo animations if any
	if((Script_GetPtrFromID(megaObject, kScriptID_Turbo) != nil) && (Script_GetPtrFromID(megaObject, kScriptID_TurboReverse) != nil))
	while(1) {
		startTime = RClock_GetTime();
		while(RClock_GetTime() < startTime + kDelay_NoTurbo * kTimeUnit) {
			InfinityPlayer_SetModelMatrix(0.0, 0.0, 0.0, 0.0, kPi / 2.0 * (float) RClock_GetTime() / (float) kTimeUnit, 0.0, kShipPreviewScale);
			InfinityPlayer_RenderFrame();
			if(CheckForUserStop())
			goto End;
		}
		
		InfinityPlayer_PlayAnimation(kScriptID_Turbo);
		while(!InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
			InfinityPlayer_SetModelMatrix(0.0, 0.0, 0.0, 0.0, kPi / 2.0 * (float) RClock_GetTime() / (float) kTimeUnit, 0.0, kShipPreviewScale);
			if(CheckForUserStop())
			goto End;
		}
		
		for(i = 0; i < megaObject->object.shapeCount; ++i)
		if((megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_1) || (megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_2))
		Shape_ResolveRendering(megaObject->object.shapeList[i]);
	
		startTime = RClock_GetTime();
		while(RClock_GetTime() < startTime + kDelay_Turbo * kTimeUnit) {
			InfinityPlayer_SetModelMatrix(0.0, 0.0, 0.0, 0.0, kPi / 2.0 * (float) RClock_GetTime() / (float) kTimeUnit, 0.0, kShipPreviewScale);
			InfinityPlayer_RenderFrame();
			if(CheckForUserStop())
			goto End;
		}
		
		for(i = 0; i < megaObject->object.shapeCount; ++i)
		if((megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_1) || (megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_2))
		Shape_DisableRendering(megaObject->object.shapeList[i]);
	
		InfinityPlayer_PlayAnimation(kScriptID_TurboReverse);
		while(!InfinityPlayer_RunAsync(kPlayer_DefaultMaxFPS)) {
			InfinityPlayer_SetModelMatrix(0.0, 0.0, 0.0, 0.0, kPi / 2.0 * (float) RClock_GetTime() / (float) kTimeUnit, 0.0, kShipPreviewScale);
			if(CheckForUserStop())
			goto End;
		}
	}
	else
	while(1) {
		startTime = RClock_GetTime();
		while(RClock_GetTime() < startTime + kDelay_NoTurbo * kTimeUnit) {
			InfinityPlayer_SetModelMatrix(0.0, 0.0, 0.0, 0.0, kPi / 2.0 * (float) RClock_GetTime() / (float) kTimeUnit, 0.0, kShipPreviewScale);
			InfinityPlayer_RenderFrame();
			if(CheckForUserStop())
			goto End;
		}
		
		for(i = 0; i < megaObject->object.shapeCount; ++i)
		if((megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_1) || (megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_2))
		Shape_ResolveRendering(megaObject->object.shapeList[i]);
	
		startTime = RClock_GetTime();
		while(RClock_GetTime() < startTime + kDelay_Turbo * kTimeUnit) {
			InfinityPlayer_SetModelMatrix(0.0, 0.0, 0.0, 0.0, kPi / 2.0 * (float) RClock_GetTime() / (float) kTimeUnit, 0.0, kShipPreviewScale);
			InfinityPlayer_RenderFrame();
			if(CheckForUserStop())
			goto End;
		}
		
		for(i = 0; i < megaObject->object.shapeCount; ++i)
		if((megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_1) || (megaObject->object.shapeList[i]->id == kShapeID_TurboFlame_2))
		Shape_DisableRendering(megaObject->object.shapeList[i]);
	}
	
End:	
	
	//Clean up
	InfinityPlayer_UnloadAnimation();
	
	//Wait for mouse up
	while(Button())
	;
	
#if __SHOW_SPINNING_CURSOR__
	SpinningCursor_Stop();
#else
	InitCursor();
#endif
	
#if __USE_AUDIO_CD_TRACKS__
	AudioCD_SetVolume(thePrefs.musicVolume / 3, thePrefs.musicVolume / 3);
#elif __USE_AUDIO_FILES__
	QDPlayer_Volume(thePrefs.musicVolume / 3);
#endif
	
	return noErr;
}

static OSErr Browser_DisplayPage(PilotPtr pilot, DataSetPtr dataSet)
{
	OSErr			theError;
	
	switch(currentPage) {
	
		case kPage_Report:
		previewEnabled = false;
		praticeEnabled = false;
		theError = Display_Report(pilot, dataSet);
		break;
		
		case kPage_Locations:
		previewEnabled = true;
		praticeEnabled = false;
		lastMouseLocation = -2;
		theError = Display_Locations(pilot, dataSet);
		break;
		
		case kPage_Ships:
		previewEnabled = true;
		praticeEnabled = true;
		lastMouseShip = -2;
		theError = Display_Ships(pilot, dataSet);
		break;
		
	}
	
	Browser_DrawCurrentTab();
	
	return theError;
}

static OSErr Browser_Start(PilotPtr pilot, DataSetPtr dataSet)
{
	Rect					theRect;
	long					i;
	CGrafPtr				savePort = NULL;
	GDHandle				saveDevice = NULL;
	PicHandle				backgroundPic;
	Boolean					started = false;
	Handle					fontSpecHandle;
	TE_ResFontDefinitionPtr	fontDef;
	
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
	UseResFile(interfaceResFileID);
	
	//Clean windows
	SetPort(backgroundWin);
	PaintRect(&(backgroundWin->portRect));
	SetPort(mainWin);
	PaintRect(&(mainWin->portRect));
	
	//Load buttons data
	buttonsData = (ButtonListHandle) Get1Resource(kButtonsResType, kButtonResID);
	if(buttonsData == nil)
	return ResError();
	DetachResource((Handle) buttonsData);
	HLock((Handle) buttonsData);
	if((**buttonsData).buttonCount != kBrowserNbButtons)
	return kError_FatalError;
	
	//Load graphics
	backgroundPic = (PicHandle) Get1Resource(kPICTResType, kBackgroundID);
	if(backgroundPic == nil)
	return ResError();
	DetachResource((Handle) backgroundPic);
	for(i = 0; i < kBrowserNbButtons; ++i) {
		buttonsPic[i][0] = (PicHandle) Get1Resource(kPICTResType, (**buttonsData).buttonList[i].overID);
		if(buttonsPic[i][0] == nil)
		return ResError();
		DetachResource((Handle) buttonsPic[i][0]);
		
		buttonsPic[i][1] = (PicHandle) Get1Resource(kPICTResType, (**buttonsData).buttonList[i].downID);
		if(buttonsPic[i][1] == nil)
		return ResError();
		DetachResource((Handle) buttonsPic[i][1]);
	}
	
	//Load tab graphics
	for(i = 0; i < kNbTabs; ++i) {
		tabPic[i] = (PicHandle) Get1Resource(kPICTResType, kTabStartID + i);
		if(tabPic[i] == nil)
		return ResError();
		DetachResource((Handle) tabPic[i]);
	}
	
	//Load back graphics
	for(i = 0; i < kNbBackImages; ++i) {
		backPic[i] = (PicHandle) Get1Resource(kPICTResType, kBackStartID + i);
		if(backPic[i] == nil)
		return ResError();
		DetachResource((Handle) backPic[i]);
	}
	
	//Load wait graphics
	waitPic = (PicHandle) Get1Resource(kPICTResType, kWaitPicID);
	if(waitPic == nil)
	return ResError();
	DetachResource((Handle) waitPic);
	
	//Load race done graphics
	raceDoneEnabledPic = (PicHandle) Get1Resource(kPICTResType, kRaceDoneEnabledPicID);
	if(raceDoneEnabledPic == nil)
	return ResError();
	DetachResource((Handle) raceDoneEnabledPic);
	
	//Load race done graphics
	raceDoneDisabledPic = (PicHandle) Get1Resource(kPICTResType, kRaceDoneDisabledPicID);
	if(raceDoneDisabledPic == nil)
	return ResError();
	DetachResource((Handle) raceDoneDisabledPic);
	
	//Load preview disabled graphic
	previewDisabledPic = (PicHandle) Get1Resource(kPICTResType, kPreviewDisabledPicID);
	if(previewDisabledPic == nil)
	return ResError();
	DetachResource((Handle) previewDisabledPic);
	
	//Load practice disabled graphic
	practiceDisabledPic = (PicHandle) Get1Resource(kPICTResType, kPracticeDisabledPicID);
	if(practiceDisabledPic == nil)
	return ResError();
	DetachResource((Handle) practiceDisabledPic);
	
#if __DEMO_MODE__
	//Load demo specific graphics
	fakeTerrainPreviewsPic = (PicHandle) Get1Resource(kPICTResType, kFakeTerrainPreviewsID);
	if(fakeTerrainPreviewsPic == nil)
	return ResError();
	DetachResource((Handle) fakeTerrainPreviewsPic);
	
	fakeShipPreviewsPic = (PicHandle) Get1Resource(kPICTResType, kFakeShipPreviewsID);
	if(fakeShipPreviewsPic == nil)
	return ResError();
	DetachResource((Handle) fakeShipPreviewsPic);
#endif

	//Draw background
	GetGWorld(&savePort, &saveDevice);
	SetGWorld(bufferGWorld, NULL);
	theRect = (**backgroundPic).picFrame;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	DrawPicture(backgroundPic, &theRect);
	DisposeHandle((Handle) backgroundPic);
	SetGWorld(savePort, saveDevice);
	SetRect(&theRect, 0, 0, kInterface_Width, kInterface_Height);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(bufferGWorld2), &theRect, &theRect, srcCopy, nil);
	
	//Create text fonts
	fontSpecHandle = Get1Resource(kTE_FontResSpecID, kFontDefID);
	if(fontSpecHandle == nil)
	return ResError();
	DetachResource(fontSpecHandle);
	HLock(fontSpecHandle);
	fontDef = (TE_ResFontDefinitionPtr) *fontSpecHandle;
	
	specs = (TE_FontSpecPtr) NewPtr(fontDef->fontSpecCount * sizeof(TE_FontSpec));
	for(i = 0; i < fontDef->fontSpecCount; ++i) {
		GetFNum(fontDef->fontSpecList[i].fontName, &specs[i].fontNum);
		if(specs[i].fontNum == 0)
		specs[i].fontNum = fontDef->fontSpecList[i].fontID;
		specs[i].fontSize = fontDef->fontSpecList[i].fontSize;
		specs[i].fontStyle = fontDef->fontSpecList[i].fontStyle;
		specs[i].fontColor = TextEngine_Color32To16(fontDef->fontSpecList[i].colorRed, fontDef->fontSpecList[i].colorGreen, fontDef->fontSpecList[i].colorBlue);
	}
	DisposeHandle(fontSpecHandle);
	
	//Turn font smoothing off
	MacOSTextAliasing_Disable();
	TextEngine_Settings_Clipping(0, kInterface_Height);
	
	//Draw pilot info
	Browser_DrawPilotInfo(pilot);
	CopyBits(GWBitMapPtr(bufferGWorld), GWBitMapPtr(mainWin), &theRect, &theRect, srcCopy, nil);
	
	//Draw current tab
	currentPage = kPage_Report;
	lastMouseShip = -2;
	lastMouseLocation = -2;
	switch(pilot->status) {
		
		case kStatus_Dead:
		case kStatus_Champion:
		currentLocation = -1;
		currentShip = -1;
		break;
		
		case kStatus_Alive:
		if((currentLocation == -1) || (PlayMode_GetLocationAccess(pilot, currentLocation) < 0)) {
			currentLocation = -1;
			for(i = 0; i < dataSet->terrainCount; ++i) {
				if(PlayMode_GetLocationAccess(pilot, i) > 0) {
					currentLocation = i;
					break;
				}
			}
		}
		if((currentShip == -1) || (PlayMode_GetShipAccess(pilot, currentShip) < 0)) {
			currentShip = -1;
			for(i = 0; i < dataSet->shipCount; ++i) {
				if(PlayMode_GetShipAccess(pilot, i) > 0) {
					currentShip = i;
					break;
				}
			}
		}
		break;
		
		case kStatus_Challenge:
		currentLocation = -1;
		if(PlayMode_GetChallengeLocationAccess(pilot) > 0)
		currentLocation = dataSet->terrainCount;
#if __DISABLE_GBOAT_ACCESS__
		if((currentShip == -1) || (PlayMode_GetShipAccess(pilot, currentShip) < 0)) {
			currentShip = -1;
			for(i = 0; i < dataSet->shipCount; ++i) {
				if(PlayMode_GetShipAccess(pilot, i) > 0) {
					currentShip = i;
					break;
				}
			}
		}
#else
		currentShip = -1;
		if(PlayMode_GetChallengeShipAccess(pilot) > 0)
		currentShip = dataSet->shipCount;
#endif
		break;
		
	}
	Browser_DisplayPage(pilot, dataSet);
	
#if __SCREEN_FADES__
	//Fade out...
	Sound_PlaySFx(kSFxFadeOut);
	Fade1ToGamma(mainGDHandle, mainGamma, kGammaSpeed, quadraticFade, nil, 0);
#endif
	
	return noErr;
}

static Boolean Browser_Run(OSType* gameMode, PilotPtr pilot, OSType* terrainID, OSType* shipID, DataSetPtr dataSet)
{
	Boolean					run = true,
							over;
	long					i,
							startTime;
	short					overButton = kButton_None,
							button;
	Point					mouse;
	EventRecord				theEvent;
	unsigned char			theKey;
	short					oldPage = currentPage;
	Boolean					startGame = false;
	
	SetPort(mainWin);
	InitCursor();
	while(run) {
		GetMouse(&mouse);
		
		//Handle mouse over
		if(currentPage == kPage_Locations)
		MouseOver_Locations(mouse, pilot, dataSet);
		else if(currentPage == kPage_Ships)
		MouseOver_Ships(mouse, pilot, dataSet);
		
		//Are we over a button?
		button = kButton_None;
		for(i = 0; i < kBrowserNbButtons; ++i)
		if(PtInRect(mouse, &(**buttonsData).buttonList[i].hotZone))
		button = i;
		
		//Are we over our current tab button?
		if(button == (kTabOffset + currentPage))
		button = kButton_None;
		
		//Are we over preview button & preview is disabled?
		if((button == kBrowserButton_Preview) && !previewEnabled)
		button = kButton_None;
		
		//Are we over practice button & practice is disabled?
		if((button == kBrowserButton_Practice) && !praticeEnabled)
		button = kButton_None;
		
		if((button == kButton_None) && (overButton != kButton_None))
		Interface_ButtonDraw(overButton, kNormal);
		else if((button != kButton_None) && (overButton != button)) {
			if(overButton != kButton_None)
			Interface_ButtonDraw(overButton, kNormal);
			Interface_ButtonDraw(button, kOver);
			Sound_PlaySFx(kSFxItemSelection);
		}
		overButton = button;
		
		//Did the user press the mouse button?
		if(Button()) {
			if(overButton != kButton_None) {
				Interface_ButtonDraw(overButton, kActive);
				Sound_PlaySFx(kSFxConfirm);
				
				//Track mouse
				over = true;
				while(Button()) {
					GetMouse(&mouse);
					//Are we still over the button?
					if(PtInRect(mouse, &(**buttonsData).buttonList[overButton].hotZone)) {
						if(over == false)
						Interface_ButtonDraw(overButton, kActive);
						over = true;
					}
					else {
						if(over == true)
						Interface_ButtonDraw(overButton, kNormal);
						over = false;
					}
					
				}
				Interface_ButtonDraw(overButton, kNormal);
				if(over) {
					if((overButton == kBrowserButton_OK) && (currentLocation != -1)) {
						*gameMode = kGameMode_Local;
						if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion))
						*terrainID = leahTerrainData.ID;
						else
						*terrainID = dataSet->terrainList[currentLocation].ID;
						if(currentShip < dataSet->shipCount)
						*shipID = dataSet->shipList[currentShip].ID;
						else
						*shipID = leahShipData.ID;
						startGame = true;
						run = false;
					}
					else if(overButton == kBrowserButton_Back)
					run = false;
					else if(overButton == kBrowserButton_Report) {
						currentPage = kPage_Report;
						overButton = kButton_None;
					}
					else if(overButton == kBrowserButton_Locations) {
						currentPage = kPage_Locations;
						overButton = kButton_None;
					}
					else if(overButton == kBrowserButton_Engines) {
						currentPage = kPage_Ships;
						overButton = kButton_None;
					}
					else if(overButton == kBrowserButton_Preview) {
						if((currentPage == kPage_Locations) && (currentLocation != -1))
						Display_TerrainPreview(pilot, dataSet);
						else if((currentPage == kPage_Ships) && (currentShip != -1))
						Display_ShipPreview(pilot, dataSet);
						Browser_DisplayPage(pilot, dataSet);
						//overButton = kButton_None;
					}
					else if(overButton == kBrowserButton_Practice) {
#if __DEMO_MODE__
						;
#else
						if((pilot->status == kStatus_Dead) || (pilot->status == kStatus_Champion)) {
							;
						}
						else {
							*gameMode = kGameMode_Practice;
							*terrainID = kNoID;
							if(currentShip < dataSet->shipCount)
							*shipID = dataSet->shipList[currentShip].ID;
							else
							*shipID = leahShipData.ID;
							startGame = true;
							run = false;
						}
#endif
					}
				}
			}
			else {
				if(currentPage == kPage_Locations)
				HandleMouse_Locations(mouse, pilot, dataSet);
				else if(currentPage == kPage_Ships)
				HandleMouse_Ships(mouse, pilot, dataSet);
			}
		}
		
		//Did the user press a key combination?
		if(OSEventAvail(everyEvent, &theEvent)) {
			if((theEvent.what == keyDown) || (theEvent.what == autoKey)) {
				theKey = (theEvent.message & keyCodeMask) >> 8;
				
				button = kButton_None;
				if(theKey == keyTab) {
					currentPage += 1;
					if(currentPage >= kNbPages)
					currentPage = kPage_Report;
					//button = kBrowserButton_Report + currentPage - kPage_Report;
				}
				else if((theKey == keyReturn) || (theKey == keyEnter) || (theKey == keyEnterPB)) {
					if(currentLocation != -1) {
						button = kBrowserButton_OK;
						*gameMode = kGameMode_Local;
						if((pilot->status == kStatus_Challenge) || (pilot->status == kStatus_Champion))
						*terrainID = leahTerrainData.ID;
						else
						*terrainID = dataSet->terrainList[currentLocation].ID;
						if(currentShip < dataSet->shipCount)
						*shipID = dataSet->shipList[currentShip].ID;
						else
						*shipID = leahShipData.ID;
						startGame = true;
						run = false;
					}
				}
				else if(theKey == keyEscape) {
					button = kBrowserButton_Back;
					run = false;
				}
				/*else if(theKey == keyArrowLeft) { //FIXME
					if((currentPage == kPage_Locations) && (currentLocation != -1)) {
						short		newLocation = -1;
					
						for(i = currentLocation - 1; i >= 0; --i) {
							if(PlayMode_GetLocationAccess(pilot, i) > 0) {
								newLocation = i;
								break;
							}
						}
						if(newLocation == -1)
						for(i = iMin(dataSet->terrainCount - 1, kLocation_MaxPreviews - 1); i > currentLocation; --i) {
							if(PlayMode_GetLocationAccess(pilot, i) > 0) {
								newLocation = i;
								break;
							}
						}
						if(newLocation != -1) {
							currentLocation = newLocation;
							Display_Locations(pilot, dataSet);
						}
					}
					else if((currentPage == kPage_Ships) && (currentShip != -1)) {
						short		newShip = -1;
					
						for(i = currentShip - 1; i >= 0; --i) {
							if(PlayMode_GetShipAccess(pilot, i) > 0) {
								newShip = i;
								break;
							}
						}
						if(newShip == -1)
						for(i = iMin(dataSet->shipCount - 1, kLocation_MaxPreviews - 1); i > currentShip; --i) {
							if(PlayMode_GetShipAccess(pilot, i) > 0) {
								newShip = i;
								break;
							}
						}
						if(newShip != -1) {
							currentShip = newShip;
							Display_Ships(pilot, dataSet);
						}
					}
				}
				else if(theKey == keyArrowRight) { //FIXME
					if((currentPage == kPage_Locations) && (currentLocation != -1)) {
						short		newLocation = -1;
					
						for(i = currentLocation + 1; i < dataSet->terrainCount; ++i) {
							if(i >= kLocation_MaxPreviews)
							break;
							
							if(PlayMode_GetLocationAccess(pilot, i) > 0) {
								newLocation = i;
								break;
							}
						}
						if(newLocation == -1)
						for(i = 0; i < currentLocation; ++i) {
							if(i >= kLocation_MaxPreviews)
							break;
							
							if(PlayMode_GetLocationAccess(pilot, i) > 0) {
								newLocation = i;
								break;
							}
						}
						if(newLocation != -1) {
							currentLocation = newLocation;
							Display_Locations(pilot, dataSet);
						}
					}
					else if((currentPage == kPage_Ships) && (currentShip != -1)) {
						short		newShip = -1;
					
						for(i = currentShip + 1; i < dataSet->shipCount; ++i) {
							if(i >= kLocation_MaxPreviews)
							break;
							
							if(PlayMode_GetShipAccess(pilot, i) > 0) {
								newShip = i;
								break;
							}
						}
						if(newShip == -1)
						for(i = 0; i < currentShip; ++i) {
							if(i >= kLocation_MaxPreviews)
							break;
							
							if(PlayMode_GetShipAccess(pilot, i) > 0) {
								newShip = i;
								break;
							}
						}
						if(newShip != -1) {
							currentShip = newShip;
							Display_Ships(pilot, dataSet);
						}
					}
				}*/
				/*else if(theKey == keySpace) {
					if((currentPage == kPage_Locations) && (currentLocation != -1)) {
						button = kBrowserButton_Preview;
						Display_TerrainPreview(pilot, dataSet);
						Browser_DisplayPage(pilot, dataSet);
					}
					else if((currentPage == kPage_Ships) && (currentShip != -1)) {
						button = kBrowserButton_Preview;
						Display_ShipPreview(pilot, dataSet);
						Browser_DisplayPage(pilot, dataSet);
					}
				}*/
				
				if(button != kButton_None) {
					if(overButton != kButton_None)
					Interface_ButtonDraw(overButton, kNormal);
					overButton = button;
					Interface_ButtonDraw(overButton, kActive);
					Sound_PlaySFx(kSFxConfirm);
					startTime = RClock_GetAbsoluteTime();
					while((RClock_GetAbsoluteTime() - startTime) < kButtonDelay)
					;
					Interface_ButtonDraw(overButton, kNormal);
				}
			}
			FlushEvents(everyEvent, 0);
		}
		
		//Do we need to display another page?
		if(oldPage != currentPage) {
			Browser_DisplayPage(pilot, dataSet);
			oldPage = currentPage;
		}
	}
	
	//Are we replaying a race?
	if(startGame && (*gameMode != kGameMode_Practice) && (Pilot_GetRaceOnLocation(pilot, *terrainID) != nil))
	--pilot->numJokers;
	
	HideCursor();
	
	return startGame;
}

static OSErr Browser_Quit()
{
	long				i;
	
	DisposeHandle((Handle) buttonsData);
	for(i = 0; i < kBrowserNbButtons; ++i) {
		DisposeHandle((Handle) buttonsPic[i][0]);
		DisposeHandle((Handle) buttonsPic[i][1]);
	}
	for(i = 0; i < kNbTabs; ++i)
	DisposeHandle((Handle) tabPic[i]);
	for(i = 0; i < kNbBackImages; ++i)
	DisposeHandle((Handle) backPic[i]);
	DisposeHandle((Handle) waitPic);
	DisposeHandle((Handle) previewDisabledPic);
	DisposeHandle((Handle) practiceDisabledPic);
	DisposeHandle((Handle) raceDoneEnabledPic);
	DisposeHandle((Handle) raceDoneDisabledPic);

#if __DEMO_MODE__
	DisposeHandle((Handle) fakeTerrainPreviewsPic);
	DisposeHandle((Handle) fakeShipPreviewsPic);
#endif
	
	DisposePtr((Ptr) specs);
	MacOSTextAliasing_Restore();
	
	return noErr;
}

OSErr Browser_Display(OSType* gameMode, PilotPtr pilot, OSType* terrainID, OSType* shipID)
{
	OSErr				theError;
	Boolean				play;
	DataSetPtr			dataSet;
	
	dataSet = PlayMode_GetDataSet(pilot->mode);
	
	theError = Browser_Start(pilot, dataSet);
	if(theError)
	return theError;
	
	play = Browser_Run(gameMode, pilot, terrainID, shipID, dataSet);
	
	Browser_Quit();
	
	if(!play)
	return kError_UserCancel;
	
	return noErr;
}

void Browser_Reset()
{
	currentShip = -1;
	currentLocation = -1;
}
