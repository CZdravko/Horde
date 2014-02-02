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
// GameEngine SceneGraph Component of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#include <GameEngine/GameModules.h>
#include <GameEngine/GameLog.h>
#include <GameEngine/GameWorld.h>

#include <GameEngine/GameComponentRegistry.h>

#include <iostream>

#include "SceneGraph.h"
#include "SceneGraphComponent.h"
#include "SceneGraphLua.h"

#include <XMLParser/utXMLParser.h>
#include <Horde3D/Horde3DUtils.h>

struct lua_State;

#ifdef PLATFORM_WIN
#    define SCENEGRAPHPLUGINEXP extern "C" __declspec( dllexport )
#else
#	 define SCENEGRAPHPLUGINEXP extern "C"
#endif

SCENEGRAPHPLUGINEXP void dllLoadGamePlugin(void)
{
	GameModules::componentRegistry()->registerComponent( "Horde3D", SceneGraphComponent::createComponent );	
	GameModules::componentRegistry()->registerManager( SceneGraphManager::instance() );	
	if ( !h3dInit(0) )
		GameLog::errorMessage(" Initialization of Horde3D failed! ");
}

SCENEGRAPHPLUGINEXP void dllUnloadGamePlugin(void)
{
	GameModules::componentRegistry()->registerComponent( "Horde3D", 0 );	
	GameModules::componentRegistry()->unregisterManager( SceneGraphManager::instance() );
	SceneGraphManager::release();
	h3dRelease();
}

SCENEGRAPHPLUGINEXP void dllRegisterLuaStack( lua_State* L )
{
	//SceneGraphLua::registerLuaBindings( L );
}

