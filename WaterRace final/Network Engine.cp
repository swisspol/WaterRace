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


#include				"WaterRace.h"
#include				"Game.h"
#include				"Network Engine.h"

#include				"Vector.h"
#include				"Quaternion.h"

//VARIABLES:

static Str31			localProcessor,
						localOS;
unsigned char			dataBuffer[kMaxBufferSize];
ShipState				networkShipBuffers[kMaxPlayers];

//ROUTINES:

/********************************************************************/
/*																	*/
/*							Utility routines						*/
/*																	*/
/********************************************************************/

OTResult Network_GetFourByteOption(EndpointRef ep, OTXTILevel level, OTXTIName name, UInt32 *value)
{
	OTResult err;
	TOption option;
	TOptMgmt request;
	TOptMgmt result;
	
	/* Set up the option buffer */
	option.len = kOTFourByteOptionSize;
	option.level = level;
	option.name = name;
	option.status = 0;
	option.value[0] = 0;// Ignored because we're getting the value.
	
	/* Set up the request parameter for OTOptionManagement to point
	to the option buffer we just filled out */
	request.opt.buf = (UInt8 *) &option;
	request.opt.len = sizeof(option);
	request.flags = T_CURRENT;
	
	/* Set up the reply parameter for OTOptionManagement. */
	result.opt.buf = (UInt8 *) &option;
	result.opt.maxlen = sizeof(option);
	
	err = OTOptionManagement(ep, &request, &result);
	if(err == noErr)
	switch (option.status) {
		
		case T_SUCCESS:
		case T_READONLY:
		*value = option.value[0];
		break;
		
		default:
		err = option.status;
		break;
	
	}
	
	return err;
}

OTResult Network_SetFourByteOption(EndpointRef ep, OTXTILevel level, OTXTIName name, UInt32 value)
{
	OTResult err;
	TOption option;
	TOptMgmt request;
	TOptMgmt result;
	
	/* Set up the option buffer to specify the option and value to
	set. */
	option.len = kOTFourByteOptionSize;
	option.level = level;
	option.name = name;
	option.status = 0;
	option.value[0] = value;
	
	/* Set up request parameter for OTOptionManagement */
	request.opt.buf = (UInt8 *) &option;
	request.opt.len = sizeof(option);
	request.flags = T_NEGOTIATE;
	
	/* Set up reply parameter for OTOptionManagement. */
	result.opt.buf = (UInt8 *) &option;
	result.opt.maxlen = sizeof(option);
	
	err = OTOptionManagement(ep, &request, &result);
	if(err == noErr) {
		if(option.status != T_SUCCESS)
		err = option.status;
	}
	
	return err;
}

InetHost Network_GetComputerIP()
{
	InetInterfaceInfo	interfaceInfo;
	
	//Get Internet config - returns a error if OT was not loaded! (kOTNotFoundErr)
	if(OTInetGetInterfaceInfo(&interfaceInfo, kDefaultInetInterface))
	return kOTAnyInetAddress;
	
	return interfaceInfo.fAddress;
}

void Network_GetComputerIPString(Str31 string)
{
	char*			theChar = (char*) &string[1];
	
	OTInetHostToString(Network_GetComputerIP(), theChar);
	string[0] = 0;
	while(*theChar++)
	++string[0];
}

void Network_GetComputerConfig(Str31 OS, Str31 processor)
{
	BlockMove(localOS, OS, localOS[0] + 1);
	BlockMove(localProcessor, processor, localProcessor[0] + 1);
}

/********************************************************************/
/*																	*/
/*							General routines						*/
/*																	*/
/********************************************************************/

OSErr Network_Init()
{
	OSErr				theError;
	long				response;
	NumVersion			version;
	Str31				text;
	
	//Init OT
	theError = InitOpenTransport();
	if(theError)
	return theError;
	
	//Find our OS
	Gestalt(gestaltSystemVersion, (long*) &version);
	
	BlockMove("\pMacOS ", localOS, sizeof(Str31));
	NumToString(version.stage, text);
	BlockMove(&text[1], &localOS[localOS[0] + 1], text[0]);
	localOS[0] += text[0];
	++localOS[0];
	localOS[localOS[0]] = '.';
	NumToString((version.nonRelRev & 0xF0) >> 4, text);
	BlockMove(&text[1], &localOS[localOS[0] + 1], text[0]);
	localOS[0] += text[0];
	
	//Find our processor
	response = 0;
	Gestalt(gestaltNativeCPUtype, &response);
	switch(response) {
		case gestaltCPU601: BlockMove("\pPowerPC 601", localProcessor, sizeof(Str31)); break;
		case gestaltCPU603: case gestaltCPU603e: case gestaltCPU603ev: BlockMove("\pPowerPC 603", localProcessor, sizeof(Str31)); break;
		case gestaltCPU604: case gestaltCPU604e: case gestaltCPU604ev: BlockMove("\pPowerPC 604", localProcessor, sizeof(Str31)); break;
		case gestaltCPU750: BlockMove("\pPowerPC G3", localProcessor, sizeof(Str31)); break;
		default: BlockMove("\pUnknown", localProcessor, sizeof(Str31)); break;
	}
	
	//Append processor speed
	Gestalt(gestaltProcClkSpeed, &response);
	response /= 1000000;
	++localProcessor[0];
	localProcessor[localProcessor[0]] = ' ';
	NumToString(response, text);
	BlockMove(&text[1], &localProcessor[localProcessor[0] + 1], text[0]);
	localProcessor[0] += text[0];
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'M';
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'H';
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'z';
	
	//Append RAM size
	Gestalt(gestaltPhysicalRAMSize, &response);
	response /= 1024 * 1024;
	++localProcessor[0];
	localProcessor[localProcessor[0]] = ' ';
	NumToString(response, text);
	BlockMove(&text[1], &localProcessor[localProcessor[0] + 1], text[0]);
	localProcessor[0] += text[0];
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'M';
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'b';
	++localProcessor[0];
	localProcessor[localProcessor[0]] = ' ';
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'R';
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'A';
	++localProcessor[0];
	localProcessor[localProcessor[0]] = 'M';
	
	return noErr;
}

void Network_Quit()
{
	CloseOpenTransport();
}

void Network_ClearShipBuffer()
{
	ShipStatePtr			state = networkShipBuffers;
	long					i;
	
	for(i = 0; i < kMaxPlayers; ++i, ++state) {
		Vector_Clear(&state->velocityG);
		Vector_Clear(&state->accelerationG);
		Vector_Clear(&state->rotationspeedL);
		Vector_Clear(&state->rotationAccelerationL);
		Vector_Clear(&state->position);
		Quaternion_EulerToQuaternion(0.0, 0.0, 0.0, &state->orientation);
		state->timeStamp = 0;
		state->motorPower = 0;
		state->mouseH = 0;
		state->turboState = 0;
		state->shipNum = 0;
	}
}
