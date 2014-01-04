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


#include					"Infinity Structures.h"
#include					"Matrix.h"
#include					"Vector.h"
#include					"Quaternion.h"
#include					"Skeleton Utils.h"
#include					"Infinity Audio Engine.h"
#include					"Cinematic Utils.h"

//CONSTANTES PREPROCESSEURS:

#define	__USE_SOUND_TIME_OFFSETS__	1

//CONSTANTES:

#define						kFakeAbsoluteTime						0
#define						kMaxTolerance							6

//MACROS:

#define Linear_Interpolation(s,e,t)	((s) + (((e) - (s)) * (t)))

//VARIABLES:

static Boolean				audio = false;
static long					audioSoundCount;
static ExtSoundHeaderPtr*	audioSoundList;
static OSType*				audioSoundIDList;

//ROUTINES:

inline void Play_Sound(OSType ID, unsigned long timeOffset)
{
	long				i;
	
	//Find sound
	for(i = 0; i < audioSoundCount; ++i)
	if(audioSoundIDList[i] == ID) {
#if __USE_SOUND_TIME_OFFSETS__
		if(InfinityAudio_InstallSound_CinematicFX(kCinematic_AudioClient, audioSoundList[i], timeOffset) != noErr)
#else
		if(InfinityAudio_InstallSound_CinematicFX(kCinematic_AudioClient, audioSoundList[i], 0) != noErr)
#endif
		SysBeep(0);
		return;
	}
}

inline void Shape_UpdateRotationMatrix(ShapePtr shape)
{
	Matrix			m;
	
	Matrix_SetRotateX(shape->rotateX, &shape->pos);
	
	Matrix_SetRotateY(shape->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &shape->pos, &shape->pos);
	
	Matrix_SetRotateZ(shape->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &shape->pos, &shape->pos);
}

