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
#include				"Data Files.h"

#include				"Clock.h"
#include				"Quaternion.h"

#include				"Keys.h"

//CONSTANTES:

#define					kMaxServerEvents				16

enum {
	kServerStatus_Offline = 0,
	kServerStatus_Online,
	kServerStatus_WaitingForClientReady,
	kServerStatus_RaceRunning
};

enum {
	kClientStatus_LocalBot = -2,
	kClientStatus_LocalPlayer = -1,
	kClientStatus_Offline = 0,
	kClientStatus_Contacted,
	kClientStatus_ConnectedToClient,
	kClientStatus_Ready,
	kClientStatus_RaceRunning
};

//SCTRUCTURES:

typedef struct {
	//Setup during game connections
	ClientInfo				info;
	InetAddress				address;
	EndpointRef				endPoint;
	short					status;
	
	//Valid only when game is running
	PlayerConfigurationPtr	player;
} ClientConnection;
typedef ClientConnection* ClientConnectionPtr;

//VARIABLES LOCALES:

static long				serverEventCount = 0;
static OSType			serverEventTypeBuffer[kMaxServerEvents];
static long				serverEventDataBuffer[kMaxServerEvents];
static short			serverStatus = kClientStatus_Offline;
static EndpointRef		serverEndPoint = kOTInvalidEndpointRef;
static ClientConnection	connectionList[kMaxPlayers];
static Str31			serverPassword;
static ServerInfo		serverInfo;
static unsigned long	serverNextSendTime[kMaxPlayers],
						serverNextSendClockTime;
static short			serverMaxPlayers;
#if __USE_SEGMENT_SIZE_PACKETS__
static unsigned long	segmentSize;
#endif

//PROTOTYPES:

static pascal void Server_ClientEndpointNotifier(void* config, OTEventCode code, OTResult result, void* cookie);

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

static void Server_PostEvent(OSType event, long data)
{
	if(serverEventCount >= kMaxServerEvents)
	return;
	
	serverEventTypeBuffer[serverEventCount] = event;
	serverEventDataBuffer[serverEventCount] = data;
	++serverEventCount;
}

static long Server_CurrentNumberPlayers()
{
	long			count = 0,
					i;
	
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status != kClientStatus_Offline)
	++count;
	
	return count;
}

static void Server_DisconnectClient(ClientConnectionPtr client, long reason)
{
	Packet				packet;
	
	//Make sure this client is online
	if(client->status <= kClientStatus_Offline)
	return;
	
	//Send disconnection reason
	packet.packetType = kPacketType_DisconnectionReason;
	packet.data = reason;
#if __USE_SEGMENT_SIZE_PACKETS__
	OTSnd(client->endPoint, &packet, segmentSize, 0);
#else
	OTSnd(client->endPoint, &packet, sizeof(Packet), 0);
#endif
	
	//Disconnect
	OTSndDisconnect(client->endPoint, nil);
	
	//Update client
	client->status = kClientStatus_Offline;
	Server_PostEvent(kNetEvent_Server_ClientDisconnection, (long) &client->info);
}

