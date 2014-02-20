#include "StatePanel.h"

#include <OgreStringConverter.h>
#include "AppDemo.h"
#include "AppUtility.h"
#include "MotionManager.h"
#include "SkeletonManager.h"
#include "CommandManager.h"
#include "AnimationManager.h"

using namespace AncelApp;

template<> StatePanel* Ogre::Singleton<StatePanel>::msSingleton = nullptr;

StatePanel::StatePanel()
	:wraps::BaseLayout("StatePanel.layout")
{

	new SkeletonManager();
	new MotionManager();
	new AnimationManager();

	assignWidget(mCBActor, "CB_ACTOR_LIST");
	assignWidget(mCBMotion, "CB_MOTION_LIST");
	assignWidget(mCBAnimation, "CB_ANIMATION_LIST");

	mMainWidget->setPosition(AppDemo::getSingletonPtr()->mRenderWnd->getWidth()- mMainWidget->getWidth(),30);
	MyGUI::Button *btn = nullptr;
	
	//bind event handler
	assignWidget(btn, "BT_REMOVE_ACTOR");
		btn->eventMouseButtonClick += MyGUI::newDelegate(this, &StatePanel::notifyMouseButtonClick);
	
	assignWidget(btn, "BT_REMOVE_MOTION");
		btn->eventMouseButtonClick += MyGUI::newDelegate(this, &StatePanel::notifyMouseButtonClick);
	
	assignWidget(btn, "BT_REMOVE_ANIMATION");
		btn->eventMouseButtonClick += MyGUI::newDelegate(this, &StatePanel::notifyMouseButtonClick);

	assignWidget(btn, "BT_ADD_ACTOR");
		btn->eventMouseButtonClick += MyGUI::newDelegate(this, &StatePanel::notifyMouseButtonClick);
	
	assignWidget(btn, "BT_ADD_MOTION");
		btn->eventMouseButtonClick += MyGUI::newDelegate(this, &StatePanel::notifyMouseButtonClick);
	
	assignWidget(btn, "BT_ADD_ANIMATION");
		btn->eventMouseButtonClick += MyGUI::newDelegate(this, &StatePanel::notifyMouseButtonClick);


	CommandManager::getInstance().registerCommand("Command_FileLoad", MyGUI::newDelegate(this, &StatePanel::loadMotion));
	CommandManager::getInstance().registerCommand("Command_FileSave", MyGUI::newDelegate(this, &StatePanel::saveMotion));
	CommandManager::getInstance().registerCommand("Command_FileClear", MyGUI::newDelegate(this, &StatePanel::removeMotion));

	CommandManager::getInstance().registerCommand("Command_RemoveActor", MyGUI::newDelegate(this, &StatePanel::removeActor));
	CommandManager::getInstance().registerCommand("Command_LoadActor", MyGUI::newDelegate(this, &StatePanel::loadActor));


	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Default", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Brass", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Bronze", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/PolishedBronze", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Chrome", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Copper", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/PolishedCopper", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/PolishedGold", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Silver", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/PolishedSilver", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Emerald", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Jade", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Obsidian", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));
	CommandManager::getInstance().registerCommand("Command_Skeleton/Bone/Pearl", MyGUI::newDelegate(this, &StatePanel::updateActorTheme));	 
}

void StatePanel::updateActorTheme(const MyGUI::UString& commandName, bool& result)
{
	std::string materialname = commandName.substr(commandName.find_first_of('_') + 1);
	Animation * anim = getActiveAnimation();
	if(anim != nullptr)
	{
		anim->upateActorTheme(materialname);
	}
}
StatePanel::~StatePanel()
{
	SkeletonManager::getSingletonPtr();
	MotionManager::getSingletonPtr();
	AnimationManager::getSingletonPtr();
}

void StatePanel::visible()
{
	mMainWidget->setVisible(!mMainWidget->getVisible());
}
bool StatePanel::setVisible(bool visibility)
{
	mMainWidget->setVisible(visibility);
	return true;
}

