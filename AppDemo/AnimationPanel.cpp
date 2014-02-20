#include "AnimationPanel.h"
#include "AppDemo.h"
#include "StatePanel.h"
#include "AnimationManager.h"

using namespace AncelApp;

template<> AnimationPanel* Ogre::Singleton<AnimationPanel>::msSingleton = nullptr;

AnimationPanel::AnimationPanel()
	:wraps::BaseLayout("AnimationPanel.layout"),
	mBindAnimation(nullptr)
	 
{ 
  	assignWidget(mBtnPlay, "BT_PLAY_ANIMATION");
	assignWidget(mBtnSkip, "BT_APPLY_SKIP");
	assignWidget(mSBarSpeed, "SLBAR_SPEED");
	assignWidget(mRBtnRecord, "RB_RECORD");

	assignWidget(mRBtnEnableLoop, "RB_ENABLE_LOOP");
	assignWidget(mRBtnEnableTracking, "RB_TRACK_ACTOR");

	assignWidget(mEBTotalFrame, "EB_TOTAL_FRAME");
	assignWidget(mEBCurrentFrame, "EB_CURRENT_FRAME");
	assignWidget(mEBSkipToFrame, "EB_SKIPTO_FRAME");

  
	MyGUI::ScrollBar* sbar;
	assignWidget(sbar,"SLBAR_SPEED");
	sbar->setScrollRange(80);
	sbar->setScrollPosition(30);
	sbar->eventScrollChangePosition += MyGUI::newDelegate(this,&AnimationPanel::changeSpeed);


	mRBtnRecord->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationPanel::notifyMouseButtonClick);
	mBtnPlay->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationPanel::notifyMouseButtonClick);
	mBtnSkip->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationPanel::notifyMouseButtonClick);
	mRBtnEnableLoop->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationPanel::notifyMouseButtonClick);
	mRBtnEnableTracking->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationPanel::notifyMouseButtonClick);

	mMainWidget->setPosition(AppDemo::getSingletonPtr()->mRenderWnd->getWidth()- mMainWidget->getWidth(), AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
	AppDemo::getSingleton().mRoot->addFrameListener(this);
}

void AnimationPanel::notifyMouseButtonClick(MyGUI::Widget* _sender)
{
	MyGUI::ButtonPtr btn = static_cast<MyGUI::ButtonPtr>(_sender);

	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName == "RB_RECORD")
	{
		bool state = btn->getStateSelected();
		btn->setStateSelected(!state);
		if(btn->getStateSelected())
 			btn->setTextColour(MyGUI::Colour(1.0f,0.0f,0.0f));
		else
 			btn->setTextColour(MyGUI::Colour(0.0f,0.0f,0.0f));
 		return;
	}

	Animation *anim =  StatePanel::getSingletonPtr()->getActiveAnimation();
	if(anim == nullptr) return;
 
	if(btnName == "BT_PLAY_ANIMATION")
	{
		//AnimationManager::getSingletonPtr()->playAll();
		Animation *anim =  StatePanel::getSingletonPtr()->getActiveAnimation();
 		anim->setEnabled(!anim->getEnabled());
		if(anim->getEnabled())
			btn->setCaption("=");
		else
			btn->setCaption(">");
	}
	else if(btnName == "BT_APPLY_SKIP")
	{
		Animation *anim =  StatePanel::getSingletonPtr()->getActiveAnimation();
		if(anim)
		{
			anim->skipTo(MyGUI::utility::parseInt(mEBSkipToFrame->getCaption()));
		}
 	}
	else if(btnName == "RB_ENABLE_LOOP")
	{
		Animation *anim =  StatePanel::getSingletonPtr()->getActiveAnimation();
		if(anim)
		{
			anim->setLoop(!anim->getLoop());
			btn->setStateSelected(anim->getLoop());
		}
	}
	else if(btnName == "RB_TRACK_ACTOR")
	{
  		if(AppDemo::getSingleton().mCamera->getParentNode()) 
		{
			AppDemo::getSingleton().mCamera->detachFromParent();
			btn->setStateSelected(false);
 		}
		else 
		{
			Animation *anim =  StatePanel::getSingletonPtr()->getActiveAnimation();
			if(anim)
			{
    			anim->getSkeleton()->getRootNode()->attachObject(AppDemo::getSingleton().mCamera);
 				btn->setStateSelected(true); 
			}
 		}
 	}
	
}
void AnimationPanel::visible()
{
	mMainWidget->setVisible(!mMainWidget->getVisible());
}
AnimationPanel::~AnimationPanel()
{

}

void AnimationPanel::changeSpeed(MyGUI::ScrollBar* sender, size_t position)
{
	Animation *anim =  StatePanel::getSingletonPtr()->getActiveAnimation();
	if(anim != nullptr)
	{ 
		anim->setUpdateSpeed(1.0/(position+20));
	}
}

bool AnimationPanel::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	Animation *anim =  StatePanel::getSingletonPtr()->getActiveAnimation();
	if(anim != nullptr)
	{
		mEBTotalFrame->setCaption(MyGUI::utility::toString(anim->getTotalFrame()));
		mEBCurrentFrame->setCaption(MyGUI::utility::toString(anim->getCurrentFrame()));
	}
	else
	{
		mEBTotalFrame->setCaption("0");
		mEBCurrentFrame->setCaption("0");
 	}
	if(mRBtnRecord->getStateSelected())
	{
		static int count = 0;
		std::string filename = "2_" + MyGUI::utility::toString(count++) + ".jpg";
		AppDemo::getSingletonPtr()->mRenderWnd->writeContentsToFile(filename);
	}
	return true;
}
void AnimationPanel::windowResized(Ogre::RenderWindow* rw)
{
	mMainWidget->setPosition(AppDemo::getSingletonPtr()->mRenderWnd->getWidth()- mMainWidget->getWidth(), AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
}