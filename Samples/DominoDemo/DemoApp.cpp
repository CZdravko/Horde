#include <iostream>

#include "DemoApp.h"

#include "GameEngine/GameEngine.h"
#include "GameEngine/GameEngine_SceneGraph.h"
#include "GameEngine/GameEngine_BulletPhysics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Horde3D/Horde3D.h"
#include "Horde3D/Horde3DUtils.h"

#if defined ( WIN32 ) || defined( _WINDOWS )
#include <GLFW/glfw.h>
#else
#include <GL/glfw.h>
#include <stdlib.h>
#endif

DemoApp::DemoApp() : m_running(true)
{
	memset(m_keys, 0, sizeof(m_keys));
	m_mousedown = 0;
	m_lastx = 0;
	m_lasty = 0;
}

DemoApp::~DemoApp()
{
	GameEngine::release();
}

bool DemoApp::init(const char *fileName)
{
	if (GameEngine::init() && GameEngine::loadScene(fileName))
	{
		m_camID = GameEngine::entityWorldID("camera");
		m_camRX =0;
		m_camRY =0;
		return true;
	}
	else
		return false;
}

void DemoApp::keyHandler()
{
	unsigned int ball = GameEngine::entityWorldID("Ball");
	float fps = GameEngine::FPS();

	if( m_keys[GLFW_KEY_F7] )
	{
		h3dSetOption( H3DOptions::DebugViewMode, h3dGetOption(H3DOptions::DebugViewMode) != 0 ? 0 : 1.0f );
		m_keys[GLFW_KEY_F7] = 0; // reset key state so we only change it once
	}
	if( m_keys[GLFW_KEY_F6] )
	{
		GameEngine::setPhysicsShapeRenderingEnabled(!GameEngine::physicsShapeRenderingEnabled());
		m_keys[GLFW_KEY_F6] = 0; // reset key state so we only change it once
	}
	if( m_keys['R'])
	{
		GameEngine::resetPhysics();
		m_keys['R'] = 0; // reset key state so we only change it once
	}

	if( m_keys['W'] )
		GameEngine::translateEntityLocal(m_camID, 0, 0, -10 / fps);
	if( m_keys['S'] )
		GameEngine::translateEntityLocal(m_camID, 0, 0, 10 / fps);
	if( m_keys['A'] )
		GameEngine::translateEntityLocal(m_camID, -10 / fps, 0, 0);
	if( m_keys['D'] )
		GameEngine::translateEntityLocal(m_camID, 10 / fps, 0, 0);

	if( m_keys['I'] )
		GameEngine::translateEntityGlobal( ball, 0, 0, -10 / fps );
	if( m_keys['K'] )
		GameEngine::translateEntityGlobal( ball, 0, 0, 10 / fps );
	if( m_keys['J'] )
		GameEngine::translateEntityGlobal( ball, -10 / fps, 0, 0 );
	if( m_keys['L'] )
		GameEngine::translateEntityGlobal( ball, 10 / fps, 0, 0 );
	if( m_keys['U'] )
		GameEngine::translateEntityGlobal( ball, 0, 10 / fps, 0 );
	if( m_keys['O'] )
		GameEngine::translateEntityGlobal( ball, 0, -10 / fps, 0 );

	m_running = m_keys[27] == 0;
}

void DemoApp::mouseMoved(float x, float y)
{
	if (m_mousedown == 1) {	// mouse button is down
		m_camRX += (m_lasty - y) * 0.0025f * GameEngine::FPS();
		m_camRY += (m_lastx - x) * 0.0025f * GameEngine::FPS();
		GameEngine::setEntityRotation(m_camID, m_camRX, m_camRY, 0);
	}

	m_lastx = x;
	m_lasty = y;
}

void DemoApp::render()
{
	keyHandler();
	GameEngine::update();
	h3dutDumpMessages();
}

void DemoApp::resize(int width, int height) {
	H3DNode camID = GameEngine::entitySceneGraphID( m_camID );
	h3dSetNodeParamI( camID, H3DCamera::ViewportXI, 0 );
	h3dSetNodeParamI( camID, H3DCamera::ViewportYI, 0 );
	h3dSetNodeParamI( camID, H3DCamera::ViewportWidthI, width );
	h3dSetNodeParamI( camID, H3DCamera::ViewportHeightI, height );
	h3dResizePipelineBuffers( h3dGetNodeParamI( camID, H3DCamera::PipeResI ), width, height );
}

void DemoApp::keyPress(unsigned int param, bool pressed)
{
	m_keys[param] = pressed ? 1 : 0;
	if (!pressed)
			printf("%.3f\n",GameEngine::FPS());
}

void DemoApp::mouseClick(int button, int action) {
	m_mousedown = action;
}

void DemoApp::release() {
	GameEngine::release();
}
