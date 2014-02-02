/*
 * AnimationPlugin.h
 *
 *  Created on: Dec 27, 2013
 *      Author: yama
 */

#ifndef ANIMATIONPLUGIN_H_
#define ANIMATIONPLUGIN_H_

#include <GameEngine/IPlugin.h>
#include <GameEngine/config.h>

namespace GameEngine {

class DLLEXP AnimationPlugin: public IPlugin {
public:
	AnimationPlugin();
	virtual ~AnimationPlugin();
	virtual void dllLoadGamePlugin(void);
	virtual void dllUnloadGamePlugin(void);
	virtual void dllLoadScene(const char* sceneFile);
	virtual void dllRegisterLuaStack(lua_State* L);
	virtual const char* getName();
};

}

#endif /* ANIMATIONPLUGIN_H_ */
