
// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2008
//
// This file is part of the GameEngine of the University of Augsburg
//
// You are not allowed to redistribute the code, if not explicitly authorized by the author
//
// ****************************************************************************************

// ****************************************************************************************
//
// GameEngine Agent Component of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2011 Ionut Damian
//
// ****************************************************************************************

#include "utils.h"
#include "GameEngine/GameEvent.h"
#include "GameEngine/GameEngine.h"
#include <GameEngine/GameEngine_SceneGraph.h>
#include "Horde3D/Horde3D.h"
#include "Config.h"
#include <string>

void utils::strcpy(const char* src, char* dst, unsigned int dstSize)
{
	unsigned int i;
	for(i=0; i< dstSize-1 && i< strlen(src); i++) //we go up to dstSize-1 because we need the last byte for '\0'
		dst[i] = src[i];

	dst[i] = '\0';
}

void utils::concat(const char* str1, const char* str2, char* dst, unsigned int dstSize)
{
	unsigned int i;

	//copy first string
	for(i=0; i< dstSize-1, i< strlen(str1); i++) //we go untill dstSize-1 because we need the last byte for '\0'
		dst[i] = str1[i];

	//copy second string
	for(i; i< dstSize-1, i< strlen(str1) + strlen(str2); i++) //we go untill dstSize-1 because we need the last byte for '\0'
		dst[i] = str2[i];

	dst[i] = '\0';
}

void utils::toLowerCase( char* str )
{
	for(unsigned int i=0; i< strlen(str); i++)
		str[i] = tolower(str[i]);
}

void utils::toUpperCase( char* str )
{
	for(unsigned int i=0; i< strlen(str); i++)
		str[i] = toupper(str[i]);
}

bool utils::mapstrcmp::operator ()(const char *a, const char *b) const
{
	return (strcmp(a,b) < 0);
}

Vec3f utils::getEntityPosition(int eID)
{
	const float *x;
	//GameEvent getTransformation(GameEvent::E_TRANSFORMATION, x, 0);
	//GameEngine::sendEvent(eID, &getTransformation);

	//we'll query the transformaiton data from H3D directly so we get the most recent values
	unsigned int hID = GameEngine::entitySceneGraphID(eID);
	h3dGetNodeTransMats(hID, 0, &x);

	return Vec3f(x[12],x[13],x[14]);
}

Vec3f utils::getEntityRotation(int eID)
{
	float x[16];
	GameEvent getTransformation(GameEvent::E_TRANSFORMATION, x, 0);
	GameEngine::sendEvent(eID, &getTransformation);

	Matrix4f mat(x);
	Vec3f p,r,s;
	mat.decompose(p,r,s);

	return r;
}

void utils::setEntityRotation(int eID, Vec3f newRot)
{
	//unsigned int hiD = GameEngine::entitySceneGraphID(entityID);
	//Vec3f p,r,s;

	//h3dGetNodeTransform( (H3DNode)hiD, &p.x,&p.y,&p.z, &r.x,&r.y,&r.z, &s.x,&s.y,&s.z );
	//h3dSetNodeTransform( (H3DNode)hiD, p.x,p.y,p.z, reqRot.x, reqRot.y, reqRot.z, s.x,s.y,s.z );

	GameEvent rotate(GameEvent::E_SET_ROTATION, &newRot, 0);
	GameEngine::sendEvent(eID, &rotate);
};

void utils::setEntityPosition(int eID, Vec3f newPos)
{
	//GameEvent translate(GameEvent::E_TRANSLATE_GLOBAL, &newPos, 0);
	//GameEngine::sendEvent(eID, &translate);

	unsigned int hiD = GameEngine::entitySceneGraphID(eID);
	Vec3f p,r,s;

	h3dGetNodeTransform( (H3DNode)hiD, &p.x,&p.y,&p.z, &r.x,&r.y,&r.z, &s.x,&s.y,&s.z );
	h3dSetNodeTransform( (H3DNode)hiD, newPos.x,newPos.y,newPos.z, r.x,r.y,r.z, s.x,s.y,s.z );
};

float utils::getDistance(Vec3f a, Vec3f b)
{
	return (a-b).length();
	//return sqrtf( abs((float)(b.x - a.x)) * abs((float)(b.y - a.y)) * abs((float)(b.z - a.z)) );
}

void utils::radToDegV(Vec3f* v)
{
	v->x = radToDeg(v->x);
	v->y = radToDeg(v->y);
	v->z = radToDeg(v->z);
}

void utils::degToRadV(Vec3f* v)
{
	v->x = degToRad(v->x);
	v->y = degToRad(v->y);
	v->z = degToRad(v->z);
}

bool utils::inCircle(float x, float y, float a, float b, float rad)
{
	return ( sqr(x - a) + sqr(y - b) <= sqr(rad) );
}

float utils::sqr(float x)
{
	return x*x;
}

utils::Axis::List utils::getLEyeHorizAxis(const char* upAxis_str)
{
	char upAxis = toupper(upAxis_str[0]);

	if(upAxis == 'X')
		return Axis::X;
	else if(upAxis == 'Y')
		return Axis::Y;
	else if(upAxis == 'Z')
		return Axis::Z;

	return Axis::Y;
}

utils::Axis::List utils::getREyeHorizAxis(const char* upAxis_str)
{
	char upAxis = toupper(upAxis_str[0]);

	if(upAxis == 'X')
		return Axis::X;
	else if(upAxis == 'Y')
		return Axis::Y;
	else if(upAxis == 'Z')
		return Axis::Z;

	return Axis::Y;
}
