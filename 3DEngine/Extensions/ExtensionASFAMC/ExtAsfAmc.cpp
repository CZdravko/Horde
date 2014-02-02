/*
 * ExtAsfAmc.cpp
 *
 *  Created on: Jan 4, 2014
 *      Author: yama
 */

#include "ExtAsfAmc.h"
#include "AMCResource.h"
#include "ASFResource.h"
#include "egModules.h"
#include "utPlatform.h"
#include <egPrerequisites.h>

namespace Horde3DAsfAmc {

using namespace Horde3D;

ExtAsfAmc::ExtAsfAmc() {
	// TODO Auto-generated constructor stub

}

ExtAsfAmc::~ExtAsfAmc() {
	// TODO Auto-generated destructor stub
}

} /* namespace Horde3DAsfAmc */

const char* Horde3DAsfAmc::ExtAsfAmc::getName() {
	return "ASFAMC";
}

bool Horde3DAsfAmc::ExtAsfAmc::init() {
	Modules::resMan().registerResType(ASF_resType, "ASF", 0x0, 0x0, ASFResource::factoryFunc);
	Modules::resMan().registerResType(AMC_resType, "AMC", 0x0, 0x0, AMCResource::factoryFunc);
	return true;
}

void Horde3DAsfAmc::ExtAsfAmc::release() {
}

namespace Horde3DAsfAmc {

using namespace Horde3D;

DLLEXP bool h3dextPrepairBonesJoints(NodeHandle modelHandle, ResHandle asfRes) {
	SceneNode *modelNode = Modules::sceneMan().resolveNodeHandle(modelHandle);
	if (modelNode == 0x0)
		return false;

	Resource *aRes = Modules::resMan().resolveResHandle(asfRes);
	if (aRes == 0x0)
		return false;

	return ((ASFResource *)aRes)->prepairBonesJoints((ModelNode*)modelNode);
}

}
