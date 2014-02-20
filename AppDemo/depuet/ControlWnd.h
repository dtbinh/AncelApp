#ifndef _ControlWnd_h_
#define _ControlWnd_h_

#include "BaseLayout\BaseLayout.h"
#include "PanelViewCell.h"
#include <PanelView/BasePanelView.h>
#include <OgreSingleton.h>

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

namespace AncelApp
{
	class PanelViewWindow: public wraps::BasePanelView<PanelViewCell> 
	{
	public:
		PanelViewWindow(std::string layout,MyGUI::Widget* parent)
			:wraps::BasePanelView<PanelViewCell>(layout,parent)
		{
		}
		void setPosition(int x, int y)
		{
			mMainWidget->setPosition(x, y);
		}
		std::size_t getWidth()
		{
			return mMainWidget->getWidth();
		}
	};

	class ControlWindow: public wraps::BaseLayout,public Ogre::Singleton<ControlWindow>
	{
	public:
		ControlWindow();
		~ControlWindow();
		void OnPinBtnClick(MyGUI::Widget* _sender);
	protected:
		MyGUI::TabItem *mItem1;
		MyGUI::TabItem *mItem2;
		PanelViewWindow *mViewWnd1;
		PanelViewWindow *mViewWnd2;
	};
}


#endif