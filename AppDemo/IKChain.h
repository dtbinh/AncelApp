#ifndef __IKChian_h_
#define __IKChian_h_
#include <list>
#include <OgreVector3.h>
#include "Bone.h"

namespace AncelIK
{
 	class IKChain
	{
	public:
		typedef double					  real_type;
		typedef AncelApp::Bone			  bone_type;
 		typedef std::list<bone_type*>     bone_ptr_container;
		
		std::size_t    size()			  {return mBoneChain.size();}
		std::size_t    getGoalDimension() {return mIsOnlyPosition ? 3 : 9; }

		Ogre::Vector3& goalPosition()	  {return mGoalPos;}
		Ogre::Vector3& goalAxisX()		  {return mGoalAxisX;}
		Ogre::Vector3& goalAxisY()	      {return mGoalAxisY;}
	
		const Ogre::Vector3& localPosition() const 	  {return mLocalPos;}
			  Ogre::Vector3& localPosition()		  {return mLocalPos;}
		
 		const Ogre::Vector3& localAxisX() const	  {return mLocalAxisX;}
			  Ogre::Vector3& localAxisX()		  {return mLocalAxisX;}

		const Ogre::Vector3& localAxisY() const   {return mLocalAxisY;}
			  Ogre::Vector3& localAxisY()	      {return mLocalAxisY;}
	
        const real_type&	weightPos()	  const   {return mWeightPos;}
			  real_type&	weightPos()		      {return mWeightPos;}

		const real_type&	weightAxisX() const   {return mWeightAxisX;}
			  real_type&	weightAxisX()	      {return mWeightAxisX;}

		const real_type&	weightAxisY() const   {return mWeightAxisY;}
			  real_type&	weightAxisY()	      {return mWeightAxisY;}

		bone_type*       getRoot()		        {return mRoot;}
		bone_type* const getRoot()  const 		{return mRoot;}

		bone_type*		 getEndEffector()	        {return mEndEffector;}
 		bone_type* const getEndEffector()	const   {return mEndEffector;}

		bool onlyPosition() const				{return mIsOnlyPosition;}
		
		bone_type*   getBone(std::size_t index) const;

		const std::string& name() const {return mChainName;}
		std::string& name() {return mChainName;}

  		bool initChain(const bone_type* root, const bone_type* leaf);

	private:
		bool mIsOnlyPosition;

		std::string   mChainName;

		Ogre::Vector3 mLocalPos;
		Ogre::Vector3 mLocalAxisX;
		Ogre::Vector3 mLocalAxisY;
 
		Ogre::Vector3 mGoalPos;
		Ogre::Vector3 mGoalAxisX;
		Ogre::Vector3 mGoalAxisY;
		
		double mWeightPos;
		double mWeightAxisX;
		double mWeightAxisY;

		bone_type*			mRoot;
		bone_type*		    mEndEffector;
		bone_ptr_container  mBoneChain;
	};
};
#endif