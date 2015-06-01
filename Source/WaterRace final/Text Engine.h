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


#ifndef __WATERRACE_TEXT_ENGINE__
#define __WATERRACE_TEXT_ENGINE__

//CONSTANTES:

#define				kTE_FontResSpecID		'FtDf'

//MACROS:

#define TextEngine_Color16(r,g,b) ((b) + (g << 5) + (r << 10))
#define TextEngine_Color32To16(r,g,b) TextEngine_Color16((r >> 3), (g >> 3), (b >> 3))

//ROUTINES:

typedef struct TextEngine_FontSpec {
	short						fontNum,
								fontSize;
	Style						fontStyle;
	short						fontColor;
};
typedef TextEngine_FontSpec TE_FontSpec;
typedef TextEngine_FontSpec* TE_FontSpecPtr;
typedef TextEngine_FontSpec** TE_FontSpecHandle;

typedef struct TextEngine_ResFontSpec {
	Str255						fontName;
	short						fontSize,
								fontID;
	Style						fontStyle;
	short						colorRed,
								colorGreen,
								colorBlue;
};
typedef TextEngine_ResFontSpec TE_ResFontSpec;
typedef TextEngine_ResFontSpec* TE_ResFontSpecPtr;
typedef TextEngine_ResFontSpec** TE_ResFontSpecHandle;

typedef struct TE_ResFontDefinition {
	short						fontSpecCount;
	TE_ResFontSpec				fontSpecList[];
};
typedef TE_ResFontDefinition* TE_ResFontDefinitionPtr;
typedef TE_ResFontDefinition** TE_ResFontDefinitionHandle;

//ROUTINES:

OSErr TextEngine_Init(short maxFontSize, short maxBufferWidth);
void TextEngine_Settings_Text(short fontNum, short fontSize, Style fontStyle);
void TextEngine_Settings_Clipping(short topClip, short bottomClip);
void TextEngine_Settings_Margings(short leftMargin, short rightMargin);

short TextEngine_GetInterline();

void TextEngine_DrawLine_Left(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint);
void TextEngine_DrawLine_Centered(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint);
void TextEngine_DrawLine_Right(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint);

void TextEngine_DrawText_Left(Ptr textPtr, short textLength, unsigned short color, PixMapPtr thePixMapPtr, Point destPoint);
void TextEngine_DrawFormattedText(Ptr textPtr, short textLength, TE_FontSpec fontSpecs[], PixMapPtr thePixMapPtr, Point destPoint);
void TextEngine_DrawFormattedText_WithFade(Ptr textPtr, short textLength, TE_FontSpec fontSpecs[], PixMapPtr thePixMapPtr, Point destPoint, long fadeHeight);

OSErr TextEngine_Quit();

#endif