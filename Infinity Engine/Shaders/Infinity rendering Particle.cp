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


#include				<fp.h>

#include				"Infinity Structures.h"
#include				"Matrix.h"
#include				"Vector.h"
#include				"Infinity rendering.h"
#include				"Infinity Utils.h"

//CONSTANTES PREPROCESSEURS:

#define	__DISABLE_FOG__	1

//CONSTANTES:

#define					kMaxTimeDifference				kTimeUnit
#define					kRandomMax						65535

//VARIABLES LOCALES:

static long _seed = 12345;

//ROUTINES:

inline float RandomFloat()
{
	_seed = (_seed * 16807) % kRandomMax;
	
	return (float) (_seed - kRandomMax / 2) / (float) (kRandomMax / 2);
}

inline float Compute_Value(float init, float rand)
{
	float	value = init;
	
	if(rand != 0.0) {
		if(rand < 0.0)
		value -= rand * RandomFloat();
		else
		value += rand * fAbs(RandomFloat());
	}

	return value;
}

void Particle_Init(InfinityParticlePtr particle, ShapeData_ParticlePtr system, MatrixPtr systemMatrix)
{
	//particle->sprite.position
	particle->sprite.size = Compute_Value(system->initSize, system->initSizeRandom);
	//particle->sprite.transparency = 1.0;
	particle->sprite.intensity = 1.0;
	//particle->sprite.hiColor_r = 0.0;
	//particle->sprite.hiColor_g = 0.0;
	//particle->sprite.hiColor_b = 0.0;
	
	particle->position.x = Compute_Value(system->initPosition.x, system->initPositionRandom.x);
	particle->position.y = Compute_Value(system->initPosition.y, system->initPositionRandom.y);
	particle->position.z = Compute_Value(system->initPosition.z, system->initPositionRandom.z);
	particle->oldPosition = particle->position;
	particle->velocity.x = Compute_Value(system->initVelocity.x, system->initVelocityRandom.x);
	particle->velocity.y = Compute_Value(system->initVelocity.y, system->initVelocityRandom.y);
	particle->velocity.z = Compute_Value(system->initVelocity.z, system->initVelocityRandom.z);
	particle->energy = Compute_Value(system->initEnergy, system->initEnergyRandom);
	particle->alive = true;
	
	if(system->flags & kFlag_GlobalSystem) {
		Matrix_TransformVector(systemMatrix, &particle->position, &particle->position);
		if(system->flags & kFlag_LocalVelocity)
		Matrix_RotateVector(systemMatrix, &particle->velocity, &particle->velocity);
	}
}

static void Particles_ReboundInBBox(ShapeData_ParticlePtr system, MatrixPtr systemMatrix, InfinityParticlePtr particle, Vector box[kBBSize])
{
	long					i;
	
	for(i = 0; i < system->particleCount; ++i, ++particle) {
		if(particle->position.x < box[0].x) {
			particle->position.x = box[0].x;
			particle->velocity.x = -particle->velocity.x;
			continue;
		}
		
		if(particle->position.x > box[1].x) {
			particle->position.x = box[1].x;
			particle->velocity.x = -particle->velocity.x;
			continue;
		}
		
		if(particle->position.y < box[0].y) {
			particle->position.y = box[0].y;
			particle->velocity.y = -particle->velocity.y;
			continue;
		}
		
		if(particle->position.y > box[4].y) {
			particle->position.y = box[4].y;
			particle->velocity.y = -particle->velocity.y;
			continue;
		}
		
		if(particle->position.z < box[0].z) {
			particle->position.z = box[0].z;
			particle->velocity.z = -particle->velocity.z;
			continue;
		}
		
		if(particle->position.z > box[2].z) {
			particle->position.z = box[2].z;
			particle->velocity.z = -particle->velocity.z;
			continue;
		}
	}
}

