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
#include "KeyframeAnimationJob.h"

#include "config.h"
#include "KeyframeAnimationResource.h"

#include <algorithm>

#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEvent.h>


const float KeyframeAnimationJob::ENDLESS_DURATION = 31536000.0f;

KeyframeAnimationJob::KeyframeAnimationJob(KeyframeAnimComponent* owner, KeyframeAnimationResource* anim, AnimationSetup* command) : m_animation(anim), m_stageID(command->Stage), m_layerID(command->Layer), m_speed(command->Speed),
	m_currentTimeWeight(-1), m_jobID(0), m_owner(owner), m_paused(false), m_pauseTimestamp(0)
{
	m_animationName = new char[strlen(command->Animation) + 1];
	strcpy_s((char*)m_animationName, strlen(command->Animation) + 1, command->Animation);

	// If speed was set to zero by the user...
	if ( command->Speed == 0 )
	{
		// check if duration was also zero...
		if ( command->Duration == 0 )
		{
			// In this case use the default speed
			command->Speed = m_speed = m_animation->Speed;
			// and calculate the duration based on the default speed and available frames
			command->Duration = m_animation->Frames / m_speed;
			// results in playing the animation exactly one time with the default speed
		}
		// If duration was less than zero the user wants an endless animation with the default speed (command->Speed == 0)
		else if ( command->Duration < 0 )
			command->Speed = m_speed = m_animation->Speed;
		else // Speed is calculated from duration and available frames (play it one time for the specified duration)
			command->Speed = m_speed = m_animation->Frames / command->Duration;
	}
	else if ( command->Duration == 0 ) // Play it one time with specified speed
		command->Duration = m_animation->Frames / m_speed;
	m_endless = command->Duration < 0;
	// Create Timeline point for the beginning
	m_timeline.push_back(TimelineWeight(GameEngine::timeStamp() + command->TimeOffset, command->Weight));
	// Create Timeline point for the end (for endless animations the end timestamp is one year in the future)
	if (m_endless)
		m_timeline.push_back(TimelineWeight(GameEngine::timeStamp() + command->TimeOffset + ENDLESS_DURATION, command->Weight));
	else
		m_timeline.push_back(TimelineWeight(GameEngine::timeStamp() + command->TimeOffset + command->Duration, command->Weight));
	// Register Animation Job in owner's registry
	std::vector<KeyframeAnimationJob*>::iterator iter = std::find(m_owner->m_animationRegistry.begin(), m_owner->m_animationRegistry.end(), (KeyframeAnimationJob*) 0);
	if ( iter == owner->m_animationRegistry.end() )
	{
		m_owner->m_animationRegistry.push_back(this);
		m_jobID = (int) m_owner->m_animationRegistry.size();
	}
	else
	{
		*iter = this;
		m_jobID = (int) (iter - m_owner->m_animationRegistry.begin()) + 1;
	}
}

KeyframeAnimationJob::~KeyframeAnimationJob()
{
#ifdef PRINT_ANIMATION_INFO
	//printf("Removing animation job\n");
#endif
	// Disable animation
	if (m_currentTimeWeight > 0)
	{
		m_animation->activate(false, m_stageID, m_layerID, false);
	}
	// Deregister in owner's registry
	m_owner->m_animationRegistry[m_jobID - 1] = 0;

	delete[] m_animationName;
}

bool KeyframeAnimationJob::past(const float timestamp)
{
#ifdef PRINT_ANIMATION_INFO
	if (!m_paused && m_timeline.back().Timestamp < timestamp)
		printf("Animation has passed at time %.3f! last timestamp: %.3f \n", timestamp, m_timeline.back().Timestamp);
#endif
	return m_paused == false && m_endless == false && m_timeline.back().Timestamp < timestamp;
}