SCENEGRAPHPLUGINEXP void dllLoadScene( const char* sceneFile )
{
	GameLog::logMessage("Setting Horde3D Configuration");
	
	XMLResults results;
	XMLNode scene = XMLNode::parseFile( sceneFile, "Configuration", &results);
	const XMLNode& engineSettings(scene.getChildNode("EngineConfig"));
	if ( !engineSettings.isEmpty() )
	{
		int maxLogLevel = atoi(engineSettings.getAttribute("maxLogLevel", "4"));
		GameLog::logMessage("MaxLogLevel: %d", maxLogLevel);
		h3dSetOption( H3DOptions::MaxLogLevel, float( maxLogLevel ) );

		int maxNumMessages = atoi(engineSettings.getAttribute("maxNumMessages", "1024"));
		GameLog::logMessage("MaxNumMessages: %d", maxNumMessages);
		h3dSetOption( H3DOptions::MaxNumMessages, float( maxNumMessages ) );

		int trilinearFiltering = atoi(engineSettings.getAttribute("trilinearFiltering", "1"));
		GameLog::logMessage("TrilinearFiltering: %d", trilinearFiltering);
		h3dSetOption( H3DOptions::TrilinearFiltering, float( trilinearFiltering ) );

		int maxAnisotropy = atoi(engineSettings.getAttribute("maxAnisotropy", "1"));
		GameLog::logMessage("MaxAnisotropy: %d", maxAnisotropy);
		h3dSetOption( H3DOptions::MaxAnisotropy, float( maxAnisotropy ) );

		bool sRGBLinearization = 
			_stricmp(engineSettings.getAttribute("sRGBLinearization", "0"), "true") == 0 ||
			_stricmp(engineSettings.getAttribute("sRGBLinearization", "0"), "1") == 0;
		GameLog::logMessage("SRGBLinearization: %s", sRGBLinearization ? "enabled" : "disabled");
		h3dSetOption( H3DOptions::SRGBLinearization, sRGBLinearization );

//		bool texCompression =
//			_stricmp(engineSettings.getAttribute("texCompression", "0"), "true") == 0 ||
//			_stricmp(engineSettings.getAttribute("texCompression", "0"), "1") == 0;
//		GameLog::logMessage("TexCompression: %s", texCompression ? "enabled" : "disabled");
//		h3dSetOption( H3DOptions::TexCompression, texCompression );

		bool loadTextures = 
			_stricmp(engineSettings.getAttribute("loadTextures", "1"), "1") == 0
			|| _stricmp(engineSettings.getAttribute("loadTextures", "1"), "true") == 0;
		GameLog::logMessage("LoadTextures: %s", loadTextures ? "enabled" : "disabled");
		h3dSetOption( H3DOptions::LoadTextures, loadTextures ? 1.0f : 0.0f );

		const XMLNode& pathes(scene.getChildNode("EnginePath"));
		if (!pathes.isEmpty())
		{
			GameLog::logMessage("AnimationPath: %s", pathes.getAttribute("animationpath", "animations"));
			h3dutSetResourcePath(H3DResTypes::Animation, pathes.getAttribute("animationpath", "animations"));
		}
		const XMLNode& engineSettings(scene.getChildNode("EngineConfig"));
		if ( !engineSettings.isEmpty() )
		{				
			bool fastAnim = 
				_stricmp(engineSettings.getAttribute("fastAnimation", "false"), "true") == 0 ||
				_stricmp(engineSettings.getAttribute("fastAnimation", "0"), "1") == 0;
			GameLog::logMessage("FastAnimation: %s", fastAnim ? "enabled" : "disabled");
			h3dSetOption( H3DOptions::FastAnimation, fastAnim ? 1.0f : 0.0f );
		}

		int shadowMapSize = atoi(engineSettings.getAttribute("shadowMapSize", "1024"));					
		GameLog::logMessage("ShadowMapSize: %d", shadowMapSize);
		h3dSetOption( H3DOptions::ShadowMapSize, float( shadowMapSize) );

		int sampleCount = atoi(engineSettings.getAttribute("sampleCount", "0"));					
		GameLog::logMessage("SampleCount: %d", sampleCount);
		h3dSetOption( H3DOptions::SampleCount, float( sampleCount) );

		bool wireframeMode = 
			_stricmp(engineSettings.getAttribute("wireframeMode", "0"), "1") == 0 ||
			_stricmp(engineSettings.getAttribute("wireframeMode", "0"), "true") == 0;
		GameLog::logMessage("WireframeMode: %s", wireframeMode ? "enabled" : "disabled");
		h3dSetOption( H3DOptions::WireframeMode, wireframeMode );

		bool debugViewMode = 
			_stricmp(engineSettings.getAttribute("debugViewMode", "0"), "1") == 0 ||
			_stricmp(engineSettings.getAttribute("debugViewMode", "0"), "true") == 0;
		GameLog::logMessage("DebugViewMode: %s", debugViewMode ? "enabled" : "disabled");
		h3dSetOption( H3DOptions::DebugViewMode, debugViewMode );

		bool dumpFailedShaders = 
			_stricmp(engineSettings.getAttribute("dumpFailedShaders", "0"), "1") == 0 ||
			_stricmp(engineSettings.getAttribute("dumpFailedShaders", "0"), "true") == 0;
		GameLog::logMessage("DumpFailedShaders: %s", dumpFailedShaders ? "enabled" : "disabled");
		h3dSetOption( H3DOptions::DumpFailedShaders, dumpFailedShaders );

		bool gatherTimeStats = 
			_stricmp(engineSettings.getAttribute("gatherTimeStats", "1"), "1") == 0 ||
			_stricmp(engineSettings.getAttribute("gatherTimeStats", "1"), "true") == 0;
		GameLog::logMessage("GatherTimeStats: %s", gatherTimeStats ? "enabled" : "disabled");
		h3dSetOption( H3DOptions::GatherTimeStats, gatherTimeStats );

	}

	const XMLNode& stereoSettings(scene.getChildNode("StereoscopyConfig"));
	if ( !stereoSettings.isEmpty() )
	{
		const char* modeC = stereoSettings.getAttribute("mode", "0");
		unsigned int mode = 0;
		if (_stricmp(modeC, "0") == 0 || _stricmp(modeC, "disabled") == 0)
			mode = 0;
		else if (_stricmp(modeC, "1") == 0 || _stricmp(modeC, "sideBySide") == 0)
			mode = 1;
		else if (_stricmp(modeC, "2") == 0 || _stricmp(modeC, "quadBuffering") == 0)
			mode = 2;
		SceneGraphManager::instance()->setStereoscopyMode(mode);

		const char* methodC = stereoSettings.getAttribute("method", "0");
		unsigned int method = 0;
		if (_stricmp(methodC, "0") == 0 || _stricmp(methodC, "toedIn") == 0)
			method = 0;
		else if (_stricmp(methodC, "1") == 0 || _stricmp(methodC, "asymmetricFrustum") == 0)
			method = 1;
		else if (_stricmp(methodC, "2") == 0 || _stricmp(methodC, "oculusRift") == 0)
			method = 2;
		SceneGraphManager::instance()->setStereoscopyMethod(method);

		SceneGraphManager::instance()->setStereoscopyParams((float)atof(stereoSettings.getAttribute("eyeOffset", "0.05")),
			(float)atof(stereoSettings.getAttribute("strabismus", "2.5")), (float)atof(stereoSettings.getAttribute("focalLength", "1")));

		const char* stereoPipeline = stereoSettings.getAttribute("renderPipeline");
		if (stereoPipeline)
		{
			int resID = h3dAddResource( H3DResTypes::Pipeline, stereoPipeline, 0 );
			h3dutLoadResourcesFromDisk( "." );
			if (resID > 0)
				SceneGraphManager::instance()->setStereoPipelineResource(resID);
		}
		const char* stereoOverlayMaterial = stereoSettings.getAttribute("overlayMaterial");
		if (stereoOverlayMaterial)
		{
			int materialId = h3dAddResource( H3DResTypes::Material, stereoOverlayMaterial, 0 );
			h3dutLoadResourcesFromDisk( "." );
			if (materialId > 0)
				SceneGraphManager::instance()->setStereoOverlayMaterial(materialId);
		}
	}

	// Loading scene graph
	const XMLNode& sceneGraph(scene.getChildNode("SceneGraph"));
	if ( sceneGraph.isEmpty() )
		GameLog::errorMessage("No Scene Graph Node found!");
	else
	{
		GameLog::logMessage("Loading SceneGraph %s", sceneGraph.getAttribute("path"));
		// Environment
		H3DRes sceneGraphID = h3dAddResource( H3DResTypes::SceneGraph, sceneGraph.getAttribute("path"), 0 );
		GameLog::logMessage("Loading Resources...");
		// Load resources
		h3dutLoadResourcesFromDisk( "." );
		GameLog::logMessage("Adding scene graph to root node");
		// Add scene nodes	
		H3DNode newSceneID = h3dAddNodes( H3DRootNode, sceneGraphID);
		SceneGraphManager::instance()->addNewHordeNode( newSceneID );
	}
	// Use the specified render cam
	if (scene.getChildNode("ActiveCamera").getAttribute("name") && h3dFindNodes( H3DRootNode, scene.getChildNode("ActiveCamera").getAttribute("name"), H3DNodeTypes::Camera ) > 0)
		SceneGraphManager::instance()->setActiveCam( h3dGetNodeFindResult(0) );

}
