/*
 * Scene2.cpp
 *
 *  Created on: Dec 7, 2013
 *      Author: yama
 */

#include "Scene2.h"
//#include "Horde3D/Horde3D.h"
//#include "Horde3DUtils.h"
#include <math.h>
#include <iomanip>
#include "cocos2d.h"
#include "PointLight.h"
#include <vector>
#include "GameEngine/utMath.h"
#include "util.h"

#include "GameEngine/GameEngine.h"
#include "GameEngine/GameModules.h"
#include "GameEngine/GameWorld.h"
#include "GameEngine/GameEngine_SceneGraph.h"

#include "Horde3D/Horde3D.h"
#include "Horde3D/Horde3DUtils.h"

#include "GameEngine/GameEngine_Animations.h"

using namespace std;
using namespace cocos2d;
using namespace Horde3D;

Scene2::Scene2() {
	for (unsigned int i = 0; i < 320; ++i) {
		_keys[i] = false;
		_prevKeys[i] = false;
	}

	_x = 15;
	_y = 3;
	_z = 20;
	_rx = -10;
	_ry = 60;
	_velocity = 10.0f;
	_curFPS = 30;

	_statMode = 0;
	_freezeMode = 0;
	_debugViewMode = false;
	_wireframeMode = false;
	_cam = 0;
	i=0;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	_contentDir = "Content";
#else
	_contentDir = "../Content";
#endif

	// Initialize engine
//		if (!h3dInit(CCDirector::sharedDirector()->getOpenGLView()->getDevice())) {
//
//			h3dutDumpMessages();
//			exit(-1);
//		}

	if (!GameEngine::init())
		exit(-1);

	scgPlugin = new GameEngine::SceneGrapgPlugin();
	GameEngine::installPlugin(scgPlugin);


	bpPlugin = new GameEngine::BulletPhysicsPlugin();
	GameEngine::installPlugin(bpPlugin);

	aPlugin = new GameEngine::AnimationPlugin();
	GameEngine::installPlugin(aPlugin);

	ikPlugin = new GameEngine::IKPlugin();
	GameEngine::installPlugin(ikPlugin);

	if (!GameEngine::loadScene("D:\\repo\\git\\Horde\\Samples\\DemoContent\\Demo.scn")) {//DominoDemo\\DominoSzene\\DominoSzene.scn
		printf("NO SCENE TO LOAD!\n");
		exit(-1);
	}

	m_camID = GameEngine::entityWorldID("camera");
	printf("m_camID: %d\n", m_camID);
				m_camRX =0;
				m_camRY =0;

	// Load resources
//#ifdef HORDE3D_D3D11
//	const char* platformDirectory = "d3d11";
//#elif HORDE3D_GL
//	const char* platformDirectory = "gl";
//#elif defined(HORDE3D_GLES2)
//	const char* platformDirectory = "gles2";
//#endif
//	CCDirector::sharedDirector()->loadResourcesFromDisk(_contentDir.c_str(),
//			platformDirectory);

	resize(CCDirector::sharedDirector()->getWinSize().width,
			CCDirector::sharedDirector()->getWinSize().height);


	printf("HANS ID: %d", GameEngine::entityWorldID("Hans"));
	printf("camera ID: %d", GameEngine::entityWorldID("camera"));
	printf("Marie ID: %d", GameEngine::entityWorldID("Marie"));

	unsigned int HansH3Id = GameEngine::entitySceneGraphID(GameEngine::entityWorldID("Hans"));
	i = 0;
	int nodes = h3dFindNodes( HansH3Id, "Bip01_R_UpperArm", H3DNodeTypes::Undefined );
	if(nodes>1) printf("More than 1 Hans");
	else{
		i = h3dGetNodeFindResult(i);
	}
}

Scene2::~Scene2() {
	// TODO Auto-generated destructor stub
}

void Scene2::update(float fDelta) {


//	GameEngine::setEntityRotation(6,((float)i)/10.0, ((float)i)/10.0,0.0);
//	i++;

	keyStateHandler();

	GameEngine::update();

	h3dutDumpMessages();
}

bool Scene2::onEvent(const cocos2d::CCEvent& e) {
	switch (e.Type) {
	case CCEvent::Quit:
		release();
		return true;
	case CCEvent::TouchBegan:
		_pointerDown = true;
		_pointerX = e.Touch.X;
		_pointerY = e.Touch.Y;
		break;
	case CCEvent::TouchEnded:
	case CCEvent::TouchCancelled:
		_pointerDown = false;
		break;
	case CCEvent::TouchMoved:
		if (_pointerDown) {
			mouseMoveEvent(e.Touch.X - _pointerX, e.Touch.Y - _pointerY);
			_pointerX = e.Touch.X;
			_pointerY = e.Touch.Y;
		}
		break;
	case CCEvent::KeyPressed:
		_keys[e.Key.Code] = true;
		break;
	case CCEvent::KeyReleased:
		_keys[e.Key.Code] = false;
		break;
	}

	return false;
}

void Scene2::release() {
	// Release engine
	GameEngine::release();
}

