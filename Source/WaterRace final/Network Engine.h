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


#ifndef __WATERRACE_NETWORK_ENGINE__
#define __WATERRACE_NETWORK_ENGINE__

#include				<OpenTransport.h>
#include				<OpenTransportProviders.h>

//CONSTANTES:

#define						kPacketType_None			'None'

//Server specific packets
#define						kPacketType_ServerInfo		'SvIf'
#define						kPacketType_GameInfo		'GmIf'
#define						kPacketType_ServerClock		'SvCl'
#define						kPacketType_ServerReady		'SvRd'
#define						kPacketType_DisconnectionReason	'DcRs'
#define						kPacketType_ServerStartTime	'SvST'

//Client specific packets
#define						kPacketType_PlayerReady		'PlRd'
#define						kPacketType_ClientInfo		'ClIf'

//Game specific packets
#define						kPacketType_ShipState		'ShSt'

//Other constants
#define						kMaxBroadCast				16
#define						kMaxBufferSize				32768 //32Kb
#define						kNetworkShipRefreshRate		5 //100/5 = 20Hz
#define						kNetworkTimeRefreshRate		10 //100/10 = 10Hz

//NET ENGINE ERRORS:
enum {
	kNetError_NoNetworkAvailable = 100,
	kNetError_ConnectionRefused,
	kNetError_Disconnected,
	kNetError_UnknownData,
	kNetError_Server_Unknown = 200,
	kNetError_Client_Unknown = 300,
	kNetError_Client_BadVersion,
	kNetError_Client_WrongPassword,
	kNetError_Client_GameStarted,
	kNetError_Client_ServerShutDown,
	kNetError_TCPConnectionLost
};

//Net server event codes
enum {
	kNetEvent_Server_ClientConnection = 'clcn', //eventData = clientinfoPtr
	kNetEvent_Server_ClientDisconnection = 'cldc', //eventData = clientinfoPtr
	kNetEvent_Server_ServerDisconnection = 'svdc' //eventData = error code
};

//Net client event codes:
enum {
	kNetEvent_Client_ReceivedServerInfo = 'svif', //eventData = serverinfoPtr
	kNetEvent_Client_ReceivedGameInfo = 'gmif', //eventData = gameInfoPtr
	kNetEvent_Client_ServerDisconnection = 'svdc' //eventData = error code
};

//STRUCTURES:

#pragma options align=mac68k

//General packet
typedef struct Packet_Definition
{
	OSType					packetType;
	unsigned long			data;
};
typedef Packet_Definition Packet;
typedef Packet_Definition* PacketPtr;

//Connection setup packets - Client and Server info
typedef struct ClientInfo_Definition
{
	OSType					packetType;
	
	short					gameVersion;
	Str31					playerName,
							playerLocation,
							password;
							
	OSType					shipID,
							setID;
};
typedef ClientInfo_Definition ClientInfo;
typedef ClientInfo_Definition* ClientInfoPtr;

typedef struct ServerInfo_Definition
{
	OSType					packetType;
	
	Str31					serverName,
							serverLocation;
	Str255					serverWelcomeMessage;
	
	Str63					serverProcessor;
	Str31					serverOS;
	unsigned short			serverVersion;
	
	OSType					serverGameSubMode;
	OSType					terrainID;
	unsigned char			terrainDifficulty;
	Str31					terrainName;
};
typedef ServerInfo_Definition ServerInfo;
typedef ServerInfo_Definition* ServerInfoPtr;

//Game info packet - Returned by server when game is completely setup
typedef struct GameInfoPlayer_Definition
{
	OSType					shipID,
							setID;
	Str31					playerName;
	Boolean					isLocal;
};
typedef GameInfoPlayer_Definition GameInfoPlayer;
typedef GameInfoPlayer_Definition* GameInfoPlayerPtr;

typedef struct GameInfo_Definition
{
	OSType					packetType;
							
	short					playerCount;
	GameInfoPlayer			playerList[kMaxPlayers];
};
typedef GameInfo_Definition GameInfo;
typedef GameInfo_Definition* GameInfoPtr;

//Broadcast client info
typedef struct BroadCastClient_Definition
{
	InetAddress				address;
	EndpointRef				endPoint;
	long					status;
	Boolean					dataReceived;
	unsigned long			dataTime;
};
typedef BroadCastClient_Definition BroadCastClient;
typedef BroadCastClient_Definition* BroadCastClientPtr;

//Game packets
typedef struct ShipState_Definition
{
	OSType					packetType;
	
	Vector					velocityG,
							accelerationG,
							rotationspeedL,
							rotationAccelerationL;
	Vector					position;
	Quaternion				orientation;
	
	unsigned long			timeStamp;
	
	short					motorPower; //Conversion needed
	short					mouseH; //Conversion needed
	unsigned char			turboState;
	unsigned char			shipNum;
};
typedef ShipState_Definition ShipState;
typedef ShipState_Definition* ShipStatePtr;

#pragma options align=reset

//VARIABLES:

extern unsigned char			dataBuffer[kMaxBufferSize];
extern ShipState				networkShipBuffers[kMaxPlayers];

//File: Network Engine.cp
OSErr Network_Init();
void Network_Quit();
OTResult Network_GetFourByteOption(EndpointRef ep, OTXTILevel level, OTXTIName name, UInt32 *value);
OTResult Network_SetFourByteOption(EndpointRef ep, OTXTILevel level, OTXTIName name, UInt32 value);
InetHost Network_GetComputerIP();
void Network_GetComputerIPString(Str31 string);
void Network_GetComputerConfig(Str31 OS, Str31 processor);
void Network_ClearShipBuffer();

//File: Network Engine Client.cp
OSErr Client_Connect(Str31 destIP, unsigned short port, Str31 password, Str31 playerName, Str31 playerLocation, OSType shipID, OSType setID);
void Client_Disconnect();
Boolean Client_GetNextEvent(OSType* event, long* data);
OSErr Client_WaitForServer();
void Client_SendData(ExtendedShipPtr ship);
OSErr Client_SetupGame();
void Client_Enter();
void Client_Leave();

//File: Network Engine Server.cp
OSErr Server_Start(unsigned short port, Str31 hostName, Str31 hostLocation, Str31 hostPassword, Str31 hostMessage, OSType terrainID, short difficulty);
OSErr Server_ConnectLocalPlayer(Str31 playerName, Str31 playerLocation, OSType shipID, OSType setID);
OSErr Server_ConnectLocalBot(OSType characterID);
void Server_ShutDown();
OSErr Server_SetupGame();
Boolean Server_GetNextEvent(OSType* event, long* data);
void Server_GetClientIP(ClientInfoPtr info, Str31 string);
OSErr Server_WaitForClients();
void Server_SendData(ExtendedShipPtr ship);
void Server_SendStartTime();
void Server_Enter();
void Server_Leave();
void Server_SendClockTime();

#endif
