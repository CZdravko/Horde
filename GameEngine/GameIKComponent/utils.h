
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
// GameEngine IK Component of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2011 Ionut Damian
//
// ****************************************************************************************

#ifndef IK_UTILS_H_
#define IK_UTILS_H_

#include <GameEngine/utMath.h>

using namespace Horde3D;

/*
 * Utility functions for the AgentComponent
 */
namespace utils
{
	struct Axis
	{
		enum List
		{
			X = 0,
			Y,
			Z
		};
	};

	///char array compare function for the map
	struct mapstrcmp
	{
		bool operator()(const char* a, const char* b) const;
	};

	///copies the characters form src to dst. Stops either when it gets to src's end or when it reaches dstSize
	void strcpy(const char* src, char* dst, unsigned int dstSize);
	///copies the characters form str1 and str2 to dst. Stops either when it gets to str2's end or when it reaches dstSize
	void concat(const char* str1, const char* str2, char* dst, unsigned int dstSize);

	///converst str to lower case
	void toLowerCase( char* str );
	///converst str to upper case
	void toUpperCase( char* str );

	///returns the current position in the scenegraph of the given entity
	Vec3f getEntityPosition(int eID);
	///returns the current rotation in the scenegraph of the given entity
	Vec3f getEntityRotation(int eID);
	///sets the given position in the scenegraph
	void setEntityRotation(int eID, Vec3f newRot);
	///sets the given rotation in the scenegraph
	void setEntityPosition(int eID, Vec3f newPos);

	///computes and returns the distance between a and b
	float getDistance(Vec3f a, Vec3f b);

	///converst v from radians to degrees
	void radToDegV(Vec3f* v);
	///converst v from degrees to radians
	void degToRadV(Vec3f* v);

	///computes whether (x,y) lies inside the circle of center (a,b) and radius rad
	bool inCircle(float x, float y, float a, float b, float rad);
	///x*x
	float sqr(float x);

	///returns the up axis as specified in the configs
	Axis::List getLEyeHorizAxis(const char* upAxis_str);
	Axis::List getREyeHorizAxis(const char* upAxis_str);
}

#endif
