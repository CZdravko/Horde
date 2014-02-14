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

#include <string>
#include <sstream>
#include <istream>

using namespace Horde3D;

namespace Horde3DAsfAmc {

ASFResource::~ASFResource() {
	// TODO Auto-generated destructor stub
}

} /* namespace Horde3DAsfAmc */

Horde3DAsfAmc::ASFResource::ASFResource(const std::string& name, int flags) :
		Resource(ASF_resType, name, flags), _pSkeleton(0x0), _pMotion(0x0), _numEntities(0), _numFrames(0) {
}

//Resource* Horde3DAsfAmc::ASFResource::clone() {
//	ASFResource *res = new ASFResource("", _flags);
//
//	*res = *this;
//
//	return res;
//}

void Horde3DAsfAmc::ASFResource::initDefault() {
	//TODO: Come up with solution for non-bone joints
//	joint_boneMap.insert(pair<string, char*>("Bip01", "ROOT"));
//	joint_childMap.insert(pair<string, string>("Bip01_Pelvis", "Bip01_Spine"));

	printf("initDefault start\n");

	joint_boneMap.insert(pair<string, char*>("Bip01_Pelvis", "lowerback"));
	joint_childMap.insert(pair<string, string>("Bip01_Pelvis", "Bip01_Spine"));

	joint_boneMap.insert(pair<string, char*>("Bip01_Spine", "upperback"));
	joint_childMap.insert(pair<string, string>("Bip01_Spine", "Bip01_Spine1"));

	joint_boneMap.insert(pair<string, char*>("Bip01_Spine1", "thorax"));
	joint_childMap.insert(pair<string, string>("Bip01_Spine1", "Bip01_Spine2"));

//	joint_boneMap.insert(pair<string, char*>("Bip01_Spine2", "lowerneck"));
//	joint_childMap.insert(pair<string, string>("Bip01_Spine2", "Bip01_Neck"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_Neck", "upperneck"));
//	joint_childMap.insert(pair<string, string>("Bip01_Neck", "Bip01_Head"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_L_UpperArm", "lhumerus"));
//	joint_childMap.insert(pair<string, string>("Bip01_L_UpperArm", "Bip01_L_Forearm"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_L_Forearm", "lradius"));
//	joint_childMap.insert(pair<string, string>("Bip01_L_Forearm", "Bip01_L_Hand"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_L_Hand", "lwrist"));
//	joint_childMap.insert(pair<string, string>("Bip01_L_Hand", "Bip01_L_Finger0"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_R_UpperArm", "rhumerus"));
//	joint_childMap.insert(pair<string, string>("Bip01_R_UpperArm", "Bip01_R_Forearm"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_L_Forearm", "rradius"));
//	joint_childMap.insert(pair<string, string>("Bip01_R_Forearm", "Bip01_R_Hand"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_R_Hand", "rwrist"));
//	joint_childMap.insert(pair<string, string>("Bip01_R_Hand", "Bip01_R_Finger0"));
//
//	//LEFT THIGH
//	joint_boneMap.insert(pair<string, char*>("Bip01_L_Thigh", "lfemur"));
//	joint_childMap.insert(pair<string, string>("Bip01_L_Thigh", "Bip01_L_Calf"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_L_Calf", "ltibia"));
//	joint_childMap.insert(pair<string, string>("Bip01_L_Calf", "Bip01_L_Foot"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_L_Foot", "lfoot"));
//	joint_childMap.insert(pair<string, string>("Bip01_L_Foot", "Bip01_L_Toe0"));
//
//	//RIGHT THIGH
//	joint_boneMap.insert(pair<string, char*>("Bip01_R_Thigh", "rfemur"));
//	joint_childMap.insert(pair<string, string>("Bip01_R_Thigh", "Bip01_R_Calf"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_R_Calf", "rtibia"));
//	joint_childMap.insert(pair<string, string>("Bip01_R_Calf", "Bip01_R_Foot"));
//
//	joint_boneMap.insert(pair<string, char*>("Bip01_R_Foot", "rfoot"));
//	joint_childMap.insert(pair<string, string>("Bip01_R_Foot", "Bip01_R_Toe0"));

	printf("initDefault end\n");
}

void Horde3DAsfAmc::ASFResource::release() {
}

