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


#ifndef __KEYS__
#define __KEYS__

//CONSTANTES:

#define				keyArrowLeft			0x7b
#define				keyArrowRight			0x7c
#define				keyArrowUp				0x7e
#define				keyArrowDown			0x7d

#define				keyCommand				0x37
#define				keyOption				0x3A
#define				keyShift				0x38
#define				keyControl				0x3B
#define				keySpace				0x31
#define				keyEscape				0x35
#define				keyTab					0x30
#define				keyBackSpace			0x33
#define				keyDelete				0x75
#define				keyCapsLock				0x39

#define				keyF1					0x7A
#define				keyF2					0x78
#define				keyF3					0x63
#define				keyF4					0x76
#define				keyF5					0x60
#define				keyF6					0x61
#define				keyF7					0x62
#define				keyF8					0x64
#define				keyF9					0x65
#define				keyF10					0x6D
#define				keyF11					0x67
#define				keyF12					0x6F
#define				keyF13					0x69
#define				keyF14					0x6B
#define				keyF15					0x71

#define				keyReturn				0x24
#define				keyEnter				0x4C
#define				keyEnterPB				0x34

#define				keyPad0					0x52
#define				keyPad1					0x53
#define				keyPad2					0x54
#define				keyPad3					0x55
#define				keyPad4					0x56
#define				keyPad5					0x57
#define				keyPad6					0x58
#define				keyPad7					0x59
#define				keyPad8					0x5B
#define				keyPad9					0x5C

#define				kButtonStateOn			0
#define				kButtonStateOff			128

//MACROS:

#define IsKeyDown(keyMap, theKey) (((unsigned char *)(keyMap))[(theKey) / 8] & 1 << ((theKey) % 8))

#endif
