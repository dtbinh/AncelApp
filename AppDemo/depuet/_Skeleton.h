/**
*-----------------------------------------------------------------------------
*Filename:  Skeleton.h
*-----------------------------------------------------------------------------
*File Description: the file is used to parse motion capture data files with .amc format
*-----------------------------------------------------------------------------
*Author: Ancel         2011/11/21               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __SKELETON_H
#define __SKELETON_H

#include <vector>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreSingleton.h>
#include <OgreSceneManager.h>
#include <OgreAnimation.h>
#include "_Animation.h"
#include "UIComponent.h"
#include <OgreVector3.h>

namespace AncelApp
{
	//-------------------------------------------------------------------------------
	class BoneNode
	{
	public:

		int           mBoneID;
		float         mBoneLength;
	 	
 		Ogre::Vector3 mPostion;
		Ogre::Quaternion mMeshOri;
		Ogre::Quaternion mLocalAxis;

		Ogre::String  mBoneName;
		Ogre::String  mMeshName;

		Ogre::Entity    *mEntity;
		Ogre::SceneNode *mHierarchicalNode;
		Ogre::SceneNode *mObjectNode;
		
		int				 mParentID;
		std::vector<int> mChildID;

		BoneNode():mBoneName(""),mEntity(0),mHierarchicalNode(0),mObjectNode(0){}
 	};
	//---------------------------------------------------------------------------------
	class Skeleton
	{
	public:
		
		Skeleton(Ogre::String skelName);
		virtual ~Skeleton();
		bool loadSkeleton(Ogre::String fileName);
		
  		void update(double timeSinceLastFrame);

		void attachToSceneMgr(Ogre::SceneManager *sceneMgr);
		void removeFromSceneMgr();
		Ogre::String getSkeletonName() { return mSkeletonName;}
		
		void attachAnimState(AnimationState *animState);
		Ogre::SceneNode* getRootNode();

		void setTheme(const Ogre::String &theme);
		Ogre::String getTheme() const;

		Ogre::Vector3 getBonePosition(Ogre::String BoneName);
		double& getTimeInterval();
		bool isAttachToScene();

		void update(const Animation *anim,std::size_t frameNum);
	protected:

		void createHierarchicalSkeleton(int boneID);
		void removeFromSceneMgr(int boneID);
		void update(int boneID);
		void update(int boneID,const Animation *anim, std::size_t frameNum);
	protected:
 
		int                     mRootID;
		double					mTimeCount;
		double					mTimeInterval;
		Ogre::String            mSkeletonName;
		
		Ogre::String			mTheme;

		AnimationState*         mAnimState;
		std::vector<BoneNode*>  mBoneSet;
		Ogre::SceneManager*     mBelongTo;
		Ogre::SceneNode*		mRootNode;
	};

	//--------------------------------------------------------------------------------

	class SkeletonManager: public Ogre::Singleton<SkeletonManager>, public UIComponent
	{
	public:
		SkeletonManager();
		~SkeletonManager();
		Skeleton* findByName(std::string skel);
		
		bool attachSkeletonToSceneMgr(std::string  skel,Ogre::SceneManager* sceneMgr);
		bool removeSkeletonFromSceneMgr(std::string  skel);
 		
		bool createSkeleton(std::string  skelName,std::string fileName);
		bool removeSkeleton(std::string  skelName);
		bool removeAll();
		void update(const Ogre::FrameEvent& evt);

		bool setTrackingTarget(std::string skel);
		
		void showTrajectory(const Animation *anim);
		void destoryTrajectory(const std::string name);

		Ogre::SceneNode*   getTrackingTarget();
		std::string		   getActiveTemplate();
 		Skeleton*          getActivatedSkeleton();
		Ogre::StringVector getSkeletonName();

		void buttonHit(OgreBites::Button* button);
		void sliderMoved(OgreBites::Slider * slider);
		void itemSelected(OgreBites::SelectMenu *menu);
	private:
		SkeletonManager(const SkeletonManager&);
		SkeletonManager& operator= (const SkeletonManager&); 
 		
		std::vector<std::vector<Skeleton*>> mSkeletonSets;
		
		std::string mTrackingTarget;
		std::map<std::string,Skeleton*>  mSkeletons;
	};
 	//-------------------------------------------------------------------------------
};
#endif