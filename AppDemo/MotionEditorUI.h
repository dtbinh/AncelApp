/**
*-----------------------------------------------------------------------------
*Filename:  MotionEditorUI.h
*-----------------------------------------------------------------------------
*File Description: the class is used as a UI for motion editing
*-----------------------------------------------------------------------------
*Author: Ancel         2012/03/02               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __MotionEditorUI_h_
#define __MotionEditorUI_h_

#include "UIComponent.h"
#include "Animation.h"
#include "AppFileExplorer.h"


namespace AncelApp
{
	class MotionEditorUI: public UIComponent,public Ogre::Singleton<MotionEditorUI>
	{
	public:
		MotionEditorUI();
   		void buttonHit(OgreBites::Button* button);
		void itemSelected(OgreBites::SelectMenu *menu);
		
	
 	private:
	
		AppFileExplorer *mFileExplorer;
	};
}

#endif