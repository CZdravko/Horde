/*
 * Spotlight.h
 *
 *  Created on: 5. sep. 2013
 *      Author: ZCule
 */

#ifndef SPOTLIGHT_H_
#define SPOTLIGHT_H_

#include "GameEngine/utMath.h"


namespace Horde3D {
class Spotlight {
public:
	Spotlight();
	virtual ~Spotlight();

	Vec3f V, T1, T2, T3, T4, dir;
	float height, r;
	int lightIndex;
};
}
#endif /* SPOTLIGHT_H_ */
