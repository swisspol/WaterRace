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


#include					<QD3DAcceleration.h>

#include					"Infinity Structures.h"
#include					"Infinity Error Codes.h"
#include					"Vector.h"

//ROUTINES:

OSErr Get_BestRaveEngine(GDHandle screenDevice, TQAEngine** theEngine)
{
	TQADevice			raveDevice;
	
	//Find the Rave engine
	raveDevice.deviceType = kQADeviceGDevice;
	raveDevice.device.gDevice = screenDevice;
	QAEngineEnable(kQAVendor_Apple, kQAEngine_AppleHW); //Enable Apple accelerator card
	*theEngine = QADeviceGetFirstEngine(&raveDevice);
	if(*theEngine == nil)
	return kError_NoRaveEngineFound;
	
	return noErr;
}

OSErr Get_SoftwareEngine(TQAEngine** theEngine)
{
	long				vendorID,
						engineID;
	
	*theEngine = QADeviceGetFirstEngine(nil);
	if(*theEngine == nil)
	return kError_NoRaveEngineFound;
	
	while(*theEngine != NULL) {
		QAEngineGestalt(*theEngine, kQAGestalt_VendorID, &vendorID);
		QAEngineGestalt(*theEngine, kQAGestalt_EngineID, &engineID);
		
		if((vendorID == kQAVendor_Apple) && (engineID == kQAEngine_AppleSW))
		return noErr;
		
		*theEngine = QADeviceGetNextEngine(nil, *theEngine);
	}
	
	return kError_SelectedRaveEngineNotFound;
}

OSErr Get_RaveEngineByName(GDHandle screenDevice, Str255 name, TQAEngine** theEngine)
{
	Str255				engineName;
	TQADevice			raveDevice;
	long				i;
	
	raveDevice.deviceType = kQADeviceGDevice;
	raveDevice.device.gDevice = screenDevice;
	QAEngineEnable(kQAVendor_Apple, kQAEngine_AppleHW); //Enable Apple accelerator card
	*theEngine = QADeviceGetFirstEngine(&raveDevice);
	if(*theEngine == nil)
	return kError_NoRaveEngineFound;
	while(*theEngine != nil) {
		QAEngineGestalt(*theEngine, kQAGestalt_ASCIINameLength, &i);
		QAEngineGestalt(*theEngine, kQAGestalt_ASCIIName, &engineName[1]);
		engineName[0] = i;
		
		if(EqualString(engineName, name, false, true))
		break;
		
		*theEngine = QADeviceGetNextEngine(&raveDevice, *theEngine);
	}
		
	if(*theEngine == nil) {
		*theEngine = QADeviceGetFirstEngine(&raveDevice);
		return kError_SelectedRaveEngineNotFound;
	}
	
	return noErr;
}

long Is3DfxKnownEngine(TQAEngine* theEngine)
{
	Str255				name;
	long				i;
	long				vendorID,
						engineID;
	
	//Gather engine info
	QAEngineGestalt(theEngine, kQAGestalt_VendorID, &vendorID);
	QAEngineGestalt(theEngine, kQAGestalt_EngineID, &engineID);
	QAEngineGestalt(theEngine, kQAGestalt_ASCIINameLength, &i);
	QAEngineGestalt(theEngine, kQAGestalt_ASCIIName, &name[1]);
	name[0] = i;
	
	//Check for Voodoo 3 - check engine and vendor ID
	if((vendorID == 17) && (engineID == 1)) {
		//Extract the Voodoo type
		if(name[7] == '3')
		return 3;
		if(name[7] == '4')
		return 4;
		if(name[7] == '5')
		return 5;
		
		return 0; //Unknow
	}
	
	//Check for Voodoo 2 - check engine and vendor ID
	if((vendorID == 8) && (engineID == 0))
	return 2;
	
	//Check for third party drivers - Does this string contains "3Dfx"
	for(i = 1; i <= name[0] - 3; ++i)
	if(name[i] == '3')
		if((name[i + 1] == 'D') || (name[i + 1] == 'd'))
			if((name[i + 2] == 'F') || (name[i + 2] == 'f'))
				if((name[i + 3] == 'X') || (name[i + 3] == 'x'))
				return 1; //We assume it's a Voodoo 1
	
	return 0;
}

