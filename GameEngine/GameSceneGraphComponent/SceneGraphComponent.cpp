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
// GameEngine SceneGraph Component of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#include "SceneGraphComponent.h"

#include <GameEngine/GameLog.h>
#include <GameEngine/GameEvent.h>
#include <GameEngine/GameWorld.h>
#include <GameEngine/GameModules.h>
#include <GameEngine/GameEntity.h>
#include <GameEngine/GameEngine.h>

#include <XMLParser/utXMLParser.h>

#include <Horde3D/Horde3D.h>
#include <Horde3D/Horde3DTerrain.h>

#include "SceneGraph.h"

// Uncomment to support visibility request only for the first camera set
//#define IGNORE_CAM_CHANGE_FOR_VISIBILITY

GameComponent* SceneGraphComponent::createComponent( GameEntity* owner )
{
	return new SceneGraphComponent( owner );
}

SceneGraphComponent::SceneGraphComponent( GameEntity* owner) : GameComponent(owner, "Horde3D"), 
	m_hordeID(0), m_visibilityFlag(0), m_terrainGeoRes(0), m_net_applyTrajection(false), m_net_maxTrajection((float)HUGE_VAL)
{
	owner->addListener(GameEvent::E_SET_TRANSFORMATION, this);
	owner->addListener(GameEvent::E_TRANSFORMATION, this);
	owner->addListener(GameEvent::E_TRANSLATE_LOCAL, this);
	owner->addListener(GameEvent::E_TRANSLATE_GLOBAL, this);
	owner->addListener(GameEvent::E_ROTATE_LOCAL, this);
	owner->addListener(GameEvent::E_MESH_DATA, this);
	owner->addListener(GameEvent::E_MORPH_TARGET, this);
	owner->addListener(GameEvent::E_SET_ROTATION, this);
	owner->addListener(GameEvent::E_SET_SCALE, this);
	owner->addListener(GameEvent::E_ACTIVATE_CAM, this);
	owner->addListener(GameEvent::E_SET_ANIM_FRAME, this);
	owner->addListener(GameEvent::E_SET_TRANSLATION, this);
	owner->addListener(GameEvent::E_SET_NODE_PARENT, this);
	owner->addListener(GameEvent::E_ATTACH, this);
	owner->addListener(GameEvent::E_COLLISION, this);
	owner->addListener(GameEvent::E_SET_ENABLED, this);
	owner->addListener(GameEvent::E_GET_ACTIVE_CAM, this);
	owner->addListener(GameEvent::E_GET_VISIBILITY, this);
	owner->addListener(GameEvent::E_GET_PROJECTION_MATRIX, this);
	owner->addListener(GameEvent::E_GET_SCENEGRAPH_ID, this);
	//printf("ID added %d\n", hordeID);
	SceneGraphManager::instance()->addComponent( this );
}

SceneGraphComponent::~SceneGraphComponent()
{
	if (m_hordeID != 0 && h3dGetNodeType(m_hordeID) != H3DNodeTypes::Undefined)
		h3dRemoveNode(m_hordeID);	
	SceneGraphManager::instance()->removeComponent(this);
	unloadTerrainGeoRes();
}

void SceneGraphComponent::setHordeID( H3DNode id )
{
	m_hordeID = id;

	if( id != 0 )
	{
		const float* t = 0x0;
		h3dGetNodeTransMats(id, 0, &t);
		// Copy absolute transformation of scene graph resource to the member variable
		if( t != 0x0 )
			memcpy(m_transformation, t, sizeof(float) * 16);
		else
			GameLog::errorMessage("Error retrieving absolute transformation matrix for HordeID %d", id);
	}
}

