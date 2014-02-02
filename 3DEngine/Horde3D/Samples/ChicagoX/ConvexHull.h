/*
 * ConvexHull.h
 *
 *  Created on: 10. sep. 2013
 *      Author: ZCule
 */

#ifndef CONVEXHULL_H_
#define CONVEXHULL_H_
#include "GameEngine/utMath.h"


namespace Horde3D {
//count points forming a convex hull vith 2*count line segments
class ConvexHull {
public:
	Vec3f points[12];
	int connectivity[24];
	int count;

//	ConvexHull() : connectivity{ 1,2 , 0,3 , 0,4 , 1,5 , 2,6 , 3,7 , 4,8 , 5,9 , 6,10 , 7,11 , 8,11 , 9,10  }{};
//	ConvexHull(Vec3f *points, int count, int *connectivity) {
//		this->points = points;
//		this->count = count;
//		this->connectivity = connectivity;
//	}
//	;

	virtual ~ConvexHull();
};
}
#endif /* CONVEXHULL_H_ */