static void Particles_KeepInBBox(ShapeData_ParticlePtr system, MatrixPtr systemMatrix, InfinityParticlePtr particle, Vector box[kBBSize])
{
	Vector					temp;
	Matrix					n;
	long					i;
	
	if(system->flags & kFlag_GlobalSystem) {
		Matrix_Negate(systemMatrix, &n);
	
		for(i = 0; i < system->particleCount; ++i, ++particle) {
			Matrix_TransformVector(&n, &particle->position, &temp);
			
			if(temp.x < box[0].x) {
				temp.x = box[1].x + temp.x - box[0].x;
				Matrix_TransformVector(systemMatrix, &temp, &particle->position);
				continue;
			}
			
			if(temp.x > box[1].x) {
				temp.x = box[0].x + temp.x - box[1].x;
				Matrix_TransformVector(systemMatrix, &temp, &particle->position);
				continue;
			}
			
			if(temp.y < box[0].y) {
				temp.y = box[4].y + temp.y - box[0].y;
				Matrix_TransformVector(systemMatrix, &temp, &particle->position);
				continue;
			}
			
			if(temp.y > box[4].y) {
				temp.y = box[0].y + temp.y - box[4].y;
				Matrix_TransformVector(systemMatrix, &temp, &particle->position);
				continue;
			}
			
			if(temp.z < box[0].z) {
				temp.z = box[2].z + temp.z - box[0].z;
				Matrix_TransformVector(systemMatrix, &temp, &particle->position);
				continue;
			}
			
			if(temp.z > box[2].z) {
				temp.z = box[0].z + temp.z - box[2].z;
				Matrix_TransformVector(systemMatrix, &temp, &particle->position);
				continue;
			}
		}
	}
	else {
		for(i = 0; i < system->particleCount; ++i, ++particle) {
			if(particle->position.x < box[0].x) {
				particle->position.x = box[1].x;
				continue;
			}
			
			if(particle->position.x > box[1].x) {
				particle->position.x = box[0].x;
				continue;
			}
			
			if(particle->position.y < box[0].y) {
				particle->position.y = box[4].y;
				continue;
			}
			
			if(particle->position.y > box[4].y) {
				particle->position.y = box[0].y;
				continue;
			}
			
			if(particle->position.z < box[0].z) {
				particle->position.z = box[2].z;
				continue;
			}
			
			if(particle->position.z > box[2].z) {
				particle->position.z = box[0].z;
				continue;
			}
		}
	}
}

static void Particles_AutoKill(ShapeData_ParticlePtr system, MatrixPtr systemMatrix, InfinityParticlePtr particle, Vector box[kBBSize])
{
	Vector					temp;
	Matrix					n;
	long					i;
	
	if(system->flags & kFlag_GlobalSystem) {
		Matrix_Negate(systemMatrix, &n);
	
		for(i = 0; i < system->particleCount; ++i, ++particle) {
			Matrix_TransformVector(&n, &particle->position, &temp);
			
			if(temp.x < box[0].x) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(temp.x > box[1].x) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(temp.y < box[0].y) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(temp.y > box[4].y) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(temp.z < box[0].z) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(temp.z > box[2].z) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
		}
	}
	else {
		for(i = 0; i < system->particleCount; ++i, ++particle) {
			if(particle->position.x < box[0].x) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(particle->position.x > box[1].x) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(particle->position.y < box[0].y) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(particle->position.y > box[4].y) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(particle->position.z < box[0].z) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
			
			if(particle->position.z > box[2].z) {
				Particle_Init(particle, system, systemMatrix);
				continue;
			}
		}
	}
}

inline void Particle_Update(InfinityParticlePtr particle, ShapeData_ParticlePtr system, MatrixPtr systemMatrix, float rate, Vector box[kBBSize])
{
	if(!particle->alive)
	return;
	
	particle->oldPosition = particle->position;
	Vector_MultiplyAdd(rate, &particle->velocity, &particle->position, &particle->position);
	Vector_Multiply(1.0 - rate * system->velocityDecrease, &particle->velocity, &particle->velocity);
	particle->velocity.y += system->gravity * rate;
	particle->energy -= system->energyDecrease * rate;
	particle->sprite.size -= system->sizeDecrease * rate;
	
	if((particle->energy <= 0.0) || (particle->sprite.size <= 0.0))
	Particle_Init(particle, system, systemMatrix);
}

