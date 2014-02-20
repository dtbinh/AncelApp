#ifndef __SkeletonPanel_h_
#define __SkeletonPanel_h_

#include <OgreSingleton.h>
#include <vector>
#include "SkeletonManager.h"
#include <PanelView/BasePanelViewItem.h>

namespace AncelApp
{
	class SkeletonPanel: public Ogre::Singleton<SkeletonPanel>,public wraps::BasePanelViewItem 
	{
	public:
		SkeletonPanel();
		~SkeletonPanel();
		void windowResized(int width, int height);
		const _Skeleton* getActiveSkeleton() const;

		virtual void initialise();
		virtual void shutdown();
	private:
		bool loadAvaliableTemplates(const std::string& fileName);
		void notifyMouseButtonClick(MyGUI::Widget* _sender);
		void createActor(const MyGUI::UString& commandName, bool& result);
		void removeActor(const MyGUI::UString& commandName, bool& result);
	private:
		std::size_t			     mSkeletonCounter;
 		std::string				 mTemplateDir;
		MyGUI::ComboBox*		 mCBSkel;					//skeletons combobox 
		std::vector<std::string> mAvaliableTemplates;
	};
}
#endif