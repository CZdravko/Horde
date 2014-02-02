/*
 * Plane.h
 *
 *  Created on: 9. sep. 2013
 *      Author: ZCule
 */

#ifndef PLANE_H_
#define PLANE_H_
#include "GameEngine/utMath.h"



class Plane {
public:

	Horde3D::Vec3f *n;
	float d;

	Plane();
	Plane(Horde3D::Vec3f *n, float d);
	virtual ~Plane();
};

#endif /* PLANE_H_ */
