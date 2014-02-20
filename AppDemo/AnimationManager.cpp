#include "AnimationManager.h"
#include "AppDemo.h"
#include "MousePicker.h"
using namespace AncelApp;

template<> AnimationManager* Ogre::Singleton<AnimationManager>::msSingleton = nullptr;

AnimationManager::AnimationManager()
{
	AppDemo::getSingleton().mRoot->addFrameListener(this);
}
AnimationManager::~AnimationManager()
{

}

std::string AnimationManager::addAnimation(const Motion *mo,const Skeleton *skel)
{
	Animation *anim = new Animation(skel, mo);
	mAnimation.push_back(anim);
	return anim->getName();
}
		
const Animation* AnimationManager::getAnimation(const std::string& animName) const
{
	for(std::size_t i = 0; i < mAnimation.size(); i++)
	{
		if(mAnimation[i]->getName() == animName)
		{
			return mAnimation[i];
	 	}
	}
	return nullptr;
}
Animation* AnimationManager::getAnimation(const std::string& animName)
{
	for(std::size_t i = 0; i < mAnimation.size(); i++)
	{
		if(mAnimation[i]->getName() == animName)
		{
			return mAnimation[i];
	 	}
	}
	return nullptr;
}
  		
bool  AnimationManager::removeAnimation(const std::string animName)
{
	for(std::size_t i = 0; i < mAnimation.size(); i++)
	{
		if(mAnimation[i]->getName() == animName)
		{
			MousePicker::getSingletonPtr()->resetPicker();
 			delete mAnimation[i];
			mAnimation.erase(mAnimation.begin() + i);
			return true;
		}
	}
	return false;
}

const std::string&  AnimationManager::getAnimationName(std::size_t index)
{
	return mAnimation[index]->getName();
}

const std::size_t AnimationManager::size() const //return the total number of avaliable motion;
{
	return mAnimation.size();
}


bool  AnimationManager::isMotionUsed(const Motion *mo)
{
	for(std::size_t i = 0; i < mAnimation.size(); i++)
		if(mAnimation[i]-> isMotionUsed(mo))
			return true;
	return false;
}
bool  AnimationManager::isSkeletonUsed(const Skeleton *skel)
{
	for(std::size_t i = 0; i < mAnimation.size(); i++)
		if(mAnimation[i]->isSkeletonUsed(skel))
			return true;
	return false;
}
void  AnimationManager::playAll()
{
	for(std::size_t i = 0; i < mAnimation.size(); i++)
		mAnimation[i]->setEnabled(true);
}

bool AnimationManager::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	for(std::size_t i = 0; i < mAnimation.size(); i++)
	{
		mAnimation[i]->update(evt.timeSinceLastFrame);
	//	std::cout << mAnimation[i]->getName() << std::endl;;
	}
	return true;
}