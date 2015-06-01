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


#include		<fp.h>

#include		"Numeric Utils.h"

//CONSTANTES:

#define			kFloatPrecision			3

#define			kTimeUnit				100
#define			kTimeSeparator			'\''

#define			kExponentSeparator		'E'
#define			kExponentSeparator2		'e'

//VARIABLES LOCALES:

static char		decimalChar = '.';

//ROUTINES:

//Internationnal support
void Get_DecimalChar()
{
	Intl0Hndl			iuSettings = nil;
	
	iuSettings = (Intl0Hndl) GetIntlResource(0);
	if(iuSettings != nil)
	decimalChar = (**iuSettings).decimalPt;
}

//Low-level
long Float2Text(float num, unsigned char* buffer, unsigned char dotChar)
{
	long			size = 0,
					tempNum,
					i,
					length = 0,
					tempNum2;

	//Check for minus
	if(num < 0.0) {
		num = -num;
		*buffer = '-';
		++size;
	}
	
	//Write digits - integer part
	tempNum = num;
	size += Integer2Text(&buffer[size], tempNum);
	
	//Add decimal separator
	buffer[size] = dotChar;
	++size;
	
	//Write digits - decimal part
	tempNum = (num - (float) tempNum) * pow(10.0, kFloatPrecision + 1);
	for(i = 0; i < kFloatPrecision; ++i)
	buffer[size + i] = '0';
	if(tempNum) {
		tempNum2 = tempNum;
		while(tempNum2 != 0) {
			tempNum2 /= 10;
			++length;
		}
		Integer2Text(&buffer[size + kFloatPrecision - length + 1], tempNum);
	}
	size += kFloatPrecision;
	
	return size;
}

float Text2Float(unsigned char* buffer, long size, unsigned char dotChar)
{
	float		num = 0.0,
				dec = 0.0;
	Boolean		minus = false,
				floating = false,
				exponenting = false,
				minusExponenting = false;
	long		exponent = 0,
				power = 0;
	
	//Check for minus
	if(size && (*buffer == '-')) {
		minus = true;
		++buffer;
		--size;
	}
	
	//Read digits
	while(size) {
		if(*buffer == dotChar)
		floating = true;
		else if((*buffer == kExponentSeparator) || (*buffer == kExponentSeparator2)) {
			exponenting = true;
			if(*(buffer + 1) == '-') {
				minusExponenting = true;
				++buffer;
				--size;
			}
		}
		else if((*buffer >= '0') && (*buffer <= '9')) {
			if(exponenting)
			power = power * 10 + (*buffer - '0');
			else if(floating) {
				dec = dec * 10.0 + (float) (*buffer - '0');
				--exponent;
			}
			else
			num = num * 10.0 + (float) (*buffer - '0');
		}
		else
		break;
		++buffer;
		--size;
	}
	
	//Clean up
	num += dec * pow(10.0, exponent);
	if(minus)
	num = -num;
	if(exponenting) {
		if(minusExponenting)
		power = -power;
		num *= pow(10.0, power);
	}
	
	return num;
}

long Integer2Text(unsigned char* buffer, long num)
{
	long		size = 0,
				numDigits = 0,
				copyNum = num,
				i;
	
	//Check for zero
	if(num == 0) {
		*buffer = '0';
		return 1;
	}
	
	//Check for minus
	if(num < 0) {
		*buffer = '-';
		++buffer;
		++size;
		num = -num;
	}
	
	//Count digits
	while(copyNum != 0) {
		++numDigits;
		copyNum /= 10;
	}
	
	//Write digits
	for(i = numDigits - 1; i >= 0; --i) {
		buffer[i] = '0' + num % 10;
		num /= 10;
	}
	size += numDigits;
	
	return size;
}

long Text2Integer(unsigned char* buffer, long size)
{
	long		num = 0;
	Boolean		minus = false;
	
	//Check for minus
	if(size && (*buffer == '-')) {
		minus = true;
		++buffer;
		--size;
	}
	
	//Read digits
	while(size) {
		if((*buffer >= '0') && (*buffer <= '9'))
		num = num * 10 + (*buffer - '0');
		else
		break;
		++buffer;
		--size;
	}
	
	//Clean up
	if(minus)
	num = -num;
	
	return num;
}

long Time2Text(unsigned char* buffer, long time)
{
	long		size = 0;
	
	//Write digits - seconds part
	size += Integer2Text(buffer, time / kTimeUnit);
	
	//Add seconds separator
	buffer[size] = kTimeSeparator;
	++size;
	
	//Write hundreds of seconds
	if(time < 0)
	time = -time;
	buffer[size] = '0';
	buffer[size + 1] = '0';
	time %= kTimeUnit;
	if(time < 10)
	Integer2Text(&buffer[size + 1], time);
	else
	Integer2Text(&buffer[size], time);
	size += 2;
	
	return size;
}

long Text2Time(unsigned char* buffer, long size)
{
	long		time = 0,
				numDigits = 0;
	Boolean		minus = false,
				separatorFound = false;
	
	//Check for minus
	if(size && (*buffer == '-')) {
		minus = true;
		++buffer;
		--size;
	}
	
	//Read digits
	while(size) {
		if(*buffer == kTimeSeparator)
		separatorFound = true;
		else if((*buffer >= '0') && (*buffer <= '9')) {
			if(numDigits < 2)
			time = time * 10 + (*buffer - '0');
			if(separatorFound)
			++numDigits;
		}
		++buffer;
		--size;
	}
	if(numDigits == 1)
	time *= 10;
	
	//Clean up
	if(!separatorFound)
	time *= kTimeUnit;
	if(minus)
	time = -time;
	
	return time;
}

//High level
void FloatToString(float num, Str31 string)
{
	string[0] = Float2Text(num, &string[1], decimalChar);
}

void StringToFloat(Str31 string, float* num)
{
	*num = Text2Float(&string[1], string[0], decimalChar);
}

void StringToInteger(Str31 string, long* num)
{
	*num = Text2Integer(&string[1], string[0]);
}

void IntegerToString(Str31 string, long num)
{
	string[0] = Integer2Text(&string[1], num);
}

void TimeToString(long time, Str31 string)
{
	string[0] = Time2Text(&string[1], time);
}

void StringToTime(Str31 string, long* time)
{
	*time = Text2Time(&string[1], string[0]);
}

void IDToString(OSType id, Str31 string)
{
	string[0] = 4;
	*((OSType*) &string[1]) = id;
}

void StringToID(Str31 string, OSType* id)
{
	*id = *((OSType*) &string[1]);
}
