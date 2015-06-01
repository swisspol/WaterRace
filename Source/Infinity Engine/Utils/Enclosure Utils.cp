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

#include			"Vector.h"
#include			"Matrix.h"

//ROUTINES:

CollisionEnclosurePtr Enclosure_New()
{
	CollisionEnclosurePtr		enclosure;
	
	enclosure = (CollisionEnclosurePtr) NewPtrClear(sizeof(CollisionEnclosure));
	
	enclosure->flags = 0;
	enclosure->closed = false;
	
	enclosure->bounding_min_x = 0.0;
	enclosure->bounding_max_x = 0.0;
	enclosure->bounding_min_z = 0.0;
	enclosure->bounding_max_z = 0.0;
	
	enclosure->wallCount = 0;
	
	return enclosure;
}

void Enclosure_Reverse(CollisionEnclosurePtr enclosure)
{
	long				i;
	Vector				temp;
	CollisionWall		temp2;
	
	for(i = 0; i < enclosure->wallCount; ++i) {
		temp = enclosure->wallList[i].right;
		enclosure->wallList[i].right = enclosure->wallList[i].left;
		enclosure->wallList[i].left = temp;
	}
	
	for(i = 0; i < enclosure->wallCount / 2; ++i) {
		temp2 = enclosure->wallList[i];
		enclosure->wallList[i] = enclosure->wallList[enclosure->wallCount - i - 1];
		enclosure->wallList[enclosure->wallCount - i - 1] = temp2;
	}
}

void Enclosure_PreProcess(CollisionEnclosurePtr enclosure)
{
	long					i;
	CollisionWallPtr		wall;
	Matrix					wallMatrix;
	
	//Compute bounding
	enclosure->bounding_min_x = kHugeDistance;
	enclosure->bounding_max_x = -kHugeDistance;
	enclosure->bounding_min_z = kHugeDistance;
	enclosure->bounding_max_z = -kHugeDistance;
	wall = enclosure->wallList;
	for(i = 0; i < enclosure->wallCount; ++i, ++wall) {
		
		if(wall->left.x < enclosure->bounding_min_x)
		enclosure->bounding_min_x = wall->left.x;
		if(wall->left.x > enclosure->bounding_max_x)
		enclosure->bounding_max_x = wall->left.x;
		
		if(wall->left.z < enclosure->bounding_min_z)
		enclosure->bounding_min_z = wall->left.z;
		if(wall->left.z > enclosure->bounding_max_z)
		enclosure->bounding_max_z = wall->left.z;
		
		if(wall->right.x < enclosure->bounding_min_x)
		enclosure->bounding_min_x = wall->right.x;
		if(wall->right.x > enclosure->bounding_max_x)
		enclosure->bounding_max_x = wall->right.x;
		
		if(wall->right.z < enclosure->bounding_min_z)
		enclosure->bounding_min_z = wall->right.z;
		if(wall->right.z > enclosure->bounding_max_z)
		enclosure->bounding_max_z = wall->right.z;
		
	}
	
	//Preprocess walls
	wall = enclosure->wallList;
	for(i = 0; i < enclosure->wallCount; ++i, ++wall) {
		wall->left.y = 0.0;
		wall->right.y = 0.0;
		
		//Compute wall width
		wall->width = Vector_Distance(wall->left, wall->right);
		
		//Compute wall matrix
		Vector_Subtract(&wall->right, &wall->left, &wallMatrix.x);
		Vector_Normalize(&wallMatrix.x, &wallMatrix.x);
		wallMatrix.y.x = 0.0;
		wallMatrix.y.y = 1.0;
		wallMatrix.y.z = 0.0;
		Vector_CrossProduct(&wallMatrix.y, &wallMatrix.x, &wallMatrix.z);
		wallMatrix.w = wall->left;
		
		//Compute wall inverse matrix
		Matrix_Negate(&wallMatrix, &wall->inverseMatrix);
	}
}
