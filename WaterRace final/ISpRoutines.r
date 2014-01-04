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


#include <MacTypes.r>
#include <InputSprocket.r>

//MACROS:

#define		rNoModifiers	rControlOff, rOptionOff, rShiftOff, \
							controlOff, optionOff, shiftOff, commandOff

//RESSOURCES:

resource 'isap' (128)
{
	callsISpInit,
	usesInputSprocket
};

resource 'setl' (128, "Default sets")
{
	currentVersion,
	{
		"Default (Keyboard)", 0, kISpDeviceClass_Keyboard, 
			kISpKeyboardID_Apple, notApplSet, isDefaultSet, 
			128,
	};
};

resource 'tset' (128, "Default (Keyboard)")
{
	supportedVersion,
	{	
		// kShip_Turbo
		spaceKey,
		rNoModifiers,
		
		// kShip_Direction - LEFT
		leftKey,
		rNoModifiers,
		
		// kShip_Direction - RIGHT
		rightKey,
		rNoModifiers,
		
		// kShip_Thrust - DOWN
		downKey,
		rNoModifiers,
		
		// kShip_Thrust - UP
		upKey,
		rNoModifiers,
		
		// kGame_ShipDrop
		tabKey,
		rNoModifiers,
		
		// kGame_Escape
		escKey,
		rNoModifiers,
		
		// kGame_Camera_Follow
		f1Key,
		rNoModifiers,
		
		// kGame_Camera_Head
		f2Key,
		rNoModifiers,
		
		// kGame_Camera_Onboard
		f3Key,
		rNoModifiers,
		
		// kGame_Camera_Chase
		f4Key,
		rNoModifiers,
		
		// kGame_Camera_Terrain
		f5Key,
		rNoModifiers,
		
		// kGame_Target_Local
		n0Key,
		rNoModifiers,
		
		// kGame_Target_1
		n1Key,
		rNoModifiers,
		
		// kGame_Target_2
		n2Key,
		rNoModifiers,
		
		// kGame_Target_3
		n3Key,
		rNoModifiers,
		
		// kGame_Target_4
		n4Key,
		rNoModifiers,
		
		// kGame_Target_5
		n5Key,
		rNoModifiers,
		
		// kGame_Target_6
		n6Key,
		rNoModifiers,
		
		// kGame_Target_7
		n7Key,
		rNoModifiers,
		
		// kGame_Target_8
		n8Key,
		rNoModifiers,
		
		// kGame_TakeScreenshot
		deleteKey,
		rNoModifiers,
		
		// kGame_MusicVolumeDown
		f9Key,
		rNoModifiers,
		
		// kGame_MusicVolumeUp
		f10Key,
		rNoModifiers,
		
		// kGame_FXVolumeDown
		f11Key,
		rNoModifiers,
		
		// kGame_FXVolumeUp
		f12Key,
		rNoModifiers
		
	};
};
