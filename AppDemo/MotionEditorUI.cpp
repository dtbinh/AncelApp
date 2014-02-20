#include "MotionEditorUI.h"
#include <ResUtility.h>
#include "Skeleton.h"

using namespace AncelApp;

template<> MotionEditorUI* Ogre::Singleton<MotionEditorUI>::ms_Singleton = 0;
MotionEditorUI::MotionEditorUI()
	:UIComponent("MotionEditorUI")
{
	OgreBites::TrayLocation tLoc = OgreBites::TL_TOP;
	
	mTrayMgr->createLabel(tLoc,"MotionEditorUI","Motion Editor UI",200.0);
	mTrayMgr->createThickSelectMenu(tLoc,"MEUI_JOINT","Joint",200,20);
	mTrayMgr->createSeparator(tLoc,"MotionEditorUI_SEP",200);
    mTrayMgr->createButton(tLoc,"MotionCurve","drawCruve",100);
}

void MotionEditorUI::itemSelected(OgreBites::SelectMenu *menu)
{

}
 
void MotionEditorUI::buttonHit(OgreBites::Button* button)
{
	if (button->getCaption() == "drawCruve")
	{
 /*		if (selAnimation != "")
		{
			
		}
 */	}
}
 

