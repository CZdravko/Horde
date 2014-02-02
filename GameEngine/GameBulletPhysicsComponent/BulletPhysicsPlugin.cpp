/*
 * SceneGrapgPlugin.cpp
 *
 *  Created on: 10. dec. 2013
 *      Author: ZCule
 */

#include <GameEngine/BulletPhysicsPlugin.h>
#include <GameEngine/GameModules.h>
#include <GameEngine/GameLog.h>
#include <GameEngine/GameWorld.h>

#include <GameEngine/GameComponentRegistry.h>

#include <iostream>

#include "Physics.h"
#include "PhysicsComponent.h"
#include "PhysicsLua.h"

#include <XMLParser/utXMLParser.h>
#include <Horde3D/Horde3DUtils.h>

namespace GameEngine {

//struct lua_State;

BulletPhysicsPlugin::BulletPhysicsPlugin() {
	// TODO Auto-generated constructor stub

}

BulletPhysicsPlugin::~BulletPhysicsPlugin() {
	// TODO Auto-generated destructor stub
}

} /* namespace GameEngine */

void GameEngine::BulletPhysicsPlugin::dllLoadGamePlugin(void) {
	GameLog::logMessage(" Initialization of Horde3D BulletPhysicsPlugin! ");
	printf(" Initialization of Horde3D BulletPhysicsPlugin! ");
	GameModules::componentRegistry()->registerComponent( "BulletPhysics", PhysicsComponent::createComponent );
		// For compatibility reasons
		GameModules::componentRegistry()->registerComponent( "Physics", PhysicsComponent::createComponent);
		GameModules::componentRegistry()->registerManager( Physics::instance() );
}

void GameEngine::BulletPhysicsPlugin::dllUnloadGamePlugin(void) {
	GameModules::componentRegistry()->registerComponent( "BulletPhysics", 0);
		// For compatibility reasons
		GameModules::componentRegistry()->registerComponent( "Physics", 0);
		GameModules::componentRegistry()->unregisterManager( Physics::instance() );
		Physics::release();
}

void GameEngine::BulletPhysicsPlugin::dllLoadScene(const char* sceneFile) {

}

void GameEngine::BulletPhysicsPlugin::dllRegisterLuaStack(lua_State* L) {
	PhysicsLua::registerLuaBindings( L );
}

const char* GameEngine::BulletPhysicsPlugin::getName() {
	return "BulletPhysics";
}

