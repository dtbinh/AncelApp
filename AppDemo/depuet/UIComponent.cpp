#include "UIComponent.h"
#include "AppDemo.h"

using namespace AncelApp;

UIComponent::UIComponent(const Ogre::String &componentName)
	:mComponentName(componentName),
	 mStatus(false)

{
 	mTrayMgr = new OgreBites::SdkTrayManager(componentName,AppDemo::getSingletonPtr()->mRenderWnd,AppDemo::getSingletonPtr()->mMouse,this);
	mTrayMgr->hideAll();
}

UIComponent::~UIComponent()
{
	if(mTrayMgr)
	{
		mTrayMgr->clearAllTrays();
		mTrayMgr->destroyAllWidgets();
		mTrayMgr->setListener(0);
		delete mTrayMgr;
	}
}

Ogre::String UIComponent::getName() const
{
	return mComponentName;
}
bool UIComponent::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	return true;
}

bool UIComponent::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	return true;
}
void UIComponent::update(const Ogre::FrameEvent& evt)
{

}
bool UIComponent::mouseMoved(const OIS::MouseEvent &evt)
{
	return mTrayMgr->injectMouseMove(evt);
}

bool UIComponent::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return mTrayMgr->injectMouseDown(evt,id);
}

bool UIComponent::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return mTrayMgr->injectMouseUp(evt,id);
}

void UIComponent::changeStatus()
{
	if(mStatus)
		mTrayMgr->hideAll();
	else
		mTrayMgr->showAll();
	mStatus = !mStatus;
}

//---------------------------------------------GUIManager ----------------------------------------------------------------
template<> GUIManager* Ogre::Singleton<GUIManager>::ms_Singleton = 0;

GUIManager::GUIManager()
	:mCurrentSelItem(2),mSelectedItemLoc(0.0)
{
	mTrayMgr = new OgreBites::SdkTrayManager("AppDemoManager",AppDemo::getSingletonPtr()->mRenderWnd,AppDemo::getSingletonPtr()->mMouse,this);
	
	Ogre::FontManager::getSingleton().getByName("SdkTrays/Caption")->load();
	Ogre::FontManager::getSingleton().getByName("SdkTrays/Value")->load();
	
	std::size_t len = AppDemo::getSingletonPtr()->mRenderWnd->getWidth();
	mTrayMgr->createSeparator(OgreBites::TL_BOTTOM, "sepeater", len * 0.55);
	
	AppDemo::getSingletonPtr()->mRoot->addFrameListener(this);
	mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);

	mOldKeyListener = AppDemo::getSingletonPtr()->mKeyboard->getEventCallback();
	mOldMouseListener = AppDemo::getSingletonPtr()->mMouse->getEventCallback();
	AppDemo::getSingletonPtr()->mKeyboard->setEventCallback(this);
	AppDemo::getSingletonPtr()->mMouse->setEventCallback(this);
	AppDemo::getSingletonPtr()->mRoot->addFrameListener(this);
}
GUIManager::~GUIManager()
{
	if(mTrayMgr)
	{
		mTrayMgr->clearAllTrays();
		mTrayMgr->destroyAllWidgets();
		mTrayMgr->setListener(0);
		delete mTrayMgr;
	}
	AppDemo::getSingletonPtr()->mRoot->removeFrameListener(this);
	AppDemo::getSingletonPtr()->mKeyboard->setEventCallback(mOldKeyListener);
	AppDemo::getSingletonPtr()->mMouse->setEventCallback(mOldMouseListener);
}
 
bool GUIManager::keyPressed(const OIS::KeyEvent &keyEventRef)
{
 	if(keyEventRef.key == OIS::KC_LEFT)
	{
		if(mCurrentSelItem != 0)
		{
 	 		mCurrentSelItem --;
 		}
		else 
		{
	 		mCurrentSelItem = mIconList.size() - 1;
 		}
		mSelectedItemLoc -= 85;
   	}
	else if(keyEventRef.key == OIS::KC_RIGHT)
	{
		if(mCurrentSelItem != mIconList.size()-1)
		{
			mCurrentSelItem ++;
		}
		else 
		{
			mCurrentSelItem = 0;
 		}
		mSelectedItemLoc += 85;
	}
	else if(keyEventRef.key == OIS::KC_H)
	{
 		changeStatus();
	}
	
	if(keyEventRef.key == OIS::KC_SYSRQ)
    {
		AppDemo::getSingleton().mRenderWnd->writeContentsToTimestampedFile("Screenshot_", ".jpg");
        return true;
    }
	return true;
}
bool GUIManager::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	return true;
}

