/*
 * SceneGrapgPlugin.h
 *
 *  Created on: 10. dec. 2013
 *      Author: ZCule
 */

#ifndef BPPLUGIN_H_
#define BPPLUGIN_H_

#include <GameEngine/IPlugin.h>
#include <GameEngine/config.h>

namespace GameEngine {

//struct lua_State;

class DLLEXP BulletPhysicsPlugin: public IPlugin {
public:
	BulletPhysicsPlugin();
	virtual ~BulletPhysicsPlugin();
	virtual void dllLoadGamePlugin(void);
	virtual void dllUnloadGamePlugin(void);
	virtual void dllLoadScene(const char* sceneFile);
	virtual void dllRegisterLuaStack(lua_State* L);
	virtual const char* getName();
};

} /* namespace GameEngine */

#endif /* SCENEGRAPGPLUGIN_H_ */
