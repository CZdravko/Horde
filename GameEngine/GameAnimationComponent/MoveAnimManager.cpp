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
#include "MoveAnimManager.h"

#include "MoveAnimComponent.h"

#include <GameEngine/GameEngine.h>

#include <vector>
#include <algorithm>

using namespace std;

struct NodeUpdate
{
	NodeUpdate(float timeStep) : m_timeStep(timeStep) {}

	void operator()(MoveAnimComponent* const ptr) const
	{
		ptr->update(m_timeStep);
	}

	float m_timeStep;
};

MoveAnimManager* MoveAnimManager::m_instance = 0x0;

MoveAnimManager* MoveAnimManager::instance()
{
	if( m_instance == 0x0 )
	{
		m_instance = new MoveAnimManager();
	}
	return m_instance;
}

void MoveAnimManager::release()
{
	delete m_instance;
	m_instance = 0;
}

void MoveAnimManager::update( )
{
	// Fixed simulation rate of 10 frames per second
	const static float SIM_RATE = 10.0f;
	const static float DELTA_TIME = 1.0f/SIM_RATE;

	const float curTime = GameEngine::currentTimeStamp();
	const float timeStep = curTime - m_updateTime;
	if ( timeStep >= DELTA_TIME ) 
	{
		for_each(m_components.begin(), m_components.end(), NodeUpdate(timeStep));
		m_updateTime = curTime;
	}
}

void MoveAnimManager::addObject(MoveAnimComponent* object)
{
	std::vector<MoveAnimComponent*>::iterator iter = find(m_components.begin(), m_components.end(), object);
	if ( iter == m_components.end() )
		m_components.push_back(object);
}

void MoveAnimManager::removeObject(MoveAnimComponent* object)
{
	std::vector<MoveAnimComponent*>::iterator iter = find(m_components.begin(), m_components.end(), object);
	if (iter != m_components.end())
		m_components.erase(iter);		
}