void SceneGraphComponent::executeEvent(GameEvent *event)
{
	switch (event->id())
	{
	case GameEvent::E_SET_TRANSFORMATION:
		setTransformation(static_cast<const float*>(event->data()));
		break;
	case GameEvent::E_TRANSFORMATION:
		memcpy(static_cast<float*>(event->data()), m_transformation, sizeof(float) * 16);
		break;
	case GameEvent::E_SET_TRANSLATION:
		{
			Vec3f* translation = static_cast<Vec3f*>(event->data());
			m_transformation[12] = translation->x;
			m_transformation[13] = translation->y;
			m_transformation[14] = translation->z;
			sendTransformation();
		}
		break;
	case GameEvent::E_SET_SCALE:
		setScale(static_cast<Vec3f*>(event->data()));
		break;
	case GameEvent::E_MESH_DATA:
		getMeshData(static_cast<MeshData*>(event->data()));
		break;
	case GameEvent::E_TRANSLATE_LOCAL:
		translateLocal(static_cast<Vec3f*>(event->data()));
		break;
	case GameEvent::E_TRANSLATE_GLOBAL:
		translateGlobal(static_cast<Vec3f*>(event->data()));
		break;
	case GameEvent::E_ROTATE_LOCAL:
		rotate(static_cast<Vec3f*>(event->data()));
		break;
	case GameEvent::E_SET_ROTATION:
		setRotation(static_cast<Vec3f*>(event->data()));
		break;
	case GameEvent::E_ATTACH:
		attach(static_cast<Attach*>(event->data()));
		break;	
	case GameEvent::E_SET_NODE_PARENT:
		setParentNode(static_cast<Attach*>(event->data()));
		break;
	case GameEvent::E_MORPH_TARGET:
		if (m_hordeID > 0)
		{
			h3dSetModelMorpher(m_hordeID, static_cast<MorphTarget*>(event->data())->Name, static_cast<MorphTarget*>(event->data())->Value);
			h3dUpdateModel(m_hordeID, H3DModelUpdateFlags::Geometry );		
		}
		break;
	case GameEvent::E_ACTIVATE_CAM:
		if (m_hordeID > 0 && h3dGetNodeType(m_hordeID) == H3DNodeTypes::Camera)
			SceneGraphManager::instance()->setActiveCam( m_hordeID );
		break;
	case GameEvent::E_SET_ANIM_FRAME:
		{
			if (m_hordeID > 0)
			{
				const SetAnimFrame* const data = static_cast<SetAnimFrame*>(event->data());			
				h3dSetModelAnimParams(m_hordeID, data->Stage, data->Time, data->Weight);	
				h3dUpdateModel(m_hordeID, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry );		
			}
		}
		break;
	case GameEvent::E_SET_ENABLED:
		setEnabled(*static_cast<bool*>(event->data()));
		break;
	case GameEvent::E_GET_VISIBILITY:
		{
			bool* visible = static_cast<bool*>(event->data());
			if (visible)
				*visible = getVisibility();
		}
		break;
	case GameEvent::E_GET_PROJECTION_MATRIX:
		{
			float* mat = static_cast<float*>(event->data());
			SceneGraphManager::instance()->getCameraProjectionMatrix(mat);
		}
		break;
	case GameEvent::E_GET_ACTIVE_CAM:
		if (SceneGraphManager::instance()->getActiveCam() == m_hordeID)
		{
			unsigned int* id = static_cast<unsigned int*>(event->data());
			if (id)
				*id = m_owner->worldId();
		}
		break;
	case GameEvent::E_GET_SCENEGRAPH_ID:
		{
			int* id = static_cast<int*>(event->data());
			if (id)
				*id = m_hordeID;
		}
		break;
	}
}

void SceneGraphComponent::update()
{
	// save the current transformation for calculation of trajectory
	memcpy(m_net_lasttransformation, m_transformation, sizeof(float) * 16);

	// 2011-05-03: code transfered to checkTransformation(), so internal calls to update() could be eliminated.
	checkTransformation();

	// apply pending trajectories
	traject();
}

