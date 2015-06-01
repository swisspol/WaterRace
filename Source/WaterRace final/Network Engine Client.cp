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
#include				"Strings.h"
#include				"Game.h"
#include				"Network Engine.h"

#include				"Clock.h"
#include				"Quaternion.h"

#include				"Keys.h"

//CONSTANTES:

#define					kMaxClientEvents				8
#define					kConnectionAttemptTimeOut		10 //10 seconds
#define					kOTConnectionRefusedError		61

enum {
	kClientStatus_Offline = 0,
	kClientStatus_ServerContacted,
	kClientStatus_ConnectedToServer,
	kClientStatus_WaitingForStart,
	kClientStatus_RaceRunning
};

#define					kMaxClockTolerance				5

//VARIABLES LOCALES:

static long				clientEventCount = 0;
static OSType			clientEventTypeBuffer[kMaxClientEvents];
static long				clientEventDataBuffer[kMaxClientEvents];
static short			clientStatus = kClientStatus_Offline;
static EndpointRef		serverEndPoint = kOTInvalidEndpointRef;
static short			disconnectionReason;
static ServerInfo		serverInfo;
static GameInfo			gameInfo;
static unsigned long	clientNextSendTime;
#if __USE_SEGMENT_SIZE_PACKETS__
static unsigned long	segmentSize;
#endif

//PROTOTYPES:

static pascal void Client_EndpointNotifier(void* config, OTEventCode code, OTResult result, void* cookie);

//ROUTINES:

static Boolean EscapeKeyDown()
{
	KeyMap			theKeys;
	long			value;
	
	GetKeys(theKeys);
	value = IsKeyDown(theKeys, keyEscape);
	if(value)
	return true;
	else
	return false;
}

static void Client_PostEvent(OSType event, long data)
{
	if(clientEventCount >= kMaxClientEvents)
	return;
	
	clientEventTypeBuffer[clientEventCount] = event;
	clientEventDataBuffer[clientEventCount] = data;
	++clientEventCount;
}

static OSErr Client_ReceiveServerData()
{
	long				dataBytes;
	OTFlags				flags;
	OSStatus			theError = noErr;
	unsigned char*		dataPtr;
	
	//Retrieve data
	dataBytes = OTRcv(serverEndPoint, dataBuffer, kMaxBufferSize, &flags);
	if(dataBytes < 0) {
		if(dataBytes == kOTLookErr) {
			theError = OTLook(serverEndPoint);
			Client_EndpointNotifier(nil, theError, 0, nil);
		}
		else if(dataBytes == kOTNoDataErr)
		theError = noErr;
		else
		theError = dataBytes;
		
		return theError;
	}
	
	//Process data
	dataPtr = dataBuffer;
	do {
		switch(*((OSType*) dataPtr)) {
			
			case kPacketType_ShipState:
			if(dataBytes < sizeof(ShipState))
			return kError_FatalError;
			{
				ShipStatePtr			state = (ShipStatePtr) dataPtr;
				
				networkShipBuffers[state->shipNum] = *state;
			}
			dataBytes -= sizeof(ShipState);
			dataPtr += sizeof(ShipState);
			break;
			
			case kPacketType_ServerInfo:
			if(dataBytes < sizeof(ServerInfo))
			return kError_FatalError;
			BlockMove(dataPtr, &serverInfo, sizeof(ServerInfo));
			Client_PostEvent(kNetEvent_Client_ReceivedServerInfo, (long) &serverInfo);
			clientStatus = kClientStatus_ConnectedToServer;
			dataBytes -= sizeof(ServerInfo);
			dataPtr += sizeof(ServerInfo);
			break;
			
			case kPacketType_GameInfo:
			if(dataBytes < sizeof(GameInfo))
			return kError_FatalError;
			BlockMove(dataPtr, &gameInfo, sizeof(GameInfo));
			Client_PostEvent(kNetEvent_Client_ReceivedGameInfo, (long) &serverInfo);
			clientStatus = kClientStatus_WaitingForStart;
			dataBytes -= sizeof(GameInfo);
			dataPtr += sizeof(GameInfo);
			break;
			
			case kPacketType_ServerClock:
			if(dataBytes < sizeof(Packet))
			return kError_FatalError;
#if __CLOCK_TOLERANCE__
			if(fAbs(RClock_GetTime() - ((PacketPtr) dataPtr)->data) > kMaxClockTolerance)
#endif
			RClock_SetTime(((PacketPtr) dataPtr)->data);
			dataBytes -= sizeof(Packet);
			dataPtr += sizeof(Packet);
			break;
			
			case kPacketType_ServerReady:
			if(dataBytes < sizeof(Packet))
			return kError_FatalError;
			clientStatus = kClientStatus_RaceRunning;
			dataBytes -= sizeof(Packet);
			dataPtr += sizeof(Packet);
			break;
			
			case kPacketType_ServerStartTime:
			if(dataBytes < sizeof(Packet))
			return kError_FatalError;
			{
				long			i;
				
				gameConfig.startTime = ((PacketPtr) dataPtr)->data;
				for(i = 0; i < shipCount; ++i) {
					shipList[i].player->lastLapTime = gameConfig.startTime;
					shipList[i].motorStartTime = gameConfig.startTime;
				}
			}
			dataBytes -= sizeof(Packet);
			dataPtr += sizeof(Packet);
			break;
			
			case kPacketType_DisconnectionReason:
			if(dataBytes < sizeof(Packet))
			return kError_FatalError;
			disconnectionReason = ((PacketPtr) dataPtr)->data;
			dataBytes -= sizeof(Packet);
			dataPtr += sizeof(Packet);
			break;
			
			default:
			return kNetError_UnknownData;
			break;
			
		}
	} while(dataBytes > 0);
	
	return theError;
}

