//
// GameEngine Animations Component
// ---------------------------------------------------------
// Copyright (C) 2007-2013 Volker Wiendl, Felix Kistler
// 
// ****************************************************************************************
#pragma once
#ifndef ANIMATIONRESOURCE_H_
#define ANIMATIONRESOURCE_H_

#include "GameEngine/utMath.h"
#include "KeyframeAnimationMeta.h"
#include <cstdio>
#include <cstring>
#include <string>

class GameEntity;

using namespace std;
using namespace Horde3D;



class KeyframeAnimationResource
{
public:
	KeyframeAnimationResource(GameEntity* entity, int animResourceID, int frames, float speed);

	~KeyframeAnimationResource();

	void activate(bool enable, int stage, int layer, bool additive);

	void update(float frame, int stage, float weight);

	void setBias(int stage, float biasTransX, float biasTransY, float biasTransZ, float biasRotX, float biasRotY, float biasRotZ, float biasRotW);

	GameEntity* const			Entity;
	int							Horde3DId;
	const int					AnimResourceID;
	const float					Speed;
	const float					Frames;

	float 						BiasTransX;
	float 						BiasTransY;
	float 						BiasTransZ;
	float 						BiasRotX;
	float 						BiasRotY;
	float 						BiasRotZ;
	float 						BiasRotW;

	KeyframeAnimationMeta* 		kfam;
};


#endif
