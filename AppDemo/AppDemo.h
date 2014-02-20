/**
*-----------------------------------------------------------------------------
*Filename:  AppDemo.h
*-----------------------------------------------------------------------------
*File Description: the 
*-----------------------------------------------------------------------------
*Author: Ancel         2011/11/21               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __AppDemo_h_
#define __AppDemo_h_


#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
 
namespace AncelApp
{
	class AppDemo: public Ogre::Singleton<AppDemo>, public OIS::KeyListener,public OIS::MouseListener,public Ogre::WindowEventListener
	{
	public:
		AppDemo();
		virtual ~AppDemo();

		bool initOgre(Ogre::String wndTitle);
		void updateOgre(double timeSinceLastFrame);

		bool keyPressed(const OIS::KeyEvent &keyEventRef);
		bool keyReleased(const OIS::KeyEvent &keyEventRef);

		bool mouseMoved(const OIS::MouseEvent &evt);
		bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
  		
 		void windowResized(Ogre::RenderWindow* rw);

		void run();

		Ogre::Root*					mRoot;
		Ogre::RenderWindow*			mRenderWnd;
 		Ogre::Log*					mLog;
		Ogre::Timer*				mTimer;
		
		OIS::Mouse*					mMouse;
		OIS::Keyboard*				mKeyboard;
		OIS::InputManager*			mInputMgr;

		Ogre::SceneManager*			mSceneMgr;
		Ogre::Camera*				mCamera;
	
		Ogre::Camera*               mCameraAux;

		std::string					mWorkDirectory;
 	private:
		void initOIS();
 	 	void setupResources();
		void initDemo();
 		void createScene();

		AppDemo(const AppDemo&);
		AppDemo& operator= (const AppDemo&); 
 		
 	private:
		bool				 mShutdown;
		//GUIManager*  mUIManager;
  	};
};
#endif 