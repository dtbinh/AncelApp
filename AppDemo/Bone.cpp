#include <algorithm>
#include "Bone.h"
#include "Skeleton.h"
#include "IKChain.h"
#include "IKSolver.h"
#include "MousePicker.h"

using namespace AncelApp;

Bone::Bone(const Bone* parent)
	:mParent(parent),
	mEntity(NULL),
	mObjectNode(NULL),
	mHierarchicalNode(NULL),
 	mMaterialName("Skeleton/Bone/Default")
{
	mTheta.resize(6,0.0);
}

Bone::~Bone()
{

}

Bone* Bone::cloneFrom(const Bone& bone)
{
 	mTheta = bone.mTheta;
	mBoneLength = bone.mBoneLength;
 
	mInitPose = bone.mInitPose;
	mAbsolute = bone.mAbsolute;
	mRelative = bone.mRelative;

	mBoneID    = bone.mBoneID;
	mBoneName  = bone.mBoneName;
	mJointType = bone.mJointType;
	mLocalAxis = bone.mLocalAxis;
	mRelativePos = bone.mRelativePos;

	mLimitsBox[0] = bone.mLimitsBox[0];
	mLimitsBox[1] = bone.mLimitsBox[1];
	mLimitsBox[2] = bone.mLimitsBox[2];

	mMeshName = bone.mMeshName;
 	mMeshOri = bone.mMeshOri;
	mMaterialName = bone.mMaterialName;

	return this;
}
void Bone::addChild(const Bone* bone)
{
	if (!bone)
		throw std::invalid_argument("invalid bone pointer");
	mChildren.push_back(const_cast<Bone*>(bone));
}
void Bone::removeFromScene(Ogre::SceneManager *sceneMgr)
{
	if (mChildren.size())
	{
		std::vector<Bone*>::iterator it = mChildren.begin();
 		for (it; it != mChildren.end(); it++)
		{
			(*it)->removeFromScene(sceneMgr);
		}
	 
 	}
	if (mHierarchicalNode != NULL)
	{
		mObjectNode->detachObject(mEntity);
	
		sceneMgr->destroyEntity(mEntity->getName());
		sceneMgr->destroySceneNode(mObjectNode);
 		sceneMgr->destroySceneNode(mHierarchicalNode);
	
		mEntity = NULL;
		mHierarchicalNode = NULL;
	}
}
void Bone::attatchBoneToScene(Ogre::SceneManager *sceneMgr, const std::size_t type)
{
	if (isRoot())
	{
		mHierarchicalNode = sceneMgr->getRootSceneNode()->createChildSceneNode(mBelongTo->getName() + "/" + name());
 	}
	else
		mHierarchicalNode = getParent()->mHierarchicalNode->createChildSceneNode(mBelongTo->getName() + "/" + name());
 
	mHierarchicalNode->setPosition(mInitPose.T);
	mHierarchicalNode->setOrientation(mInitPose.Q);
   	
	mHierarchicalNode->setInitialState();
 	
	mObjectNode = mHierarchicalNode->createChildSceneNode(mBelongTo->getName() + "/" + name() + "/Obj");
	mObjectNode->setOrientation(mMeshOri);
	
	if(type)
		mMeshName = "Cylinder.mesh";
	
	mEntity = sceneMgr->createEntity(mBelongTo->getName() + "/" + name() + "/Entity", mMeshName); //mBoneSet[boneID]->mBoneName+".mesh");
	if(type)
		mEntity->setMaterialName("Skeleton/Bone/Emerald");
	else 
		mEntity->setMaterialName("Skeleton/Bone/Default");
	
	mEntity->setCastShadows(true);
	if(mBoneName == "head")
		mEntity->setQueryFlags(EQM_BONE_MASK);
	else
		mEntity->setQueryFlags(EQM_NO_MASK);

	Ogre::Any any = Ogre::Any(UserAnyPair(Ogre::Any(static_cast<PickableObject*>(mBelongTo)), Ogre::Any(mBelongTo)));
	mEntity->setUserAny(any);

	mObjectNode->attachObject(mEntity);
	if(type)
		 mObjectNode->scale(1.5, mBoneLength, 1.5);
	else 
		 mObjectNode->scale(mBoneLength, mBoneLength, mBoneLength);

	std::vector<Bone*>::iterator it = mChildren.begin();
 	for (it; it != mChildren.end(); it++)
	{
		(*it)->attatchBoneToScene(sceneMgr, type);
	}
}

