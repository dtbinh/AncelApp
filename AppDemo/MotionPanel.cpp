#include "AppDemo.h"
#include "MotionPanel.h"
#include "MotionManager.h"
#include "AppUtility.h"
//#include "SkeletonPanel.h"
#include "AnimationStatePanel.h"
#include "CommandManager.h"
using namespace AncelApp;

template<> MotionPanel* Ogre::Singleton<MotionPanel>::ms_Singleton = 0;
MotionPanel::MotionPanel()
	:BasePanelViewItem("PanelMotion.layout")
{
	
}
MotionPanel::~MotionPanel()
{
	delete MotionManager::getSingletonPtr();
}
void MotionPanel::initialise()
{
	mPanelCell->setCaption("Motion");
 
	assignWidget(mCBAnim,"CBAnimation",true,false);

	MyGUI::Button* btn = nullptr;
 	 	
	assignWidget(btn,"BTPlay",true,false);
 	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionPanel::playMotion);

	assignWidget(btn,"BTRotate",true,false);
 	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionPanel::rotateMotion);

	CommandManager::getInstance().registerCommand("Command_FileLoad", MyGUI::newDelegate(this, &MotionPanel::loadMotion));
	CommandManager::getInstance().registerCommand("Command_FileSave", MyGUI::newDelegate(this, &MotionPanel::saveMotion));
	CommandManager::getInstance().registerCommand("Command_FileClear", MyGUI::newDelegate(this, &MotionPanel::removeMotion));
 	new MotionManager();
}
void MotionPanel::shutdown()
{
}
void MotionPanel::notifyMouseButtonClick(MyGUI::Widget* _sender)
{
	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
}
void MotionPanel::loadMotion(const MyGUI::UString& commandName, bool& result)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("*", HWND(hWnd));

	if (filename != "")
	{
		MatrixXd mat = loadData(filename);
		if (mat.rows() > 0)
		{
			int pos = filename.find_last_of('\\') + 1;
			std::string motionName = filename.substr(pos);
			Motion *anim = new Motion(mat, motionName);
			MotionManager::getSingletonPtr()->addMotion(anim);
 			mCBAnim->addItem(motionName);
			mCBAnim->setIndexSelected(mCBAnim->getItemCount() - 1);
 		}
	}
}
void MotionPanel::saveMotion(const MyGUI::UString& commandName, bool& result)
{
  	std::size_t index = mCBAnim->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		unsigned long hWnd;
		AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

		std::string fileName = AncelApp::saveFile("dat",HWND(hWnd));

		if (fileName != "")
		{
 			if (fileName.find('.') == std::string::npos)
					fileName += ".dat";
		  
			std::string moName = mCBAnim->getItemNameAt(index);
			const Motion* mo =  MotionManager::getSingleton().getMotion(moName);
			if(mo)
				mo->writeToFile(fileName);
		}
 	}
}
void MotionPanel::removeMotion(const MyGUI::UString& commandName, bool& result)
{
   	std::size_t index = mCBAnim->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		std::string motionName = mCBAnim->getItemNameAt(index);
		MotionManager::getSingleton().removeMotion(motionName);
		mCBAnim->removeItemAt(index);
		mCBAnim->clearIndexSelected();
 	}
}
void MotionPanel::playMotion(MyGUI::Widget* sender)
{
 	std::size_t index = mCBAnim->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		std::string  motionName = mCBAnim->getItemNameAt(index);
		const Motion*      mo   = MotionManager::getSingleton().getMotion(motionName);
		const _Skeleton *skel   = SkeletonPanel::getSingleton().getActiveSkeleton();
			
		if(mo && skel) 
		{
			AnimationStatePanel::getSingleton().createAnimState(mo,skel);
 		}
	}
}
void MotionPanel::rotateMotion(MyGUI::Widget* sender)
{
	std::size_t index = mCBAnim->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		std::string  motionName = mCBAnim->getItemNameAt(index);
		Motion* mo = MotionManager::getSingleton().getMotion(motionName);
 			
		if(mo) 
		{
			MyGUI::UString strAngle = static_cast<MyGUI::EditBox*>(mMainWidget->findWidget(mPrefix + "EBAngle"))->getCaption();
			MyGUI::UString strAxisX = static_cast<MyGUI::EditBox*>(mMainWidget->findWidget(mPrefix + "EBAxisX"))->getCaption();
			MyGUI::UString strAxisY = static_cast<MyGUI::EditBox*>(mMainWidget->findWidget(mPrefix + "EBAxisY"))->getCaption();
			MyGUI::UString strAxisZ = static_cast<MyGUI::EditBox*>(mMainWidget->findWidget(mPrefix + "EBAxisZ"))->getCaption();
 	
			float angle = Ogre::StringConverter::parseReal(strAngle);
			float axisX = Ogre::StringConverter::parseReal(strAxisX);
			float axisY = Ogre::StringConverter::parseReal(strAxisY);
			float axisZ = Ogre::StringConverter::parseReal(strAxisZ);
			
			mo->rotateMotion(angle, Ogre::Vector3(axisX, axisY, axisZ));
  		}
	}
}
void MotionPanel::upateMotionList()
{
	mCBAnim->removeAllItems();
	for(std::size_t i = 0; i < MotionManager::getSingleton().size(); i++)
	{
		mCBAnim->addItem(MotionManager::getSingleton().getMotionName(i));
	}
	assert(mCBAnim->getItemCount() > 0);
	mCBAnim->setIndexSelected(mCBAnim->getItemCount() - 1);
}