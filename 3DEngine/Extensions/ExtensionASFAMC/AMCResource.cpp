/*
 * AMCResource.cpp
 *
 *  Created on: Jan 4, 2014
 *      Author: yama
 */

#include "AMCResource.h"
#include <egResource.h>
#include "asfamc/skeleton.h"

namespace Horde3DAsfAmc {

using namespace Horde3D;


AMCResource::~AMCResource() {
	// TODO Auto-generated destructor stub
}

} /* namespace Horde3DAsfAmc */

Horde3DAsfAmc::AMCResource::AMCResource(const std::string& name, int flags) :
			Resource( AMC_resType, name, flags ) , _pSkeleton(0x0) {
}



void Horde3DAsfAmc::AMCResource::initDefault() {
}

void Horde3DAsfAmc::AMCResource::release() {
}

bool Horde3DAsfAmc::AMCResource::load(const char* data, int size) {


	return true;
}

Horde3D::Resource* Horde3DAsfAmc::AMCResource::clone() {

	AMCResource *res = new AMCResource("", _flags);

			*res = *this;

			return res;
}
