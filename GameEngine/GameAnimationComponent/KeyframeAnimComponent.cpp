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
// GameEngine Animation Component of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
//
// ****************************************************************************************

#include "KeyframeAnimComponent.h"
#include "KeyframeAnimManager.h"
#include "GameEngine_Animations.h"

#include <GameEngine/GameEntity.h>
#include <GameEngine/GameEngine.h>
#include <GameEngine/GameLog.h>


#include <Horde3D/Horde3D.h>
#include <Horde3D/Horde3DUtils.h>

#include <algorithm>
#include <fstream>
#include <list>

#include <XMLParser/utXMLParser.h>

#include <float.h>

#include "config.h"

#include "KeyframeStageController.h"
#include "KeyframeAnimationJob.h"
#include "KeyframeAnimationResource.h"

GameComponent* KeyframeAnimComponent::createComponent( GameEntity* owner )
{
	return new KeyframeAnimComponent( owner );
}

KeyframeAnimComponent::KeyframeAnimComponent(GameEntity* owner) : GameComponent(owner, "KeyframeAnimComponent"), MAX_STAGES(16)
{
	owner->addListener(GameEvent::E_PLAY_ANIM, this);
	owner->addListener(GameEvent::E_STOP_ANIM, this);
	owner->addListener(GameEvent::E_PAUSE_ANIM, this);
	owner->addListener(GameEvent::E_RESUME_ANIM, this);
	owner->addListener(GameEvent::E_UPDATE_ANIM, this);
	owner->addListener(GameEvent::E_GET_ANIM_LENGTH, this);

	m_stageControllers = new KeyframeStageController[MAX_STAGES];

	m_initTimestamps = new float[MAX_STAGES];

	for (int i=0; i<MAX_STAGES; ++i)
	{
		m_stageControllers[i].StageID = i;
		m_stageControllers[i].Owner = this;

		m_initTimestamps[i] = 0;
	}

	KeyframeAnimManager::instance()->addObject(this);
}

KeyframeAnimComponent::~KeyframeAnimComponent()
{
	KeyframeAnimManager::instance()->removeObject(this);
	release();
	delete[] m_stageControllers;
}

void KeyframeAnimComponent::release()
{
	for (int i=0; i<MAX_STAGES; ++i)
		m_stageControllers[i].clear();
	std::map<std::string, KeyframeAnimationResource*>::iterator iter = m_animationResources.begin();
	while (iter != m_animationResources.end())
	{
		h3dRemoveResource(iter->second->AnimResourceID);
		delete iter->second;
		++iter;
	}
	m_animationResources.clear();
	h3dReleaseUnusedResources();
}

void KeyframeAnimComponent::executeEvent(GameEvent *event)
{
	switch(event->id())
	{
	case GameEvent::E_PLAY_ANIM:
		setupAnim(static_cast<AnimationSetup*>(event->data()));
		break;
	case GameEvent::E_STOP_ANIM:
		stopAnim(*((const int*)event->data()));
		break;
	case GameEvent::E_PAUSE_ANIM:
		pauseAnim(*((const int*)event->data()));
		break;
	case GameEvent::E_RESUME_ANIM:
		resumeAnim(*((const int*)event->data()));
		break;
	case GameEvent::E_UPDATE_ANIM:
		updateAnim(static_cast<AnimationUpdate*>(event->data()));
		break;
	case GameEvent::E_GET_ANIM_LENGTH:
		{
			AnimLengthData* data = static_cast<AnimLengthData*>(event->data());
			*(data->Length) = getAnimLength(data->Name, data->Speed);
		}
		break;
	}
}

void KeyframeAnimComponent::loadFromXml(const XMLNode* description)
{
	release();
	int animCount = description->nChildNode("StaticAnimation");
	for (int i=0; i<animCount; ++i)
	{
		XMLNode animation = description->getChildNode("StaticAnimation", i);
		const char* file = animation.getAttribute("file", "");
		const char* name = animation.getAttribute("name", "");
		if (m_animationResources.find(name) != m_animationResources.end())
		{
			GameLog::errorMessage("Another animation with the name '%s' has been already loaded!", name);
			return;
		}
		std::string path = h3dutGetResourcePath(H3DResTypes::Animation);
		if ( path.size() > 0 && path[path.size()-1] != '\\' && path[path.size()-1] != '/' )
			path += '/';
		// Open resource file
		std::ifstream inf( (path + file).c_str(), std::ios::binary );
		if( inf ) // Resource file found
		{
			// Find size of resource file
			inf.seekg( 0, std::ios::end );
			const int size = (int) inf.tellg();
			// Copy resource file to memory
			char *data = new char[size + 1];
			inf.seekg( 0 );
			inf.read( data, size );
			inf.close();
			// Null-terminate buffer - this is important for XML parsers
			data[size] = '\0';
			// Add animation resources to horde resource manager
			const int resourceID = h3dAddResource( H3DResTypes::Animation, file, 0 );
			// Send resource data to engine
			h3dLoadResource( resourceID, data, size );
			delete[] data;
			int frames = h3dGetResParamI(resourceID, H3DAnimRes::EntityElem, 0, H3DAnimRes::EntFrameCountI);
			KeyframeAnimationResource* anim = new KeyframeAnimationResource( owner(), resourceID, frames, static_cast<float>(atof(animation.getAttribute("fps","30.0"))) );
			m_animationResources.insert(std::make_pair(name, anim));
		}
		else // Resource file not found
			GameLog::errorMessage("Animation file %s not found ", file);
	}
}

