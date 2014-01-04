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


#ifndef __NUMERIC__
#define __NUMERIC__

//ROUTINES:

//File: Numeric Utils.cp
void Get_DecimalChar();
long Float2Text(float num, unsigned char* buffer, unsigned char dotChar);
float Text2Float(unsigned char* buffer, long size, unsigned char dotChar);
long Integer2Text(unsigned char* buffer, long num);
long Text2Integer(unsigned char* buffer, long size);

void FloatToString(float num, Str31 string);
void StringToFloat(Str31 string, float* num);
void StringToInteger(Str31 string, long* num);
void IntegerToString(Str31 string, long num);
void TimeToString(long time, Str31 string);
void StringToTime(Str31 string, long* time);
void IDToString(OSType id, Str31 string);
void StringToID(Str31 string, OSType* id);

#endif
