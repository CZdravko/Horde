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
#include "utMath.h"

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



using namespace Horde3D;
using namespace Horde3DAsfAmc;





DLLEXP void h3dextBuildJointIdxMap(ResHandle asfRes) {

	Resource *aRes = Modules::resMan().resolveResHandle(asfRes);
	if (aRes == 0x0)
		return;

	((ASFResource *)aRes)->buildJointIdxMap(0x0);
}


DLLEXP void h3dextSetModel(NodeHandle modelHandle, ResHandle asfRes) {
	SceneNode *modelNode = Modules::sceneMan().resolveNodeHandle(modelHandle);
	if (modelNode == 0x0)
		return;

	Resource *aRes = Modules::resMan().resolveResHandle(asfRes);
	if (aRes == 0x0)
		return;

	((ASFResource *)aRes)->setModel((SceneNode*)modelNode);
}

DLLEXP int h3dextGetNumFrames(ResHandle asfRes) {

	Resource *aRes = Modules::resMan().resolveResHandle(asfRes);
	if (aRes == 0x0)
		return -1;

	return ((ASFResource *)aRes)->getNumFrames();
}

DLLEXP int h3dextGetNumEtities(ResHandle asfRes) {

	Resource *aRes = Modules::resMan().resolveResHandle(asfRes);
	if (aRes == 0x0)
		return -1;

	return ((ASFResource *)aRes)->getNumEntities();
}

DLLEXP void h3dextRebasePosture(ResHandle asfRes, int frameIndex, Matrix4f** frameTs, Quaternion** frameQs, bool basePosture,  bool skeletonBasePosture) {

	Resource *aRes = Modules::resMan().resolveResHandle(asfRes);
	if (aRes == 0x0)
		return;

	((ASFResource *)aRes)->rebasePosture(frameIndex, frameTs, frameQs, basePosture,  skeletonBasePosture);
}

DLLEXP void h3dextGetEntityName(ResHandle asfRes, int enitityIndex, char* name){

	Resource *aRes = Modules::resMan().resolveResHandle(asfRes);
	if (aRes == 0x0)
		return;

	((ASFResource *)aRes)->getEntityName(enitityIndex, name);
}