static void Script_Update_Shape(ScriptPtr script, long localTime, unsigned long absoluteTime)
{
	long				i,
						j;
	AnimationPtr		animation;
	EventPtr			timeEvent,
						prevEvent,
						nextEvent;
	ShapePtr			shape;
	float				dTime;
	Quaternion			localQuaternion;
					
	//We've past script's end
	if(localTime >= script->length) {
		//Do we have to loop?
		if(script->flags & kFlag_Loop) {
			//Find our real localTime position
			localTime = localTime % script->length + script->loopTime;
			script->startTime = absoluteTime - localTime;
			
			//Let's call us recursively
			for(i = 0; i < script->animationCount; ++i) {
				script->animationList[i]->running = true;
				if(audio && (script->animationList[i]->type == kType_SoundAnimation)) {
					for(j = script->animationList[i]->eventCount - 1; j >= 0; --j)
					if(script->animationList[i]->eventList[j].time + kMaxTolerance >= localTime)
					script->animationList[i]->shapeID = j;
				}
			}
			Script_Update_Shape(script, localTime, absoluteTime);
		}
		else {
			//Do we need to reset positions?
			if(script->flags & kFlag_ResetOnStop) { //Set positions according to init events
				for(i = 0; i < script->animationCount; ++i) {
					animation = script->animationList[i];
					if(animation->type != kType_Animation)
					continue;
					
					shape = (ShapePtr) animation->shapeID;
					timeEvent = &animation->eventList[0];
					
					if(script->flags & kFlag_UseQuaternions) {
						localQuaternion = *((QuaternionPtr) &timeEvent->rotateX);
						Quaternion_Normalize(&localQuaternion);
						Quaternion_QuaternionToMatrix(&localQuaternion, &shape->pos);
					}
					else {
						shape->rotateX = timeEvent->rotateX;
						shape->rotateY = timeEvent->rotateY;
						shape->rotateZ = timeEvent->rotateZ;
						Shape_UpdateRotationMatrix(shape);
					}
					shape->pos.w = timeEvent->position;
				}
			}
			else {
				for(i = 0; i < script->animationCount; ++i) { //Set positions accrding to last event
					animation = script->animationList[i];
					if(!animation->running)
					continue;
					if(animation->type != kType_Animation)
					continue;
					
					shape = (ShapePtr) animation->shapeID;
					timeEvent = &animation->eventList[animation->eventCount - 1];
					
					if(script->flags & kFlag_UseQuaternions) {
						localQuaternion = *((QuaternionPtr) &timeEvent->rotateX);
						Quaternion_Normalize(&localQuaternion);
						Quaternion_QuaternionToMatrix(&localQuaternion, &shape->pos);
					}
					else {
						shape->rotateX = timeEvent->rotateX;
						shape->rotateY = timeEvent->rotateY;
						shape->rotateZ = timeEvent->rotateZ;
						Shape_UpdateRotationMatrix(shape);
					}
					shape->pos.w = timeEvent->position;
				}
			}
			
			//Script playing is done!
			script->flags &= ~kFlag_Running;
			for(i = 0; i < script->animationCount; ++i)
			script->animationList[i]->running = false;
		}
		return;
	}
	
	//Update animations
	for(i = 0; i < script->animationCount; ++i) {
		animation = script->animationList[i];
		if(!animation->running)
		continue;
		
		if(animation->type == kType_Animation) {
			//Get animated shape
			shape = (ShapePtr) animation->shapeID;
			
			//We've past animation's end
			if(localTime >= animation->endTime) {
				//Set positions according to last event
				timeEvent = &animation->eventList[animation->eventCount - 1];
				
				if(script->flags & kFlag_UseQuaternions) {
					localQuaternion = *((QuaternionPtr) &timeEvent->rotateX);
					Quaternion_Normalize(&localQuaternion);
					Quaternion_QuaternionToMatrix(&localQuaternion, &shape->pos);
				}
				else {
					shape->rotateX = timeEvent->rotateX;
					shape->rotateY = timeEvent->rotateY;
					shape->rotateZ = timeEvent->rotateZ;
					Shape_UpdateRotationMatrix(shape);
				}
				shape->pos.w = timeEvent->position;
				
				//Animation playing is done!
				animation->running = false;
				continue;
			}
			
			//Find our event
			for(j = animation->eventCount - 2; j >= 0; --j) {
				prevEvent = &animation->eventList[j];
				if(localTime >= prevEvent->time) {
					nextEvent = &animation->eventList[j + 1];
					dTime = (float) (localTime - prevEvent->time) / (float) (nextEvent->time - prevEvent->time);
					
					//Interpolate angles
					if(script->flags & kFlag_UseQuaternions) {
						Quaternion_LerpInterpolation((QuaternionPtr) &prevEvent->rotateX, (QuaternionPtr) &nextEvent->rotateX, dTime, &localQuaternion);
						Quaternion_Normalize(&localQuaternion);
						Quaternion_QuaternionToMatrix(&localQuaternion, &shape->pos);
					}
					else {
						shape->rotateX = Linear_Interpolation(prevEvent->rotateX, nextEvent->rotateX, dTime);
						shape->rotateY = Linear_Interpolation(prevEvent->rotateY, nextEvent->rotateY, dTime);
						shape->rotateZ = Linear_Interpolation(prevEvent->rotateZ, nextEvent->rotateZ, dTime);
						Shape_UpdateRotationMatrix(shape);
					}
					
					//Interpolate position
					shape->pos.w.x = Linear_Interpolation(prevEvent->position.x, nextEvent->position.x, dTime);
					shape->pos.w.y = Linear_Interpolation(prevEvent->position.y, nextEvent->position.y, dTime);
					shape->pos.w.z = Linear_Interpolation(prevEvent->position.z, nextEvent->position.z, dTime);
					break;
				}
			}
		}
		else if(audio && (animation->type == kType_SoundAnimation)) {
			//We past a sound event
			if(localTime >= animation->eventList[animation->shapeID].time) {
				if(localTime - animation->eventList[animation->shapeID].time <= kMaxTolerance)
				Play_Sound(((SoundEventPtr) &animation->eventList[animation->shapeID])->soundID, localTime - animation->eventList[animation->shapeID].time);
				++animation->shapeID;
				if(animation->shapeID >= animation->eventCount)
				animation->running = false; //no more events
			}
		}
	}
}