static OSErr Server_ReceiveClientData(ClientConnectionPtr client)
{
	long					dataBytes;
	OTFlags					flags;
	OSStatus				theError = noErr;
	unsigned char*		dataPtr;
	
	//Retrieve data
	dataBytes = OTRcv(client->endPoint, dataBuffer, kMaxBufferSize, &flags);
	if(dataBytes < 0) {
		if(dataBytes == kOTLookErr) {
			theError = OTLook(client->endPoint);
			Server_ClientEndpointNotifier(client, theError, 0, nil);
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
				long					i,
										bytes;
				
				networkShipBuffers[state->shipNum] = *state;
				
				//Forward to all clients
				for(i = 0; i < kMaxPlayers; ++i)
				if((connectionList[i].status == kClientStatus_RaceRunning) && (&connectionList[i] != client)) {
#if __USE_SEGMENT_SIZE_PACKETS__
					bytes = OTSnd(connectionList[i].endPoint, state, segmentSize, 0);
#else
					bytes = OTSnd(connectionList[i].endPoint, state, sizeof(ShipState), 0);
#endif
					if(bytes == kOTLookErr)
					Server_ClientEndpointNotifier(client, OTLook(connectionList[i].endPoint), 0, nil);
				}
			}
			dataBytes -= sizeof(ShipState);
			dataPtr += sizeof(ShipState);
			break;
			
			case kPacketType_ClientInfo:
			if(dataBytes < sizeof(ClientInfo))
			return kError_FatalError;
			{
				ClientInfoPtr			info = (ClientInfoPtr) dataPtr;
				long					bytes;
				
				//Make sure we are running the same version of the game
				if(info->gameVersion != kNetGameVersion) {
					client->info = *info;
					Server_DisconnectClient(client, kNetError_Client_BadVersion);
					return kNetError_Disconnected;
				}
				
				//Compare passwords
				if(serverPassword[0] && !EqualString(info->password, serverPassword, true, true)) {
					client->info = *info;
					Server_DisconnectClient(client, kNetError_Client_WrongPassword);
					return kNetError_Disconnected;
				}
				
				//OK: we can accept this player
				client->info = *info;
				client->status = kClientStatus_ConnectedToClient;
				Server_PostEvent(kNetEvent_Server_ClientConnection, (long) &client->info);
				
				//Send server info
				bytes = OTSnd(client->endPoint, &serverInfo, sizeof(ServerInfo), 0);
				if(bytes == kOTLookErr)
				Server_ClientEndpointNotifier(client, OTLook(client->endPoint), 0, nil);
			}
			dataBytes -= sizeof(ClientInfo);
			dataPtr += sizeof(ClientInfo);
			break;
			
			case kPacketType_PlayerReady:
			if(dataBytes < sizeof(Packet))
			return kError_FatalError;
			client->status = kClientStatus_Ready;
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

static pascal void Server_ClientEndpointNotifier(void* client, OTEventCode code, OTResult result, void* cookie)
{
	switch(code) {
		
		case T_DATA:
		Server_ReceiveClientData((ClientConnectionPtr) client);
		break;
		
		case kOTProviderWillClose:
		case kOTProviderIsClosed:
		if(((ClientConnectionPtr) client)->status == kClientStatus_RaceRunning) {
			BlockMove(kString_Network_PlayerOffLine, ((ClientConnectionPtr) client)->player->networkText, sizeof(Str31));
			((ClientConnectionPtr) client)->player->shipPilotType = kPilot_RemoteOffLine;
		}
		((ClientConnectionPtr) client)->status = kClientStatus_Offline;
		Server_PostEvent(kNetEvent_Server_ClientDisconnection, (long) &((ClientConnectionPtr) client)->info);
		break;
		
		case T_DISCONNECT:
		OTRcvDisconnect(((ClientConnectionPtr) client)->endPoint, nil);
		if(serverStatus == kServerStatus_WaitingForClientReady) {
			//One client was disconnect while we were waiting for the race to begin -> we need to go offline
			serverStatus = kServerStatus_Offline;
		}
		if(((ClientConnectionPtr) client)->status == kClientStatus_RaceRunning) {
			BlockMove(kString_Network_PlayerOffLine, ((ClientConnectionPtr) client)->player->networkText, sizeof(Str31));
			((ClientConnectionPtr) client)->player->shipPilotType = kPilot_RemoteOffLine;
		}
		((ClientConnectionPtr) client)->status = kStatus_OffLine;
		Server_PostEvent(kNetEvent_Server_ClientDisconnection, (long) &((ClientConnectionPtr) client)->info);
		break;
		
		case T_PASSCON:
		if(result == noErr)
		((ClientConnectionPtr) client)->status = kClientStatus_ConnectedToClient;
		break;

	}
}

static OSErr Server_HandleConnectionRequest()
{
	TCall					theCall;
	InetAddress				callingAddress;
	long					i;
	OSErr					theError;
	ClientConnectionPtr		client = nil;
	
	//Listen for the call
	theCall.addr.maxlen = sizeof(struct InetAddress);
	theCall.addr.len = sizeof(struct InetAddress);
	theCall.addr.buf = (unsigned char*) &callingAddress;
	theCall.opt.maxlen = 0;
	theCall.opt.buf = nil;
	theCall.udata.maxlen = 0;
	theCall.udata.buf = nil;
	theError = OTListen(serverEndPoint, &theCall);
	if(theError)
	return theError;
	
	//Do we accept connections?
	if(serverStatus != kServerStatus_Online) {
		theCall.addr.len = 0;
		theCall.opt.len = 0;
		theCall.udata.len = 0;
		return OTSndDisconnect(serverEndPoint, &theCall);
	}
	
	//Have we reached the max number of players?
	if(Server_CurrentNumberPlayers() >= serverMaxPlayers) {
		theCall.addr.len = 0;
		theCall.opt.len = 0;
		theCall.udata.len = 0;
		return OTSndDisconnect(serverEndPoint, &theCall);
	}
	
	//Find an empty slot
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_Offline) {
		client = &connectionList[i];
		break;
	}
	
	//No slot available: disconnect player
	if(client == nil) {
		theCall.addr.len = 0;
		theCall.opt.len = 0;
		theCall.udata.len = 0;
		return OTSndDisconnect(serverEndPoint, &theCall);
	}
	
	//Setup client
	client->status = kClientStatus_Contacted;
	client->address = callingAddress;
	
	//Accept connection
	theCall.addr.len = 0;
	theCall.opt.len = 0;
	theCall.udata.len = 0;
	theError = OTAccept(serverEndPoint, client->endPoint, &theCall);
	
	return noErr;
}

static pascal void Server_EndpointNotifier(void* config, OTEventCode code, OTResult result, void* cookie)
{
	switch(code) {
		
		case T_LISTEN:
		Server_HandleConnectionRequest();
		break;
		
		case kOTProviderWillClose:
		case kOTProviderIsClosed:
		serverStatus = kServerStatus_Offline;
		break;
		
		/*case T_DISCONNECT:
		OTRcvDisconnect(serverEndPoint, nil);
		serverStatus = kServerStatus_Offline;
		break;*/

	}
}

OSErr Server_Start(unsigned short port, Str31 hostName, Str31 hostLocation, Str31 hostPassword, Str31 hostMessage, OSType terrainID, short difficulty)
{
	InetAddress				localAddress,
							realLocalAddress;
	TEndpointInfo			endPointInfo;
	OSStatus				theError;
	TBind					requestBind,
							returnedBind;
	OTConfigurationRef		OTConfig;
	ClientConnectionPtr		client;
	long					i;
	TerrainPreloadedDataPtr	terrainData;
	
	if(serverEndPoint != kOTInvalidEndpointRef)
	return kError_FatalError;
	
	//Create OT Internet address
	OTInitInetAddress(&localAddress, port, Network_GetComputerIP()); //kOTAnyInetAddress
	
	//Create the OT configuration
	OTConfig = OTCreateConfiguration(kTCPName);
	
	//Open master EndPoint
	serverEndPoint = OTOpenEndpoint(OTCloneConfiguration(OTConfig), 0, &endPointInfo, &theError);
	if(theError == noErr)
	theError = OTSetSynchronous(serverEndPoint); //not necessary
	if(theError == noErr)
	theError = OTSetBlocking(serverEndPoint);
	if(theError) {
		Server_ShutDown();
		return theError;
	}
	
	//Bind EndPoint to local address
	requestBind.addr.len = sizeof(struct InetAddress);
	requestBind.addr.buf = (unsigned char*) &localAddress;
	requestBind.qlen = kMaxPlayers;
	returnedBind.addr.maxlen = sizeof(struct InetAddress);
	returnedBind.addr.buf = (unsigned char*) &realLocalAddress;
	theError = OTBind(serverEndPoint, &requestBind, &returnedBind);
	if(theError) {
		Server_ShutDown();
		return theError;
	}
	
	//Open client EndPoints
	for(i = 0; i < kMaxPlayers; ++i) {
		client = &connectionList[i];
		
		//Create OT Internet address
		OTInitInetAddress(&localAddress, port + i + 1, Network_GetComputerIP());
		
		//Open EndPoint
		client->endPoint = OTOpenEndpoint(OTCloneConfiguration(OTConfig), 0, &endPointInfo, &theError);
		if(theError == noErr)
		theError = OTSetSynchronous(client->endPoint);
		if(theError == noErr)
		theError = OTSetBlocking(client->endPoint);
		if(theError) {
			Server_ShutDown();
			return theError;
		}
		
		//Bind EndPoint to local address
		requestBind.addr.len = sizeof(struct InetAddress);
		requestBind.addr.buf = (unsigned char*) &localAddress;
		requestBind.qlen = 0;
		theError = OTBind(client->endPoint, &requestBind, nil);
		if(theError) {
			Server_ShutDown();
			return theError;
		}
		
		//Specify EndPoints options
		theError = Network_SetFourByteOption(client->endPoint, INET_TCP, TCP_NODELAY, T_YES);
		if(theError)
		return theError;
		theError = Network_SetFourByteOption(client->endPoint, XTI_GENERIC, XTI_SNDLOWAT, 0);
		if(theError)
		return theError;
		theError = Network_SetFourByteOption(client->endPoint, XTI_GENERIC, XTI_RCVLOWAT, 0);
		if(theError)
		return theError;
		
		//Install notifier
		theError = OTSetAsynchronous(client->endPoint);
		if(theError)
		return theError;
		theError = OTInstallNotifier(client->endPoint, Server_ClientEndpointNotifier, client);
		if(theError)
		return theError;
		
		//Make sure no event is pending
		theError = OTLook(client->endPoint);
		if(theError) {
			OTEnterNotifier(client->endPoint);
			Server_ClientEndpointNotifier(client, theError, 0, nil);
			OTLeaveNotifier(client->endPoint);
		}
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
	
	//Setup server info
	serverInfo.packetType = kPacketType_ServerInfo;
	BlockMove(hostName, serverInfo.serverName, sizeof(Str31));
	BlockMove(hostLocation, serverInfo.serverLocation, sizeof(Str31));
	BlockMove(hostMessage, serverInfo.serverWelcomeMessage, sizeof(Str255));
	Network_GetComputerConfig(serverInfo.serverOS, serverInfo.serverProcessor);
	serverInfo.serverVersion = kNetGameVersion;
	serverInfo.serverGameSubMode = kGameSubMode_Net_SelectShip;
	serverInfo.terrainID = terrainID;
	serverInfo.terrainDifficulty = difficulty;
	terrainData = GetTerrainData_ByID(&coreData, terrainID);
	BlockMove(terrainData->description.realName, serverInfo.terrainName, sizeof(Str31));
	serverMaxPlayers = terrainData->description.maxNetworkPlayers;
	if(serverMaxPlayers > kMaxPlayers)
	serverMaxPlayers = kMaxPlayers;
	
	//Clean up
	OTDestroyConfiguration(OTConfig);
	
	//Install notifier on master EndPoint
	theError = OTSetAsynchronous(serverEndPoint);
	if(theError)
	return theError;
	theError = OTInstallNotifier(serverEndPoint, Server_EndpointNotifier, nil);
	if(theError)
	return theError;
	
	//Make sure no event is pending
	theError = OTLook(serverEndPoint);
	if(theError) {
		OTEnterNotifier(serverEndPoint);
		Server_EndpointNotifier(nil, theError, 0, nil);
		OTLeaveNotifier(serverEndPoint);
	}
	
	//Setup server status
	serverEventCount = 0;
	for(i = 0; i < kMaxPlayers; ++i)
	serverNextSendTime[i] = 0;
	serverNextSendClockTime = 0;
	BlockMove(hostPassword, serverPassword, sizeof(Str31));
	serverStatus = kServerStatus_Online;
	for(i = 0; i < kMaxPlayers; ++i)
	connectionList[i].status = kClientStatus_Offline;
	Network_ClearShipBuffer();
	
	return noErr;
}

OSErr Server_ConnectLocalPlayer(Str31 playerName, Str31 playerLocation, OSType shipID, OSType setID)
{
	ClientConnectionPtr		client = nil;
	long					i;
	
	if(serverStatus != kServerStatus_Online)
	return kError_FatalError;
	
	//Disable server notifier
	OTEnterNotifier(serverEndPoint);
	
	//Have we reached the max number of players?
	if(Server_CurrentNumberPlayers() >= serverMaxPlayers) {
		OTLeaveNotifier(serverEndPoint);
		return kError_MaxItems;
	}
	
	//Find an empty slot
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_Offline) {
		client = &connectionList[i];
		break;
	}
	if(client == nil) {
		OTLeaveNotifier(serverEndPoint);
		return kError_MaxItems;
	}
	
	//Setup client
	client->status = kClientStatus_LocalPlayer;
	client->address.fHost = Network_GetComputerIP();
	client->info.gameVersion = kNetGameVersion;
	BlockMove(playerName, client->info.playerName, sizeof(Str31));
	BlockMove(playerLocation, client->info.playerLocation, sizeof(Str31));
	client->info.password[0] = 0;
	client->info.shipID = shipID;
	client->info.setID = setID;
	
	Server_PostEvent(kNetEvent_Server_ClientConnection, (long) &client->info);
	
	OTLeaveNotifier(serverEndPoint);
	
	return noErr;
}

OSErr Server_ConnectLocalBot(OSType characterID)
{
	ClientConnectionPtr			client = nil;
	long						i;
	CharacterPreloadedDataPtr	characterData;
	
	if(serverStatus != kServerStatus_Online)
	return kError_FatalError;
	
	//Find bot info
	characterData = GetCharacterData_ByID(&coreData, characterID);
	if(characterData == nil)
	return kError_CharacterNotFound;
	
	//Disable server notifier
	OTEnterNotifier(serverEndPoint);
	
	//Have we reached the max number of players?
	if(Server_CurrentNumberPlayers() >= serverMaxPlayers) {
		OTLeaveNotifier(serverEndPoint);
		return kError_MaxItems;
	}
	
	//Find an empty slot
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_Offline) {
		client = &connectionList[i];
		break;
	}
	if(client == nil) {
		OTLeaveNotifier(serverEndPoint);
		return kError_MaxItems;
	}
	
	//Setup client
	client->status = kClientStatus_LocalBot;
	client->address.fHost = Network_GetComputerIP();
	client->info.gameVersion = kNetGameVersion;
	CopyFirstName(characterData->description.realName, client->info.playerName);
	BlockMove(kString_Network_BotLocation, client->info.playerLocation, sizeof(Str31));
	client->info.password[0] = 0;
	client->info.shipID = characterData->description.shipID;
	client->info.setID = characterData->description.textureSetID;
	
	Server_PostEvent(kNetEvent_Server_ClientConnection, (long) &client->info);
	
	OTLeaveNotifier(serverEndPoint);
	
	return noErr;
}

