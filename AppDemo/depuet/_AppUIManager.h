/**
*-----------------------------------------------------------------------------
*Filename:  AppUIManager.h
*-----------------------------------------------------------------------------
*File Description: the class used to manager all the ui components of the demoapp
*-----------------------------------------------------------------------------
*Author: Ancel         2012/5/30               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __AppUIManager_h_
#define __AppUIManager_h_

#include <CEGUI.h>
#include <RendererModules/Ogre/CEGUIOgreRenderer.h>



#include "AppDemo.h"
#include <string>

namespace AncelApp
{
	class Explorer
	{
	public:
		Explorer(std::string layoutname);

		CEGUI::Window* getRootWindow();
	protected:
		CEGUI::Window *mSheet;
	};

  	class AppUIManager: public Ogre::Singleton<AppUIManager>, public OIS::KeyListener, public OIS::MouseListener, public Ogre::FrameListener
	{
	public:
		AppUIManager();
		~AppUIManager();
		bool bootstrapUI();

		bool keyPressed(const OIS::KeyEvent &keyEventRef);
		bool keyReleased(const OIS::KeyEvent &keyEventRef);

		bool mouseMoved(const OIS::MouseEvent &evt);
		bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

		static CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID);
 	private:
  		AppUIManager(const AppUIManager&);
		bool operator = (const AppUIManager&);

		CEGUI::Window *mSheet;
		CEGUI::OgreRenderer *mUIRenderer;
	};
}
#endif


