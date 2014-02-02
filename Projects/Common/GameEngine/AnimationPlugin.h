/*
 * AnimationPlugin.h
 *
 *  Created on: Dec 27, 2013
 *      Author: yama
 */

#ifndef ANIMATIONPLUGIN_H_
#define ANIMATIONPLUGIN_H_

#include <GameEngine/IPlugin.h>

#ifdef GAMEENGINE_EXPORTS
#       define DLLEXP __declspec( dllexport )
#		define GAMEENGINE_API __declspec(dllexport)
#	 else
#       define DLLEXP __declspec( dllimport )
#		define GAMEENGINE_API __declspec(dllimport)
#endif

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