inline void Node_UpdateRotationMatrix(NodePtr node)
{
	Matrix			m;
	
	Matrix_SetRotateX(node->rotateX, &node->pos);
	
	Matrix_SetRotateY(node->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &node->pos, &node->pos);
	
	Matrix_SetRotateZ(node->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &node->pos, &node->pos);
}

static void Script_Update_Skeleton(ScriptPtr script, long localTime, unsigned long absoluteTime)
{
	long				i,
						j;
	AnimationPtr		animation;
	EventPtr			timeEvent,
						prevEvent,
						nextEvent;
	NodePtr				node;
	float				dTime;
	Quaternion			localQuaternion;
					
	//We've past script's end
	if(localTime > script->length) {
		//Do we have to loop?
		if(script->flags & kFlag_Loop) {
			//Find our real localTime position
			localTime = localTime % script->length + script->loopTime;
			script->startTime = absoluteTime - localTime;
			
			//Let's call us recursively
			for(i = 0; i < script->animationCount; ++i) {
				script->animationList[i]->running = true;
				if(audio && (script->animationList[i]->type == kType_SoundAnimation)) {
					for(j = script->animationList[i]->eventCount - 1; j >= 0; --j)
					if(script->animationList[i]->eventList[j].time + kMaxTolerance >= localTime)
					script->animationList[i]->shapeID = j;
				}
			}
			Script_Update_Skeleton(script, localTime, absoluteTime);
		}
		else {
			//Do we need to reset positions?
			if(script->flags & kFlag_ResetOnStop) { //Set positions according to init events
				for(i = 0; i < script->animationCount; ++i) {
					animation = script->animationList[i];
					if(animation->type != kType_Animation)
					continue;
					
					node = &((SkeletonPtr) script->skeletonID)->nodeList[animation->shapeID];
					timeEvent = &animation->eventList[0];
					
					if(script->flags & kFlag_UseQuaternions) {
						localQuaternion = *((QuaternionPtr) &timeEvent->rotateX);
						Quaternion_Normalize(&localQuaternion);
						Quaternion_QuaternionToMatrix(&localQuaternion, &node->pos);
					}
					else {
						node->rotateX = timeEvent->rotateX;
						node->rotateY = timeEvent->rotateY;
						node->rotateZ = timeEvent->rotateZ;
						Node_UpdateRotationMatrix(node);
					}
					node->pos.w = timeEvent->position;
				}
			}
			else {
				for(i = 0; i < script->animationCount; ++i) { //Set positions accrding to last event
					animation = script->animationList[i];
					if(!animation->running)
					continue;
					if(animation->type != kType_Animation)
					continue;
					
					node = &((SkeletonPtr) script->skeletonID)->nodeList[animation->shapeID];
					timeEvent = &animation->eventList[animation->eventCount - 1];
					
					if(script->flags & kFlag_UseQuaternions) {
						localQuaternion = *((QuaternionPtr) &timeEvent->rotateX);
						Quaternion_Normalize(&localQuaternion);
						Quaternion_QuaternionToMatrix(&localQuaternion, &node->pos);
					}
					else {
						node->rotateX = timeEvent->rotateX;
						node->rotateY = timeEvent->rotateY;
						node->rotateZ = timeEvent->rotateZ;
						Node_UpdateRotationMatrix(node);
					}
					node->pos.w = timeEvent->position;
				}
			}
			Skeleton_Morph((SkeletonPtr) script->skeletonID);
			
			//Script playing is done!
			script->flags &= ~kFlag_Running;
			for(i = 0; i < script->animationCount; ++i)
			script->animationList[i]->running = false;
		}
		return;
	}
	
	//Update animations
	for(i = 0; i < script->animationCount; ++i) {
		animation = script->animationList[i];
		if(!animation->running)
		continue;
		
		if(animation->type == kType_Animation) {
			//Get animated node
			node = &((SkeletonPtr) script->skeletonID)->nodeList[animation->shapeID];
			
			//We've past animation's end
			if(localTime >= animation->endTime) {
				//Set positions according to last event
				timeEvent = &animation->eventList[animation->eventCount - 1];
				
				if(script->flags & kFlag_UseQuaternions) {
					localQuaternion = *((QuaternionPtr) &timeEvent->rotateX);
					Quaternion_Normalize(&localQuaternion);
					Quaternion_QuaternionToMatrix(&localQuaternion, &node->pos);
				}
				else {
					node->rotateX = timeEvent->rotateX;
					node->rotateY = timeEvent->rotateY;
					node->rotateZ = timeEvent->rotateZ;
					Node_UpdateRotationMatrix(node);
				}
				node->pos.w = timeEvent->position;
				
				//Animation playing is done!
				animation->running = false;
				continue;
			}
			
			//Find our event
			for(j = animation->eventCount - 2; j >= 0; --j) {
				prevEvent = &animation->eventList[j];
				if(localTime >= prevEvent->time) {
					nextEvent = &animation->eventList[j + 1];
					dTime = (float) (localTime - prevEvent->time) / (float) (nextEvent->time - prevEvent->time);
					
					//Interpolate angles
					if(script->flags & kFlag_UseQuaternions) {
						Quaternion_LerpInterpolation((QuaternionPtr) &prevEvent->rotateX, (QuaternionPtr) &nextEvent->rotateX, dTime, &localQuaternion);
						Quaternion_Normalize(&localQuaternion);
						Quaternion_QuaternionToMatrix(&localQuaternion, &node->pos);
					}
					else {
						node->rotateX = Linear_Interpolation(prevEvent->rotateX, nextEvent->rotateX, dTime);
						node->rotateY = Linear_Interpolation(prevEvent->rotateY, nextEvent->rotateY, dTime);
						node->rotateZ = Linear_Interpolation(prevEvent->rotateZ, nextEvent->rotateZ, dTime);
						Node_UpdateRotationMatrix(node);
					}	
					
					//Interpolate position
					if(animation->flags & kFlag_Translation) {
						node->pos.w.x = Linear_Interpolation(prevEvent->position.x, nextEvent->position.x, dTime);
						node->pos.w.y = Linear_Interpolation(prevEvent->position.y, nextEvent->position.y, dTime);
						node->pos.w.z = Linear_Interpolation(prevEvent->position.z, nextEvent->position.z, dTime);
					}
					else
					node->pos.w = prevEvent->position;
					break;
				}
			}
		}
		else if(audio && (animation->type == kType_SoundAnimation)) {
			//We past a sound event
			if(localTime >= animation->eventList[animation->shapeID].time) {
				if(localTime - animation->eventList[animation->shapeID].time <= kMaxTolerance)
				Play_Sound(((SoundEventPtr) &animation->eventList[animation->shapeID])->soundID, localTime - animation->eventList[animation->shapeID].time);
				++animation->shapeID;
				if(animation->shapeID >= animation->eventCount)
				animation->running = false; //no more events
			}
		}
	}
	Skeleton_Morph((SkeletonPtr) script->skeletonID);
}

