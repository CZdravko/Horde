// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
//
// This file is part of the GameEngine developed at the
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************
//


// ****************************************************************************************
//
// GameEngine Core Library of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2011 Christoph Nuscheler
//
// ****************************************************************************************
#include "GameNetworkManager.h"

#include "GameModules.h"
#include "GameWorld.h"
#include "GameEntity.h"

#include <GameEngine/GameLog.h>

#include <zlib/zlib.h>

#include <stdio.h>

#ifndef PLATFORM_WIN
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#define closesocket(a) close(a)
#define INVALID_SOCKET -1
#endif

using namespace std;

enum PacketType {
	INVALID_PACKET,
	CONNECT,
	DISCONNECT,
	ACCEPT,
	REJECT,
	STATE_UPDATE,
	DISCOVER,
	RES_DISCOVER,
	REQUEST_UPDATES,
	ACK_REQUEST_UPDATES
};

struct ClientRecord {
	SOCKADDR_IN		adress;
	SOCKET			socket;
	uint32_t			client_id;
	uint32_t			client_tick;

	//list<pair<string,string>*>	allowedUpdates;
	set< pair<string,string> >	requestedUpdates;

	uint32_t	requestedUpdates_changed;
	bool	requestedUpdates_acknowledged;

	// create UDP client
	ClientRecord(SOCKADDR_IN cladr, uint32_t clid) {
		socket = 0;
		adress = cladr;
		client_id = clid;
		client_tick = 0;
		requestedUpdates_changed = 0;
		requestedUpdates_acknowledged = true;
	}

	// create TCP client
	ClientRecord(SOCKET clsocket, uint32_t clid) {
		socket = clsocket;
		client_id = clid;
		client_tick = 0;
		requestedUpdates_changed = 0;
		requestedUpdates_acknowledged = true;
	}

	~ClientRecord() {
		requestedUpdates.clear();
		if (socket) {
			closesocket(socket);
			socket = 0;
		}
	}

	void requestUpdate(string entity, string component) {
		pair<string,string> p(entity, component);
		requestedUpdates.insert(p);
		requestedUpdates_acknowledged = false;
	}

	void disrequestUpdate(string entity, string component) {
		pair<string,string> p(entity, component);
		requestedUpdates.erase(p);
		requestedUpdates_acknowledged = false;
	}

	bool isUpdateRequested(string entity, string component) {
		pair<string,string> p(entity, component);
		return (requestedUpdates.find(p) != requestedUpdates.end());
	}
};

struct NetworkMessage {
	static const uint32_t MAXDATALENGTH = 65521;         // maximum usable amount of data = UDP max packet size minus 15 reserved bytes

	char message[65536];							// data field size = UDP max packet size
	Bytef compressed_data[72088];					// data field size = usable data size * 1.1 + 12 bytes;  additional bytes needed by zlib (de-)compression routine
	uLongf compressed_data_size;

	enum {
		OFFSET_DL = 0,
		OFFSET_IDENT = 2,
		OFFSET_CLIENTID = 6,
		OFFSET_TYPE = 10,
		OFFSET_TICK = 11,
		OFFSET_DATA = 15
	};

	// magic bytes
	inline bool isGameEngineMessage() {
		return (message[OFFSET_IDENT] == 'G')
			&& (message[OFFSET_IDENT + 1] == 'a')
			&& (message[OFFSET_IDENT + 2] == 'm')
			&& (message[OFFSET_IDENT + 3] == 'E');
	}

	inline bool isCompressed() {
		if (message[OFFSET_TYPE] & (char)0x80)
			return true;
		else
			return false;
	}

	PacketType getType() {
		return (PacketType) (message[OFFSET_TYPE] & (char)0x7F);
	}

	uint32_t getDataLength() {
		return
			((message[OFFSET_DL] & 0xFF) << 8) +
			(message[OFFSET_DL + 1] & 0xFF);
	}

	uint32_t getTotalLength() {
		return getDataLength() + OFFSET_DATA;
	}

	uint32_t getTick() {
		return
			((message[OFFSET_TICK] & 0xFF) << 24) +
			((message[OFFSET_TICK + 1] & 0xFF) << 16) +
			((message[OFFSET_TICK + 2] & 0xFF) << 8) +
			(message[OFFSET_TICK + 3] & 0xFF);
	}

	uint32_t getClientID() {
		return
			((message[OFFSET_CLIENTID] & 0xFF) << 24) +
			((message[OFFSET_CLIENTID + 1] & 0xFF) << 16) +
			((message[OFFSET_CLIENTID + 2] & 0xFF) << 8) +
			(message[OFFSET_CLIENTID + 3] & 0xFF);
	}

	void makeGameEngineMessage() {
		message[OFFSET_IDENT] = 'G';
		message[OFFSET_IDENT + 1] = 'a';
		message[OFFSET_IDENT + 2] = 'm';
		message[OFFSET_IDENT + 3] = 'E';
	}

	void setType(PacketType type) {
		message[OFFSET_TYPE] = type | (message[OFFSET_TYPE] & (char)0x80);
	}

	void setDataLength(unsigned short bytes) {
		message[OFFSET_DL] = bytes >> 8;
		message[OFFSET_DL + 1] = (char)bytes;
	}

