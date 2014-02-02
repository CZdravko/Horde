/*
 * AMCResource.h
 *
 *  Created on: Jan 4, 2014
 *      Author: yama
 */

#ifndef AMCRESOURCE_H_
#define AMCRESOURCE_H_

#include <egResource.h>
#include "asfamc/skeleton.h"

namespace Horde3DAsfAmc {

using namespace Horde3D;

const int AMC_resType = 150;


class AMCResource: public Horde3D::Resource {
public:
	AMCResource(const std::string &name, int flags);
	virtual ~AMCResource();

	static Resource *factoryFunc(const std::string &name, int flags) {
		return new AMCResource(name, flags);
	}

	Resource *clone();

	void initDefault();
	void release();
	bool load(const char *data, int size);


private:
	Skeleton* _pSkeleton;

};

} /* namespace Horde3DAsfAmc */

#endif /* AMCRESOURCE_H_ */
