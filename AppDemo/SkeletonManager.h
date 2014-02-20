#ifndef _SkeletonManager_h_
#define _SkeletonManager_h_

#include <OgreSingleton.h>
#include "Skeleton.h"
#include <vector>
namespace AncelApp
{
	class SkeletonManager: public Ogre::Singleton<SkeletonManager>
	{
	public:

		SkeletonManager();
		~SkeletonManager();

		void	   destoryAll();
		bool       removeActor(const std::string& skelName);

		Skeleton*   getActor(const std::string& skelName);
		std::string  loadActor(const std::string& filename);
 
		//Skeleton* createSkeleton(const std::string& skelName, const std::string& templateName); 
	private:
		SkeletonManager(const SkeletonManager&);
		SkeletonManager operator =(const SkeletonManager&);
	private:
 		std::vector<Skeleton*>  mSkeletons;
  	};
}
#endif