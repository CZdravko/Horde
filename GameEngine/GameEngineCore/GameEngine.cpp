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

#include "GameWorld.h"
#include "GameEntity.h"
#include "GameLog.h"

#include "GameModules.h"
#include "GameNetworkManager.h"
#include "GamePlugInManager.h"
#include "GameComponentRegistry.h"
#include "TimingManager.h"

#include <XMLParser/utXMLParser.h>

#if defined PLATFORM_WIN
#	include <direct.h>
#else
#	include <unistd.h>
#	ifndef max
#		define max(a,b) (((a) > (b)) ? (a) : (b))
#	endif
#endif

#include <stdio.h>

namespace GameEngine
{
	/// Global variables
	bool Initialized		   = false;		
	
	bool checkXMLParseResult(const XMLResults& results)
	{
		switch (results.error)
		{
		case eXMLErrorNone:
			return true;
		default:
			GameLog::errorMessage("Parsing error in line %d column %d: %s", results.nLine, results.nColumn, XMLNode::getError(results.error));
			return false;
		}
	}

	/***********      Initialization of a new  Scene *********/
	GAMEENGINE_API bool init(const char* pluginFile /*= 0x0*/)
	{
		GameLog::logMessage("----GameEngine initializing...----");
		if (Initialized)
		{
			GameLog::warnMessage("%s, Line %d: GameEngine already initialized", __FILE__, __LINE__);
			return false;
		}
		
//		if( !GameModules::plugInManager()->init(pluginFile) )
//			return false;

		if( !GameModules::networkManager()->init() ) {
			GameLog::errorMessage("NetworkManager failed to initialize.");
			return false;
		}

		GameLog::logMessage("----GameEngine successfuly initialized----");
		Initialized = true;
		return true;
	}
	GAMEENGINE_API void release()
	{
		if( Initialized )
		{
			Initialized = false;
			GameModules::release();
			GameLog::close();
		}
	}
	GAMEENGINE_API void releaseAllEntities()
	{
		if (Initialized)
		{
			GameModules::releaseGameWorld();
		}
	}
	GAMEENGINE_API bool isInitialized()
	{
		return Initialized;
	}


	GAMEENGINE_API void update()
	{
		// Update Network Manager
		GameModules::networkManager()->update();

		// Update all components depending on the time
		TimingManager::update();

		/***
		 *   Update Component managers
		 ***/
		GameModules::componentRegistry()->updateComponentManagers();
	}

	GAMEENGINE_API size_t componentNames(char* names, size_t size)
	{
		return GameModules::componentRegistry()->componentNames( names, size );
	}

	GAMEENGINE_API float FPS()
	{
		return TimingManager::FPS();
	}


	GAMEENGINE_API float timeStamp()
	{
		return TimingManager::timeStamp();
	}


	GAMEENGINE_API float currentTimeStamp()
	{
		return TimingManager::currentTimeStamp();
	}

	GAMEENGINE_API bool loadScene(const char* sceneFile)
	{
		if (sceneFile == 0) return false;

		GameLog::logMessage("----Loading scene file '%s'----", sceneFile);
		// get path of file loaded
		std::string fileNameStr = sceneFile;
		size_t lastSeparator1 = fileNameStr.find_last_of( '/' );
		size_t lastSeparator2 = fileNameStr.find_last_of( '\\' );		
		if (lastSeparator1 != std::string::npos || lastSeparator2 != std::string::npos )
		{
			size_t index = max( (int) lastSeparator1, (int) lastSeparator2 );
			// change working directory to directory containing the scene file
#if defined PLATFORM_WIN
			_chdir(fileNameStr.substr(0, index).c_str());
#else
			chdir(fileNameStr.substr(0, index).c_str());
#endif
			fileNameStr = fileNameStr.substr(index+1);
		}

		//// Read XML content of scn file
		XMLResults results;
		XMLNode scene = XMLNode::parseFile(fileNameStr.c_str(), "Configuration", &results);
		if (checkXMLParseResult(results) && !scene.isEmpty())
		{				
			GameModules::plugInManager()->loadScene( fileNameStr.c_str() );				

			// Load extras
			XMLNode extras(scene.getChildNode("Extras"));
			if (!extras.isEmpty())
			{
				int childs = extras.nChildNode("GameEntity");
				for (int i = 0; i < childs; ++i)
				{
					// Load extra nodes
					createGameEntity(extras.getChildNode("GameEntity", i));
				}				
			}
		}
		else
		{				
			GameLog::errorMessage("Error loading scene file '%s'\n", sceneFile);
			return false;
		}
		// Why should we reset the time after loading a scene?
		// Some components/managers may already have initialized with the current one
		//TimingManager::reset();
		// Update to initialize attached entities
		GameModules::componentRegistry()->updateComponentManagers();
		GameLog::logMessage("----Finished loading scene file '%s'----", sceneFile);
		return true;
	}

