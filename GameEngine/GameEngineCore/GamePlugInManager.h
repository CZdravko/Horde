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
// Copyright (C) 2007-2011 Volker Wiendl, Felix Kistler
// 
// ****************************************************************************************
#ifndef GAMEPLUGINMANAGER_H_
#define GAMEPLUGINMANAGER_H_

#include <GameEngine/config.h>

#include <map>
#include <string>

#include <GameEngine/IPlugin.h>

class DynLib;
struct lua_State;
struct SQVM;

/** \addtogroup GameEngineCore
 * @{
 */

/**
 * \brief Handles all plugin instances 
 * 
 * The PlugIn Manager takes response to load and unload the GameEngine plugins that contains
 * the components of entities. 
 * 
 * @author Volker Wiendl
 * @date Jun 2008
 */ 
class GamePlugInManager
{
	friend class GameModules;

	/// \typedef Prototype for loading a plugin (MUST be implemented by plugins)
	typedef void (*LOAD_GAME_PLUGIN)(void);

	/// \typedef UNLOAD_GAME_PLUGIN Prototype for unloading a plugin (MUST be implemented by plugins)
	typedef void (*UNLOAD_GAME_PLUGIN)(void);

	/// \typedef Prototype for scene loading method in plugins
	typedef void (*LOAD_SCENE)( const char* scene );

	/// \typedef Prototype for lua registration method in plugins
	typedef void (*REGISTER_LUA_STACK)( lua_State* L );

public:
	/**
	 * \brief Loads plugins
	 * 
	 * When calling this method all plugins defined in a 'plugin.cfg' file will be loaded.
	 * This file has to be stored in the same directory as the GameEngineCore library.
	 * For the debug version of the GameEngineCore the file is called 'plugin_debug.cfg'.
	 * 
	 * @param pluginFile (optional) defines the path of a text file that includes all GameEngine component dlls to be loaded (plugin.cfg)
	 * @return true if the plugin file has been read successfully, false otherwise (note
	 * that if plugins failed to load this method still returns true, but error messages
	 * will be written to the GameEngineLog.html)
	 */ 
	bool init(const char* pluginFile = 0x0);

	/**
	 * \brief Unloads all plugins
	 * 
	 * To unload the dynamically loaded plugins you have to call release()
	 */ 
	void release();
	
	/**
	 * \brief Informs the plugins about the loading of the specified scene
	 * 
	 * @param sceneFile a scenefile currently loaded by the GameEngineCore
	 */ 
	void loadScene( const char* sceneFile );

	/**
	 * \brief Register lua binding of plugins
	 * 
	 * The GameEngine itself does not provide a lua stack. If the application
	 * that uses the GameEngine want's to control the GameEngine using lua, it
	 * can create a lua stack and call this method afterwards. This way all 
	 * lua bindings will be registered on the application's lua stack.
	 * @param L 
	 */ 
	void registerLuaStack( lua_State* L );

	//Manual Plugin installation
	void installPlugin(IPlugin* plugin);

private:
	/// Private Contstructor ( friend class GameModules )
	GamePlugInManager(); 
	/// Priate Destructor (friend class GameModules )
	~GamePlugInManager();

	/**
	 * \brief Loads plugin from directory in an environment variable
	 *
	 * @param dllname the name of the dll (with extension)
	 * @param envVarName name of the environment variable
	 */ 
	void loadPluginFromEnvVar(const char* dllName, const std::string& envVarName);

	/**
	 * \brief Searches and loads plugin dlls with their name pattern in the
	 * given path
	 * @param path the directory path for the dll (ending with a slash)
	 * @return true if at least one matching dll was found
	 * 
	 */ 
	bool searchPluginDlls(const char* path);

	/**
	 * \brief Tries to load the dll with the given filepath
	 *
	 * @param path the directory path for the dll (ending with a slash)
	 * @param dllname the name of the dll (with extension)
	 * @return true if the loading was successful
	 * 
	 */
	bool loadDll(const char* path, const char* dllname);

	/// All currently loaded plugin instances
	std::map<std::string, DynLib*> m_plugIns;


	/// All manually loaded plugin instances
	std::map<std::string, IPlugin*> m_manualPlugIns;

};

/*! @}*/
#endif