void KeyframeAnimComponent::applyChanges(const float timestamp)
{
	std::vector<AnimationUpdate>::iterator iter = m_pendingAnimUpdates.begin();
	while( iter != m_pendingAnimUpdates.end() )
	{
		if (iter->TimeOffset <=  timestamp)
		{
			iter->TimeOffset = 0;
			updateAnim(&(*iter));
			iter = m_pendingAnimUpdates.erase(iter);
		}
		else ++iter;
	}
}
void KeyframeAnimComponent::update(const float timestamp)
{
	for (int i=0; i<MAX_STAGES; ++i)
		m_stageControllers[i].update(timestamp);
}

bool KeyframeAnimComponent::isPlaying(const char *animation)
{
	std::map<std::string, KeyframeAnimationResource*>::iterator iter = m_animationResources.find(animation);
	if(iter == m_animationResources.end()) return false;

	for (int i=0; i<MAX_STAGES; ++i)
	{
		if( !m_stageControllers[i].Animations.empty() &&
			m_stageControllers[i].Animations.front()->m_animation->AnimResourceID == iter->second->AnimResourceID)
		{
			return true;
		}
	}
	return false;
}

float KeyframeAnimComponent::getAnimLength(const char *animation, float speed /*= 0*/)
{
	std::map<std::string, KeyframeAnimationResource*>::iterator iter = m_animationResources.find(animation);
	if(iter == m_animationResources.end()) return 0;

	if (speed > 0)
		return iter->second->Frames / speed;
	else if (iter->second->Speed > 0)
		return iter->second->Frames / iter->second->Speed;
	return 0;
}

float KeyframeAnimComponent::getAnimSpeed(const char *animation)
{
	std::map<std::string, KeyframeAnimationResource*>::iterator iter = m_animationResources.find(animation);
	if(iter == m_animationResources.end()) return 0;

	return iter->second->Speed;
}

void KeyframeAnimComponent::setupAnim(AnimationSetup *command)
{
	std::map<std::string, KeyframeAnimationResource*>::iterator iter = m_animationResources.find(command->Animation);
	if (command->Stage >= MAX_STAGES || command->Stage < 0)
	{
		GameLog::errorMessage("Invalid Stage ID '%d'! Value must between 0 and %d", command->Stage, MAX_STAGES);
		return;
	}
	if (iter == m_animationResources.end())
	{
		GameLog::errorMessage("setupAnim: Animation %s not found!", command->Animation);
		return;
	}
	KeyframeAnimationJob* job = new KeyframeAnimationJob(this, iter->second, command);
	command->JobID = job->m_jobID;
	m_stageControllers[command->Stage].addAnimationJob(job);
}

void KeyframeAnimComponent::updateAnim(AnimationUpdate *command)
{
	const float time = GameEngine::timeStamp();
	if (m_animationRegistry.size() >= command->JobID && command->JobID > 0)
	{
		KeyframeAnimationJob* job = m_animationRegistry[command->JobID - 1];
		if (job)
		{
			switch (command->ParamType)
			{
			case GameEngineAnimParams::Weight:
				job->addTimelinePoint(time + command->TimeOffset, command->Value);
				break;
			case GameEngineAnimParams::Speed:
				if (command->TimeOffset > 0)
					m_pendingAnimUpdates.push_back(AnimationUpdate(command->JobID, command->ParamType, command->Value, command->TimeOffset + time));
				else
					job->changeSpeed(command->Value);
				break;
			case GameEngineAnimParams::Duration:
				if (command->TimeOffset > 0)
					m_pendingAnimUpdates.push_back(AnimationUpdate(command->JobID, command->ParamType, command->Value, command->TimeOffset + time));
				else
					job->changeDuration(command->Value);
				break;
			}
		}
	}
}

void KeyframeAnimComponent::stopAnim(const int stage)
{
	if (stage >= MAX_STAGES)
		return;

	if (m_stageControllers[stage].Animations.size() == 0)
		return;

	m_stageControllers[stage].popAnimationJob();
}

void KeyframeAnimComponent::pauseAnim(const int stage)
{
	if (stage >= MAX_STAGES)
		return;

	if (m_stageControllers[stage].Animations.size() == 0)
		return;

	KeyframeAnimationJob* job = m_stageControllers[stage].Animations.front();

	job->pause();
}