	GAMEENGINE_API unsigned int createGameEntity( const char *xmlText )
	{
		if (xmlText == 0 || strlen(xmlText) == 0)
			return 0;
		XMLResults results;
		// parse node attachment
		XMLNode attachment = XMLNode::parseString(xmlText, "GameEntity", &results);
		if (results.error == eXMLErrorNone)
			return createGameEntity(attachment); 
		else
			GameLog::errorMessage("Error reading GameEntity\n%s\n", xmlText);
		return 0;
	}

	unsigned int createGameEntity( const XMLNode& attachment )
	{
		const char* entityName = attachment.getAttribute("name");
		if (entityName == 0)
		{
			entityName = "unnamed";
			GameLog::warnMessage("The Attachment contains no name attribute, now calling it 'unnamed'!");
		}
		EntityID entityID = entityName;
			
		GameEntity* entity = GameModules::gameWorld()->createEntity(entityID);
	
		if (entity)
		{
			const int childNodes = attachment.nChildNode();
			for( int i = 0; i < childNodes; ++i )
			{
				const XMLNode& node = attachment.getChildNode(i);
				GameComponent* component = GameModules::componentRegistry()->createComponent(node.getName(), entity);
				if( component ) component->loadFromXml( &node );
				else GameLog::warnMessage( "No plugin found to handle '%s' nodes", node.getName() );
			}
			return entity->worldId();
		}

		return 0;
	}


	GAMEENGINE_API void removeGameEntity( unsigned int entityWorldID)
	{
		GameModules::gameWorld()->releaseEntity(entityWorldID);			
	}

	GAMEENGINE_API void renameGameEntity( unsigned int entityWorldID, const char* newEntityName )
	{
		GameModules::gameWorld()->renameEntity(entityWorldID, newEntityName);		
	}

	GAMEENGINE_API unsigned int entityWorldID( const char* entityID )
	{
		return GameModules::gameWorld()->entityID(entityID);
	}

	GAMEENGINE_API const char* entityName(unsigned int worldID)
	{
		GameEntity* t = GameModules::gameWorld()->entity(worldID);
		if(t)
		{
			return t->id().c_str();
		}
		return "";
	}
	GAMEENGINE_API bool setComponentData(unsigned int entityWorldID, const char* componentID, const char* xmlData)
	{
		GameEntity* entity = GameModules::gameWorld()->entity(entityWorldID);
		if (entity)
		{
			GameComponent* component = entity->component(componentID);
			if (xmlData != 0) // create or reload component if xmldata is valid
			{
				if (component == 0)
				{
					component = GameModules::componentRegistry()->createComponent(componentID, entity);
					if( component == 0x0 ) return false;
				}
				XMLResults pResults;
				XMLNode componentData = XMLNode::parseString(xmlData, 0, &pResults);
				if (checkXMLParseResult(pResults) && !componentData.isEmpty())
					component->loadFromXml(&componentData);
			}
			else // remove component if xmldata is a null pointer
				delete component;
			return true;
		}
		else
			return false;
	}

	GAMEENGINE_API void installPlugin(IPlugin* plugin){
		GameModules::plugInManager()->installPlugin(plugin);
	}
}