	void setTick(uint32_t tick) {
		message[OFFSET_TICK] = tick >> 24;
		message[OFFSET_TICK + 1] = tick >> 16;
		message[OFFSET_TICK + 2] = tick >> 8;
		message[OFFSET_TICK + 3] = tick;
	}

	void setClientID(uint32_t clid) {
		message[OFFSET_CLIENTID] = clid >> 24;
		message[OFFSET_CLIENTID + 1] = clid >> 16;
		message[OFFSET_CLIENTID + 2] = clid >> 8;
		message[OFFSET_CLIENTID + 3] = clid;
	}

	inline char* data(int offset) {
		return &(message[OFFSET_DATA]) + offset;
	}

	void compressData() {
		if (getDataLength() == 0)
			return;

		compressed_data_size = 72088;
		int z_result = compress(compressed_data, &compressed_data_size, (const Bytef*)data(0), getDataLength());

		if (z_result != Z_OK) {
			printf(" NetworkManager: zlib compression failed!\n");
			return;
		} //else
			//printf(" NetworkManager: compressed %i data bytes to %i bytes.\n", getDataLength(), compressed_data_size);

		memcpy(data(0), compressed_data, compressed_data_size);
		setDataLength((unsigned short)compressed_data_size);

		message[OFFSET_TYPE] |= (char)0x80;		// set compressed flag
	}

	void decompressData() {
		if (getDataLength() == 0)
			return;

		memcpy(compressed_data, data(0), getDataLength());
		uLongf dataLength = MAXDATALENGTH;
		int z_result = uncompress((Bytef*)data(0), &dataLength, compressed_data, getDataLength());

		if (z_result != Z_OK) {
			printf(" NetworkManager: zlib decompression failed! Error Code = %i\n", z_result);
			return;
		}

		setDataLength((unsigned short)dataLength);

		message[OFFSET_TYPE] &= (char)0x7F;		// unset compressed flag
	}
};

GameNetworkManager::GameNetworkManager()
{
	m_currentState = GameEngine::Network::INVALID_STATE;

	m_outgoing_message = new NetworkMessage();
	m_incoming_message = new NetworkMessage();

	m_outgoing_message->makeGameEngineMessage();

	m_gamestate = new GameState();

	m_applicationID = new char[11];
	strcpy_s(m_applicationID, 11, "GameEngine");

	m_onClientConnect = 0;
	m_onClientDisconnect = 0;
	m_onStateRequest = 0;
}

GameNetworkManager::~GameNetworkManager()
{
	release();
}

bool GameNetworkManager::init()
{
	GameLog::logMessage("Initializing GameNetworkManager. Using zlib version %s", ZLIB_VERSION);

#if defined PLATFORM_WIN
	// init Winsock
	WSADATA wsa;
	if ( WSAStartup(MAKEWORD(2,0),&wsa) != 0) {
		GameLog::errorMessage("GameNetworkManager: Could not initialize Winsock.");
		return false;
	}
#endif

	m_currentState = GameEngine::Network::DISCONNECTED;

	m_sv_adress.sin_family = AF_INET;
#if defined PLATFORM_WIN
	m_sv_adress.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	m_sv_adress.sin_addr.s_addr = INADDR_ANY;
#endif
	m_sv_adress.sin_port = htons(22888);

	m_sv_tickinterval = 5;
	m_sv_clientTimeoutInterval = 600;
	m_sv_clientid = 1;

	m_cl_serveradress.sin_family = AF_INET;
	m_cl_serveradress.sin_port = htons(22888);

	m_cl_tickinterval = 5;

	m_useCompression = true;

	m_useTCP = false;

	return true;
}

void GameNetworkManager::release()
{
	disconnect();
	m_currentState = GameEngine::Network::INVALID_STATE;

	// close UDP socket
	if ( m_socket != 0 )
		closesocket(m_socket);

	// release Winsock
#if defined PLATFORM_WIN
	WSACleanup();
#endif

	// clean messages
	delete m_outgoing_message;
	delete m_incoming_message;

	// clean GameState object
	delete m_gamestate;

	delete[] m_applicationID;
}


void GameNetworkManager::setupServer() {
	if (m_currentState != GameEngine::Network::DISCONNECTED) {
		GameLog::warnMessage(" NetworkManager: Failed to start Server! Already connected or serving.");
		return;
	}

	m_sv_tick = 0;

	// empty client IDs
	m_sv_clients.clear();

	m_sv_orphanSockets.clear();

	if (m_useTCP) {

		// set up TCP listen socket
		if ( ( m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ) == INVALID_SOCKET ) {
			GameLog::errorMessage("GameNetworkManager: Could not create TCP socket.");
			return;
		}

		// enable non-blocking
#if defined PLATFORM_WIN
		unsigned long mode = 1;
		ioctlsocket(m_socket, FIONBIO, &mode);
#else
		int mode;
		mode = fcntl(m_socket, F_GETFL, 0);
		fcntl(m_socket, F_SETFL, mode | O_NONBLOCK);
#endif

		// disable Nagle's algorithm (sacrifice a little throughput for drastically reduced latency)
		int flag = 1;
		setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );

		// bind socket
		bind(m_socket, (SOCKADDR*) &m_sv_adress, sizeof(SOCKADDR));

		// listen
		listen(m_socket, SOMAXCONN);

	} else {

		// set up UDP socket
		if ( ( m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) == INVALID_SOCKET ) {
			GameLog::errorMessage("GameNetworkManager: Could not create UDP socket.");
			return;
		}

#if defined PLATFORM_WIN
		unsigned long mode = 1;
		ioctlsocket(m_socket, FIONBIO, &mode);
#else
		int mode;
		mode = fcntl(m_socket, F_GETFL, 0);
		fcntl(m_socket, F_SETFL, mode | O_NONBLOCK);
#endif

		// bind socket
		bind(m_socket, (SOCKADDR*) &m_sv_adress, sizeof(SOCKADDR));
	}

	m_currentState = GameEngine::Network::SERVING;
}