void SceneGraphComponent::checkTransformation()
{
	// Check if transformation has been changed (e.g. by a transformation change of a parent node)
	if ( m_hordeID > 0 && h3dCheckNodeTransFlag( m_hordeID, true ) )
	{
		// Update the locally stored global transformation 
		const float* absTrans = 0;
		h3dGetNodeTransMats(m_hordeID, 0, &absTrans);
		memcpy(m_transformation, absTrans, sizeof(float) * 16);

		GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(m_transformation, 16), this);
		m_owner->executeEvent(&event);
		m_visibilityFlag = 0;
	}
}

void SceneGraphComponent::loadFromXml( const XMLNode* description )
{
	const char* id = description->getAttribute("id", "0");
	int newID = atoi(id);
	
	// Remove component
	if( newID == 0 && m_hordeID != 0)
	{
		m_hordeID = newID;
		SceneGraphManager::instance()->removeComponent(this);
		GameLog::errorMessage( " No valid Horde3D ID for SceneGraphComponent in entity '%s'", m_owner->id().c_str() );	
	}
	// Change component
	else if ( m_hordeID != 0 && m_hordeID != newID )
	{
		GameLog::errorMessage(" Warning: The scene graph component for entity '%s' has been already initialized with '%d' Untested behavior!", m_owner->id().c_str(), m_hordeID);

	}
	//
	if( newID != 0 )
	{
		m_hordeID = newID;
		const float* trans = 0;
		h3dGetNodeTransMats(m_hordeID, 0, &trans);
		if( trans )
		{
			SceneGraphManager::instance()->addComponent(this);
			memcpy( m_transformation, trans, sizeof( float ) * 16 );
			m_visibilityFlag = 0;
		}
		else
		{
			m_hordeID = 0;
			SceneGraphManager::instance()->removeComponent(this);
			GameLog::errorMessage("Entity '%s': The ID '%d' seems to be no valid Horde3D ID!", m_owner->id().c_str(), newID);
		}
	}
		
}

void SceneGraphComponent::setTransformation(const float *transformation)
{
	memcpy(m_transformation, transformation, sizeof(float) * 16);
	sendTransformation();
}

void SceneGraphComponent::sendTransformation()
{
	if (m_hordeID > 0)
	{
//		printf("Inside send Transformation with hID: %d\n", m_hordeID);
		const float* parentMat = 0;
		// since the event transformation is absolute we have to create a relative transformation matrix for Horde3D
		h3dGetNodeTransMats(h3dGetNodeParent(m_hordeID), 0, &parentMat);		
		if (parentMat) h3dSetNodeTransMat(m_hordeID, (Matrix4f(parentMat).inverted() * Matrix4f(m_transformation)).x);
		else printf("no parent MAT\n");
		// ensure reset of Horde3D transformation flag since we have updated it from the GameEngine and such don't need to be informed 
		// when calling SceneGraphComponent::update() (note that the node transformed flag will be set again if someone else will update
		// the transformation from outside, so this way of avoiding unnecessary updates should be safe)
		h3dCheckNodeTransFlag( m_hordeID, true );
		//printf("SceneGraphComponent::setTransformation\n\t %.3f, %.3f, %.3f\n", transformation[12], transformation[13], transformation[14]);
		m_visibilityFlag = 0;
	}
}

