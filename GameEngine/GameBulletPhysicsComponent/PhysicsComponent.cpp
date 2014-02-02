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
// GameEngine Bullet Component of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#include "PhysicsComponent.h"

#include <GameEngine/GameLog.h>
#include <GameEngine/GameEntity.h>
#include <GameEngine/GameModules.h>
#include <GameEngine/GameComponentRegistry.h>
#include <GameEngine/GameWorld.h>
#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEngine_SceneGraph.h>

#include <GameEngine/utMath.h>

#include "Physics.h"

#include <btBulletDynamicsCommon.h>
#include <btGImpactConvexDecompositionShape.h>

#include <XMLParser/utXMLParser.h>

#include <Horde3D/Horde3D.h>
#include <Horde3D/Horde3DUtils.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif

GameComponent* PhysicsComponent::createComponent(GameEntity* owner) {
	GameLog::logMessage("CREATING BULLET COMPONENT!");
	return new PhysicsComponent(owner);
}

PhysicsComponent::PhysicsComponent(GameEntity *owner) :
		GameComponent(owner, "BulletPhysics"), m_motionState(0), m_collisionShape(0), m_btTriangleMesh(0), m_rigidBody(0), m_proxy(0), lX(0), lY(0), lZ(0) {
	owner->addListener(GameEvent::E_SET_TRANSFORMATION, this);
	owner->addListener(GameEvent::E_SET_TRANSLATION, this);
	owner->addListener(GameEvent::E_SET_ROTATION, this);
	owner->addListener(GameEvent::E_TRANSLATE_LOCAL, this);
	owner->addListener(GameEvent::E_TRANSLATE_GLOBAL, this);
	owner->addListener(GameEvent::E_ROTATE_LOCAL, this);
}

PhysicsComponent::~PhysicsComponent() {
	release();
}

bool PhysicsComponent::checkEvent(GameEvent *event) {
	//printf("checking event\n");
	if (m_rigidBody == 0 || m_rigidBody->getActivationState() == DISABLE_SIMULATION)
		return true;

	// Only catch transformation if this is a dynamic object
	if (m_motionState) {
		switch (event->id()) {
		case GameEvent::E_SET_TRANSFORMATION:
			// We catch this event by using the physics engine for changing the transformation
			// this way the transformation event will not affect any other component and
			// the event's original transformation wouldn't be set
			setTransformation(static_cast<float*>(event->data()));
			return false;
		case GameEvent::E_SET_TRANSLATION: {
			Vec3f* translation = static_cast<Vec3f*>(event->data());
			setTranslation(translation->x, translation->y, translation->z);
		}
			return false;
		case GameEvent::E_SET_ROTATION: {
			Vec3f* rotation = static_cast<Vec3f*>(event->data());
			setRotation(rotation->x, rotation->y, rotation->z);
		}
			return false;
		}
	}
	return true;
}

void PhysicsComponent::executeEvent(GameEvent *event) {
	if (m_rigidBody == 0)
		return;

	switch (event->id()) {
	case GameEvent::E_SET_TRANSFORMATION:
		setTransformation(static_cast<const float*>(event->data()));
		break;
	case GameEvent::E_SET_TRANSLATION: {
		Vec3f* translation = static_cast<Vec3f*>(event->data());
		setTranslation(translation->x, translation->y, translation->z);
	}
		break;
	case GameEvent::E_SET_ROTATION: {
		Vec3f* rotation = static_cast<Vec3f*>(event->data());
		setRotation(rotation->x, rotation->y, rotation->z);
	}
		break;
	}
}

void PhysicsComponent::update() {
//	printf("m_rigidBody->isActive(): %d\n", m_rigidBody->isActive());
	if (!m_proxy)
		if (m_owner == 0 || m_rigidBody == 0 || !m_rigidBody->isActive())
			return;

	// Get Transformation from Bullet
	btTransform trans;
	m_motionState->getWorldTransform(trans);

	// Send Transformation to other components
	sendTransformation(trans, false);
}

void PhysicsComponent::reset() {
	if (m_rigidBody && m_motionState) {
		// Reset the transformation to it's initial state
		sendTransformation(m_motionState->m_startWorldTrans, true);
	}
}

void PhysicsComponent::setEnabled(bool enable) {
	if (m_rigidBody) {
		if (enable)
			m_rigidBody->activate();
		else if (!enable /*&& m_rigidBody->getActivationState() != DISABLE_SIMULATION*/)
			m_rigidBody->forceActivationState(ISLAND_SLEEPING);
	}
}

