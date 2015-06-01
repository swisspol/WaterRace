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


#ifndef __WATERRACE_COLLISION_ENGINE__
#define __WATERRACE_COLLISION_ENGINE__

//PROTOTYPES:

//File: Collision Engine.cp
void Check_BonusCollision_Local(ExtendedShipPtr ship, MatrixPtr inverseShipMatrix);
#if __BOTS_CAN_PICKUP_TURBOS__
void Check_BonusCollision_Bot(ExtendedShipPtr ship, MatrixPtr inverseShipMatrix);
#endif
Boolean Box_Collision(CollisionBoxPtr source, CollisionBoxPtr dest);
Boolean Object_Collision(MegaObjectPtr source, MatrixPtr sourceMatrix, float sourceScale, MegaObjectPtr dest, MatrixPtr destMatrix, float destScale);
Boolean Object_Collision_WithNormal(MegaObjectPtr source, MatrixPtr sourceMatrix, float sourceScale, MegaObjectPtr dest, MatrixPtr destMatrix, float destScale, VectorPtr normal);
Boolean ShipEnclosure_Collision(ExtendedShipPtr ship, CollisionEnclosurePtr enclosure);
Boolean ShipEnclosure_Collision_WithNormal(ExtendedShipPtr ship, CollisionEnclosurePtr enclosure, VectorPtr normal);

#endif
