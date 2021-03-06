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
// GameEngine Core Library of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007-2009 Volker Wiendl, Felix Kistler
// 
// ****************************************************************************************
#include "GameWorld.h"

#include "GameLog.h"
#include "GameEntity.h"

#include <map>
#include <stack>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

//struct DeleteEntity
//{ 
//	template<typename T> void operator()(const T* ptr) const
//	{
//		delete ptr;
//	}
//};

struct ExecuteEvent
{ 
	ExecuteEvent(GameEvent* e) : event(e) {}
	void operator()(GameEntity* ptr) const
	{
		if (ptr != 0) ptr->executeEvent(event);
	}

	GameEvent* event;
};

struct GameWorldPrivate
{
	~GameWorldPrivate()
	{
		while( !WorldList.empty())
		{
			delete WorldList.back();
			WorldList.pop_back();
		}
		WorldMap.clear();	
		while (!NextIndexList.empty())
			NextIndexList.pop();
	}

	
	void addListener(GameEvent::EventID id, GameEntity* listener)
	{
		// Don't add entities multiple times
		std::vector<GameEntity*>::iterator component = 
			find(Listeners[id].begin(), Listeners[id].end(), listener);
		if (component == Listeners[id].end())
			Listeners[id].push_back(listener);
	}

	void removeListener(GameEvent::EventID id, GameEntity* listener)
	{	
		if (!Listeners[id].empty())
		{
			std::vector<GameEntity*>::iterator component = 
				find(Listeners[id].begin(), Listeners[id].end(), listener);
			if (component != Listeners[id].end())
				Listeners[id].erase(component);
		}
	}

	void removeListener(GameEntity* listener)
	{		
		for (int i=0; i < GameEvent::EVENT_COUNT; ++i)
		{
			std::vector<GameEntity*>::iterator component = 
				find(Listeners[i].begin(), Listeners[i].end(), listener);
			if (component != Listeners[i].end())
				Listeners[i].erase(component);
		}
	}

	std::vector<GameEntity*>			Listeners[GameEvent::EVENT_COUNT];

	std::map<EntityID, unsigned int>	WorldMap;
	std::vector<GameEntity*>			WorldList;

	std::stack<unsigned int>			NextIndexList;
};



GameWorld::GameWorld() 
{
	m_world = new GameWorldPrivate();
}

GameWorld::~GameWorld()
{
	delete m_world;
	m_world = 0x0;
}


GameEntity*	GameWorld::entity(const EntityID& id)
{
	if( m_world == 0x0 ) return 0;

	map<EntityID, unsigned int>::iterator iter = m_world->WorldMap.find(id);
	if (iter == m_world->WorldMap.end())
		return 0;
	return m_world->WorldList.at(iter->second - 1);
}

GameEntity*	GameWorld::entity(const unsigned int id)
{
	if( id > 0 && id <= m_world->WorldList.size() )
		return m_world->WorldList.at(id - 1);
	else
		return 0;
}


unsigned int GameWorld::entityID(const EntityID& id)
{
//	printf("Listing entities:\n");
//	map<EntityID, unsigned int>::iterator printiter = m_world->WorldMap.begin();
//	while(printiter != m_world->WorldMap.end()){
//		printf("name, id : %s, %d\n", (printiter->first).c_str(), printiter->second);
//		printiter++;
//	}
//	printf("looking for: %s\n",id.c_str());
	map<EntityID, unsigned int>::iterator iter = m_world->WorldMap.find(id);
//	printf("found : %s with id: %d\n",iter->first.c_str(), iter->second);
	if (iter != m_world->WorldMap.end())
		return iter->second;
	else
		return 0;
}

GameEntity* GameWorld::createEntity(const EntityID& id)
{
	map<EntityID, unsigned int>::iterator iter = m_world->WorldMap.find(id);
	
	EntityID newID;
	bool changeID = false;
	if (iter != m_world->WorldMap.end())
	{
		// Try to find a free id
		stringstream idstream;
		unsigned int i;
		for (i = 0; i < 1000 && iter != m_world->WorldMap.end(); ++i)
		{
			// Reset stream buffer
			idstream.str("");
			// Clear flags
			idstream.clear();
			// Now add old name and number
			idstream << id << i;
			// And search for an entity with the same name
			iter = m_world->WorldMap.find(idstream.str());		
		}
		if (i < 1000)
		{
			GameLog::warnMessage("Entity with name '%s' already exists, renaming to '%s'", id.c_str(), idstream.str().c_str());
			newID = idstream.str();
			changeID = true;
		}
		else
		{
			GameLog::errorMessage("No free name found for Entity '%s'!", id.c_str());
			return 0x0;
		}
	}

	GameEntity* entity = new GameEntity(changeID ? newID : id);
	if ( m_world->NextIndexList.empty() )
	{
		m_world->WorldList.push_back(entity);
		entity->m_worldId = (unsigned int) m_world->WorldList.size();
		m_world->WorldMap.insert(make_pair(changeID ? newID : id, entity->m_worldId));
	}
	else
	{
		entity->m_worldId = m_world->NextIndexList.top() + 1;
		m_world->WorldList[m_world->NextIndexList.top()] = entity;
		m_world->WorldMap.insert(make_pair(changeID ? newID : id, m_world->NextIndexList.top() + 1));
		m_world->NextIndexList.pop();
	}
	return entity;
}