void PhysicsComponent::setCollisionResponse(bool enable) {
	m_CF_Disabled = !enable;
	if (!enable) {
		m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	} else {
		m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}
void PhysicsComponent::resetForce() {
	if (m_rigidBody) {
		m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
		m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
	}
}

void PhysicsComponent::applyForce(const float x, const float y, const float z, const float dx, const float dy, const float dz) {
	if (m_rigidBody) {
		m_rigidBody->applyForce(btVector3(x, y, z), btVector3(dx, dy, dz));
		m_rigidBody->activate();
	}
}

void PhysicsComponent::applyImpulse(const float x, const float y, const float z, const float dx, const float dy, const float dz) {
	if (m_rigidBody) {
		m_rigidBody->applyImpulse(btVector3(x, y, z), btVector3(dx, dy, dz));
		m_rigidBody->activate();
	}
}

void PhysicsComponent::loadFromXml(const XMLNode* description) {
	GameLog::logMessage("LOADING BULLET FROM XML");
	// Delete old physics representation
	//release();

	Matrix4f objTrans;
	m_owner->executeEvent(&GameEvent(GameEvent::E_TRANSFORMATION, &GameEventData((float*) objTrans.x, 16), this));

	Vec3f t, r, s;
	objTrans.decompose(t, r, s);

	// Parse Physics Node Configuration
	float mass = static_cast<float>(atof(description->getAttribute("mass", "0.0")));

	const char* shape = description->getAttribute("shape", "Box");
	// create collision shape based on the node configuration
	if (shape && _stricmp(shape, "Box") == 0) // Bounding Box Shape
			{
		float dimX = static_cast<float>(atof(description->getAttribute("x", "1.0")));
		float dimY = static_cast<float>(atof(description->getAttribute("y", "1.0")));
		float dimZ = static_cast<float>(atof(description->getAttribute("z", "1.0")));
		// update box settings with node scaling (TODO is this necessary if we already set the scale by using setLocalScaling?)
		//m_collisionShape = new btBoxShape(btVector3(dimX * s.x, dimY * s.y, dimZ * s.z));
		m_collisionShape = new btBoxShape(btVector3(dimX, dimY, dimZ));
	} else if (shape && _stricmp(shape, "Sphere") == 0) // Sphere Shape
			{
		float radius = static_cast<float>(atof(description->getAttribute("radius", "1.0")));
		m_collisionShape = new btSphereShape(radius);
	} else if (shape && _stricmp(shape, "Cylinder") == 0) // Cylinder Shape
			{
		float radius0 = static_cast<float>(atof(description->getAttribute("radius", "1.0")));
		float height = static_cast<float>(atof(description->getAttribute("height", "1.0")));
		m_collisionShape = new btCylinderShape(btVector3(radius0, height, radius0));
	} else // Mesh Shape
	{
		MeshData meshData;
		GameEvent meshEvent(GameEvent::E_MESH_DATA, &meshData, this);
		// get mesh data from graphics engine
		m_owner->executeEvent(&meshEvent);

		if (meshData.VertexBase && (meshData.TriangleBase32 || meshData.TriangleBase16)) {
			// Create new mesh in physics engine
			m_btTriangleMesh = new btTriangleMesh();
			int offset = 3;
			if (meshData.TriangleMode == 5) // Triangle Strip
				offset = 1;

			// copy mesh from graphics to physics
			bool index16 = false;
			if (meshData.TriangleBase16)
				index16 = true;
			for (unsigned int i = 0; i < meshData.NumTriangleIndices - 2; i += offset) {
				unsigned int index1 = index16 ? (meshData.TriangleBase16[i] - meshData.VertRStart) * 3 : (meshData.TriangleBase32[i] - meshData.VertRStart) * 3;
				unsigned int index2 = index16 ? (meshData.TriangleBase16[i + 1] - meshData.VertRStart) * 3 : (meshData.TriangleBase32[i + 1] - meshData.VertRStart) * 3;
				unsigned int index3 = index16 ? (meshData.TriangleBase16[i + 2] - meshData.VertRStart) * 3 : (meshData.TriangleBase32[i + 2] - meshData.VertRStart) * 3;
				m_btTriangleMesh->addTriangle(btVector3(meshData.VertexBase[index1], meshData.VertexBase[index1 + 1], meshData.VertexBase[index1 + 2]),
						btVector3(meshData.VertexBase[index2], meshData.VertexBase[index2 + 1], meshData.VertexBase[index2 + 2]),
						btVector3(meshData.VertexBase[index3], meshData.VertexBase[index3 + 1], meshData.VertexBase[index3 + 2]));
			}

			bool useQuantizedAabbCompression = true;

			if (mass > 0) {
				//btGImpactMeshShape* shape = new btGImpactMeshShape(m_btTriangleMesh);

				btGImpactConvexDecompositionShape* shape = new btGImpactConvexDecompositionShape(m_btTriangleMesh, btVector3(1.f, 1.f, 1.f), btScalar(0.1f), true);

				shape->updateBound();

				//btCollisionDispatcher* dispatcher = static_cast<btCollisionDispatcher *>(Physics::instance()->dispatcher());
				//btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

				m_collisionShape = shape;

				//m_collisionShape = new btConvexTriangleMeshShape(m_btTriangleMesh);
			} else
				// BvhTriangleMesh can be used only for static objects
				m_collisionShape = new btBvhTriangleMeshShape(m_btTriangleMesh, useQuantizedAabbCompression);
		} else {
			GameLog::errorMessage("The mesh data for the physics representation couldn't be retrieved");
			return;
		}
	}

	GameLog::logMessage("PARSED SHAPE FROM XML");

	bool kinematic = _stricmp(description->getAttribute("kinematic", "false"), "true") == 0 || _stricmp(description->getAttribute("kinematic", "0"), "1") == 0;

	bool nondynamic = _stricmp(description->getAttribute("static", "false"), "true") == 0 || _stricmp(description->getAttribute("static", "0"), "1") == 0;

	bool ragdoll = _stricmp(description->getAttribute("ragdoll", "false"), "true") == 0 || _stricmp(description->getAttribute("ragdoll", "0"), "1") == 0;
	// Create initial transformation without scale
	btTransform tr;
	tr.setIdentity();
	tr.setRotation(btQuaternion(r.x, r.y, r.z));

	btMatrix3x3 rot = tr.getBasis();

	XMLNode offsetXMLNode = description->getChildNode("Offset");

	if (!offsetXMLNode.isEmpty()) {
		lX = static_cast<float>(atof(offsetXMLNode.getAttribute("lX", "0.0")));
		lY = static_cast<float>(atof(offsetXMLNode.getAttribute("lY", "0.0")));
		lZ = static_cast<float>(atof(offsetXMLNode.getAttribute("lZ", "0.0")));
	}

	btVector3 offset = btVector3(lX * s.x, lY * s.y, lZ * s.z);

	tr.setOrigin(btVector3(t.x, t.y, t.z) + rot * offset);
	// Set local scaling in collision shape because Bullet does not support scaling in the world transformation matrices
	m_collisionShape->setLocalScaling(btVector3(s.x, s.y, s.z));
	btVector3 localInertia(0, 0, 0);
	//rigidbody is dynamic if and only if mass is non zero otherwise static
	if (mass != 0)
		m_collisionShape->calculateLocalInertia(mass, localInertia);
	if (mass != 0 || kinematic)
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		m_motionState = new btDefaultMotionState(tr);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_motionState, m_collisionShape, localInertia);
	rbInfo.m_startWorldTransform = tr;
	//rbInfo.m_restitution = btScalar( atof(description->getAttribute("restitution", "0")) );
	//rbInfo.m_friction = btScalar( atof(description->getAttribute("static_friction", "0.5")) );
	// Threshold for deactivation of objects (if movement is below this value the object gets deactivated)
	//rbInfo.m_angularSleepingThreshold = 0.8f;
	//rbInfo.m_linearSleepingThreshold = 0.8f;

	m_rigidBody = new btRigidBody(rbInfo);
	m_rigidBody->setUserPointer(this);
	m_rigidBody->setDeactivationTime(2.0f);

	// Add support for collision detection if mass is zero but kinematic is explicitly enabled
	if (kinematic && mass == 0 && !nondynamic) {
		m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
	}
	if (nondynamic && mass == 0) {
		m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
	}

	bool isTrigger = _stricmp(description->getAttribute("solid", "true"), "false") == 0 || _stricmp(description->getAttribute("solid", "1"), "0") == 0;
	if (isTrigger) {
		setCollisionResponse(true);
	}

	GameLog::logMessage("I'm a new Physics body: %s my Motion state: %d", m_owner->id().c_str(), m_motionState);
	printf("I'm a new Physics body: %s my Motion state: %d\n", m_owner->id().c_str(), m_motionState);

	if(!ragdoll) Physics::instance()->addObject(this);

	/*Geometry Proxy*/
	XMLNode proxyXMLNode = description->getChildNode("Proxy");
	if (proxyXMLNode.isEmpty())
		return;

	m_proxy = GameModules::gameWorld()->entity(proxyXMLNode.getAttribute("name", ""));

	if (m_proxy) {
		m_proxy->addListener(GameEvent::E_SET_TRANSFORMATION, this);
		m_proxy->addListener(GameEvent::E_SET_TRANSLATION, this);
		m_proxy->addListener(GameEvent::E_SET_ROTATION, this);
		m_proxy->addListener(GameEvent::E_TRANSLATE_LOCAL, this);
		m_proxy->addListener(GameEvent::E_TRANSLATE_GLOBAL, this);
		m_proxy->addListener(GameEvent::E_ROTATE_LOCAL, this);
	} else
		printf("No PROXY FOUND with EntityID: %s\n", proxyXMLNode.getAttribute("name", ""));

	/*Adding constraints*/

	XMLNode constraintXMLNode = description->getChildNode("Constraint");
	if (constraintXMLNode.isEmpty())
		return;

	const char* constraintType = constraintXMLNode.getAttribute("type", "Hinge");
	const char* parentName = constraintXMLNode.getAttribute("parent", "");
	PhysicsComponent* parent = getParent(parentName);

	if (!parent) {
		printf("NO PARENT FOUND\n");
		return;
	}

	XMLNode transformXMLNode = constraintXMLNode.getChildNode("TransformA");
	float qX = static_cast<float>(atof(transformXMLNode.getAttribute("qx", "1.0")));
	float qY = static_cast<float>(atof(transformXMLNode.getAttribute("qy", "1.0")));
	float qZ = static_cast<float>(atof(transformXMLNode.getAttribute("qz", "1.0")));
	float qW = static_cast<float>(atof(transformXMLNode.getAttribute("qw", "1.0")));
	float vX = static_cast<float>(atof(transformXMLNode.getAttribute("vx", "1.0")));
	float vY = static_cast<float>(atof(transformXMLNode.getAttribute("vy", "1.0")));
	float vZ = static_cast<float>(atof(transformXMLNode.getAttribute("vz", "1.0")));
	btTransform transformA;
	transformA.setIdentity();
	printf("%f \t %f \t %f  \n ", qX, qY, qZ);
	transformA.getBasis().setEulerZYX(qZ, qY, qX);
//	transformA.getBasis().setEulerZYX(M_PI_2,0,0);
	transformA.setOrigin(btVector3(vX * s.x, vY * s.y, vZ * s.z));
//	printf("%f \t %f \t %f \t %s \n ", vX*s.x, vY*s.y, vZ*s.z, m_owner ? m_owner->id().c_str() : "no name");
//	btTransform transformA = btTransform(btQuaternion(qX, qY, qZ, qW), btVector3(vX*s.x, vY*s.y, vZ*s.z));

	transformXMLNode = constraintXMLNode.getChildNode("TransformB");
	qX = static_cast<float>(atof(transformXMLNode.getAttribute("qx", "1.0")));
	qY = static_cast<float>(atof(transformXMLNode.getAttribute("qy", "1.0")));
	qZ = static_cast<float>(atof(transformXMLNode.getAttribute("qz", "1.0")));
	qW = static_cast<float>(atof(transformXMLNode.getAttribute("qw", "1.0")));
	vX = static_cast<float>(atof(transformXMLNode.getAttribute("vx", "1.0")));
	vY = static_cast<float>(atof(transformXMLNode.getAttribute("vy", "1.0")));
	vZ = static_cast<float>(atof(transformXMLNode.getAttribute("vz", "1.0")));
//	btTransform transformB = btTransform(btQuaternion(qX, qY, qZ, qW), btVector3(vX*s.x, vY*s.y, vZ*s.z));
	btTransform transformB;
	transformB.setIdentity();
	transformB.getBasis().setEulerZYX(qZ, qY, qX);
//	transformB.getBasis().setEulerZYX(M_PI_2,0,0);
	transformB.setOrigin(btVector3(vX * s.x, vY * s.y, vZ * s.z));

	XMLNode limitXMLNode = constraintXMLNode.getChildNode("Limit");

	if (_stricmp(constraintType, "Hinge") == 0) {
		btHingeConstraint* parentConstraint = new btHingeConstraint(*(parent->rigidBody()), *m_rigidBody, transformA, transformB);
		float low = static_cast<float>(atof(limitXMLNode.getAttribute("low", "0.0")));
		float high = static_cast<float>(atof(limitXMLNode.getAttribute("high", "0.75")));
		float softness = static_cast<float>(atof(limitXMLNode.getAttribute("softness", "0.9")));
		float biasFactor = static_cast<float>(atof(limitXMLNode.getAttribute("biasFactor", "0.3")));
		float relaxationFactor = static_cast<float>(atof(limitXMLNode.getAttribute("relaxationFactor", "1.0")));

		parentConstraint->setLimit(low, high, softness, biasFactor, relaxationFactor);
		m_parentConstraint = parentConstraint;
	} else if (_stricmp(constraintType, "ConeTwist") == 0) {
		btConeTwistConstraint* parentConstraint = new btConeTwistConstraint(*(parent->rigidBody()), *m_rigidBody, transformA, transformB);
		float swingSpan1 = static_cast<float>(atof(limitXMLNode.getAttribute("swingSpan1", "1.0")));
		float swingSpan2 = static_cast<float>(atof(limitXMLNode.getAttribute("swingSpan2", "1.0")));
		float twistSpan = static_cast<float>(atof(limitXMLNode.getAttribute("twistSpan", "1.0")));
		float softness = static_cast<float>(atof(limitXMLNode.getAttribute("softness", "0.9")));
		float biasFactor = static_cast<float>(atof(limitXMLNode.getAttribute("biasFactor", "0.3")));
		float relaxationFactor = static_cast<float>(atof(limitXMLNode.getAttribute("relaxationFactor", "1.0")));

		parentConstraint->setLimit(swingSpan1, swingSpan2, twistSpan, softness, biasFactor, relaxationFactor);
		m_parentConstraint = parentConstraint;
	}

	if(!ragdoll) Physics::instance()->addConstraint(m_parentConstraint);

}

