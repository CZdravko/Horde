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

#include "GameEngine_Network.h"

#include "GameWorld.h"
#include "GameEntity.h"

#include "GameModules.h"
#include "GameNetworkManager.h"

namespace GameEngine
{
	GAMEENGINE_API void connectToServer(const char* ip_addr) {
		GameModules::networkManager()->connectToServer(ip_addr);
	}

	GAMEENGINE_API void startServer() {
		GameModules::networkManager()->setupServer();
	}

	GAMEENGINE_API void disconnect() {
		GameModules::networkManager()->disconnect();
	}

	GAMEENGINE_API bool startDistributing(const char* entityID, const char* componentID) {
		GameEntity* ge = GameModules::gameWorld()->entity(entityID);

		if (!ge)
			return false;

		GameComponent* gc = ge->component(componentID);

		if (!gc)
			return false;

		return GameModules::networkManager()->registerServerComponent(gc);
	}

	GAMEENGINE_API bool stopDistributing(const char* entityID, const char* componentID) {
		GameEntity* ge = GameModules::gameWorld()->entity(entityID);

		if (!ge)
			return false;

		GameComponent* gc = ge->component(componentID);

		if (!gc)
			return false;

		return GameModules::networkManager()->deregisterServerComponent(gc);
	}

	GAMEENGINE_API void requestClientUpdate(unsigned long clientID, const char* entityID, const char* componentID) {
		GameModules::networkManager()->requestClientUpdate(clientID, entityID, componentID);
	}

	GAMEENGINE_API void disrequestClientUpdate(unsigned long clientID, const char* entityID, const char* componentID) {
		GameModules::networkManager()->disrequestClientUpdate(clientID, entityID, componentID);
	}

	GAMEENGINE_API void registerCallbackOnClientConnect(void (*callback)(unsigned long)) {
		GameModules::networkManager()->registerCallbackOnClientConnect(callback);
	}

	GAMEENGINE_API void registerCallbackOnClientDisconnect(void (*callback)(unsigned long)) {
		GameModules::networkManager()->registerCallbackOnClientDisconnect(callback);
	}

	GAMEENGINE_API void registerCallbackOnStateRequest(void (*callback)(std::string, std::string)) {
		GameModules::networkManager()->registerCallbackOnStateRequest(callback);
	}

	GAMEENGINE_API Network::NetworkState getNetworkState() {
		return GameModules::networkManager()->getState();
	}

	GAMEENGINE_API bool setNetworkOption(GameEngine::Network::NetworkOption option, const unsigned long value) {
		return GameModules::networkManager()->setOption(option, value);
	}

	GAMEENGINE_API bool setNetworkOption(GameEngine::Network::NetworkOption option, const char* value) {
		return GameModules::networkManager()->setOption(option, value);
	}

	GAMEENGINE_API bool setNetworkOption(GameEngine::Network::NetworkOption option, const bool value) {
		return GameModules::networkManager()->setOption(option, value);
	}

	GAMEENGINE_API bool removeClient(const unsigned long clientID) {
		return GameModules::networkManager()->removeClient(clientID);
	}
}
