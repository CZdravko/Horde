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
// Copyright (C) 2007 Volker Wiendl
//
// ****************************************************************************************
#ifndef GAMEEVENT_H_
#define GAMEEVENT_H_

class GameComponent;
class GameEntity;

#include <string>
#include <cstring>
#include <vector>
//#include <assert.h>

#include <GameEngine/utMath.h>
#include <stdlib.h>
#include <map>

using namespace Horde3D;
/** \addtogroup GameEngineCore
 * @{
 */

/**
 * \brief Generic data class for a GameEvent
 *
 * @author Volker Wiendl
 * @date Mai 2008
 */
class GameEventData {
public:
	enum TypeID {
		INVALID = -1, BOOLEAN, INT, FLOAT, DOUBLE, STRING, ARRAY, CUSTOM
	};

	explicit GameEventData() :
			m_typeID(INVALID), m_size(0), m_owner(false) {
		m_data.ptr = 0;
	}

	explicit GameEventData(int value) :
			m_typeID(INT), m_size(0), m_owner(false) {
		m_data.i = value;
	}

	explicit GameEventData(float value) :
			m_typeID(FLOAT), m_size(0), m_owner(false) {
		m_data.f = value;
	}

	explicit GameEventData(double value) :
			m_typeID(DOUBLE), m_size(0), m_owner(false) {
		m_data.d = value;
	}

	explicit GameEventData(const char* data) :
			m_typeID(STRING), m_size(0), m_owner(false) {
		m_data.s = data;
	}

	explicit GameEventData(bool value) :
			m_typeID(BOOLEAN), m_size(0), m_owner(false) {
		m_data.b = value;
	}

	explicit GameEventData(float* data, size_t elements) :
			m_typeID(ARRAY), m_size(sizeof(float) * elements), m_owner(false) {
		m_data.ptr = data;
	}

	explicit GameEventData(char* data, size_t elements) :
			m_typeID(ARRAY), m_size(sizeof(char) * elements), m_owner(false) {
		m_data.ptr = data;
	}

	GameEventData(void* data) :
			m_typeID(CUSTOM), m_owner(false) {
		m_data.ptr = data;
	}

	//GameEventData(const GameEventData& other) : m_typeID(other.m_typeID), m_size(other.m_size), m_data(other.m_data), m_owner(true)
	//{
	//	switch(m_typeID)
	//	{
	//	case CUSTOM:
	//		other.clone( this );
	//		break;
	//	case STRING:
	//		m_data.s = new char[strlen( other.m_data.s ) + 1];
	//		strcpy( m_data.s, other.m_data.s );
	//		break;
	//	default:
	//		m_owner = false;
	//		break;
	//	}
	//}

	//const GameEventData& operator= (const GameEventData& other)
	//{
	//	if(m_owner)	free(m_data.ptr);
	//	m_size = other.m_size;
	//	m_typeID = other.m_typeID;
	//	switch(m_typeID)
	//	{
	//	case CUSTOM:
	//		other.clone( this );
	//		m_owner = true;
	//		break;
	//	case STRING:
	//		m_data.s = new char[strlen( other.m_data.s ) + 1];
	//		strcpy( m_data.s, other.m_data.s );
	//		m_owner = true;
	//		break;
	//	default:
	//		m_data = other.m_data;
	//		m_owner = false;
	//	}
	//	return *this;
	//}

	virtual ~GameEventData() {
		if (m_owner) {
			switch (m_typeID) {
			case ARRAY:
				free(m_data.ptr);
				break;
			case STRING:
				delete[] m_data.s;
				break;
			default:
				break;
			}
		}
	}

	void* data() {
		switch (m_typeID) {
		case INVALID:
			return 0;
		case BOOLEAN:
			return &m_data.b;
		case INT:
			return &m_data.i;
		case FLOAT:
			return &m_data.f;
		case DOUBLE:
			return &m_data.d;
		default:
			return m_data.ptr;
		}
	}

	virtual GameEventData* clone() const {
		GameEventData* clonedData = new GameEventData(*this);

		switch (m_typeID) {
		case ARRAY:
			clonedData->m_data.ptr = malloc(m_size);
			clonedData->m_owner = true;
			memcpy(clonedData->m_data.ptr, m_data.ptr, m_size);
			break;
		case STRING:
			clonedData->m_data.s = new char[strlen(m_data.s) + 1];
			clonedData->m_owner = true;
			memcpy((char*) clonedData->m_data.s, m_data.s, strlen(m_data.s) + 1);
			break;
		default:
			return clonedData;
		}
		return clonedData;
	}

protected:
	GameEventData(TypeID id) :
			m_typeID(id), m_size(0), m_owner(false) {
		m_data.ptr = 0;
	}

	union Data {
		bool b;
		const char* s;
		float f;
		double d;
		int i;
		void* ptr;
	};

	TypeID m_typeID;
	Data m_data;
	size_t m_size;
	bool m_owner;

private:
	GameEventData(const GameEventData& other) :
			m_typeID(other.m_typeID), m_data(other.m_data), m_owner(false) {
	}
	const GameEventData& operator=(const GameEventData& other) {
		return *this;
	}

};

/**
 * \brief Class to pass events between components
 *
 * @author Volker Wiendl
 * @date Mai 2008
 */
class GameEvent {

public:
	enum EventID {
		E_INVALID,	// Invalid eventID

		///////////////////////////////////////////////////////////////////////////////////////////
		/**
		 * 1. SET EVENTS:
		 *
		 * @send:
		 * If these events are send by a component, it directly wants to modify an attribute of other components,
		 * or cause a specific action of another component.
		 *
		 * @listen:
		 * If a component listens to such an event, it has an attribut that will be modified by the event,
		 * or an action that will be executed by the event,
		 * or it just wants to be notified when such an attribute changes or action occures in another component.
		 *
		 * @eventData: the event data includes the new value for the attribute to set (or to modify it by),
		 * or optional information for the action to be executed
		 *
		 * @allowed by:
		 * Every component can send and/or listen to these events.
		 *
		 * @check?:
		 * You should call checkEvent() before executeEvent().
		 */
		///////////////////////////////////////////////////////////////////////////////////////////
		E_SET_TRANSFORMATION,	/// A transformation change event, containing the new transformation
		E_SET_ROTATION,			/// Set the rotation of the object in world space
		E_SET_TRANSLATION,		/// A translation change event, containing new global translation (Vec3fData)
		E_SET_SCALE,			/// Set the scale of the object
		E_TRANSLATE_LOCAL,		/// Translate Object along it's local axis (using a Vec3fData)
		E_TRANSLATE_GLOBAL,		/// Translate object along the world axis (using a Vec3fData)
		E_ROTATE_LOCAL,			/// Rotate object about it's local axis (using a Vec3fData)
		E_MORPH_TARGET,			/// Sets the model morpher of a Horde3D representation to the provided morph target position
		E_MORPH_TARGET_ANIM,    /// Animates a morph target
		E_PLAY_ANIM,			/// Play an animation
		E_STOP_ANIM,
		E_PAUSE_ANIM,
		E_RESUME_ANIM,
		E_UPDATE_ANIM,			/// Updates a running animation
		E_SET_ANIM_FRAME,		/// Sets the frame of a loaded animation
		E_ATTACH,				/// Attaches a SceneNode to another SceneNode
		E_SET_NODE_PARENT,		/// Sets a new Parent for a SceneNode
		E_SPEAK,				/// Speak a sentence
		E_SET_VOICE,			/// Change the TTS Voice of a TTSComponent
		E_ACTIVATE_CAM,			/// Make the current entity the active camera if it is one
		E_PERFORM_ACTION,		/// Perform an action
		E_GO_TO_ENTITY,			/// Move close to another entity
		E_GO_TO_POSITION,		/// Move to a position
		E_GO_TO_STOPPED,		/// Entity has arrived at the desired location
		E_SET_PROPERTY,			/// Sets a property
		E_ADJUST_PROPERTY,		/// Adjusts a property
		E_INTERACT,				/// Causes an Entity to interact with another Entity.
		E_INTERACT_PARTNER,		/// Informs an Entity that another Entity is interacting with it and causes reactions for the interaction
		E_SET_SOUND_GAIN,		/// Sets the sound gain
		E_SET_SOUND_LOOP,		/// Sets the sound to loop
		E_SET_SOUND_OFFSET,		/// Sets the offset of a sound
		E_SET_SOUND_FILE,		/// Sets a sound file, also starts playing it
		E_SET_SOUND_WITH_USER_DATA, /// Sets a sound already providing the decoded data
		E_PLAY_SOUND,			/// Plays or resumes the currently set sound
		E_PAUSE_SOUND,			/// Pauses a currently playing sound
		E_STOP_SOUND,			/// Stops a currently playing sound
		E_SET_PHONEMES_FILE,	/// Sets a phonemes file
		E_SET_ENABLED,			/// Enables components (wether the component should be rendered/updated or not)
		E_PICKUP,				/// Attaches an entity under the child scene node of another entity
		E_PUTDOWN,				/// Detaches an entity
		E_SET_MOVE_ANIM,		/// Change one of the move animations. @data = pair<string, string>: tag, name of animation
		E_SET_CROWD_TAG,		/// Sets the current crowd sim tag of a particle, so it starts wandering around if the tag > 0
		E_SET_CROWD_IGNORE_FORCES, /// The crowd particle will ignore forces if the particle is not moving
		IK_SOLVE,				/// Computes the IK with the given data and applies it to the chain
		IK_CREATEIKANIM,		/// Computes the IK with the given data, generates a keyframe animation of the chain transformation and stores the anim resource in data->result
		IK_PLAYIKANIM,			/// Plays a previously generated IKAnim
		IK_GAZE,				/// Computes and applies a gaze action to the designated model (agent)
		IK_SETPARAMI,			/// Sets an IK parameter (IK_Param) of type integer
		IK_SETPARAMF,			/// Sets an IK parameter (IK_Param) of type float
		GB_ON_OFF,				/// activates/deactivates character's gaze behaviour
		GB_CONTROL_ON_OFF,		/// activates/deactivates direct control of character via arrow keys
		GB_SEARCH_ON_OFF,		/// activates/deactivates character's searching behaviour
		GB_WAVE_BACK,			/// makes the npc react to a waving event
		GB_RESET,				/// reset all collected data
		SP_START_SCRIPT,		/// starts sn action script
		IB_EYE_FIXATED,			/// informs about an eye fixation
		FACS_SET_EXPRESSION,	/// sets a facial expression /* deprecated */
		E_SEND_SOCKET_DATA,		/// sends data via the socket component
		E_SET_FACS,				/// sets a facial expression
		E_STOP_MOVING,			/// Halts the entity in case it is moving and it can halt. DataType: BOOL
		E_SET_PATH_GOAL,		/// Sets a new goal (world coordinates) for which a path has to be calculated. DataType: Vec3f

