/*
date: 2013-8-5
des: 通过鼠标点击获取骨架的所有点
*/

#ifndef __ImagePanel_h_
#define __ImagePanel_h_


#include <vector>
#include <OgreSingleton.h>
#include <PanelView/BasePanelViewItem.h>
#include <BaseLayout/BaseLayout.h>
#include <OgreVector3.h>
#include "Motion.h"
#include "Skeleton.h"
#include "Animation.h"
#include "StatePanel.h"     //调用 updateActorTheme 
#include "AnimationEditor.h"


#define NUMBONE 22



namespace AncelApp
{
	struct Point1
	{
		double x,y;
	};

	struct Point2
	{
		double x,y,z;
	};


	class ImagePanel:public Ogre::Singleton<ImagePanel>,public wraps::BaseLayout
	{
	public:
		ImagePanel();
		~ImagePanel();

		virtual void initialise();
		virtual void shutdown();
		bool setVisible(bool visibility);
		virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	protected:
		void showPanel(const MyGUI::UString& commandName,bool& result);
		void windowButtonPressed(MyGUI::Window* _sender,const std::string& _name);
		void notifyMouseButtonClick(MyGUI::Widget* _sender);
		void windowResized(Ogre::RenderWindow* rw);
		void EnableGetPoint(const MyGUI::UString& commandName, bool& result);
	
	private:
		MyGUI::ButtonPtr mBtnGetPoint;
		bool enableGetPoint;
		Point1 Image2DPos[NUMBONE];
		Point2 Image3DPos[NUMBONE];
		int count;
	};
}

#endif