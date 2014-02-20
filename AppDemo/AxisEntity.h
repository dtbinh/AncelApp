#ifndef __AxisEntity_h
#define __AxisEntity_h

#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreSingleton.h>
#include <OgreSceneNode.h>

#include "MousePicker.h"


namespace AncelApp
{
	class AxisEntity: public Ogre::Singleton<AxisEntity> 
	{
	public:
		AxisEntity(Ogre::SceneManager* sceneMgr);
		
		void bindObject(Ogre::Entity * Obj);
		
		void setPosition(const Ogre::Vector3& pos);
		void setVisible(bool visibility);
		virtual bool  notifyReleased();
		virtual bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		virtual bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset);
 	private:
 		Ogre::Entity* mXAxis;
		Ogre::Entity* mYAxis;
		Ogre::Entity* mZAxis;

		Ogre::ManualObject* mAxisLine;
		Ogre::SceneNode*    mAxisNode;
		Ogre::SceneNode*    mAxisLineNode;
		
		PickableObject*     mBindObject;

		Ogre::Real			mDistance; 
		int				    mPickAxis;
		Ogre::Vector3       mMouseOffset;
	};
};
#endif