		AG_ANIM_PLAY,			/// Loads an animation and plays it back. Param: AgentAnimationData(const char*/int,SourceType,int,float,float,int,char*,char*), returns:  an int
		AG_ANIM_STOP,			/// stops an animation's playback. Param: AgentAnimationData(int), returns: int
		AG_ANIM_SET_EXTENT,		/// Sets an animation's spatial extent. Param: AgentAnimationData(int,float)
		AG_ANIM_SET_SPEED,		/// Sets the animation's playback speed. Param: AgentAnimationData(int,float)
		AG_ANIM_SET_STROKES,	/// Sets the unmber of strokes an animations should perform. Param: AgentAnimationData(int,int)
		AG_ANIM_CLEAR,			/// Clears all animation stages, deleting all loaded animations. No data required
		AG_MOVEMENT,			/// Performs a movement. Param: AgentMovementData(Vec3f/int,Type,const char*,const char*,float,bool), returns:  an int
		AG_MOVEMENT_SET_SPEED,	/// Sets the default movement speed. Param: float
		AG_GAZE,				/// Gazes towards the target. Param: AgentGazeData(Vec3f/int,float,float), returns: int
		AG_DISABLE_GAZE,		/// Disables gaze for an entity. Param: bool		
		AG_GAZE_SET_SPEED,		/// Sets the agent's default gaze speed. Param: float
		AG_HEADNOD,				/// Starts an alternating head motion. Param: AgentGazeData(int,float,int,float,float)
		AG_HEADSHAKE,			/// Starts an alternating head motion. Param: AgentGazeData(int,float,int,float,float)
		AG_FORMATION_ADD,		/// Adds a member to the formation. Param: int
		AG_FORMATION_DEL,		/// Removes a member from the formation. Param: int
		AG_FORMATION_EVENT,		/// Fires an event meant to attract the attention of the members of the formation. Param: int
		AG_FORMATION_REACT,		/// Reacts to the specified event. Param: AgentFormationData(int,int)
		AG_SET_IPDIST,			/// Sets the interpersonal distance constraints of the agent. Param: AgentFormationData(float,float)
		AG_SET_ORIENTCUST,		/// Sets the deviation from the normal orientation of the agent. Param: float
		AG_SET_REPOSANIM,		/// Sets the  repositioning animation name. Param: const char*
		AG_SET_PARAM,			/// Sets the value of a component parameter. Param: AgentConfigParam(int,int/float/const char*) [second value ignored]
		AG_SET_ICON,			/// Sets the name of the icon entity. Param: const char*
		AG_SET_ICONVISIBLE,		/// Sets the visibility of the icon entity. Param: bool
		AG_SET_VISIBLE,			/// Sets the visibility of the agent. Param: bool

		NV_SAY_SENTENCE,		/// sentence to create speaking behavior for
		NV_HEAR_SENTENCE,		/// sentence to create listening behavior for
		NV_HEAD_ANGLES,			/// manual gaze target for the head (using Vec3f)
		NV_EYE_ANGLES,			/// manual gaze target for the eyes (using Vec3f)
		EM_SET_EMOTION,			/// set mood to a specific emotion, e.g. at the beginning of a dialogue (using string)
		EM_OWN_EMOTION,			/// emotional input for a character (using EmotionData)
		//EM_OBSERVE_EMOTION,		/// react to other characters' emotion (using EmotionData)
		EM_OWN_CONTEXT,			/// add new context information to mood calculation(using EmotionData)
		D_INIT_SPEAKERS,		/// initialize speakers
		D_START_DIALOGUE,		/// start playing the parsed dialogue
		CC_CHARACTER_ACTION,	/// character performs an action, snt from BTcomp to CharControl
		CC_ABORT_ACTION,		/// to abort an action that is performed
		CC_REPORT_COMPLETION,	/// reports whether a character action was succesfull, or failed
		BT_ACTIVATE_ON_OFF,		/// activates, or deactivates the BTComponent

		NM_CHANGED,				/// navmesh changed

		KW_INIT,				/// initializes a KinectWorld smart object after the entire scene was loaded
		KW_SET_PROPERTY,		/// sets a property of a KinectWorld smart object (using a pointer to a subclass of KWPropertyData)
		KW_EXECUTE_ACTION,		/// triggers the given action of a KinectWorld smart object (using const char*)
		KW_EXECUTE_DIALOGUE_ACTION,		/// triggers the given dialogue action of a KinectWorld smart object (using const char*)
		KW_APPLY_EFFECT,		/// applies an effect to the KinectWorld smart object (using KWSmartObjectEffect*)

		///////////////////////////////////////////////////////////////////////////////////////////
		/**
		 * 1. GET EVENTS:
		 *
		 * @send:
		 * If these events are send by a component, it wants to get the current value of another component's attribute.
		 *
		 * @listen:
		 * If a component listens to such an event, it has an attribut that it will return to the sender.
		 *
		 * @eventData: Contains a pointer where the value should be copied to.
		 *
		 * @allowed by:
		 * Every component can send such an event, only one should listen to it for setting the value.
		 *
		 * @check?:
		 * You don't need to call checkEvent() before executeEvent().
		 */
		///////////////////////////////////////////////////////////////////////////////////////////
		E_TRANSFORMATION,		/// Copies the current object transformation to the memory buffer provided by the Event (currently float[16])
		E_MESH_DATA,			/// Copies the mesh parameters of a mesh representation in Horde3D to the provided MeshData struct within the event
		E_GET_ACTIVE_CAM,		/// Global event: get the currently active camera entity world id
		E_GET_PROPERTY,			/// Gets a property
		E_GET_INTERACTION_POS,	/// Gets the interaction position
		E_GET_SOUND_DISTANCE,	/// Get the distance of the current sound node to the active listener. @data pointer to float
		E_GET_SOUND_ISPLAYING,	/// Returns whether a sound is currently playing
		E_GET_VISIBILITY,		/// Returns whether the current entity is visible by the active cam
		E_GET_PROJECTION_MATRIX,		/// Returns the current camera projection matrix @data pointer to float[16] for the matrix
		E_GET_SCENEGRAPH_ID,	/// Returns the entity's scenegraph id (hordeID)
		E_GET_ANIM_LENGTH,		/// Get the length of an animation in seconds (using all frames and default speed).
		E_GET_SOCKET_DATA,		/// Gets the data received by the socket component @data: pointer to char* where data is copied to
		E_GET_SOCKET_NEWEST_MSG,		/// Gets the newest message received by the socket component and throws away all older messages @data: pointer to char* where data is copied to
		EM_GET_MOOD_PAD,		/// gets the current mood from the Emotion component as PAD values (using Vec3f)
		D_CURRENT_SENTENCE,		/// get current sentence from Dialogue components
		IK_GETPARAMI,			/// Gets an IK parameter (IK_Param) of type integer
		IK_GETPARAMF,			/// Gets an IK parameter (IK_Param) of type float
		CC_CHECK_CONDITION,		/// Gets sensory input for AI/BT

		AG_ANIM_GET_STATUS,		/// Gets the animation's status. Param: AgentAnimationData(int), returns: int
		AG_ANIM_GET_SPEED,		/// Gets the speed value of an animation. Param: AgentAnimationData(int), returns: float
		AG_ANIM_GET_EXTENT,		/// Gets the spatial extent value of an animation. Param: AgentAnimationData(int), returns: float
		AG_ANIM_GET_STROKES,	/// Gets the stroke repetition value of an animation. Param: AgentAnimationData(int), returns: int
		AG_MOVEMENT_GET_STATUS,	/// Gets the movement's status. Param: AgentMovementData(int), returns: int
		AG_MOVEMENT_GET_SPEED,	/// Gets the default movement speed. Param: AgentMovementData(), returns: float
		AG_GAZE_GET_STATUS,		/// Gets the gaze node's status. Param: AgentGazeData(int), returns: int
		AG_GAZE_GET_SPEED,		/// Gets the agent's default gaze speed. Param: AgentGazeData(), returns: float
		AG_FORMATION_GET_AGENTS,		/// Gets the members of the formation. Param: AgentFormationData(int**), returns: int
		AG_FORMATION_GET_TYPE,	/// Gets the type of the formation. Param: AgentFormationData(), returns: int
		AG_FORMATION_GET_ENTRY,	/// Computes, returns:  an entry point to the formation. Param: AgentFormationData(int), returns: Vec3f
		AG_GET_IPDIST,			/// Gets the interpersonal distance constraints of the agent. Param: AgentFormationData(), returns: float,float
		AG_GET_ORIENTCUST,		/// Gets the deviation value from the normal orientation of the agent. Param: AgentFormationData(), returns: float
		AG_GET_REPOSANIM,		/// Gets the the repositioning animation name. Param: AgentFormationData(), returns: const char*
		AG_GET_PARAM,			/// Gets the value of a component parameter. Param: AgentConfigData(), returns: float/int/const char*

		KW_IS_SMART_OBJECT,		/// Checks if the entity has a KWSmartObjectComponent (using bool*) 
		KW_IS_SELECTABLE,		/// Checks if the entity can be selected by the user (using bool*)
		KW_IS_VISIBLE,			/// Checks if the object is visible on the given camera (using KWVisibleData*) 
		KW_GET_NAME,			/// Gets the name of the KinectWorld smart object (using string*)
		KW_GET_PROPERTY,		/// sets a property of a KinectWorld smart object (using a pointer to a subclass of KWPropertyData)
		KW_GET_ACTIONS,			/// Gets the list of actions stored in a KinectWorld smart object (using vector<KWActionData*>*)
		KW_GET_DIALOGUE_ACTIONS,			/// Gets the list of dialogue actions stored in a KinectWorld smart object (using vector<KWDialogueActionData*>*)

		///////////////////////////////////////////////////////////////////////////////////////////
		/**
		 * 1. CHANGE EVENTS:
		 *
		 * @send/listen:
		 * These events inform that an attribute has changed, or an action has been executed
		 *
		 * @eventData: Contains the new value for the attribute or optional information about the action.
		 *
		 * @allowed by:
		 * Every component can listen to these events.
		 * Only one component should send them (with few exceptions).
		 *
		 * @check?:
		 * You don't need to call checkEvent() before executeEvent().
		 */
		///////////////////////////////////////////////////////////////////////////////////////////
		E_KEY_PRESS,			/// A key has been pressed
		E_ANIM_STOPPED,			/// Indicates that an animation has stopped
		E_SPEAKING_STOPPED,		/// Speak a sentence is finished
		E_SPEAKING_STARTED,		/// Speak a sentence is finished
		E_COLLISION,			/// A collision occured
		E_ACTIVE_CAM_CHANGE,	/// Global event: the current active camera has changed, @data: the cams worldID
		E_WITNESS_ACTION,		/// Witness an action
		GB_KEY_PRESSED,			/// Inidicates a key press event
		E_AILOD_CHANGE,			/// Occurs when the ai lod of an entity has changed, @data: the entity's new lod value
		GL_DRAW,				/// A GL function call. @data: Uses the DataType GLFunction. @allowed by: GLDrawingComponent listens, any componant may send
		GP_STATE_CHANGE,		/// The state of a gamepad has changed
		NV_GAZING_START,		/// informs agent that he is being gazed at
		NV_GAZING_STOP,			/// informs agent that he is no longer being gazed at
		NV_SPEAKING_STOPPED,	/// tells dialogue partner to stop listening
		EM_MOOD_CHANGED,		/// informs an agent that the emotion was updated (sending new mood as Vec3f)
		SP_RHEME_START,			/// rheme started
		SP_THEME_START,			/// theme started
		SP_SPOKEN_WORD,			/// contains currently spoken word
		E_SOUND_STOPPED,		/// the sound of the entity has stopped playing
		E_SOCKET_NEW_DATA,		/// Sends all data received since the last update call
		E_TARGET_REACHED,		/// Informs listeners that a target has been reached.
		E_SCENE_PLAYER_ACTION,	/// Notfies about all scene player events that are not already handled internally

