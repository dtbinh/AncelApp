/**
*-----------------------------------------------------------------------------
*Filename:  MotionGenerationUI.h
*-----------------------------------------------------------------------------
*File Description: 
*-----------------------------------------------------------------------------
*Author: Ancel         2012/03/05               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __MotionGenerationUI_h_
#define __MotionGenerationUI_h_

#include "UIComponent.h"
#include <MotionSyn.h>
#include <OgreSingleton.h>
#include "AppFileExplorer.h"

namespace AncelApp
{
	class MotionGenerationUI: public Ogre::Singleton<MotionGenerationUI>,public UIComponent
	{
	public:
		MotionGenerationUI();
		void buttonHit(OgreBites::Button* button);
 		void itemSelected(OgreBites::SelectMenu *menu);

 	private:
		ResModel::MotionSyn *mSynthesizer;
		AppFileExplorer *mFileExplorer;
 	};
}

#endif