void SceneGraphComponent::getMeshData(MeshData* data)
{
	if (m_hordeID > 0)
	{
		H3DRes geoResource = 0;
		int vertexOffset = 0;
		int indexOffset = 0;
		switch(h3dGetNodeType(m_hordeID))
		{
		case H3DNodeTypes::Mesh:
			geoResource = h3dGetNodeParamI(h3dGetNodeParent(m_hordeID), H3DModel::GeoResI);
			data->NumVertices = h3dGetNodeParamI(m_hordeID, H3DMesh::VertREndI) - h3dGetNodeParamI(m_hordeID, H3DMesh::VertRStartI) + 1;
			data->NumTriangleIndices = h3dGetNodeParamI(m_hordeID, H3DMesh::BatchCountI);		
			data->VertRStart = h3dGetNodeParamI(m_hordeID, H3DMesh::VertRStartI);
			vertexOffset = h3dGetNodeParamI(m_hordeID, H3DMesh::VertRStartI) * 3;
			indexOffset = h3dGetNodeParamI(m_hordeID, H3DMesh::BatchStartI);
			break;
		case H3DNodeTypes::Model:
			geoResource = h3dGetNodeParamI(m_hordeID, H3DModel::GeoResI);
			data->NumVertices = h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoVertexCountI);
			data->NumTriangleIndices = h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexCountI);		
			break;
		case H3DEXT_NodeType_Terrain:
			unloadTerrainGeoRes();
			geoResource = h3dextCreateTerrainGeoRes( 
				m_hordeID, 
				h3dGetNodeParamStr( m_hordeID, H3DNodeParams::NameStr ), 
				h3dGetNodeParamF( m_hordeID, H3DEXTTerrain::MeshQualityF, 0) );		
			data->NumVertices = h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoVertexCountI);
			data->NumTriangleIndices = h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexCountI);		
			m_terrainGeoRes = geoResource;
			break;
		}
		float* vb = (float*)h3dMapResStream(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoVertPosStream, true, false);
		data->VertexBase = new float[(data->NumVertices*3)];
		memcpy(data->VertexBase, vb+vertexOffset, sizeof(float)*data->NumVertices*3);
		h3dUnmapResStream(geoResource);

		//Triangle indices, must cope with 16 bit and 32 bit
		if (h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndices16I)) {
			unsigned short* tb = (unsigned short*)h3dMapResStream(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexStream, true, false);
			data->TriangleBase16 = new unsigned short[data->NumTriangleIndices];
			memcpy(data->TriangleBase16, tb+indexOffset, sizeof(unsigned short)*data->NumTriangleIndices);
			h3dUnmapResStream(geoResource);
		} else {
			unsigned int* tb = (unsigned int*)h3dMapResStream(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexStream, true, false);
			data->TriangleBase32 = new unsigned int[data->NumTriangleIndices];
			memcpy(data->TriangleBase32, tb+indexOffset, sizeof(unsigned int)*data->NumTriangleIndices);
			h3dUnmapResStream(geoResource);
		}
		//data->TriangleMode = Horde3D::getResourceParami(geoResource, GeometryResParams::TriangleMode);
	}
}

void SceneGraphComponent::translateLocal(const Vec3f* translation)
{
	// ensure that m_transfomration is up to date
	checkTransformation();

	/*Matrix4f trans(m_transformation);
	Vec3f t( (trans * *translation) );
	trans.x[12] = t.x;
	trans.x[13] = t.y;
	trans.x[14] = t.z;
	GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(trans.x, 16), 0);*/
	
	// Backup trans
	Vec3f transBackup(m_transformation[12], m_transformation[13], m_transformation[14]);

	// Apply it
	Vec3f t( (Matrix4f(m_transformation) * *translation) );
	m_transformation[12] = t.x;
	m_transformation[13] = t.y;
	m_transformation[14] = t.z;
	
	// Check the event
	GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(m_transformation, 16), this);
	if (m_owner->checkEvent(&event))
	{
		// Send trans to horde
		sendTransformation();
		// and to other components
		m_owner->executeEvent(&event);
	}
	else
	{
		// Revert changes
		m_transformation[12] = transBackup.x;
		m_transformation[13] = transBackup.y;
		m_transformation[14] = transBackup.z;
	}
}

void SceneGraphComponent::translateGlobal(const Vec3f* translation)
{
	// ensure that m_transfomration is up to date
	checkTransformation();

	/*Matrix4f trans(m_transformation);
	trans.x[12] += translation->x;
	trans.x[13] += translation->y;
	trans.x[14] += translation->z;
	GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(trans.x, 16), 0);*/

	// Backup trans
	Vec3f transBackup(m_transformation[12], m_transformation[13], m_transformation[14]);

	m_transformation[12] += translation->x;
	m_transformation[13] += translation->y;
	m_transformation[14] += translation->z;
	
	GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(m_transformation, 16), this);
	if (m_owner->checkEvent(&event))
	{
		// Send transformation to horde
		sendTransformation();
		//and to the other components
		m_owner->executeEvent(&event);
	}
	else
	{
		// Revert changes
		m_transformation[12] = transBackup.x;
		m_transformation[13] = transBackup.y;
		m_transformation[14] = transBackup.z;
	}
}