void Server_ShutDown()
{
	EndpointRef			endPoint;
	long				i;
	
	//Kill master EndPoint
	if(serverEndPoint != kOTInvalidEndpointRef) {
		OTSetSynchronous(serverEndPoint);
		OTRemoveNotifier(serverEndPoint);
		OTUnbind(serverEndPoint);
		OTSetNonBlocking(serverEndPoint);
		OTCloseProvider(serverEndPoint);
	}
	
	//Setup
	serverStatus = kStatus_OffLine;
	serverEndPoint = kOTInvalidEndpointRef;
	
	//Disconnect all active clients and kill their EndPoints
	for(i = 0; i < kMaxPlayers; ++i) {
		if(connectionList[i].endPoint != kOTInvalidEndpointRef) {
			endPoint = connectionList[i].endPoint;
			
			OTSetSynchronous(endPoint);
			OTRemoveNotifier(endPoint);
			Server_DisconnectClient(&connectionList[i], kNetError_Client_ServerShutDown);
			OTUnbind(endPoint);
			OTSetNonBlocking(endPoint);
			OTCloseProvider(endPoint);
		}
		connectionList[i].endPoint = kOTInvalidEndpointRef;
		connectionList[i].status = kClientStatus_Offline;
	}
}

OSErr Server_SetupGame()
{
	long						i,
								j,
								bytes;
	PlayerConfigurationPtr		player;
	GameInfo					info;
	
	if(serverStatus != kServerStatus_Online)
	return kError_FatalError;
	
	//Setup game info
	gameConfig.version = kNetGameVersion;
	gameConfig.gameMode = kGameMode_Host;
	gameConfig.gameSubMode = serverInfo.serverGameSubMode;
	gameConfig.gameMode_save = gameConfig.gameMode;
	gameConfig.gameSubMode_save = gameConfig.gameSubMode;
	gameConfig.terrainID = serverInfo.terrainID;
	gameConfig.terrainDifficulty = serverInfo.terrainDifficulty;
	gameConfig.playerCount = 0;
	
	//Clear players
	for(i = 0; i < kMaxPlayers; ++i)
	Clear_Player(&gameConfig.playerList[i]);
	
	//Setup players
	for(i = 0; i < kMaxPlayers; ++i)
	if((connectionList[i].status == kClientStatus_ConnectedToClient) || (connectionList[i].status == kClientStatus_LocalBot) || (connectionList[i].status == kClientStatus_LocalPlayer)) {
		player = &gameConfig.playerList[gameConfig.playerCount];
		
		player->characterID = kNoID;
		player->shipID = connectionList[i].info.shipID;
		player->shipTextureID = connectionList[i].info.setID;
		if(connectionList[i].status == kClientStatus_LocalPlayer)
		player->shipPilotType = kPilot_Local;
		else if(connectionList[i].status == kClientStatus_LocalBot)
		player->shipPilotType = kPilot_Bot;
		else
		player->shipPilotType = kPilot_Remote;
		BlockMove(connectionList[i].info.playerName, player->playerName, sizeof(Str31));
		player->playerDataPtr = nil;
		
		connectionList[i].player = player;
		
		++gameConfig.playerCount;
	}
	
	//Setup game info
	info.packetType = kPacketType_GameInfo;
	info.playerCount = gameConfig.playerCount;
	for(i = 0; i < gameConfig.playerCount; ++i) {
		info.playerList[i].shipID = gameConfig.playerList[i].shipID;
		info.playerList[i].setID = gameConfig.playerList[i].shipTextureID;
		BlockMove(gameConfig.playerList[i].playerName, info.playerList[i].playerName, sizeof(Str31));
	}
	
	//Send game info to active players
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_ConnectedToClient) {
		for(j = 0; j < info.playerCount; ++j) {
			if(&gameConfig.playerList[j] == connectionList[i].player)
			info.playerList[j].isLocal = true;
			else
			info.playerList[j].isLocal = false;
		}
		
		bytes = OTSnd(connectionList[i].endPoint, &info, sizeof(GameInfo), 0);
		if(bytes == kOTLookErr)
		Server_ClientEndpointNotifier(&connectionList[i], OTLook(connectionList[i].endPoint), 0, nil);
	}
	
	serverStatus = kServerStatus_WaitingForClientReady;
	
	return noErr;
}

