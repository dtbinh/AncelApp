#include "SceneObjectPanel.h"
#include <OgreStringConverter.h>
#include "AppDemo.h"
#include "SceneEntityManager.h"
using namespace AncelApp;

template<> SceneObjectPanel* Ogre::Singleton<SceneObjectPanel>::msSingleton = nullptr;

SceneObjectPanel::SceneObjectPanel()
	:wraps::BaseLayout("SceneEditorPanel.layout"),
	mScale(nullptr),
 	mRotateAngleY(nullptr),
	mPickableObj(0)
{
 	assignWidget(mBtnApplyRotate, "BT_APPLY_ROTATE");
	assignWidget(mBtnApplyScale, "BT_APPLY_SCALE");

	assignWidget(mEditBoxScaleX, "EB_SCALE_X");
	assignWidget(mEditBoxScaleY, "EB_SCALE_Y");
	assignWidget(mEditBoxScaleZ, "EB_SCALE_Z");

	assignWidget(mBtnFixXAxis,"RB_FIX_XAXIS");
	assignWidget(mBtnFixYAxis,"RB_FIX_YAXIS");
	assignWidget(mBtnFixZAxis,"RB_FIX_ZAXIS");

	assignWidget(mEditBoxRotateY, "EB_ROTATE_Y");
	
	mBtnFixXAxis->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyStateChanged);
	mBtnFixYAxis->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyStateChanged);
	mBtnFixZAxis->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyStateChanged);
	
	mBtnApplyRotate->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyApplyOperation);
	mBtnApplyScale->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyApplyOperation);
	
	mEditBoxScaleX->eventEditTextChange += MyGUI::newDelegate(this, &SceneObjectPanel::notifyTextChanged);
	mEditBoxScaleY->eventEditTextChange += MyGUI::newDelegate(this, &SceneObjectPanel::notifyTextChanged);
	mEditBoxScaleZ->eventEditTextChange +=  MyGUI::newDelegate(this, &SceneObjectPanel::notifyTextChanged);
	mEditBoxRotateY->eventEditTextChange += MyGUI::newDelegate(this, &SceneObjectPanel::notifyTextChanged);

	MyGUI::Button* btn;
	assignWidget(btn,"BT_SCALE_X_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyScaleChange);
	 
	assignWidget(btn,"BT_SCALE_X_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyScaleChange);
 
	assignWidget(btn,"BT_SCALE_Y_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyScaleChange);
 
	assignWidget(btn,"BT_SCALE_Y_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyScaleChange);
 
	assignWidget(btn,"BT_SCALE_Z_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyScaleChange);
 
	assignWidget(btn,"BT_SCALE_Z_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyScaleChange);
 
	assignWidget(btn,"BT_ROTATE_Y_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyRotationChange);
 
	assignWidget(btn,"BT_ROTATE_Y_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::notifyRotationChange);

	assignWidget(btn,"BT_REMOVE");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &SceneObjectPanel::removeEntity);

	this->mMainWidget->setPosition(0, AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
	setVisible(false);
}

SceneObjectPanel::~SceneObjectPanel()
{

}
bool SceneObjectPanel::setVisible(bool visibility)
{
	mMainWidget->setVisible(visibility);
	return true;
}

void SceneObjectPanel::removeEntity(MyGUI::Widget* _sender)
{
	if(mPickableObj)
	{
		SceneEntity* se = static_cast<SceneEntity*>(mPickableObj);
		MousePicker::getSingletonPtr()->resetPicker();
		SceneEntityManager::getSingletonPtr()->removeEntity(se->getEntityName());
		setVisible(false);
	}
}
void SceneObjectPanel::notifyRotationChange(MyGUI::Widget* _sender)
{
	if(mPickableObj == nullptr) return;

	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName == "BT_ROTATE_Y_LEFT")
	{
 		*mRotateAngleY -= 1;
		std::string str = Ogre::StringConverter::toString(*mRotateAngleY);
		mEditBoxRotateY->setCaption(str);
 	}
 	else if(btnName == "BT_ROTATE_Y_RIGHT")
	{
 		*mRotateAngleY += 1;
		std::string str = Ogre::StringConverter::toString(*mRotateAngleY);
		mEditBoxRotateY->setCaption(str);
 	}
	
 	mPickableObj->applyOperation(1);

}
void SceneObjectPanel::notifyScaleChange(MyGUI::Widget* _sender)
{
	if(mPickableObj == nullptr) return; 

	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName  ==  "BT_SCALE_X_RIGHT")
	{
		mScale->x += 1;
		std::string str = Ogre::StringConverter::toString(mScale->x);
		mEditBoxScaleX->setCaption(str);
 	}
	else if(btnName == "BT_SCALE_X_LEFT")
	{
 		mScale->x -= 1;
		std::string str = Ogre::StringConverter::toString(mScale->x);
		mEditBoxScaleX->setCaption(str);
 	}
 	else if(btnName == "BT_SCALE_Y_LEFT")
	{
 		mScale->y -= 1;
		std::string str = Ogre::StringConverter::toString(mScale->y);
		mEditBoxScaleY->setCaption(str);
 	}
 	else if(btnName == "BT_SCALE_Y_RIGHT")
	{
 		mScale->y += 1;
		std::string str = Ogre::StringConverter::toString(mScale->y);
		mEditBoxScaleY->setCaption(str);
 	}
 	else if(btnName == "BT_SCALE_Z_LEFT")
	{
 		mScale->z -= 1;
		std::string str = Ogre::StringConverter::toString(mScale->z);
		mEditBoxScaleZ->setCaption(str);
 	}
 	else if(btnName == "BT_SCALE_Z_RIGHT")
	{
 		mScale->z += 1;
		std::string str = Ogre::StringConverter::toString(mScale->z);
		mEditBoxScaleZ->setCaption(str);
 	}
	mPickableObj->applyOperation(0); 
}

void SceneObjectPanel::notifyTextChanged(MyGUI::EditBox* _sender)
{
	if(mPickableObj == nullptr) return;

	assert(mScale != nullptr);

	std::string boxName = _sender->getName();
	boxName = boxName.substr(mPrefix.length(),boxName.length() - mPrefix.length());
	
	std::string caption = _sender->getCaption();
	if(boxName == "EB_ROTATE_Y")
	{
		*mRotateAngleY = Ogre::StringConverter::parseReal(caption);
		mPickableObj->applyOperation(1);
	}
	else 
	{
		if(boxName == "EB_SCALE_X")
		{
	 		mScale->x = Ogre::StringConverter::parseReal(caption);
		}
		else if(boxName == "EB_SCALE_Y")
		{
			mScale->y  = Ogre::StringConverter::parseReal(caption);
		}
		else if(boxName == "EB_SCALE_Z")
		{
			mScale->z  = Ogre::StringConverter::parseReal(caption);
		}
	  	mPickableObj->applyOperation(0);
 	}	
}
bool SceneObjectPanel::bindObject(PickableObject* obj)
{
	mPickableObj = obj;
	return true;
}
void SceneObjectPanel::notifyApplyOperation(MyGUI::Widget* _sender)
{
	if(mPickableObj == nullptr) return;

	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName == "BT_APPLY_SCALE")
	{
		if(mPickableObj != nullptr)
			mPickableObj->applyOperation(0);
	}
	else if(btnName == "BT_APPLY_ROTATE")
	{
		if(mPickableObj != nullptr)
			mPickableObj->applyOperation(1);
	}
}

void SceneObjectPanel::bindData(Ogre::Vector3& scale, float& rotateAngel, bool &isFixX, bool &isFixY, bool &isFixZ)
{
	mRotateAngleY = &rotateAngel;
	Ogre::StringConverter::toString(*mRotateAngleY);
	mEditBoxRotateY->setCaption(Ogre::StringConverter::toString(*mRotateAngleY));

	mScale = &scale;
	mIsXFixed = &isFixX;
	mIsYFixed = &isFixY;
	mIsZFixed = &isFixZ;
	
	mBtnFixXAxis->setStateSelected(*mIsXFixed);
	mBtnFixYAxis->setStateSelected(*mIsYFixed);
	mBtnFixZAxis->setStateSelected(*mIsZFixed);

	mEditBoxScaleX->setCaption(Ogre::StringConverter::toString(mScale->x));
	mEditBoxScaleY->setCaption(Ogre::StringConverter::toString(mScale->y));
	mEditBoxScaleZ->setCaption(Ogre::StringConverter::toString(mScale->z));
}
void SceneObjectPanel::notifyStateChanged(MyGUI::Widget* _sender)
{
	if(mPickableObj == nullptr) return;


	MyGUI::Button *btn = static_cast<MyGUI::Button*>(_sender);

	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName == "RB_FIX_XAXIS")
	{
		*mIsXFixed = !(*mIsXFixed);
		btn->setStateSelected((*mIsXFixed));
	}
	else if(btnName == "RB_FIX_YAXIS")
	{
		*mIsYFixed = !(*mIsYFixed);
		btn->setStateSelected((*mIsYFixed));
	}
	else
	{
		*mIsZFixed = !(*mIsZFixed);
		btn->setStateSelected((*mIsZFixed));
 	}
}
void SceneObjectPanel::windowResized(Ogre::RenderWindow* rw)
{
	 mMainWidget->setPosition(0, AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
}