#ifndef __AnimationState_h
#define __AnimationState_h

#include "Skeleton.h"
#include "Motion.h"

namespace AncelApp
{  
	class AnimationEditor;

 	class Animation 
	{
	public:
		Animation(const Skeleton* skel, const Motion *anim);
		Animation::~Animation();

		void	update(double timeSinceLastFrame);
		void    skipTo(const std::size_t& frameNum);
	 
		void    setUpdateSpeed(const double updateSpeed) {mUpdateSpeed = updateSpeed;}
		double  getUpdateSpped() const  {return mUpdateSpeed;}
		
		bool    getLoop()  const               { return mEnableLoop;}
		void    setLoop(const bool loop)       { mEnableLoop = loop;}
		bool	getEnabled() const             { return mEnable;}
		void    setEnabled(bool enabled)       { mEnable = enabled;}

		bool    isMotionUsed(const Motion* mo)       {return mo == mMotion;}
		bool    isSkeletonUsed(const Skeleton* skel) {return skel == mSkeleton;}
		
		const   std::string & getName() const;
  
		bool  notifyReleased();
		bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset);
 
		virtual bool  updatePosition(const Ogre::Vector3 &pos);
 
		int getTotalFrame()   const {return mTotalFrame;}
		int getCurrentFrame() const {return mCurrentFrame;}

		Skeleton*  getSkeleton() const {return mSkeleton;}
		Motion*    getMotion() const{ return mMotion;}
  		void updateShiftValue(const Ogre::Vector3 &shift);

		void applyShift();
		void applyRotation();
		void upateActorTheme(const std::string& themeName);
 
	private:
		const static  int  ST_ACTIVE   = 0;
		const static  int  ST_INACTIVE = 1;
 
		std::size_t      mTotalFrame;
		std::size_t      mCurrentFrame;
	 		
		double           mUpdateSpeed;
  		 	
		bool             mEnable;
		bool             mEnableLoop;
 
		Motion          *mMotion;
		Skeleton		*mSkeleton;   
		
		std::string      mAnimationName;

		AnimationEditor  *mEditor;
		double   timeSinceLastUpdate;
  	};
}
#endif