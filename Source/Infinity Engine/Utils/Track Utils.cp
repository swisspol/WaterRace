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


#include			"Infinity Structures.h"
#include			"Infinity Terrain.h"

//ROUTINES:

RaceTrackPtr Track_New()
{
	RaceTrackPtr		track;
	
	track = (RaceTrackPtr) NewPtrClear(sizeof(RaceTrack));
	
	BlockMove("\pUntitled", track->name, sizeof(Str31));
	track->flags = 0;
	track->id = kNoID;
	track->color_r = 1.0;
	track->color_g = 1.0;
	track->color_b = 1.0;
	track->lapNumber = 1;
	track->checkCount = 0;
	
	return track;
}

void Track_Reverse(RaceTrackPtr track)
{
	long				i;
	Vector				temp;
	
	for(i = 0; i < track->checkCount; ++i) {
		temp = track->checkList[i].rightMark;
		track->checkList[i].rightMark = track->checkList[i].leftMark;
		track->checkList[i].leftMark = temp;
	}
}