void GameNetworkManager::connectToServer(const char* ip_addr) {
	if (m_currentState != GameEngine::Network::DISCONNECTED) {
		GameLog::warnMessage(" NetworkManager: Failed to connect to server! Already connected or serving.");
		return;
	}

	if (m_useTCP) {
		// set up TCP socket
		if ( ( m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ) == INVALID_SOCKET ) {
			GameLog::errorMessage("GameNetworkManager: Could not create TCP socket.");
			return;
		}

		// enable non-blocking
#if defined PLATFORM_WIN
		unsigned long mode = 1;
		ioctlsocket(m_socket, FIONBIO, &mode);
#else
		int mode;
		mode = fcntl(m_socket, F_GETFL, 0);
		fcntl(m_socket, F_SETFL, mode | O_NONBLOCK);
#endif

		// disable Nagle's algorithm (sacrifice a little throughput for drastically reduced latency)
		int flag = 1;
		setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );

	} else {
		// set up TCP socket
		if ( ( m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) == INVALID_SOCKET ) {
			GameLog::errorMessage("GameNetworkManager: Could not create TCP socket.");
			return;
		}

		// enable non-blocking
#if defined PLATFORM_WIN
		unsigned long mode = 1;
		ioctlsocket(m_socket, FIONBIO, &mode);
#else
		int mode;
		mode = fcntl(m_socket, F_GETFL, 0);
		fcntl(m_socket, F_SETFL, mode | O_NONBLOCK);
#endif
	}

#if defined PLATFORM_WIN
	m_cl_serveradress.sin_addr.S_un.S_addr = inet_addr(ip_addr);
#else
	inet_pton(AF_INET, ip_addr, &m_cl_serveradress.sin_addr);
#endif

	m_cl_tick = 0;
	m_cl_retrytimer = 10;

	// connect (if TCP)
	// fix server adress (if UDP)
	connect(m_socket, (SOCKADDR*) &m_cl_serveradress, sizeof(SOCKADDR_IN));

	m_currentState = GameEngine::Network::CONNECTING_TO_SERVER;
}

void GameNetworkManager::disconnect() {
	if (m_currentState == GameEngine::Network::CONNECTED_TO_SERVER) {
		cl_disconnect();
	}

	if (m_currentState == GameEngine::Network::SERVING) {
		// send goodbye messages to clients
		m_outgoing_message->setDataLength(0);
		m_outgoing_message->setTick(m_sv_tick);
		m_outgoing_message->setType(DISCONNECT);

		sv_broadcastOutgoingMessage();

		m_sv_clients.clear();
	}

	m_currentState = GameEngine::Network::DISCONNECTED;
}

bool GameNetworkManager::removeClient(const unsigned long clientID) {
	return sv_removeClient((uint32_t)clientID);
}

void GameNetworkManager::update() {

	switch (m_currentState) {

		case GameEngine::Network::SERVING:
			m_sv_tick++;
			if (m_useTCP)
				sv_acceptIncomingTCPConnections();
			sv_receiveMessages();
			sv_testClientsTimeout();
			sv_sendStateRequests();
			sv_transmitComponentStates();
			break;

		case GameEngine::Network::CONNECTING_TO_SERVER:
			if (m_cl_retrytimer == 10) {
				m_cl_retrytimer = 0;
				// send/resend CONNECT message
				cl_connectToServer();
			} else
				m_cl_retrytimer++;

			cl_awaitAccept();
			break;

		case GameEngine::Network::CONNECTED_TO_SERVER:
			cl_transmitComponentStates();
			cl_handleServerMessages();
			break;

		default:
			break;
	}

}

