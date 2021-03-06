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
// Copyright (C) 2007 Felix Kistler
// 
// ****************************************************************************************
#include "MoveAnimComponent.h"

#include "MoveAnimManager.h"

#include "GameEngine_Animations.h"

#include <GameEngine/GameEntity.h>
#include <GameEngine/GameEngine.h>

#include <XMLParser/utXMLParser.h>


GameComponent* MoveAnimComponent::createComponent( GameEntity* owner )
{
	return new MoveAnimComponent( owner );
}

MoveAnimComponent::MoveAnimComponent(GameEntity *owner) : GameComponent(owner, "MoveAnimComponent"), 
	m_oldPos(0,0,0), m_newPos(0, 0, 0), m_moveAnim(0), m_moveBackAnim(0), m_moveLeftAnim(0),
	m_moveRightAnim(0),	m_speed(1.0f), m_activeAnim(0), m_idle(false), m_idleAnimCount(0), m_idleTime(0),
	m_randSeed(false), m_LODToStopAnim(999), m_lod(-1), m_rotationOffset(0), 
	m_runAnim(0), m_runThreshold(999.0f), m_currentSpeedIndex(0), m_avgSpeed(0)
{
	for( int i=0; i< 5; ++i )
		m_idleAnim[i] = 0x0;

	for(int j = 0; j < SPEED_HISTORY_SIZE; j++)
		m_lastSpeeds[j] = 0;

	//get initial position
	float absTrans[16];
	GameEvent getTransformation(GameEvent::E_TRANSFORMATION, absTrans, this);	
	owner->executeEvent(&getTransformation);
	Vec3f s;
	Matrix4f(absTrans).decompose(m_newPos, m_rotation, s);
	m_oldPos;	
	
	//listen for transformation changes
	owner->addListener(GameEvent::E_SET_TRANSFORMATION, this);
	owner->addListener(GameEvent::E_SET_TRANSLATION, this);

	// listen if move anim should be set
	owner->addListener(GameEvent::E_SET_MOVE_ANIM, this);

	//listen for lod change
	owner->addListener(GameEvent::E_AILOD_CHANGE, this);
	
	MoveAnimManager::instance()->addObject(this);
}

MoveAnimComponent::~MoveAnimComponent()
{
	MoveAnimManager::instance()->removeObject(this);

	delete m_moveAnim;
	delete m_moveBackAnim;
	delete m_moveLeftAnim;
	delete m_moveRightAnim;
	delete m_runAnim;
	for( int i=0; i< m_idleAnimCount; ++i )
	{
		delete m_idleAnim[i];
	}
}

void MoveAnimComponent::executeEvent(GameEvent *event)
{
	switch(event->id())
	{
	case GameEvent::E_SET_TRANSFORMATION:
		{
			Matrix4f absTrans = Matrix4f(static_cast<const float*>(event->data()));
			static Vec3f s;
			absTrans.decompose(m_newPos, m_rotation, s);
		}
		break;
	case GameEvent::E_AILOD_CHANGE:
		m_lod = *static_cast<int*>(event->data());
		break;
	case GameEvent::E_SET_TRANSLATION:
		m_newPos = *static_cast<Vec3f*>(event->data());
		break;
	case GameEvent::E_SET_MOVE_ANIM:
		std::pair<std::string, std::string>* data = 
			static_cast<std::pair<std::string, std::string>*>(event->data());
		changeMoveAnim(data->first, data->second);
		break;
	}
}

