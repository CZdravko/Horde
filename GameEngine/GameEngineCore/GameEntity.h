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
#ifndef GAMEENTITY_H_
#define GAMEENTITY_H_

#include <string>
#include <map>
#include <GameEngine/utMath.h>

#include "GameComponent.h"
#include "GameEvent.h"

struct GameEntityPrivate;

typedef std::string EntityID;

/** \addtogroup GameEngineCore
 * @{
 */

/**
 * \brief Aggregation of Game Components
 *
 * A GameEntity aggregates several game components and provides event methods to send GameEvent instances 
 * to all aggregated components and other componentes that have registered as listener.
 * 
 * @author Volker Wiendl, Felix Kistler
 * @date 2008-2009
 */
class DLLEXP GameEntity
{
	friend class GameComponent;
	friend class GameWorld;
	friend struct GameWorldPrivate;

public:	
	/**
	 * \brief A unique id (string) of the entity within the GameWorld
	 * 
	 * @returns unique id	 
	 */
	const EntityID& id() {return m_id;}

	/** 
	 * \brief the GameWorld index for this entity
	 * @return index of entity in game world
	 */
	unsigned int worldId() { return m_worldId; }

	/**
	 * \brief Adds a component as a listener for the specified event
	 * 
	 * @param eventType the ID of the GameEvent the component wants to listen to
	 * @param listener the component that wants to be notified about the specified event
	 */
	void addListener( GameEvent::EventID eventType, GameComponent* listener );

	/**
	 * \brief Removes a component from the list of listeners for a specific event
	 * 
	 * @param eventType the ID of the GameEvent the component has been listen to
	 * @param listener the component that wants to should be removed from the event's listener list
	 */
	void removeListener( GameEvent::EventID eventType, GameComponent* listener );

	/**
	 * \brief Removes a component from the entity's communication system
	 * 
	 * After calling this method the specified component won't be notified about any event any longer
	 * @param listener the component that should not be notified about events any longer
	 */
	void removeListener( GameComponent* listener );

	/**
	 * \brief Checks all listening components if the given event is allowed to be executed
	 * 
	 * \todo improve documentation of events and maybe change the whole checkevent execute event system to a more consistent one
	 * 
	 * @param event pointer to a GameEvent instance that has been constructed with a specific ID and optional data
	 * @returns false if one component blocks the given event, otherwise true
	 */
	bool checkEvent( GameEvent* event ) const;

	/**
	 * \brief Sends the specified event to all components listening for it
	 * 
	 * When calling this method all components that have registered themself as a listener for the
	 * specified event will be notified to update their state according to the event's type and data.
	 *
	 * @param event pointer to a GameEvent instance that has been constructed with a specific ID and optional data
	 */
	void executeEvent( GameEvent* event );

	/**
	 * \brief Query specific component from the entity
	 * 
	 * @param id the specific ID of one component	 
	 * @return pointer to the GameComponent instance or 0 if no such component is aggregated in the entity
	 */
	GameComponent*	component( const GameComponent::ComponentID& id );

private:
	/**
	 * \brief Private Constructor
	 * 
	 * Only the GameWorld is allowed to create new instances of this class
	 *
	 * @param id a unique id for this entity
	 */
	GameEntity( const EntityID& id );
	/**
	 * \brief Private Destructor
	 * 
	 * Only the GameWorld is allowed to destroy a GameEntity instance
	 */
	~GameEntity();

	/**
	 * \brief Adds a component to the entity
	 * 
	 * Components can be added only by the component itself (called in GameComponent Base Class Constructor).
	 * If a component with the same ComponentID has been added before, the previous component will be destroyed and
	 * removed.
	 *
	 * @param component pointer to the instance of a GameComponent	
	 * @see GameComponent
	 */
	void addComponent( GameComponent* component );

	/**
	 * \brief Removes and deletes a component from the entity
	 * 
	 * Removes and deletes the component instance (if available) with the specified id. 
	 * @param id the specific ID of a GameEngine component plugin
	 */
	inline void removeComponent( const GameComponent::ComponentID& id );

	/**
	 * \brief Removes the component from the entity
	 * 
	 * Removes the specified component from the entity, but DOES NOT delete the instance.
	 * (Will be called from the GameComponent destructor)
	 *
	 * @param component pointer to the component to be removed
	 */
	void removeComponent( GameComponent* component );

	/// Private data of the entity
	GameEntityPrivate*			m_privateData;

#pragma warning( push )
#pragma warning( disable : 4251 )
	/// Unique ID for the entity within the GameWorld
	EntityID					m_id;
#pragma warning( pop )

	/// Index for the entity within the GameWorld
	unsigned int				m_worldId;
};

/*! @}*/
#endif
