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
#include			"Matrix.h"

//ROUTINES:

SkeletonPtr Skeleton_New(OSType shapeID, ShapePtr shape)
{
	SkeletonPtr		skeleton;
	NodePtr			node;
	
	skeleton = (SkeletonPtr) NewPtrClear(sizeof(Skeleton) + sizeof(ShapePoint) * shape->pointCount);
	node = &skeleton->nodeList[0];
	
	skeleton->id = kNoID;
	skeleton->flags = 0;
	skeleton->shapeID = shapeID;
	skeleton->nodeCount = 1;
	skeleton->shapePtr = nil;
	
	node->parentNode = kNoNode;
	node->childNodeCount = 0;
	node->rotateX = 0.0;
	node->rotateY = 0.0;
	node->rotateZ = 0.0;
	Matrix_Clear(&node->pos);
	node->verticeCount = 0;
	BlockMove("\pRoot node", node->name, sizeof(Str31));
	
	shape->flags |= kFlag_HasSkeleton;
	
	return skeleton;
}

void Node_UpdateMatrix(NodePtr node)
{
	Vector			p;
	Matrix			m;
	
	p = node->pos.w;
	
	Matrix_SetRotateX(node->rotateX, &node->pos);
	
	Matrix_SetRotateY(node->rotateY, &m);
	Matrix_MultiplyByMatrix(&m, &node->pos, &node->pos);
	
	Matrix_SetRotateZ(node->rotateZ, &m);
	Matrix_MultiplyByMatrix(&m, &node->pos, &node->pos);
	
	node->pos.w = p;
}

inline void Morph_Vertices(ShapePtr shape, SkeletonPtr skeleton, NodePtr node, MatrixPtr m)
{
	long				i;
	
	for(i = 0; i < node->verticeCount; ++i) {
		//Transform vertex
		Matrix_TransformVector(m, &skeleton->points[node->verticeList[i]].vector, (VectorPtr) &shape->pointList[node->verticeList[i]]);
		//Transform normal
		Matrix_RotateVector(m, &skeleton->points[node->verticeList[i]].normal, &shape->normalList[node->verticeList[i]]);
	}
}

static void Node_Morph(ShapePtr shape, SkeletonPtr skeleton, MatrixPtr m, long nodeID)
{
	NodePtr				node,
						nextNode;
	long				i;
	Matrix				r;
	
	node = &skeleton->nodeList[nodeID];
	
	//Draw child nodes
	for(i = 0; i < node->childNodeCount; ++i) {
		nextNode = &skeleton->nodeList[node->childNodeList[i]];
		
		//Draw child node
		Matrix_Cat(&nextNode->pos, m, &r);
		Morph_Vertices(shape, skeleton, nextNode, &r);
		
		//Draw recursively
		Node_Morph(shape, skeleton, &r, node->childNodeList[i]);
	}
}

void Skeleton_Morph(SkeletonPtr skeleton)
{
	Matrix				r1;
	
	//Draw source node
	r1 = skeleton->nodeList[0].pos;
	Morph_Vertices(skeleton->shapePtr, skeleton, &skeleton->nodeList[0], &r1);
	
	//Draw nodes recursively
	Node_Morph(skeleton->shapePtr, skeleton, &r1, 0);
}
