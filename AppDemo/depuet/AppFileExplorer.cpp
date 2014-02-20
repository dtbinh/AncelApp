#include "AppFileExplorer.h"
#include <windows.h>


using namespace AncelApp;

int AppFileExplorer::numExplorer = 0;
AppFileExplorer::AppFileExplorer(const std::string &initialPath,const std::string &filter,FileExplorerListerner *listener)
{
  
	std::string name = "AppFileExplorer : " + Ogre::StringConverter::toString(AppFileExplorer::numExplorer);
	mExplorerName = name;
	
	AppFileExplorer::numExplorer++;

	mTrayMgr = new OgreBites::SdkTrayManager(name,AppDemo::getSingletonPtr()->mRenderWnd,AppDemo::getSingletonPtr()->mMouse,this);
	
	
	Ogre::StringVector sv;
	sv.push_back("C:\\");
	sv.push_back("D:\\");
	sv.push_back("E:\\");

	OgreBites::TrayLocation tLoc = OgreBites::TL_CENTER;

 	mTrayMgr->createSeparator(tLoc,name + "Line_Two",500);
  	mTrayMgr->createLabel(tLoc,name + "LB_Title","File Explorer",150);
	mTrayMgr->createSeparator(tLoc,name + "Line_Four",600);
 	mTrayMgr->createLongSelectMenu(tLoc,name + "DiskMenu","Disk",450,3,sv);
	mTrayMgr->createSeparator(tLoc,name + "Line_Five",600);
 	mTrayMgr->createLongSelectMenu(tLoc,name + "PathMenu","Path",450,10);
	mTrayMgr->createButton(tLoc,name + "BT_ok","ok",100);
	mTrayMgr->createButton(tLoc,name + "BT_cancel","cancel",100);
	mTrayMgr->createSeparator(tLoc,name + "Line_Six",600); 
	mTrayMgr->hideAll();
	
	mCurrentFilePath.resize(3);

	if(initialPath[0] == 'C' || initialPath[0] == 'c')
		mCurrentFilePath[0] = initialPath;
	else 
		mCurrentFilePath[0] = "C:\\";
	if(initialPath[0] == 'D' || initialPath[0] == 'd')
		mCurrentFilePath[1] = initialPath;
	else 
		mCurrentFilePath[1] = "D:\\";
	if(initialPath[0] == 'E' || initialPath[0] == 'e')
		mCurrentFilePath[2] = initialPath;
	else 
		mCurrentFilePath[2] = "E:\\";

	mInitialFilePath = initialPath;
	mExplorerListener = listener;
	mOldMouseListener = NULL;

	mFilter = filter;
	//updateItems(initialPath);
	
}
void AppFileExplorer::show()
{
	mOldMouseListener = AppDemo::getSingleton().mMouse->getEventCallback();
	AppDemo::getSingleton().mMouse->setEventCallback(this);
	OgreBites::SelectMenu* diskM = (OgreBites::SelectMenu*)mTrayMgr->getWidget(mExplorerName + "DiskMenu");
 	updateItems(mCurrentFilePath[diskM->getSelectionIndex()]);

	mTrayMgr->showAll();

}
AppFileExplorer::~AppFileExplorer()
{
	AppFileExplorer::numExplorer--;
	mTrayMgr->destroyAllWidgets();
	delete mTrayMgr;
}

void AppFileExplorer::itemSelected(OgreBites::SelectMenu* selMenu)
{
	OgreBites::SelectMenu* diskM = (OgreBites::SelectMenu*)mTrayMgr->getWidget(mExplorerName + "DiskMenu");
 	if(selMenu->getName() == mExplorerName + "PathMenu")
	{
		if(mAttriVec[selMenu->getSelectionIndex()])
		{
			std::string selItem = selMenu->getSelectedItem();
		
			if(selItem == "..")
			{
				int loc = mCurrentFilePath[diskM->getSelectionIndex()].find_last_of('\\');
				mCurrentFilePath[diskM->getSelectionIndex()] = mCurrentFilePath[diskM->getSelectionIndex()].substr(0,loc);
			}
			else 
			{
				if(mCurrentFilePath[diskM->getSelectionIndex()][mCurrentFilePath[diskM->getSelectionIndex()].length()-1] != '\\' 
					&& mCurrentFilePath[diskM->getSelectionIndex()][mCurrentFilePath[diskM->getSelectionIndex()].length()-1] != '/')
					mCurrentFilePath[diskM->getSelectionIndex()] += "\\";
				mCurrentFilePath[diskM->getSelectionIndex()] += selMenu->getSelectedItem();
			}
			int index = diskM->getSelectionIndex();
			diskM->setItems(mCurrentFilePath);
			diskM->selectItem(index,false);
			updateItems(mCurrentFilePath[diskM->getSelectionIndex()]);
		}
 	}
	if(selMenu->getName() == mExplorerName + "DiskMenu")
	{
		int index = diskM->getSelectionIndex();
		diskM->setItems(mCurrentFilePath);
		diskM->selectItem(index,false);
		updateItems(mCurrentFilePath[diskM->getSelectionIndex()]);
	}
}

