/*
 * IKPlugin.cpp
 *
 *  Created on: Dec 27, 2013
 *      Author: yama
 */

#include "IKPlugin.h"
#include <GameEngine/GameModules.h>
#include <GameEngine/GameComponentRegistry.h>
#include <GameEngine/GameLog.h>

#include "IKManager.h"
#include "IKComponent.h"
#include "Config.h"




GameEngine::IKPlugin::IKPlugin() {
}

GameEngine::IKPlugin::~IKPlugin() {
}

void GameEngine::IKPlugin::dllLoadGamePlugin(void) {
	GameModules::componentRegistry()->registerComponent( "IK", IKComponent::createComponent );
		GameModules::componentRegistry()->registerManager( IKManager::instance() );
}

void GameEngine::IKPlugin::dllUnloadGamePlugin(void) {
	GameModules::componentRegistry()->registerComponent( "IK", 0);
		GameModules::componentRegistry()->unregisterManager( IKManager::instance() );
		IKManager::release();
}

void GameEngine::IKPlugin::dllLoadScene(const char* sceneFile) {
}

void GameEngine::IKPlugin::dllRegisterLuaStack(lua_State* L) {
}

const char* GameEngine::IKPlugin::getName() {
	return "IKPlugin";
}