static pascal void Client_EndpointNotifier(void* config, OTEventCode code, OTResult result, void* cookie)
{
	switch(code) {
		
		case T_DATA:
		Client_ReceiveServerData();
		break;
		
		case kOTProviderWillClose:
		case kOTProviderIsClosed:
		
		clientStatus = kClientStatus_Offline;
		Client_PostEvent(kNetEvent_Client_ServerDisconnection, kNetError_TCPConnectionLost);
		break;
		
		case T_DISCONNECT:
		OTRcvDisconnect(serverEndPoint, nil);
		if(clientStatus == kClientStatus_RaceRunning) {
			long		i;
			
			for(i = 0; i < gameConfig.playerCount; ++i) {
				BlockMove(kString_Network_ServerOffLine, gameConfig.playerList[i].networkText, sizeof(Str31));
				if(gameConfig.playerList[i].shipPilotType == kPilot_Remote)
				gameConfig.playerList[i].shipPilotType = kPilot_RemoteOffLine;
			}
		}
		clientStatus = kClientStatus_Offline;
		Client_PostEvent(kNetEvent_Client_ServerDisconnection, disconnectionReason);
		break;

	}
}

OSErr Client_Connect(Str31 destIP, unsigned short port, Str31 password, Str31 playerName, Str31 playerLocation, OSType shipID, OSType setID)
{
	InetAddress			hostAddress,
						realHostAddress,
						localAddress,
						realLocalAddress;
	InetHost			hostIP;
	TEndpointInfo		endPointInfo;
	OSStatus			theError;
	TBind				requestBind,
						returnedBind;
	TCall				sndcall,
						rcvcall;
	ClientInfo			info;
	long				bytes;
	TDiscon				disconnectionInfo;
												
	if(serverEndPoint != kOTInvalidEndpointRef)
	return kError_FatalError;
	
	//Create OT Internet address
	destIP[destIP[0] + 1] = 0; //Convert pascal string to C string
	OTInetStringToHost((Ptr) &destIP[1], &hostIP);
	OTInitInetAddress(&hostAddress, port, hostIP);
	OTInitInetAddress(&localAddress, port, Network_GetComputerIP()); //kOTAnyInetAddress
	
	//Open EndPoint
	serverEndPoint = OTOpenEndpoint(OTCreateConfiguration(kTCPName), 0, &endPointInfo, &theError);
	if(theError == noErr)
	theError = OTSetSynchronous(serverEndPoint); //not necessary
	if(theError == noErr)
	theError = OTSetBlocking(serverEndPoint);
	if(theError) {
		Client_Disconnect();
		return theError;
	}
	
	//Bind EndPoint to local address
	requestBind.addr.len = sizeof(struct InetAddress);
	requestBind.addr.buf = (unsigned char*) &localAddress;
	requestBind.qlen = 2;
	returnedBind.addr.maxlen = sizeof(struct InetAddress);
	returnedBind.addr.buf = (unsigned char*) &realLocalAddress;
	theError = OTBind(serverEndPoint, &requestBind, &returnedBind);
	if(theError) {
		Client_Disconnect();
		return theError;
	}
	
	//Set connection attemp timeout
	theError = Network_SetFourByteOption(serverEndPoint, INET_TCP, TCP_CONN_ABORT_THRESHOLD, kConnectionAttemptTimeOut * 1000);
	if(theError) {
		Client_Disconnect();
		return theError;
	}
	
	//Connect to host
	sndcall.addr.len = sizeof(struct InetAddress);
	sndcall.addr.buf = (unsigned char *) &hostAddress;
	sndcall.opt.len = 0;
	sndcall.opt.buf = nil;
	sndcall.udata.len = 0;
	sndcall.udata.buf = nil;
	rcvcall.addr.maxlen = sizeof(struct InetAddress);
	rcvcall.addr.buf = (unsigned char *) &realHostAddress;
	rcvcall.opt.maxlen = 0;
	rcvcall.opt.buf = nil;
	rcvcall.udata.maxlen = 0;
	rcvcall.udata.buf = nil;
	theError = OTConnect(serverEndPoint, &sndcall, &rcvcall);
	if(theError) {
		if(theError == kOTLookErr) {
			theError = OTLook(serverEndPoint);
			if(theError == T_DISCONNECT) {
				disconnectionInfo.udata.maxlen = 0;
				disconnectionInfo.udata.buf = nil;
				theError = OTRcvDisconnect(serverEndPoint, &disconnectionInfo); //disconnectionInfo may be NULL
				if(theError == noErr) {
					if(disconnectionInfo.reason == kOTConnectionRefusedError)
					theError = kNetError_ConnectionRefused;
					else
					theError = disconnectionInfo.reason;
				}
			}
		}
		Client_Disconnect();
		return theError;
	}
	
#if __USE_SEGMENT_SIZE_PACKETS__
	//Get TCP segment size
	theError = Network_GetFourByteOption(serverEndPoint, INET_TCP, TCP_MAXSEG, &segmentSize);
	if(theError)
	return theError;
#endif

	//Specify EndPoints options
	theError = Network_SetFourByteOption(serverEndPoint, INET_TCP, TCP_NODELAY, T_YES);
	if(theError)
	return theError;
	theError = Network_SetFourByteOption(serverEndPoint, XTI_GENERIC, XTI_SNDLOWAT, 0);
	if(theError)
	return theError;
	theError = Network_SetFourByteOption(serverEndPoint, XTI_GENERIC, XTI_RCVLOWAT, 0);
	if(theError)
	return theError;
	
	//Install notifier
	theError = OTSetAsynchronous(serverEndPoint);
	if(theError)
	return theError;
	theError = OTInstallNotifier(serverEndPoint, Client_EndpointNotifier, nil);
	if(theError)
	return theError;
	
	//Make sure no event is pending
	theError = OTLook(serverEndPoint);
	if(theError) {
		OTEnterNotifier(serverEndPoint);
		Client_EndpointNotifier(nil, theError, 0, nil);
		OTLeaveNotifier(serverEndPoint);
	}
	
	//Send client info
	info.packetType = kPacketType_ClientInfo;
	info.gameVersion = kNetGameVersion;
	BlockMove(playerName, info.playerName, sizeof(Str31));
	BlockMove(playerLocation, info.playerLocation, sizeof(Str31));
	BlockMove(password, info.password, sizeof(Str31));
	info.shipID = shipID;
	info.setID = setID;
	bytes = OTSnd(serverEndPoint, &info, sizeof(ClientInfo), 0);
	if(bytes < 0) {
		theError = bytes;
		if(theError == kOTLookErr) {
			theError = OTLook(serverEndPoint);
			Client_EndpointNotifier(nil, theError, 0, nil);
		}
	}
	
	//Setup
	clientEventCount = 0;
	disconnectionReason = kNetError_Client_Unknown;
	clientStatus = kClientStatus_ServerContacted;
	clientNextSendTime = 0;
	Network_ClearShipBuffer();
	
	return theError;
}