void Particle_DrawShape(StatePtr state, ShapePtr shape, MatrixPtr globalPos, MatrixPtr negatedCamera, VectorPtr cameraPos, ShapePtr shapeList[], long clip)
{
	Matrix					localPos;
	InfinityParticlePtr		particle;
	long					i;
	ShapeData_ParticlePtr	data;
	unsigned long			time = state->frameTime;
	float					force,
							maxEnergy,
							rate;
	long					saveBlend,
							saveFog;
	
	//Extract particle shape data
	if(shape->dataPtr == nil)
	return;
	data = (ShapeData_ParticlePtr) shape->dataPtr;
	
	//Get local matrix - max depth = 4
	localPos = *globalPos;
	if(shape->parent != nil)
	Shape_LinkMatrixByParent(shape, &localPos);
	if(shape->flags & kFlag_RelativePos)
	Matrix_Cat(&shape->pos, &localPos, &localPos);
	if(!(data->flags & kFlag_GlobalSystem))
	Matrix_Cat(&localPos, negatedCamera, &localPos);
	
	//Update particules
	if(fAbs(time - data->lastTime) < kMaxTimeDifference) {
		if(time != data->lastTime) {
			rate = (float) (time - data->lastTime) / (float) kTimeUnit;
			particle = (InfinityParticlePtr) shape->pointList;
			for(i = 0; i < data->particleCount; ++i, ++particle)
			Particle_Update(particle, data, &localPos, rate, shape->boundingBox);
		}
	}
	data->lastTime = time;
	
	//Behaviour
	if(data->flags & kFlag_AutoKill)
	Particles_AutoKill(data, &localPos, (InfinityParticlePtr) shape->pointList, shape->boundingBox);
	if(data->flags & kFlag_KeepInBBox)
	Particles_KeepInBBox(data, &localPos, (InfinityParticlePtr) shape->pointList, shape->boundingBox);
	if(data->flags & kFlag_ReboundInBBox)
	Particles_ReboundInBBox(data, &localPos, (InfinityParticlePtr) shape->pointList, shape->boundingBox);
	
	//Render particles
	maxEnergy = data->initEnergy + fAbs(data->initEnergyRandom);
	particle = (InfinityParticlePtr) shape->pointList;
	for(i = 0; i < data->particleCount; ++i, ++particle) {
		if(data->flags & kFlag_GlobalSystem)
		Matrix_TransformVector(negatedCamera, &particle->position, &particle->sprite.position);
		else
		Matrix_TransformVector(&localPos, &particle->position, &particle->sprite.position);
		
		force = particle->energy / maxEnergy;
		particle->sprite.transparency = force;
		particle->sprite.hiColor_r = force * data->modulate_r;
		particle->sprite.hiColor_g = force * data->modulate_g;
		particle->sprite.hiColor_b = force * data->modulate_b;
	}
	if(data->flags & kFlag_NoZBuffer)
	QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_True);
	if(data->flags & kFlag_Premultiplied) {
		saveBlend = QAGetInt(state->drawContext, kQATag_Blend);
		if(saveBlend != kQABlend_PreMultiply)
		QASetInt(state->drawContext, kQATag_Blend, kQABlend_PreMultiply);
#if __DISABLE_FOG__
		saveFog = QAGetInt(state->drawContext, kQATag_FogMode);
		if(saveFog != kQAFogMode_None)
		QASetInt(state->drawContext, kQATag_FogMode, kQAFogMode_None);
#endif
	}
	Texture_DrawSprites(state, data->particleCount, (InfinitySpritePtr) shape->pointList, sizeof(InfinityParticle), shape->texturePtr, true);
	if(data->flags & kFlag_Premultiplied) {
		if(saveBlend != kQABlend_PreMultiply)
		QASetInt(state->drawContext, kQATag_Blend, saveBlend);
#if __DISABLE_FOG__
		if(saveFog != kQAFogMode_None)
		QASetInt(state->drawContext, kQATag_FogMode, saveFog);
#endif
	}
	if(data->flags & kFlag_NoZBuffer)
	QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_LT);
	
	//Flush!
	//QAFlush(state->drawContext);*/
}
