#include "AppUIManager.h"
#include "ActorAttributesExplorer.h"

using namespace AncelApp;

//UIFrameWindow::UIFrameWindow(std::string layoutFileName)
//{
//	mFramewWnd = CEGUI::WindowManager::getSingleton().loadWindowLayout(layoutFileName);
//}
//
//UIFrameWindow::~UIFrameWindow()
//{
//}

Explorer::Explorer(std::string layoutname)
{
	mSheet = CEGUI::WindowManager::getSingleton().loadWindowLayout(layoutname);
}

CEGUI::Window* Explorer::getRootWindow()
{
	return mSheet;
}





template<> AppUIManager* Ogre::Singleton<AppUIManager>::ms_Singleton = 0;

AppUIManager::AppUIManager()
	:mUIRenderer(NULL)
{
}
AppUIManager::~AppUIManager()
{
}


bool AppUIManager::bootstrapUI()
{
	if(mUIRenderer != NULL) 
		return true;
	
	mUIRenderer = &CEGUI::OgreRenderer::bootstrapSystem();

	CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
	CEGUI::Font::setDefaultResourceGroup("Fonts");
	CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
	CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

	CEGUI::SchemeManager::getSingleton().create("OgreTray.scheme");
	//CEGUI::System::getSingleton().setDefaultMouseCursor("Vanilla", "MouseArrow");
	//mSheet = CEGUI::WindowManager::getSingleton().loadWindowLayout("Demo8.layout");
	mSheet = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "AppeDemo/Sheet");
	new ActorAttributesExplorer();
	mSheet->addChildWindow(ActorAttributesExplorer::getSingleton().getRootWindow());




//
//	CEGUI::Window *quit = wmgr.createWindow("SleekSpace/HorizontalScrollbar", "CEGUIDemo/QuitButton");
////	quit->setText("Quit");
//
//	quit->setPosition(CEGUI::UVector2(CEGUI::UDim(0.55, 0), CEGUI::UDim(0.15, 0)));
//	quit->setSize(CEGUI::UVector2(CEGUI::UDim(0.85, 0), CEGUI::UDim(0.15, 0)));
//	CEGUI::FrameWindow *fWmd = (CEGUI::FrameWindow*)wmgr.createWindow("SleekSpace/FrameWindow","fWmd");
//	
//	fWmd->setText("hello world");
//	fWmd->setAlpha(0.8);
//	fWmd->setPosition(CEGUI::UVector2(CEGUI::UDim(0.2f,0), CEGUI::UDim( 0.2f, 0)));
//	fWmd->setSize(CEGUI::UVector2(CEGUI::UDim(0.2f,0), CEGUI::UDim( 0.2f, 0)));
//	fWmd->addChildWindow(quit);
//	//sheet->addChildWindow(quit);
//	sheet->addChildWindow(fWmd);
	CEGUI::System::getSingleton().setGUISheet(mSheet);
	return true;
}

CEGUI::MouseButton AppUIManager::convertButton(OIS::MouseButtonID buttonID)
{
	switch (buttonID)
    {
    case OIS::MB_Left:
        return CEGUI::LeftButton;
 
    case OIS::MB_Right:
        return CEGUI::RightButton;
 
    case OIS::MB_Middle:
        return CEGUI::MiddleButton;
 
    default:
        return CEGUI::LeftButton;
    }
}

bool AppUIManager::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	return true;
}
bool AppUIManager::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	return true;
}

bool AppUIManager::mouseMoved(const OIS::MouseEvent &evt)
{
	return true;
}
bool AppUIManager::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return true;
}
bool AppUIManager::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return true;
}

bool AppUIManager::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	return true;
}
