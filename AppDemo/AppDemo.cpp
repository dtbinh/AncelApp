#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include "VideoTexture.h"
#include "AppDemo.h"
#include <Windows.h>
#include "resource.h"
#include "Motion.h"
#include "Skeleton.h"
#include "CameraPanel.h"
#include "IKSolver.h"
#include "MousePicker.h"
#include "UIManager.h"
#include "AnimationPanel.h"
#include "MotionGraphsPanel.h"
#include "ImagePanel.h"
#include "StatePanel.h"


using namespace AncelApp;

template<> AppDemo* Ogre::Singleton<AppDemo>::msSingleton = 0;

AppDemo::AppDemo()
	:mLog(0),
	mRoot(0),
	mRenderWnd(0),
	mTimer(0),
	mInputMgr(0),
	mKeyboard(0),
	mMouse(0),
	mShutdown(false)
{
	 
}
AppDemo::~AppDemo()
{
	delete UIManager::getSingletonPtr();

 	mRenderWnd->removeAllViewports();
   
	mSceneMgr->destroyAllCameras();
    mRoot->destroySceneManager(mSceneMgr);
	
	mLog->logMessage("Shutdown Application");

 	if(mInputMgr) OIS::InputManager::destroyInputSystem(mInputMgr);
	if(mRoot)	  delete mRoot;
}

bool AppDemo::initOgre(Ogre::String wndTitle)
{
	new Ogre::LogManager();
	mLog = Ogre::LogManager::getSingletonPtr()->createLog("AppLog.log",true,true,false);
	
#ifdef _DEBUG
	mLog->setDebugOutputEnabled(true);
	mRoot = new Ogre::Root("config/plugins_d.cfg","config/ogre.cfg");
#else 
	mLog->setDebugOutputEnabled(false);
	mRoot = new Ogre::Root("config/plugins.cfg","config/ogre.cfg");
#endif
	if(!mRoot->restoreConfig())
	{
		if(!mRoot->showConfigDialog())
		 return false;
	}
	mRenderWnd = mRoot->initialise(true,wndTitle);
  
	setupResources();
       
    mTimer = new Ogre::Timer();
    mTimer->reset();
	
	initOIS();
	
	Ogre::WindowEventUtilities::addWindowEventListener(mRenderWnd,this);
	char dir[100];
	::GetCurrentDirectoryA(100,dir);
	mWorkDirectory = dir;
	return true;
}

void AppDemo::initOIS(void)
{
	unsigned long hWnd;
	mRenderWnd->getCustomAttribute("WINDOW",static_cast<void*>(&hWnd));

//	LONG iconId = (LONG)LoadIcon( GetModuleHandle(0),MAKEINTRESOURCE(IDI_APPICON));
//	SetClassLong(HWND(hWnd), GCL_HICON, iconId);

	OIS::ParamList pl;
	pl.insert(OIS::ParamList::value_type("WINDOW",Ogre::StringConverter::toString(hWnd)));
	pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_FOREGROUND"));
	pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_NONEXCLUSIVE"));
	
	mInputMgr = OIS::InputManager::createInputSystem(pl);
	mMouse    = static_cast<OIS::Mouse*>(mInputMgr->createInputObject(OIS::OISMouse,true));
	mKeyboard = static_cast<OIS::Keyboard*>(mInputMgr->createInputObject(OIS::OISKeyboard,true));
	
	mMouse->getMouseState().width  = mRenderWnd->getWidth();
	mMouse->getMouseState().height = mRenderWnd->getHeight();
	
    mKeyboard->setEventCallback(this);
    mMouse->setEventCallback(this);
}

