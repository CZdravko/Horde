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
// Copyright (C) 2007-2009 Felix Kistler
// 
// ****************************************************************************************
#ifndef MOVEANIMCOMPONENT_H_
#define MOVEANIMCOMPONENT_H_

#include <GameEngine/GameComponent.h>
#include <GameEngine/GameEvent.h>

/**
 * \brief Automatic animation control for moving entities
 * 
 * A common problem when moving graphical representations like virtual characters 
 * is a correct animation of the virtual representation. One way to adjust the speed
 * of a moving animation (e.g. a walk animation) automatically is to use the transformation
 * change between two frames. Dependent on the length of the difference vector of the position's
 * between the current and the last frame, the animation can be adjusted to be played faster or
 * slower. The MoveAnimComponent implements an automatic animation control using this approach.
 * 
 * To add such a component you also need a KeyframeAnimComponent for the playback of animations.
 * The following example would create the necessary components:
 * <br>
 * \code
  <GameEntity name="VirtualCharacter">
    <KeyframeAnimation>
      <StaticAnimation fps="30" name="idle" file="female_idle.anim" />
      <StaticAnimation fps="30" name="walk" file="female_walk.anim" />
    </KeyframeAnimation>
    <MoveAnimation moveAnim="walk" idleAnim="idle" speed="7.0" blendTime="0.5"/>
  </GameEntity>
  \endcode
 * <br>
 * In this example you have two static animations for the KeyframeAnimComponent. 
 * These two animations will be used in the MoveAnimComponent referenced by the
 * attributes moveAnim and idleAnim. If the translation change of an 
 * entity between two frames falls below a specific threshold the idleAnimation will be activated.
 * Otherwise the activeAnimation will be used. To adjust the animation speed by a constant
 * factor (maybe because the default framerate of the animation does not fit to the characters translation
 * changes) you can set a constant scaling factor using the speed attribute.
 * 
 * @author Felix Kistler & Volker Wiendl
 * @date Jun 2008
 */ 
class MoveAnimComponent : public GameComponent
{

public:
	/**
	 * \brief Factory method for MoveAnimComponent
	 * 
	 * To register the component in the GameEngineCore a static factory method has to be provided,
	 * that will create a new instance of this component.
	 * 
	 * @param owner the GameEntity that will contain the newly created compoment ( must not be Null )
	 * @return a new MoveAnimComponent instance (casted to the parent class)
	 */ 
	static GameComponent* createComponent( GameEntity* owner );

	/**
	 * \brief Constructor
	 * 
	 * Creates a new MoveAnimComponent that will be a part of the given entity
	 * @param owner the GameEntity that will contain the newly created component (must not be Null)
	 */ 
	MoveAnimComponent(GameEntity *owner);

	/**
	 * \brief Destructor
	 */ 
	~MoveAnimComponent();


	/**
	 * \brief Check if event can be executed 
	 * 
	 * Returns false if the provided event should be not executed.
	 * This can be usefull if a translation event is blocked e.g. by a collision detection
	 * @param event pointer to the event to test
	 * @return true if all components accept the event, false if one of the component has catched it and 
	 * don't want it to be executed
	 */
	bool checkEvent(GameEvent* event){return true;};
	
	/**
	 * \brief Executes the provided event
	 * @param event pointer to the event to execute
	 */
	void executeEvent(GameEvent* event);

	/**
	 * \brief Load component from XML description
	 * 
 	 * Initializes the component from the given XML data. The XML node has to be of the following structure
	 * \code <MoveAnimation activeAnimation="walk" idleAnimation="idle" speed="7.0" /> \endcode <br>
	 * @param description a XML Node representing the data for the component ( must not be null)
	 */ 
	void loadFromXml(const XMLNode* description);

	/**
	 * \brief Plays the next frame of the animation 
	 * 
	 * Dependent on the current fps and settings the activeAnimation or idleAnimation 
	 * will be adjusted
	 * @param timeStep the time past since the last update
	 */ 
	void update(float timeStep);

private:
	/**
	 * \brief Toggles between animations
	 * 
	 * @param anim which animation shall be played.
	 * @param idle whether the animation is an idle one.
	 */ 
	void setAnim(AnimationSetup* anim, bool idle=false);

	/**
	 * \brief Change one of the configured anims
	 * 
	 * @param tag specifying which anim should be set
	 * @param name the name of the new anim
	 */ 
	void changeMoveAnim(const std::string& tag, const std::string& name);
	
	/// Configuration setup for the animation played when translating the entity in -z direction
	AnimationSetup* m_moveAnim;
	/// Configuration setup for the animation played when translating the entity in +z direction
	AnimationSetup* m_moveBackAnim;
	/// Configuration setup for the animation played when translating the entity in +x direction
	AnimationSetup* m_moveRightAnim;
	/// Configuration setup for the animation played when translating the entity in -x direction
	AnimationSetup* m_moveLeftAnim;
	/// Configuration setup for the animation played when translation fast in the -z direction
	AnimationSetup* m_runAnim;
	/// up to 5 alternative Configuration setups for the animation played when entity is not moving
	AnimationSetup* m_idleAnim[5];
	/// count of idle Animations
	int m_idleAnimCount;
	/// time to switch idleAnimation next
	float m_idleTime;

	/// Position in previous and current frame
	Vec3f m_oldPos, m_newPos, m_rotation;

	/// Constant scale factor for moving animation
	float m_speed;

	// translation speed at which the run animation is turned on instead of the move animation
	float m_runThreshold;
	// translation speed at which the idle animation is switched to the move animation
	float m_moveThreshold;


	enum Constants
	{
		SPEED_HISTORY_SIZE = 5
	};

	// Last 5 speeds
	float m_lastSpeeds[SPEED_HISTORY_SIZE];
	// The current writing pos
	unsigned int m_currentSpeedIndex;
	// The current average speed
	float m_avgSpeed;

	/// Current animation playing
	AnimationSetup* m_activeAnim;

	// Whether object is idle
	bool m_idle;

	// If the rand seed was set
	bool m_randSeed;

	// Animations are only played if lod is below this value
	int m_LODToStopAnim;
	// Current lod value
	int m_lod;

	// Offset for the rotation to calculate the direction we are walking to
	float m_rotationOffset;

};


#endif
