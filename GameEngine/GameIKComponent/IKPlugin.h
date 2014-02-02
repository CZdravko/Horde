/*
 * IKPlugin.h
 *
 *  Created on: Dec 27, 2013
 *      Author: yama
 */

#ifndef IKPLUGIN_H_
#define IKPLUGIN_H_

#include <GameEngine/IPlugin.h>
#include <GameEngine/config.h>

namespace GameEngine{

class DLLEXP IKPlugin: public IPlugin {
public:
	IKPlugin();
	virtual ~IKPlugin();
	virtual void dllLoadGamePlugin(void);
	virtual void dllUnloadGamePlugin(void);
	virtual void dllLoadScene(const char* sceneFile);
	virtual void dllRegisterLuaStack(lua_State* L);
	virtual const char* getName();
};

}
#endif /* IKPLUGIN_H_ */