void SceneGraphComponent::setScale(const Vec3f *scale)
{
	/*Vec3f tr,rotation,sc;
	Matrix4f(m_transformation).decompose(tr,rotation,sc);	
	
	Matrix4f trans = Matrix4f::ScaleMat( scale->x, scale->y, scale->z );
	trans = trans * Matrix4f(Quaternion(rotation.x, rotation.y, rotation.z));
	trans.translate(m_transformation[12], m_transformation[13], m_transformation[14]);
	
	GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(trans.x, 16), 0);*/

	Vec3f tr,rotation,sc;
	Matrix4f(m_transformation).decompose(tr,rotation,sc);	
	
	Matrix4f trans = Matrix4f::ScaleMat( scale->x, scale->y, scale->z );
	trans = trans * Matrix4f(Quaternion(rotation.x, rotation.y, rotation.z));
	trans.translate(tr.x, tr.y, tr.z);
	
	GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(trans.x, 16), this);
	if (m_owner->checkEvent(&event))
	{
		// Apply the new transformation
		memcpy( m_transformation, trans.x, sizeof( float ) * 16 );
		// Send it to horde
		sendTransformation();
		//and to the other components
		m_owner->executeEvent(&event);
	}
}


void SceneGraphComponent::rotate(const Vec3f* rotation)
{
	// ensure that m_transfomration is up to date
	checkTransformation();

	Matrix4f trans( Matrix4f(m_transformation) * Matrix4f(Quaternion(degToRad(rotation->x), degToRad(rotation->y), degToRad(rotation->z))) );
	
	// Create event without sender attribute, such the scenegraph component will be updated using executeEvent too
	/*GameEvent event(GameEvent::E_SET_TRANSFORMATION, &GameEventData(trans.x, 16), 0);*/

	GameEventData* ged = new GameEventData(trans.x, 16);
	GameEvent event(GameEvent::E_SET_TRANSFORMATION, ged, this);
	// Check if the new transformation can be set
	if (m_owner->checkEvent(&event))
	{
		// Apply transformation
		memcpy( m_transformation, trans.x, sizeof( float ) * 16 );
		// Send transformation to horde
		sendTransformation();
		// and to the other components
		m_owner->executeEvent(&event);
	}
	delete ged;
}

void SceneGraphComponent::setRotation(const Vec3f* rotation)
{	
	Vec3f scale = Matrix4f(m_transformation).getScale();	
	Matrix4f trans = Matrix4f::ScaleMat( scale.x, scale.y, scale.z );
	trans = trans * Matrix4f(Quaternion(degToRad(rotation->x), degToRad(rotation->y), degToRad(rotation->z)));
	trans.translate(m_transformation[12], m_transformation[13], m_transformation[14]);
	
	// Create event without sender attribute, such the scenegraph component will be updated using executeEvent too
	//GameEvent event(GameEvent::E_SET_TRANSFORMATION, &GameEventData(trans.x, 16), 0);

	GameEventData* ged = new GameEventData(trans.x, 16);
		GameEvent event(GameEvent::E_SET_TRANSFORMATION, ged, this);
	//GameEvent event(GameEvent::E_SET_TRANSFORMATION, &GameEventData(trans.x, 16), this);
	// Check if the new transformation can be set
	if (m_owner->checkEvent(&event))
	{
		// Apply transformation
		memcpy( m_transformation, trans.x, sizeof( float ) * 16 );
		// Send transformation to horde
		sendTransformation();
		// and to the other components
		m_owner->executeEvent(&event);
	}
	delete ged;
}