bool Horde3DAsfAmc::ASFResource::load(const char* data, int size) {
	if (!Resource::load(data, size))
		return false;

	string twoliner(data);
	istringstream ssin(twoliner);

	string first_line, second_line;
	std::getline(ssin, first_line);
	std::getline(ssin, second_line);

	printf("first_line.size(): %d \n", first_line.size());
	printf("second_line.size(): %d \n", second_line.size());

	char* sf_name = new char[first_line.size()];
	char* mf_name = new char[second_line.size()];

	strncpy(sf_name, first_line.c_str(), first_line.size() - 1);
	sf_name[first_line.size()] = '\0';

	strncpy(mf_name, second_line.c_str(), second_line.size() - 1);
	mf_name[second_line.size()] = '\0';

	printf(" %s \n", sf_name);
	printf(" %s \n", mf_name);

	char* s = "D:\\repo\\git\\Horde\\Samples\\Content\\animations\\test.asf";
	char* m = "D:\\repo\\git\\Horde\\Samples\\Content\\animations\\walk.amc";

	std::ifstream is(s, std::ios::in);

	if (is.fail())
		printf("FAILED TO OPEN FILE\n");
	else
		printf("SUCCSSFULLY OPENED FILE\n");

	_pSkeleton = new Skeleton(s, 1.0);

	_pMotion = new Motion(m, 1.0, _pSkeleton);

	_numFrames = _pMotion->GetNumFrames();

//
//	getAbsoluteUnitVectors();

	initDefault();

	return true;
}

Horde3D::Resource* Horde3DAsfAmc::ASFResource::clone() {
	ASFResource *res = new ASFResource("", _flags);

	*res = *this;

	return res;
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
	return absoluteUnitVectors;
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
	Matrix4f posture = Matrix4f::RotMat(bone->rx, bone->ry, bone->rz);
	cumulativeTransform = cumulativeTransform * posture;
	absoluteUnitVectors[bone->idx] = cumulativeTransform * Vec3f(bone->dir[0], bone->dir[1], bone->dir[2]);
}

//
void Horde3DAsfAmc::ASFResource::buildJointIdxMap(SceneNode* model) {
	if (!model)
		model = _pNode;

	std::vector<SceneNode*> children = model->getChildren();
	for (unsigned int chId = 0; chId < children.size(); ++chId) {
		if (children[chId]->getType() == SceneNodeTypes::Joint) {
			joint_idxMap.insert(pair<NodeHandle, int>(children[chId]->getHandle(), _numEntities));
			idx_jointMap.insert(pair<int, JointNode*>(_numEntities, (JointNode*) children[chId]));
			_numEntities++;
			buildJointIdxMap(children[chId]);
		}
	}
}

void Horde3DAsfAmc::ASFResource::rebasePosture(int frameIndex, Matrix4f** frameTs, Quaternion** frameQs, bool basePosture, bool skeletonBasePosture) {


	if (skeletonBasePosture){
		_pSkeleton->setBasePosture();
	}else{
		Posture posture = *_pMotion->GetPosture(frameIndex);
		_pSkeleton->setPosture(posture);
	}

	SceneNode* root_Joint = _pNode->getChildren().front(); //Bip01
	Matrix4f absTrans = Matrix4f() * root_Joint->getRelTrans();
	Matrix4f* frame = new Matrix4f[_numEntities];
	Quaternion* quat = new Quaternion[_numEntities];
	rebaseTree(root_Joint, getAbsoluteUnitVectors(), absTrans, frame, quat, basePosture);
	frameTs[frameIndex] = frame;
	frameQs[frameIndex] = quat;

}

void Horde3DAsfAmc::ASFResource::initjointH_BoneidxMap(SceneNode* node) {
	if (!node)
		node = _pNode;
//TODO: Check if node repeated
//	if (node->getType() == SceneNodeTypes::Joint) {
//		map<string, char*>::iterator joint_boneIter = joint_boneMap.find(node->getName());
//		if (joint_boneIter != joint_boneMap.end()) {
//			int boneIndex = _pSkeleton->name2idx(joint_boneIter->second);
//			jointH_BoneidxMap.insert(std::pair<NodeHandle, int>(node->getHandle(), boneIndex));
//			string bone_child = joint_childMap.find(node->getName())->second;
//			std::vector<SceneNode*> children = node->getChildren();
//			for (unsigned int childId = 0; childId < children.size(); childId++) {
//				if (!bone_child.compare(children[childId]->getName())) {
//					pJointH_cJointMap.insert(std::pair<NodeHandle, JointNode*>(node->getHandle(), (JointNode*) children[childId]));
//				}
//				initjointH_BoneidxMap(children[childId]);
//			}
//		}
//	}

	std::vector<SceneNode*> children = node->getChildren();
	if (node->getType() == SceneNodeTypes::Joint) {
		map<string, char*>::iterator joint_boneIter = joint_boneMap.find(node->getName());
		if (joint_boneIter != joint_boneMap.end()) {
			int boneIndex = _pSkeleton->name2idx(joint_boneIter->second);
			if (jointH_BoneidxMap.find(node->getHandle()) == jointH_BoneidxMap.end() && boneIndex != -1) {
				printf("FOUND: %s\n", node->getName().c_str());
				bool gotit = false;
				string bone_child = joint_childMap.find(node->getName())->second;
				for (unsigned int childId = 0; childId < children.size(); childId++) {
					if (!bone_child.compare(children[childId]->getName())) {
						gotit = true;
						jointH_BoneidxMap.insert(std::pair<NodeHandle, int>(node->getHandle(), boneIndex));
						pJointH_cJointMap.insert(std::pair<NodeHandle, JointNode*>(node->getHandle(), (JointNode*) children[childId]));
					}
				}
				if (!gotit) {
					printf("DIDN'TFIND: %s\n", node->getName().c_str());
				}
			} else {
				printf("boneIndex: %d\t", boneIndex);
				printf("jointName: %s\n", node->getName().c_str());
			}
		}
	}
	for (int c = 0; c < children.size(); c++)
		initjointH_BoneidxMap(children[c]);
}

