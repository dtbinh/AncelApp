#ifndef _StatePanel_h_
#define _StatePanel_h_

#include <OgreSingleton.h>
#include <BaseLayout\BaseLayout.h>
#include <OgreVector3.h> 

#include "Motion.h"
#include "Skeleton.h"
#include "Animation.h"

namespace AncelApp
{
	class StatePanel:public Ogre::Singleton<StatePanel>, public wraps::BaseLayout
	{
		public:
			StatePanel();
			~StatePanel();
	 	
			bool setVisible(bool visibility);
			
			void addMotion(Motion *mo);
			void loadMotion(const MyGUI::UString& commandName, bool& result);
			
			void saveMotion(const MyGUI::UString& commandName, bool& result);
			void removeMotion(const MyGUI::UString& commandName, bool& result);

			void loadActor(const MyGUI::UString& commandName, bool& result);
			void removeActor(const MyGUI::UString& commandName, bool& result);

			void removeAnimation(const MyGUI::UString& commandName, bool& result);
			void createAnimation(const MyGUI::UString& commandName, bool& result);

			void updateActorTheme(const MyGUI::UString& commandName, bool& result);
			Motion*     getActiveMotion();
			Skeleton*   getActiveActor();
			Animation*  getActiveAnimation();

			void notifyMouseButtonClick(MyGUI::Widget* _sender);
		/*	void notifyApplyOperation(MyGUI::Widget* _sender);
			void notifyRotationChange(MyGUI::Widget* _sender);
			void notifyScaleChange(MyGUI::Widget* _sender);
 			void notifyTextChanged(MyGUI::EditBox* _sender);
			notifyMouseButtonClick
			void notifyMouseButtonclicked(MyGUI::Widget* _sender);*/

			void windowResized(Ogre::RenderWindow* rw);

			void visible();
		private:
			MyGUI::ComboBoxPtr mCBActor;
			MyGUI::ComboBoxPtr mCBMotion;
			MyGUI::ComboBoxPtr mCBAnimation;
  		};
}
#endif;