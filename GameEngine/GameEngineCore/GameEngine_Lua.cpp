// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the GameEngine developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************
//


// ****************************************************************************************
//
// GameEngine Core Library of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#include "GameEngine.h"

extern "C"
{
#include "Lua/lua.h"
#include "Lua/lualib.h"
#include "Lua/lauxlib.h"
}

#include "GamePlugInManager.h"
#include "GameModules.h"

namespace GameEngine
{
	namespace LuaBindings
	{
		/**
		 * Returns the current FPS
		 */ 
		int GameEngine_FPS(lua_State* L)
		{
			lua_pushnumber(L, GameEngine::FPS());
			return 1;
		}

		//int GameEngine_setActiveCamera(lua_State* L)
		//{
		//	GameEngine::setActiveCamera(luaL_checkstring(L,1));
		//	return 0;
		//}

		static const luaL_reg gameengine_meta_methods[] = {
			{0,0}
		};

		const luaL_reg gameengine_methods[] = {
			{"FPS",							GameEngine_FPS},
			//{"setActiveCamera",				GameEngine_setActiveCamera},
			{0,0}
		};
	}

	#define newtable(L) (lua_newtable(L), lua_gettop(L))

	void registerLuaStack(lua_State *L)
	{
		int metatable, methods;

		lua_pushliteral(L, "GameEngine");         /* name of GameEngine table */
		methods   = newtable(L);           /* GameEngine methods table */
		metatable = newtable(L);           /* GameEngine metatable */
		lua_pushliteral(L, "__index");     /* add index event to metatable */
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);        /* metatable.__index = methods */
		lua_pushliteral(L, "__metatable"); /* hide metatable */
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);        /* metatable.__metatable = methods */
		luaL_openlib(L, 0, LuaBindings::gameengine_meta_methods,  0); /* fill metatable */
		luaL_openlib(L, 0, LuaBindings::gameengine_methods, 1); /* fill GameEngine methods table */
		lua_settable(L, LUA_GLOBALSINDEX); /* add GameEngine to globals */

		GameModules::plugInManager()->registerLuaStack( L );

	}
}