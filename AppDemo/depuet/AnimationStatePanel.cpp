#include "AnimationStatePanel.h"
#include "AppDemo.h"
#include "CommandManager.h"

using namespace AncelApp;

template<> AnimationStatePanel* Ogre::Singleton<AnimationStatePanel>::ms_Singleton = 0;

AnimationStatePanel::AnimationStatePanel()
	:mCBState(nullptr),
	BasePanelViewItem("PanelAnimState.layout")
{
	AppDemo::getSingleton().mRoot->addFrameListener(this);
}

bool AnimationStatePanel::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	for(std::size_t i = 0; i < mAnimateStateSet.size(); i++)
		mAnimateStateSet[i]->update(evt.timeSinceLastFrame);
	return true;
}

void AnimationStatePanel::initialise()
{
	mPanelCell->setCaption("Animtion");
	assignWidget(mCBState,"CBAnimState",false);
	mCBState->eventComboAccept += MyGUI::newDelegate(this, &AnimationStatePanel::animStateChanged);
	
	MyGUI::Button *btn = nullptr;

	assignWidget(btn,"CBLoopEnable",false);
 	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationStatePanel::loopStateChange);

	assignWidget(btn,"BTPlay",false);
  	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationStatePanel::activeStateChange);

	assignWidget(btn,"BTShowPath",false);
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationStatePanel::showMotionPath);

	MyGUI::ScrollBar* sbar = nullptr;
	assignWidget(sbar,"sbarSpeed",false);
	sbar->setScrollRange(100);
	sbar->setScrollPosition(20);
	sbar->eventScrollChangePosition += MyGUI::newDelegate(this,&AnimationStatePanel::changeSpeed);

	CommandManager::getInstance().registerCommand("Command_ModeEdit", MyGUI::newDelegate(this, &AnimationStatePanel::modelChange));
	CommandManager::getInstance().registerCommand("Command_ModeObject", MyGUI::newDelegate(this, &AnimationStatePanel::modelChange));
}
void AnimationStatePanel::shutdown()
{

}

void AnimationStatePanel::createAnimState(const Motion* anim,const _Skeleton *skel)
{
	Animation * animState = new Animation(skel,anim);	
	mAnimateStateSet.push_back(animState);
	
	mCBState->addItem(animState->getName());
	mCBState->setIndexSelected(mCBState->getItemCount() - 1);
	animStateChanged(NULL,0);
}

int AnimationStatePanel::getEditorModel() const
{
	return mEidtorModel;
}

void AnimationStatePanel::modelChange(const MyGUI::UString& commandName, bool& result)
{
	 if(commandName == "Command_ModeObject")
	 	 mEidtorModel = 1;
 	 else
 		 mEidtorModel = 0;
}

void AnimationStatePanel::loopStateChange(MyGUI::Widget* sender)
{
 	if(mCBState->getIndexSelected() != MyGUI::ITEM_NONE)
	{
 		std::string animStateName = mCBState->getItemNameAt(mCBState->getIndexSelected());

		MyGUI::Button* checkBox = static_cast<MyGUI::Button*>(mMainWidget->findWidget(mPrefix + "CBLoopEnable"));
 		checkBox->setStateSelected(!checkBox->getStateSelected()); 

		for (std::size_t i = 0; i < mAnimateStateSet.size(); i++)
		{
			if (animStateName == mAnimateStateSet[i]->getName())
			{
				mAnimateStateSet[i]->setLoop(checkBox->getStateSelected());
				break;
			}
		}
	}
}

void AnimationStatePanel::activeStateChange(MyGUI::Widget* sender)
{
 	if(mCBState->getIndexSelected() != MyGUI::ITEM_NONE)
	{
 		std::string animStateName = mCBState->getItemNameAt(mCBState->getIndexSelected());

		MyGUI::Button* btn = static_cast<MyGUI::Button*>(mMainWidget->findWidget(mPrefix + "BTPlay"));
 	 
		for (std::size_t i = 0; i < mAnimateStateSet.size(); i++)
		{
			if (animStateName == mAnimateStateSet[i]->getName())
			{
				mAnimateStateSet[i]->setEnabled(!mAnimateStateSet[i]->getEnabled());
				if(mAnimateStateSet[i]->getEnabled())
					btn->setCaption("stop");
				else
					btn->setCaption("play");
				break;
			}
		}
	}
}

void AnimationStatePanel::animStateChanged(MyGUI::ComboBox* _sender, size_t _index)
{
	std::string animStateName = mCBState->getItemNameAt(mCBState->getIndexSelected());

	for (std::size_t i = 0; i < mAnimateStateSet.size(); i++)
	{
		if (animStateName == mAnimateStateSet[i]->getName())
		{
			MyGUI::Button* checkBox = static_cast<MyGUI::Button*>(mMainWidget->findWidget(mPrefix + "CBLoopEnable"));
			checkBox->setStateSelected(mAnimateStateSet[i]->getLoop());
			
			MyGUI::Button* btn = static_cast<MyGUI::Button*>(mMainWidget->findWidget(mPrefix + "BTPlay"));
 			if(mAnimateStateSet[i]->getEnabled())
				btn->setCaption("stop");
			else
				btn->setCaption("play");
			break;
		}
	}
}

void AnimationStatePanel::showMotionPath(MyGUI::Widget* sender)
{
	if(mCBState->getIndexSelected() != MyGUI::ITEM_NONE)
	{
		std::string animStateName = mCBState->getItemNameAt(mCBState->getIndexSelected());

		for (std::size_t i = 0; i < mAnimateStateSet.size(); i++)
		{
			if (animStateName == mAnimateStateSet[i]->getName())
			{
				mAnimateStateSet[i]->showMotionPath(); 
 				break;
			}
		}
	}
}
void AnimationStatePanel::changeSpeed(MyGUI::ScrollBar* sender, size_t position)
{
	if(mCBState->getIndexSelected() != MyGUI::ITEM_NONE)
	{
		std::string animStateName = mCBState->getItemNameAt(mCBState->getIndexSelected());

		for (std::size_t i = 0; i < mAnimateStateSet.size(); i++)
		{
			if (animStateName == mAnimateStateSet[i]->getName())
			{
				mAnimateStateSet[i]->setUpdateSpeed(double(position)/1000.0);
 				break;
			}
		}
	}
}