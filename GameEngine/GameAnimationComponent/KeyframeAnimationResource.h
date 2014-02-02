//
// GameEngine Animations Component
// ---------------------------------------------------------
// Copyright (C) 2007-2013 Volker Wiendl, Felix Kistler
// 
// ****************************************************************************************
#pragma once
#ifndef ANIMATIONRESOURCE_H_
#define ANIMATIONRESOURCE_H_

class GameEntity;

class KeyframeAnimationResource
{
public:
	KeyframeAnimationResource(GameEntity* entity, int animResourceID, int frames, float speed);

	~KeyframeAnimationResource();

	void activate(bool enable, int stage, int layer, bool additive);

	void update(float frame, int stage, float weight);

	GameEntity* const			Entity;
	int							Horde3DId;
	const int					AnimResourceID;
	const float					Speed;
	const float					Frames;
};
#endif