Boolean Server_GetNextEvent(OSType* event, long* data)
{
	long				i;
	
	if(!serverEventCount)
	return false;
	
	*event = serverEventTypeBuffer[0];
	*data = serverEventDataBuffer[0];
	for(i = 0; i < serverEventCount - 1; ++i) {
		serverEventTypeBuffer[i] = serverEventTypeBuffer[i + 1];
		serverEventDataBuffer[i] = serverEventDataBuffer[i + 1];
	}
	--serverEventCount;
	
	return true;
}

void Server_GetClientIP(ClientInfoPtr info, Str31 string)
{
	long				i;
	char*				theChar = (char*) &string[1];
	
	string[0] = 0;
		
	for(i = 0; i < kMaxPlayers; ++i)
	if(&connectionList[i].info == info) {
		OTInetHostToString(connectionList[i].address.fHost, theChar);
		while(*theChar++)
		++string[0];
		return;
	}
}

static Boolean Server_AllClientsReady()
{
	long				i;
	
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_ConnectedToClient)
	return false;
	
	return true;
}

OSErr Server_WaitForClients()
{
	Packet				packet;
	OSStatus			theError = noErr;
	long				bytes,
						i;
	
	if(serverStatus != kServerStatus_WaitingForClientReady)
	return kError_FatalError;
	
	//Wait for all clients ready signal
	do {
		if(EscapeKeyDown())
		theError = kError_UserCancel;
		if(serverStatus == kServerStatus_Offline) //TCP/IP was disconnected or one client was disconnected
		theError = kError_UserCancel;
	} while((theError == noErr) && !Server_AllClientsReady());
	if(theError)
	return theError;
	
	//Send clock packet
	packet.packetType = kPacketType_ServerClock;
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_Ready) {
		packet.data = RClock_GetTime();
		bytes = OTSnd(connectionList[i].endPoint, &packet, sizeof(Packet), 0);
		if(bytes == kOTLookErr)
		Server_ClientEndpointNotifier(&connectionList[i], OTLook(connectionList[i].endPoint), 0, nil);
	}
	
	//Send server ready signal
	packet.packetType = kPacketType_ServerReady;
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_Ready) {
		bytes = OTSnd(connectionList[i].endPoint, &packet, sizeof(Packet), 0);
		if(bytes == kOTLookErr)
		Server_ClientEndpointNotifier(&connectionList[i], OTLook(connectionList[i].endPoint), 0, nil);
	}
	
	//Update status
	serverStatus = kServerStatus_RaceRunning;
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_Ready)
	connectionList[i].status = kClientStatus_RaceRunning;
	
	return noErr;
}