void AppDemo::setupResources(void)
{
	Ogre::String secName, typeName, archName;
    Ogre::ConfigFile config;

#ifdef _DEBUG
    config.load("config/resources_d.cfg");
#else 
	config.load("config/resources.cfg");
#endif 

    Ogre::ConfigFile::SectionIterator seci = config.getSectionIterator();
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void AppDemo::windowResized(Ogre::RenderWindow* rw)
{
 	mMouse->getMouseState().width  = rw->getWidth();
	mMouse->getMouseState().height = rw->getHeight();
	UIManager::getSingletonPtr()->windowResized(rw);
}

bool AppDemo::keyPressed(const OIS::KeyEvent &keyEvt)
{
	if(mKeyboard->isKeyDown(OIS::KC_SYSRQ))
    {
        mRenderWnd->writeContentsToTimestampedFile("Screenshot_", ".jpg");
        return true;
    }
	else if(mKeyboard->isKeyDown(OIS::KC_H))
	{
		AnimationPanel::getSingletonPtr()->visible();
		StatePanel::getSingletonPtr()->visible();
	}
// add in 2013-8-5 for ImagePanel 
// add position
//	else if (mKeyboard->isKeyDown(OIS::KC_0))
//	{
//		ImagePanel::getSingletonPtr()->
//	}
	MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(keyEvt.key), keyEvt.text);
 	return true; 
}

bool AppDemo::keyReleased(const OIS::KeyEvent &keyEvt)
{
	MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(keyEvt.key));
	return true;
}
void AppDemo::initDemo()
{
 	//mSceneMgr
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC,"SceneMgr_Workbench");
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.7f,0.7f,0.7f));
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
	mSceneMgr->setFog(Ogre::FOG_EXP2,Ogre::ColourValue(0.8431f,0.941f,1.0f),0.0002f);
	//mSceneMgr->setFog(Ogre::FOG_EXP2,Ogre::ColourValue(0.0f,0.0f,0.0f),0.005f);
	//Camera
	mCamera = mSceneMgr->createCamera("DemoCamera");
	mCamera->setPosition(0.0f,30.0f,111.96152422706631880582339024518f);
	mCamera->lookAt(0.0,0.0,0.0);
 	mCamera->setNearClipDistance(1);
	
	mCameraAux = mSceneMgr->createCamera("DemoCameraAux");
	mCameraAux->setPosition(0.0f,15.0f,55.596152422706631880582339024518f);
	mCameraAux->lookAt(0.0,0.0,0.0);
 	mCameraAux->setNearClipDistance(1);

	//ViewPort
	mRenderWnd->removeAllViewports();
	Ogre::Viewport *vp = mRenderWnd->addViewport(mCamera);
	//Ogre::Viewport *vpAux = mRenderWnd->addViewport(mCameraAux,1,0,0.6,0.3,0.4);
	
	//vpAux->setOverlaysEnabled(false);
	//mCameraAux->setAspectRatio(vpAux->getActualWidth()/Ogre::Real(vpAux->getActualHeight()));
 	//vpAux->setBackgroundColour(Ogre::ColourValue(0.8431f,0.941f,1.0f));

	mCamera->setAspectRatio(vp->getActualWidth()/Ogre::Real(vp->getActualHeight()));
  	vp->setBackgroundColour(Ogre::ColourValue(0.8431f,0.941f,1.0f));
	//vp->setBackgroundColour(Ogre::ColourValue(0.70f,0.7f,0.7f));
	new UIManager();
	UIManager::getSingletonPtr()->bootUI(mRenderWnd,mSceneMgr);
	new MousePicker(mCamera,mSceneMgr);

	new CameraPanel(mCamera);
//	new MainControlPanel();
}
void AppDemo::createScene(void)
{
 	//Floor


	Ogre::Plane plane(Ogre::Vector3::UNIT_Y,Ogre::Vector3(0.0f,0.0f,0.0f));
	Ogre::MeshManager::getSingleton().createPlane("floor",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,plane,2000,2000,40,40,true,1,40,40,Ogre::Vector3::UNIT_Z);
 	Ogre::Entity *ent = mSceneMgr->createEntity("floorEnt","floor");
  	mSceneMgr->getRootSceneNode()->attachObject(ent);
	ent->setCastShadows(false);
	ent->setMaterialName("AncelDemo/Floor");
	ent->setQueryFlags(EQM_NO_MASK);


/*

	//for test  add in 2013-4-18  cal the meshs' size
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y,Ogre::Vector3(0.0f,0.0f,0.0f));
	Ogre::MeshManager::getSingleton().createPlane("floor",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,plane,800,800,80,80,true,1,80,80,Ogre::Vector3::UNIT_Z);
	Ogre::Entity *ent = mSceneMgr->createEntity("floorEnt","floor");
	mSceneMgr->getRootSceneNode()->attachObject(ent);
	ent->setCastShadows(false);
	ent->setMaterialName("AncelDemo/Floor");
	ent->setQueryFlags(EQM_NO_MASK);
*/


  
	// light 
	Ogre::Light * l = mSceneMgr->createLight("mainLight");
	l->setType(Ogre::Light::LT_DIRECTIONAL);
	l->setDirection(Ogre::Vector3(0.2f,-1.0f,-0.5f));
	l->setPosition(Ogre::Vector3(0,20,0));
  	l->setDiffuseColour(Ogre::ColourValue(1.0,1.0,1.0));
	l->setSpecularColour(Ogre::ColourValue(1.0,1.0,1.0)); 
 	 

	VideoTexture::getSingleton().createVideoScene();
}

