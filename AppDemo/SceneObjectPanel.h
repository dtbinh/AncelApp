#ifndef _SceneObjectPanel_h_
#define _SceneObjectPanel_h_

#include <OgreSingleton.h>
#include <BaseLayout\BaseLayout.h>
#include <OgreVector3.h>
#include "MousePicker.h"

namespace AncelApp
{
	
	class SceneObjectPanel:public Ogre::Singleton<SceneObjectPanel>, public wraps::BaseLayout
	{
	public:
		SceneObjectPanel();
		~SceneObjectPanel();
 		
		bool setVisible(bool visibility);
		bool bindObject(PickableObject* obj);
		void bindData(Ogre::Vector3& scale, float& rotateAngel, bool &isFixX, bool &isFixY, bool &isFixZ);

		void notifyApplyOperation(MyGUI::Widget* _sender);
		void notifyRotationChange(MyGUI::Widget* _sender);
		void notifyScaleChange(MyGUI::Widget* _sender);
 		void notifyTextChanged(MyGUI::EditBox* _sender);

		void notifyStateChanged(MyGUI::Widget* _sender);

		void removeEntity(MyGUI::Widget* _sender);

		void windowResized(Ogre::RenderWindow* rw);
	private:

		PickableObject* mPickableObj;

		MyGUI::Button* mBtnApplyScale;
	    MyGUI::Button* mBtnApplyRotate;

		MyGUI::Button* mBtnFixXAxis;
		MyGUI::Button* mBtnFixYAxis;
		MyGUI::Button* mBtnFixZAxis;
	
		MyGUI::EditBox* mEditBoxScaleX;
		MyGUI::EditBox* mEditBoxScaleY;
		MyGUI::EditBox* mEditBoxScaleZ;

		MyGUI::EditBox* mEditBoxRotateY;
 
		Ogre::Vector3* mScale;
		
		bool*     mIsXFixed;
		bool*     mIsYFixed;
		bool*     mIsZFixed;
		
		float*    mRotateAngleY;
	};
};

#endif