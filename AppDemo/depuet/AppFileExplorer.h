/**
*-----------------------------------------------------------------------------
*Filename:  AppFileExplorer.h
*-----------------------------------------------------------------------------
*File Description:  
*-----------------------------------------------------------------------------
*Author: Ancel         2011/11/21               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __AppFileExplorer_h
#define __AppFileExplorer_h

#include <vector>
#include <string>
#include "AppDemo.h"

namespace AncelApp 
{
	class FileExplorerListerner
	{
	public:
		FileExplorerListerner(){}
		virtual void notifyFileOpen(const std::string &fileFullPath) = 0;
	};

	class AppFileExplorer: public OgreBites::SdkTrayListener, public OIS::MouseListener
	{
	public:
		AppFileExplorer(const std::string &initialPath,const std::string &filter,
						FileExplorerListerner *listener);
		~AppFileExplorer();
		void show();
		void hide();

		bool mouseMoved(const OIS::MouseEvent &evt);
		bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	 	void itemSelected(OgreBites::SelectMenu* selMenu);
 
		void buttonHit(OgreBites::Button* button);

 	private:
		void updateItems(std::string path);
 	private:
		static int numExplorer;
		FileExplorerListerner *mExplorerListener; 
		
		std::string mInitialFilePath;
		Ogre::StringVector mCurrentFilePath;
		
		std::string mFilter;
		std::string mExplorerName;
		
		OgreBites::SdkTrayManager*	mTrayMgr;
		Ogre::StringVector mStrVec;
		std::vector<bool>  mAttriVec;
		
		OIS::MouseListener *mOldMouseListener;
   	};
}
#endif
