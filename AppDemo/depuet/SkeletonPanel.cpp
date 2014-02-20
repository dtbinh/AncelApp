#define TIXML_USE_STL
#define _SCL_SECURE_NO_WARNINGS

#include "AppDemo.h"
#include "SkeletonPanel.h"
#include <tinyxml.h>
#include <Windows.h>
#include "CommandManager.h"

using namespace AncelApp;

template<> SkeletonPanel* Ogre::Singleton<SkeletonPanel>::ms_Singleton = 0;
SkeletonPanel::~SkeletonPanel()
{
	delete SkeletonManager::getSingletonPtr();
}

SkeletonPanel::SkeletonPanel()
	:mCBSkel(0),
	mSkeletonCounter(0),
	BasePanelViewItem("SkeletonsPanel.layout")
{

	
}
bool SkeletonPanel::loadAvaliableTemplates(const std::string& fileName)
{
	TiXmlDocument* skelDoc = new TiXmlDocument();
	if(!skelDoc->LoadFile(fileName))
		throw std::logic_error("Load Skeleton File Failed !!");
	
	TiXmlElement *rootEle = skelDoc->FirstChildElement();
	TiXmlElement *ele = rootEle->FirstChildElement();
	
	while(ele)
	{
		std::string templateName;
		ele->QueryStringAttribute("file",&templateName);
		mAvaliableTemplates.push_back(templateName);
		ele = rootEle->NextSiblingElement();
	}
	
	return true;
}

void SkeletonPanel::createActor(const MyGUI::UString& commandName, bool& result)
{
 	MyGUI::ComboBox* templates = static_cast<MyGUI::ComboBox*>(mMainWidget->findWidget(mPrefix + "CBTemplates"));
		
	MyGUI::UString str = mTemplateDir + "\\data\\templates\\" + templates->getItemNameAt(templates->getIndexSelected());
	std::string skeletonName = "skel_" + Ogre::StringConverter::toString(mSkeletonCounter++);
	_Skeleton* skel = SkeletonManager::getSingletonPtr()->createSkeleton(skeletonName,str);
	::SetCurrentDirectoryA(mTemplateDir.c_str());
	if (skel)
	{
		skel->attachSkeletonToScene(AppDemo::getSingleton().mSceneMgr);
 		 
		mCBSkel->addItem(skeletonName);
		mCBSkel->setIndexSelected(mCBSkel->getItemCount() - 1);
	}
}

void SkeletonPanel::removeActor(const MyGUI::UString& commandName, bool& result)
{
 	std::size_t index = mCBSkel->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		MyGUI::UString skeletonName = mCBSkel->getItemNameAt(index);
		SkeletonManager::getSingletonPtr()->destorySkeleton(skeletonName);
 		mCBSkel->removeItemAt(mCBSkel->getIndexSelected());
		mCBSkel->clearIndexSelected();
	}
}

void SkeletonPanel::windowResized(int width, int height)
{
	MyGUI::IntSize size = mMainWidget->getSize();
//	mWidgets.at(0)->setPosition(0,0);
//	mWidgets.at(0)->setSize(size.width, height);
}
const _Skeleton* SkeletonPanel::getActiveSkeleton() const
{
 	std::size_t index = mCBSkel->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		MyGUI::UString skeletonName = mCBSkel->getItemNameAt(index);
		return SkeletonManager::getSingletonPtr()->getSkeleton(skeletonName);
 	}
	return NULL;
}
void SkeletonPanel::initialise()
{
	char dir[MAX_PATH];
	::GetCurrentDirectoryA(MAX_PATH,dir);	
	mTemplateDir  = dir;
//	mTemplateDir += "\\data\\templates\\";
	
	new SkeletonManager();
	mPanelCell->setCaption("Actors");

	loadAvaliableTemplates("data/templates/templates.xml");
	
	assignWidget(mCBSkel, "CBSkeletons", false);
	 
	MyGUI::ComboBox* templates = nullptr;
	assignWidget(templates, "CBTemplates", false);
 
	for(std::size_t i = 0; i < mAvaliableTemplates.size(); i++)
	 	templates->addItem(mAvaliableTemplates[i]);
	if(mAvaliableTemplates.size() > 0)
		templates->setIndexSelected(0);

	CommandManager::getInstance().registerCommand("Command_CreateActor", MyGUI::newDelegate(this, &SkeletonPanel::createActor));
	CommandManager::getInstance().registerCommand("Command_RemoveActor", MyGUI::newDelegate(this, &SkeletonPanel::removeActor));
 
	if(mAvaliableTemplates.size() > 0)
	{
		bool result;
		createActor("Command_CreateActor", result);
	}
}
void SkeletonPanel::shutdown()
{

}