//GameEntity* removeEntity(const EntityID& id)
//{
//	GameEntity* entity = 0;
//	map<idstring, unsigned int>::iterator iter = WorldMap.find(id);
//	if (iter != WorldMap.end())
//	{
//		entity = WorldList.at(iter->second);
//		NextIndexList.push(iter->second);
//		WorldList.at(iter->second) = 0;
//		WorldMap.erase(iter);
//		
//	}
//	return entity;
//}

void GameWorld::releaseEntity(const EntityID& id)
{
	map<EntityID, unsigned int>::iterator iter = m_world->WorldMap.find(id);
	if (iter != m_world->WorldMap.end())
	{
		m_world->NextIndexList.push(iter->second - 1);
		delete m_world->WorldList.at(iter->second - 1);
		m_world->WorldList.at(iter->second - 1) = 0;
		m_world->WorldMap.erase(iter);			
	}
}


void GameWorld::releaseEntity(const unsigned int id)
{
	GameEntity* entityPtr = entity(id);
	if( entityPtr )
	{
		releaseEntity( entityPtr->id() );
	}
}

void GameWorld::renameEntity(const EntityID& oldId, const EntityID& newId)
{
	releaseEntity(newId);
	map<EntityID, unsigned int>::iterator iter = m_world->WorldMap.find(oldId);
	if (iter != m_world->WorldMap.end())
	{
		unsigned int listID = iter->second;
		m_world->WorldMap.erase(iter);
		m_world->WorldMap.insert(std::make_pair(newId, listID));		
	}
}


void GameWorld::renameEntity(const unsigned int oldId, const EntityID& newId)
{
	GameEntity* entityPtr = entity(oldId);
	if(entityPtr)
	{
		renameEntity( entityPtr->id(), newId );
	}
}


bool GameWorld::checkEvent(GameEvent* event)
{
	//vector<GameEntity*>::iterator iter = m_world->WorldList.begin();
	//const vector<GameEntity*>::iterator end = m_world->WorldList.end();
	//while( iter != end )
	//{
	//	if( (*iter)->checkEvent(event) == false ) return false;
	//	++iter;
	//}
	//return true;
	const int id = event->id();
	std::vector<GameEntity*>::iterator entityIter = m_world->Listeners[id].begin();
	const std::vector<GameEntity*>::iterator entitysEnd = m_world->Listeners[id].end();
	while ( entityIter != entitysEnd )
	{
		if (!(*entityIter)->checkEvent(event))
			return false;
		entityIter++; 
	}
	return true;
}

void GameWorld::executeEvent(GameEvent* event)
{
	//for_each(m_world->WorldList.begin(), m_world->WorldList.end(), ExecuteEvent(event));
	const int id = event->id();
	for_each(m_world->Listeners[id].begin(), m_world->Listeners[id].end(), ExecuteEvent(event));
}

bool GameWorld::checkEvent(unsigned int id, GameEvent* event)
{
	if (id <= m_world->WorldList.size() && m_world->WorldList[id])	
		return m_world->WorldList[id]->checkEvent(event);	
	else
		return false;
}

void GameWorld::executeEvent(unsigned int id, GameEvent* event)
{
	if (id <= m_world->WorldList.size() && m_world->WorldList[id])	
		m_world->WorldList[id]->executeEvent(event);		
}

void GameWorld::addListener( GameEvent::EventID eventType, GameEntity* listener )
{
	m_world->addListener( eventType, listener );
}

void GameWorld::removeListener( GameEvent::EventID eventType, GameEntity* listener )
{
	m_world->removeListener( eventType, listener );
}

void GameWorld::removeListener( GameEntity* listener )
{
	m_world->removeListener( listener );
}
