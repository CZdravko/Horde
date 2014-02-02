/*
 * PointLight.h
 *
 *  Created on: 12. sep. 2013
 *      Author: ZCule
 */

#ifndef POINTLIGHT_H_
#define POINTLIGHT_H_


#include "GameEngine/utMath.h"

namespace Horde3D {
class PointLight {
public:
	int lightIndex;
	float r;
	Vec3f position;

	PointLight(Vec3f position, float r, int lightIndex);

	PointLight();


	virtual ~PointLight();


};
}
#endif /* POINTLIGHT_H_ */