void Horde3DAsfAmc::ASFResource::rebaseTree(SceneNode* joint, Vec3f* absoluteUnitVectors, Matrix4f absTrans, Matrix4f* frameT, Quaternion* frameQ, bool basePosture) {

	Matrix4f jointFrameTransform;
	Quaternion jointFrameQuaternion;
	//TODO: make a map of Joint NodeHandle -> bone_idx
	map<NodeHandle, int>::iterator boned_joint_HandleItter = jointH_BoneidxMap.find(joint->getHandle());
	if (boned_joint_HandleItter != jointH_BoneidxMap.end() && !basePosture) {
		//rebase
		//get the child that represents the vector
		//get the child's relative unit vector

		JointNode* childJoint = pJointH_cJointMap.find(boned_joint_HandleItter->first)->second;
		Vec3f ct, cr, cs, pt, pr, ps;
		childJoint->getRelTrans().decompose(ct, cr, cs);
		joint->getRelTrans().decompose(pt, pr, ps);
		Vec3f relative_unit_vec = ct.normalized();

		//get matching Skeleton Bone vector
		//Transform skeleton's unit vector to joint-local space by multiplying it
		//with the inverse of joint's absolute transform
		//calculate transform that will make both vectors parallel
		//Quaternion of the rotation is calculated as:
		//(v1 x v1) for (x,y,z) part and v1.v2 for w
		Vec3f skeleton_vec = absoluteUnitVectors[boned_joint_HandleItter->second];
		Vec3f skeleton_vec_joint_space = absTrans.inverted() * skeleton_vec;
		Vec3f jot = relative_unit_vec.cross(skeleton_vec_joint_space);
		jointFrameQuaternion = Quaternion(jot.x, jot.y, jot.z, relative_unit_vec.dot(skeleton_vec_joint_space));
		jointFrameTransform = Matrix4f(jointFrameQuaternion);
		jointFrameTransform.translate(pt.x, pt.y, pt.z);
	} else {
		jointFrameQuaternion = Quaternion(0, 0, 0, 1);
		jointFrameTransform = Matrix4f(joint->getRelTrans());			//IDENTITY
	}

	int entityIndex = joint_idxMap.find(joint->getHandle())->second;
	frameT[entityIndex] = jointFrameTransform;
	frameQ[entityIndex] = jointFrameQuaternion;

	std::vector<SceneNode *> children = joint->getChildren();

	for (unsigned int child_idx = 0; child_idx < children.size(); child_idx++) {
		Matrix4f chAbsTrans = absTrans * jointFrameTransform * children[child_idx]->getRelTrans();
		rebaseTree(children[child_idx], absoluteUnitVectors, chAbsTrans, frameT, frameQ, basePosture);
	}
}

void Horde3DAsfAmc::ASFResource::setModel(SceneNode* node) {
	_pNode = node;

	buildJointIdxMap(node);
	initjointH_BoneidxMap(node);
}

void Horde3DAsfAmc::ASFResource::getEntityName(int enitityIndex, char* name) {
	map<int, JointNode*>::iterator indexedJointIter = idx_jointMap.find(enitityIndex);
	if (indexedJointIter != idx_jointMap.end()) {
		JointNode* j = indexedJointIter->second;
		strncpy(name, j->getName().c_str(), j->getName().size());
	}
}
