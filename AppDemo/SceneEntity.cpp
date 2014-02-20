#include "SceneEntity.h"
#include <string>
#include "AppDemo.h"
#include <MyGUI.h>
#include "AxisEntity.h"
#include "SceneObjectPanel.h"

using namespace AncelApp;

SceneEntity::SceneEntity(const std::string &meshName)
	:mIsXFixed(true),
	mIsYFixed(true),
	mIsZFixed(true),
	mScale(Ogre::Vector3(1.0f,1.0f,1.0f)),
	mRotateAngel(0)
{
	mPrefix = MyGUI::utility::toString(this, "_");
	mSceneNode = AppDemo::getSingleton().mSceneMgr->getRootSceneNode()->createChildSceneNode(mPrefix + "sceneNode");
	mSceneEntity = 	AppDemo::getSingleton().mSceneMgr->createEntity(mPrefix + meshName, meshName);
	mSceneNode->attachObject(mSceneEntity);
 	Ogre::Any any = Ogre::Any(UserAnyPair(Ogre::Any(static_cast<PickableObject*>(this)), Ogre::Any(), Ogre::Any()));
	mSceneEntity->setUserAny(any);

	mMeshName = meshName;
}

SceneEntity::~SceneEntity()
{
	mSceneNode->detachAllObjects();
	AppDemo::getSingletonPtr()->mSceneMgr->destroySceneNode(mSceneNode);
}

std::string SceneEntity::getEntityName() const
{
	return mPrefix + mMeshName;
}
bool SceneEntity::notifyReleased() 
{
	SceneObjectPanel::getSingletonPtr()->setVisible(false);
	SceneObjectPanel::getSingletonPtr()->bindObject(nullptr);
	mSceneNode->showBoundingBox(false);
	AxisEntity::getSingletonPtr()->setVisible(false);
	return true;
}

bool SceneEntity::notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
{
	if(evt.state.buttonDown(OIS::MB_Left))
	{
		Ogre::Vector3 position = ray.getPoint(mCollsionDepth) + mPickedNodeOffset;
		Ogre::Vector3 pos = position - mSceneNode->getPosition();

		float dis = pos.length();
		if(dis < 500)
		{
			if(mIsXFixed) position.x = mSceneNode->getPosition().x;
			if(mIsYFixed) position.y = mSceneNode->getPosition().y;
			if(mIsZFixed) position.z = mSceneNode->getPosition().z;
 
			mSceneNode->setPosition(position);
			AxisEntity::getSingletonPtr()->setPosition(position);
			AxisEntity::getSingletonPtr()->setVisible(true);
		}
	}
	return true;
}
bool SceneEntity::notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset) 
{
	//AxisEntity::getSingletonPtr()->setVisible(true);
	//AxisEntity::getSingletonPtr()->setPosition(mSceneNode->getPosition());

	SceneObjectPanel::getSingletonPtr()->bindData(mScale, mRotateAngel,mIsXFixed,mIsYFixed,mIsZFixed);
	SceneObjectPanel::getSingletonPtr()->bindObject(static_cast<PickableObject*>(this));
	SceneObjectPanel::getSingletonPtr()->setVisible(true);

	mCollsionDepth = entry.distance;
	mPickedNodeOffset = offset;
	AxisEntity::getSingletonPtr()->setVisible(true);
	AxisEntity::getSingletonPtr()->setPosition(mSceneNode->getPosition());
	mSceneNode->showBoundingBox(true);
	return true;
}
bool SceneEntity::updatePosition(const Ogre::Vector3 &pos)
{
	return true;
}
bool  SceneEntity::applyOperation(int operation)
{
	if(operation == 0)
	{
		mSceneNode->setScale(mScale.x,mScale.y,mScale.z);
	}
	if(operation == 1)
	{
		Ogre::Quaternion ori;
		ori.FromAngleAxis(Ogre::Radian(Ogre::Math::PI/180*mRotateAngel),Ogre::Vector3(0,1,0));
		mSceneNode->setOrientation(ori);
	}
	return true;
}

void SceneEntity::setPosition(const Ogre::Vector3& pos)
{
	mSceneNode->setPosition(pos);
}

void SceneEntity::setRotation(float angel)
{
	mRotateAngel = angel;
	Ogre::Quaternion ori;
	ori.FromAngleAxis(Ogre::Radian(Ogre::Math::PI/180*mRotateAngel),Ogre::Vector3(0,1,0));
	mSceneNode->setOrientation(ori);
}

void  SceneEntity::setScale(const Ogre::Vector3& scale)
{
	mScale = scale;
	mSceneNode->setScale(mScale);
}

Ogre::Vector3  SceneEntity::getPosition() const
{
	return mSceneNode->getPosition();
}
float  SceneEntity::getRotation() const
{
	return mRotateAngel;
}

Ogre::Vector3  SceneEntity::getScale() const
{
	return mScale;
}

