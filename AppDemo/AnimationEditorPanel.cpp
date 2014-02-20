#include "AnimationEditorPanel.h"
#include "AppDemo.h"
#include <cmath>
using namespace AncelApp;


template<> AnimationEditorPanel* Ogre::Singleton<AnimationEditorPanel>::msSingleton = nullptr;

AnimationEditorPanel::AnimationEditorPanel()
	:BaseLayout("AnimationEditorPanel.layout"),
	mEditor(nullptr)
{
	
 	mMainWidget->setPosition(0, AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
	setVisible(false);
	MyGUI::Button* btn = nullptr;

	assignWidget(btn,"RB_FIX_XAXIS");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyStateChanged);
	
	assignWidget(btn,"RB_FIX_YAXIS");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyStateChanged);
	
	assignWidget(btn,"RB_FIX_ZAXIS");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyStateChanged);
	
	assignWidget(btn,"RB_ROOT_PATH");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyStateChanged);
 
	assignWidget(btn,"RB_AUX");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyStateChanged);

	assignWidget(mEditBoxShiftX, "EB_SHIFT_X");
	assignWidget(mEditBoxShiftY, "EB_SHIFT_Y");
	assignWidget(mEditBoxShiftZ, "EB_SHIFT_Z");
	assignWidget(mEditBoxRotateY,"EB_ROTATE_Y");

	assignWidget(mIKChainListBox,  "LB_CHAIN");
	assignWidget(mIKChainRoot,     "CB_CHAIN_ROOT");
	assignWidget(mIKChainLeaf,     "CB_CHAIN_LEAF");
	assignWidget(mBtnAddChain,     "BT_ADD_CHAIN");
	assignWidget(mBtnRemoveChain,  "BT_REMOVE_CHAIN");
	assignWidget(mBtnShowPath,     "BT_SHOWPATH");
	
	mIKChainRoot->setComboModeDrop(true);
	mIKChainRoot->eventComboAccept += MyGUI::newDelegate(this, &AnimationEditorPanel::selectedChanged);
	mIKChainLeaf->setComboModeDrop(true);
	mIKChainLeaf->eventComboAccept += MyGUI::newDelegate(this, &AnimationEditorPanel::selectedChanged);

	mIKChainListBox->eventListMouseItemActivate += MyGUI::newDelegate(this, &AnimationEditorPanel::chainSelected);
	
	mBtnAddChain->eventMouseButtonClick +=  MyGUI::newDelegate(this, &AnimationEditorPanel::addChain);
	mBtnRemoveChain->eventMouseButtonClick +=  MyGUI::newDelegate(this, &AnimationEditorPanel::removeChain);
	mBtnShowPath->eventMouseButtonClick += MyGUI::newDelegate(this,&AnimationEditorPanel::showPath); 

	mEditBoxShiftX->eventEditTextChange += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyTextChanged);
	mEditBoxShiftY->eventEditTextChange += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyTextChanged);
	mEditBoxShiftZ->eventEditTextChange += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyTextChanged);
	mEditBoxRotateY->eventEditTextChange += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyTextChanged);
 
	assignWidget(btn,"BT_SHIFT_X_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);
	 
	assignWidget(btn,"BT_SHIFT_X_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);
 
	assignWidget(btn,"BT_SHIFT_Y_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);
 
	assignWidget(btn,"BT_SHIFT_Y_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);
 
	assignWidget(btn,"BT_SHIFT_Z_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);
 
	assignWidget(btn,"BT_SHIFT_Z_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);
 
	assignWidget(btn,"BT_ROTATE_Y_LEFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);

	assignWidget(btn,"BT_ROTATE_Y_RIGHT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyShiftChanged);

	assignWidget(btn,"BT_APPLY_ROTATE");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyMouseButtonClick);

	assignWidget(btn,"BT_APPLY_SHIFT");
	btn->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationEditorPanel::notifyMouseButtonClick);
	//AppDemo::getSingletonPtr()->mRoot->addFrameListener(this);
}

AnimationEditorPanel::~AnimationEditorPanel()
{

}

void AnimationEditorPanel::chainSelected(MyGUI::ListBox* _sender, size_t _index)
{
	std::size_t index = mIKChainListBox->getIndexSelected();
	mEditor->colorChain(index,"OrangePath");
	//mEditor->setPathVisibility(index);
}

bool AnimationEditorPanel::setVisible(bool visibility)
{
	mMainWidget->setVisible(visibility);
	return true;
}
void AnimationEditorPanel::selectedChanged(MyGUI::ComboBox* sender, size_t _index)
{
	std::string boxName = sender->getName();
	boxName = boxName.substr(mPrefix.length(),boxName.length() - mPrefix.length());

	if(boxName == "CB_CHAIN_LEAF")
	{
		std::vector<std::string> availableRoot = mEditor->geAvailableRoot(mIKChainLeaf->getItem(mIKChainLeaf->getIndexSelected()));
		mIKChainRoot->removeAllItems();
		for(std::size_t  i = 0; i < availableRoot.size(); i++)
			mIKChainRoot->addItem(availableRoot[i]);
		mIKChainRoot->setIndexSelected(availableRoot.size() - 1);
	}
}
void AnimationEditorPanel::addChain(MyGUI::Widget* _sender)
{
	if(mEditor)
	{
		std::string root = mIKChainRoot->getItem(mIKChainRoot->getIndexSelected());
		std::string leaf = mIKChainLeaf->getItem(mIKChainLeaf->getIndexSelected());

		if(mEditor->addChain(root,leaf))
		{
			mIKChainListBox->addItem(root + "--->" + leaf);
			mIKChainListBox->setIndexSelected(mIKChainListBox->getItemCount() - 1);
		}
 	}
}

void AnimationEditorPanel::removeChain(MyGUI::Widget* _sender)
{
	if(mEditor)
	{
		std::size_t index = mIKChainListBox->getIndexSelected();
		if(index != MyGUI::ITEM_NONE)
		{
 			mEditor->colorChain(index,"");
			mIKChainListBox->removeItemAt(index);
			mEditor->removeChain(index);
 		}
	}
}

void AnimationEditorPanel::showPath(MyGUI::Widget* _sender)
{
	if(mEditor)
	{
		std::size_t index = mIKChainListBox->getIndexSelected();
		if(index != MyGUI::ITEM_NONE)
		{
			mEditor->setPathVisibility(index);
 		}
	}
}
void AnimationEditorPanel::notifyTextChanged(MyGUI::EditBox* _sender)
{
	if(!mEditor) return;
	 
 	std::string boxName = _sender->getName();
	boxName = boxName.substr(mPrefix.length(),boxName.length() - mPrefix.length());
	
	std::string caption = _sender->getCaption();
	if(boxName == "EB_ROTATE_Y")
	{
		mEditor->mRotateAngel = Ogre::StringConverter::parseReal(caption);
 	}
	else if(boxName == "EB_SHIFT_X")
	{
		mEditor->mRootShift.x = Ogre::StringConverter::parseReal(caption);
	}
	else if(boxName == "EB_SHIFT_Y")
	{
		mEditor->mRootShift.y = Ogre::StringConverter::parseReal(caption);
	}
	else if(boxName == "EB_SHIFT_Z")
	{
		mEditor->mRootShift.z = Ogre::StringConverter::parseReal(caption);
	}
}
void AnimationEditorPanel::notifyMouseButtonClick(MyGUI::Widget* _sender)
{
	if(!mEditor) return;

	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());

	if(btnName == "BT_APPLY_ROTATE")
	{
		mEditor->applyRotation();
		updateUICaption();
	}
	else if(btnName == "BT_APPLY_SHIFT")
	{
		mEditor->applyShift();
		updateUICaption();
	}
}
void AnimationEditorPanel::notifyStateChanged(MyGUI::Widget* _sender)
{
	if(!mEditor) return;

	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());
	
	if(btnName == "RB_FIX_XAXIS")
	{
		mEditor->mFixXAxis = !mEditor->mFixXAxis;
		btn->setStateSelected(mEditor->mFixXAxis);
	}
	else if(btnName == "RB_FIX_YAXIS")
	{
		mEditor->mFixYAxis = !mEditor->mFixYAxis;
		btn->setStateSelected(mEditor->mFixYAxis);
	}
	else if(btnName == "RB_FIX_ZAXIS")
	{
		mEditor->mFixZAxis = !mEditor->mFixZAxis;
		btn->setStateSelected(mEditor->mFixZAxis);
 	}
	else if(btnName == "RB_ROOT_PATH")
	{
		mEditor->setRootPathVisibility();
		btn->setStateSelected(mEditor->getRootPathVisibility());
	}
	else if(btnName == "RB_AUX")
	{
 		mEditor->setAuxVisibility(!btn->getStateSelected());
		btn->setStateSelected(!btn->getStateSelected());
	}
}
bool AnimationEditorPanel::updateUICaption()
{
	if(!mEditor) return true;

	MyGUI::Button* btn;
	assignWidget(btn,"RB_FIX_XAXIS");
	btn->setStateSelected(mEditor->mFixXAxis);	 
	assignWidget(btn,"RB_FIX_YAXIS");
	btn->setStateSelected(mEditor->mFixYAxis);	 
	assignWidget(btn,"RB_FIX_ZAXIS");
	btn->setStateSelected(mEditor->mFixZAxis);	 

	assignWidget(btn,"RB_ROOT_PATH");
	btn->setStateSelected(mEditor->getRootPathVisibility());	 
 
	std::string str = Ogre::StringConverter::toString(mEditor->mRotateAngel);
	mEditBoxRotateY->setCaption(str);

	str = Ogre::StringConverter::toString(mEditor->mRootShift.x);
  	mEditBoxShiftX->setCaption(str);

	str = Ogre::StringConverter::toString(mEditor->mRootShift.y);
  	mEditBoxShiftY->setCaption(str);

	str = Ogre::StringConverter::toString(mEditor->mRootShift.z);
  	mEditBoxShiftZ->setCaption(str);

	std::vector<std::pair<std::string,std::string>> chainNameLs = mEditor->getChainNameList();

	mIKChainListBox->removeAllItems();
	for(std::size_t i = 0; i < chainNameLs.size(); i++)
	{
		mIKChainListBox->addItem(chainNameLs[i].first + "----->" + chainNameLs[i].second);
	}

  	return true;
}
void AnimationEditorPanel::notifyShiftChanged(MyGUI::Widget* _sender)
{
 	MyGUI::Button* btn = static_cast<MyGUI::Button*>(_sender);
 
	//TODO: change compare to AnyData injected in the widget later
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length() - mPrefix.length());


	if(btnName == "BT_SHIFT_X_LEFT")
	{
		mEditor->mRootShift.x += 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRootShift.x);
		mEditBoxShiftX->setCaption(str);
	}
	else if(btnName == "BT_SHIFT_X_RIGHT")
	{
		mEditor->mRootShift.x -= 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRootShift.x);
		mEditBoxShiftX->setCaption(str);
	}
	else if(btnName == "BT_SHIFT_Y_LEFT") 
	{
		mEditor->mRootShift.y += 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRootShift.y);
		mEditBoxShiftY->setCaption(str);
 	}
	else if(btnName == "BT_SHIFT_Y_RIGHT") 
	{
		mEditor->mRootShift.y -= 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRootShift.y);
		mEditBoxShiftY->setCaption(str);
 	}
	else if(btnName == "BT_SHIFT_Z_LEFT")	 
	{
		mEditor->mRootShift.z += 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRootShift.z);
		mEditBoxShiftZ->setCaption(str);
 	}
	else if(btnName == "BT_SHIFT_Z_RIGHT")	 
	{
		mEditor->mRootShift.z -= 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRootShift.z);
		mEditBoxShiftZ->setCaption(str);
 	}
	else if(btnName == "BT_ROTATE_Y_LEFT")
	{
		mEditor->mRotateAngel += 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRotateAngel);
		mEditBoxRotateY->setCaption(str);
	}
	else if(btnName == "BT_ROTATE_Y_RIGHT")
	{
		mEditor->mRotateAngel -= 1;
		std::string str = Ogre::StringConverter::toString(mEditor->mRotateAngel);
		mEditBoxRotateY->setCaption(str);
	}
}
bool AnimationEditorPanel::bindAnimationEditor(AnimationEditor* editor)
{
	mEditor = editor;
 	if(mEditor)
	{
  		updateUICaption();
		std::vector<std::string> boneNameLs = mEditor->getBoneNameList();
		mIKChainLeaf->removeAllItems();

		for(std::size_t  i = 0; i < boneNameLs.size(); i++)
			mIKChainLeaf->addItem(boneNameLs[i]);
		mIKChainLeaf->setIndexSelected(0);
		std::vector<std::string> availableRoot = mEditor->geAvailableRoot(mIKChainLeaf->getItem(mIKChainLeaf->getIndexSelected()));

		mIKChainRoot->removeAllItems();
		for(std::size_t  i = 0; i < availableRoot.size(); i++)
			mIKChainRoot->addItem(availableRoot[i]);
		mIKChainRoot->setIndexSelected(availableRoot.size() - 1);
	}
	return true;
}
void AnimationEditorPanel::windowResized(Ogre::RenderWindow* rw)
{
	mMainWidget->setPosition(0, AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
}