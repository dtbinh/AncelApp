#include "UIManager.h"
#include "VideoTexture.h"
#include "CommandManager.h"
#include "MainMenuControl.h"
#include "SkeletonView.h"
#include "AppDemo.h"
#include "AxisEntity.h"
#include "ObjectPanel.h"
#include "SceneObjectPanel.h"
#include "SceneEntityManager.h"
#include "StatePanel.h"
#include "AnimationPanel.h"
#include "AnimationEditorPanel.h"
#include "MotionSynthesisPanel.h"
#include "MotionGraphsPanel.h"
#include "ImagePanel.h"
#include "PathEditingEvaluator.h"
#include "IKSolverTest.h"


using namespace AncelApp;

template<> UIManager* Ogre::Singleton<UIManager>::msSingleton = 0;

UIManager::UIManager()
{

}
UIManager::~UIManager()
{
  	delete CommandManager::getInstancePtr();
	delete MainMenuControl::getInstancePtr();
 
	delete SceneEntityManager::getSingletonPtr();
    delete SceneObjectPanel::getSingletonPtr();
	delete AnimationPanel::getSingletonPtr();
	delete StatePanel::getSingletonPtr();
	delete AnimationEditorPanel::getSingletonPtr();
	delete MotionSynthesisPanel::getSingletonPtr();
	delete MotionGraphsPanel::getSingletonPtr();
	delete ImagePanel::getSingletonPtr();
	delete PathEditingEvaluator::getSingletonPtr();
	delete IKSolverTest::getSingletonPtr();
	delete VideoTexture::getSingletonPtr();

	mDemoGUI->shutdown();
	delete mDemoGUI;
	mDemoGUI = 0;   
	mPlatform->shutdown();
	delete mPlatform;
	mPlatform = 0;
}
bool UIManager::bootUI(const Ogre::RenderWindow* wnd, Ogre::SceneManager* mgr)
{
	mPlatform = new MyGUI::OgrePlatform();
	if(!mPlatform) 
		return false;
	mPlatform->initialise(const_cast<Ogre::RenderWindow*>(wnd),  mgr);
	mDemoGUI = new MyGUI::Gui();
	if(!mDemoGUI)
		return false;
	mDemoGUI->initialise("MyGUI_Core.xml");
	initialiseUI(wnd);
	return true;
}

void UIManager::initialiseUI(const Ogre::RenderWindow* wnd)
{
	new CommandManager();
 	//new ControlWindow();
	new MainMenuControl();
	//new SkeletonView();
 
	new VideoTexture();
	new SceneEntityManager();
	new SceneObjectPanel();
	new StatePanel();
	new AnimationPanel();
	new AnimationEditorPanel();
	new MotionSynthesisPanel();
	new MotionGraphsPanel();
	new ImagePanel();
	//SkeletonView::getSingletonPtr()->createScene();
 	new AxisEntity(AppDemo::getSingleton().mSceneMgr);
	new PathEditingEvaluator();
	new IKSolverTest();
	
}
void UIManager::windowResized(Ogre::RenderWindow* rw)
{
 	AnimationPanel::getSingletonPtr()->windowResized(rw);
	StatePanel::getSingletonPtr()->windowResized(rw);
	AnimationEditorPanel::getSingletonPtr()->windowResized(rw);
	SceneObjectPanel::getSingletonPtr()->windowResized(rw);
}