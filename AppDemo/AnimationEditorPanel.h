#ifndef _AnimationEditorPanel_h_
#define _AnimationEditorPanel_h_
  
#include <OgreSingleton.h>
#include <BaseLayout\BaseLayout.h>
#include <OgreVector3.h>
#include "MousePicker.h"
#include "AnimationEditor.h"

namespace AncelApp
{
	class AnimationEditorPanel :public Ogre::Singleton<AnimationEditorPanel>, public wraps::BaseLayout
	{
	public:
		AnimationEditorPanel();
		~AnimationEditorPanel();
 		
		bool setVisible(bool visibility);
		bool bindAnimationEditor(AnimationEditor* editor);
		
		void notifyApplyOperation(MyGUI::Widget* _sender);
		void notifyRotationChange(MyGUI::Widget* _sender);
  		void notifyTextChanged(MyGUI::EditBox* _sender);

		void notifyStateChanged(MyGUI::Widget* _sender);
		void notifyShiftChanged(MyGUI::Widget* _sender);

		void notifyMouseButtonClick(MyGUI::Widget* _sender);
		
		void chainSelected(MyGUI::ListBox* _sender, size_t _index);
		void selectedChanged(MyGUI::ComboBox* _sender, size_t _index);
		void addChain(MyGUI::Widget* _sender);
		void removeChain(MyGUI::Widget* _sender);
		void showPath(MyGUI::Widget* _sender);


		bool updateUICaption();

		bool isFixXAxis(){return (mEditor) ? mEditor->isFixXAxis(): false;};
		bool isFixYAxis(){return (mEditor) ? mEditor->isFixYAxis(): false;};
		bool isFixZAxis(){return (mEditor) ? mEditor->isFixZAxis(): false;};

		void windowResized(Ogre::RenderWindow* rw);
	private:
 		AnimationEditor* mEditor;

 		MyGUI::Button*   mBtnApplyScale;
	    MyGUI::Button*   mBtnApplyRotate;

		MyGUI::Button*   mBtnFixXAxis;
		MyGUI::Button*   mBtnFixYAxis;
		MyGUI::Button*   mBtnFixZAxis;
	
		MyGUI::EditBox*  mEditBoxShiftX;
		MyGUI::EditBox*  mEditBoxShiftY;
		MyGUI::EditBox*  mEditBoxShiftZ;

		MyGUI::EditBox*  mEditBoxRotateY;
 
		MyGUI::ListBox*  mIKChainListBox;
		
		MyGUI::ComboBoxPtr  mIKChainRoot;
		MyGUI::ComboBoxPtr  mIKChainLeaf;

		MyGUI::Button* mBtnAddChain;
		MyGUI::Button* mBtnRemoveChain;
		MyGUI::Button* mBtnShowPath;

		Ogre::Vector3* mScale;
		
		bool*     mIsXFixed;
		bool*     mIsYFixed;
		bool*     mIsZFixed;
		
		float*    mRotateAngleY;
	};
}

#endif