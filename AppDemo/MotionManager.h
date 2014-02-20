#ifndef __MotionManager_h
#define __MotionManager_h

#include <map>
#include <vector>
#include <OgreSingleton.h>

#include "Motion.h"

namespace AncelApp
{
	class MotionManager: public Ogre::Singleton<MotionManager> 
	{
	public:
		MotionManager();
		~MotionManager();
		void  addMotion(const Motion *anim);
		
		std::string loadMotion(const std::string& filename);
		bool  saveMotion(const std::string &animName, const std::string& fileName) ;//const;

 		const Motion* getMotion(const std::string& animName) const;
			  Motion* getMotion(const std::string& animName);
		bool  removeMotion(const std::string animName);
		const std::string&  getMotionName(std::size_t index);
		const std::size_t size() const; //return the total number of avaliable motion;
	private:
		//TODO List may be more efficient
 		std::vector<Motion*>	mMotion;
	};
}

#endif