void GameNetworkManager::cl_connectToServer() {
	m_outgoing_message->setType(CONNECT);
	// application identifier
	strcpy_s(m_outgoing_message->data(0), m_outgoing_message->MAXDATALENGTH, m_applicationID);
	m_outgoing_message->setDataLength(strlen(m_applicationID) + 1);
	m_outgoing_message->setTick(m_cl_tick);

	send(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
}

void GameNetworkManager::cl_disconnect() {
	m_outgoing_message->setType(DISCONNECT);
	m_outgoing_message->setDataLength(0);
	m_outgoing_message->setTick(m_cl_tick);

	send(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
}

void GameNetworkManager::cl_awaitAccept() {
	//SOCKADDR_IN from_addr;
	//int from_len = sizeof(SOCKADDR_IN);

	while ( ((m_useTCP) && readNextTCPMessage(m_incoming_message, m_socket))
        ||  ((!m_useTCP) && (recv(m_socket, m_incoming_message->message, 65536, 0) > 0))) {

		if (!m_incoming_message->isGameEngineMessage())
			continue;

		if (m_incoming_message->getType() == ACCEPT) {
			// connection has been accepted by server
			m_cl_id = m_incoming_message->getClientID();
			m_currentState = GameEngine::Network::CONNECTED_TO_SERVER;
			// set local tick to server's tick
			m_cl_tick = m_incoming_message->getTick();
			break;

		} else if (m_incoming_message->getType() == REJECT) {
			// connection has been refused by server
			m_currentState = GameEngine::Network::DISCONNECTED;
			if (m_socket != INVALID_SOCKET)
				closesocket(m_socket);
			break;
		}
	}
}

void GameNetworkManager::cl_handleServerMessages() {
	//SOCKADDR_IN from_addr;
	//int from_len = sizeof(SOCKADDR_IN);

	while ( ((m_useTCP) && readNextTCPMessage(m_incoming_message, m_socket))
        ||  ((!m_useTCP) && (recv(m_socket, m_incoming_message->message, 65536, 0) > 0))) {

		if (!m_incoming_message->isGameEngineMessage())
			continue;

		if (m_incoming_message->getClientID() != m_cl_id)
			continue;

		switch (m_incoming_message->getType()) {

			case REQUEST_UPDATES:
				{
					//printf("REQUEST_UPDATES received, length = %i", m_incoming_message->getDataLength());
					/*if (m_incoming_message->isCompressed()) {
						m_incoming_message->decompressData();		// decompress message

						if (m_incoming_message->isCompressed())		// decompression failed
							continue;
					}*/

					m_cl_components.clear();	// TODO: hopefully, this won't call the GameComponents' destructors?

					uint32_t datacursor = 0;

					while (datacursor < m_incoming_message->getDataLength()) {
						std::string entityID(m_incoming_message->data(datacursor));						datacursor += entityID.length() + 1;
						if (datacursor > m_incoming_message->getDataLength())
							break;
						std::string componentID(m_incoming_message->data(datacursor));					datacursor += componentID.length() + 1;

						GameEntity* ge = GameModules::gameWorld()->entity(entityID);

						if (!ge)
							continue;

						GameComponent* gc = ge->component(componentID);

						if (!gc)
							continue;

						m_cl_components.insert(gc);

						// execute callback
						if (m_onStateRequest)
							m_onStateRequest(entityID, componentID);
					}

					// send acknowledgement
					m_outgoing_message->setClientID(m_cl_id);
					m_outgoing_message->setTick(m_cl_tick);
					m_outgoing_message->setType(ACK_REQUEST_UPDATES);
					uint32_t request_changed = m_incoming_message->getTick();
					memcpy(m_outgoing_message->data(0), &request_changed, sizeof(uint32_t));
					send(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
				}
			break;

			case STATE_UPDATE:

				// discard if too old
				if (m_incoming_message->getTick() < m_cl_tick)
					break;
				else
				{
					m_cl_tick = m_incoming_message->getTick();

					if (m_incoming_message->isCompressed()) {
						m_incoming_message->decompressData();		// decompress message

						if (m_incoming_message->isCompressed())		// decompression failed
							continue;
					}

					uint32_t datacursor = 0;

					while (datacursor < m_incoming_message->getDataLength()) {
						uint32_t statelength = 0;
						memcpy(&statelength, m_incoming_message->data(datacursor), sizeof(uint32_t));		datacursor += sizeof(uint32_t);

						std::string entityID(m_incoming_message->data(datacursor));						datacursor += entityID.length() + 1;
						std::string componentID(m_incoming_message->data(datacursor));					datacursor += componentID.length() + 1;

						GameEntity* ge = GameModules::gameWorld()->entity(entityID);

						// discard if this is a component registered on this client
						if (m_cl_components.find(ge->component(componentID)) != m_cl_components.end()) {
							datacursor += statelength;
							continue;
						}

						// set component's state
						m_gamestate->clearAndCopyBufferFrom(m_incoming_message->data(datacursor), statelength); 	datacursor += statelength;
						ge->component(componentID)->setSerializedState(*m_gamestate);
					}
				}
				break;

			case DISCONNECT:
				// connection terminated by server
				m_currentState = GameEngine::Network::DISCONNECTED;
				break;

			default:
				break;
		}
	}
}

void GameNetworkManager::cl_transmitComponentStates() {
	static uint32_t count = 0;

	// do nothing for m_cl_tickinterval frames
	if (++count != m_cl_tickinterval) {
		return;
	}

	count = 0;

	// compose a packet
	// this client's ID
	m_outgoing_message->setClientID(m_cl_id);

	// type
	m_outgoing_message->setType(STATE_UPDATE);

	// client tick
	m_outgoing_message->setTick(m_cl_tick);

	m_outgoing_message->setDataLength(0);

	uint32_t datacursor = 0;

	std::set<GameComponent*>::iterator comp_it = m_cl_components.begin();

	while (comp_it != m_cl_components.end()) {
		m_gamestate->clear();
		(*comp_it)->getSerializedState(*m_gamestate);
		uint32_t state_length = m_gamestate->getLength();
		uint32_t componentID_length = (*comp_it)->componentID().length();
		uint32_t entityID_length = (*comp_it)->owner()->id().length();

		// does component provide game state?
		if (state_length > 0) {

			// check for NetworkMessage overflow
			if (datacursor + state_length + componentID_length + entityID_length + sizeof(uint32_t) > NetworkMessage::MAXDATALENGTH) {

				// transmit data and start a new package
				if (m_useCompression)
					m_outgoing_message->compressData();

				send(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);

				m_outgoing_message->setDataLength(0);
			}

			// update packet's data length
			m_outgoing_message->setDataLength(m_outgoing_message->getDataLength() + state_length + entityID_length + componentID_length + 2 + sizeof(uint32_t));

			// size of following state data (doesn't include length of NULL-terminated entity/component IDs
			memcpy(m_outgoing_message->data(datacursor), &state_length, sizeof(uint32_t));	datacursor += sizeof(uint32_t);

			// Entity ID
			strcpy(m_outgoing_message->data(datacursor), (*comp_it)->owner()->id().c_str());	datacursor += entityID_length + 1;

			// Component ID
			strcpy(m_outgoing_message->data(datacursor), (*comp_it)->componentID().c_str());	datacursor += componentID_length + 1;

			// Component's state
			memcpy(
				m_outgoing_message->data(datacursor), m_gamestate->getBuffer(), m_gamestate->getLength());		datacursor += state_length;

		}

		comp_it++;
	}

	// send message to server
	if (m_useCompression)
		m_outgoing_message->compressData();

	send(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
}


void GameNetworkManager::sv_acceptIncomingTCPConnections() {
	SOCKET s;
	s = accept(m_socket, NULL, NULL);	// client address is not relevant to us when dealing with TCP

	while (s != INVALID_SOCKET) {

		// enable non-blocking
#if defined PLATFORM_WIN
		unsigned long mode = 1;
		ioctlsocket(s, FIONBIO, &mode);
#else
		int mode;
		mode = fcntl(s, F_GETFL, 0);
		fcntl(s, F_SETFL, mode | O_NONBLOCK);
#endif

		// disable Nagle's algorithm (sacrifice a little throughput for drastically reduced latency)
		int flag = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );

		m_sv_orphanSockets.insert(s);

		// next new incoming connection
		s = accept(m_socket, NULL, NULL);
	}
}

uint32_t GameNetworkManager::sv_addClient(SOCKADDR_IN client) {
	// adds a UDP client
	uint32_t id = m_sv_clientid++;

	ClientRecord* cr = new ClientRecord(client, id);
	cr->client_tick = m_sv_tick;

	m_sv_clients.insert(std::pair<uint32_t,ClientRecord*>(id,cr));

	return id;
}

uint32_t GameNetworkManager::sv_addClient(SOCKET clientsocket) {
	// adds a TCP client
	uint32_t id = m_sv_clientid++;

	ClientRecord* cr = new ClientRecord(clientsocket, id);
	cr->client_tick = m_sv_tick;

	m_sv_clients.insert(std::pair<uint32_t,ClientRecord*>(id,cr));

	m_sv_orphanSockets.erase(clientsocket);

	return id;
}

bool GameNetworkManager::sv_removeClient(uint32_t clientID) {
	std::map<uint32_t,ClientRecord*>::iterator it = m_sv_clients.find(clientID);

	if (it == m_sv_clients.end())
		return false;

	// send goodbye message
	m_outgoing_message->setClientID(clientID);
	m_outgoing_message->setType(DISCONNECT);
	m_outgoing_message->setDataLength(0);
	m_outgoing_message->setTick(m_sv_tick);

	sv_sendOutgoingMessageTo(clientID);

	// remove client
	delete it->second;
	m_sv_clients.erase(it);

	if (m_onClientDisconnect)
		m_onClientDisconnect(clientID);

	return true;
}

void GameNetworkManager::sv_receiveMessages() {
	SOCKADDR_IN from_addr;
	int from_len = sizeof(SOCKADDR_IN);

	if (!m_useTCP) {

		// UDP messages
#if defined PLATFORM_WIN
		while ((!m_useTCP) && (recvfrom(m_socket, m_incoming_message->message, 65536, 0, (SOCKADDR*) &from_addr, &from_len) > 0))   {
#else
		while ((!m_useTCP) && (recvfrom(m_socket, m_incoming_message->message, 65536, 0, (SOCKADDR*) &from_addr, (socklen_t*)&from_len) > 0))   {
#endif
			sv_handleIncomingMessage(&from_addr, 0);
		}

	} else {
		// TCP messages from clients
		std::map<uint32_t,ClientRecord*>::iterator cl_it = m_sv_clients.begin();
		while(cl_it != m_sv_clients.end()) {
			ClientRecord* client = cl_it->second;

			cl_it++;

			// three steps: Does client still exist? Does it own a socket (TCP)? Was data received?
			while ((client) && (client->socket) && readNextTCPMessage(m_incoming_message, client->socket)) {
				sv_handleIncomingMessage(0, (client->socket));
			}
		}

		// TCP messages from other TCP connections (wannabe clients)
		std::set<SOCKET>::iterator sock_it = m_sv_orphanSockets.begin();
		while (sock_it != m_sv_orphanSockets.end()) {
			SOCKET s = *sock_it;

			if (s == INVALID_SOCKET) {
				std::set<SOCKET>::iterator sock_it_temp = sock_it;
				sock_it++;
				m_sv_orphanSockets.erase(sock_it_temp);
				continue;
			}

			sock_it++;

			while (readNextTCPMessage(m_incoming_message, s)) {
				sv_handleIncomingMessage(0, s);
			}
		}
	}
}

void GameNetworkManager::sv_handleIncomingMessage(SOCKADDR_IN* fromaddr, SOCKET fromsocket) {
	// fromsocket == 0  =>  UDP client

	// is it a GameEngine message?
	if (!m_incoming_message->isGameEngineMessage())
		return;

	// determine message type
	switch (m_incoming_message->getType()) {
		uint32_t id;

		// someone is looking for servers
		case DISCOVER:
			m_outgoing_message->setType(RES_DISCOVER);
			m_outgoing_message->setClientID(0);
			m_outgoing_message->setDataLength(0);
			m_outgoing_message->setTick(m_sv_tick);
			if (!fromsocket)
				sendto(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0, (SOCKADDR*)fromaddr, sizeof(SOCKADDR_IN));
			else
				send(fromsocket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
			break;

		// new client connects
		case CONNECT:
			// check for application ID match
			if (strcmp(m_applicationID, m_incoming_message->data(0))) {
				m_outgoing_message->setType(REJECT);
				m_outgoing_message->setClientID(0);
				m_outgoing_message->setDataLength(0);
				m_outgoing_message->setTick(m_sv_tick);
				if (!fromsocket)
					sendto(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0, (SOCKADDR*)fromaddr, sizeof(SOCKADDR_IN));
				else {
					send(fromsocket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
					m_sv_orphanSockets.erase(fromsocket);
					closesocket(fromsocket);
				}
			} else {
				// application ID matches, welcome new client
				if (!fromsocket)
					id = sv_addClient(*fromaddr);
				else
					id = sv_addClient(fromsocket);
				m_outgoing_message->setType(ACCEPT);
				m_outgoing_message->setClientID(id);
				m_outgoing_message->setDataLength(0);
				m_outgoing_message->setTick(m_sv_tick);
				sv_sendOutgoingMessageTo(id);
				if (m_onClientConnect)
					m_onClientConnect(id);
			}
			break;

		// client disconnects
		case DISCONNECT:
			sv_removeClient(m_incoming_message->getClientID());
			break;

		// client acknowledges changed requests
		case ACK_REQUEST_UPDATES:
			{
				uint32_t clientID = m_incoming_message->getClientID();

				ClientRecord* client = NULL;

				std::map<uint32_t,ClientRecord*>::iterator it = m_sv_clients.find(clientID);

				if (it == m_sv_clients.end())		// client ID not found on server
					break;

				client = it->second;

				// check for tick match
				uint32_t acknowledge_tick;
				memcpy(&acknowledge_tick, m_incoming_message->data(0), sizeof(uint32_t));
				if (client->requestedUpdates_changed == acknowledge_tick) {
					client->requestedUpdates_acknowledged = true;
				}
			}
			break;

		// client transmits the state of its entities
		case STATE_UPDATE:
			{
				uint32_t clientID = m_incoming_message->getClientID();

				ClientRecord* client = NULL;

				std::map<uint32_t,ClientRecord*>::iterator it = m_sv_clients.find(clientID);

				if (it == m_sv_clients.end())		// client ID not found on server
					break;

				client = it->second;

				// discard if too old
				if (client->client_tick > m_incoming_message->getTick())
					break;

				client->client_tick = m_incoming_message->getTick();

				if (m_incoming_message->isCompressed()) {
					m_incoming_message->decompressData();		// decompress message

					if (m_incoming_message->isCompressed())		// decompression failed
						return;
				}

				uint32_t datacursor = 0;

				while (datacursor < m_incoming_message->getDataLength()) {
					uint32_t statelength = 0;
					memcpy(&statelength, m_incoming_message->data(datacursor), sizeof(uint32_t));		datacursor += sizeof(uint32_t);

					std::string entityID(m_incoming_message->data(datacursor));						datacursor += entityID.length() + 1;
					std::string componentID(m_incoming_message->data(datacursor));					datacursor += componentID.length() + 1;

					// is this state update allowed for this client?
					if ( !client->isUpdateRequested(entityID, componentID) ) {
						datacursor += statelength;
						continue;
					}

					GameEntity* ge = GameModules::gameWorld()->entity(entityID);

					// set component's state
					m_gamestate->clearAndCopyBufferFrom(m_incoming_message->data(datacursor), statelength); 	datacursor += statelength;
					ge->component(componentID)->setSerializedState(*m_gamestate);
				}
			}
			break;

		//case ALIVE:	obsolete, empty STATE_UDPATE acts as an ALIVE message
		//	break;

		default:
			break;
	}
}

void GameNetworkManager::sv_testClientsTimeout() {
	static uint32_t count = 0;

	// do not test for m_sv_tickinterval frames
	if (++count != m_sv_tickinterval) {
		return;
	}

	count = 0;

	std::map<uint32_t,ClientRecord*>::iterator it = m_sv_clients.begin();

	while (it != m_sv_clients.end()) {
		if (m_sv_tick - it->second->client_tick > m_sv_clientTimeoutInterval) {
			printf(" NetworkManager: Client at adress %s timed out and will be removed.\n", inet_ntoa(it->second->adress.sin_addr));
			delete it->second;
			std::map<uint32_t,ClientRecord*>::iterator itt = it;
			it++;
			m_sv_clients.erase(itt);
		} else {
			it++;
		}
	}
}

void GameNetworkManager::sv_sendStateRequests() {
	static uint32_t count = 0;

	// do not send for m_sv_tickinterval frames
	if (++count != m_sv_tickinterval) {
		return;
	}

	count = 0;

	std::map<uint32_t,ClientRecord*>::iterator it = m_sv_clients.begin();

	while (it != m_sv_clients.end()) {
		ClientRecord* client = it->second;
		if (!client->requestedUpdates_acknowledged) {
			m_outgoing_message->setClientID(client->client_id);
			m_outgoing_message->setTick(client->requestedUpdates_changed);
			m_outgoing_message->setType(REQUEST_UPDATES);

			uint32_t offset = 0;
			set< pair<string,string> >::iterator reqit = client->requestedUpdates.begin();
			while (reqit != client->requestedUpdates.end()) {
				pair<string,string> p = (*reqit);

				strcpy_s(m_outgoing_message->data(offset), NetworkMessage::MAXDATALENGTH - offset, p.first.c_str());
				offset+=strlen(p.first.c_str()) + 1;

				strcpy_s(m_outgoing_message->data(offset), NetworkMessage::MAXDATALENGTH - offset, p.second.c_str());
				offset+=strlen(p.second.c_str()) + 1;

				reqit++;
			}

			m_outgoing_message->setDataLength(offset);

			sv_sendOutgoingMessageTo(client);
		}
		it++;
	}
}

void GameNetworkManager::sv_transmitComponentStates() {
	static uint32_t count = 0;

	// do nothing for m_sv_tickinterval frames
	if (++count != m_sv_tickinterval) {
		return;
	}

	count = 0;

	// compose a packet

	// type
	m_outgoing_message->setType(STATE_UPDATE);

	// server tick
	m_outgoing_message->setTick(m_sv_tick);

	m_outgoing_message->setDataLength(0);

	uint32_t datacursor = 0;

	std::set<GameComponent*>::iterator comp_it = m_sv_components.begin();

	while (comp_it != m_sv_components.end()) {

		m_gamestate->clear();
		(*comp_it)->getSerializedState(*m_gamestate);
		uint32_t state_length = m_gamestate->getLength();
		uint32_t componentID_length = (*comp_it)->componentID().length();
		uint32_t entityID_length = (*comp_it)->owner()->id().length();

		// does component provide game state?
		if (state_length > 0) {

			// check for NetworkMessage overflow
			if (datacursor + state_length > NetworkMessage::MAXDATALENGTH) {

				// transmit data and start a new package
				sv_broadcastOutgoingMessage();

				m_outgoing_message->setDataLength(0);
			}

			// update packet's data length
			m_outgoing_message->setDataLength(m_outgoing_message->getDataLength() + state_length + entityID_length + componentID_length + 2 + sizeof(uint32_t));

			// size of following state data (doesn't include length of NULL-terminated entity/component IDs
			memcpy(m_outgoing_message->data(datacursor), &state_length, sizeof(uint32_t));	datacursor += sizeof(uint32_t);

			// Entity ID
			strcpy(m_outgoing_message->data(datacursor), (*comp_it)->owner()->id().c_str());	datacursor += entityID_length + 1;

			// Component ID
			strcpy(m_outgoing_message->data(datacursor), (*comp_it)->componentID().c_str());	datacursor += componentID_length + 1;

			// Component's state
			memcpy(m_outgoing_message->data(datacursor), m_gamestate->getBuffer(), m_gamestate->getLength());		datacursor += state_length;

		}

		comp_it++;
	}

	// send message to clients
	sv_broadcastOutgoingMessage();
}

void GameNetworkManager::sv_sendOutgoingMessageTo(uint32_t clientID) {
	ClientRecord* client = m_sv_clients.find(clientID)->second;
	sv_sendOutgoingMessageTo(client);
}

void GameNetworkManager::sv_sendOutgoingMessageTo(ClientRecord* client) {
	if (client->socket)	// TCP mode
		send(client->socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
	else				// UDP mode
		sendto(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0, (SOCKADDR*)&(client->adress), sizeof(SOCKADDR_IN));
}

void GameNetworkManager::sv_broadcastOutgoingMessage() {
	if (m_useCompression)
		m_outgoing_message->compressData();

	// send the message to all clients
	std::map<uint32_t,ClientRecord*>::iterator cl_it = m_sv_clients.begin();

	while (cl_it != m_sv_clients.end()) {
		ClientRecord* client = cl_it->second;
		m_outgoing_message->setClientID(client->client_id);

		if (client->socket)	// TCP mode
			send(client->socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0);
		else				// UDP mode
			sendto(m_socket, m_outgoing_message->message, m_outgoing_message->getTotalLength(), 0, (SOCKADDR*)&(client->adress), sizeof(SOCKADDR_IN));

		cl_it++;
	}
}

bool GameNetworkManager::readNextTCPMessage(NetworkMessage* network_message, SOCKET tcp_socket) {
    // take a peek in the TCP buffer, at least the 2 byte DL field has to be read.
    if (recv(tcp_socket, network_message->message, NetworkMessage::OFFSET_DL + 2, MSG_PEEK) < NetworkMessage::OFFSET_DL + 2)
        return false;

    int message_length = network_message->getTotalLength();

    // now that we know the length of the message, try to read the whole message
    if (recv(tcp_socket, network_message->message, message_length, MSG_PEEK) < message_length)
        return false;

    // successfully read the whole message, remove it from the TCP stream
    recv(tcp_socket, network_message->message, message_length, 0);

    return true;
}

GameEngine::Network::NetworkState GameNetworkManager::getState() {
	return m_currentState;
}

bool GameNetworkManager::registerServerComponent(GameComponent* component) {

	// no duplicates
	if (m_sv_components.find(component) == m_sv_components.end()) {
		m_sv_components.insert(component);
		return true;
	} else {
		return false;
	}
}

bool GameNetworkManager::deregisterServerComponent(GameComponent* component) {

	if (m_sv_components.find(component) != m_sv_components.end()) {
		m_sv_components.erase(component);
		return true;
	} else {
		return false;
	}
}

bool GameNetworkManager::setOption(GameEngine::Network::NetworkOption option, const unsigned long value) {  // long holds at least 4 bytes

	switch (option) {
		case GameEngine::Network::SV_IP:
			if (m_currentState == GameEngine::Network::DISCONNECTED) {
#if defined PLATFORM_WIN
				m_sv_adress.sin_addr.S_un.S_addr = value;
#else
				m_sv_adress.sin_addr.s_addr = (uint32_t)value;
#endif
				return true;
			} else {
				return false;
			}
			break;


		case GameEngine::Network::SV_PORT:
			if (m_currentState == GameEngine::Network::DISCONNECTED) {
				m_sv_adress.sin_port = htons((uint16_t)value);
				return true;
			} else {
				return false;
			}
			break;


		case GameEngine::Network::SV_TICKRATE:
			m_sv_tickinterval = (uint32_t)value;
			return true;


		case GameEngine::Network::SV_CLIENT_TIMEOUT_INTERVAL:
			m_sv_clientTimeoutInterval = (uint32_t)value;
			return true;


		case GameEngine::Network::CL_SERVERPORT:
			if (m_currentState == GameEngine::Network::DISCONNECTED) {
				m_cl_serveradress.sin_port = htons((uint16_t)value);
				return true;
			} else {
				return false;
			}
			break;


		case GameEngine::Network::CL_TICKRATE:
			m_cl_tickinterval = (uint32_t)value;
			return true;

		default:
			return false;
	}

	return false;
}

bool GameNetworkManager::setOption(GameEngine::Network::NetworkOption option, const char *value) {

	switch (option) {
		case GameEngine::Network::SV_IP:
			if (m_currentState == GameEngine::Network::DISCONNECTED) {
#if defined PLATFORM_WIN
				m_sv_adress.sin_addr.S_un.S_addr = inet_addr(value);
#else
				inet_pton(AF_INET, value, &m_sv_adress.sin_addr);
#endif
				return true;
			} else {
				return false;
			}
			break;

		case GameEngine::Network::APPLICATION_ID:
			if (m_applicationID) {
				delete[] m_applicationID;
				m_applicationID = 0;
			}
			m_applicationID = new char[strlen(value) + 1];
			strcpy_s(m_applicationID, strlen(value) + 1, value);
			break;

		default:
			return false;
	}

	return false;
}

bool GameNetworkManager::setOption(GameEngine::Network::NetworkOption option, const bool value) {

	switch (option) {
		case GameEngine::Network::USE_TCP:
			if (m_currentState == GameEngine::Network::DISCONNECTED) {
				m_useTCP = value;
			} else {
				return false;
			}
			break;

		case GameEngine::Network::USE_COMPRESSION:
			m_useCompression = value;
			return true;

		default:
			return false;
	}

	return false;
}

void GameNetworkManager::requestClientUpdate(unsigned long clientID, const char* entityID, const char* componentID) {
	std::map<uint32_t,ClientRecord*>::iterator cl_it = m_sv_clients.find((uint32_t)clientID);

	if (cl_it == m_sv_clients.end())
		// client ID not found
		return;

	cl_it->second->requestUpdate(entityID, componentID);
}

void GameNetworkManager::disrequestClientUpdate(unsigned long clientID, const char* entityID, const char* componentID) {
	std::map<uint32_t,ClientRecord*>::iterator cl_it = m_sv_clients.find((uint32_t)clientID);

	if (cl_it == m_sv_clients.end())
		// client not found
		return;

	cl_it->second->disrequestUpdate(entityID, componentID);
}

void GameNetworkManager::registerCallbackOnClientConnect(void (*callback)(unsigned long)) {
	m_onClientConnect = callback;
}

void GameNetworkManager::registerCallbackOnClientDisconnect(void (*callback)(unsigned long)) {
	m_onClientDisconnect = callback;
}

void GameNetworkManager::registerCallbackOnStateRequest(void (*callback)(std::string, std::string)) {
	m_onStateRequest = callback;
}
