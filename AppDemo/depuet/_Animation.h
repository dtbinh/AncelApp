/**
*-----------------------------------------------------------------------------
*Filename:  Animation.h
*-----------------------------------------------------------------------------
*File Description: the file is used to parse motion capture data files with .amc format
*-----------------------------------------------------------------------------
*Author: Ancel         2011/11/21               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __ANIMATION_H
#define __ANIMATION_H

#include <vector>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreAnimationState.h>
#include <OgreSceneNode.h>
#include <OgreSingleton.h>
#include <map>
#include <Matrix.h>
#include "AppFileExplorer.h"
#include "UIComponent.h"

namespace AncelApp
{
 	class AnimationTrack
	{
	public:
		AnimationTrack(bool Tr,bool Ro);
		AnimationTrack(AnimationTrack&);
		virtual ~AnimationTrack();

	public:
		void pushBackKeyframe(const Ogre::Vector3& euler,const Ogre::Vector3 &trans);
 		void pushBackKeyframe(const Ogre::Quaternion &rotate,Ogre::Vector3 trans = Ogre::Vector3::ZERO);

		const Ogre::Quaternion& getRotation(size_t frameNum) const;
		const Ogre::Vector3&	getTransition(size_t frameNum) const;
		const bool& trsition() const {return mTr;}
		const bool& rotation() const {return mRo;}
		size_t getTotalFrame() const {return mRotation.size();}
	private:
		bool mTr;		//transition
		bool mRo;		//rotation
  
		std::vector<Ogre::Vector3> mTransition;
		std::vector<Ogre::Quaternion> mRotation;
	};
	//------------------------------------------------------------------------
	class Animation
	{
	public:
		Animation(const ResUtil::Matrix *motion, const Ogre::String &name);
		virtual ~Animation();
 		AnimationTrack* createAnimationTrack(const std::string & boneName,bool ro = true,bool tr = false);
	 	AnimationTrack* getAnimationTrack(const std::string &boneName) const;	
		size_t getTotalFrame() const;
		std::string getAnimName() const;
		const ResUtil::Matrix& getRawMotion() const;
   	private:
		ResUtil::Matrix mRawmotion;
 		std::string mAnimtionName;	
		std::map<std::string,AnimationTrack*> mTrackList;
	};
	//------------------------------------------------------------------------

	class AnimationState
	{
	public:
		AnimationState(const Animation* anim);
		virtual ~AnimationState(){};
		
		void    attachAnimation(const Animation* anim);

		size_t  getTotalFrame() const {return mTotalFrame;}

		size_t  getCurrentFrame() const {return mCurrentFrame;}
		void    setCurrentFrame(const size_t currentFrame) {mCurrentFrame = currentFrame;}
 		bool    getLoop() const { return mLoop;}
		void    setLoop(const bool loop){mLoop = loop;}
 		bool	getEnabled() const {return mEnabled;}
		void    setEnabled(bool enabled) {mEnabled = enabled;}
 		void    update(Ogre::SceneNode *node,std::string boneName);
 		void    updataState();
	private:
 		bool       mLoop;
		bool       mEnabled;
		size_t     mTotalFrame;
		size_t     mCurrentFrame;
		const Animation *mAnimPointer;
 	};

	//------------------------------------------------------------------------
	class AnimationManager: public Ogre::Singleton<AnimationManager>,public UIComponent
	{
 	public:
		AnimationManager();
		virtual ~AnimationManager();
 
		void createAnimtion(ResUtil::Matrix *Motion,std::string name);
  		const Animation* getAnimationByName(std::string animationName) const;

		Ogre::StringVector getAllAnimtion() const;
		void sliderMoved(OgreBites::Slider * slider);
		void buttonHit(OgreBites::Button* button);
	private:
 		int mAcivateAnimtion;
		AppFileExplorer *mFileExplorer;
		std::map<std::string,Animation*> mAvaliableAnimtion;
	};

}
#endif