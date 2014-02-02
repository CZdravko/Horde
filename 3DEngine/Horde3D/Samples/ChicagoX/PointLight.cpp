/*
 * PointLight.cpp
 *
 *  Created on: 12. sep. 2013
 *      Author: ZCule
 */

#include "PointLight.h"


namespace Horde3D {
PointLight::PointLight() {
	// TODO Auto-generated constructor stub

}

PointLight::PointLight(Vec3f position, float r, int lightIndex) {
	this->position = position;
	this->r = r;
	this->lightIndex = lightIndex;
}

PointLight::~PointLight() {
	// TODO Auto-generated destructor stub
}

}