		// TODO: move the following events to the right category
		SP_BOOKMARK,
		SP_TTS_PAUSE,
		SP_TTS_RESUME,
		SP_TTS_SKIP,			///Skip the next n sentences. If n is negative, the speech will skip backwards.

		E_MOUSE_MOVE,			/// for gui component
		E_MOUSE_CLICK,			/// for gui component
		E_GUI_EVENT,			/// for gui component; used for own GUI events
		E_EXITPOINTER,			/// for gui component; used for pointer to m_running

		//EVENTS FOR SUGARCANEISLAND PROJECT
		SCI_NODE_ID,
		SCI_DECISION_ID,
		SCI_DECIDED_ID,
		SCI_QUICKTIME_ID,
		SCI_QUICKTIME_RESULT,
		SCI_BLACKOUT,
		SCI_END,
		SCI_MODE,
		SCI_ACTIONSOUND,

		E_SET_ANIM_BIAS,		/// Sets animation BIAS

		EVENT_COUNT				/// Must be the last entry in the enumeration !!!!
	};
	static GameEvent::EventID convertStringEvent(std::string in) {
		if (in.find("E_INVALID") != std::string::npos)
			return GameEvent::E_INVALID;
		if (in.find("E_SET_TRANSFORMATION") != std::string::npos)
			return GameEvent::E_SET_TRANSFORMATION;
		if (in.find("E_SET_ROTATION") != std::string::npos)
			return GameEvent::E_SET_ROTATION;
		if (in.find("E_SET_TRANSLATION") != std::string::npos)
			return GameEvent::E_SET_TRANSLATION;
		if (in.find("E_SET_SCALE") != std::string::npos)
			return GameEvent::E_SET_SCALE;
		if (in.find("E_TRANSLATE_LOCAL") != std::string::npos)
			return GameEvent::E_TRANSLATE_LOCAL;
		if (in.find("E_TRANSLATE_GLOBAL") != std::string::npos)
			return GameEvent::E_TRANSLATE_GLOBAL;
		if (in.find("E_ROTATE_LOCAL") != std::string::npos)
			return GameEvent::E_ROTATE_LOCAL;
		if (in.find("E_TRANSFORMATION") != std::string::npos)
			return GameEvent::E_TRANSFORMATION;
		if (in.find("E_MESH_DATA") != std::string::npos)
			return GameEvent::E_MESH_DATA;
		if (in.find("E_MORPH_TARGET") != std::string::npos)
			return GameEvent::E_MORPH_TARGET;
		if (in.find("E_MORPH_TARGET_ANIM") != std::string::npos)
			return GameEvent::E_MORPH_TARGET_ANIM;
		if (in.find("E_KEY_PRESS") != std::string::npos)
			return GameEvent::E_KEY_PRESS;
		if (in.find("E_PLAY_ANIM") != std::string::npos)
			return GameEvent::E_PLAY_ANIM;
		if (in.find("E_UPDATE_ANIM") != std::string::npos)
			return GameEvent::E_UPDATE_ANIM;
		if (in.find("E_ANIM_STOPPED") != std::string::npos)
			return GameEvent::E_ANIM_STOPPED;
		if (in.find("E_SET_ANIM_FRAME") != std::string::npos)
			return GameEvent::E_SET_ANIM_FRAME;
		if (in.find("E_ATTACH") != std::string::npos)
			return GameEvent::E_ATTACH;
		if (in.find("E_SET_NODE_PARENT") != std::string::npos)
			return GameEvent::E_SET_NODE_PARENT;
		if (in.find("E_SPEAKING_STOPPED") != std::string::npos)
			return GameEvent::E_SPEAKING_STOPPED;
		if (in.find("E_SPEAK") != std::string::npos)
			return GameEvent::E_SPEAK;
		if (in.find("E_SET_VOICE") != std::string::npos)
			return GameEvent::E_SET_VOICE;
		if (in.find("E_COLLISION") != std::string::npos)
			return GameEvent::E_COLLISION;
		if (in.find("E_ACTIVATE_CAM") != std::string::npos)
			return GameEvent::E_ACTIVATE_CAM;
		if (in.find("E_PERFORM_ACTION") != std::string::npos)
			return GameEvent::E_PERFORM_ACTION;
		if (in.find("E_WITNESS_ACTION") != std::string::npos)
			return GameEvent::E_WITNESS_ACTION;
		if (in.find("E_GO_TO_ENTITY") != std::string::npos)
			return GameEvent::E_GO_TO_ENTITY;
		if (in.find("E_GO_TO_POSITION") != std::string::npos)
			return GameEvent::E_GO_TO_POSITION;
		if (in.find("E_GO_TO_STOPPED") != std::string::npos)
			return GameEvent::E_GO_TO_STOPPED;
		if (in.find("E_SET_PROPERTY") != std::string::npos)
			return GameEvent::E_SET_PROPERTY;
		if (in.find("E_GET_PROPERTY") != std::string::npos)
			return GameEvent::E_GET_PROPERTY;
		if (in.find("E_ADJUST_PROPERTY") != std::string::npos)
			return GameEvent::E_ADJUST_PROPERTY;
		if (in.find("E_INTERACT") != std::string::npos)
			return GameEvent::E_INTERACT;
		if (in.find("E_INTERACT_PARTNER") != std::string::npos)
			return GameEvent::E_INTERACT_PARTNER;
		if (in.find("E_SET_SOUND_GAIN") != std::string::npos)
			return GameEvent::E_SET_SOUND_GAIN;
		if (in.find("E_SET_SOUND_LOOP") != std::string::npos)
			return GameEvent::E_SET_SOUND_LOOP;
		if (in.find("E_SET_SOUND_FILE") != std::string::npos)
			return GameEvent::E_SET_SOUND_FILE;
		if (in.find("E_SET_PHONEMES_FILE") != std::string::npos)
			return GameEvent::E_SET_PHONEMES_FILE;
		if (in.find("E_SET_ENABLED") != std::string::npos)
			return GameEvent::E_SET_ENABLED;
		if (in.find("E_PICKUP") != std::string::npos)
			return GameEvent::E_PICKUP;
		if (in.find("E_AILOD_CHANGE") != std::string::npos)
			return GameEvent::E_AILOD_CHANGE;
		if (in.find("E_ACTIVE_CAM_CHANGE") != std::string::npos)
			return GameEvent::E_ACTIVE_CAM_CHANGE;
		if (in.find("GL_DRAW") != std::string::npos)
			return GameEvent::GL_DRAW;
		if (in.find("GP_STATE_CHANGE") != std::string::npos)
			return GameEvent::GP_STATE_CHANGE;
		if (in.find("SCI_NODE_ID") != std::string::npos)
			return GameEvent::SCI_NODE_ID;
		if (in.find("SCI_DECISION_ID") != std::string::npos)
			return GameEvent::SCI_DECISION_ID;
		if (in.find("SCI_DECIDED_ID") != std::string::npos)
			return GameEvent::SCI_DECIDED_ID;
		if (in.find("SCI_QUICKTIME_ID") != std::string::npos)
			return GameEvent::SCI_QUICKTIME_ID;
		if (in.find("SCI_QUICKTIME_RESULT") != std::string::npos)
			return GameEvent::SCI_QUICKTIME_RESULT;
		if (in.find("SCI_BLACKOUT") != std::string::npos)
			return GameEvent::SCI_BLACKOUT;
		if (in.find("SCI_END") != std::string::npos)
			return GameEvent::SCI_END;
		if (in.find("SCI_MODE") != std::string::npos)
			return GameEvent::SCI_MODE;
		if (in.find("SCI_ACTIONSOUND") != std::string::npos)
			return GameEvent::SCI_ACTIONSOUND;
		if (in.find("E_SET_PATH_GOAL") != std::string::npos)
			return GameEvent::E_SET_PATH_GOAL;
		if (in.find("E_TARGET_REACHED") != std::string::npos)
			return GameEvent::E_TARGET_REACHED;
		if (in.find("E_STOP_MOVING") != std::string::npos)
			return GameEvent::E_STOP_MOVING;
		if (in.find("E_MOUSE_MOVE") != std::string::npos)
			return GameEvent::E_MOUSE_MOVE;
		if (in.find("E_MOUSE_CLICK") != std::string::npos)
			return GameEvent::E_MOUSE_CLICK;
		if (in.find("E_GUI_EVENT") != std::string::npos)
			return GameEvent::E_GUI_EVENT;
		if (in.find("E_EXITPOINTER") != std::string::npos)
			return GameEvent::E_EXITPOINTER;
		return GameEvent::EVENT_COUNT;

	}
	GameEvent(EventID id, GameEventData* data, GameComponent* sender) :
			m_id(id), m_pData(data), m_sender(sender), m_owner(false) {
	}

	GameEvent(EventID id, const GameEventData& data, GameComponent* sender) :
			m_id(id), m_pData(data.clone()), m_sender(sender), m_owner(true) {
	}

	~GameEvent() {
		if (m_owner)
			delete m_pData;
	}

	const GameEvent& operator=(const GameEvent& other) {
		if (m_owner)
			delete m_pData;
		m_pData = other.m_pData ? other.m_pData->clone() : 0;
		m_owner = true;
		m_id = other.m_id;
		m_sender = other.m_sender;
		return *this;
	}

	GameEvent(const GameEvent& other) :
			m_id(other.m_id), m_pData(other.m_pData ? other.m_pData->clone() : 0x0), m_sender(other.m_sender), m_owner(true) {

	}

	const EventID id() const {
		return m_id;
	}
	void* data() const {
		return m_pData ? m_pData->data() : 0x0;
	}
	const GameComponent* sender() const {
		return m_sender;
	}

private:

	EventID m_id;
	bool m_owner;
	GameEventData* m_pData;
	const GameComponent* m_sender;
};

/**
 * Container for Mesh Information provided by E_MESH_DATA
 */
class MeshData: public GameEventData {
public:

	MeshData() :
			GameEventData(CUSTOM), NumVertices(0), NumTriangleIndices(0), VertexBase(0), VertRStart(0), TriangleBase16(0), TriangleBase32(0), TriangleMode(4) {
		m_data.ptr = this;
	}

	~MeshData() {
		if (m_owner) {
			delete[] VertexBase;
			if (TriangleBase16)
				delete[] TriangleBase16;
			if (TriangleBase32)
				delete[] TriangleBase16;
		}
	}

	unsigned int NumVertices, NumTriangleIndices;

	float* VertexBase;

	unsigned int VertRStart;

	unsigned int* TriangleBase32;
	unsigned short* TriangleBase16;

	unsigned int TriangleMode;

	virtual GameEventData* clone() const {
		MeshData* clonedData = new MeshData();
		clonedData->m_owner = true;
		clonedData->VertexBase = new float[NumVertices];
		memcpy((void*) clonedData->VertexBase, VertexBase, sizeof(float) * NumVertices);

		if (TriangleBase32) {
			clonedData->TriangleBase32 = new unsigned int[NumTriangleIndices];
			memcpy((void*) clonedData->TriangleBase32, TriangleBase32, sizeof(unsigned int) * NumVertices);
		} else {
			clonedData->TriangleBase16 = new unsigned short[NumTriangleIndices];
			memcpy((void*) clonedData->TriangleBase16, TriangleBase16, sizeof(unsigned short) * NumVertices);
		}

		clonedData->NumTriangleIndices = NumTriangleIndices;
		clonedData->NumVertices = NumVertices;
		clonedData->VertRStart = VertRStart;
		clonedData->TriangleMode = TriangleMode;
		return clonedData;
	}

};