void Client_Disconnect()
{
	//Kill client EndPoint
	if(serverEndPoint != kOTInvalidEndpointRef) {
		OTSetSynchronous(serverEndPoint);
		OTRemoveNotifier(serverEndPoint);
		if(clientStatus)
		OTSndDisconnect(serverEndPoint, nil);
		OTUnbind(serverEndPoint);
		OTSetNonBlocking(serverEndPoint);
		OTCloseProvider(serverEndPoint);
	}
	
	//Setup
	clientStatus = kClientStatus_Offline;
	serverEndPoint = kOTInvalidEndpointRef;
}

Boolean Client_GetNextEvent(OSType* event, long* data)
{
	long				i;
	
	if(!clientEventCount)
	return false;
	
	*event = clientEventTypeBuffer[0];
	*data = clientEventDataBuffer[0];
	for(i = 0; i < clientEventCount - 1; ++i) {
		clientEventTypeBuffer[i] = clientEventTypeBuffer[i + 1];
		clientEventDataBuffer[i] = clientEventDataBuffer[i + 1];
	}
	--clientEventCount;
	
	return true;
}

OSErr Client_WaitForServer()
{
	Packet				packet;
	OSStatus			theError = noErr;
	long				bytes;
	
	if(clientStatus != kClientStatus_WaitingForStart)
	return kError_FatalError;
	
	//Setup packet
	packet.packetType = kPacketType_PlayerReady;
	
	//Send ready signal to server
	bytes = OTSnd(serverEndPoint, &packet, sizeof(Packet), 0);
	if(bytes < 0) {
		theError = bytes;
		if(bytes == kOTLookErr) {
			theError = OTLook(serverEndPoint);
			Client_EndpointNotifier(nil, theError, 0, nil);
		}
		return theError;
	}
	
	//Wait for server signal
	do {
		if(EscapeKeyDown())
		theError = kError_UserCancel;
		if(clientStatus == kClientStatus_Offline) //we were disconnected from server
		theError = kError_UserCancel;
	} while((theError == noErr) && (clientStatus != kClientStatus_RaceRunning));
	
	return theError;
}

