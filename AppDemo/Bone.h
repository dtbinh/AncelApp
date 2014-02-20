#ifndef __Bone_h
#define __Bone_h

#include <vector>
#include <OgreQuaternion.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include "MousePicker.h"
//#include "BoneAccessor.h"

namespace AncelIK
{
	class BoneAccessor;
};

namespace AncelApp
{
	class Skeleton;
 
	class Bone//: public AncelIK::BoneTraits
	{
	public:
 		typedef struct 
		{
			Ogre::Quaternion Q;
			Ogre::Vector3	 T;
		}	Transform_Type;

		enum JointType{JT_NR = 0, JT_Rx = 1, JT_Ry = 2, JT_Rz = 4, JT_Rxy = 3, JT_Rxz = 5, JT_Ryz = 6, JT_Rxyz = 7};
 	public:
	
		Bone(const Bone* parent = NULL);
		~Bone();
		Bone* cloneFrom(const Bone& bone);

		bool isRoot() const		{return (!mParent);}
		bool isLeaf() const		{return mChildren.empty();}

		std::string& name()		{return mBoneName;}

		float&             boneLength()          {return mBoneLength;}
		const Bone*        getParent() const     {return mParent;}
		const std::size_t& id()	   const	 {return mBoneID;}
		std::size_t&	   id()	           	     {return mBoneID;}

		const JointType&   type()	const		 {return mJointType;}
			  JointType&   type()				 {return mJointType;}

	 	Transform_Type&	   initPose()			 {return mInitPose;}
	
		Ogre::Quaternion&  meshOrientation()	 {return mMeshOri;}
		Ogre::String&	   meshName()			 {return mMeshName;};

		void			   addChild(const Bone* bone);
		void			   setBelongTo(Skeleton* belongTo) { mBelongTo = belongTo;}
 		Ogre::Vector2&	   limitsBox(std::size_t index)     {return mLimitsBox[index];}	

		void attatchBoneToScene(Ogre::SceneManager *sceneMgr, const std::size_t type);
		void removeFromScene(Ogre::SceneManager *sceneMgr);

		void update(std::vector<double>::const_iterator& it, bool updateSceneNode);

		void getTheta(std::vector<double>::iterator& it) const;
		void setTheta(std::vector<double>::const_iterator& it, bool updateSceneNode);
		void computePosition();

		Ogre::Vector3&          getGlobalPos()         {return mAbsolute.T;}
		const Ogre::Vector3&    getGlobalPos() const   {return mAbsolute.T;}

		Ogre::Quaternion&		getGlobalOri()		   {return mAbsolute.Q;}
		const Ogre::Quaternion& getGlobalOri()  const  {return mAbsolute.Q;}

		Ogre::Vector3&			getRelativePos()       {return mRelative.T;}
		const Ogre::Vector3&    getRelativePos() const {return mRelative.T;}
		
		std::size_t		      getActiveDofs() const;
		std::vector<float>&	  getTheta() {return mTheta;}
		void  setVisibility(bool flag); 
	    void  computeJointLimitsProjection(std::vector<double>::iterator &it);
		void  showBoundingBox(bool visible);
		void  updateMaterial(std::string materialName);
		const std::string& getMaterialName() const {return mMaterialName;}
	protected:
  	 	Skeleton*				 mBelongTo;			//the skeleton pointer which the bone belong to
		const Bone*				 mParent;			//the bone's parent bone pointer
		std::vector<Bone*>       mChildren;
				
		std::vector<float>	     mTheta;			// three for rotation and three for transition 
 		float					 mBoneLength;		// the length of bone

	 	std::size_t				 mBoneID;
		Ogre::String			 mBoneName;
		JointType				 mJointType;
		Ogre::Quaternion	  	 mLocalAxis;		// the local axis of the bone
		Ogre::Vector3			 mRelativePos;		//postion relative to its parent;
		Ogre::Vector2			 mLimitsBox[3];

		Transform_Type			 mInitPose;
		Transform_Type			 mAbsolute;
		Transform_Type			 mRelative;

		Ogre::Entity*		     mEntity;
		Ogre::Quaternion	 	 mMeshOri;
		
		Ogre::String			 mMeshName;
 		Ogre::String			 mMaterialName;

		Ogre::SceneNode*		 mObjectNode;
		Ogre::SceneNode*		 mHierarchicalNode;
		
		friend class AncelIK::BoneAccessor;
 	 };
}

#endif