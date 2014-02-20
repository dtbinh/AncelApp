#ifndef _SceneEntityManager_h
#define _SceneEntityManager_h

#include "SceneEntity.h"
#include <OgreSingleton.h>
#include "MotionGraphs.h"
#include <vector>
#include <MyGUI.h>
namespace AncelApp
{
	class SceneEntityManager :public Ogre::Singleton<SceneEntityManager>
	{
	public:
		
		SceneEntityManager();
		~SceneEntityManager();
		void addSceneEntity(const MyGUI::UString& commandName, bool& result);
		void removeEntity(const std::string& meshName);
		void loadScene(const MyGUI::UString& commandName, bool& result);
		void saveScene(const MyGUI::UString& commandName, bool& result);
		void addOneEntity();
	private:
		std::vector<SceneEntity*> mEntities;
		//MotionGraphs mMotionGraphs;
	};
}

#endif



