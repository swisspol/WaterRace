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
#include				"Infinity Terrain.h"
#include				"Infinity rendering.h"
#include				"Matrix.h"
#include				"Vector.h"
#include				"Clipping Utils.h"
#include				"Projection Utils.h"

//CONSTANTES:

#define					kWireFrameDrawingOffset	0.01

//ROUTINES:

void Terrain_RenderSky(ShapePtr shape, StatePtr state, MatrixPtr localPos, MatrixPtr negatedCamera)
{
	long				x;
	TQAIndexedTriangle	*trianglePtr;
	TQAVTexture			*texturePtr;
	long				triangleCount,
						triangleCount2,
						verticeCount;
	VertexPtr			vertexPtr;
	VectorPtr			normalPtr;
	TriFacePtr			triPtr;
	Matrix				r1;
				
	//Get local matrix
	Matrix_Cat(localPos, negatedCamera, &r1);
	
	//Set texture
	if(shape->texturePtr != state->lastTexture) {
		QASetPtr(state->drawContext, kQATag_Texture, shape->texturePtr);
		state->lastTexture = shape->texturePtr;
	}
	
	//Create vertice buffer and triangles buffer
	verticeCount = shape->pointCount;
	triangleCount = 0;
	triangleCount2 = 0;
	
	//Fill vertice buffer
	texturePtr = vTextureBuffer;
	vertexPtr = shape->pointList;
	normalPtr = shape->normalList;
	for(x = 0; x < shape->pointCount; ++x) {
		//Calculate vertex coordinates
		Matrix_TransformVector(&r1, &vertexPtr->point, (VectorPtr) texturePtr);
		
		//Fill vertex
		texturePtr->uOverW = vertexPtr->u;
		texturePtr->vOverW = vertexPtr->v;
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = 1.0;
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
		texturePtr->a = vertexPtr->c;
		
		++texturePtr;
		++vertexPtr;
		++normalPtr;
	}
	
	//Set texture method
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_None);
	
	//Fill triangles buffer #1
	triPtr = shape->triangleList;
	for(x = 0; x < shape->triangleCount; ++x) {
		//Clip triangle Hitter
		ClipTriangleTexture_Z(state, vTextureBuffer, &verticeCount, trianglesBuffer, &triangleCount, triPtr->corner[0], triPtr->corner[1], triPtr->corner[2]);
		++triPtr;
	}

	//Project vTextureBuffer
	Project_TextureBuffer(state, verticeCount);
	
	//Clip triangles and fill triangle buffer #2
	trianglePtr = trianglesBuffer;
	for(x = 0; x < triangleCount; ++x) {
		ClipTriangleTexture_XY(state, vTextureBuffer, &verticeCount, trianglesBuffer2, &triangleCount2, trianglePtr->vertices[0], trianglePtr->vertices[1], trianglePtr->vertices[2]);
		++trianglePtr;
	}
	
	//Draw triangles
	QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_None);
	QASubmitVerticesTexture(state->drawContext, verticeCount, vTextureBuffer);
	QADrawTriMeshTexture(state->drawContext, triangleCount2, trianglesBuffer2);
	QASetInt(state->drawContext, kQATag_ZFunction, kQAZFunction_LT);
	
	//Flush!
	//QAFlush(state->drawContext);
	
	//ATI driver bug workaround
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate + kQATextureOp_Highlight);
}

