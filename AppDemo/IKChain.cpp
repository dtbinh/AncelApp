#include "IKChain.h"

using namespace AncelIK;

IKChain::bone_type* IKChain::getBone(std::size_t index) const
{
 	std::size_t cnt = 0;
 	std::list<bone_type*>::const_iterator it = mBoneChain.begin();
  	while(it != mBoneChain.end() && cnt < index)
	{
		it++; 
		cnt++;
	}

	if(it != mBoneChain.end()) 
		return *it;
 
	return NULL;
}

bool IKChain::initChain(const bone_type* root, const bone_type* leaf)
{
	mRoot        =	const_cast<bone_type*>(root);
	mEndEffector =  const_cast<bone_type*>(leaf);
 
    if(!mRoot)            throw std::invalid_argument("root was null");
    if(!mEndEffector)     throw std::invalid_argument("end_effector was null");
 	 
    bone_type * bone = mEndEffector;
	mBoneChain.push_front(mEndEffector);
    
	while(bone!=root && bone)
    {
		bone_type * parent = const_cast<bone_type*>( bone->getParent());
		mBoneChain.push_front(parent);
		bone = parent;
    }


    if(bone==0)  throw std::logic_error("root was not an ancestor of end-effector");

    // Setup default goal positions and weights
	mWeightPos   = 1;
	mWeightAxisX = 1;
	mWeightAxisY = 1;

	mLocalPos   = Ogre::Vector3(0,0,0);
	mLocalAxisX = Ogre::Vector3(1,0,0);
	mLocalAxisY = Ogre::Vector3(0,1,0);

	mGoalPos = mEndEffector->getGlobalPos();

	Ogre::Quaternion Q = mEndEffector->getGlobalOri();
	Ogre::Matrix3 M;
	
	Q.ToRotationMatrix(M);

	mGoalAxisX = M.GetColumn(0);
	mGoalAxisY = M.GetColumn(1);

	mIsOnlyPosition = true;
	mChainName = mEndEffector->name();

	return true;
}