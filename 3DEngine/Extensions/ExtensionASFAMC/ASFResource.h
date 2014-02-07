/*
 * ASFResource.h
 *
 *  Created on: Jan 4, 2014
 *      Author: yama
 */

#ifndef ASFRESOURCE_H_
#define ASFRESOURCE_H_

#include <egResource.h>
#include <egPrerequisites.h>
#include <egModel.h>
#include "asfamc/skeleton.h"

namespace Horde3DAsfAmc {

using namespace Horde3D;
using namespace std;


const int ASF_resType = 151;


class ASFResource: public Horde3D::Resource {
public:
	ASFResource(const std::string &name, int flags);
	virtual ~ASFResource();

	static Resource *factoryFunc(const std::string &name, int flags) {
		return new ASFResource(name, flags);
	}

	Resource *clone();

	void initDefault();
	void release();
	bool load(const char *data, int size);

	bool prepairBonesJoints(ModelNode* modelNode);



private:
	Skeleton* _pSkeleton;
	map<NodeHandle, JointNode*> joint_idxMap;
	map<NodeHandle, int> jointH_BoneidxMap;
	map<NodeHandle, JointNode*> pJointH_cJointMap; // Vector
	map<string, string> joint_boneMap;
	map<string, string> joint_childMap;
	map<string,int> joint_name_idxMap;
	map<NodeHandle, Vec3f> joint_idx_unitVecMap;
	std::map<NodeHandle, Matrix4f> frameTransformMap;
	std::vector<Matrix4f> frameTransforms;

	void buildJointIdxMap(SceneNode* model);
	void traverseSkeleton(Bone* bone, Matrix4f cumulativeTransform, Vec3f* absoluteUnitVectors);
	Vec3f* getAbsoluteUnitVectors();
	void transformBone(Bone* bone, Matrix4f &cumulativeTransform, Vec3f* absoluteUnitVectors);
	void rebasePosture(ModelNode* modelNode, Posture posture);
	void rebaseTree(SceneNode* joint, Vec3f* absoluteUnitVectors, Matrix4f absTrans);
};

} /* namespace Horde3DAsfAmc */

#endif /* ASFRESOURCE_H_ */