void SceneGraphComponent::setParentNode(const Attach* data)
{
	// Default parent is the root node
	H3DNode otherNode = H3DRootNode;
	
	// Get the horde id of the new parent
	GameEntity* entity = GameModules::gameWorld()->entity(data->EntityID);
	if (entity)
	{
		SceneGraphComponent* component = static_cast<SceneGraphComponent*>(entity->component("Horde3D"));
		if (component)
			otherNode = component->hordeId();
	}

	// Set the parent node in any case
	setParentNode(otherNode, data);
}

void SceneGraphComponent::attach(const Attach* data)
{
	// Get the real horde id
	H3DNode otherNode = H3DRootNode;
	GameEntity* entity = GameModules::gameWorld()->entity(data->EntityID);
	if (entity && m_hordeID > 0)
	{
		SceneGraphComponent* component = static_cast<SceneGraphComponent*>(entity->component("Horde3D"));
		if (component)
		{
			component->setParentNode(m_hordeID, data);
			//otherNode = component->hordeId();
		}
	}
	
	/*if(data->Child && strcmp(data->Child,"") != 0 )
	{
		h3dFindNodes( m_hordeID, data->Child, H3DNodeTypes::Undefined );
		H3DNode child = h3dGetNodeFindResult(0);
		h3dSetNodeParent(otherNode, child);
	}
	else
	{
		h3dSetNodeParent(otherNode, m_hordeID);
	}
	
	h3dSetNodeTransform(otherNode,data->Tx,data->Ty, data->Tz,
		data->Rx, data->Ry, data->Rz,
		data->Sx, data->Sy, data->Sz);*/
}

void SceneGraphComponent::setParentNode(H3DNode newParentNode, const Attach* data)
{
	if (m_hordeID > 0)
	{
		if(data->Child && strcmp(data->Child,"") != 0 )
		{
			int nodes = h3dFindNodes( newParentNode, data->Child, H3DNodeTypes::Undefined );
			H3DNode child = h3dGetNodeFindResult(0);
	
			h3dSetNodeParent(m_hordeID, child);
		}
		else
		{
			h3dSetNodeParent(m_hordeID, newParentNode);
		}

		h3dSetNodeTransform(m_hordeID,data->Tx,data->Ty, data->Tz,
			data->Rx, data->Ry, data->Rz,
			data->Sx, data->Sy, data->Sz);

		checkTransformation();
	}
}


void SceneGraphComponent::setEnabled(bool enable)
{
	if (m_hordeID > 0)
	{
		// Not sure if we want to use recursive == true or false here
		// Also not sure about the visibility state
		h3dSetNodeFlags(m_hordeID, enable ? 0 : H3DNodeFlags::Inactive, true );
	}
}

void SceneGraphComponent::unloadTerrainGeoRes()
{
	if( m_terrainGeoRes != 0 )
	{
		h3dRemoveResource( m_terrainGeoRes );
		h3dReleaseUnusedResources();
		m_terrainGeoRes = 0;
	}
}

bool SceneGraphComponent::getVisibility()
{
	if (m_visibilityFlag == 0)
	{
#ifdef IGNORE_CAM_CHANGE_FOR_VISIBILITY
		static unsigned int camID = SceneGraphManager::instance()->getActiveCam();
		bool occCulling = SceneGraphManager::instance()->getActiveCam() == camID;
		if (h3dCheckNodeVisibility(hordeId(), camID, occCulling, false)!= -1)
			m_visibilityFlag = 1;
#else
		if (h3dCheckNodeVisibility(hordeId(), SceneGraphManager::instance()->getActiveCam(), true, false) != -1)
			m_visibilityFlag = 1;
#endif
		else
			m_visibilityFlag = 2;
	}
	return (m_visibilityFlag == 1);
}

