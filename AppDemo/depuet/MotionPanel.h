#ifndef __MotionPanel_h_
#define __MotionPanel_h_

#include <MyGUI.h>
#include <OgreSingleton.h>
#include <vector>
#include "EditObject.h"
#include "MotionManager.h"
#include <PanelView/BasePanelViewItem.h>
namespace AncelApp
{
	class MotionPanel :public Ogre::Singleton<MotionPanel>, public wraps::BasePanelViewItem
	{
	public:
		MotionPanel();
		~MotionPanel();
	 
		virtual void initialise();
		virtual void shutdown();
		void upateMotionList();
	protected:
		void notifyMouseButtonClick(MyGUI::Widget* sender);
 		void loadMotion(const MyGUI::UString& commandName, bool& result);
		void saveMotion(const MyGUI::UString& commandName, bool& result);
		void removeMotion(const MyGUI::UString& commandName, bool& result);
		void playMotion(MyGUI::Widget* sender);
		void rotateMotion(MyGUI::Widget* sender);
	private:
		MotionPanel(const MotionPanel&);
		MotionPanel operator= (const MotionPanel&);
	private:
 		MyGUI::ComboBox*		     mCBAnim;
 		std::vector<EditObject*> mAnimateStateSet;
	};
}
#endif