void Client_SendData(ExtendedShipPtr ship)
{
	ShipState			state;
	long				bytes;
	
	if(clientStatus != kClientStatus_RaceRunning)
	return;
	
	if(worldState->frameTime < clientNextSendTime)
	return;
	
	//Setup ShipState info
	state.packetType = kPacketType_ShipState;
	state.velocityG = ship->dynamics.velocityG;
	state.accelerationG = ship->dynamics.accelerationG;
	state.rotationspeedL = ship->dynamics.rotationspeedL;
	state.rotationAccelerationL = ship->dynamics.rotationAccelerationL;
	state.position = ((ObjectPtr) ship)->pos.w;
	Quaternion_MatrixToQuaternion(&((ObjectPtr) ship)->pos, &state.orientation);
	state.timeStamp = worldState->frameTime;
	state.turboState = ship->turboState;
	state.shipNum = ship->shipNum;
	state.motorPower = ship->dynamics.motorPower / kPowerUnitToWatt;
	state.mouseH = ship->mouseH * kShortMax;
	
	//Send data to server
#if __USE_SEGMENT_SIZE_PACKETS__
	bytes = OTSnd(serverEndPoint, &state, segmentSize, 0);
#else
	bytes = OTSnd(serverEndPoint, &state, sizeof(ShipState), 0);
#endif
	if(bytes == kOTLookErr)
	Client_EndpointNotifier(nil, OTLook(serverEndPoint), 0, nil);
	
	clientNextSendTime = worldState->frameTime + kNetworkShipRefreshRate;
}

OSErr Client_SetupGame()
{
	long						i;
	PlayerConfigurationPtr		player;
	
	if(clientStatus != kClientStatus_WaitingForStart)
	return kError_FatalError;
	
	//Setup game info
	gameConfig.version = kNetGameVersion;
	gameConfig.gameMode = kGameMode_Join;
	gameConfig.gameSubMode = serverInfo.serverGameSubMode;
	gameConfig.gameMode_save = gameConfig.gameMode;
	gameConfig.gameSubMode_save = gameConfig.gameSubMode;
	gameConfig.terrainID = serverInfo.terrainID;
	gameConfig.terrainDifficulty = serverInfo.terrainDifficulty;
	gameConfig.playerCount = gameInfo.playerCount;
	
	//Clear players
	for(i = 0; i < kMaxPlayers; ++i)
	Clear_Player(&gameConfig.playerList[i]);
	
	//Setup players
	for(i = 0; i < gameInfo.playerCount; ++i) {
		player = &gameConfig.playerList[i];
		
		player->characterID = kNoID;
		player->shipID = gameInfo.playerList[i].shipID;
		player->shipTextureID = gameInfo.playerList[i].setID;
		if(gameInfo.playerList[i].isLocal)
		player->shipPilotType = kPilot_Local;
		else
		player->shipPilotType = kPilot_Remote;
		BlockMove(gameInfo.playerList[i].playerName, player->playerName, sizeof(Str31));
		player->playerDataPtr = nil;
	}
	
	return noErr;
}

void Client_Enter()
{
	if(clientStatus == kClientStatus_RaceRunning)
	OTEnterNotifier(serverEndPoint);
}

void Client_Leave()
{
	if(clientStatus == kClientStatus_RaceRunning)
	OTLeaveNotifier(serverEndPoint);
}
