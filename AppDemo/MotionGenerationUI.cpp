#include "MotionGeneartionUI.h"
#include "_Animation.h"
#include "AppUtility.h"

using namespace AncelApp;

template<> MotionGenerationUI* Ogre::Singleton<MotionGenerationUI>::ms_Singleton = 0;

MotionGenerationUI::MotionGenerationUI()
	:UIComponent("MotionGenerationUI"),
	mSynthesizer(NULL)
{
	OgreBites::TrayLocation tLoc = OgreBites::TL_TOPRIGHT;
	mTrayMgr->createLabel(tLoc, "MGUI_TITLE", "MotionGenerationUI", 200);
	mTrayMgr->createSeparator(tLoc, "MGUI_SEPA1", 200);
 	mTrayMgr->createThickSelectMenu(tLoc, "MGUI_Model", "model", 200, 10);
	mTrayMgr->createButton(tLoc,"MGUI_LoadModel", "load Model", 200);
	mTrayMgr->createThickSelectMenu(tLoc, "MGUI_actor", "actor", 200, 10);
	mTrayMgr->createThickSelectMenu(tLoc, "MGUI_Content", "content", 200, 10);
	mTrayMgr->createButton(tLoc, "MGUI_SYN", "synthesis",200);
}
void MotionGenerationUI::buttonHit(OgreBites::Button* button)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW",static_cast<void*>(&hWnd));

	if(button->getCaption() == "load Model")
	{
	 	string filename = loadFile("MGPM", HWND(hWnd));
		if(filename != "")
		{
			assert(mSynthesizer == NULL);
			mSynthesizer = new ResModel::MotionSyn(filename);
			mSynthesizer->initSynthesis();
		}
	}
	else if(button->getCaption() == "synthesis")
	{
 		static int syn_num = 0;
		if(mSynthesizer != NULL)
		{
			std::vector<std::size_t> identity;
			identity.push_back(0);
			identity.push_back(1);
			identity.push_back(2);
			//ResUtil::Matrix mo = mSynthesizer->generate(0, identity, 200);
			//ResUtil::Matrix mo = mSynthesizer->synthesis(0,1,500);
			double state = 0;
			//ResUtil::Matrix mo = mSynthesizer->synTrainsiton(0,0,1,100,ResUtil::CVector3D<double>(0,15,0),state);

			//AnimationManager::getSingleton().createAnimtion(&mo, "syn_" + Ogre::StringConverter::toString(syn_num));
		 	
			
			
			for(std::size_t i = 0; i < mSynthesizer->getMotionSemgemtnNum(); i++)
			{
			//ResUtil::Matrix mo = mSynthesizer->synthesis(2,1,200);
			  ResUtil::Matrix mo = mSynthesizer->reconstruct(i);
				AnimationManager::getSingleton().createAnimtion(&mo, "syn_" + Ogre::StringConverter::toString(syn_num));
				syn_num++;

				//mo = mSynthesizer->syc(1,5);
				//AnimationManager::getSingleton().createAnimtion(&mo, "syn_" + Ogre::StringConverter::toString(syn_num));
				//syn_num++;

		   }
 		}
		else
		{
			::MessageBoxA(HWND(hWnd), "Please load model first !", "Message", MB_OK|MB_ICONWARNING);
		}
	}
}
void MotionGenerationUI::itemSelected(OgreBites::SelectMenu *menu)
{

}