void Server_SendData(ExtendedShipPtr ship)
{
	ShipState			state;
	long				bytes,
						i;
	
	if(serverStatus != kServerStatus_RaceRunning)
	return;
	
	if(worldState->frameTime < serverNextSendTime[ship->shipNum])
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
	
	//Send to all active remote clients
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_RaceRunning) {
#if __USE_SEGMENT_SIZE_PACKETS__
		bytes = OTSnd(connectionList[i].endPoint, &state, segmentSize, 0);
#else
		bytes = OTSnd(connectionList[i].endPoint, &state, sizeof(ShipState), 0);
#endif
		if(bytes == kOTLookErr)
		Server_ClientEndpointNotifier(&connectionList[i], OTLook(connectionList[i].endPoint), 0, nil);
	}
	
	serverNextSendTime[ship->shipNum] = worldState->frameTime + kNetworkShipRefreshRate;
}

void Server_SendStartTime()
{
	Packet				packet;
	long				bytes,
						i;
	
	if(serverStatus != kServerStatus_RaceRunning)
	return;
	
	//Setup Packet info
	packet.packetType = kPacketType_ServerStartTime;
	packet.data = gameConfig.startTime;
	
	//Send to all active remote clients
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_RaceRunning) {
#if __USE_SEGMENT_SIZE_PACKETS__
		bytes = OTSnd(connectionList[i].endPoint, &packet, segmentSize, 0);
#else
		bytes = OTSnd(connectionList[i].endPoint, &packet, sizeof(Packet), 0);
#endif
		if(bytes == kOTLookErr)
		Server_ClientEndpointNotifier(&connectionList[i], OTLook(connectionList[i].endPoint), 0, nil);
	}
}

