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
#include "asfamc/motion.h"
#include "utMath.h"

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

	void buildJointIdxMap(int frame, std::map<std::string, Horde3D::Vec3f>* pJointMap);
	void buildMaps(SceneNode* model);

	void setModel(SceneNode* node);

	int getNumFrames(){return _numFrames;};
	int getNumEntities(){return _numEntities;};
	void rebasePosture(int frameIndex, Matrix4f** frameTs, Quaternion** frameQs, bool basePosture, bool skeletonBasePosture);
	void compare_bones(SceneNode* joint, Vec3f* absoluteUnitVectors, Matrix4f absTrans, Matrix4f* frameT, Quaternion* frameQ);
	void getEntityName(int enitityIndex, char* name);

	void draw_bones(SceneNode* joint, Vec3f* absoluteUnitVectors, Matrix4f absTrans, Matrix4f* frameT, Quaternion* frameQ);



private:
	Skeleton* _pSkeleton;
	Motion* _pMotion;
	SceneNode* _pNode;

	uint32 _numEntities;
	uint32 _numFrames;

	map<NodeHandle, int> joint_idxMap;
	map<int, JointNode*> idx_jointMap;
	map<NodeHandle, int> jointH_BoneidxMap;
	map<NodeHandle, JointNode*> pJointH_cJointMap; // Vector
	map<string, char*> joint_boneMap;
	map<string, string> joint_childMap;
	map<string,int> joint_name_idxMap;
	map<NodeHandle, Vec3f> joint_idx_unitVecMap;
	std::map<NodeHandle, Matrix4f> frameTransformMap;
	std::vector< map<NodeHandle, Matrix4f> > frames;

	void initjointH_BoneidxMap(SceneNode* model);

	void traverseSkeleton(Bone* bone, Matrix4f cumulativeTransform, Vec3f* absoluteUnitVectors);
	Vec3f* getAbsoluteUnitVectors();
	void getAbsoluteVectors(Bone* bone, Bone* parent, Vec3f* absoluteUnitVectors, Vec3f* absoluteUnitVectors);
	void transformBone(Bone* bone, Matrix4f &cumulativeTransform, Vec3f* absoluteUnitVectors);

	void rebaseTree(SceneNode* joint, Vec3f* absoluteUnitVectors, Matrix4f absTrans, Matrix4f* frameT, Quaternion* frameQ, bool basePosture);
};

} /* namespace Horde3DAsfAmc */

#endif /* ASFRESOURCE_H_ */
