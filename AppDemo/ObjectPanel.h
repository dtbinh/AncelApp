#ifndef _ObjectPanel_h_
#define _ObjectPanel_h_

#include <OgreSingleton.h>
#include <BaseLayout\BaseLayout.h>
#include <OgreVector3.h>
#include "MousePicker.h"

namespace AncelApp
{
	
	class ObjectEditorPanel:public Ogre::Singleton<ObjectEditorPanel>, public wraps::BaseLayout
	{
	public:
		ObjectEditorPanel();
		~ObjectEditorPanel();
		
		 
		
		bool setVisible(bool visibility);
		bool bindObject(PickableObject* obj);
		void bindData(Ogre::Vector3* shift, float *realAngel);

		void notifyApplyOperation(MyGUI::Widget* _sender);
		void notifyRotationChange(MyGUI::Widget* _sender);
		void notifyTransitionChange(MyGUI::Widget* _sender);
 		void notifyTextChanged(MyGUI::EditBox* _sender);
	private:
		PickableObject* mPickableObj;

		MyGUI::Button* mBtnApplyRotate;
		MyGUI::Button* mBtnApplyTransition;
	
		MyGUI::EditBox* mEditBoxTransX;
		MyGUI::EditBox* mEditBoxTransY;
		MyGUI::EditBox* mEditBoxTransZ;

		MyGUI::EditBox* mEditBoxRotateY;
		 
		Ogre::Vector3* mTraslation;
		float* mRotateAngleY;
	};
};

#endif