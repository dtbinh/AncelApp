#ifndef _IKSolverTest_h_
#define _IKSolverTest_h_

#include "MousePicker.h"
#include "IKSolver.h"
#include <MyGUI.h>
#include <OgreSingleton.h>
#include "Skeleton.h"
#include <OgreSceneNode.h>

namespace AncelApp
{
	class IKSolverTest: public PickableObject,public Ogre::Singleton<IKSolverTest>
	{
	public:
		IKSolverTest();
		~IKSolverTest();

		void initSingleChain(const MyGUI::UString& commandName, bool& result);
		void initSkeletonChain(const MyGUI::UString& commandName, bool& result);
		void setVisibility(const MyGUI::UString& commandName, bool& result);
		void changeControlChain(const MyGUI::UString& commandName, bool& result);

		bool  notifyReleased();
		bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset);
	private:
		bool  mVisibility;
		
		Skeleton           *mSkeleton;
		AncelIK::IKSolver	mIkSolver;

		Ogre::Real			mCollsionDepth;
		Ogre::Vector3		mPickedNodeOffset;

		Ogre::SceneNode    *mSphereNode;
		Ogre::SceneNode    *mCubeNode;

		//std::vector<SceneNode*> mDebugSphere;
		std::size_t        mControlChianNum;
	};
}

#endif