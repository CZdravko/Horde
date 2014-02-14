// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#include "Scene.h"
#include "Horde3D/Horde3D.h"
#include "Horde3D/Horde3DUtils.h"
#include <math.h>
#include <iomanip>
#include "cocos2d.h"
#include "PointLight.h"
#include <vector>
#include "GameEngine/utMath.h"
#include "util.h"

#include "Horde3D/Horde3DASFAMC.h"

using namespace std;
using namespace cocos2d;
using namespace Horde3D;

// Convert from degrees to radians
//inline float degToRad(float f) {
//	return f * (3.1415926f / 180.0f);
//}

float Scene::quantizedZ[17] = { 0.1, 5.0, 6.8, 9.2, 12.6, 17.1, 23.2, 31.5, 42.9, 58.3, 79.2, 108, 146, 199, 271, 368, 500 };

Scene::Scene() {
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

	color_index = 0;
	tTexH = 256;
	tTexW = 256;

	m_fFOVy = 45;
	m_fZNear = 5;

//	numZDivisions = 16;
//	numYDivisions = 17;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	_contentDir = "Content";
#else
	_contentDir = "../Content";
#endif

	// Initialize engine
	if (!h3dInit(CCDirector::sharedDirector()->getOpenGLView()->getDevice())) {

		h3dutDumpMessages();
		exit(-1);
	}

	// Set options
	h3dSetOption(H3DOptions::LoadTextures, 1);
	h3dSetOption(H3DOptions::MaxAnisotropy, 4);
	h3dSetOption(H3DOptions::ShadowMapSize, 2048);
	h3dSetOption(H3DOptions::FastAnimation, 1);

	// Add resources
	// Pipelines
	_forwardPipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0);
	_deferredPipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/deferred.pipeline.xml", 0);
	// Overlays
	_fontMatRes = h3dAddResource(H3DResTypes::Material, "overlays/font.material.xml", 0);
	_panelMatRes = h3dAddResource(H3DResTypes::Material, "overlays/panel.material.xml", 0);
	_logoMatRes = h3dAddResource(H3DResTypes::Material, "overlays/logo.material.xml", 0);
	// Shader for deferred shading
	H3DRes lightMatRes = h3dAddResource(H3DResTypes::Material, "materials/light.material.xml", 0);
	// Environment
	H3DRes envRes = h3dAddResource(H3DResTypes::SceneGraph, "models/platform/platform.scene.xml", 0);
	// Skybox
	H3DRes skyBoxRes = h3dAddResource(H3DResTypes::SceneGraph, "models/skybox/skybox.scene.xml", 0);

	//H3DRes ASFRes = h3dAddResource(H3DEXT_ASF_resType, "animations/test.anim.xml", 0);

	//Adding 2 lights:
	PointLight p1 = PointLight(Vec3f(0, 0, -10), 2, 1);
	//PointLight p2 = PointLight(Vec3f(-1, 2, -3), 2, 2);

	Spotlight s1 = Spotlight();
	s1.V = Vec3f(0.0, 0.0, 2.5);
	s1.dir = Vec3f(1.0, 1.0, 1.0).normalized();
	s1.r = 3.0;
	s1.height = 9.0;
	s1.lightIndex = 2;
	m_vSpotLights.push_back(s1);

	m_vPointLights.push_back(p1);
	//m_vPointLights.push_back(p2);

	//light Color texture
	lightColorTex = h3dCreateTexture("lightColorTex", 256, 4, H3DFormats::TEX_RGBA8, H3DResFlags::NoTexMipmaps);
	uint8_t* pLightColor = static_cast<uint8_t*>(h3dMapResStream(lightColorTex, H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	//L1

	pLightColor = pack_float(0.9f, pLightColor);
	pLightColor = pack_float(0.7f, pLightColor);
	pLightColor = pack_float(0.7f, pLightColor);
	pLightColor = pack_float(0.7f, pLightColor);

	h3dUnmapResStream(lightColorTex);
//	//L2
//	pLightColor = pack_float(0.9f, pLightColor);
//	pLightColor = pack_float(0.7f, pLightColor);
//	pLightColor = pack_float(0.7f, pLightColor);
//	pLightColor = pack_float(0.7f, pLightColor);

	//light Rr texture
	lightRrTex = h3dCreateTexture("lightRrTex", 256, 4, H3DFormats::TEX_RGBA8, H3DResFlags::NoTexMipmaps);
	uint8_t* pLightRr = static_cast<uint8_t*>(h3dMapResStream(lightRrTex, H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	//L1
	pLightRr = pack_float(p1.position.x, pLightRr);
	pLightRr = pack_float(p1.position.y, pLightRr);
	pLightRr = pack_float(p1.position.z, pLightRr);
	pLightRr = pack_float(p1.r, pLightRr);
	//L2
	/*pLightRr = pack_float(p2.position.x, pLightRr);
	 pLightRr = pack_float(p2.position.y, pLightRr);
	 pLightRr = pack_float(p2.position.z, pLightRr);
	 pLightRr = pack_float(p2.r, pLightRr);*/

	h3dUnmapResStream(lightRrTex);

	//Tile texture

	tileTex = h3dCreateTexture("tileTex", tTexW, tTexH, H3DFormats::TEX_RGBA8, H3DResFlags::NoTexMipmaps);
	uint8_t* pTileTex = static_cast<uint8_t*>(h3dMapResStream(tileTex, H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	for (int Z = 0; Z < 16; Z++) {
		for (int Y = 0; Y < 16; Y++) {
			for (int X = 0; X < 16; X++) {
				pTileTex[(256 * Z + 16 * Y + X) * 64] = 0;
			}
		}
	}

	h3dUnmapResStream(tileTex);

	// Load resources
#ifdef HORDE3D_D3D11
	const char* platformDirectory = "d3d11";
#elif HORDE3D_GL
	const char* platformDirectory = "gl";
#elif defined(HORDE3D_GLES2)
	const char* platformDirectory = "gles2";
#endif
	CCDirector::sharedDirector()->loadResourcesFromDisk(_contentDir.c_str(), platformDirectory);

	// Add scene nodes
	// Add camera
	_cam = h3dAddCameraNode(H3DRootNode, "Camera", _forwardPipeRes);
	//h3dSetNodeParamI( _cam, H3DCamera::OccCullingI, 1 );
	// Add environment
	H3DNode env = h3dAddNodes(H3DRootNode, envRes);
	h3dSetNodeTransform(env, 0, 0, 0, 0, 0, 0, 0.23f, 0.23f, 0.23f);
	// Add skybox
	H3DNode sky = h3dAddNodes(H3DRootNode, skyBoxRes);
	h3dSetNodeTransform(sky, 0, 0, 0, 0, 0, 0, 210, 50, 210);
	h3dSetNodeFlags(sky, H3DNodeFlags::NoCastShadow, true);
	// Add light source
	H3DNode light = h3dAddLightNode(H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP");
	h3dSetNodeTransform(light, 0, 20, 50, -30, 0, 0, 1, 1, 1);
	h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 200);
	h3dSetNodeParamF(light, H3DLight::FovF, 0, 90);
	h3dSetNodeParamI(light, H3DLight::ShadowMapCountI, 3);
	h3dSetNodeParamF(light, H3DLight::ShadowSplitLambdaF, 0, 0.9f);
	h3dSetNodeParamF(light, H3DLight::ShadowMapBiasF, 0, 0.001f);
	h3dSetNodeParamF(light, H3DLight::ColorF3, 0, 0.9f);
	h3dSetNodeParamF(light, H3DLight::ColorF3, 1, 0.7f);
	h3dSetNodeParamF(light, H3DLight::ColorF3, 2, 0.75f);

	_crowdSim = new CrowdSim(_contentDir);
	_crowdSim->init();

	resize(CCDirector::sharedDirector()->getWinSize().width, CCDirector::sharedDirector()->getWinSize().height);
}

void Scene::update(float fDelta) {
	keyStateHandler();

	color_index++;
//	if (color_index % 256 == 0) {
//		updateLightTexture();
//	}

	for (unsigned int i = 0; i < Key::Count; ++i)
		_prevKeys[i] = _keys[i];

	_curFPS = 1.0f / fDelta;

	h3dSetOption(H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f);
	h3dSetOption(H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f);

	if (!_freezeMode) {
		_crowdSim->update(_curFPS);
	}

	// Set camera parameters
	h3dSetNodeTransform(_cam, _x, _y, _z, _rx, _ry, 0, 1, 1, 1);

	// Show stats
	h3dutShowFrameStats(_fontMatRes, _panelMatRes, _statMode);
	if (_statMode > 0) {
		if (h3dGetNodeParamI(_cam, H3DCamera::PipeResI) == _forwardPipeRes)
			h3dutShowText("Pipeline: forward", 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes);
		else
			h3dutShowText("Pipeline: deferred", 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes);
	}

	// Show logo
	const float ww = (float) h3dGetNodeParamI(_cam, H3DCamera::ViewportWidthI) / (float) h3dGetNodeParamI(_cam, H3DCamera::ViewportHeightI);
	const float ovLogo[] = { ww - 0.4f, 0.8f, 0, 1, ww - 0.4f, 1, 0, 0, ww, 1, 1, 0, ww, 0.8f, 1, 1 };
	h3dShowOverlays(ovLogo, 4, 1.f, 1.f, 1.f, 1.f, _logoMatRes, 0);

	// Render scene
	h3dRender(_cam);

	// Finish rendering of frame
	h3dFinalizeFrame();

	// Remove all overlays
	h3dClearOverlays();

	// Write all messages to log file
	h3dutDumpMessages();
}

bool Scene::onEvent(const CCEvent& e) {
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

void Scene::release() {
	delete _crowdSim;
	_crowdSim = 0x0;

	// Release engine
	h3dRelease();
}

void Scene::resize(int width, int height) {
	//init private variables
	m_iWinHeight = height;
	m_iWinWidth = width;

	// Resize viewport
	h3dSetNodeParamI(_cam, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportWidthI, width);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportHeightI, height);

	// Set virtual camera parameters
	h3dSetupCameraView(_cam, 45.0f, (float) width / height, 0.1f, 1000.0f);
	h3dResizePipelineBuffers(_deferredPipeRes, width, height);
	h3dResizePipelineBuffers(_forwardPipeRes, width, height);

	fRightEdge = (float) (m_fZNear * tan(m_fFOVy * M_PI / 360.0));
	float fAspectRatio = ((float) m_iWinHeight) / ((float) m_iWinWidth);
	fTopEdge = fRightEdge * fAspectRatio;

	xStep = 2 * fRightEdge / 16;
	yStep = 2 * fTopEdge / 16;
	quantizedY[0] = -100;
	quantizedX[0] = -100;
	for (int i = 1; i < 17; i++) {
		quantizedY[i] = -fTopEdge + i * yStep;
		quantizedX[i] = -fRightEdge + i * xStep;
	}
	quantizedY[17] = 100;
	quantizedX[17] = 100;
}

void Scene::keyStateHandler() {
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
			m_vPointLights[0].position.z += 0.5;
		}
		if (_keys[Key::S]) {
			// Move backward
			_x += sinf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			 _y += sinf(-degToRad(_rx)) * curVel;
			 _z += cosf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			m_vPointLights[0].position.z -= 0.5;
		}
		if (_keys[Key::A]) {
			// Strafe left
			_x += sinf(degToRad(_ry - 90)) * curVel;
			 _z += cosf(degToRad(_ry - 90)) * curVel;
			m_vPointLights[0].position.x -= 0.5;
		}
		if (_keys[Key::D]) {
			// Strafe right
			_x += sinf(degToRad(_ry + 90)) * curVel;
			 _z += cosf(degToRad(_ry + 90)) * curVel;
			m_vPointLights[0].position.x += 0.5;
		}
	}
}

void Scene::mouseMoveEvent(float dX, float dY) {
	if (_freezeMode == 2)
		return;

	// Look left/right
	_ry -= dX / 100.0f * 30.0f;

	// Loop up/down but only in a limited range
	_rx += dY / 100.0f * 30.0f;
	if (_rx > 90)
		_rx = 90;
	if (_rx < -90)
		_rx = -90;
}

void Scene::updateLightTexture() {
	uint8_t *pTileTex = static_cast<uint8_t*>(h3dMapResStream(tileTex, H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	//drop all counts to 0
	for (int i = 0; i < (16 * 16 * 16 * 16 * 4); i++)
		pTileTex[i] = 0;

	//get camera view matrix
	const float *camTrans;
	h3dGetNodeTransMats(_cam, 0x0, &camTrans);
	Matrix4f viewMat(Matrix4f(camTrans).inverted());

	//for each Point light
//	for (int i = 0; i < m_vPointLights.size(); i++) {
//		//transform position to view space
//
//		printf("Point Light %d position: %f, %f, %f\n", i,
//				m_vPointLights[i].position.x, m_vPointLights[i].position.y,
//				m_vPointLights[i].position.z);
//		Vec3f R = viewMat * m_vPointLights[i].position;
//
//		printf("Transformed Light %d position: %f, %f, %f\n", i, R.x, R.y, R.z);
//
//		//write to tile texture
//		tileIt2(m_vPointLights[i].position, m_vPointLights[i].r,
//				m_vPointLights[i].lightIndex, pTileTex);
//	}

	//for each Spot light
	for (int i = 0; i < m_vSpotLights.size(); i++) {
		//transform position to view space

		printf("Point Light %d position: %f, %f, %f\n", i, m_vSpotLights[i].V.x, m_vSpotLights[i].V.y, m_vSpotLights[i].V.z);
		Vec3f R = viewMat * m_vSpotLights[i].V;

		printf("Transformed Light %d position: %f, %f, %f\n", i, R.x, R.y, R.z);

		//write to tile texture
		tileIt(m_vSpotLights[i], pTileTex);
	}
	h3dUnmapResStream(tileTex);
}

void Scene::getQZRange(float ZMin, float ZMax, int& lower, int& upper) {
	int i = 0;
	for (; i < numZDivisions; i++)
		if (ZMin <= quantizedZ[i])
			break;
	lower = i; // - 1;
	for (; i < numZDivisions; i++)
		if (ZMax <= quantizedZ[i])
			break;
	upper = i - 1;
}

void Scene::getQYRange(float YMin, float YMax, int* lower, int* upper) {
	int i = 0;
	for (; i < numYDivisions; i++)
		if (YMin <= quantizedY[i])
			break;
	(*lower) = i; // - 1;
	for (; i < numYDivisions; i++)
		if (YMax <= quantizedY[i])
			break;
	(*upper) = i - 1;
}

//projects a float value at 'z' to a fixed Z plane
//l ... (in/out) scalar to be projected
//z ... (in) z of l
void Scene::project2Near(float& l, float Z) {
	l = l * m_fZNear / Z;
}

//projects a Vec3 to a fixed Z plane
//point ... (in/out) Vec3 to be projected
void Scene::project2Near(Vec3f& point) {
	point = point * (m_fZNear / point.z);
}

//returns a bounding circle
//Bottom ... (in/out) circle 1 center position
//rbottom ... (in/out) circle 1 radius
//Top ... (in) circle 2 center position
//rtop ... (in) circle 2 radius
void Scene::flattenSphere(Vec3f& Bottom, Vec3f& Top, float& rbottom, float& rtop) {
	Vec3f ndelta = (Top - Bottom).normalized();
	Vec3f T1 = Bottom + ndelta * rbottom;
	Vec3f T2 = Top - ndelta * rtop;
	Bottom = (T1 + T2) / 2;
	rbottom = (T2 - T1).length() / 2;
}

//returns the position and radius of a circle
//rc^2  = r^2 - dZ^2
//RC = (R.x, R.y, Z)
void Scene::cutSphere(float Z, Vec3f R, float r, Vec3f& RCut, float& rc) {
	RCut.x = R.x;
	RCut.y = R.y;
	RCut.z = Z;

	float dZ = Z - R.z;
	rc = sqrt(r * r - dZ * dZ);
}

void Scene::markYRowTilesSphere(float from, float to, int count, uint8_t Z, uint8_t Y, uint8_t lightIndex, uint8_t* texPointer) {
	int ixStart = (int) ((from + fRightEdge) / xStep);
	int ixStop = (int) ((to + fRightEdge) / xStep);

	ixStart = ixStart > 0 ? ixStart < 15 ? ixStart : 15 : 0;
	ixStop = ixStop > 0 ? ixStop < 15 ? ixStop : 15 : 0;
	/*printf("Marking tiles from %d to %d for Z=%d and Y=%d\n", ixStart, ixStop, Z,
	 Y);*/
	int ZYindex = 256 * Z + 16 * Y;
	for (int j = ixStart; j <= ixStop; j++) {
		int index = ZYindex + j;
		index *= 64;
		uint8_t count = texPointer[index];
		//printf("Calculated index = 256 * Z + 16 * Y + j = %d\n", index);
		if (texPointer[index + count + 4] != lightIndex) {
			texPointer[index + count + 4] = lightIndex; //TODO : check if this works
			texPointer[index]++; //increment count
		}
	}

}

//Marks tiles
//Input is an array of pairs of points that represent ranges of X
void Scene::markYRowTiles(float* crosssections, int count, int Z, int Y, uint8_t lightIndex, uint8_t* texPointer) {
	for (int i = 0; i < count; i++) {
		float xStart = crosssections[2 * i] < crosssections[2 * i + 1] ? crosssections[2 * i] : crosssections[2 * i + 1];
		float xStop = crosssections[2 * i] > crosssections[2 * i + 1] ? crosssections[2 * i] : crosssections[2 * i + 1];
		int ixStart = (int) ((xStart + fRightEdge) / xStep);
		int ixStop = (int) ((xStop + fRightEdge) / xStep);

		ixStart = ixStart > 0 ? ixStart < 15 ? ixStart : 15 : 0;
		ixStop = ixStop > 0 ? ixStop < 15 ? ixStop : 15 : 0;

		int ZYindex = 256 * Z + 16 * Y;
		for (int j = ixStart; j <= ixStop; j++) {
			int index = ZYindex + j;
			index *= 64;
			uint8_t count = texPointer[index];
			if (texPointer[index + count + 4] != lightIndex) {
				texPointer[index + count + 4] = lightIndex;
				texPointer[index]++; //increment count
			}
		}
	}

}

void Scene::markZRowTilesSphere(Vec3f* R, float r, uint8_t Z, uint8_t lightIndex, uint8_t* texPointer) {
	float rx = R->x;
	float ry = R->y;

	//TODO : fix it
	int iyMin, iyMax;
	if (ry - r + fTopEdge > 0)
		iyMin = (int) ((ry - r + fTopEdge) / yStep) + 1;
	else
		iyMin = 0;

	if (ry + r + fTopEdge > 0)
		iyMax = (int) ((ry + r + fTopEdge) / yStep);
	else
		iyMax = 0;

	/*printf("calculating iYmin: ((ry - r + fTopEdge) / yStep : %f / %f = %d",
	 ry - r + fTopEdge, yStep, iyMin);
	 */
	iyMin = iyMin > 0 ? iyMin < 15 ? iyMin : 15 : 0;
	iyMax = iyMax > 0 ? iyMax < 15 ? iyMax : 15 : 0;

	/*printf("iyMin, iyMax, fTopEdge,  yStep, ry: %d, %d, %f, %f, %f\n", iyMin,
	 iyMax, fTopEdge, yStep, ry);
	 */
	int center = 0;
	if (iyMin > iyMax) {
		markYRowTilesSphere(rx - r, rx + r, 0, Z, iyMax, lightIndex, texPointer);
		return;
	}

	int Y = iyMin;
	float y = -fTopEdge + Y * yStep;
	while (y < ry) {
		if (Y > 0) {
			float dy = ry - y;
			float rc;
			rc = sqrt(r * r - dy * dy);
			markYRowTilesSphere(rx - rc, rx + rc, 0, Z, Y - 1, lightIndex, texPointer);
		}
		Y++;
		if (Y > 16)
			return;
		y = -fTopEdge + Y * yStep;
	}
	if (Y > 0)
		markYRowTilesSphere(rx - r, rx + r, 0, Z, Y - 1, lightIndex, texPointer);

	while (Y < iyMax) {
		if (Y >= 0) {
			float y = -fTopEdge + Y * yStep;
			float dy = ry - y;
			float rc;
			rc = sqrt(r * r - dy * dy);
			markYRowTilesSphere(rx - rc, rx + rc, 0, Z, Y, lightIndex, texPointer);
		}
		Y++;
		if (Y > 15)
			return;
	}
}

//packs a float value to a char array
//float range between -500 and 500 0-256
uint8_t* Scene::pack_float(float val, uint8_t* chbuffer) {
	val = val / 4 + 125;
	uint8_t r, g, b, a;
	r = val;
	g = ((int) (val * 256)) & 0xff;
	b = ((int) (val * 65536)) & 0xff;
	a = ((int) (val * 16777216)) & 0xff;
	*chbuffer++ = r; // R
	*chbuffer++ = g; // G
	*chbuffer++ = b; // B
	*chbuffer++ = a; // A
	return chbuffer;
}

void Scene::tileIt2(Vec3f R, float r, uint8_t lightIndex, uint8_t* texPointer) {
	R.z = -R.z;

	float minZ = R.z - r;
	float maxZ = R.z + r;

	//get Quantized Z range
	int qZMax, qZMin;
	getQZRange(minZ, maxZ, qZMin, qZMax);

	if (qZMax < qZMin) {
		project2Near(r, R.z);
		project2Near(R);
		markZRowTilesSphere(&R, r, qZMax, lightIndex, texPointer);
		return;
	}

	Vec3f Rcut = Vec3f(R);
	float rcut = r;

	int z = qZMin;
	while (float cutZ = quantizedZ[z] < R.z) {
		if (z > 0) {
			float cutZ = quantizedZ[z];
			cutSphere(cutZ, R, r, Rcut, rcut);
			project2Near(rcut, Rcut.z);
			project2Near(Rcut);
			markZRowTilesSphere(&Rcut, rcut, z - 1, lightIndex, texPointer);
		}
		z++;
		if (z > 16)
			return;
	}
	if (z > 0) {
		project2Near(rcut, Rcut.z);
		project2Near(Rcut);
		markZRowTilesSphere(&Rcut, rcut, z - 1, lightIndex, texPointer);
	}
	while (z <= qZMax) {
		/*if (z>0){*/
		float cutZ = quantizedZ[z];
		cutSphere(cutZ, R, r, Rcut, rcut);
		project2Near(rcut, Rcut.z);
		project2Near(Rcut);
		markZRowTilesSphere(&Rcut, rcut, z, lightIndex, texPointer);
		//}
		z++;
		if (z > 15)
			return;
	}
}

void Scene::tileIt(Spotlight spot, uint8_t* texPointer) {
	Vec3f U, R, ln;
	Vec3f T1, T2, T3, T4, T, V;
	Vec3f points[128];
	ConvexHull ch;
	if (spot.dir.x < 0.0001 && spot.dir.y < 0.0001) { //parallel to Z
		U = Vec3f(0.0, 1.0, 0.0);
		R = Vec3f(1.0, 0.0, 0.0);
		ln = Vec3f(0.0, 1.0, 0.0);
	} else {
		U = Vec3f(0, 0, 1).cross(spot.dir).cross(spot.dir);
		R = (spot.dir.cross(U)).normalized();
		ln = Vec3f(spot.dir.x, spot.dir.y, 0).normalized();
	}
	V = spot.V;
	T = spot.V + spot.dir * spot.height;
	T1 = T + U * spot.r + R * spot.r;
	T2 = T + U * spot.r - R * spot.r;
	T3 = T - U * spot.r + R * spot.r;
	T4 = T - U * spot.r - R * spot.r;

	printf("%f\t %f\t %f \n", T1.x, T1.y, T1.z);
	printf("%f\t %f\t %f \n", T2.x, T2.y, T2.z);
	printf("%f\t %f\t %f \n", T3.x, T3.y, T3.z);
	printf("%f\t %f\t %f \n", T4.x, T4.y, T4.z);

	Vec3f* minZ;
	Vec3f* maxZ;

	minZ = minimZ(&V, &T1);
	maxZ = maximZ(&V, &T1);
	minZ = minimZ(minZ, &T3);
	maxZ = maximZ(maxZ, &T3);

	//get Quantized Z range
	int qZMax, qZMin;
	getQZRange(minZ->z, maxZ->z, qZMin, qZMax);

	printf("Zmin: %d \nZmax: %d \n", qZMin, qZMax);

	if (qZMax < qZMin) {
		points[0] = V;
		points[1] = V;
		points[2] = T1;
		points[3] = T2;
		points[4] = T3;
		points[5] = T4;
		flatten(points, 6, R, ln, ch);
		markZRowTiles(ch, qZMax, spot.lightIndex, texPointer);
		return;
	}

	Vec3f *middle;
	if (minZ == &V) {
		middle = (maxZ == &T1) ? &T3 : &T1;
	} else if (maxZ == &V) {
		middle = (minZ == &T1) ? &T3 : &T1;
	} else {
		middle = &V;
	}

	float t;
	int start = 0, stop = 0;
	//Making points invariant
	Vec3f U1, U2, D1, D2, M1, M2;
	if (middle == &V) {
		if (maxZ == &T1) {
			U1 = T1;
			U2 = T2;
			D1 = T3;
			D2 = T4;
		} else {
			U1 = T3;
			U2 = T4;
			D1 = T1;
			D2 = T2;
		}
		M1 = V;
		M2 = V;
	} else if (maxZ == &V) {
		if (middle == &T1) {
			M1 = T1;
			M2 = T2;
			D1 = T3;
			D2 = T4;
		} else {
			M1 = T3;
			M2 = T4;
			D1 = T1;
			D2 = T2;
		}
		U1 = V;
		U2 = V;
	} else {

		if (middle == &T1) {
			M1 = T1;
			M2 = T2;
			U1 = T3;
			U2 = T4;
		} else {
			M1 = T3;
			M2 = T4;
			U1 = T1;
			U2 = T2;
		}
		D1 = V;
		D2 = V;
	}

	Vec3f mPoints[4];
	mPoints[0] = M1;
	mPoints[1] = M2;
	IntersectSegmentZPlane(U1, D1, middle->z, t, mPoints[2]);
	IntersectSegmentZPlane(U2, D2, middle->z, t, mPoints[3]);

	Vec3f zPoints[numZDivisions][4];
	for (int i = qZMin; i <= qZMax; i++) {
		IntersectSegmentZPlane(U1, D1, quantizedZ[i], t, zPoints[i - qZMin][0]);
		IntersectSegmentZPlane(U2, D2, quantizedZ[i], t, zPoints[i - qZMin][1]);
		if (quantizedZ[i] < M1.z) {
			IntersectSegmentZPlane(M1, D1, quantizedZ[i], t, zPoints[i - qZMin][2]);
			IntersectSegmentZPlane(M2, D2, quantizedZ[i], t, zPoints[i - qZMin][3]);
		} else {
			IntersectSegmentZPlane(M1, U1, quantizedZ[i], t, zPoints[i - qZMin][2]);
			IntersectSegmentZPlane(M2, U2, quantizedZ[i], t, zPoints[i - qZMin][3]);
		}
	}

	//bottom interval
	points[stop++] = D1;
	points[stop++] = D2;
	if (middle->z <= quantizedZ[qZMin]) {

		//two sections below the first Z plane
		//this results in three shapes to be flattened
		points[stop++] = mPoints[0];
		points[stop++] = mPoints[1];
		points[stop++] = mPoints[2];
		points[stop++] = mPoints[3];
	}
	points[stop++] = zPoints[0][0];
	points[stop++] = zPoints[0][1];
	points[stop++] = zPoints[0][2];
	points[stop++] = zPoints[0][3];

	flatten(points, stop, R, ln, ch);
	markZRowTiles(ch, qZMin - 1, spot.lightIndex, texPointer);

	for (int i = 0; i < qZMax - qZMin; i++) {
		stop = 0;
		points[stop++] = zPoints[i][0];
		points[stop++] = zPoints[i][1];
		points[stop++] = zPoints[i][2];
		points[stop++] = zPoints[i][3];
		if (middle->z <= quantizedZ[qZMin + i + 1] && middle->z >= quantizedZ[qZMin + i]) {
			//two sections below the upper Z plane
			//this results in three shapes to be flattened
			points[stop++] = mPoints[0];
			points[stop++] = mPoints[1];
			points[stop++] = mPoints[2];
			points[stop++] = mPoints[3];
		}
		points[stop++] = zPoints[i + 1][0];
		points[stop++] = zPoints[i + 1][1];
		points[stop++] = zPoints[i + 1][2];
		points[stop++] = zPoints[i + 1][3];

		printf("BEFORE FLATTENING:\n");
		for (int var = 0; var < stop; ++var) {
			printf("%f \t %f \t %f \n", points[var].x, points[var].y, points[var].z);
		}

		flatten(points, stop, R, ln, ch);
		markZRowTiles(ch, qZMin + i, spot.lightIndex, texPointer);
	}

	//top interval
	stop = 0;
	int zpindex = qZMax - qZMin;
	points[stop++] = zPoints[zpindex][0];
	points[stop++] = zPoints[zpindex][1];
	points[stop++] = zPoints[zpindex][2];
	points[stop++] = zPoints[zpindex][3];
	if (middle->z >= quantizedZ[qZMax]) {
		//two sections above the first Z plane
		//this results in three shapes to be flattened
		points[stop++] = mPoints[0];
		points[stop++] = mPoints[1];
		points[stop++] = mPoints[2];
		points[stop++] = mPoints[3];
	}

	points[stop++] = U1;
	points[stop++] = U2;
	flatten(points, stop, R, ln, ch);
	markZRowTiles(ch, qZMax, spot.lightIndex, texPointer);
}

//return a convex hull connectivity from
//points .... array of points
//count .... size of array
//l .... line vector(T1-T2)
//ln .... line normal
//prallel lines on a Z=const plane are parallel under perspective projections -> similar
//Projecting points to a single Z plane
//If we always project to a fixed plane than koeficients can be fixed
void Scene::flatten(Vec3f *points, int count, Vec3f l, Vec3f ln, ConvexHull &ch) {
	//TODO : For now we are using a simplified method of returning a bounding rectangle
	//Maybe good enough, but it should return a convex hull
	//NOT GOOD

	//FIND max line length and max distance between lines
	// distance between lines by finding min and max signed distance between line point and first point projected to "ln"

	Vec3f T1, T2, T3, T4;
	//ConvexH c; - bullet versionn

	int idmin = 0, idmax = 0;
	float lmax = 0, len;
	float d[12];
	points[0].x = points[0].x * m_fZNear / points[0].z;
	points[0].y = points[0].y * m_fZNear / points[0].z;
	points[0].z = m_fZNear;
	for (int i = 0; i < count / 2; i++) {
		//Projecting points to m_fZNear
		points[2 * i].x = points[2 * i].x * m_fZNear / points[2 * i].z;
		points[2 * i].y = points[2 * i].y * m_fZNear / points[2 * i].z;
		points[2 * i].z = m_fZNear;
		points[2 * i + 1].x = points[2 * i + 1].x * m_fZNear / points[2 * i + 1].z;
		points[2 * i + 1].y = points[2 * i + 1].y * m_fZNear / points[2 * i + 1].z;
		points[2 * i + 1].z = m_fZNear;

		//gathering span
		d[i] = ln.dot(points[2 * i] - points[0]);

		len = Vec3f(points[2 * i] - points[2 * i + 1]).length();
		idmin = d[i] < d[idmin] ? i : idmin;
		idmax = d[i] > d[idmax] ? i : idmax;
		lmax = len > lmax ? len : lmax;
	}

	//we need to sort points starting from dmin going to dmax
	float yzk[12];
	float* indArr[12];
	for (int i = 0; i < count / 2; i++) {
		yzk[i] = d[i]; // / ch.points[i].z; //They are already projected
		indArr[i] = &yzk[i];
	}

	//Sorting by span
	qsort(indArr, count / 2, sizeof(float*), compre_floats);

	//Generating points and connectivity information for convex polygon
	int ch_pc = 0;
	int point_idxs[6];
	point_idxs[0] = 2 * (indArr[0] - yzk);

	for (int i = 0; i < count / 2 - 1;) {
		int temp = i + 1;
		int ith = indArr[i] - yzk;
		int ihtplus1 = indArr[i + 1] - yzk;
		Vec3f pi = points[2 * ith];
		Vec3f pj = points[2 * ihtplus1];
		float dy = (pj.y - pi.y);
		float dx = (pj.x - pi.x);
		float k = dx != 0 ? dy / dx : 100000.0;
		for (int j = i + 2; j < count / 2; ++j) {
			int jth = indArr[j] - yzk;
			pj = points[2 * jth];
			dy = (pj.y - pi.y);
			dx = (pj.x - pi.x);
			float kj = dx != 0 ? dy / dx : 100000.0;
			if (((kj < k) && kj < 0) || ((kj > k) && kj > 0)) {
				k = kj;
				temp = j;
			}
		}
		i = temp;
		ch_pc++;
		point_idxs[ch_pc] = 2 * (indArr[i] - yzk);
	}

	ch_pc++;
	point_idxs[ch_pc] = 2 * (indArr[count / 2] - yzk);

	ch.points[0] = points[point_idxs[0]];
	ch.points[1] = points[point_idxs[0] + 1];
	ch.connectivity[0] = 1;
	ch.connectivity[1] = 2;
	ch.connectivity[2] = 0;
	ch.connectivity[3] = 3;

	int var;
	for (var = 1; var < ch_pc - 1; var++) {
		ch.points[2 * var] = points[point_idxs[var]];
		ch.connectivity[2 * (2 * var)] = 2 * var - 2;
		ch.connectivity[2 * (2 * var) + 1] = 2 * var + 2;

		ch.points[2 * var + 1] = points[point_idxs[var] + 1];
		ch.connectivity[2 * (2 * var + 1)] = 2 * var + 1 - 2;
		ch.connectivity[2 * (2 * var + 1) + 1] = 2 * var + 1 + 2;
	}

	ch.points[2 * var] = points[point_idxs[var]];
	ch.points[2 * var + 1] = points[point_idxs[var] + 1];
	ch.connectivity[2 * (2 * var)] = 2 * var - 2;
	ch.connectivity[2 * (2 * var) + 1] = 2 * var + 1;
	ch.connectivity[2 * (2 * var + 1)] = 2 * var + 1 - 2;
	ch.connectivity[2 * (2 * var + 1) + 1] = 2 * var;

	ch.count = 2 * ch_pc;
}

void Scene::markZRowTiles(ConvexHull& ch, int Z, uint8_t lightIndex, uint8_t* texPointer) {
	float* indArr[12];
	float yzk[12];
	int sortdIndex[12];
	for (int i = 0; i < ch.count; i++) {
		yzk[i] = ch.points[i].y; // / ch.points[i].z; //They are already projected
		indArr[i] = &yzk[i];
		//printf("%f ",ch.points[i].y);
	}

	qsort(indArr, ch.count, sizeof(float*), compre_floats);

	for (int i = 0; i < ch.count; i++) {
		sortdIndex[i] = indArr[i] - yzk;
	}

//find Y extents
	int iqYmin, iqYmax;
	int minYindex = sortdIndex[0];
	int maxYindex = sortdIndex[ch.count - 1];
	getQYRange(yzk[minYindex], yzk[maxYindex], &iqYmin, &iqYmax);

//loop throgh points min + 1 to max - 1 and find cross sections
//so there are count-2 cross sections
	if (iqYmin <= iqYmax && iqYmin < 16) {
		float crossections[12];
		int T1, T2, T3, T4;
		T1 = sortdIndex[0];
		T2 = ch.connectivity[2 * T1];
		T3 = T1;
		T4 = ch.connectivity[2 * T1 + 1];
		int i = 1;
		int crosIndex = 1, iY = iqYmin;
		crossections[0] = ch.points[sortdIndex[0]].x;
		crossections[1] = ch.points[sortdIndex[0]].x;
		for (; i < ch.count - 1; i++) {
			int Ttemp = sortdIndex[i];

			//cross section is y[i] with T1-T2 and T3-T4
			float x1, x2, x3, x4, y1, y2, y3, y4;
			x1 = ch.points[T1].x;
			x2 = ch.points[T2].x;
			x3 = ch.points[T3].x;
			x4 = ch.points[T4].x;

			y1 = ch.points[T1].y;
			y2 = ch.points[T2].y;
			y3 = ch.points[T3].y;
			y4 = ch.points[T4].y;

//			//TODO : Check if dX = 0 -> divide by 0
			//if either dx or k == 0 -> divide by 0
			//if dX == 0 k=500000 if k==0 k=0.0001
			float a12 = x1 != x2 ? (y2 - y1) / (x2 - x1) : 500000;
			float a34 = x3 != x4 ? (y4 - y3) / (x4 - x3) : 500000;

			a12 = abs(a12) < 0.0001 ? 0.0001 : a12;
			a34 = abs(a34) < 0.0001 ? 0.0001 : a34;

			while (ch.points[Ttemp].y > quantizedY[iY]) {
				float y = quantizedY[iY];
				crossections[2 * crosIndex] = (y - y1) / a12 + x1; //(y1 - a12 * x1) / a; //(a - a12);
				crossections[2 * crosIndex + 1] = (y - y3) / a34 + x3; //(y3 - a34 * x3) / a; //(a - a34);
				if (iY > 0)
					markYRowTiles(crossections, crosIndex + 1, Z, iY - 1, lightIndex, texPointer);
				crossections[0] = crossections[2 * crosIndex];
				crossections[1] = crossections[2 * crosIndex + 1];
				crosIndex = 1;
				iY++;
			}

			float y = ch.points[Ttemp].y; // / ch.points[Ttemp].x;
			crossections[2 * crosIndex] = (y - y1) / a12 + x1; //(y1 - a12 * x1) / a; //(a - a12);
			crossections[2 * crosIndex + 1] = (y - y3) / a34 + x3; //(y3 - a34 * x3) / a; //(a - a34);
			crosIndex++;

			int C1temp, C2temp;
			C1temp = ch.connectivity[2 * Ttemp];
			C2temp = ch.connectivity[2 * Ttemp + 1];
			if (T2 == Ttemp) {
				T2 = T1 == C1temp ? C2temp : C1temp;
				T1 = Ttemp;
			} else if (T4 == Ttemp) {
				T4 = T3 == C1temp ? C2temp : C1temp;
				T3 = Ttemp;
			}
		}
//		markYRowTiles(crossections, crosIndex + 1, Z, iqYmax, lightIndex,
//				texPointer);
	}
}
