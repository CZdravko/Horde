// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007-2013 Volker Wiendl
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
// GameEngine Animations Component
// ---------------------------------------------------------
// Copyright (C) 2007-2013 Volker Wiendl, Felix Kistler
// 
// ****************************************************************************************
#include "KeyframeAnimationResource.h"

#include "config.h"

#include <GameEngine/config.h>
#include <GameEngine/GameEntity.h>
#include <GameEngine/GameEvent.h>
#include <GameEngine/GameEngine.h>

#include <Horde3D/Horde3D.h>

KeyframeAnimationResource::KeyframeAnimationResource(GameEntity* entity, int animResourceID, int frames, float speed) :
		Entity(entity), AnimResourceID(animResourceID), Speed(speed), Frames((float) frames), Horde3DId(0) {
	GameEventData data(&Horde3DId);
	GameEvent ev(GameEvent::E_GET_SCENEGRAPH_ID, &data, 0x0);
	GameEngine::sendEvent(Entity->worldId(), &ev);
}

KeyframeAnimationResource::~KeyframeAnimationResource() {
}

void KeyframeAnimationResource::activate(bool enable, int stage, int layer, bool additive) {
	const char* mask = "";
	h3dSetupModelAnimStage(Horde3DId, stage, enable ? AnimResourceID : 0, layer, mask, additive);
}

void KeyframeAnimationResource::update(float frame, int stage, float weight) {
	SetAnimFrame setAnimFrame(stage, frame, weight);
	// Set current frame and weight
	GameEvent event(GameEvent::E_SET_ANIM_FRAME, &setAnimFrame, 0);
	Entity->executeEvent(&event);
}

void KeyframeAnimationResource::setBias(int stage, float biasTransX, float biasTransY, float biasTransZ, float biasRotX, float biasRotY, float biasRotZ, float biasRotW) {
	BiasTransX = biasTransX;
	BiasTransY = biasTransY;
	BiasTransZ = biasTransZ;
	BiasRotX = biasRotX;
	BiasRotY = biasRotY;
	BiasRotZ = biasRotZ;
	BiasRotW = biasRotW;

	h3dSetModelAnimBias(Horde3DId, stage, biasTransX, biasTransY, biasTransZ, biasRotX, biasRotY, biasRotZ, biasRotW);
}
