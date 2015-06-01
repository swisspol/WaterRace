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


#ifndef __INFINITY_PROJECTION__
#define __INFINITY_PROJECTION__

//INLINE ROUTINES:

inline void Project_TextureBuffer(StatePtr state, long verticeCount)
{
	long				i;
	TQAVTexture			*texturePtr = vTextureBuffer;
	float				pixelConversion = state->pixelConversion,
						iw,
						p1 = state->p1,
						p2 = state->p2,
						projectionOffset_X = state->projectionOffset_X,
						projectionOffset_Y = state->projectionOffset_Y;
						
	if(state->linearZBuffer) {
		float			invRange = state->invRange,
						d = state->d;
		
		for(i = 0; i < verticeCount; ++i, ++texturePtr) {
			iw = 1.0 / texturePtr->z;
			
			texturePtr->x = texturePtr->x * iw * pixelConversion + projectionOffset_X;
			texturePtr->y = texturePtr->y * iw * pixelConversion + projectionOffset_Y;
			texturePtr->z = (texturePtr->z - d) * invRange;
			texturePtr->invW = iw;
			texturePtr->uOverW *= iw;
			texturePtr->vOverW *= iw;
		}
	}
	else {
		float			p1 = state->p1,
						p2 = state->p2;
		
		for(i = 0; i < verticeCount; ++i, ++texturePtr) {
			iw = 1.0 / texturePtr->z;
			
			texturePtr->x = texturePtr->x * iw * pixelConversion + projectionOffset_X;
			texturePtr->y = texturePtr->y * iw * pixelConversion + projectionOffset_Y;
			texturePtr->z = ((texturePtr->z * p1) - p2) / texturePtr->z;
			texturePtr->invW = iw;
			texturePtr->uOverW *= iw;
			texturePtr->vOverW *= iw;
		}
	}
}

inline void Project_GouraudBuffer(StatePtr state, long verticeCount)
{
	long				i;
	TQAVGouraud			*texturePtr = vGouraudBuffer;
	float				pixelConversion = state->pixelConversion,
						iw,
						projectionOffset_X = state->projectionOffset_X,
						projectionOffset_Y = state->projectionOffset_Y;
						
	if(state->linearZBuffer) {
		float			invRange = state->invRange,
						d = state->d;
		
		for(i = 0; i < verticeCount; ++i, ++texturePtr) {
			iw = 1.0 / texturePtr->z;
			
			texturePtr->x = texturePtr->x * iw * pixelConversion + projectionOffset_X;
			texturePtr->y = texturePtr->y * iw * pixelConversion + projectionOffset_Y;
			texturePtr->z = (texturePtr->z - d) * invRange;
			texturePtr->invW = iw;
		}
	}
	else {
		float			p1 = state->p1,
						p2 = state->p2;
		
		for(i = 0; i < verticeCount; ++i, ++texturePtr) {
			iw = 1.0 / texturePtr->z;
			
			texturePtr->x = texturePtr->x * iw * pixelConversion + projectionOffset_X;
			texturePtr->y = texturePtr->y * iw * pixelConversion + projectionOffset_Y;
			texturePtr->z = ((texturePtr->z * p1) - p2) / texturePtr->z;
			texturePtr->invW = iw;
		}
	}
}

#endif
