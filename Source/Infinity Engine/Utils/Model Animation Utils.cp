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


#include			<fp.h>

#include			"Infinity Structures.h"
#include			"Infinity Terrain.h"
#include			"MetaFile Structures.h"
#include			"Infinity Error Codes.h"

#include			"Matrix.h"
#include			"Vector.h"
#include			"Quaternion.h"
#include			"Infinity Rendering.h"

//MACROS:

#define Linear_Interpolation(s,e,t)	((s) + (((e) - (s)) * (t)))

//VARIABLES:

static Boolean		sequencerRunning = false;

//ROUTINES:

void Anim_Start(TerrainPtr terrain, unsigned long absoluteTime)
{
	long			i;
	
	if(terrain->animLength == 0)
	return;
	
	sequencerRunning = true;
	for(i = 0; i < terrain->animCount; ++i)
	terrain->animList[i]->running = true;
	
	terrain->animStartTime = absoluteTime;
}

void Anim_Stop()
{
	sequencerRunning = false;
}

static void Anim_Update(TerrainPtr terrain, long localTime)
{
	long				i,
						j;
	ModelAnimationPtr	animation;
	KeyFramePtr			timeEvent,
						prevEvent,
						nextEvent;
	float				dTime;
	Quaternion			localQuaternion;
	long				newLocalTime;
	
	//Update animations
	for(i = 0; i < terrain->animCount; ++i) {
		animation = terrain->animList[i];
		if(!animation->running)
		continue; //we should check if any animation is running
		
		//We've past animation's end
		if(localTime >= animation->endTime) {
			if(animation->flags & kAnimFlag_Loop) {
				//Convert time to local animation time
				newLocalTime = (localTime - animation->endTime) % (animation->endTime - animation->loopTime) + animation->loopTime;
			}
			else {
				//Set positions according to last event
				timeEvent = &animation->frameList[animation->frameCount - 1];
				
				localQuaternion = *((QuaternionPtr) &timeEvent->rotateX);
				Quaternion_Normalize(&localQuaternion);
				Quaternion_QuaternionToMatrix(&localQuaternion, &animation->pos);
				animation->pos.w = timeEvent->position;
				
				//Animation playing is done!
				animation->running = false;
				continue;
			}
		}
		else
		newLocalTime = localTime;
		
		//Find our event
		for(j = animation->frameCount - 2; j >= 0; --j) {
			prevEvent = &animation->frameList[j];
			if(newLocalTime >= prevEvent->time) {
				nextEvent = &animation->frameList[j + 1];
				dTime = (float) (newLocalTime - prevEvent->time) / (float) (nextEvent->time - prevEvent->time);
				
				//Interpolate angles
				Quaternion_LerpInterpolation((QuaternionPtr) &prevEvent->rotateX, (QuaternionPtr) &nextEvent->rotateX, dTime, &localQuaternion);
				Quaternion_Normalize(&localQuaternion);
				Quaternion_QuaternionToMatrix(&localQuaternion, &animation->pos);
				
				//Interpolate position
				animation->pos.w.x = Linear_Interpolation(prevEvent->position.x, nextEvent->position.x, dTime);
				animation->pos.w.y = Linear_Interpolation(prevEvent->position.y, nextEvent->position.y, dTime);
				animation->pos.w.z = Linear_Interpolation(prevEvent->position.z, nextEvent->position.z, dTime);
				break;
			}
		}
	}
}

void Anim_Run(TerrainPtr terrain, unsigned long absoluteTime)
{
	Anim_Update(terrain, absoluteTime - terrain->animStartTime);
}
