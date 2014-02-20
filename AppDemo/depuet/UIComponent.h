/**
*-----------------------------------------------------------------------------
*Filename:  GUIManager.h
*-----------------------------------------------------------------------------
*File Description: the class is used to manager GUI Units
*-----------------------------------------------------------------------------
*Author: Ancel         2011/11/21               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/


#ifndef __GUIManager_h_
#define __GUIManager_h_

#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreSingleton.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <vector>


namespace AncelApp
{
	

	class UIComponent: public OgreBites::SdkTrayListener
	{
	public:
		UIComponent(const Ogre::String &componentName);
		virtual ~UIComponent();
		
		virtual bool keyPressed(const OIS::KeyEvent &keyEventRef);
		virtual bool keyReleased(const OIS::KeyEvent &keyEventRef);

		virtual bool mouseMoved(const OIS::MouseEvent &evt);
		virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

		virtual void changeStatus();
		virtual void update(const Ogre::FrameEvent& evt);
	 	Ogre::String getName() const;
 	protected:
		bool mStatus;
		Ogre::String mComponentName;
		OgreBites::SdkTrayManager *mTrayMgr;
 	};
	//------------------------------------------//------------------------------------------------------

	class GUIManager: public OIS::KeyListener, public OIS::MouseListener, public OgreBites::SdkTrayListener,public Ogre::FrameListener,public Ogre::Singleton<GUIManager> 
	{
	public:

		GUIManager();
		~GUIManager();
 
		bool keyPressed(const OIS::KeyEvent &keyEventRef);
		bool keyReleased(const OIS::KeyEvent &keyEventRef);

		bool mouseMoved(const OIS::MouseEvent &evt);
		bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

		bool append(UIComponent* ui,Ogre::String materialName,bool isActived = false);
 
	 	void changeStatus();
		
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
  	private:

		bool   mStatus;

		double mSelectedItemLoc;

		std::size_t mCurrentSelItem;
 
 		OgreBites::SdkTrayManager *mTrayMgr;

		OIS::KeyListener* mOldKeyListener;
		OIS::MouseListener* mOldMouseListener;

		std::vector<UIComponent*> mComponentList;
		std::vector<Ogre::OverlayContainer*> mIconList;

		std::vector<std::size_t> mActivateComponentList;
  	};
}
#endif