void Server_Enter()
{
	long			i;
	
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_RaceRunning)
	OTEnterNotifier(connectionList[i].endPoint);
}

void Server_Leave()
{
	long			i;
	
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_RaceRunning)
	OTLeaveNotifier(connectionList[i].endPoint);
}

/*TMTask		serverTimeTask;

static void ClockSend_Task(TMTaskPtr task)
{
	Packet				packet;
	long				bytes,
						i;
						
	//Send clock packet
	packet.packetType = kPacketType_ServerClock;
	packet.data = RClock_GetTime();
	
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_RaceRunning) {
#if __USE_SEGMENT_SIZE_PACKETS__
		bytes = OTSnd(connectionList[i].endPoint, &packet, segmentSize, 0);
#else
		bytes = OTSnd(connectionList[i].endPoint, &packet, sizeof(Packet), 0);
#endif
		if(bytes == kOTLookErr)
		Server_ClientEndpointNotifier(&connectionList[i], OTLook(connectionList[i].endPoint), 0, nil);
	}
	
	PrimeTimeTask((QElemPtr) &serverTimeTask, kNetworkTimeRefreshRate * 10);
}

OSErr Server_StartSendingClockTime()
{
	OSErr			theError;
	
	if(serverStatus != kServerStatus_RaceRunning)
	return kError_FatalError;
	
	serverTimeTask.tmAddr = NewTimerProc(ClockSend_Task);
	serverTimeTask.tmWakeUp = 0;
	serverTimeTask.tmReserved = 0;
	
	theError = InstallXTimeTask((QElemPtr) &serverTimeTask);
	if(theError)
	return theError;
	
	return PrimeTimeTask((QElemPtr) &serverTimeTask, kNetworkTimeRefreshRate * 10);
}

OSErr Server_StopSendingClockTime()
{
	return RemoveTimeTask((QElemPtr) &serverTimeTask);
}*/

void Server_SendClockTime()
{
	Packet				packet;
	long				bytes,
						i;
	
	if(serverStatus != kServerStatus_RaceRunning)
	return;
	
	if(worldState->frameTime < serverNextSendClockTime)
	return;
	
	//Send clock packet
	packet.packetType = kPacketType_ServerClock;
	for(i = 0; i < kMaxPlayers; ++i)
	if(connectionList[i].status == kClientStatus_RaceRunning) {
		packet.data = RClock_GetTime();
#if __USE_SEGMENT_SIZE_PACKETS__
		bytes = OTSnd(connectionList[i].endPoint, &packet, segmentSize, 0);
#else
		bytes = OTSnd(connectionList[i].endPoint, &packet, sizeof(Packet), 0);
#endif
		if(bytes == kOTLookErr)
		Server_ClientEndpointNotifier(&connectionList[i], OTLook(connectionList[i].endPoint), 0, nil);
	}
	
	serverNextSendClockTime = worldState->frameTime + kNetworkTimeRefreshRate;
}
