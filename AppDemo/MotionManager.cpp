#include "MotionManager.h"
#include "AppUtility.h"
#include "AnimationManager.h"

using namespace AncelApp;

template<> MotionManager* Ogre::Singleton<MotionManager>::msSingleton = 0;

MotionManager::MotionManager()
{

}
MotionManager::~MotionManager()
{
	for(std::size_t i = 0; i < mMotion.size(); i++)
		delete mMotion[i];
	mMotion.clear();
}
void  MotionManager::addMotion(const Motion *anim)
{
	if(anim) 	mMotion.push_back(const_cast<Motion*>(anim));
}
const Motion* MotionManager::getMotion(const std::string& animName) const
{
	for(std::size_t i = 0; i < mMotion.size(); i++)
	{
		if(mMotion[i]->getName() == animName)
			return mMotion[i];
	}
	return NULL;
}
Motion* MotionManager::getMotion(const std::string& animName)
{
	for(std::size_t i = 0; i < mMotion.size(); i++)
	{
		if(mMotion[i]->getName() == animName)
			return mMotion[i];
	}
	return NULL;
}
bool MotionManager::removeMotion(const std::string animName)
{
	std::vector<Motion*>::iterator it;
	for(it = mMotion.begin(); it !=  mMotion.end(); it++)
	{
		if((*it)->getName() == animName)
		{
			if(!AnimationManager::getSingletonPtr()->isMotionUsed(*it))
			{
				delete *it;
				mMotion.erase(it);
				return true;
			}
			return false;
		}
	}
	return false;
}
const std::size_t MotionManager::size() const
{
	return mMotion.size();
}
const std::string&  MotionManager::getMotionName(std::size_t index)
{
	assert(index < mMotion.size());
	return mMotion[index]->getName();
}

std::string MotionManager::loadMotion(const std::string& filename)
{
	MatrixXd mat = loadData(filename);
	if (mat.rows() > 0)
	{
		int pos = filename.find_last_of('\\') + 1;
		std::string motionName = filename.substr(pos);
		Motion *anim = new Motion(mat, motionName);
			
		mMotion.push_back(anim);
 		return anim->getName();
 	}
	return "";
}
bool  MotionManager::saveMotion(const std::string &animName, const std::string& fileName) //const
{
	 Motion* anim = getMotion(animName);
	if(anim)
	{
		anim->writeToFile(fileName);
		return true;
	}
	return false;
}