#include "ControlWnd.h"
//#include "BonePanel.h"
//#include "SkeletonPanel.h"
//#include "MotionPanel.h"

#include "PanelMotionGeneration.h"
#include "AppDemo.h"
using namespace AncelApp;

template<> ControlWindow* Ogre::Singleton<ControlWindow>::ms_Singleton = 0;

ControlWindow::ControlWindow()
	:BaseLayout("DemoUI.layout")
{
//	new BonePanel();
//	new SkeletonPanel();
//	new MotionPanel();
	new AnimationStatePanel();
	new PanelMotionGeneration();

	
	MyGUI::Button *btn;
	assignWidget(btn,"CtrlWndPin");
	assignWidget(mItem1,"NewPage");
	assignWidget(mItem2,"MotionGenerationPage");
	mMainWidget->setPosition(AppDemo::getSingleton().mRenderWnd->getWidth()-mMainWidget->getWidth(),35);
	mMainWidget->setSize(mMainWidget->getWidth(),AppDemo::getSingleton().mRenderWnd->getHeight()-32);

	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ControlWindow::OnPinBtnClick);
	mViewWnd1 = new PanelViewWindow("PanelView.layout",mItem1);
//	mViewWnd1->addItem(SkeletonPanel::getSingletonPtr());
//	mViewWnd1->addItem(MotionPanel::getSingletonPtr());
	mViewWnd1->addItem(AnimationStatePanel::getSingletonPtr());
	

	mViewWnd2 = new PanelViewWindow("PanelView.layout",mItem2);
	mViewWnd2->addItem(PanelMotionGeneration::getSingletonPtr());
}
ControlWindow::~ControlWindow()
{
	delete mViewWnd1;
	delete mViewWnd2;
//	delete SkeletonPanel::getSingletonPtr();
//	delete MotionPanel::getSingletonPtr();
	delete AnimationStatePanel::getSingletonPtr();
	delete PanelMotionGeneration::getSingletonPtr();
}
void ControlWindow::OnPinBtnClick(MyGUI::Widget* _sender)
{
	MyGUI::Button* btn =  static_cast<MyGUI::Button*>(_sender);
	if(!btn->getStateCheck())
	{
		MyGUI::ControllerManager::getInstance().removeItem(mMainWidget);
		btn->setStateCheck(true);
	}
	else
	{
		MyGUI::ControllerItem* item = MyGUI::ControllerManager::getInstance().createItem(MyGUI::ControllerEdgeHide::getClassTypeName());
		MyGUI::ControllerEdgeHide *controller =  item->castType<MyGUI::ControllerEdgeHide>();
		controller->setShadowSize(5);
		controller->setRemainPixels(5);
		MyGUI::ControllerManager::getInstance().addItem(mMainWidget,item);
		btn->setStateCheck(false);
 	}
}