/**
 * \brief Container for MorphTarget data used by a GameEvent
 *
 * @author Volker Wiendl
 * @date  Jun 2008
 *
 */
class MorphTarget: public GameEventData {
public:
	MorphTarget(const char* name, float value) :
			GameEventData(CUSTOM), Name(name), Value(value) {
		m_data.ptr = this;
	}

	MorphTarget(const MorphTarget& copy) :
			GameEventData(CUSTOM), Value(copy.Value) {
		m_data.ptr = this;
		m_owner = true;
		const size_t len = copy.Name ? strlen(copy.Name) : 0;
		Name = new char[len + 1];
		memcpy((char*) Name, copy.Name, len);
		const_cast<char*>(Name)[len] = '\0';
	}

	~MorphTarget() {
		if (m_owner)
			delete[] Name;
	}

	GameEventData* clone() const {
		return new MorphTarget(*this);
	}

	const char* Name; // Maybe this is not a good data type, since this requires the given name pointer to be valid until execute event has been finished
	float Value;
};

/**
 * \brief Container for MorphTargetAnimation data used by a GameEvent
 *
 * @author Nikolaus Bee
 * @date  Aug 2008
 *
 */
class MorphTargetAnimation: public GameEventData {
public:
	MorphTargetAnimation(const char* name, float toWeight, float duration) :
			GameEventData(CUSTOM), Name(name), ToWeight(toWeight), Duration(duration) {
		m_data.ptr = this;
	}

	MorphTargetAnimation(const MorphTargetAnimation& copy) :
			GameEventData(CUSTOM), ToWeight(copy.ToWeight), Duration(copy.Duration) {
		m_data.ptr = this;
		m_owner = true;
		const size_t len = copy.Name ? strlen(copy.Name) : 0;
		Name = new char[len + 1];
		memcpy((char*) Name, copy.Name, len);
		const_cast<char*>(Name)[len] = '\0';
	}

	~MorphTargetAnimation() {
		if (m_owner)
			delete[] Name;
	}

	GameEventData* clone() const {
		return new MorphTargetAnimation(*this);
	}

	const char* Name; // Maybe this is not a good data type, since this requires the given name pointer to be valid until execute event has been finished
	float ToWeight;
	float Duration;
};

/**
 */
//struct CollisionData
//{
//	CollisionData(GameEntity* obj1, GameEntity* obj2) : Obj1(obj1), Obj2(obj2) {}
//	GameEntity* Obj1;
//	GameEntity* Obj2;
//};
/**
 * Container for an Animation configuration used by a GameEvent
 */
class AnimationSetup: public GameEventData {
public:
	AnimationSetup(const char* animation, int stage, float speed, float duration, float weight, float timeoffset, int layer = 0, float startFrame = 0) :
			GameEventData(CUSTOM), Animation(animation), Stage(stage), Speed(speed), Duration(duration), Weight(weight), TimeOffset(timeoffset), Layer(layer), JobID(0), StartFrame(startFrame) {
		m_data.ptr = this;
	}

	AnimationSetup(const AnimationSetup& copy) :
			GameEventData(CUSTOM), Stage(copy.Stage), Speed(copy.Speed), Duration(copy.Duration), Weight(copy.Weight), TimeOffset(copy.TimeOffset), Layer(copy.Layer), JobID(copy.JobID), StartFrame(
					copy.StartFrame) {
		m_data.ptr = this;
		m_owner = true;
		const size_t len = copy.Animation ? strlen(copy.Animation) : 0;
		Animation = new char[len + 1];
		memcpy((char*) Animation, copy.Animation, len);
		const_cast<char*>(Animation)[len] = '\0';
	}

	~AnimationSetup() {
		if (m_owner)
			delete[] Animation;
	}

	GameEventData* clone() const {
		return new AnimationSetup(*this);
	}

	const char* Animation;
	const int Stage;
	float Speed;
	float Duration;
	const float Weight;
	const float TimeOffset;
	const int Layer;
	const float StartFrame;
	int JobID;

};

/**
 * \brief Container for parameters to update an Animation using a GameEvent
 *
 * @author Volker Wiendl
 * @date  Jun 2008
 *
 */
class AnimationUpdate: public GameEventData {
public:
	AnimationUpdate(const int jobID, const int paramType, const float value, const float timeOffset) :
			GameEventData(CUSTOM), JobID(jobID), ParamType(paramType), Value(value), TimeOffset(timeOffset) {
		m_data.ptr = this;
	}

	AnimationUpdate(const AnimationUpdate& copy) :
			GameEventData(CUSTOM), JobID(copy.JobID), ParamType(copy.ParamType), Value(copy.Value), TimeOffset(copy.TimeOffset) {
		m_data.ptr = this;
	}

	GameEventData* clone() const {
		return new AnimationUpdate(*this);
	}

	const AnimationUpdate& operator=(const AnimationUpdate& other) {
		JobID = other.JobID;
		ParamType = other.ParamType;
		Value = other.Value;
		TimeOffset = other.TimeOffset;
		return *this;
	}

	unsigned int JobID;
	int ParamType;
	float Value;
	float TimeOffset;

};

class AnimationBias: public GameEventData {
public:
	AnimationBias(const int jobID, float biasTransX, float biasTransY, float biasTransZ, float biasRotX, float biasRotY, float biasRotZ, float biasRotW) :
			GameEventData(CUSTOM), JobID(jobID), BiasTransX(biasTransX), BiasTransY(biasTransY), BiasTransZ(biasTransZ),
			BiasRotX(biasRotX), BiasRotY(biasRotY), BiasRotZ(biasRotZ), BiasRotW(biasRotW){
		m_data.ptr = this;
	}

	AnimationBias(const AnimationBias& copy) :
			GameEventData(CUSTOM), JobID(copy.JobID), BiasTransX(copy.BiasTransX), BiasTransY(copy.BiasTransY), BiasTransZ(copy.BiasTransZ),
			BiasRotX(copy.BiasRotX), BiasRotY(copy.BiasRotY), BiasRotZ(copy.BiasRotZ), BiasRotW(copy.BiasRotW) {
		m_data.ptr = this;
	}

	GameEventData* clone() const {
		return new AnimationBias(*this);
	}

	const AnimationBias& operator=(const AnimationBias& other) {
		JobID = other.JobID;
		BiasTransX = other.BiasTransX;
		BiasTransY = other.BiasTransY;
		BiasTransZ = other.BiasTransZ;
		BiasRotX = other.BiasRotX;
		BiasRotY = other.BiasRotY;
		BiasRotZ = other.BiasRotZ;
		BiasRotW = other.BiasRotW;
		return *this;
	}

	unsigned int JobID;
	float BiasTransX;
	float BiasTransY;
	float BiasTransZ;
	float BiasRotX;
	float BiasRotY;
	float BiasRotZ;
	float BiasRotW;

};

class SetAnimFrame: public GameEventData {
public:
	SetAnimFrame(int stage, float time, float weight) :
			GameEventData(CUSTOM), Stage(stage), Time(time), Weight(weight) {
		m_data.ptr = this;
	}

	const int Stage;
	const float Time;
	const float Weight;

	GameEventData* clone() const {
		return new SetAnimFrame(Stage, Time, Weight);
	}
};

class Property: public GameEventData {

public:
	Property(const char* name, double value) :
			GameEventData(CUSTOM), Name(name), Value(value) {
		m_data.ptr = this;
	}

	Property(const Property& copy) :
			GameEventData(CUSTOM), Value(copy.Value) {
		m_data.ptr = this;
		m_owner = true;
		const size_t lenName = copy.Name ? strlen(copy.Name) : 0;
		Name = new char[lenName + 1];
		memcpy((char*) Name, copy.Name, lenName);
		const_cast<char*>(Name)[lenName] = '\0';

	}

	~Property() {
		if (m_owner) {
			delete[] Name;
		}
	}

	const char* Name;
	double Value;

	GameEventData* clone() const {
		return new Property(*this);

	}
};

class GoToPosition: public GameEventData {

public:
	GoToPosition(float x, float y, float z, float speed) :
			GameEventData(CUSTOM), X(x), Y(y), Z(z), Speed(speed), Result(-1) {
		m_data.ptr = this;
	}

	GoToPosition(float x, float y, float z) :
			GameEventData(CUSTOM), X(x), Y(y), Z(z), Speed(0.0f), Result(-1) {
		m_data.ptr = this;
	}

	GoToPosition(const GoToPosition& copy) :
			GameEventData(CUSTOM), X(copy.X), Y(copy.Y), Z(copy.Z), Speed(copy.Speed), Result(copy.Result) {
		m_data.ptr = this;
		m_owner = true;
	}

	~GoToPosition() {
	}

	float X;
	float Y;
	float Z;
	float Speed;
	int Result;

	GameEventData* clone() const {
		return new GoToPosition(*this);
	}

};

class SuccessData: public GameEventData {

public:
	SuccessData(int id, bool successful) :
			GameEventData(CUSTOM), Id(id), Successful(successful) {
		m_data.ptr = this;
	}

	SuccessData(const SuccessData& copy) :
			GameEventData(CUSTOM), Id(copy.Id), Successful(copy.Successful) {
		m_data.ptr = this;
		m_owner = true;
	}

	~SuccessData() {
		if (m_owner) {

		}
	}

	int Id;
	bool Successful;

	GameEventData* clone() const {
		return new SuccessData(*this);

	}

};

class GoTo: public GameEventData {

public:
	GoTo(const char* targetID, const char* animName, float speed) :
			GameEventData(CUSTOM), TargetID(targetID), AnimName(animName), Speed(speed), Result(-1) {
		m_data.ptr = this;
	}

	GoTo(const char* targetID, float speed) :
			GameEventData(CUSTOM), TargetID(targetID), AnimName(0), Speed(speed), Result(-1) {
		m_data.ptr = this;
	}

	GoTo(const char* targetID) :
			GameEventData(CUSTOM), TargetID(targetID), AnimName(0), Speed(0.0f), Result(-1) {
		m_data.ptr = this;
	}

	GoTo(const GoTo& copy) :
			GameEventData(CUSTOM), Speed(copy.Speed), Result(copy.Result) {
		m_data.ptr = this;
		m_owner = true;

		const size_t lenAnimName = copy.AnimName ? strlen(copy.AnimName) : 0;
		AnimName = new char[lenAnimName + 1];
		memcpy((char*) AnimName, copy.AnimName, lenAnimName);
		const_cast<char*>(AnimName)[lenAnimName] = '\0';

		const size_t lenTarget = copy.TargetID ? strlen(copy.TargetID) : 0;
		TargetID = new char[lenTarget + 1];
		memcpy((char*) TargetID, copy.TargetID, lenTarget);
		const_cast<char*>(TargetID)[lenTarget] = '\0';
	}

	void setAnimName(const char* name) {
		const size_t lenAnimName = strlen(name);
		AnimName = new char[lenAnimName + 1];
		memcpy((char*) AnimName, name, lenAnimName);
		const_cast<char*>(AnimName)[lenAnimName] = '\0';
	}

	~GoTo() {
		if (m_owner) {
			delete[] TargetID;
			delete[] AnimName;
		}
	}

	const char* TargetID;
	const char* AnimName;
	float Speed;
	int Result;

	GameEventData* clone() const {
		return new GoTo(*this);

	}

};