void SceneGraphComponent::getBoundingBox(float* minX, float* minY, float* minZ, float* maxX, float* maxY, float* maxZ)
{
	h3dGetNodeAABB(hordeId(), minX, minY, minZ, maxX, maxY, maxZ);
}


void SceneGraphComponent::getSerializedState(GameState& state)
{
	// Horde3D node flags (entity enabled?)
	int nodeFlags = 0;
	if (m_hordeID > 0)
	{
		nodeFlags = h3dGetNodeFlags(m_hordeID);
	}
	state.storeInt32(nodeFlags);

	// current transformation
	for (int i = 0; i < 16; i++) {
		state.storeFloat(m_transformation[i]);
	}

	// transformation one frame ago
	for (int i = 0; i < 16; i++) {
		state.storeFloat(m_net_lasttransformation[i]);
	}

	// elapsed time of last frame
	state.storeFloat(GameEngine::FPS());
}

void SceneGraphComponent::setSerializedState(GameState& state)
{
	int nodeFlags;
	if (state.readInt32(&nodeFlags))
		return;

	if (m_hordeID > 0) {
		h3dSetNodeFlags(m_hordeID, nodeFlags, true );
	}

	float read_transformation[16];
	for (int i = 0; i < 16; i++) {
		if (state.readFloat(&read_transformation[i]))
			return;
	}
	setTransformation(read_transformation);

	// set last transformation (i. e. one frame ago)
	for (int i = 0; i < 16; i++) {
		if (state.readFloat(&read_transformation[i]))
			return;
	}
	memcpy(m_net_lasttransformation, read_transformation, sizeof(float) * 16);

	// calculate trajectory
	Vec3f t, r, s, lt, lr, ls;

	Matrix4f(m_transformation).decompose(t, r, s);
	Matrix4f(m_net_lasttransformation).decompose(lt, lr, ls);

	if ((s != ls) || (r != lr) || (t != lt))
	{
		if ((t - lt).length() > m_net_maxTrajection) {
			m_net_traject_translation.x = 0;
			m_net_traject_translation.y = 0;
			m_net_traject_translation.z = 0;
			m_net_traject_rotation.x = 0;
			m_net_traject_rotation.y = 0;
			m_net_traject_rotation.z = 0;
			m_net_traject_scale.x = 0;
			m_net_traject_scale.y = 0;
			m_net_traject_scale.z = 0;
		} else {
			m_net_traject_translation = t - lt;
			m_net_traject_rotation = r - lr;
			m_net_traject_scale = s - ls;
		}
		m_net_applyTrajection = true;
	} else
		m_net_applyTrajection = false;

	float remotefps;

	if (state.readFloat(&remotefps))	// to be able to traject accordingly to remote speed (reduces entity "jumping")
		return;

	m_net_traject_speedup = remotefps / GameEngine::FPS();

	//printf("speedup = %f\n", m_net_traject_speedup);
}

void SceneGraphComponent::traject()
{
	// apply trajections
	if (m_net_applyTrajection) {

		// apply translation
		m_transformation[12] += m_net_traject_translation.x * m_net_traject_speedup;
		m_transformation[13] += m_net_traject_translation.y * m_net_traject_speedup;
		m_transformation[14] += m_net_traject_translation.z * m_net_traject_speedup;

		// apply rotation
		Matrix4f trans( Matrix4f(m_transformation) * 
			Matrix4f(Quaternion(
				m_net_traject_rotation.x * m_net_traject_speedup,
				m_net_traject_rotation.y * m_net_traject_speedup,
				m_net_traject_rotation.z * m_net_traject_speedup
			)) );
		memcpy( m_transformation, trans.x, sizeof( float ) * 16 );

		// TODO: apply scale

		sendTransformation();

		GameEvent event(GameEvent::E_SET_TRANSFORMATION, GameEventData(m_transformation, 16), this);
		m_owner->executeEvent(&event);
	}
}

void SceneGraphComponent::setMaxTrajection(float maxTrajection) {
	m_net_maxTrajection = maxTrajection;
}