void PhysicsComponent::release() {
	Physics::instance()->removeObject(this);
	delete m_rigidBody;
	m_rigidBody = 0;
	delete m_motionState;
	m_motionState = 0;
	delete m_collisionShape;
	m_collisionShape = 0;
	delete m_btTriangleMesh;
	m_btTriangleMesh = 0;
	if (m_parentConstraint != NULL)
		delete m_parentConstraint;
}

void PhysicsComponent::sendTransformation(const btTransform& transformation, bool apply) {
#ifdef BT_USE_DOUBLE_PRECISION
	double m[16];
	transformation.getBasis().scaled(m_collisionShape->getLocalScaling()).getOpenGLSubMatrix(m);
	float x[16];
	x[0] = m[0]; x[1] = m[1]; x[2] = m[2]; x[3] = m[3];
	x[4] = m[4]; x[5] = m[5]; x[6] = m[6]; x[7] = m[7];
	x[8] = m[8]; x[9] = m[9]; x[10] = m[10]; x[11] = m[11];
	x[12] = transformation.getOrigin().x();
	x[13] = transformation.getOrigin().y();
	x[14] = transformation.getOrigin().z();
	x[15] = 1.0f;
#else
	float x[16];
	btMatrix3x3 scaledBasis = transformation.getBasis().scaled(m_collisionShape->getLocalScaling());
	btVector3 voffset = scaledBasis * btVector3(lX, lY, lZ);
	transformation.getBasis().scaled(m_collisionShape->getLocalScaling()).getOpenGLSubMatrix(x);

	x[12] = transformation.getOrigin().x();
	x[13] = transformation.getOrigin().y();
	x[14] = transformation.getOrigin().z();
	x[15] = 1.0f;
#endif
	// Merge scaling with scale free transformation
#ifdef DEBUG
	GameEvent proxy_event(GameEvent::E_SET_TRANSFORMATION, &GameEventData(x, 16), this);

	if (m_proxy) {
//		printf("PhysicsComponent::update: %.3f, %.3f, %.3f\n", x[12], x[13], x[14]);
//		printf("SENDING PROXY A TRANSFORMATION");
		if (m_proxy->checkEvent(&proxy_event))
			m_proxy->executeEvent(&proxy_event);

		//return;
	}
#endif

	x[12] = transformation.getOrigin().x() - voffset.x();
	x[13] = transformation.getOrigin().y() - voffset.y();
	x[14] = transformation.getOrigin().z() - voffset.z();

	GameEvent event(GameEvent::E_SET_TRANSFORMATION, &GameEventData(x, 16), this);

	if (m_owner->checkEvent(&event)) {
//		printf("PhysicsComponent::update: %.3f, %.3f, %.3f\n", x[12], x[13], x[14]);
		m_owner->executeEvent(&event);
		if (apply)
			setTransformation(x);
	} else {
		// Reset Physics State to original transformation
		GameEvent e(GameEvent::E_TRANSFORMATION, GameEventData((float*) x, 16), this);
		m_owner->executeEvent(&e);
		setTransformation(x);
		// deactivate object until it is reactivated again by another collision or physics adjustment
		m_rigidBody->setActivationState(WANTS_DEACTIVATION);
	}

}

