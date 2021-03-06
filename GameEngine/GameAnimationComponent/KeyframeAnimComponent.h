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
// GameEngine Core of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
//
// ****************************************************************************************

#ifndef KEYFRAMEANIMCOMPONENT_H_
#define KEYFRAMEANIMCOMPONENT_H_

#include <GameEngine/GameComponent.h>

#include <map>

class KeyframeStageController;
class KeyframeAnimationResource;
class KeyframeAnimationJob;
class KeyframeAnimationMeta;
class ICharacterControllerEvent;

class KeyframeAnimComponent : public GameComponent
{
	friend class KeyframeAnimationJob;

public:
	static GameComponent* createComponent( GameEntity* owner );

	KeyframeAnimComponent(GameEntity *owner);
	~KeyframeAnimComponent();



	bool checkEvent(GameEvent* event) { return true; }
	void executeEvent(GameEvent* event);

	void loadFromXml(const XMLNode* description);

	void applyChanges( const float timestamp );
	void update( float timestamp );

	bool isPlaying( const char* animation );

	float getAnimLength( const char* animation, float speed = 0);

	KeyframeAnimationMeta* getAnimResMeta(const char* animation);

	float getAnimSpeed( const char* animation );

	int getJobCount()
	{
		return int(m_animationRegistry.size());
	};
	int getJobID(std::string animName);

	void getSerializedState(GameState& state);

	void setSerializedState(GameState& state);

	void registerCharacterController(ICharacterControllerEvent* charCont){this->m_charCont = charCont;};

private:
	void release();

	void setupAnim(AnimationSetup* command);

	void stopAnim(const int stage);

	void pauseAnim(const int stage);

	void resumeAnim(const int stage);

	void updateAnim(AnimationUpdate* command);

	void setAnimBias(AnimationBias* command);

	KeyframeStageController*					m_stageControllers;

	std::map<std::string, KeyframeAnimationResource*>	m_animationResources;

	std::vector<KeyframeAnimationJob*>			m_animationRegistry;

	std::vector<AnimationUpdate>		m_pendingAnimUpdates;

	const int							MAX_STAGES;

	// needed on a networking KeyframeAnimComponent
	float*								m_initTimestamps;	// saves the timestamp an animation was initiated

	ICharacterControllerEvent*			m_charCont;
};

#endif
