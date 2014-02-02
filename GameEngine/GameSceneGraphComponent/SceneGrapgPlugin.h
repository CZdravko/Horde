/*
 * SceneGrapgPlugin.h
 *
 *  Created on: 10. dec. 2013
 *      Author: ZCule
 */

#ifndef SCENEGRAPGPLUGIN_H_
#define SCENEGRAPGPLUGIN_H_

#include <GameEngine/IPlugin.h>
#include <GameEngine/config.h>

namespace GameEngine {

//struct lua_State;

class DLLEXP SceneGrapgPlugin: public IPlugin {
public:
	SceneGrapgPlugin();
	virtual ~SceneGrapgPlugin();
	virtual void dllLoadGamePlugin(void);
	virtual void dllUnloadGamePlugin(void);
	virtual void dllLoadScene(const char* sceneFile);
	virtual void dllRegisterLuaStack(lua_State* L);
	virtual const char* getName();
};

} /* namespace GameEngine */

#endif /* SCENEGRAPGPLUGIN_H_ */