void MoveAnimComponent::loadFromXml(const XMLNode* description)
{
	// Get speed factor to multiply on animation speed
	m_speed = static_cast<float>(atof(description->getAttribute("speed", "1.0"))); 

	// Get animation names
	const char* move = description->getAttribute("moveAnimation", "");
	const char* idle = description->getAttribute("idleAnimation", "");
	const char* idle2 = description->getAttribute("idleAnimation2", "");
	const char* idle3 = description->getAttribute("idleAnimation3", "");
	const char* idle4 = description->getAttribute("idleAnimation4", "");
	const char* idle5 = description->getAttribute("idleAnimation5", "");
	const char* moveBack = description->getAttribute("moveBackAnimation", "");
	const char* moveLeft = description->getAttribute("moveLeftAnimation", "");
	const char* moveRight = description->getAttribute("moveRightAnimation", "");
	const char* run = description->getAttribute("runAnimation", "");

	// Get lod value from which on animation is stopped, but remove listener if not set
	m_LODToStopAnim = static_cast<int>(atoi(description->getAttribute("LODToStopAnim", "9999")));
	if (m_LODToStopAnim == 9999)
		m_owner->removeListener(GameEvent::E_AILOD_CHANGE, this);

	// Rotation offset of 180� is standard, as most characters are modeled looking towards the user from the screen
	// But walking forward is meant to be walking "into" the screen
	m_rotationOffset = degToRad((float) atof(description->getAttribute("rotationOffset", "180")));


	// Play animation on stage 10 to not disturb other animations on this model
	if( _stricmp(move, "") != 0 )
	{
		m_moveAnim = static_cast<AnimationSetup*>(AnimationSetup(move, 10, 0, -1.0f, 1.0f, 0.0f).clone());		
		m_moveAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveAnim->Animation);
	}
	if( _stricmp(moveBack, "") != 0 )
	{
		m_moveBackAnim = static_cast<AnimationSetup*>(AnimationSetup(moveBack, 10, 0, -1.0f, 1.0f, 0.0f).clone());		
		m_moveBackAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveBackAnim->Animation);
	}
	if( _stricmp(moveLeft, "") != 0 )
	{
		m_moveLeftAnim = static_cast<AnimationSetup*>(AnimationSetup(moveLeft, 10, 0, -1.0f, 1.0f, 0.0f).clone());		
		m_moveLeftAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveLeftAnim->Animation);
	}
	if( _stricmp(moveRight, "") != 0 )
	{
		m_moveRightAnim = static_cast<AnimationSetup*>(AnimationSetup(moveRight, 10, 0, -1.0f, 1.0f, 0.0f).clone());		
		m_moveRightAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveRightAnim->Animation);
	}

	if( _stricmp(run, "") != 0 )
	{
		m_runAnim = static_cast<AnimationSetup*>(AnimationSetup(run, 10, 0, -1.0f, 1.0f, 0.0f).clone());		
		m_runAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_runAnim->Animation);
	}

	// Threshold for the speed from which on the run animation is used,
	// also represents the translation speed at which the setted animation playback speed is applied
	m_runThreshold = static_cast<float>(atof(description->getAttribute("runThreshold", "999")));

	m_moveThreshold = static_cast<float>(atof(description->getAttribute("moveThreshold", "0.0005"))); 
	
	// Idle anims have weight of 0.01, which means that they are suppressed as soon as another animation is playing
	m_idleAnimCount = 0;
	if( _stricmp(idle, "") != 0 )
	{
		m_idleAnim[m_idleAnimCount] = static_cast<AnimationSetup*>(AnimationSetup(idle, 10, 0, -1.0f, 0.01f, 0.0f).clone());
	
		//initially start idle animation
		setAnim(m_idleAnim[m_idleAnimCount], true);
		m_idleAnimCount++;
	}
	if( _stricmp(idle2, "") != 0 )
	{
		m_idleAnim[m_idleAnimCount] = static_cast<AnimationSetup*>(AnimationSetup(idle2, 10, 0, -1.0f, 0.01f, 0.0f).clone());
		m_idleAnimCount++;
	}
	if( _stricmp(idle3, "") != 0 )
	{
		m_idleAnim[m_idleAnimCount] = static_cast<AnimationSetup*>(AnimationSetup(idle3, 10, 0, -1.0f, 0.01f, 0.0f).clone());
		m_idleAnimCount++;
	}
	if( _stricmp(idle4, "") != 0 )
	{
		m_idleAnim[m_idleAnimCount] = static_cast<AnimationSetup*>(AnimationSetup(idle4, 10, 0, -1.0f, 0.01f, 0.0f).clone());
		m_idleAnimCount++;
	}
	if( _stricmp(idle5, "") != 0 )
	{
		m_idleAnim[m_idleAnimCount] = static_cast<AnimationSetup*>(AnimationSetup(idle5, 10, 0, -1.0f, 0.01f, 0.0f).clone());
		m_idleAnimCount++;
	}
}

