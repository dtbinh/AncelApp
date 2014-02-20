#include "SceneEntityManager.h"
#include "CommandManager.h"
#include "AppUtility.h"
#include "AppDemo.h"
#include "MotionGraphs.h"
#include <math.h>
using namespace AncelApp;

template<> SceneEntityManager* Ogre::Singleton<SceneEntityManager>::msSingleton = 0;

SceneEntityManager::SceneEntityManager()
{
	CommandManager::getInstance().registerCommand("Command_LoadMesh", MyGUI::newDelegate(this, &SceneEntityManager::addSceneEntity));
	CommandManager::getInstance().registerCommand("Command_SceneLoad", MyGUI::newDelegate(this, &SceneEntityManager::loadScene));
	CommandManager::getInstance().registerCommand("Command_SceneSave", MyGUI::newDelegate(this, &SceneEntityManager::saveScene));
}

void SceneEntityManager::addSceneEntity(const MyGUI::UString& commandName, bool& result)
{
	unsigned long hWnd;
	
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("mesh", HWND(hWnd));
 
	if (filename != "")
	{
		::SetCurrentDirectoryA(AppDemo::getSingletonPtr()->mWorkDirectory.c_str());
		filename = filename.substr(filename.find_last_of('\\')+1,std::string::npos);
		SceneEntity *scent =  new SceneEntity(filename);
		mEntities.push_back(scent);
	}

}

void SceneEntityManager::addOneEntity()
{
	std::string filename = "media\\sceneEntity\\book.mesh";

	if (filename != "")
	{
		::SetCurrentDirectoryA(AppDemo::getSingletonPtr()->mWorkDirectory.c_str());
		std::ifstream sceneLoader(filename);
		//filename = filename.substr(filename.find_last_of('\\')+1,std::string::npos);
		SceneEntity *scent =  new SceneEntity(filename);
		//scent->setScale()
		mEntities.push_back(scent);
	}
}


