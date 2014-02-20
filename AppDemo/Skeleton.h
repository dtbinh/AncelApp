#ifndef _Skeleton_h
#define _Skeleton_h

#include "Bone.h"
#include "MousePicker.h"
#include <OgreSceneNode.h>
namespace AncelApp
{
 	class Skeleton: public PickableObject
	{
	public:
		typedef AncelApp::Bone	bone_type;

		Skeleton(const std::string& skeletonName, const std::size_t type = 0);
		~Skeleton();
		bool   cloneFrom(const Skeleton& skel);
		bool   attachSkeletonToScene(Ogre::SceneManager *sceneMgr);
		void   removeSkeletonFromScene();
		
		void   setThetaParameters(const std::vector<double> &theta, bool updateSceneNode);
		void   getThetaParameters(std::vector<double> &theta) const;

		void   update(const std::vector<double> &theta, bool updateSceneNode = true);
		void   update2(const std::vector<double> &theta, bool updateSceneNode = true);

		void   getSkeletonParameters(std::vector<double> &theta) const;
		void   computeSkeletonLimitsProjection(std::vector<double> &theta);

		       bone_type*	 createBone(Bone *parent = NULL);
	    const  bone_type*	 getRoot() const                             {return mRoot;}
		       bone_type*	 getRoot()                                   {return mRoot;}
		const  bone_type*	 getBone(const std::size_t& boneID) const    {return mBones[boneID];}
		const  bone_type*	 getBone(const std::string& name)   const    {return mBones[mBoneNameToID.at(name)];}
			   bone_type*	 getBone(const std::size_t& boneID)			 {return mBones[boneID];}
			   bone_type*	 getBone(const std::string& name)			 {return mBones[mBoneNameToID[name]];}

		std::string	 getName() const;   
		Ogre::SceneNode* getRootNode() const {return mActorNode;}
		void setSkeletonType(std::size_t type) {mSkeletonType = type;}
		

		bool				 loadSkeletonFromXML(const std::string &skeletonFileName);

 		std::size_t			 getBoneNumber()   const  {return mBones.size();}
		std::size_t          getTotalFreedom() const;
		std::size_t          getSkeletonType() const {return mSkeletonType;}

		std::vector<bone_type*>::const_iterator begin() {return mBones.begin();}
		std::vector<bone_type*>::const_iterator end()   {return mBones.end();}

		void setVisibility(bool flag);

		virtual bool  notifyReleased();
		virtual bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		virtual bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset);
 		virtual bool  mousePressed(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		
		void setPickEventHandle(PickableObject *handle);
		PickableObject* getPickEventHandle() {return mPickEventHandle;}
		void  updateMaterial(std::string materialName);
	protected:
		Ogre::Real				   mCollsionDepth;
		Ogre::Vector3			   mPickedNodeOffset;
		Ogre::Vector3			   mPressedPosition;

		Ogre::SceneNode*           mActorNode;
		bone_type*		           mRoot;
		std::vector<bone_type*>	   mBones;
		std::map<std::string, int> mBoneNameToID;
		
		PickableObject*			   mPickEventHandle;
		Ogre::SceneManager*        mBelongTo;		  //The scene manager pointer which the skeleton is attached to
		std::string			       mSkeletonName;
		bool				       mIsBoneCreated;  
		std::size_t				   mSkeletonType;
	};
}

#endif