void AppDemo::run()
{
	if(!initOgre("AncelApp")) return;

	mLog->logMessage("Demo Initialised!");

	initDemo();
	createScene();

	int timeSinceLastFrame = 1;
	int startTime = 0;

	while(!mShutdown)
	{
		if(mRenderWnd->isClosed())
			mShutdown = true;

		Ogre::WindowEventUtilities::messagePump();
		
		if(mRenderWnd->isActive())
		{
	 		startTime = mTimer->getMillisecondsCPU();

			mKeyboard->capture();
			mMouse->capture();
 			 
			mRoot->renderOneFrame();
  			timeSinceLastFrame = mTimer->getMillisecondsCPU() - startTime;
		}
		else
		{
	         Sleep(50);
  		}
	}
  	mLog->logMessage("Main Loop Quit");
}

bool AppDemo::mouseMoved(const OIS::MouseEvent &evt)
{
	MyGUI::PointerManager::getInstance().setVisible(false);
    bool ret = MyGUI::InputManager::getInstance().injectMouseMove(evt.state.X.abs,evt.state.Y.abs,evt.state.Z.abs);
	
    if(ret) return true;

	MousePicker::getSingleton().mouseMoved(evt);
	CameraPanel::getSingleton().mouseMoved(evt);
	/*::ShowCursor(false);
	mMouse->getMouseState().width  = mRenderWnd->getWidth();
	mMouse->getMouseState().height = mRenderWnd->getHeight();
	if( evt.state.X.abs < mRenderWnd->getViewport(0)->getActualLeft()+1
		||(evt.state.Y.abs < mRenderWnd->getViewport(0)->getActualTop()+1)
		||(evt.state.X.abs > mRenderWnd->getViewport(0)->getActualLeft()+mRenderWnd->getViewport(0)->getActualWidth())
		||(evt.state.Y.abs > mRenderWnd->getViewport(0)->getActualTop()+mRenderWnd->getViewport(0)->getActualHeight()))
	{
		if(!mSysMouseShowFlag)
		{
			ShowCursor(1);
			mTrayMgr->hideCursor();
				mSysMouseShowFlag = true;
		}
	}
	else
	{
		if(mSysMouseShowFlag)
		{
			ShowCursor(0);
			mTrayMgr->showCursor();
			mSysMouseShowFlag = false;
		}
	}*/
	return true;
}
bool AppDemo::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
 	bool flag = MyGUI::InputManager::getInstance().injectMousePress(evt.state.X.abs,evt.state.Y.abs,MyGUI::MouseButton::Enum(id));
	if(!flag)
	{
		MousePicker::getSingleton().mousePressed(evt,id);
		MotionGraphsPanel::getSingleton().mousePressed(evt,id);		
		ImagePanel::getSingleton().mousePressed(evt,id);
	}
		
 	return true;
}
bool AppDemo::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
 	bool flag = MyGUI::InputManager::getInstance().injectMouseRelease(evt.state.X.abs,evt.state.Y.abs,MyGUI::MouseButton::Enum(id));
	if(!flag)
	{
		MousePicker::getSingleton().mouseReleased(evt,id);
	}
 		
 	return true;
}
