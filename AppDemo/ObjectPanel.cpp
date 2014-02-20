#include "ObjectPanel.h"
#include <OgreStringConverter.h>
using namespace AncelApp;

template<> ObjectEditorPanel* Ogre::Singleton<ObjectEditorPanel>::msSingleton = nullptr;

ObjectEditorPanel::ObjectEditorPanel()
	:wraps::BaseLayout("ObjectEditorPanel.layout"),
	mTraslation(nullptr),
 	mRotateAngleY(nullptr),
	mPickableObj(0)
{
	assignWidget(mBtnApplyRotate, "BT_APPLY_TRANS");
	//assignWidget(mBtnApplyRotate, "BT_APPLY_ROTATE");
	assignWidget(mBtnApplyTransition, "BT_APPLY_TRANS");

	assignWidget(mBtnApplyRotate, "BT_APPLY_ROTATE");
	assignWidget(mBtnApplyTransition, "BT_APPLY_TRANS");

	assignWidget(mEditBoxTransX, "EB_TRANS_X");
	assignWidget(mEditBoxTransY, "EB_TRANS_Y");
	assignWidget(mEditBoxTransZ, "EB_TRANS_Z");

	assignWidget(mEditBoxRotateY, "EB_ROTATE_Y");
	 
	mBtnApplyRotate->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyApplyOperation);
	mBtnApplyTransition->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyApplyOperation);
	
	mEditBoxTransX->eventEditTextChange += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTextChanged);
	mEditBoxTransY->eventEditTextChange += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTextChanged);
	mEditBoxTransZ->eventEditTextChange +=  MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTextChanged);
	mEditBoxRotateY->eventEditTextChange += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTextChanged);

	MyGUI::Button* btn;
	assignWidget(btn,"BT_TRANS_X_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTransitionChange);
	 
	assignWidget(btn,"BT_TRANS_X_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTransitionChange);
 
	assignWidget(btn,"BT_TRANS_Y_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTransitionChange);
 
	assignWidget(btn,"BT_TRANS_Y_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTransitionChange);
 
	assignWidget(btn,"BT_TRANS_Z_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTransitionChange);
 
	assignWidget(btn,"BT_TRANS_Z_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyTransitionChange);
 
	assignWidget(btn,"BT_ROTATE_Y_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyRotationChange);
 
	assignWidget(btn,"BT_ROTATE_Y_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &ObjectEditorPanel::notifyRotationChange);

	setVisible(true);
}

ObjectEditorPanel::~ObjectEditorPanel()
{

}
bool ObjectEditorPanel::setVisible(bool visibility)
{
	mMainWidget->setVisible(visibility);
	return true;
}
void ObjectEditorPanel::notifyRotationChange(MyGUI::Widget* _sender)
{
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
	
	if(mPickableObj != nullptr)
		mPickableObj->updateOperation(1);

}
void ObjectEditorPanel::notifyTransitionChange(MyGUI::Widget* _sender)
{
	assert(mTraslation != nullptr);

	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName  ==  "BT_TRANS_X_RIGHT")
	{
		mTraslation->x += 1;
		std::string str = Ogre::StringConverter::toString(mTraslation->x);
		mEditBoxTransX->setCaption(str);
 	}
	else if(btnName == "BT_TRANS_X_RIGHT")
	{
 		mTraslation->x -= 1;
		std::string str = Ogre::StringConverter::toString(mTraslation->x);
		mEditBoxTransX->setCaption(str);
 	}
 	else if(btnName == "BT_TRANS_Y_LEFT")
	{
 		mTraslation->y -= 1;
		std::string str = Ogre::StringConverter::toString(mTraslation->y);
		mEditBoxTransY->setCaption(str);
 	}
 	else if(btnName == "BT_TRANS_Y_RIGHT")
	{
 		mTraslation->y += 1;
		std::string str = Ogre::StringConverter::toString(mTraslation->y);
		mEditBoxTransY->setCaption(str);
 	}
 	else if(btnName == "BT_TRANS_Z_LEFT")
	{
 		mTraslation->z -= 1;
		std::string str = Ogre::StringConverter::toString(mTraslation->z);
		mEditBoxTransZ->setCaption(str);
 	}
 	else if(btnName == "BT_TRANS_Z_RIGHT")
	{
 		mTraslation->z += 1;
		std::string str = Ogre::StringConverter::toString(mTraslation->z);
		mEditBoxTransZ->setCaption(str);
 	}
	if(mPickableObj != nullptr)
		mPickableObj->updateOperation(0); 
}

void ObjectEditorPanel::notifyTextChanged(MyGUI::EditBox* _sender)
{
	assert(mTraslation != nullptr);

	std::string boxName = _sender->getName();
	boxName = boxName.substr(mPrefix.length(),boxName.length() - mPrefix.length());
	
	std::string caption = _sender->getCaption();
	if(boxName == "EB_ROTATE_Y")
	{
		*mRotateAngleY = Ogre::StringConverter::parseReal(caption);
		if(mPickableObj != nullptr)
			mPickableObj->updateOperation(1);
	}
	else 
	{
		if(boxName == "EB_TRANS_X")
		{
	 		mTraslation->x = Ogre::StringConverter::parseReal(caption);
		}
		else if(boxName == "EB_TRANS_Y")
		{
			mTraslation->y  = Ogre::StringConverter::parseReal(caption);
		}
		else if(boxName == "EB_TRANS_Z")
		{
			mTraslation->z  = Ogre::StringConverter::parseReal(caption);
		}
		if(mPickableObj != nullptr)
			mPickableObj->updateOperation(0);
 	}	
}
bool ObjectEditorPanel::bindObject(PickableObject* obj)
{
	mPickableObj = obj;
	return true;
}
void ObjectEditorPanel::notifyApplyOperation(MyGUI::Widget* _sender)
{
	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName == "BT_APPLY_TRANS")
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

void ObjectEditorPanel::bindData(Ogre::Vector3* shift, float *realAngel)
{
	mRotateAngleY = realAngel;
	Ogre::StringConverter::toString(*realAngel);
	mEditBoxRotateY->setCaption(Ogre::StringConverter::toString(*realAngel));

	mTraslation = shift;
 
	mEditBoxTransX->setCaption(Ogre::StringConverter::toString(shift->x));
	mEditBoxTransY->setCaption(Ogre::StringConverter::toString(shift->y));
	mEditBoxTransZ->setCaption(Ogre::StringConverter::toString(shift->z));
}
