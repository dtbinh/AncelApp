#ifndef __AnimationStatePanel_h_
#define __AnimationStatePanel_h_

#include <OgreSingleton.h>
#include <OgreFrameListener.h>
#include <PanelView/BasePanelViewItem.h>

#include "Animation.h"
#include "MotionManager.h"

namespace AncelApp
{
	class AnimationStatePanel
		:public Ogre::FrameListener,
		 public wraps::BasePanelViewItem,
 		 public Ogre::Singleton<AnimationStatePanel>
	{
	public:
		AnimationStatePanel();
		bool frameRenderingQueued(const Ogre::FrameEvent& evt);
		virtual void initialise();
		virtual void shutdown();
		void createAnimState(const Motion* anim,const _Skeleton *skel);
		void modelChange(const MyGUI::UString& commandName, bool& result);
		int getEditorModel() const;
	protected:
		void loopStateChange(MyGUI::Widget* sender);
		void activeStateChange(MyGUI::Widget* sender);
		void animStateChanged(MyGUI::ComboBox* _sender, size_t _index);
		void showMotionPath(MyGUI::Widget* sender);
		void changeSpeed(MyGUI::ScrollBar* sender, size_t position);
 	private:
		
		int mEidtorModel;
		MyGUI::ComboBox*		     mCBState;
		std::vector<Animation*> mAnimateStateSet;
	};
}
#endif