void Scene2::resize(int width, int height) {

	H3DNode camID = GameEngine::entitySceneGraphID(
			GameEngine::entityWorldID("camera"));
	h3dSetNodeParamI(camID, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(camID, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(camID, H3DCamera::ViewportWidthI, width);
	h3dSetNodeParamI(camID, H3DCamera::ViewportHeightI, height);
	h3dResizePipelineBuffers(h3dGetNodeParamI(camID, H3DCamera::PipeResI),
			width, height);
}

void Scene2::keyStateHandler() {
	// ----------------
	// Key-press events
	// ----------------
	if (_keys[Key::Space] && !_prevKeys[Key::Space])  // Space
			{
		if (++_freezeMode == 3)
			_freezeMode = 0;
	}

	if (_keys[Key::F3] && !_prevKeys[Key::F3])  // F3
			{
		if (h3dGetNodeParamI(_cam, H3DCamera::PipeResI) == _forwardPipeRes)
			h3dSetNodeParamI(_cam, H3DCamera::PipeResI, _deferredPipeRes);
		else
			h3dSetNodeParamI(_cam, H3DCamera::PipeResI, _forwardPipeRes);
	}

	if (_keys[Key::F7] && !_prevKeys[Key::F7])  // F7
		_debugViewMode = !_debugViewMode;

	if (_keys[Key::F8] && !_prevKeys[Key::F8])  // F8
		_wireframeMode = !_wireframeMode;

	if (_keys[Key::F6] && !_prevKeys[Key::F6])  // F6
			{
		_statMode += 1;
		if (_statMode > H3DUTMaxStatMode)
			_statMode = 0;
	}

	// --------------
	// Key-down state
	// --------------
	if (_freezeMode != 2) {
		float curVel = _velocity / _curFPS;

		if (_keys[Key::LShift])
			curVel *= 5;	// LShift

		if (_keys[Key::W]) {
			GameEngine::playAnim( GameEngine::entityWorldID("Marie"), "walk" ); // start to play Marie's idle animation
			// Move forward
			_x -= sinf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			_y -= sinf(-degToRad(_rx)) * curVel;
			_z -= cosf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
		}
		if (_keys[Key::S]) {
			GameEngine::playAnim( GameEngine::entityWorldID("Hans"), "walk" );
			// Move backward
			_x += sinf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			_y += sinf(-degToRad(_rx)) * curVel;
			_z += cosf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
		}
		if (_keys[Key::A]) {
			// Strafe left
			_x += sinf(degToRad(_ry - 90)) * curVel;
			_z += cosf(degToRad(_ry - 90)) * curVel;
		}
		if (_keys[Key::D]) {
			// Strafe right
			_x += sinf(degToRad(_ry + 90)) * curVel;
			_z += cosf(degToRad(_ry + 90)) * curVel;
		}
	}
}

void Scene2::mouseMoveEvent(float dX, float dY) {


	// Look left/right
		_ry -= dX / 100.0f * 30.0f;

		// Loop up/down but only in a limited range
		_rx += dY / 100.0f * 30.0f;
		if (_rx > 90)
			_rx = 90;
		if (_rx < -90)
			_rx = -90;

			m_camRX += (m_lasty - dY) * 0.0025f * GameEngine::FPS();
			m_camRY += (m_lastx - dX) * 0.0025f * GameEngine::FPS();

//			const float* TransMat;
//			h3dGetNodeTransMats(i, 0, &TransMat);

			int forearmid = GameEngine::entityWorldID("Hans");//Bip01_L_Forearm
			printf("forearmid = %d\n", forearmid);

			GameEngine::setEntityRotation(forearmid, _rx, _ry, 0);

		m_lastx = dX;
		m_lasty = dY;

		return;
	// ----------------
	// Key-press events
	// ----------------
	if (_keys[Key::Space] && !_prevKeys[Key::Space])  // Space
			{
		if (++_freezeMode == 3)
			_freezeMode = 0;
	}

	if (_keys[Key::F3] && !_prevKeys[Key::F3])  // F3
			{
		if (h3dGetNodeParamI(_cam, H3DCamera::PipeResI) == _forwardPipeRes)
			h3dSetNodeParamI(_cam, H3DCamera::PipeResI, _deferredPipeRes);
		else
			h3dSetNodeParamI(_cam, H3DCamera::PipeResI, _forwardPipeRes);
	}

	if (_keys[Key::F7] && !_prevKeys[Key::F7])  // F7
		_debugViewMode = !_debugViewMode;

	if (_keys[Key::F8] && !_prevKeys[Key::F8])  // F8
		_wireframeMode = !_wireframeMode;

	if (_keys[Key::F6] && !_prevKeys[Key::F6])  // F6
			{
		_statMode += 1;
		if (_statMode > H3DUTMaxStatMode)
			_statMode = 0;
	}

	// --------------
	// Key-down state
	// --------------
	if (_freezeMode != 2) {
		float curVel = _velocity / _curFPS;

		if (_keys[Key::LShift])
			curVel *= 5;	// LShift

		if (_keys[Key::W]) {
			// Move forward
			_x -= sinf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			_y -= sinf(-degToRad(_rx)) * curVel;
			_z -= cosf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
		}
		if (_keys[Key::S]) {
			// Move backward
			_x += sinf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			_y += sinf(-degToRad(_rx)) * curVel;
			_z += cosf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
		}
		if (_keys[Key::A]) {
			// Strafe left
			_x += sinf(degToRad(_ry - 90)) * curVel;
			_z += cosf(degToRad(_ry - 90)) * curVel;
		}
		if (_keys[Key::D]) {
			// Strafe right
			_x += sinf(degToRad(_ry + 90)) * curVel;
			_z += cosf(degToRad(_ry + 90)) * curVel;
		}
	}
}