void AppFileExplorer::buttonHit(OgreBites::Button* button)
{
	OgreBites::SelectMenu* diskM = (OgreBites::SelectMenu*)mTrayMgr->getWidget(mExplorerName + "DiskMenu");
	
	if(button->getCaption() == "ok")
	{
		OgreBites::SelectMenu *selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget(mExplorerName + "PathMenu");
		
		std::string selItem = selMenu->getSelectedItem();
		
		int index = selMenu->getSelectionIndex();
		
		if(mAttriVec[index])
		{
			if(selItem == "..")
			{
				int loc = mCurrentFilePath[diskM->getSelectionIndex()].find_last_of('\\');
				mCurrentFilePath[diskM->getSelectionIndex()] = mCurrentFilePath[diskM->getSelectionIndex()].substr(0,loc);
			}
			else 
			{
				if(mCurrentFilePath[diskM->getSelectionIndex()][mCurrentFilePath[diskM->getSelectionIndex()].length()-1] != '\\' 
					&& mCurrentFilePath[diskM->getSelectionIndex()][mCurrentFilePath[diskM->getSelectionIndex()].length()-1] != '/')
					mCurrentFilePath[diskM->getSelectionIndex()] += "\\";
				mCurrentFilePath[diskM->getSelectionIndex()] += selMenu->getSelectedItem();
			}
			int index = diskM->getSelectionIndex();
			diskM->setItems(mCurrentFilePath);
			diskM->selectItem(index,false);
			updateItems(mCurrentFilePath[diskM->getSelectionIndex()]);
 		 	return;
		}
		else 
		{
 			if(mCurrentFilePath[diskM->getSelectionIndex()][mCurrentFilePath[diskM->getSelectionIndex()].length()-1] != '\\' 
				&& mCurrentFilePath[diskM->getSelectionIndex()][mCurrentFilePath[diskM->getSelectionIndex()].length()-1] != '/')
					mCurrentFilePath[diskM->getSelectionIndex()] += "\\";
			if(mExplorerListener != NULL)
				mExplorerListener->notifyFileOpen(mCurrentFilePath[diskM->getSelectionIndex()] + selMenu->getSelectedItem());
  		}
   	}
	else if(button->getCaption() == "cancel")
	{
		mExplorerListener->notifyFileOpen("");
	}
	AppDemo::getSingleton().mMouse->setEventCallback(mOldMouseListener);
	mTrayMgr->hideAll();
 	
}

bool AppFileExplorer::mouseMoved(const OIS::MouseEvent &evt)
{
	return mTrayMgr->injectMouseMove(evt);
}
bool AppFileExplorer::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return mTrayMgr->injectMouseDown(evt,id);
}
bool AppFileExplorer::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return mTrayMgr->injectMouseUp(evt,id);
}

void  AppFileExplorer::updateItems(std::string path)
{
	mAttriVec.clear();
	mStrVec.clear();
 	
	if(path.length() == 0)	
			return;
	if(path[path.length()-1] != '\\')
		 path += "\\*";
	else 
		 path += "*";
 
	WIN32_FIND_DATAA fd;
	HANDLE hFindFile = FindFirstFileA(path.c_str(), &fd);
	if(hFindFile == INVALID_HANDLE_VALUE)
	{
		::FindClose(hFindFile); return;
	}

  	BOOL bIsDirectory;
 
	BOOL bFinish = FALSE;
	while(!bFinish)
	{
 	 	bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
		
		std::string fileName = fd.cFileName;
		
		if((fileName != ".") && ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0))
		{
			if(bIsDirectory || fileName == "..") 
			{
					mAttriVec.push_back(true);
					Ogre::DisplayString str = fd.cFileName;
 					mStrVec.push_back(str.asUTF8());
			}
			else 
			{
					std::string ext = fileName.substr(fileName.find_last_of('.'));
					if(ext  == mFilter)
					{
						mStrVec.push_back(fd.cFileName);
 						mAttriVec.push_back(false);
 					}
 			}
 			
		}
		bFinish = (FindNextFileA(hFindFile, &fd) == FALSE);
	}
		
	::FindClose(hFindFile);

	OgreBites::SelectMenu *selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget(mExplorerName + "PathMenu");
	selMenu->setItems(mStrVec);
	if(mStrVec.size() > 1)
		selMenu->selectItem(1,false);
}
