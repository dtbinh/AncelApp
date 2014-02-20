#ifndef __PathEditingEvaluator_h_
#define __PathEditingEvaluator_h_

#include <MyGUI.h>
#include "MousePicker.h"
#include "Path.h"

namespace AncelApp
{
	class PathEditingEvaluator: public PickableObject,public Ogre::Singleton<PathEditingEvaluator>
	{
	public:
		PathEditingEvaluator();
		~PathEditingEvaluator();
  		bool  notifyReleased();
		bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset);
		
		void createHalfCirclePath(const MyGUI::UString& commandName, bool& result);
		void createStraightLinePath(const MyGUI::UString& commandName, bool& result);
		void createStringPath(const MyGUI::UString& commandName, bool& result);
//add 2013=8-6   for ImagePanel  by ruan.answer
		void createPointPath();

		void remove(const MyGUI::UString& commandName, bool& result);
 	private:
		Ogre::Real			mCollsionDepth;
		Ogre::Vector3		mPickedNodeOffset;
		Ogre::SceneNode*    mPickedNode;
		std::size_t			mPickedIndex;

		bool                mVisibility;
		Path* mDemoPath;
	};
};
#endif