bool GUIManager::mouseMoved(const OIS::MouseEvent &evt)
{
 	for(std::size_t i = 0; i < mActivateComponentList.size(); i++)
	{
		if(mComponentList[mActivateComponentList[i]]->mouseMoved(evt))
			return true;
	}
	return false;
}
bool GUIManager::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
   	for (std::size_t i = 0; i < mIconList.size(); i++)
	{
 	 	if (mIconList[i]->isVisible() && OgreBites::Widget::isCursorOver(mIconList[i],
			Ogre::Vector2(evt.state.X.abs,evt.state.Y.abs), 0))
		{
			int t = int(i);
			
			if(t - int(mCurrentSelItem) > 2)
				 t -= 5;
			else if (t - int(mCurrentSelItem) < -2)
				 t += 5;

			int offsetIndex = Ogre::Math::Abs(t - (int)mCurrentSelItem);
			double offset = offsetIndex * 100 - 7.5 * offsetIndex * (offsetIndex + 1) ;
		
			mSelectedItemLoc += (t < int(mCurrentSelItem)) ? - offset : offset;
			mCurrentSelItem = i;
			mComponentList[i]->changeStatus();
			return true;
		}
	}
 	 
	for(std::size_t i = 0; i < mActivateComponentList.size(); i++)
	{
		if(mComponentList[mActivateComponentList[i]]->mousePressed(evt, id))
			return true;
	}
	return false;
}
bool GUIManager::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
 	for(std::size_t i = 0; i < mActivateComponentList.size(); i++)
	{
		if(mComponentList[mActivateComponentList[i]]->mouseReleased(evt, id))
			return true;
	}
	return false;
}

bool GUIManager::append(UIComponent* ui,Ogre::String materialName,bool isActived)
{
	std::vector<UIComponent*>::iterator it = std::find(mComponentList.begin(),mComponentList.end(),ui);

	if(it != mComponentList.end()) return false;
	if(isActived)
		mActivateComponentList.push_back(mComponentList.size());
 	mComponentList.push_back(ui);
	
	Ogre::OverlayManager &om = Ogre::OverlayManager::getSingleton();
	  
	Ogre::BorderPanelOverlayElement* bp = (Ogre::BorderPanelOverlayElement*)
		om.createOverlayElementFromTemplate("SdkTrays/Picture", "BorderPanel", ui->getName() + "_icon");
	
	bp->setMaterialName(materialName);
	bp->setHorizontalAlignment(Ogre::GHA_CENTER);
	bp->setVerticalAlignment(Ogre::GVA_BOTTOM);
	bp->setDimensions(50,34);
	bp->setPosition(0,-100);
	
	Ogre::BorderPanelOverlayElement* frame = 
		(Ogre::BorderPanelOverlayElement*)bp->getChildIterator().getNext();
	frame->setDimensions(50+15,34+15);

 	mTrayMgr->getTraysLayer()->add2D(bp);
	mIconList.push_back(bp); 
	return true;
}

bool GUIManager::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if(mIconList.size() > 0)
	{
		double offsetVal = evt.timeSinceLastFrame * 500;
		if(mSelectedItemLoc < 0)
		{
			mSelectedItemLoc += offsetVal;
			if(mSelectedItemLoc > 0)
				mSelectedItemLoc = 0;
		}
		else if(mSelectedItemLoc > 0)
		{
			mSelectedItemLoc -= offsetVal;
			if(mSelectedItemLoc < 0)
				mSelectedItemLoc = 0;
		}
 		 
 		double totalLen = 500;
		double totalHeight = 50;

		for(std::size_t i = 0; i < mIconList.size(); i++)
		{
			int t = int(i);
			
			if(t - int(mCurrentSelItem) > 2)
				 t -= 5;
			else if (t - int(mCurrentSelItem) < -2)
				 t += 5;
			
			int offset = Ogre::Math::Abs(t - int(mCurrentSelItem));

		/*	if(offset > 2)
			{
		 		mIconList[i]->hide();
				continue;
			}
			else {
				mIconList[i]->show();
			}*/

			double offsetPos = offset * 100 - 7.5 * offset * (offset + 1);
			
			Ogre::Real pos = mSelectedItemLoc;
			
			if(t < int(mCurrentSelItem))
				pos -= offsetPos;
			else 
				pos += offsetPos;
 			
			double height = totalHeight * (totalLen - Ogre::Math::Abs(pos)) / totalLen;
	 		
			mIconList[i]->setPosition(pos - height * 0.6, - 30 - height);
 			mIconList[i]->setDimensions(height * 1.2, height);
			
 			Ogre::BorderPanelOverlayElement* frame = 
					(Ogre::BorderPanelOverlayElement*)mIconList[i]->getChildIterator().getNext();
			
			frame->setDimensions(height * 1.2 + 15,height + 15);
			
			if(i == mCurrentSelItem)
				frame->setBorderMaterialName("SdkTrays/Frame/Over");
			else 
				frame->setBorderMaterialName("SdkTrays/Frame");
 		}
	}
 	mTrayMgr->frameRenderingQueued(evt);

	for(std::size_t i = 0; i < mIconList.size(); i++)
	{
		mComponentList[i]->update(evt);
	}
	return true;
}
 
void GUIManager::changeStatus()
{
	if(mStatus)
		mTrayMgr->hideAll();
	else
		mTrayMgr->showAll();
	mStatus = !mStatus;
}