class CharacterAction: public GameEventData {

public:
	CharacterAction(const char* action, const char* parameters, unsigned int id) :
			GameEventData(CUSTOM), Action(action), Parameters(parameters), Id(id) {
		m_data.ptr = this;
	}

	CharacterAction(const CharacterAction& copy) :
			GameEventData(CUSTOM), Id(copy.Id) {
		m_data.ptr = this;
		m_owner = true;

		const size_t lenAction = copy.Action ? strlen(copy.Action) : 0;
		Action = new char[lenAction + 1];
		memcpy((char*) Action, copy.Action, lenAction);
		const_cast<char*>(Action)[lenAction] = '\0';

		const size_t lenParameters = copy.Parameters ? strlen(copy.Parameters) : 0;
		Parameters = new char[lenParameters + 1];
		memcpy((char*) Parameters, copy.Parameters, lenParameters);
		const_cast<char*>(Parameters)[lenParameters] = '\0';
	}

	~CharacterAction() {
		if (m_owner) {
			delete[] Action;
			delete[] Parameters;
		}
	}

	const char* Action;
	const char* Parameters;
	unsigned int Id;

	GameEventData* clone() const {
		return new CharacterAction(*this);

	}

};

class CheckCondition: public GameEventData {

public:
	CheckCondition(const char* condition, const char* parameters) :
			GameEventData(CUSTOM), Condition(condition), Parameters(parameters) {
		m_data.ptr = this;
	}

	CheckCondition(const CheckCondition& copy) :
			GameEventData(CUSTOM), ConditionValue(copy.ConditionValue) {
		m_data.ptr = this;
		m_owner = true;

		const size_t lenAction = copy.Condition ? strlen(copy.Condition) : 0;
		Condition = new char[lenAction + 1];
		memcpy((char*) Condition, copy.Condition, lenAction);
		const_cast<char*>(Condition)[lenAction] = '\0';

		const size_t lenParameters = copy.Parameters ? strlen(copy.Parameters) : 0;
		Parameters = new char[lenParameters + 1];
		memcpy((char*) Parameters, copy.Parameters, lenParameters);
		const_cast<char*>(Parameters)[lenParameters] = '\0';
	}

	~CheckCondition() {
		if (m_owner) {
			delete[] Condition;
			delete[] Parameters;
		}
	}

	const char* Condition;
	const char* Parameters;
	bool ConditionValue;

	GameEventData* clone() const {
		return new CheckCondition(*this);
	}
};

class Attach: public GameEventData {

public:
	Attach(const char* childName, unsigned int entityID, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz) :
			GameEventData(CUSTOM), Child(childName), EntityID(entityID), Tx(tx), Ty(ty), Tz(tz), Rx(rx), Ry(ry), Rz(rz), Sx(sx), Sy(sy), Sz(sz) {
		m_data.ptr = this;
	}

	Attach(const char* childName, unsigned int id) :
			GameEventData(CUSTOM), Child(childName), EntityID(id), Tx(0), Ty(0), Tz(0), Rx(0), Ry(0), Rz(0), Sx(1), Sy(1), Sz(1) {
		m_data.ptr = this;
	}

	Attach(const Attach& copy) :
			GameEventData(CUSTOM), Tx(copy.Tx), Ty(copy.Ty), Tz(copy.Tz), Rx(copy.Rx), Ry(copy.Ry), Rz(copy.Rz), Sx(copy.Sx), Sy(copy.Sy), Sz(copy.Sz), EntityID(copy.EntityID) {
		m_data.ptr = this;
		m_owner = true;

		const size_t lenChildname = copy.Child ? strlen(copy.Child) : 0;
		Child = new char[lenChildname + 1];
		memcpy((char*) Child, copy.Child, lenChildname);
		const_cast<char*>(Child)[lenChildname] = '\0';
	}

	~Attach() {
		if (m_owner) {
			delete[] Child;
		}
	}

	const char* Child;
	unsigned int EntityID;
	float Tx;
	float Ty;
	float Tz;
	float Rx;
	float Ry;
	float Rz;
	float Sx;
	float Sy;
	float Sz;

	GameEventData* clone() const {
		return new Attach(*this);

	}
};

/**
 * \brief Container for parameters of the IK component
 *
 * @author Ionut Damian
 * @date  April 2010
 *
 */
class IKData: public GameEventData {
public:
	///Constructor for gaze data
	IKData(float TargetX, float TargetY, float TargetZ, bool MoveLEye, bool MoveREye, bool MoveHead, int Head_pitch, bool Simulate = false) :
			GameEventData(CUSTOM), endEffectorName(0), stopName(0), targetX(TargetX), targetY(TargetY), targetZ(TargetZ), moveLEye(MoveLEye), moveREye(MoveREye), moveHead(MoveHead), head_pitch(
					Head_pitch), result(-1), simulate(Simulate) {
		m_data.ptr = this;
	}
	///Constructor for ik data
	IKData(const char* EndEffectorName, const char* StopName, float TargetX, float TargetY, float TargetZ, bool Simulate = false) :
			GameEventData(CUSTOM), endEffectorName(EndEffectorName), stopName(StopName), targetX(TargetX), targetY(TargetY), targetZ(TargetZ), result(-1), simulate(Simulate) {
		m_data.ptr = this;
	}
	///Constructor for ik anim data
	IKData(int AnimStage, float AnimWeight, float AnimSpeed) :
			GameEventData(CUSTOM), result(-1), animStage(AnimStage), animSpeed(AnimSpeed), animWeight(AnimWeight) {
		m_data.ptr = this;
	}

	///Constructor for ik param data (look up parameters in IK_Param)
	IKData(int IK_Parameter, int value) :
			GameEventData(CUSTOM), endEffectorName(0), stopName(0), ikparam(IK_Parameter), ikparam_valuei(value) {
		m_data.ptr = this;
	}
	///Constructor for ik param data (look up parameters in IK_Param)
	IKData(int IK_Parameter, float value) :
			GameEventData(CUSTOM), endEffectorName(0), stopName(0), ikparam(IK_Parameter), ikparam_valuef(value) {
		m_data.ptr = this;
	}

	IKData(const IKData& copy) :
			GameEventData(CUSTOM), targetX(copy.targetX), targetY(copy.targetY), targetZ(copy.targetZ), result(copy.result), simulate(copy.simulate), ikparam(copy.ikparam), ikparam_valuei(
					copy.ikparam_valuei), ikparam_valuef(copy.ikparam_valuef), animStage(copy.animStage), animWeight(copy.animWeight), animSpeed(copy.animSpeed) {
		m_data.ptr = this;
		m_owner = true;

		if (copy.endEffectorName != 0) {
			const size_t lenEEName = copy.endEffectorName ? strlen(copy.endEffectorName) : 0;
			endEffectorName = new char[lenEEName + 1];
			memcpy((char*) endEffectorName, copy.endEffectorName, lenEEName);
			const_cast<char*>(endEffectorName)[lenEEName] = '\0';
		} else
			endEffectorName = 0;

		if (copy.stopName != 0) {
			const size_t lenStopName = copy.stopName ? strlen(copy.stopName) : 0;
			stopName = new char[lenStopName + 1];
			memcpy((char*) stopName, copy.stopName, lenStopName);
			const_cast<char*>(stopName)[lenStopName] = '\0';
		} else
			stopName = 0;
	}

	~IKData() {
		if (m_owner) {
			if (endEffectorName != 0)
				delete[] endEffectorName;
			if (stopName != 0)
				delete[] stopName;
		}
	}

	GameEventData* clone() const {
		return new IKData(*this);
	}

	const char* endEffectorName;
	const char* stopName;
	float targetX, targetY, targetZ;
	bool moveLEye, moveREye, moveHead;
	int head_pitch;
	bool simulate;
	int animStage;
	float animSpeed;
	float animWeight;
	int result;
	int ikparam;
	float ikparam_valuef;
	int ikparam_valuei;
};

/**
 * \brief Container for parameters of AgentComponent/Movement
 *
 * @author Ionut Damian
 * @date  March 2011
 *
 */
class AgentMovementData: public GameEventData {
public:
	enum Type {
		GoTo_Location = 0, GoTo_Entity, GoTo_Formation, Locomotion, Orientation, Rotation,
	};

	///Data container constructor for AG_MOVEMENT
	///@param target the target vector
	///@param type the movement type
	///@param speed the movement speed (use -1 for agent default)
	///@param putInQueue doesn't start the movement immediately but places it in a queue.
	///@param orientAnimName the name, as defined in the animation lexicon, of the animation to be played during the orientation (use 0 for default agent walk animation)
	///@param orientAnimName the name, as defined in the animation lexicon, of the animation to be played during the locomotion (use 0 for default agent walk animation)
	AgentMovementData(Vec3f target, Type type, float speed, bool putInQueue, const char* walkAnimName, const char* orientAnimName) :
			GameEventData(CUSTOM), m_targetV(target), m_type(type), m_walkAnimName(walkAnimName), m_orientAnimName(orientAnimName), m_speed(speed), m_putInQueue(putInQueue), m_return(-1) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_MOVEMENT
	///@param target the target entity id
	///@param type the movement type
	///@param speed the movement speed (use -1 for agent default)
	///@param putInQueue doesn't start the movement immediately but places it in a queue.
	///@param orientAnimName the name, as defined in the animation lexicon, of the animation to be played during the orientation (use 0 for default agent walk animation)
	///@param orientAnimName the name, as defined in the animation lexicon, of the animation to be played during the locomotion (use 0 for default agent walk animation)
	AgentMovementData(int target, Type type, const char* walkAnimName, const char* orientAnimName, float speed, bool putInQueue) :
			GameEventData(CUSTOM), m_targetI(target), m_type(type), m_walkAnimName(walkAnimName), m_orientAnimName(orientAnimName), m_speed(speed), m_putInQueue(putInQueue), m_return(-1) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_MOVEMENT_GET_STATUS
	///@param movementID the id of the movement, as returned by a movement call-up
	AgentMovementData(int movementID) :
			GameEventData(CUSTOM), m_movementID(movementID), m_return(-1), m_walkAnimName(0), m_orientAnimName(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_MOVEMENT_GET_SPEED
	AgentMovementData() :
			GameEventData(CUSTOM), m_returnF(-1), m_walkAnimName(0), m_orientAnimName(0) {
		m_data.ptr = this;
	}

	AgentMovementData(const AgentMovementData& copy) :
			GameEventData(CUSTOM), m_targetV(copy.m_targetV), m_targetI(copy.m_targetI), m_type(copy.m_type), m_return(copy.m_return), m_movementID(copy.m_movementID), m_speed(copy.m_speed), m_putInQueue(
					copy.m_putInQueue) {
		m_data.ptr = this;
		m_owner = true;

		if (copy.m_walkAnimName != 0) {
			const size_t len = strlen(copy.m_walkAnimName);
			m_walkAnimName = new char[len + 1];
			memcpy((char*) m_walkAnimName, copy.m_walkAnimName, len);
			const_cast<char*>(m_walkAnimName)[len] = '\0';
		} else
			m_walkAnimName = 0;

		if (copy.m_orientAnimName != 0) {
			const size_t len = strlen(copy.m_orientAnimName);
			m_orientAnimName = new char[len + 1];
			memcpy((char*) m_orientAnimName, copy.m_orientAnimName, len);
			const_cast<char*>(m_orientAnimName)[len] = '\0';
		} else
			m_orientAnimName = 0;

	}

	~AgentMovementData() {
		if (m_owner) {
			if (m_walkAnimName != 0)
				delete[] m_walkAnimName;
			if (m_orientAnimName != 0)
				delete[] m_orientAnimName;
		}
	}

	GameEventData* clone() const {
		return new AgentMovementData(*this);
	}

	int getReturnInt() {
		return m_return;
	}

	float getReturnFloat() {
		return m_returnF;
	}

	Vec3f m_targetV;
	int m_targetI;
	Type m_type;
	float m_speed;
	bool m_putInQueue;
	int m_movementID;
	const char* m_walkAnimName;
	const char* m_orientAnimName;
	int m_return;
	float m_returnF;
};

/**
 * \brief Container for parameters of AgentComponent/Animation
 *
 * @author Ionut Damian
 * @date  March 2011
 *
 */
class AgentAnimationData: public GameEventData {
public:
	enum SourceType {
		AnimationName = 0, AnimationResource, AnimationFile, AnimationID,
	};

