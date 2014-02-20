#ifndef __MotionSynthesisPanel_h_
#define __MotionSynthesisPanel_h_

#include <vector>
#include <OgreSingleton.h>
#include <PanelView/BasePanelViewItem.h>
#include <MotionSyn.h>

namespace AncelApp
{
	class MotionSynthesisPanel :public Ogre::Singleton<MotionSynthesisPanel>,  public wraps::BaseLayout
	{
	public:
		MotionSynthesisPanel();
		~MotionSynthesisPanel();
		virtual void initialise();
		virtual void shutdown();
	private:
		void loadModel(MyGUI::Widget* sender);
		void generateMotion(MyGUI::Widget* sender);
		void selectedChanged(MyGUI::Widget* sender);
 
		void removeModel(MyGUI::Widget* sender);
		void addContent(MyGUI::Widget* sender);
		void visualX(MyGUI::Widget* sender);
		
		void showPanel(const MyGUI::UString& commandName, bool& result);
		void windowButtonPressed(MyGUI::Window* _sender, const std::string& _name);

		void reconstruction(MyGUI::Widget* sender);
		void synTransition(MyGUI::Widget* sender);
		void synInterpolation(MyGUI::Widget* sender);
 	private:
		MyGUI::ComboBoxPtr	mCBFactorA;
		MyGUI::ComboBoxPtr	mCBFactorB;
		MyGUI::ComboBoxPtr	mCBModel;
		MyGUI::ComboBoxPtr	mCBMotion;

		MyGUI::ComboBoxPtr  mCBActor;
		MyGUI::ComboBoxPtr  mCBContent;

		MyGUI::ButtonPtr	mBtReconstruction;
		MyGUI::ButtonPtr	mBtSynthesisMotion;
		MyGUI::ButtonPtr	mBtSynthesisSmooth;
		MyGUI::ButtonPtr	mBtLoad;
		MyGUI::ButtonPtr	mBtRemove;
		MyGUI::ButtonPtr	mBtAdd;

		MyGUI::EditBox*     mEBWightFactorA;
 		MyGUI::EditBox*     mEBWightFactorB;
 		MyGUI::EditBox*     mEBMotionLength;
 		MyGUI::EditBox*     mEBInterval;

		MyGUI::ListBox*     mListsContent;
		
		std::vector<ResModel::MotionSyn*> mModelList;
	};
}

#endif