void KeyframeAnimComponent::resumeAnim(const int stage)
{
	if (stage >= MAX_STAGES)
		return;

	if (m_stageControllers[stage].Animations.size() == 0)
		return;

	KeyframeAnimationJob* job = m_stageControllers[stage].Animations.front();

	job->unPause();
}

int KeyframeAnimComponent::getJobID(std::string animName)
{
	std::map<std::string, KeyframeAnimationResource*>::iterator iter = m_animationResources.find(animName);
	if(iter != m_animationResources.end())
	{
		for(unsigned int i = 0; i < m_animationRegistry.size(); ++i)
		{
			if(m_animationRegistry[i]->m_animation = iter->second) return i+1;
		}
	}
	return 0;
}

// getSerializedState: for low bandwidth usage, only the currently playing animation on each stage gets transmitted
void KeyframeAnimComponent::getSerializedState(GameState& state) {
	// transmitting local timestamp to be able to create correct animation timelines on clients
	float localtime = GameEngine::timeStamp();
	state.storeFloat(localtime);

	// serializing stage controllers		ASSUMPTION: MAX_STAGES is equal on client and server

	for (int i = 0; i < MAX_STAGES; i++) {
		unsigned int numAnims = m_stageControllers[i].Animations.size();
		state.storeUInt32(numAnims);

		if (m_stageControllers[i].Animations.size() == 0)
			continue;

		// currently playing animation
		KeyframeAnimationJob* job = m_stageControllers[i].Animations.front();

		state.storeString(job->m_animationName);
		state.storeBool(job->m_endless);
		state.storeBool(job->m_paused);
		state.storeFloat(job->m_speed);

		unsigned int tlsize = job->m_timeline.size();
		state.storeUInt32(tlsize);

		for (size_t j = 0; j < job->m_timeline.size(); j++) {
			state.storeFloat(job->m_timeline[j].Timestamp);
			state.storeFloat(job->m_timeline[j].Weight);
		}
	}
}

// setSerializedState: In a networking scenario, a locally playing animation might get overwritten by an animation playing on a remote, sending GameEngine (on the same stage)!
void KeyframeAnimComponent::setSerializedState(GameState& state) {

	char anim_name[256];
	bool anim_endless;
	bool anim_paused;
	float anim_speed;

	KeyframeAnimationJob::TimelineWeight tl_weight(0, 0);

	// calulate sender/recipient real time discrepancy
	float remotetimestamp;
	if (state.readFloat(&remotetimestamp))
		return;
	float deltatimestamp = remotetimestamp - GameEngine::timeStamp();

	for (int i = 0; i < MAX_STAGES; i++) {
		unsigned int stage_length;
		if(state.readUInt32(&stage_length))
			return;
		if (stage_length == 0) {
			// empty stage controller
			stopAnim(i);
			continue;
		} else {
			if (state.readString(anim_name, 256))
				return;
			if (state.readBool(&anim_endless))
				return;
			if (state.readBool(&anim_paused))
				return;
			if (state.readFloat(&anim_speed))
				return;

			unsigned int tlsize;

			if (state.readUInt32(&tlsize))
				return;
			if (state.readFloat(&tl_weight.Timestamp))
				return;
			if (state.readFloat(&tl_weight.Weight))
				return;

			if (anim_paused)
				m_initTimestamps[i] = 0;	// workaround to force update on paused animations

			// was this job already registered?
			if (tl_weight.Timestamp == m_initTimestamps[i]) {

				// skip to next stage controller
				for (size_t j = 1; j < tlsize; j++) {
					state.skipBytes(sizeof(float)*2);
				}

				continue;
			}

			// save timestamp
			m_initTimestamps[i] = tl_weight.Timestamp;

			AnimationSetup command(anim_name, i, anim_speed, 1.0f, 1.0f, 0);	// constant values will be adjusted afterwards

			m_stageControllers[i].clear();

			setupAnim(&command);

			if (m_stageControllers[i].Animations.size() == 0) {		// something went terribly wrong
				// skip to next stage controller
				for (size_t j = 1; j < tlsize; j++) {
					state.skipBytes(sizeof(float)*2);
				}

				continue;
			}

			// adjusting the job's timeline to client's real time
			KeyframeAnimationJob* job = m_stageControllers[i].Animations.front();
			job->m_endless = anim_endless;
			job->m_timeline.clear();

			tl_weight.Timestamp -= deltatimestamp;

			job->m_timeline.push_back(tl_weight);

			for (size_t j = 1; j < tlsize; j++) {
				if (state.readFloat(&tl_weight.Timestamp))
					return;
				if (state.readFloat(&tl_weight.Weight))
					return;

				tl_weight.Timestamp -= deltatimestamp;
				job->m_timeline.push_back(tl_weight);
			}

			job->m_paused = anim_paused;
		}
	}
}
