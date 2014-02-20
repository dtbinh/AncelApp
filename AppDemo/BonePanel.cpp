#include "BonePanel.h"

using namespace AncelApp;

template<> BonePanel* Ogre::Singleton<BonePanel>::msSingleton = 0;

BonePanel::BonePanel()
	:BasePanelViewItem("PanelStatic.layout")
{
}
void BonePanel::initialise()
{
	mPanelCell->setCaption("Bone Options");
}

void BonePanel::shutdown()
{
}