void Script_Run(ScriptPtr script, unsigned long absoluteTime)
{
	if(script->type == kType_Skeleton)
	Script_Update_Skeleton(script, absoluteTime - script->startTime, absoluteTime);
	else
	Script_Update_Shape(script, absoluteTime - script->startTime, absoluteTime);
}

void Script_DisplayTime(ScriptPtr script, unsigned long localTime)
{
	long				i;
	
	if(script->flags & kFlag_Running)
	return;
	if(script->length == 0)
	return;
	
	for(i = 0; i < script->animationCount; ++i)
	script->animationList[i]->running = true;
	
	if(script->type == kType_Skeleton)
	Script_Update_Skeleton(script, localTime, kFakeAbsoluteTime);
	else
	Script_Update_Shape(script, localTime, kFakeAbsoluteTime);
}

void Script_Stop(ScriptPtr script)
{
	if(!(script->flags & kFlag_Running))
	return;
	
	if(audio)
	InfinityAudio_CleanUp(kCinematic_AudioClient);
	
	if(script->flags & kFlag_ResetOnStop) {
		if(script->type == kType_Skeleton)
		Script_Update_Skeleton(script, 0, 0);
		else
		Script_Update_Shape(script, 0, 0);
	}
	
	script->flags &= ~kFlag_Running;
}

