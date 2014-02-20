#include "SkeletonManager.h"
#include "AppDemo.h"
#include "AnimationManager.h"
using namespace AncelApp;

template<> SkeletonManager* Ogre::Singleton<SkeletonManager>::msSingleton = 0;

SkeletonManager::SkeletonManager()
{
	
}
SkeletonManager::~SkeletonManager()
{
	destoryAll();
}

bool   SkeletonManager::removeActor(const std::string& skelName)
{
  	for(std::size_t i = 0; i < mSkeletons.size(); i++)
	{
		if(mSkeletons[i]->getName() == skelName)
		{
			if(!AnimationManager::getSingletonPtr()->isSkeletonUsed(mSkeletons[i]))
			{
				delete mSkeletons[i];
				mSkeletons.erase(mSkeletons.begin() + i);
				return true;
			}
			return false;
		}
	}
 	return false;
}
void SkeletonManager::destoryAll()
{
	for(std::size_t i = 0; i < mSkeletons.size(); i++)
	{
		delete mSkeletons[i];
	}
	mSkeletons.clear();
}

Skeleton* SkeletonManager::getActor(const std::string& skelName)
{
	for(std::size_t i = 0; i < mSkeletons.size(); i++)
	{
		if(mSkeletons[i]->getName() == skelName)
		{
			return mSkeletons[i];
		}
	}
	return NULL;
}

std::string SkeletonManager::loadActor(const std::string& filename)
{
  	 std::string skleName = filename.substr(filename.find_last_of('\\')+1);
	 Skeleton* skel = new Skeleton(skleName);
	 skel->loadSkeletonFromXML(filename);
 	 mSkeletons.push_back(skel);
	 ::SetCurrentDirectoryA(AppDemo::getSingletonPtr()->mWorkDirectory.c_str());
	 if(skel)
	  	skel->attachSkeletonToScene(AppDemo::getSingleton().mSceneMgr);

	 return skel->getName();
}
