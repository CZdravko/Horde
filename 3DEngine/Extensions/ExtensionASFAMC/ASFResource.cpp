/*
 * ASFResource.cpp
 *
 *  Created on: Jan 4, 2014
 *      Author: yama
 */

#include "ASFResource.h"
//#include <egResource.h>
#include "asfamc/skeleton.h"
#include <egPrerequisites.h>
#include <egModel.h>
#include <egModules.h>
#include <utMath.h>

#include <fstream>
#include <cstdio>
#include <cstring>

using namespace Horde3D;

namespace Horde3DAsfAmc {

ASFResource::~ASFResource() {
	// TODO Auto-generated destructor stub
}

} /* namespace Horde3DAsfAmc */

Horde3DAsfAmc::ASFResource::ASFResource(const std::string& name, int flags) :
		Resource(ASF_resType, name, flags), _pSkeleton(0x0) {
}

//Resource* Horde3DAsfAmc::ASFResource::clone() {
//	ASFResource *res = new ASFResource("", _flags);
//
//	*res = *this;
//
//	return res;
//}

void Horde3DAsfAmc::ASFResource::initDefault() {
	joint_boneMap.insert(pair<string, string>("Bip_01_Pelvis", "ROOT"));
}

void Horde3DAsfAmc::ASFResource::release() {
}

bool Horde3DAsfAmc::ASFResource::load(const char* data, int size) {
	if (!Resource::load(data, size))
		return false;

	char* f_name = new char[size + 1];

	strncpy(f_name, data, size);
	f_name[size] = '\0';

//	std::ifstream is(f_name, std::ios::in);
//
//	if (is.fail()) printf("FAILED TO OPEN FILE\n");
//	else printf("SUCCSSFULLY OPENED FILE\n");
	_pSkeleton = new Skeleton(f_name, 1.0);

	return true;
}

Horde3D::Resource* Horde3DAsfAmc::ASFResource::clone() {
	ASFResource *res = new ASFResource("", _flags);

	*res = *this;

	return res;
}

bool Horde3DAsfAmc::ASFResource::prepairBonesJoints(ModelNode* modelNode) {
	//going through jointList and calculating  direction vectors
	for (std::map<NodeHandle, JointNode*>::iterator it = joint_idxMap.begin(); it != joint_idxMap.end(); ++it) {
		//it->second.Method();
		JointNode* joint = (JointNode* )it->second;
		const std::string jointName = joint->getName();
		map<string, string>::iterator parent_childIter = joint_childMap.find(jointName);
		if (parent_childIter == joint_childMap.end())
			continue;

		std::vector<SceneNode *> childrenJoints = joint->getChildren();
		for (int c = 0; c < childrenJoints.size(); ++c) {
			if (childrenJoints[c]->getName().compare(parent_childIter->second) == 0) {
				JointNode* child = (JointNode*)childrenJoints[c];
				Vec3f pt, pr, ps, ct, cr, cs;
				joint->getTransform(pt, pr, ps);
				child->getTransform(ct, cr, cs);
				Vec3f jointUnitVec = (ct - pt).normalized();
				joint_idx_unitVecMap.insert(pair<NodeHandle, Vec3f>(joint->getHandle(), jointUnitVec));
			}
		}
	}



	return true;
}

void Horde3DAsfAmc::ASFResource::traverseSkeleton(Bone* bone, Matrix4f cumulativeTransfom, Vec3f* absoluteUnitVectors) {
	if (bone) {
		transformBone(bone, cumulativeTransfom, absoluteUnitVectors);
		traverseSkeleton(bone->child, cumulativeTransfom, absoluteUnitVectors);
		traverseSkeleton(bone->sibling, cumulativeTransfom, absoluteUnitVectors);
	}
}

void Horde3DAsfAmc::ASFResource::getAbsoluteUnitVectors() {
	Vec3f* absoluteUnitVectors = new Vec3f[_pSkeleton->numBonesInSkel(_pSkeleton->getRoot()[0])];
	Matrix4f cumulativeTransform = Matrix4f(); //IDENTITY
	traverseSkeleton(_pSkeleton->getRoot(), cumulativeTransform, absoluteUnitVectors);
}

void Horde3DAsfAmc::ASFResource::transformBone(Bone* bone, Matrix4f cumulativeTransform, Vec3f* absoluteUnitVectors) {
	float transformASF2H3D[16];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; ++j) {
			transformASF2H3D[4 * i + j] = bone->rot_parent_current[j][i];
		}
	}
	Matrix4f boneMat(transformASF2H3D); // = new Matrix4f()
	cumulativeTransform = cumulativeTransform * boneMat;
	absoluteUnitVectors[bone->idx] = cumulativeTransform * Vec3f(bone->dir[0], bone->dir[1], bone->dir[2]);
}

void Horde3DAsfAmc::ASFResource::buildJointIdxMap(SceneNode* model) {
	std::vector<SceneNode*> children = model->getChildren();
	for (int chId = 0; chId < children.size(); ++chId) {
		if (children[chId]->getType() == SceneNodeTypes::Joint) {
			joint_idxMap.insert(pair<NodeHandle, JointNode*>(children[chId]->getHandle(), (JointNode*)children[chId]));
			buildJointIdxMap(children[chId]);
		}
	}
}