void Terrain_RenderMesh_Textured(TerrainMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos)
{
	long				x;
	TQAIndexedTriangle	*trianglePtr;
	TQAVTexture			*texturePtr;
	long				triangleCount,
						triangleCount2,
						verticeCount;
	VertexPtr			vertexPtr;
	TriFacePtr			triPtr;
	LightningPtr		light;
	Vector				v1,
						v2,
						n;
	
	//Set texture
	if(mesh->texturePtr != state->lastTexture) {
		QASetPtr(state->drawContext, kQATag_Texture, mesh->texturePtr);
		state->lastTexture = mesh->texturePtr;
	}
	
	//Create vertice buffer and triangles buffer
	verticeCount = mesh->pointCount;
	triangleCount = 0;
	triangleCount2 = 0;
	
	//Fill vertice buffer
	texturePtr = vTextureBuffer;
	vertexPtr = mesh->pointList;
	light = mesh->lightningList;
	for(x = 0; x < mesh->pointCount; ++x) {
		//Calculate vertex coordinates
		Matrix_TransformVector(negatedCamera, &vertexPtr->point, (VectorPtr) texturePtr);
		
		//Fill vertex
		texturePtr->uOverW = vertexPtr->u;
		texturePtr->vOverW = vertexPtr->v;
		if(mesh->flags & kFlag_Modulate) {
			texturePtr->kd_r = light->kd_r;
			texturePtr->kd_g = light->kd_g;
			texturePtr->kd_b = light->kd_b;
		}
		else
		texturePtr->kd_r = texturePtr->kd_g = texturePtr->kd_b = mesh->diffuse;
		if(mesh->flags & kFlag_Highlight) {
			texturePtr->ks_r = light->ks_r;
			texturePtr->ks_g = light->ks_g;
			texturePtr->ks_b = light->ks_b;
		}
		else
		texturePtr->ks_r = texturePtr->ks_g = texturePtr->ks_b = 0.0;
		texturePtr->a = vertexPtr->c;
		
		++texturePtr;
		++vertexPtr;
		++light;
	}
	
	//Set texture method
	if(mesh->flags & kFlag_Highlight)
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate + kQATextureOp_Highlight);
	else
	QASetInt(state->drawContext, kQATag_TextureOp, kQATextureOp_Modulate);
	
	//Fill triangles buffer #1
	triPtr = mesh->triangleList;
	for(x = 0; x < mesh->triangleCount; ++x) {
		//Remove backfacing triangles
		if(mesh->flags & kFlag_RemoveBF) {
			Vector_Subtract((VectorPtr) &vTextureBuffer[triPtr->corner[1]], (VectorPtr) &vTextureBuffer[triPtr->corner[0]], &v1);
			Vector_Subtract((VectorPtr) &vTextureBuffer[triPtr->corner[2]], (VectorPtr) &vTextureBuffer[triPtr->corner[0]], &v2);
			Vector_CrossProduct(&v2, &v1, &n);
			if(Vector_DotProduct((VectorPtr) &vTextureBuffer[triPtr->corner[0]], &n) > 0) {
				++triPtr;
				continue;
			}
		}

		//Clip triangle Yon
		if(mesh->clipping & kZClipping)
		ClipTriangleTexture_Z(state, vTextureBuffer, &verticeCount, trianglesBuffer, &triangleCount, triPtr->corner[0], triPtr->corner[1], triPtr->corner[2]);
		else {
			trianglesBuffer[triangleCount].triangleFlags = 0;
			trianglesBuffer[triangleCount].vertices[0] = triPtr->corner[0];
			trianglesBuffer[triangleCount].vertices[1] = triPtr->corner[1];
			trianglesBuffer[triangleCount].vertices[2] = triPtr->corner[2];
			++triangleCount;
		}
		
		++triPtr;
	}

	//Project vTextureBuffer
	Project_TextureBuffer(state, verticeCount);
	
	if(mesh->clipping & kXYClipping) {
		//Clip triangles and fill triangle buffer #2
		trianglePtr = trianglesBuffer;
		for(x = 0; x < triangleCount; ++x) {
			ClipTriangleTexture_XY(state, vTextureBuffer, &verticeCount, trianglesBuffer2, &triangleCount2, trianglePtr->vertices[0], trianglePtr->vertices[1], trianglePtr->vertices[2]);
			++trianglePtr;
		}
		
		//Draw triangles
		QASubmitVerticesTexture(state->drawContext, verticeCount, vTextureBuffer);
		QADrawTriMeshTexture(state->drawContext, triangleCount2, trianglesBuffer2);
	}
	else {
		QASubmitVerticesTexture(state->drawContext, verticeCount, vTextureBuffer);
		QADrawTriMeshTexture(state->drawContext, triangleCount, trianglesBuffer);
	}
	
	//Flush!
	//QAFlush(state->drawContext);
}

void Terrain_RenderMesh_WireFrame(TerrainMeshPtr mesh, StatePtr state, MatrixPtr negatedCamera, VectorPtr cameraPos, float r, float g, float b)
{
	long				i;
	TriFacePtr			tri;
	TQAVGouraud			*gouraudPtr;
	VertexPtr			vertexPtr;
	Vector				v1,
						v2,
						n;
	LightningPtr		light;
	
	gouraudPtr = vGouraudBuffer;
	vertexPtr = mesh->pointList;
	light = mesh->lightningList;
	for(i = 0; i < mesh->pointCount; ++i) {
		Matrix_TransformVector(negatedCamera, &vertexPtr->point, (VectorPtr) gouraudPtr);
#if 1
		gouraudPtr->y += kWireFrameDrawingOffset;
#endif
		if(mesh->flags & kFlag_Modulate) {
			gouraudPtr->r = r * light->kd_r;
			gouraudPtr->g = g * light->kd_g;
			gouraudPtr->b = b * light->kd_b;
		}
		else {
			gouraudPtr->r = r * mesh->diffuse;
			gouraudPtr->g = g * mesh->diffuse;
			gouraudPtr->b = b * mesh->diffuse;
		}
		gouraudPtr->a = 1.0;
		
		++vertexPtr;
		++gouraudPtr;
		++light;
	}
	
	tri = mesh->triangleList;
	for(i = 0; i < mesh->triangleCount; ++i) {
		if(mesh->flags & kFlag_RemoveBF) {
			Vector_Subtract((VectorPtr) &vGouraudBuffer[tri->corner[1]], (VectorPtr) &vGouraudBuffer[tri->corner[0]], &v1);
			Vector_Subtract((VectorPtr) &vGouraudBuffer[tri->corner[2]], (VectorPtr) &vGouraudBuffer[tri->corner[0]], &v2);
			Vector_CrossProduct(&v2, &v1, &n);
			if(Vector_DotProduct((VectorPtr) &vGouraudBuffer[tri->corner[0]], &n) > 0) {
				++tri;
				continue;
			}
		}
		
		Gouraud_DrawLine(state, &vGouraudBuffer[tri->corner[0]], &vGouraudBuffer[tri->corner[1]]);
		Gouraud_DrawLine(state, &vGouraudBuffer[tri->corner[1]], &vGouraudBuffer[tri->corner[2]]);
		Gouraud_DrawLine(state, &vGouraudBuffer[tri->corner[2]], &vGouraudBuffer[tri->corner[0]]);
		
		++tri;
	}
}