Boolean IsATIKnownEngine(TQAEngine* theEngine)
{
	long				vendorID,
						engineID;
	
	//Gather engine info
	QAEngineGestalt(theEngine, kQAGestalt_VendorID, &vendorID);
	QAEngineGestalt(theEngine, kQAGestalt_EngineID, &engineID);
	
	//Check for ATI engine
	if(vendorID != kQAVendor_ATI)
	return false;
	
	//Check for Rage 2 / Rage Pro
	if(engineID == 4)
	return true;
	
	//Check for Rage 128
	if(engineID == 5)
	return true;
	
	//Check for Radeon
	if(engineID == 6)
	return true;
	
	return false;
}

OSErr IsAppleSoftwareEngine(TQAEngine* theEngine)
{
	long				vendorID,
						engineID;
	
	QAEngineGestalt(theEngine, kQAGestalt_VendorID, &vendorID);
	QAEngineGestalt(theEngine, kQAGestalt_EngineID, &engineID);
		
	if((vendorID == kQAVendor_Apple) && (engineID == kQAEngine_AppleSW))
	return true;
	
	return false;
}

void SetUpBBox_Homogene(VectorPtr center, float size, Vector bBox[kBBSize])
{
	size /= 2.0;
	
	bBox[0].x = center->x - size;
	bBox[0].y = center->y - size;
	bBox[0].z = center->z - size;
	
	bBox[1].x = center->x + size;
	bBox[1].y = center->y - size;
	bBox[1].z = center->z - size;
	
	bBox[2].x = center->x + size;
	bBox[2].y = center->y - size;
	bBox[2].z = center->z + size;
	
	bBox[3].x = center->x - size;
	bBox[3].y = center->y - size;
	bBox[3].z = center->z + size;
	
	bBox[4].x = center->x - size;
	bBox[4].y = center->y + size;
	bBox[4].z = center->z - size;
	
	bBox[5].x = center->x + size;
	bBox[5].y = center->y + size;
	bBox[5].z = center->z - size;
	
	bBox[6].x = center->x + size;
	bBox[6].y = center->y + size;
	bBox[6].z = center->z + size;
	
	bBox[7].x = center->x - size;
	bBox[7].y = center->y + size;
	bBox[7].z = center->z + size;
}

void SetUpBBox_Corners(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, Vector bBox[kBBSize])
{
	bBox[0].x = minX;
	bBox[0].y = minY;
	bBox[0].z = minZ;
	
	bBox[1].x = maxX;
	bBox[1].y = minY;
	bBox[1].z = minZ;
	
	bBox[2].x = maxX;
	bBox[2].y = minY;
	bBox[2].z = maxZ;
	
	bBox[3].x = minX;
	bBox[3].y = minY;
	bBox[3].z = maxZ;
	
	bBox[4].x = minX;
	bBox[4].y = maxY;
	bBox[4].z = minZ;
	
	bBox[5].x = maxX;
	bBox[5].y = maxY;
	bBox[5].z = minZ;
	
	bBox[6].x = maxX;
	bBox[6].y = maxY;
	bBox[6].z = maxZ;
	
	bBox[7].x = minX;
	bBox[7].y = maxY;
	bBox[7].z = maxZ;
}

Boolean PointOnSide(VectorPtr p, VectorPtr v1, VectorPtr v2, VectorPtr v3)
{
	Vector				side,
						normal,
						dir,
						point;
	
	//Find side, dir & point
	Vector_Subtract(v2, v1, &side);
	side.z = 0.0;
	Vector_Subtract(v3, v1, &dir);
	dir.z = 0.0;
	Vector_Subtract(p, v1, &point);
	point.z = 0.0;
	
	//Get normal
	normal.x = -side.y;
	normal.y = side.x;
	normal.z = 0.0;
	//Is the normal correct
	if(Vector_DotProduct(&normal, &dir) <= 0.0) {
		normal.x = side.y;
		normal.y = -side.x;
	}
	
	//Is point on side
	return (Vector_DotProduct(&normal, &point) > 0.0);
}

float ClickInFace(VectorPtr mouse, Vector vertices[3])
{
	//Make sure face is not a line
	if(((long) vertices[0].x == (long) vertices[1].x) && ((long) vertices[0].x == (long) vertices[2].x))
	return -1.0;
	if(((long) vertices[0].y == (long) vertices[1].y) && ((long) vertices[0].y == (long) vertices[2].y))
	return -1.0;
	
	//Check if click is inside the point
	if(!PointOnSide(mouse, &vertices[0], &vertices[1], &vertices[2]))
	return -1.0;
	if(!PointOnSide(mouse, &vertices[0], &vertices[2], &vertices[1]))
	return -1.0;
	if(!PointOnSide(mouse, &vertices[1], &vertices[2], &vertices[0]))
	return -1.0;
	
	return (vertices[0].z + vertices[1].z + vertices[2].z) / 3.0;
}