void Script_Start(ScriptPtr script, unsigned long absoluteTime)
{
	long			i;
	EventPtr		timeEvent;
	
	if(script->flags & kFlag_Running)
	return;
	if(script->length == 0)
	return;
	
	script->flags |= kFlag_Running;
	for(i = 0; i < script->animationCount; ++i) {
		script->animationList[i]->running = true;
		if(script->animationList[i]->type == kType_SoundAnimation)
		script->animationList[i]->shapeID = 0;
	}
	
	if(script->flags & kFlag_SmoothStart) {
		if(script->type == kType_Skeleton) {
			NodePtr				node;
			
			for(i = 0; i < script->animationCount; ++i) {
				if(script->animationList[i]->type != kType_Animation)
				continue;
				node = &((SkeletonPtr) script->skeletonID)->nodeList[script->animationList[i]->shapeID];
				timeEvent = &script->animationList[i]->eventList[0];
				
				//Precalculate first quaternion if necessary
				if(script->flags & kFlag_UseQuaternions) {
					Matrix_ExtractAngles(&node->pos, &node->rotateX, &node->rotateY, &node->rotateZ);
					Quaternion_EulerToQuaternion(-node->rotateX, -node->rotateY, -node->rotateZ, (QuaternionPtr) &timeEvent->rotateX);
				}
				else {
					timeEvent->rotateX = node->rotateX;
					timeEvent->rotateY = node->rotateY;
					timeEvent->rotateZ = node->rotateZ;
				}
				timeEvent->position = node->pos.w;
			}
		}
		else {
			ShapePtr			shape;
			
			for(i = 0; i < script->animationCount; ++i) {
				if(script->animationList[i]->type != kType_Animation)
				continue;
				shape = (ShapePtr) script->animationList[i]->shapeID;
				timeEvent = &script->animationList[i]->eventList[0];
				
				//Precalculate first quaternion if necessary
				if(script->flags & kFlag_UseQuaternions) {
					Matrix_ExtractAngles(&shape->pos, &shape->rotateX, &shape->rotateY, &shape->rotateZ);
					Quaternion_EulerToQuaternion(-shape->rotateX, -shape->rotateY, -shape->rotateZ, (QuaternionPtr) &timeEvent->rotateX);
				}
				else {
					timeEvent->rotateX = shape->rotateX;
					timeEvent->rotateY = shape->rotateY;
					timeEvent->rotateZ = shape->rotateZ;
				}
				timeEvent->position = shape->pos.w;
			}
		}
	}
	else {
		if(script->type == kType_Skeleton)
		Script_Update_Skeleton(script, 0, kFakeAbsoluteTime);
		else
		Script_Update_Shape(script, 0, kFakeAbsoluteTime);
	}
	
	script->startTime = absoluteTime;
}

void Script_AudioEnable(long soundCount, ExtSoundHeaderPtr* soundList, OSType* soundIDList)
{
	audioSoundCount = soundCount;
	audioSoundList = soundList;
	audioSoundIDList = soundIDList;
	
	audio = true;
}

void Script_AudioDisable()
{
	InfinityAudio_CleanUp(kCinematic_AudioClient);
	
	audioSoundCount = 0;
	audioSoundList = nil;
	audioSoundIDList = nil;
	
	audio = false;
}