	///Data container constructor for AG_ANIM_PLAY
	///@param source the filename or the name of the animation
	///@param sourceType the type of the sources (SourceType)
	///@param speed desired playback speed of the animation (0 = still, 1 = normal, 2= double, ..), use -1 for agent default
	///@param spatialExtent the spatial extent of the animation (0 = still, 0.5 = half, 1 = normal), use -1 for agent default
	///@param strokeRepetitions number of stroke repetitions the animation should perform (0 = normal playback, 1 = one extra stroke is performed, ..), use -1 for agent default
	///@param startNode the name of the node that represents the starting node of the chain to be animated (ex: startNode=Bip01_L_Clavicle willl result in animating only the left arm). Leave 0 for whole body.
	///@param syncWord TTS word the animation start should be synchronized with
	AgentAnimationData(const char* source, SourceType sourceType, int animType, float speed, float spatialExtent, int strokeRepetitions, char* startNode, char* syncWord) :
			GameEventData(CUSTOM), m_sourceS(source), m_sourceType(sourceType), m_animType(animType), m_startNode(startNode), m_syncWord(syncWord), m_speed(speed), m_spatialExtent(spatialExtent), m_strokeReps(
					strokeRepetitions), m_returnI(-1) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_ANIM_PLAY
	///@param source the animation id or the resource id of the animation
	///@param sourceType the type of the sources (SourceType)
	///@param speed desired playback speed of the animation (0 = still, 1 = normal, 2= double, ..), use -1 for agent default
	///@param spatialExtent the spatial extent of the animation (0 = still, 0.5 = half, 1 = normal), use -1 for agent default
	///@param strokeRepetitions number of stroke repetitions the animation should perform (0 = normal playback, 1 = one extra stroke is performed, ..), use -1 for agent default
	///@param startNode the name of the node that represents the starting node of the chain to be animated (ex: startNode=Bip01_L_Clavicle willl result in animating only the left arm). Leave 0 for whole body.
	///@param syncWord TTS word the animation start should be synchronized with
	AgentAnimationData(int source, SourceType sourceType, int animType, float speed, float spatialExtent, int strokeRepetitions, char* startNode, char* syncWord) :
			GameEventData(CUSTOM), m_sourceI(source), m_sourceType(sourceType), m_animType(animType), m_startNode(startNode), m_syncWord(syncWord), m_speed(speed), m_spatialExtent(spatialExtent), m_strokeReps(
					strokeRepetitions), m_returnI(-1), m_sourceS(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_ANIM_STOP, AG_ANIM_GET_STATUS, AG_ANIM_GET_SPEED, AG_ANIM_GET_EXTENT, AG_ANIM_GET_STROKES
	///@param playbackID the playback id of the animation (use -1 to get agent default)
	AgentAnimationData(int playbackID) :
			GameEventData(CUSTOM), m_playbackID(playbackID), m_returnI(-1), m_returnF(-1), m_sourceS(0), m_startNode(0), m_syncWord(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_ANIM_SET_STROKES
	///@param playbackID the playback id of the animation (use -1 to set agent default)
	///@param value the the value we want to set
	AgentAnimationData(int playbackID, int value) :
			GameEventData(CUSTOM), m_playbackID(playbackID), m_valueI(value), m_returnI(-1), m_sourceS(0), m_startNode(0), m_syncWord(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_ANIM_SET_EXTENT, AG_ANIM_SET_SPEED
	///@param playbackID the playback id of the animation (use -1 to set agent default)
	///@param value the the value we want to set
	AgentAnimationData(int playbackID, float value) :
			GameEventData(CUSTOM), m_playbackID(playbackID), m_valueF(value), m_returnI(-1), m_sourceS(0), m_startNode(0), m_syncWord(0) {
		m_data.ptr = this;
	}

	AgentAnimationData(const AgentAnimationData& copy) :
			GameEventData(CUSTOM), m_sourceI(copy.m_sourceI), m_sourceType(copy.m_sourceType), m_animType(copy.m_animType), m_speed(copy.m_speed), m_spatialExtent(copy.m_spatialExtent), m_strokeReps(
					copy.m_strokeReps), m_returnI(copy.m_returnI), m_returnF(copy.m_returnF), m_valueF(copy.m_valueF), m_valueI(copy.m_valueI), m_playbackID(copy.m_playbackID) {
		m_data.ptr = this;
		m_owner = true;

		if (copy.m_sourceS != 0) {
			const size_t len = strlen(copy.m_sourceS);
			m_sourceS = new char[len + 1];
			memcpy((char*) m_sourceS, copy.m_sourceS, len);
			const_cast<char*>(m_sourceS)[len] = '\0';
		} else
			m_sourceS = 0;

		if (copy.m_startNode != 0) {
			const size_t len = strlen(copy.m_startNode);
			m_startNode = new char[len + 1];
			memcpy((char*) m_startNode, copy.m_startNode, len);
			const_cast<char*>(m_startNode)[len] = '\0';
		} else
			m_startNode = 0;

		if (copy.m_syncWord != 0) {
			const size_t len = strlen(copy.m_syncWord);
			m_syncWord = new char[len + 1];
			memcpy((char*) m_syncWord, copy.m_syncWord, len);
			const_cast<char*>(m_syncWord)[len] = '\0';
		} else
			m_syncWord = 0;

	}

	~AgentAnimationData() {
		if (m_owner) {
			if (m_sourceS != 0)
				delete[] m_sourceS;
			if (m_startNode != 0)
				delete[] m_startNode;
			if (m_syncWord != 0)
				delete[] m_syncWord;
		}
	}

	GameEventData* clone() const {
		return new AgentAnimationData(*this);
	}

	int getReturnInt() {
		return m_returnI;
	}

	float getReturnFloat() {
		return m_returnF;
	}

	const char* m_sourceS;
	int m_sourceI;
	SourceType m_sourceType;
	int m_animType;
	int m_valueI;
	float m_valueF;
	int m_playbackID;
	int m_strokeReps;
	float m_speed;
	float m_spatialExtent;
	const char* m_startNode;
	const char* m_syncWord;
	int m_returnI;
	float m_returnF;
};

/**
 * \brief Container for parameters of AgentComponent/Gaze
 *
 * @author Ionut Damian
 * @date  March 2011
 *
 */
class AgentGazeData: public GameEventData {
public:
	enum Type {
		GazeToPoint = 0, GazeToEntity
	};

	///Data container constructor for AG_GAZE
	///@param entityWorldID the entity we want to use the function on
	///@param target the coordinates of the target position
	///@param speed the gazing speed (use -1 for agent default)
	///@param duration the gazing duration in seconds, use -1 for constant gazing (default 10.0f)
	AgentGazeData(Vec3f target, float speed, float duration) :
			GameEventData(CUSTOM), m_targetV(target), m_speed(speed), m_duration(duration), m_returnI(-1), m_type(GazeToPoint) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_GAZE
	///@param entityWorldID the entity we want to use the function on
	///@param targetEntityID the target entity
	///@param speed the gazing speed (use -1 for agent default)
	///@param duration the gazing duration in seconds, use -1 for constant gazing (default 10.0f)
	AgentGazeData(int target, float speed, float duration) :
			GameEventData(CUSTOM), m_targetI(target), m_speed(speed), m_duration(duration), m_returnI(-1), m_type(GazeToEntity) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_HEADNOD/HEADSHAKE
	///@param entityWorldID the entity we want to use the function on
	///@param axis the axis of the shake  (use -1 for default)
	///@param extent the extent(amplitutde) of the shake  (use -1 for default)
	///@param count the number of motions  (use -1 for default)
	///@param speed the motion speed (use -1 for agent default)
	///@param duration the duration in seconds of a single motion (use -1 for default)
	AgentGazeData(int axis, float extent, int count, float speed, float duration) :
			GameEventData(CUSTOM), m_axis(axis), m_extent(extent), m_count(count), m_speed(speed), m_duration(duration) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_GAZE_GET_STATUS
	///@param gazeID the playback id of the gaze node
	AgentGazeData(int gazeID) :
			GameEventData(CUSTOM), m_gazeID(gazeID), m_returnI(-1) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_GAZE_GET_SPEED
	AgentGazeData() :
			GameEventData(CUSTOM), m_returnF(-1) {
		m_data.ptr = this;
	}

	AgentGazeData(const AgentGazeData& copy) :
			GameEventData(CUSTOM), m_targetV(copy.m_targetV), m_targetI(copy.m_targetI), m_returnI(copy.m_returnI), m_returnF(copy.m_returnF), m_speed(copy.m_speed), m_duration(copy.m_duration), m_gazeID(
					copy.m_gazeID), m_type(copy.m_type), m_axis(copy.m_axis), m_extent(copy.m_extent), m_count(copy.m_count) {
		m_data.ptr = this;
		m_owner = true;
	}

	~AgentGazeData() {
	}

	GameEventData* clone() const {
		return new AgentGazeData(*this);
	}

	int getReturnInt() {
		return m_returnI;
	}

	float getReturnFloat() {
		return m_returnF;
	}

	Vec3f m_targetV;
	int m_targetI;
	float m_speed;
	float m_duration;
	float m_extent;
	int m_gazeID;
	int m_axis;
	int m_count;
	int m_returnI;
	float m_returnF;
	Type m_type;
};

/**
 * \brief Container for parameters of AgentComponent/Formation
 *
 * @author Ionut Damian
 * @date  March 2011
 *
 */
class AgentFormationData: public GameEventData {
public:
	;
	///Data container constructor for AG_FORMATION_REACT
	///@param entityID the entity id of the event sender
	///@param _event the event to react to
	AgentFormationData(int entityID, int _event) :
			GameEventData(CUSTOM), m_entityID(entityID), m_event(_event), m_returnS(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_FORMATION_GET_AGENTS
	///@param members a pointer to an integer array
	AgentFormationData(int** members) :
			GameEventData(CUSTOM), m_members(members), m_returnI(-1), m_returnS(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_FORMATION_GET_TYPE, AG_FORMATION_GET_ENTRY, AG_GET_IPDIST, AG_GET_ORIENTCUST, AG_GET_REPOSANIM
	AgentFormationData() :
			GameEventData(CUSTOM), m_returnI(-1), m_returnF1(-1), m_returnF2(-1), m_returnS(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_SET_IPDIST
	///@param min the minimal ipdistance value
	///@param max the maximal ipdistance value
	AgentFormationData(float min, float max) :
			GameEventData(CUSTOM), m_min(min), m_max(max), m_returnS(0) {
		m_data.ptr = this;
	}

	AgentFormationData(const AgentFormationData& copy) :
			GameEventData(CUSTOM), m_returnV(copy.m_returnV), m_returnI(copy.m_returnI), m_returnF1(copy.m_returnF1), m_returnF2(copy.m_returnF2), m_members(copy.m_members), m_event(copy.m_event), m_entityID(
					copy.m_entityID), m_min(copy.m_min), m_max(copy.m_max) {
		m_data.ptr = this;
		m_owner = true;

		if (copy.m_returnS != 0) {
			const size_t len = strlen(copy.m_returnS);
			m_returnS = new char[len + 1];
			memcpy((char*) m_returnS, copy.m_returnS, len);
			const_cast<char*>(m_returnS)[len] = '\0';
		} else
			m_returnS = 0;

	}

	~AgentFormationData() {
		if (m_owner) {
			if (m_returnS != 0)
				delete[] m_returnS;
		}
	}

	GameEventData* clone() const {
		return new AgentFormationData(*this);
	}

	Vec3f getReturnVect() {
		return m_returnV;
	}

	int getReturnInt() {
		return m_returnI;
	}

	float getReturnFloat1() {
		return m_returnF1;
	}

	float getReturnFloat2() {
		return m_returnF2;
	}

	char* getReturnString() {
		return m_returnS;
	}

	Vec3f m_returnV;
	int m_returnI;
	float m_returnF1, m_returnF2;
	char* m_returnS;
	int** m_members;
	int m_event;
	int m_entityID;
	float m_min, m_max;
};

/**
 * \brief Container for parameters of AgentComponent/Config
 *
 * @author Ionut Damian
 * @date  March 2011
 *
 */
class AgentConfigData: public GameEventData {
public:
	enum Type {
		INT = 0, FLOAT, STRING
	};

	///Data container constructor for AG_GET_PARAM, AG_SET_PARAM
	///@param param the parameter of type Agent_Param
	///@param value the value to set
	AgentConfigData(int param, int value) :
			GameEventData(CUSTOM), m_type(INT), m_param(param), m_valueI(value), m_returnI(-1), m_returnF(-1), m_returnS(0), m_valueS(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_GET_PARAM, AG_SET_PARAM
	///@param param the parameter of type Agent_Param
	///@param value the value to set
	AgentConfigData(int param, float value) :
			GameEventData(CUSTOM), m_type(FLOAT), m_param(param), m_valueF(value), m_returnI(-1), m_returnF(-1), m_returnS(0), m_valueS(0) {
		m_data.ptr = this;
	}

	///Data container constructor for AG_GET_PARAM, AG_SET_PARAM
	///@param param the parameter of type Agent_Param
	///@param value the value to set
	AgentConfigData(int param, const char* value) :
			GameEventData(CUSTOM), m_type(STRING), m_param(param), m_valueS(value), m_returnI(-1), m_returnF(-1), m_returnS(0) {
		m_data.ptr = this;
	}

	AgentConfigData(const AgentConfigData& copy) :
			GameEventData(CUSTOM), m_valueI(copy.m_valueI), m_valueF(copy.m_valueF), m_returnI(copy.m_returnI), m_returnF(copy.m_returnF), m_type(copy.m_type) {
		m_data.ptr = this;
		m_owner = true;

		if (copy.m_returnS != 0) {
			const size_t len = strlen(copy.m_returnS);
			m_returnS = new char[len + 1];
			memcpy((char*) m_returnS, copy.m_returnS, len);
			const_cast<char*>(m_returnS)[len] = '\0';
		} else
			m_returnS = 0;

		if (copy.m_valueS != 0) {
			const size_t len = strlen(copy.m_valueS);
			m_valueS = new char[len + 1];
			memcpy((char*) m_valueS, copy.m_valueS, len);
			const_cast<char*>(m_valueS)[len] = '\0';
		} else
			m_valueS = 0;

	}

	~AgentConfigData() {
		if (m_owner) {
			if (m_returnS != 0)
				delete[] m_returnS;
			if (m_valueS != 0)
				delete[] m_valueS;
		}
	}

	GameEventData* clone() const {
		return new AgentConfigData(*this);
	}

	int getReturnInt() {
		return m_returnI;
	}

	float getReturnFloat() {
		return m_returnF;
	}

	const char* getReturnString() {
		return m_returnS;
	}

	int m_param;
	int m_returnI, m_valueI;
	float m_returnF, m_valueF;
	const char *m_returnS, *m_valueS;
	Type m_type;
};

class Vec3fData: public GameEventData {
public:
	Vec3fData(const float& x, const float& y, const float& z) :
			GameEventData(CUSTOM), m_vector(Vec3f(x, y, z)) {
		m_data.ptr = &m_vector;
	}

	Vec3fData(const Vec3f& vec) :
			GameEventData(CUSTOM), m_vector(vec) {
		m_data.ptr = &m_vector;
	}

	Vec3fData(Vec3f* vec) :
			GameEventData(CUSTOM) {
		m_data.ptr = vec;
	}

	virtual GameEventData* clone() const {
		return new Vec3fData(*((Vec3f*) m_data.ptr));
	}

private:
	Vec3f m_vector;
};

class Interaction: public GameEventData {

public:
	Interaction(unsigned int targetID, const std::string& interactionName, const std::string& satisfies, unsigned int interactionID, int slotID = -1) :
			GameEventData(CUSTOM), TargetID(targetID), InteractionName(interactionName), SlotID(slotID), InteractionID(interactionID), Satisfies(satisfies) {
		m_data.ptr = this;
	}

	Interaction(const std::string& target, const std::string& interactionName, const std::string& satisfies, unsigned int interactionID, int slotID = -1) :
			GameEventData(CUSTOM), Target(target), TargetID(0), InteractionName(interactionName), SlotID(slotID), InteractionID(interactionID), Satisfies(satisfies) {
		m_data.ptr = this;
	}

	Interaction(const Interaction& copy) :
			GameEventData(CUSTOM), TargetID(copy.TargetID), Target(copy.Target), InteractionName(copy.InteractionName), SlotID(copy.SlotID), InteractionID(copy.InteractionID), Satisfies(
					copy.Satisfies) {
		m_data.ptr = this;
	}

	~Interaction() {
	}

	unsigned int TargetID;
	std::string Target;
	std::string InteractionName;
	std::string Satisfies;
	int SlotID;
	unsigned int InteractionID;

	GameEventData* clone() const {
		return new Interaction(*this);
	}
};

class GLFunction: public GameEventData {

public:
	enum functions {
		drawCross = 0, drawCircle, drawLine, drawNumber, setColor, drawCircleFilled
	};

	/* function drawCross uses param0 as x, param1 as y and param2 as z
	 function setColor uses param0 as r, param2 as g and param3 as b */
	GLFunction(unsigned int functionID, float param0, float param1, float param2) :
			GameEventData(CUSTOM), FunctionID(functionID), Param0(param0), Param1(param1), Param2(param2), Param3(0), Param4(0), Param5(0), Param6(0), Param7(0), Param8(0), Param9(0) {
		m_data.ptr = this;
	}

	/* function drawCircle uses param0 as x, param1 as y, param2 as z and param3 as radius
	 function drawNumber uses param0 as number, param1 as x, param2 as y, param3 as z*/
	GLFunction(unsigned int functionID, float param0, float param1, float param2, float param3) :
			GameEventData(CUSTOM), FunctionID(functionID), Param0(param0), Param1(param1), Param2(param2), Param3(param3), Param4(0), Param5(0), Param6(0), Param7(0), Param8(0), Param9(0) {
		m_data.ptr = this;
	}

	/* function drawLine uses param0 as x0, param1 as y0, param2 as z0, param3 as x1, param4 as y1, param5 as z1*/
	GLFunction(unsigned int functionID, float param0, float param1, float param2, float param3, float param4, float param5) :
			GameEventData(CUSTOM), FunctionID(functionID), Param0(param0), Param1(param1), Param2(param2), Param3(param3), Param4(param4), Param5(param5), Param6(0), Param7(0), Param8(0), Param9(0) {
		m_data.ptr = this;
	}

	GLFunction(const GLFunction& copy) :
			GameEventData(CUSTOM), FunctionID(copy.FunctionID), Param0(copy.Param0), Param1(copy.Param1), Param2(copy.Param2), Param3(copy.Param3), Param4(copy.Param4), Param5(copy.Param5), Param6(
					copy.Param6), Param7(copy.Param7), Param8(copy.Param8), Param9(copy.Param9) {
		m_data.ptr = this;
		m_owner = true;
	}

	~GLFunction() {
		if (m_owner) {
		}
	}

	unsigned int FunctionID;
	const float Param0;
	const float Param1;
	const float Param2;
	const float Param3;
	const float Param4;
	const float Param5;
	const float Param6;
	const float Param7;
	const float Param8;
	const float Param9;

	GameEventData* clone() const {
		return new GLFunction(*this);

	}
};

class GamepadData: public GameEventData {
	//the values for Sticks and Triggers is in a range between 0 and 1
	//the values for buttons may be 0 for not pressed, 1 for newly pressed
	//index is used as indicator, if controller is connected. if index is -1, then its not. Else index is the index of the controller (0-3)

public:
	GamepadData() :
			GameEventData(CUSTOM), Index(-1), Connected(0), StickLeftX(0), StickLeftY(0), StickRightX(0), StickRightY(0), DigitalPadX(0), DigitalPadY(0), ButtonA(0), ButtonB(0), ButtonX(0), ButtonY(
					0), ButtonStart(0), ButtonBack(0), TriggerLeft(0), TriggerRight(0), ShoulderRight(0), ShoulderLeft(0), StickLeftClick(0), StickRightClick(0), VibratorLeft(0), VibratorRight(0) {
		m_data.ptr = this;
	}

	GamepadData(unsigned int gamepadIndex, bool connected, float thumbStickLX, float thumbStickLY, float thumbStickRX, float thumbStickRY, float dPadX, float dPadY, unsigned int butA,
			unsigned int butB, unsigned int butX, unsigned int butY, unsigned int butStart, unsigned int butBack, float triggerL, float triggerR, unsigned int shoulderR, unsigned int shoulderL,
			unsigned int thumbStickLClick, unsigned int thumbStickRClick) :
			GameEventData(CUSTOM), Index(gamepadIndex), StickLeftX(thumbStickLX), StickLeftY(thumbStickLY), StickRightX(thumbStickRX), StickRightY(thumbStickRY), DigitalPadX(dPadX), DigitalPadY(
					dPadY), ButtonA(butA), ButtonB(butB), ButtonX(butX), ButtonY(butY), ButtonStart(butStart), ButtonBack(butBack), TriggerLeft(triggerL), TriggerRight(triggerR), ShoulderRight(
					shoulderL), ShoulderLeft(shoulderR), StickLeftClick(thumbStickLClick), StickRightClick(thumbStickRClick), VibratorLeft(0), VibratorRight(0) {
		m_data.ptr = this;
	}

	GamepadData(const GamepadData& copy) :
			GameEventData(CUSTOM), Index(copy.Index), Connected(0), StickLeftX(copy.StickLeftX), StickLeftY(copy.StickLeftY), StickRightX(copy.StickRightX), StickRightY(copy.StickRightY), DigitalPadX(
					copy.DigitalPadX), DigitalPadY(copy.DigitalPadY), ButtonA(copy.ButtonA), ButtonB(copy.ButtonB), ButtonX(copy.ButtonX), ButtonY(copy.ButtonY), ButtonStart(copy.ButtonStart), ButtonBack(
					copy.ButtonBack), TriggerLeft(copy.TriggerLeft), TriggerRight(copy.TriggerRight), ShoulderRight(copy.ShoulderRight), ShoulderLeft(copy.ShoulderLeft), StickLeftClick(
					copy.StickLeftClick), StickRightClick(copy.StickRightClick), VibratorLeft(copy.VibratorLeft), VibratorRight(copy.VibratorRight) {
		m_data.ptr = this;
		m_owner = true;
	}

	int Index;
	bool Connected;
	float StickLeftX;
	float StickLeftY;
	float StickRightX;
	float StickRightY;
	float DigitalPadX;
	float DigitalPadY;
	unsigned int ButtonA;
	unsigned int ButtonB;
	unsigned int ButtonX;
	unsigned int ButtonY;
	unsigned int ButtonStart;
	unsigned int ButtonBack;
	float TriggerLeft;
	float TriggerRight;
	unsigned int ShoulderRight;
	unsigned int ShoulderLeft;
	float VibratorRight;
	float VibratorLeft;
	unsigned int StickLeftClick;
	unsigned int StickRightClick;

	GameEventData* clone() const {
		return new GamepadData(*this);
	}
};

/**
 * \brief Data container for the E_GET_ANIM_LENGTH event
 */
class AnimLengthData: public GameEventData {

public:
	AnimLengthData(const char* name, float* length, float speed = 0) :
			GameEventData(CUSTOM), Name(name), Length(length), Speed(speed) {
		m_data.ptr = this;
	}

	AnimLengthData(const AnimLengthData& copy) :
			GameEventData(CUSTOM), Length(copy.Length), Speed(copy.Speed) {
		m_data.ptr = this;
		m_owner = true;
		const size_t lenName = copy.Name ? strlen(copy.Name) : 0;
		Name = new char[lenName + 1];
		memcpy((char*) Name, copy.Name, lenName);
		const_cast<char*>(Name)[lenName] = '\0';
	}

	~AnimLengthData() {
		if (m_owner) {
			delete[] Name;
		}
	}

	const char* Name;
	float Speed;
	float* Length;

	AnimLengthData* clone() const {
		return new AnimLengthData(*this);

	}
};

/**
 * Container for Emotion Data used by a GameEvent
 */
class EmotionData: public GameEventData {
public:
	EmotionData(const char* emotionName, bool checkIntensifier) :
			GameEventData(CUSTOM), emotion(emotionName), intensifier(checkIntensifier) {
		m_data.ptr = this;
	}

	EmotionData(const EmotionData& copy) :
			GameEventData(CUSTOM), emotion(copy.emotion), intensifier(copy.intensifier) {
		m_data.ptr = this;
		m_owner = true;
		const size_t len = copy.emotion ? strlen(copy.emotion) : 0;
		emotion = new char[len + 1];
		memcpy((char*) emotion, copy.emotion, len);
		const_cast<char*>(emotion)[len] = '\0';
	}

	~EmotionData() {
		if (m_owner)
			delete[] emotion;
	}

	GameEventData* clone() const {
		return new EmotionData(*this);
	}

	const char* emotion;
	bool intensifier;
};

class SentenceData: public GameEventData {
public:
	SentenceData(const char* sentence, unsigned int partnerId) :
			GameEventData(CUSTOM), text(sentence), partnerID(partnerId) {
		m_data.ptr = this;
	}

	SentenceData(const SentenceData& copy) :
			GameEventData(CUSTOM), text(copy.text), partnerID(copy.partnerID) {
		m_data.ptr = this;
		m_owner = true;
		const size_t len = copy.text ? strlen(copy.text) : 0;
		text = new char[len + 1];
		memcpy((char*) text, copy.text, len);
		const_cast<char*>(text)[len] = '\0';
	}

	~SentenceData() {
		if (m_owner)
			delete[] text;
	}

	GameEventData* clone() const {
		return new SentenceData(*this);
	}

	const char* text;
	unsigned int partnerID;
};

class SoundResourceData: public GameEventData {
public:
	SoundResourceData(const char* userData, int dataSize, int samplesPerSec, int bitsPerSample, int numChannels) :
			GameEventData(CUSTOM), m_userData(userData), m_dataSize(dataSize), m_samplesPerSec(samplesPerSec), m_bitsPerSample(bitsPerSample), m_numChannels(numChannels) {
		m_data.ptr = this;
	}

	SoundResourceData(const SoundResourceData& copy) :
			GameEventData(CUSTOM), m_userData(copy.m_userData), m_dataSize(copy.m_dataSize), m_samplesPerSec(copy.m_samplesPerSec), m_bitsPerSample(copy.m_bitsPerSample), m_numChannels(
					copy.m_numChannels) {
		m_data.ptr = this;
		m_owner = true;
	}

	~SoundResourceData() {
	}

	GameEventData* clone() const {
		return new SoundResourceData(*this);
	}

	// Pointer to data
	const char* m_userData;
	// It's size
	int m_dataSize;
	// Sound properties
	int m_samplesPerSec;
	int m_bitsPerSample;
	int m_numChannels;
};

/**
 * Exchange container for KinectWorld smart object actions, used in a KW_GET_ACTIONS event.
 * Contains the action name, its availability and the inputs necessary to trigger it.
 */
class KWActionData: public GameEventData {

public:
	KWActionData(std::string actionName, std::string keyword, std::string gestureName, bool enabled) :
			GameEventData(CUSTOM) {
		m_data.ptr = this;
		_actionName = actionName;
		_keyword = keyword;
		_gestureName = gestureName;
		_enabled = enabled;
	}

	~KWActionData() {
	}

	KWActionData(KWActionData const& kwa) :
			GameEventData(CUSTOM) {
		_actionName.assign(kwa.getActionName());
		_keyword.assign(kwa.getKeyword());
		_gestureName.assign(kwa.getGestureName());
		_enabled = kwa.isEnabled();
	}

	const std::string& getActionName() const {
		return _actionName;
	}

	const std::string& getKeyword() const {
		return _keyword;
	}

	const std::string& getGestureName() const {
		return _gestureName;
	}

	void setEnabled(bool enabled) {
		_enabled = enabled;
	}

	bool isEnabled() const {
		return _enabled;
	}

private:
	KWActionData();
	KWActionData& operator=(KWActionData const& kwa);

	std::string _actionName;
	std::string _keyword;
	std::string _gestureName;
	bool _enabled;
};

/**
 * Exchange container for KinectWorld smart object dialogue actions, used in a KW_GET_ACTIONS event.
 * In addition to the information of KWActionData it contains the full sentence used for input.
 */
class KWDialogueActionData: public KWActionData {

public:
	KWDialogueActionData(std::string actionName, std::string sentence, std::vector<std::string> keywords, std::string gestureName, bool enabled) :
			KWActionData(actionName, "", gestureName, enabled) {
		_fullSentence = sentence;
		for (size_t i = 0; i < keywords.size(); i++)
			_sentenceKeywords.push_back(keywords.at(i));
	}

	~KWDialogueActionData() {
	}

	KWDialogueActionData(KWDialogueActionData const& kwa) :
			KWActionData(kwa.getActionName(), kwa.getKeyword(), kwa.getGestureName(), kwa.isEnabled()) {
		_fullSentence.assign(kwa.getFullSentence());
		for (size_t i = 0; i < kwa.getSentenceKeywords().size(); i++)
			_sentenceKeywords.push_back(kwa.getSentenceKeywords().at(i));

	}

	const std::string& getFullSentence() const {
		return _fullSentence;
	}

	const std::vector<std::string>& getSentenceKeywords() const {
		return _sentenceKeywords;
	}

private:
	KWDialogueActionData();
	KWDialogueActionData& operator=(KWDialogueActionData const& kwa);

	std::string _fullSentence;
	std::vector<std::string> _sentenceKeywords;
};

/**
 * Exchange container for the KW_IS_VISIBLE event.
 */
class KWVisibleData: public GameEventData {

public:
	KWVisibleData(unsigned int cameraNode) :
			GameEventData(CUSTOM) {
		m_data.ptr = this;
		_cameraNode = cameraNode;
		_visible = false;
	}

	~KWVisibleData() {
	}

	KWVisibleData(KWVisibleData const& kv) :
			GameEventData(CUSTOM) {
		m_data.ptr = this;
		_cameraNode = kv.getCameraNode();
		_visible = kv.getVisible();
	}

	const unsigned int getCameraNode() const {
		return _cameraNode;
	}

	void setCameraNode(unsigned int cameraNode) {
		_cameraNode = cameraNode;
	}

	const bool getVisible() const {
		return _visible;
	}

	void setVisible(bool visible) {
		_visible = visible;
	}

private:
	KWVisibleData();
	KWVisibleData& operator=(KWVisibleData const& kv);

	unsigned int _cameraNode;
	bool _visible;
};

/**
 * Base container for querying (KW_GET_PROPERTY) or setting (KW_SET_PROPERTY)
 * a property of a KinectWorld smart object.
 * For such an event the subclasses have to be used.
 */
class KWPropertyData: public GameEventData {
public:
	enum PropertyType {
		NUMBER, BOOL, STRING
	};
	virtual ~KWPropertyData() {
	}

	const std::string& getName() {
		return _name;
	}
	PropertyType getType() {
		return _type;
	}
	void setFound(bool found) {
		_found = found;
	}
	bool getFound() {
		return _found;
	}

protected:
	KWPropertyData(std::string name, PropertyType type) :
			GameEventData(CUSTOM) {
		m_data.ptr = this;
		_name.assign(name);
		_type = type;
		_found = false;
	}
	KWPropertyData();

	std::string _name;
	PropertyType _type;
	bool _found;
};

/**
 * Container for a numerical property of a KinectWorld smart object.
 */
class KWNumPropertyData: public KWPropertyData {
public:
	KWNumPropertyData(std::string name, float value) :
			KWPropertyData(name, KWPropertyData::NUMBER) {
		_value = value;
	}

	~KWNumPropertyData() {
	}

	float getValue() {
		return _value;
	}
	void setValue(float value) {
		_value = value;
	}

private:
	KWNumPropertyData() {
	}
	float _value;
};

/**
 * Container for a boolean property of a KinectWorld smart object.
 */
class KWBoolPropertyData: public KWPropertyData {
public:
	KWBoolPropertyData(std::string name, bool value) :
			KWPropertyData(name, KWPropertyData::BOOL) {
		_value = value;
	}

	~KWBoolPropertyData() {
	}

	bool getValue() {
		return _value;
	}
	void setValue(bool value) {
		_value = value;
	}

private:
	KWBoolPropertyData() {
	}
	bool _value;
};

/**
 * Container for a string property of a KinectWorld smart object.
 */
class KWStrPropertyData: public KWPropertyData {
public:
	KWStrPropertyData(std::string name, std::string value) :
			KWPropertyData(name, KWPropertyData::STRING) {
		_value.assign(value);
	}

	~KWStrPropertyData() {
	}

	const std::string& getValue() {
		return _value;
	}
	void setValue(std::string value) {
		_value.assign(value);
	}

private:
	KWStrPropertyData() {
	}
	std::string _value;
};

/*! @}*/
#endif