void KeyframeAnimationJob::update(const float timestamp)
{
	// How many timeline weight do we have?
	const int   timeweights = (int) m_timeline.size();
	// If there are no more timelightweights to handle return
	if (m_timeline.empty() || m_currentTimeWeight + 1 >= timeweights) return;
	// Get the time index relative to the start
	const float animTime = timestamp - m_timeline.front().Timestamp;

	// If we have not started the animation yet, the current time weight is less than zero and...
	if (m_currentTimeWeight < 0)
	{
		// if the animation is not yet active...
		if (m_timeline.front().Timestamp > timestamp) return; // ... return
		//  otherwise we have to activate it
		else m_animation->activate(true, m_stageID, m_layerID, m_timeline.front().Weight < 0);
		m_currentTimeWeight = 0;
#ifdef PRINT_ANIMATION_INFO
		//printf("Starting animation at animTime %.3f\n", animTime);
#endif
	}

	if (m_paused) {
		// Animation is paused. Don't advance.
		return;
	}

	// Get next timeline weight
	while( m_currentTimeWeight + 1 < timeweights )
	{
		// get current timeline weight
		TimelineWeight& tw1 = m_timeline[m_currentTimeWeight];
		// and the next one
		TimelineWeight& tw2 = m_timeline[m_currentTimeWeight + 1];

		// if the next timeline weight is in the future
		if (tw2.Timestamp >= timestamp)
		{
			// Calculate new interpolated weight
			float t1 = timestamp - tw1.Timestamp;
			float t2 = tw2.Timestamp - timestamp;
			float kappaT = t1 / (t2 + t1);
			float interpolatedWeight = tw1.Weight + (tw2.Weight - tw1.Weight) * kappaT;
			//printf("Animation %d update at %d: %.3f, %3f\n", m_jobID, m_currentTimeWeight, animTime, interpolatedWeight);
			//printf("Frame %.3f Speed %.3f AnimTime %.3f\n", m_speed * animTime, m_speed, animTime);
			m_animation->update(animTime * m_speed, m_stageID, interpolatedWeight);
			break;
		}
		++m_currentTimeWeight;
#ifdef PRINT_ANIMATION_INFO
		printf("Animation %d now at %d of %d: Timestamp %.3f Timeline %.3f\n", m_jobID, m_currentTimeWeight, timeweights, timestamp, tw2.Timestamp);
#endif
	}
}

void KeyframeAnimationJob::addTimelinePoint(const float timestamp, const float weight)
{
	// Search for the right place to insert the new timeline weight
	std::vector<TimelineWeight>::iterator iter = m_timeline.begin();
	while(iter != m_timeline.end())
	{
		// Replace an existing timeline weight if we update an existing timestamp
		if (iter->Timestamp == timestamp)
		{
			iter->Weight = weight;
			break;
		}
		// Insert a new one if there is a timestamp in the future
		else if (iter->Timestamp > timestamp)
		{

#ifdef PRINT_ANIMATION_INFO
			printf("Inserting timeline point %.3f with weight %.3f\n", timestamp, weight);
#endif
			iter = m_timeline.insert(iter, TimelineWeight(timestamp, weight));
			break;
		}
		++iter;
	}
	// If there are still more timestamps in the future, we will overwrite their weights to the one added
	// since otherwise the weights of the future timestamps may change the animation in an unexpected way back to the original weight
	while( iter != m_timeline.end() )
	{
#ifdef PRINT_ANIMATION_INFO
		printf("Adjusting timeline at %.3f to weight %.3f\n", iter->Timestamp, weight);
#endif
		iter->Weight = weight;
		++iter;
	}
}

void KeyframeAnimationJob::changeSpeed(const float speed)
{
	const float currentTime = GameEngine::timeStamp();
	// adjust beginning that we don't get a gap after changing the speed
	m_timeline.front().Timestamp = (m_timeline.front().Timestamp * m_speed + currentTime * (speed - m_speed) ) / speed;
	m_speed = speed;
}

void KeyframeAnimationJob::changeDuration(float duration)
{
	if (duration < 0)
	{
		duration = ENDLESS_DURATION;
		m_endless = true;
	}
	else if (m_endless)
		m_endless = false;

	const float end = GameEngine::timeStamp() + duration;

	float weight = m_timeline.back().Weight;
	while(m_timeline.back().Timestamp > end)
	{
		weight = m_timeline.back().Weight;
		m_timeline.pop_back();
	}
	m_timeline.push_back(TimelineWeight(end, weight));
}

void KeyframeAnimationJob::pause() {
	m_paused = true;
	// save timestamp to be able to readjust timeline on resume
	m_pauseTimestamp = GameEngine::timeStamp();
}

void KeyframeAnimationJob::unPause() {
	if (!m_paused)
		return;

	// resume the animation with adjusted timeline
	float pauseDuration = GameEngine::timeStamp() - m_pauseTimestamp;

	for (unsigned int i = 0; i < m_timeline.size(); i++) {
		m_timeline[i].Timestamp += pauseDuration;
	}

	m_paused = false;
}
