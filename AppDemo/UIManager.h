#ifndef __UIManager_h_
#define __UIManager_h_
  
#include <OgreSingleton.h>

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>


namespace AncelApp
{
 	class UIManager:public Ogre::Singleton<UIManager>
	{
	public:
 		UIManager();
		~UIManager();
		bool bootUI(const Ogre::RenderWindow *wnd, Ogre::SceneManager *mgr);
		void windowResized(Ogre::RenderWindow* rw);
	protected:
		
		void initialiseUI(const Ogre::RenderWindow* wnd);
	private:
 	 		
 		MyGUI::Gui*          mDemoGUI;
		MyGUI::OgrePlatform* mPlatform;
	};

};
#endif