void MoveAnimComponent::update(float timeStep)
{
	if (timeStep > Math::Epsilon)
	{
		// Only calc next anim if LOD is low enough
		if (m_lod < m_LODToStopAnim)
		{
			//calculate translated distance and speed in x,z plane
			Vec3f dist = m_newPos - m_oldPos;
			dist.y = 0;
			float speed = m_speed * dist.length() / timeStep;
			static const float pi3rd = Math::Pi / 3;
		
			//if (m_runAnim)
			{
				// Currently the average speed is only used for the run animation
				m_lastSpeeds[m_currentSpeedIndex] = speed;
				m_currentSpeedIndex = (m_currentSpeedIndex + 1) % SPEED_HISTORY_SIZE;
				m_avgSpeed = 0;
				for (int i = 0; i < SPEED_HISTORY_SIZE; i++)
				{
					m_avgSpeed += m_lastSpeeds[i];
				}
				m_avgSpeed /= SPEED_HISTORY_SIZE;
			}

			// Object is moving
			if( m_avgSpeed >= m_moveThreshold && speed > 0)
			{
				AnimationSetup* nextAnim = 0x0;
				// Direction in radiants = angle of walking direction - (y-rotation of the object + rotation offset)
				float direction = atan2( -dist.x, -dist.z ) - (m_rotation.y + m_rotationOffset);
			
				// Normalize direction angle to [-PI, +PI]
				// To get less if cases
				if( direction > Math::Pi ) 
					direction = direction - Math::TwoPi;
				if( direction < -Math::Pi )
					direction = Math::TwoPi + direction;

				// Find the right move animation to play by the moving direction
				// If the exact animation is not specified, play the one for forward moving
				if (m_moveBackAnim != 0x0 && (direction >= 2*pi3rd || direction <= -2*pi3rd))
				{
					nextAnim = m_moveBackAnim;
				}
				else if (m_moveLeftAnim != 0x0 && (direction > pi3rd && direction < 2*pi3rd))
				{
					nextAnim = m_moveLeftAnim;
				}
				else if (m_moveRightAnim != 0x0 && (direction > -2*pi3rd && direction < -pi3rd))
				{
					nextAnim = m_moveRightAnim;
				}
				else if (m_runAnim != 0 && m_avgSpeed > m_runThreshold)
				{
					nextAnim = m_runAnim;
					// Reduce playback speed for run anim
					speed -= m_runThreshold - m_speed;
					//printf("Running speed: %.5f\n", speed);
				}
				else if (m_moveAnim != 0x0)
				{
					nextAnim = m_moveAnim;
				}
				// Set anim or reset everything if none found (nextAnim == 0x0)
				setAnim(nextAnim);
			}
			// Play idle animation if too slow and one is specified
			else if (m_idleAnimCount > 0)
			{
				// We weren't in idle state before or the animation time for the idle anim has passed
				if(!m_idle || m_idleTime < 0)
				{
					if(!m_randSeed)
					{
						// rand hasn't been seeded yet
						srand( (unsigned int) (100000 / timeStep) );
						m_randSeed = true;
					}
					// Choose and set new random idle animation
					int i = rand() % m_idleAnimCount; 
					setAnim( m_idleAnim[i], true );
				}
				else
				{
					// The current idle animation is still playing
					// So decrement timer
					m_idleTime -= timeStep;
				}
			}
			// We don't have an anim to play, so stop any currently playing 
			else
			{
				setAnim(0x0);
			}

			//update animation speed (only none idle animations)
			if( m_activeAnim != 0x0 && !m_idle)
			{
				GameEvent updateAnim(GameEvent::E_UPDATE_ANIM, &AnimationUpdate(m_activeAnim->JobID, GameEngineAnimParams::Speed, m_activeAnim->Speed * m_avgSpeed, 0), 0);
				if (m_owner->checkEvent(&updateAnim))
					m_owner->executeEvent(&updateAnim);
			}
		}
		// The LOD is above the critical value, so stop all current anims
		else
			setAnim(0x0);
	
		//update position
		m_oldPos = m_newPos;
	}
}