void Bone::update(std::vector<double>::const_iterator& it,bool updateSceneNode)
{
	if (isRoot())
	{
		mTheta[0] = static_cast<float>(*it++);
		mTheta[1] = static_cast<float>(*it++);
		mTheta[2] = static_cast<float>(*it++);
		mTheta[3] = static_cast<float>(*it++);
		mTheta[4] = static_cast<float>(*it++);
		mTheta[5] = static_cast<float>(*it++);
	}
	else
	{
		if (mJointType & 1)	mTheta[3] = static_cast<float>(*it++);
		if (mJointType & 2)	mTheta[4] = static_cast<float>(*it++);
		if (mJointType & 4)	mTheta[5] = static_cast<float>(*it++);
	}

	if (updateSceneNode)
	{
		Ogre::Matrix3 mat;
		mat.FromEulerAnglesZYX(Ogre::Radian(Ogre::Degree(mTheta[5]).valueRadians()),
							   Ogre::Radian(Ogre::Degree(mTheta[4]).valueRadians()),
							   Ogre::Radian(Ogre::Degree(mTheta[3]).valueRadians()));
		Ogre::Quaternion Q;
		Q.FromRotationMatrix(mat);
		mHierarchicalNode->resetToInitialState();
		if (isRoot())
			mHierarchicalNode->setPosition(mTheta[0], mTheta[1], mTheta[2]);
		
		mHierarchicalNode->rotate(Q);
	}
}
void Bone::computePosition()
{
	Ogre::Matrix3 mat;
	mat.FromEulerAnglesZYX(Ogre::Radian(Ogre::Degree(mTheta[5]).valueRadians()),
						   Ogre::Radian(Ogre::Degree(mTheta[4]).valueRadians()),
						   Ogre::Radian(Ogre::Degree(mTheta[3]).valueRadians()));
	Ogre::Quaternion Q;
	Q.FromRotationMatrix(mat);

	mRelative.T = mInitPose.T;
	mRelative.Q = mInitPose.Q;
	
	mRelative.Q = mRelative.Q * Q;

	if (isRoot())
	{
		mRelative.T.x =  mTheta[0];
		mRelative.T.y =  mTheta[1];
		mRelative.T.z =  mTheta[2];
		mAbsolute.T	  =  mRelative.T;
 		mAbsolute.Q   =  mRelative.Q;
	}
	else
	{
		mAbsolute.T = mParent->mAbsolute.Q * mRelative.T + mParent->mAbsolute.T;
		mAbsolute.Q = mParent->mAbsolute.Q * mRelative.Q;
	}

 	for (std::vector<Bone*>::iterator it = mChildren.begin(); it != mChildren.end(); it++)
		(*it)->computePosition();
}

void Bone::getTheta(std::vector<double>::iterator& it) const
{
	if(mJointType & 1) 	*it++ = mTheta[3];
 	if(mJointType & 2) 	*it++ = mTheta[4]; 
 	if(mJointType & 4) 	*it++ = mTheta[5]; 
}

void Bone::setTheta(std::vector<double>::const_iterator& it, bool updateSceneNode)
{
	if (mJointType & 1)	mTheta[3] = static_cast<float>(*it++);
	if (mJointType & 2)	mTheta[4] = static_cast<float>(*it++);
	if (mJointType & 4)	mTheta[5] = static_cast<float>(*it++);

	if (updateSceneNode)
	{
		Ogre::Matrix3 mat;
		mat.FromEulerAnglesZYX(Ogre::Radian(Ogre::Degree(mTheta[5]).valueRadians()),
							   Ogre::Radian(Ogre::Degree(mTheta[4]).valueRadians()),
							   Ogre::Radian(Ogre::Degree(mTheta[3]).valueRadians()));
		Ogre::Quaternion Q;
		Q.FromRotationMatrix(mat);
		mHierarchicalNode->resetToInitialState();
		if (isRoot())
			mHierarchicalNode->setPosition(mTheta[0], mTheta[1], mTheta[2]);
 		mHierarchicalNode->rotate(Q);
	}
}
std::size_t	Bone::getActiveDofs() const
{
 	if(mJointType == JT_NR)   return 0;
	if(mJointType == JT_Rx  || mJointType == JT_Ry  || mJointType == JT_Rz)  return 1;
	if(mJointType == JT_Rxy || mJointType == JT_Rxz || mJointType == JT_Ryz) return 2;
	if(mJointType == JT_Rxyz) return 3;
	
	throw std::logic_error("undefined joint type!!");
}

void  Bone::computeJointLimitsProjection(std::vector<double>::iterator &it)
{
	if(isRoot())
	{
		*it = mTheta[3]; *it++;
		*it = mTheta[4]; *it++;
		*it = mTheta[5]; *it++;
	}
	else 
	{
		if(mJointType & 1) {
		  		double val = *it;
 		 		*it = std::max(mLimitsBox[0].x, std::min(float(*it), mLimitsBox[0].y)); 
 		 		it++;
		}
		if(mJointType & 2)
		{
	     		double val = *it;
		 		*it = std::max(mLimitsBox[1].x, std::min(float(*it), mLimitsBox[1].y));
	   			it++;
		} 
		if(mJointType & 4)
		{
			double val = *it;
			*it = std::max(mLimitsBox[2].x, std::min(float(*it), mLimitsBox[2].y));
			it++;
		}
	}
}
void  Bone::setVisibility(bool flag)
{
	mEntity->setVisible(flag);
}
void  Bone::showBoundingBox(bool visible)
{
	mHierarchicalNode->showBoundingBox(visible);
}
void  Bone::updateMaterial(std::string materialName)
{
	mMaterialName = materialName;
	mEntity->setMaterialName(mMaterialName);
}