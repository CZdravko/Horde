/*
 * AnimationPlugin.cpp
 *
 *  Created on: Dec 27, 2013
 *      Author: yama
 */

#include "AnimationPlugin.h"
#include <GameEngine/GameModules.h>
#include <GameEngine/GameLog.h>
#include <GameEngine/GameComponentRegistry.h>

#include <XMLParser/utXMLParser.h>

#include <Horde3D/Horde3DUtils.h>

#include <iostream>

#include "MoveAnimManager.h"
#include "MoveAnimComponent.h"
#include "KeyframeAnimComponent.h"
#include "KeyframeAnimManager.h"
#include "MorphtargetAnimComponent.h"
#include "MorphtargetAnimManager.h"
#include "AnimationLua.h"

GameEngine::AnimationPlugin::AnimationPlugin() {
}

GameEngine::AnimationPlugin::~AnimationPlugin() {
}

void GameEngine::AnimationPlugin::dllLoadGamePlugin(void) {
	GameModules::componentRegistry()->registerComponent( "KeyframeAnimation", KeyframeAnimComponent::createComponent );
		GameModules::componentRegistry()->registerComponent( "MoveAnimation", MoveAnimComponent::createComponent );
		GameModules::componentRegistry()->registerComponent( "MorphtargetAnimation", MorphtargetAnimComponent::createComponent );

		GameModules::componentRegistry()->registerManager( MoveAnimManager::instance() );
		GameModules::componentRegistry()->registerManager( KeyframeAnimManager::instance() );
		GameModules::componentRegistry()->registerManager( MorphtargetAnimManager::instance() );

		h3dInit(0);
}

void GameEngine::AnimationPlugin::dllUnloadGamePlugin(void) {
	GameModules::componentRegistry()->registerComponent( "KeyframeAnimation", 0 );
		GameModules::componentRegistry()->registerComponent( "MoveAnimation", 0 );
		GameModules::componentRegistry()->registerComponent( "MorphtargetAnimation", 0 );

		GameModules::componentRegistry()->unregisterManager( MoveAnimManager::instance() );
		GameModules::componentRegistry()->unregisterManager( KeyframeAnimManager::instance() );
		GameModules::componentRegistry()->unregisterManager( MorphtargetAnimManager::instance() );

		MoveAnimManager::release();
		KeyframeAnimManager::release();
		MorphtargetAnimManager::release();

		h3dRelease();
}

void GameEngine::AnimationPlugin::dllLoadScene(const char* sceneFile) {
}

void GameEngine::AnimationPlugin::dllRegisterLuaStack(lua_State* L) {
	AnimationLua::registerLuaBindings( L );
}

const char* GameEngine::AnimationPlugin::getName() {
	return "AnimationPlugin";
}