void SceneEntityManager::loadScene(const MyGUI::UString& commandName, bool& result)
{
	unsigned long hWnd;
	
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("scene", HWND(hWnd));

	if (filename != "")
	{
		::SetCurrentDirectoryA(AppDemo::getSingletonPtr()->mWorkDirectory.c_str());
		//filename = filename.substr(filename.find_last_of('\\')+1,std::string::npos);

		std::ifstream sceneLoader(filename);

		int entityNumber = 0;
		sceneLoader >> entityNumber;

		for(std::size_t i = 0; i < entityNumber; i++)
		{
			std::string entityName;
			sceneLoader >> entityName;
 			SceneEntity *scent =  new SceneEntity(entityName);
			
			float rotateAngel = 0;
			Ogre::Vector3 pos,scale;
			sceneLoader >> pos.x >> pos.y >> pos.z;
			sceneLoader >> scale.x >> scale.y >> scale.z;
			sceneLoader >> rotateAngel;
			scent->setScale(scale);
			scent->setPosition(pos);
			scent->setRotation(rotateAngel);

/*
			int temx = int((pos.x + 400)/8);
			int temz = int((pos.z - 400)/8);*/
			int temx = int((pos.x + 400)/8);
			int temz = int((pos.z + 400)/8);
			bool flag = false;

			if (entityName == "book.mesh")   // 18 25   z  x
			{
				for (int ii = 1; ii <= (int)scale.x*3; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z*2; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x*3/2+ii,temz-(int)scale.z+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "chair.mesh")  // 9,9
			{
				for (int ii = 1; ii <= (int)scale.x; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x/2+ii,temz-(int)scale.z/2+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "Circle.319.mesh") // 3,3
			{
				for (int ii = 1; ii <= (int)scale.x/2; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z/2; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x/4+ii,temz-(int)scale.z/4+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "stairs.mesh") // 37,30
			{
				for (int ii = 1; ii <= (int)scale.x*4; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z*5; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x*2+ii,temz-(int)scale.z*5/2+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "Stol_02_struct.034.mesh")  // 7 ,8
			{
				for (int ii = 1; ii <= (int)scale.x; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x/2+ii,temz-(int)scale.z/2+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "Trash_can.mesh") //  7,7
			{
				for (int ii = 1; ii <= (int)scale.x; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x/2+ii,temz-(int)scale.z/2+jj);
						flag = true;
					}
				}
			}
			if (!flag)
			{
				MotionGraphs::getSingletonPtr()->setMap(temx,temz);
			}
/*
			int temx = int((pos.x + 400)/4);
			int temz = int((pos.z - 400)/4);
			bool flag = false;

			if (entityName == "book.mesh")   // 18 25   z  x
			{
				for (int ii = 1; ii <= (int)scale.x*6; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z*4; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x*3+ii,temz-(int)scale.z*2+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "chair.mesh")  // 9,9
			{
				for (int ii = 1; ii <= (int)scale.x*2; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z*2; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x+ii,temz-(int)scale.z+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "Circle.319.mesh") // 3,3
			{
				for (int ii = 1; ii <= (int)scale.x; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x/2+ii,temz-(int)scale.z/2+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "stairs.mesh") // 37,30
			{
				for (int ii = 1; ii <= (int)scale.x*7; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z*10; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x*4+ii,temz-(int)scale.z*5+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "Stol_02_struct.034.mesh")  // 7 ,8
			{
				for (int ii = 1; ii <= (int)scale.x*2; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z*2; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x+ii,temz-(int)scale.z+jj);
						flag = true;
					}
				}
			}
			else if (entityName == "Trash_can.mesh") //  7,7
			{
				for (int ii = 1; ii <= (int)scale.x*2; ii++)
				{
					for (int jj = 1; jj <= (int)scale.z*2; jj++)
					{
						MotionGraphs::getSingletonPtr()->setMap(temx-(int)scale.x+ii,temz-(int)scale.z+jj);
						flag = true;
					}
				}
			}
			if (!flag)
			{
				MotionGraphs::getSingletonPtr()->setMap(temx,temz);
			}
*/
			mEntities.push_back(scent);
		}
		sceneLoader.close();
	}
}
void SceneEntityManager::saveScene(const MyGUI::UString& commandName, bool& result)
{
	unsigned long hWnd;
	
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::saveFile("scene", HWND(hWnd));

	if (filename != "")
	{
		::SetCurrentDirectoryA(AppDemo::getSingletonPtr()->mWorkDirectory.c_str());
		//filename = filename.substr(filename.find_last_of('\\')+1,std::string::npos);

		std::ofstream scenewriter(filename);

		int entityNumber = 0;
		scenewriter << mEntities.size() << " ";

		for(std::size_t i = 0; i < mEntities.size(); i++)
		{
			 
			scenewriter << mEntities[i]->getMeshName() << " ";
			scenewriter << mEntities[i]->getPosition().x << " "; 
			scenewriter << mEntities[i]->getPosition().y << " ";
			scenewriter << mEntities[i]->getPosition().z << " ";
 
			scenewriter << mEntities[i]->getScale().x << " "; 
			scenewriter << mEntities[i]->getScale().y << " ";
			scenewriter << mEntities[i]->getScale().z << " ";

			scenewriter << mEntities[i]->getRotation() << " ";
			
			scenewriter << std::endl;
 		}

		scenewriter.close();
	}
}



SceneEntityManager::~SceneEntityManager()
{
	for(std::size_t i = 0; i < mEntities.size(); i++)
		delete mEntities[i];
}
void SceneEntityManager::removeEntity(const std::string& meshName)
{
	for(std::size_t i = 0; i < mEntities.size(); i++)
	{
		if(mEntities[i]->getEntityName() == meshName)
		{
			delete mEntities[i];
			mEntities.erase(mEntities.begin() + i);
			break;
		}
	}
}