void PhysicsComponent::setTranslation(const float x, const float y, const float z) {
	btTransform transform;
	if (m_motionState)
		m_motionState->getWorldTransform(transform);
	else
		transform = m_rigidBody->getWorldTransform();

	transform.setOrigin(btVector3(x, y, z));

	if (m_motionState)
		m_motionState->setWorldTransform(transform);
	m_rigidBody->setWorldTransform(transform);
	m_rigidBody->setInterpolationWorldTransform(transform);
	if (!m_rigidBody->isStaticObject()) {
		m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
		m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
		m_rigidBody->activate(true);
	}
}

void PhysicsComponent::setRotation(const float x, const float y, const float z) {
	btTransform transform;
	if (m_motionState)
		m_motionState->getWorldTransform(transform);
	else
		transform = m_rigidBody->getWorldTransform();

	// TODO maybe this order is not correct (UNTESTED)
	transform.setRotation(btQuaternion(x, y, z));

	if (m_motionState)
		m_motionState->setWorldTransform(transform);
	m_rigidBody->setWorldTransform(transform);
	m_rigidBody->setInterpolationWorldTransform(transform);
	if (!m_rigidBody->isStaticObject()) {
		m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
		m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
		m_rigidBody->activate(true);
	}
}

void PhysicsComponent::setTransformation(const float* m) {
	// we need a scale free rotation

	float sx = sqrtf(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
	float sy = sqrtf(m[4] * m[4] + m[5] * m[5] + m[6] * m[6]);
	float sz = sqrtf(m[8] * m[8] + m[9] * m[9] + m[10] * m[10]);

	// TODO Fix this
	btVector3 localScaling = m_collisionShape->getLocalScaling();
	if (sx != localScaling.x() || sy != localScaling.y() || sz != localScaling.z())
		m_collisionShape->setLocalScaling(btVector3(sx, sy, sz));

	btMatrix3x3 rotation(m[0] / sx, m[4] / sy, m[8] / sz, m[1] / sx, m[5] / sy, m[9] / sz, m[2] / sx, m[6] / sy, m[10] / sz);

	btVector3 offset(lX * sx, lY * sy, lZ * sz);
	btVector3 origin = btVector3(m[12], m[13], m[14]) + rotation * offset;

	btTransform transform(rotation, origin);	// + ovec
	//printf("PhysicsComponent::setTransformation\n\t %.3f, %.3f, %.3f\n", m[12], m[13], m[14]);	

	if (m_motionState)
		m_motionState->setWorldTransform(transform);
	m_rigidBody->setWorldTransform(transform);
	m_rigidBody->setInterpolationWorldTransform(transform);
	if (!m_rigidBody->isStaticObject()) {
		m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
		m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
		m_rigidBody->activate(true);
	}
}

PhysicsComponent* PhysicsComponent::getParent(const char* parentName) {
//	printf("Parent WID: %d\n", GameEngine::entityWorldID(parentName));
//	printf("Parent Entity: %d\n", GameModules::gameWorld()->entity( GameEngine::entityWorldID(parentName) ));
	GameEntity* parentGameEntity = GameModules::gameWorld()->entity(GameEngine::entityWorldID(parentName));

	if (parentGameEntity)
		return ((PhysicsComponent*) parentGameEntity->component("BulletPhysics"));
	else
		return 0;
}