void StatePanel::addMotion(Motion *mo)
{
	std::string motionName = mo->getName();
	if(motionName != "")
	{
		MotionManager::getSingletonPtr()->addMotion(mo);
		mCBMotion->addItem(motionName);
		mCBMotion->setIndexSelected(mCBMotion->getItemCount() - 1);
	}
}
void StatePanel::loadMotion(const MyGUI::UString& commandName, bool& result)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("dat", HWND(hWnd));
 	if (filename != "")
	{ 
  		std::string motionName = MotionManager::getSingletonPtr()->loadMotion(filename);
		if(motionName != "")
		{
			mCBMotion->addItem(motionName);
			mCBMotion->setIndexSelected(mCBMotion->getItemCount() - 1);
		}
  	}
}
void StatePanel::saveMotion(const MyGUI::UString& commandName, bool& result)
{
	std::size_t index = mCBMotion->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		unsigned long hWnd;
		AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

		std::string fileName = AncelApp::saveFile("dat",HWND(hWnd));

		if (fileName != "")
		{
 			if (fileName.find('.') == std::string::npos)
					fileName += ".dat";
		  
			std::string moName = mCBMotion->getItemNameAt(index);
			MotionManager::getSingleton().saveMotion(moName,fileName);
 		}
 	}
}
void StatePanel::removeMotion(const MyGUI::UString& commandName, bool& result)
{
	std::size_t index = mCBMotion->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
	 	std::string moName = mCBMotion->getItemNameAt(index);
		if(MotionManager::getSingleton().removeMotion(moName))
		{
				mCBMotion->removeItemAt(index);
				mCBMotion->clearIndexSelected();
				if(mCBMotion->getItemCount() > index)
					mCBMotion->setIndexSelected(index);
		}
 	}
}
void StatePanel::notifyMouseButtonClick(MyGUI::Widget* _sender)
{
	MyGUI::ButtonPtr btn = static_cast<MyGUI::ButtonPtr>(_sender);

	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());
	
	bool ret = false;

	if (btnName == "BT_ADD_ACTOR")
		loadActor("",ret);
	else if (btnName == "BT_ADD_MOTION")
		loadMotion("",ret);
	else if (btnName == "BT_REMOVE_MOTION")
		removeMotion("",ret);
	else if (btnName == "BT_REMOVE_ACTOR")
		removeActor("", ret);
	else if(btnName == "BT_ADD_ANIMATION")
		createAnimation("", ret);
	else if(btnName == "BT_REMOVE_ANIMATION") 
		removeAnimation("", ret);
}
Motion* StatePanel::getActiveMotion()
{
	std::size_t index = mCBMotion->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		std::string moName = mCBMotion->getItemNameAt(index);
		
		return MotionManager::getSingleton().getMotion(moName);
	}
	return nullptr;
}
Skeleton*  StatePanel::getActiveActor()
{
	std::size_t index = mCBActor->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		std::string actorName = mCBActor->getItemNameAt(index);
		
		return SkeletonManager::getSingletonPtr()->getActor(actorName);
	}
	return nullptr;
}
Animation*  StatePanel::getActiveAnimation()
{
	std::size_t index = mCBAnimation->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		std::string anim = mCBAnimation->getItemNameAt(index);
		
		return AnimationManager::getSingletonPtr()->getAnimation(anim);
	}
 	return nullptr;
}


void StatePanel::loadActor(const MyGUI::UString& commandName, bool& result)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("xml", HWND(hWnd));
 	if (filename != "")
	{ 
  		std::string actorName = SkeletonManager::getSingletonPtr()->loadActor(filename);
		if (actorName != "")
		{
			mCBActor->addItem(actorName);
			mCBActor->setIndexSelected(mCBActor->getItemCount() - 1);
		}
  	}
}
void StatePanel::removeActor(const MyGUI::UString& commandName, bool& result)
{
	std::size_t index = mCBActor->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
	 	std::string actorName = mCBActor->getItemNameAt(index);
		if(SkeletonManager::getSingleton().removeActor(actorName))
		{
			mCBActor->removeItemAt(index);
			mCBActor->clearIndexSelected();
		}
 	}
}

void StatePanel::removeAnimation(const MyGUI::UString& commandName, bool& result)
{
	std::size_t index = mCBAnimation->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
	 	std::string anim = mCBAnimation->getItemNameAt(index);
		
		if (AnimationManager::getSingleton().removeAnimation(anim))
		{
			mCBAnimation->removeItemAt(index);
			mCBAnimation->clearIndexSelected();
		}
  	}

}
void StatePanel::createAnimation(const MyGUI::UString& commandName, bool& result)
{
	Motion *mo = getActiveMotion();
	Skeleton *skel = getActiveActor();

	if (mo != nullptr && skel != nullptr)
	{
		if (!AnimationManager::getSingleton().isSkeletonUsed(skel))
		{
			std::string anim = AnimationManager::getSingleton().addAnimation(mo,skel);
			mCBAnimation->addItem(anim);
			mCBAnimation->setIndexSelected(mCBAnimation->getItemCount() - 1);
		}
	}
}
void StatePanel::windowResized(Ogre::RenderWindow* rw)
{
	mMainWidget->setPosition(AppDemo::getSingletonPtr()->mRenderWnd->getWidth()- mMainWidget->getWidth(),30);
}