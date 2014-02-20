#ifndef _AnimationPanel_h_
#define _AnimationPanel_h_

#include <OgreSingleton.h>
#include <BaseLayout\BaseLayout.h>
#include <OgreVector3.h>
#include "Animation.h"


namespace AncelApp
{
	
	class AnimationPanel:public Ogre::Singleton<AnimationPanel>, public wraps::BaseLayout, public Ogre::FrameListener
	{
	public:
		AnimationPanel();
		~AnimationPanel();
 	
		bool frameRenderingQueued(const Ogre::FrameEvent& evt);
 
		void notifyApplyOperation(MyGUI::Widget* _sender);
		void notifyRotationChange(MyGUI::Widget* _sender);
		void notifyScaleChange(MyGUI::Widget* _sender);
 		void notifyTextChanged(MyGUI::EditBox* _sender);

		void notifyMouseButtonClick(MyGUI::Widget* _sender);
		void changeSpeed(MyGUI::ScrollBar* sender, size_t position);
		void windowResized(Ogre::RenderWindow* rw);

		void visible();
	private:
		Animation *mBindAnimation;

		MyGUI::Button* mBtnPlay;
	    MyGUI::Button* mBtnSkip;
		MyGUI::Button* mRBtnEnableLoop;
		MyGUI::Button* mRBtnEnableTracking;
		MyGUI::Button* mRBtnRecord;

		MyGUI::ScrollBar* mSBarSpeed;
 		 
		MyGUI::EditBox* mEBTotalFrame;
		MyGUI::EditBox* mEBCurrentFrame;
		MyGUI::EditBox* mEBSkipToFrame;
	};
};

#endif