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


#ifndef __INFINITY_METAFILE_STRUCTURES__
#define __INFINITY_METAFILE_STRUCTURES__

//CONSTANTES:

//File constants
#define					kMetaFileCreator		'Meta'
#define					kMetaFileType			'Meta'
#define					kMetaFile_Version		0x0104

//File flags
#define					kMetaFlag_Protected		(1 << 0)

//Other constants
#define					kMetaFile_PreviewH		128
#define					kMetaFile_PreviewV		85
#define					kMetaFile_NoID			'----'
#define					kMetaFile_NoData		(-1)
#define					kMetaFile_NoTagFound	(-1)
#define					kMetaFile_MaxSets		16
#define					kMetaFile_NoVersion		0xFFFF
#define					kMetaFile_MaxTagEntries	2048
#define					kMetaFile_UseID			(-1)

//Master tags
#define					kTag_MegaObject			'mega'
#define					kTag_Terrain			'terr'
#define					kTag_Shape				'shap'
#define					kTag_CompressedTexture	'cptx'
#define					kTag_TextureSet			'txst'
#define					kTag_Script				'scrp'
#define					kTag_Camera				'cmra'
#define					kTag_Skeleton			'sklt'
#define					kTag_Data				'data'
#define					kTag_TerrainMesh		'tmsh'
#define					kTag_SeaMesh			'smsh'
#define					kTag_AmbientSound		'absd'
#define					kTag_ModelAnimation		'mdan'
#define					kTag_Track				'trck'
#define					kTag_ItemList			'item'
#define					kTag_Enclosure			'encl'

//Fake tags
#define					kFakeTag_ModelGeometry	'mgeo'
#define					kFakeTag_ModelSkeleton	'mske'
#define					kFakeTag_ModelAnimation	'mani'
#define					kFakeTag_ModelCameras	'mcam'
#define					kFakeTag_TerrainGeometry	'tgeo'
#define					kFakeTag_TerrainTextureList	'ttxl'
#define					kFakeTag_ModelTextureList	'mtxl'
#define					kFakeTag_TerrainCameras	'tcam'
#define					kFakeTag_TerrainSounds	'tsnd'
#define					kFakeTag_TerrainAnimations	'tani'
#define					kFakeTag_TerrainTracks	'ttrk'
#define					kFakeTag_TerrainEnclosures	'tecl'

//Subtags
#define					kSubTag_PointList		'pnts'
#define					kSubTag_TriangleList	'trig'
#define					kSubTag_NormalList		'nrls'
#define					kSubTag_TextureImage	'txim'
#define					kSubTag_Animation		'anim'
#define					kSubTag_LightningList	'ligt'
#define					kSubTag_ShapeData		'shdt'

//Model builder tags
#define					kTag_CameraLayout		'cmly'

//Terrain builder tags
#define					kTag_CameraPosition		'cmps'
#define					kTag_ViewSetup			'view'

//Misc tags
#define					kSubTag_None			'----'
#define					kTag_IncompleteData		'<-->'
#define					kTag_Unknown			'????'
#define					kTag_DigitalSignature	'sign'
#define					kTag_Preview			'prev'

//MegaObject & Terrain skipflags
#define					kMetaFile_SkipNone			0
#define					kMetaFile_SkipSkeletons		(1 << 0)
#define					kMetaFile_SkipScripts		(1 << 1)
#define					kMetaFile_SkipCameras		(1 << 2)
#define					kMetaFile_SkipAmbientSounds	(1 << 3)
#define					kMetaFile_SkipAnimations	(1 << 4)
#define					kMetaFile_SkipTracks		(1 << 5)
#define					kMetaFile_SkipSpecialItems	(1 << 6)
#define					kMetaFile_SkipEnclosures	(1 << 7)

//Version constants
#define					kVersion_3DModelInfo	0x0100

//STRUCTURES:

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct {
	short			version;
	unsigned short	flags;
	OSType			creatorCode;
	long			tagTablePosition,
					tagTableSize;
} MetaFile_Header;
typedef MetaFile_Header* MetaFile_HeaderPtr;

typedef struct {
	unsigned char	depth,
					flags;
	short			version;
	OSType			tag,
					subTag,
					ID;
	long			dataSize,
					dataPosition;
} MetaFile_Entry;
typedef MetaFile_Entry* MetaFile_EntryPtr;

typedef struct {
	unsigned long	tagCount;
	MetaFile_Entry	tagList[];
} MetaFile_Table;
typedef MetaFile_Table* MetaFile_TablePtr;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

typedef struct {
	short				fileRefNum;
	long				currentDepth; //used when writing
	long				currentTag;
	MetaFile_Table		tagTable;
} MetaFile_Reference;
typedef MetaFile_Reference* MetaFile_ReferencePtr;

#endif
