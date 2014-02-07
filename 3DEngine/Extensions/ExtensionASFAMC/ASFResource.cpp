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
		JointNode* joint = (JointNode*) it->second;
		const std::string jointName = joint->getName();
		map<string, string>::iterator parent_childIter = joint_childMap.find(jointName);
		if (parent_childIter == joint_childMap.end())
			continue;

		std::vector<SceneNode *> childrenJoints = joint->getChildren();
		for (int c = 0; c < childrenJoints.size(); ++c) {
			if (childrenJoints[c]->getName().compare(parent_childIter->second) == 0) {
				JointNode* child = (JointNode*) childrenJoints[c];
				Vec3f ct, cr, cs;
				//RELATIVE TRANSFORMS
				//RELATIVE VECTOR = Child Translation
				child->getTransform(ct, cr, cs);
				Vec3f jointUnitVec = ct.normalized();
				joint_idx_unitVecMap.insert(pair<NodeHandle, Vec3f>(joint->getHandle(), jointUnitVec));
			}
		}
	}

	return true;
}

void Horde3DAsfAmc::ASFResource::traverseSkeleton(Bone* bone, Matrix4f cumulativeTransfom, Vec3f* absoluteUnitVectors) {
	if (bone) {
		//CALL sibling first so to copy matrix onto stack
		traverseSkeleton(bone->sibling, cumulativeTransfom, absoluteUnitVectors);
		//This one changes the matrix
		transformBone(bone, cumulativeTransfom, absoluteUnitVectors);
		traverseSkeleton(bone->child, cumulativeTransfom, absoluteUnitVectors);
	}
}

Vec3f* Horde3DAsfAmc::ASFResource::getAbsoluteUnitVectors() {
	Vec3f* absoluteUnitVectors = new Vec3f[_pSkeleton->numBonesInSkel(_pSkeleton->getRoot()[0])];
	Matrix4f cumulativeTransform = Matrix4f(); //IDENTITY
	traverseSkeleton(_pSkeleton->getRoot(), cumulativeTransform, absoluteUnitVectors);
}

void Horde3DAsfAmc::ASFResource::transformBone(Bone* bone, Matrix4f &cumulativeTransform, Vec3f* absoluteUnitVectors) {
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
			joint_idxMap.insert(pair<NodeHandle, JointNode*>(children[chId]->getHandle(), (JointNode*) children[chId]));
			buildJointIdxMap(children[chId]);
		}
	}
}

void Horde3DAsfAmc::ASFResource::rebasePosture(ModelNode* modelNode, Posture posture) {

	SceneNode* root_Joint = modelNode->getChildren().front(); //Bip01
	Matrix4f absTrans = Matrix4f() * root_Joint->getRelTrans();
	rebaseTree(root_Joint, getAbsoluteUnitVectors(), absTrans);

}

void Horde3DAsfAmc::ASFResource::rebaseTree(SceneNode* joint, Vec3f* absoluteUnitVectors, Matrix4f absTrans) {

	std::vector<SceneNode *> children = joint->getChildren();
	for (int child_idx = 0; child_idx < children.size(); child_idx++) {
		Matrix4f chAbsTrans = absTrans * children[child_idx]->getRelTrans();
		//TODO: male a map of Joint NodeHandle -> bone_idx
		map<NodeHandle, int>::iterator boned_joint_HandleItter = jointH_BoneidxMap.find(joint->getHandle());
		if (boned_joint_HandleItter != jointH_BoneidxMap.end()) {
			//rebase
			//get the child that represents the vector
			//get the child's relative unit vector

			JointNode* childJoint = pJointH_cJointMap.find(boned_joint_HandleItter->first)->second;
			Vec3f ct, cr, cs, pt, pr, ps;
			childJoint->getRelTrans().decompose(ct, cr, cs);
			Vec3f relative_unit_vec = ct.normalized();

			//get matching Skeleton Bone vector
			//Transform skeleton's unit vector to joint-local space by multiplying it
			//with the inverse of joint's absolute transform
			//calculate transform that will make both vectors parallel
			//Quaternion of the rotation is calculated as:
			//(v1 x v1) for (x,y,z) part and v1.v2 for w
			Vec3f skeleton_vec = absoluteUnitVectors[boned_joint_HandleItter->second];
			Vec3f skeleton_vec_joint_space = chAbsTrans.inverted() * skeleton_vec;
			Vec3f jot = relative_unit_vec.cross(skeleton_vec_joint_space);
			Quaternion q = Quaternion(jot.x, jot.y, jot.z, relative_unit_vec.dot(skeleton_vec_joint_space));
			frameTransformMap.insert(pair<NodeHandle, Matrix4f>(joint->getHandle(), Matrix4f(q)));
		} else {
			frameTransformMap.insert(pair<NodeHandle, Matrix4f>(joint->getHandle(), Matrix4f()));
		}

		rebaseTree(children[child_idx], absoluteUnitVectors, chAbsTrans);
	}
}
