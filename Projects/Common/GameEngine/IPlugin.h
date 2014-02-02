/*
 * IPlugin.h
 *
 *  Created on: 9. dec. 2013
 *      Author: ZCule
 */

#ifndef IPLUGIN_H_
#define IPLUGIN_H_


struct lua_State;

class IPlugin {
public:
	//IPlugin();
	virtual ~IPlugin(){};
	virtual void dllLoadGamePlugin(void) = 0;
	virtual void dllUnloadGamePlugin(void) = 0;
	virtual void dllLoadScene(const char* sceneFile) = 0;
	virtual void dllRegisterLuaStack( lua_State* L ) = 0;
	virtual const char* getName() = 0;
};


#endif /* IPLUGIN_H_ */
