#ifndef __AnimationManager_h
#define __AnimationManager_h

#include <map>
#include <vector>
#include <OgreSingleton.h>

#include "Animation.h"
#include "Motion.h"
#include "Skeleton.h"

namespace AncelApp
{
	 
	class AnimationManager: public Ogre::Singleton<AnimationManager>, public Ogre::FrameListener 
	{
	public:
		AnimationManager();
		~AnimationManager();
		
		bool frameRenderingQueued(const Ogre::FrameEvent& evt);

		std::string addAnimation(const Motion *anim,const Skeleton *skel);
	 	const Animation* getAnimation(const std::string& animName) const;
			  Animation* getAnimation(const std::string& animName);
  		void  playAll();
		bool  isMotionUsed(const Motion *skel);
		bool  isSkeletonUsed(const Skeleton *skel);
 		bool  removeAnimation(const std::string animName);
		const std::string&  getAnimationName(std::size_t index);
		const std::size_t size() const; //return the total number of avaliable motion;
	private:
		//TODO List may be more efficient
 		std::vector<Animation*>	mAnimation;
	};
}

#endif