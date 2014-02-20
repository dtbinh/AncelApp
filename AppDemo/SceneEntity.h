#ifndef __SceneEntity_h_
#define __SceneEntity_h_

#include "MousePicker.h"
#include <OgreEntity.h>
#include <OgreSceneNode.h>

namespace AncelApp
{
	class SceneEntity: public PickableObject
	{
	public:
		SceneEntity(const std::string &meshName);
		~SceneEntity();
		std::string getEntityName() const;         
		
		virtual bool  notifyReleased();
		virtual bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		virtual bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset);
		virtual bool  updatePosition(const Ogre::Vector3 &pos);

		virtual bool  applyOperation(int operation);

		void		  setPosition(const Ogre::Vector3& pos);
		void          setRotation(float angel);
		void          setScale(const Ogre::Vector3&scale);

		Ogre::Vector3  getPosition() const;
		float          getRotation() const;
		Ogre::Vector3  getScale() const;

		std::string    getMeshName() const {return mMeshName;}
  	private:
 		Ogre::Real		mCollsionDepth;
		Ogre::Vector3	mPickedNodeOffset;

		bool            mIsXFixed;
		bool            mIsYFixed;
		bool            mIsZFixed;
		Ogre::Vector3   mScale;
		float           mRotateAngel;

		std::string     mMeshName;
		std::string     mPrefix;
		Ogre::SceneNode *mSceneNode;
		Ogre::Entity    *mSceneEntity;
		
	};
}

#endif