void MoveAnimComponent::setAnim(AnimationSetup *anim, bool idle /*=false*/)
{
	//Do nothing if animation is already playing
	if( m_activeAnim == anim )
	{
		m_idle = idle;
		return;
	}

	//Stop last animation if still playing
	if( m_activeAnim != 0x0 )
	{
		GameEvent updateAnim(GameEvent::E_UPDATE_ANIM, &AnimationUpdate(m_activeAnim->JobID, GameEngineAnimParams::Duration, 0, 0), 0);
		if (m_owner->checkEvent(&updateAnim))
		{
			m_owner->executeEvent(&updateAnim);
			m_activeAnim = 0x0;
		}
	}

	//If there is no new animation, we have finished
	if( anim == 0x0 ) 
	{
		if (m_runAnim)
		{
			// Reset last speeds as we have resetted our current anim
			for(int j = 0; j < SPEED_HISTORY_SIZE; j++)
				m_lastSpeeds[j] = 0;
		}

		m_idle = idle;
		return;
	}

	//Play given animation
	GameEvent walk(GameEvent::E_PLAY_ANIM, anim, this);
	if (m_owner->checkEvent(&walk))
	{
		m_owner->executeEvent(&walk);
		m_activeAnim = anim;
		m_idle = idle;

		//Calculate time for playing next random idle animation
		if( idle )
		{
			m_idleTime = GameEngine::getAnimLength( m_owner->worldId(), m_activeAnim->Animation );
			m_idleTime *= 1.0f + (rand() % 4);
		}
	}
}


void MoveAnimComponent::changeMoveAnim(const std::string& tag, const std::string& name)
{
	AnimationSetup* animToDelete = 0x0;
	if (tag.compare("moveAnimation") == 0)
	{
		animToDelete = m_moveAnim;
		m_moveAnim = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 1.0f, 0.0f).clone());		 
		m_moveAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveAnim->Animation);
	}
	else if (tag.compare("idleAnimation") == 0)
	{
		if (m_idleAnimCount > 0)
			animToDelete = m_idleAnim[0];
		else m_idleAnimCount++;
		m_idleAnim[0] = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 0.0f, 0.0f).clone());
	}
	else if (tag.compare("idleAnimation2") == 0 && m_idleAnimCount > 0)
	{
		if (m_idleAnimCount > 1)
			animToDelete = m_idleAnim[1];
		else m_idleAnimCount++;
		m_idleAnim[1] = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 0.0f, 0.0f).clone());
	}
	else if (tag.compare("idleAnimation3") == 0 && m_idleAnimCount > 1)
	{
		if (m_idleAnimCount > 2)
			animToDelete = m_idleAnim[2];
		else m_idleAnimCount++;
		m_idleAnim[2] = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 0.0f, 0.0f).clone());
	}
	else if (tag.compare("idleAnimation4") == 0 && m_idleAnimCount > 2)
	{
		if (m_idleAnimCount > 3)
			animToDelete = m_idleAnim[3];
		else m_idleAnimCount++;
		m_idleAnim[3] = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 0.0f, 0.0f).clone());
	}
	else if (tag.compare("idleAnimation5") == 0 && m_idleAnimCount > 3)
	{
		if (m_idleAnimCount > 4)
			animToDelete = m_idleAnim[4];
		else m_idleAnimCount++;
		m_idleAnim[4] = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 0.0f, 0.0f).clone());
	}
	else if (tag.compare("moveBackAnimation") == 0)
	{
		animToDelete = m_moveBackAnim;
		m_moveBackAnim = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 1.0f, 0.0f).clone());		 
		m_moveBackAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveBackAnim->Animation);
	}
	else if (tag.compare("moveLeftAnimation") == 0)
	{
		animToDelete = m_moveLeftAnim;
		m_moveLeftAnim = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 1.0f, 0.0f).clone());		 
		m_moveLeftAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveLeftAnim->Animation);
	}
	else if (tag.compare("moveRightAnimation") == 0)
	{
		animToDelete = m_moveRightAnim;
		m_moveRightAnim = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 1.0f, 0.0f).clone());		 
		m_moveRightAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_moveRightAnim->Animation);
	}
	else if (tag.compare("runAnimation") == 0)
	{
		animToDelete = m_runAnim;
		m_runAnim = static_cast<AnimationSetup*>(AnimationSetup(name.c_str(), 10, 0, -1.0f, 1.0f, 0.0f).clone());		 
		m_runAnim->Speed = GameEngine::getAnimSpeed(m_owner->worldId(), m_runAnim->Animation);
	}

	if (m_activeAnim != 0x0 && animToDelete == m_activeAnim)
	{
		// Stop active anim because it will be deleted
		GameEvent updateAnim(GameEvent::E_UPDATE_ANIM, &AnimationUpdate(m_activeAnim->JobID, GameEngineAnimParams::Duration, 0, 0), 0);
		if (m_owner->checkEvent(&updateAnim))
		{
			m_owner->executeEvent(&updateAnim);
			m_activeAnim = 0x0;
		}
	}
	// Reset idle state to reload changed idle animation
	m_idle = false;
	// Now the anim can be safely deleted
	delete animToDelete;
}