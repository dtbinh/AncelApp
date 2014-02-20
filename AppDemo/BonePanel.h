#ifndef __BonePanel_h
#define __BonePanel_h

#include <PanelView/BasePanelViewItem.h>
#include <OgreSingleton.h>
namespace AncelApp
{
	class BonePanel: public Ogre::Singleton<BonePanel>, public wraps::BasePanelViewItem
	{
	public:
		BonePanel();
 		virtual void initialise();
		virtual void shutdown();
	};
};

#endif