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
#pragma once
#ifndef ANIMATIONJOB_H_
#define ANIMATIONJOB_H_

#include "KeyframeAnimationResource.h"
#include "KeyframeAnimComponent.h"

/**
* This class is used to handle a play animation command.
* It can be updated to change the way the animation is played
*/
class KeyframeAnimationJob
{
public:

	struct TimelineWeight
	{
		TimelineWeight(float timestamp, float weight) : Timestamp(timestamp), Weight(weight) {}

		float Timestamp, Weight;
	};

	/**
	* Creates a new Animation job, based on the given animation and command settings.
	* @param owner the owner stores a registry where all animation jobs are represented to allow updates
	* @param anim the animation resource that will be played
	* @param command the parameters how to play the animation
	*/
	KeyframeAnimationJob(KeyframeAnimComponent* owner, KeyframeAnimationResource* anim, AnimationSetup* command);

	~KeyframeAnimationJob();

	bool past(const float timestamp);

	void update(const float timestamp);

	void addTimelinePoint(const float timestamp, const float weight);

	void changeSpeed(const float speed);

	void changeDuration(float duration);

	void pause();

	void unPause();

	void setStartFrame(const float startFrame);

	void setAnimBias(float biasTransX, float biasTransY, float biasTransZ, float biasRotX, float biasRotY, float biasRotZ, float biasRotW);

	std::vector<TimelineWeight>	m_timeline;
	int							m_jobID;
	bool						m_endless;
	KeyframeAnimationResource*	m_animation;
	const char*					m_animationName;
	const int					m_stageID;
	const int					m_layerID;
	float						m_speed;
	int							m_currentTimeWeight;
	KeyframeAnimComponent*		m_owner;
	static const float			ENDLESS_DURATION;
	bool						m_paused;
	float						m_pauseTimestamp;
	float						m_startFrame;

	float 						m_biasTransX;
	float 						m_biasTransY;
	float 						m_biasTransZ;
	float 						m_biasRotX;
	float 						m_biasRotY;
	float 						m_biasRotZ;
	float 						m_biasRotW;
};
#endif

