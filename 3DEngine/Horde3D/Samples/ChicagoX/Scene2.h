/*
 * Scene2.h
 *
 *  Created on: Dec 7, 2013
 *      Author: yama
 */

#ifndef SCENE2_H_
#define SCENE2_H_

#include "crowd.h"
#include <string>
#include <vector>
#include "CCEvent.h"
#include "nodes/CCNode.h"
#include "PointLight.h"
#include "GameEngine/utMath.h"
#include <stdint.h>
#include "Spotlight.h"
#include "ConvexHull.h"

#include <GameEngine/SceneGrapgPlugin.h>
#include <GameEngine/BulletPhysicsPlugin.h>
#include <GameEngine/AnimationPlugin.h>
#include <GameEngine/IKPlugin.h>


using namespace Horde3D;

class Scene2: public cocos2d::CCNode {
public:
	Scene2();
	virtual ~Scene2();

	void setKeyState(int key, bool state) {
		_prevKeys[key] = _keys[key];
		_keys[key] = state;
	}

	const char *getTitle() {
		return "Chicago - Horde3D Sample";
	}

	void update(float fDelta);
	bool onEvent(const cocos2d::CCEvent& e);

	void release();
	void resize(int width, int height);

	void keyStateHandler();
	void mouseMoveEvent(float dX, float dY);

private:
	void keyHandler();

private:
	bool _keys[cocos2d::Key::Count], _prevKeys[cocos2d::Key::Count];
	bool _pointerDown;
	float _pointerX, _pointerY;

	float _x, _y, _z, _rx, _ry;  // Viewer position and orientation
	float _velocity;  // Velocity for movement
	float _curFPS;

	int _statMode;
	int _freezeMode;
	bool _debugViewMode, _wireframeMode;


	float m_camRX, m_camRY;
		int	  m_camID;
		int  m_mousedown;
		float m_lastx, m_lasty;

	// Engine objects
	H3DRes _fontMatRes, _panelMatRes;
	H3DRes _logoMatRes, _forwardPipeRes, _deferredPipeRes;
	H3DNode _cam;
	H3DRes texRes;
	H3DRes lightColorTex; //RGB8 Texture with light color
	H3DRes lightRrTex; //RGBA16F Texture with x,z,y,r values
	H3DRes tileTex; //RGBA16F Texture with x,z,y,r values

	std::string _contentDir;

	GameEngine::SceneGrapgPlugin* scgPlugin;
	GameEngine::BulletPhysicsPlugin* bpPlugin;
	GameEngine::AnimationPlugin* aPlugin;
	GameEngine::IKPlugin* ikPlugin;

	int i;
};

#endif /* SCENE2_H_ */
