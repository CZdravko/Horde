/*
 * SceneGrapgPlugin.h
 *
 *  Created on: 10. dec. 2013
 *      Author: ZCule
 */

#ifndef SCENEGRAPGPLUGIN_H_
#define SCENEGRAPGPLUGIN_H_

#include <GameEngine/IPlugin.h>
//#include <GameEngine/config.h>

#ifdef GAMEENGINE_EXPORTS
#       define DLLEXP __declspec( dllexport )
#		define GAMEENGINE_API __declspec(dllexport)
#	 else
#       define DLLEXP __declspec( dllimport )
#		define GAMEENGINE_API __declspec(dllimport)
#endif

namespace GameEngine {

//struct lua_State;

class DLLEXP SceneGrapgPlugin: public IPlugin {
public:
	SceneGrapgPlugin();
	virtual ~SceneGrapgPlugin();
	void dllLoadGamePlugin(void);
	void dllUnloadGamePlugin(void);
	void dllLoadScene(const char* sceneFile);
	void dllRegisterLuaStack(lua_State* L);
	const char* getName();
};

} /* namespace GameEngine */

#endif /* SCENEGRAPGPLUGIN_H_ */
