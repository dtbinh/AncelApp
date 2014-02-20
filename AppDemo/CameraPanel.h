/**
*-----------------------------------------------------------------------------
*Filename:  CameraModeUI.h
*-----------------------------------------------------------------------------
*File Description: the class is used as a UI for managing camera status
*-----------------------------------------------------------------------------
*Author: Ancel         2012/03/02               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __CameraPanel_h_
#define __CameraPanel_h_

#include <OgreCamera.h>
#include <OgreSingleton.h>
#include <MyGUI_Button.h>

#include <OgreSceneQuery.h>
#include "Path.h"

namespace AncelApp
{
	class CameraPanel:public Ogre::Singleton<CameraPanel>
	{
	public:
		CameraPanel(Ogre::Camera *cam);
		~CameraPanel();
		virtual bool mouseMoved(const OIS::MouseEvent &evt);
		virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
 		
		void notifyMouseButtonClick(MyGUI::Widget* _sender);
		
	private:
		MyGUI::Button *mBtnFixed;
		MyGUI::Button *mBtnTracking;
		MyGUI::Button *mBtnManipulable;
   
		std::size_t    mCamMode;
 		Ogre::Camera*  mCamera;
   	};
}

#endif


