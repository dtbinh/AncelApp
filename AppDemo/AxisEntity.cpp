#include "AxisEntity.h"
#include "MousePicker.h"
#include "AppDemo.h"

using namespace AncelApp;

template<> AxisEntity* Ogre::Singleton<AxisEntity>::msSingleton = 0;

AxisEntity::AxisEntity(Ogre::SceneManager* sceneMgr)
	:mDistance(0)
{
	mXAxis = sceneMgr->createEntity("XAxis","XAxis.mesh");
	mYAxis = sceneMgr->createEntity("YAxis","YAxis.mesh");
	mZAxis = sceneMgr->createEntity("ZAxis","ZAxis.mesh");

	mXAxis->setMaterialName("wxOgreMeshViewer/AxisX_Red");
	mYAxis->setMaterialName("wxOgreMeshViewer/AxisY_Green");
	mZAxis->setMaterialName("wxOgreMeshViewer/AxisZ_Blue");
	mAxisNode = sceneMgr->getRootSceneNode()->createChildSceneNode("AxisNode");

	mXAxis->setQueryFlags(EQM_NO_MASK);
	mYAxis->setQueryFlags(EQM_NO_MASK);
	mZAxis->setQueryFlags(EQM_NO_MASK);
	
	//Ogre::Any any = Ogre::Any(UserAnyPair(Ogre::Any(static_cast<PickableObject*>(this)), Ogre::Any(mXAxis)));
	//mXAxis->setUserAny(any);
	//
	//any = Ogre::Any(UserAnyPair(Ogre::Any(static_cast<PickableObject*>(this)), Ogre::Any(mYAxis)));
	//mYAxis->setUserAny(any);

	//any = Ogre::Any(UserAnyPair(Ogre::Any(static_cast<PickableObject*>(this)), Ogre::Any(mZAxis)));
	//mZAxis->setUserAny(any);

	mAxisNode->attachObject(mXAxis);
	mAxisNode->attachObject(mYAxis);
	mAxisNode->attachObject(mZAxis);

	mAxisLine = AppDemo::getSingleton().mSceneMgr->createManualObject("AxisEntity_AxisLine");
	mAxisLine->setDynamic(true);
	mAxisLine->estimateIndexCount(2);
	mAxisLine->estimateVertexCount(2);
	
	mAxisLine->begin("visX",Ogre::RenderOperation::OT_LINE_STRIP);
		mAxisLine->position(-100,0,0);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
		mAxisLine->position( 100,0,0);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
	mAxisLine->end();
	mAxisLineNode = mAxisNode->createChildSceneNode("AxisLineNode");
	mAxisLineNode->attachObject(mAxisLine);
	
	setVisible(false);
}

void AxisEntity::setPosition(const Ogre::Vector3& pos)
{
	mAxisNode->setPosition(pos);
}
void AxisEntity::setVisible(bool visibility)
{
	mXAxis->setVisible(visibility);
	mYAxis->setVisible(visibility);
	mZAxis->setVisible(visibility);
	mAxisLineNode->setVisible(false);
}
bool  AxisEntity::notifyReleased()
{
	mAxisLineNode->setVisible(false);
	mXAxis->setVisible(false);
	mYAxis->setVisible(false);
	mZAxis->setVisible(false);
	return true;
}
bool  AxisEntity::notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
{
	Ogre::Vector3 pos = ray.getPoint(mDistance);
	pos += mMouseOffset;
	Ogre::Vector3 oriPos = mAxisNode->getPosition();
	if(mPickAxis == 0)
	{
		mAxisNode->setPosition(pos.x, oriPos.y, oriPos.z);
  	}
	else if(mPickAxis == 1)
	{
		mAxisNode->setPosition(oriPos.x, pos.y, oriPos.z);
	}
	else if(mPickAxis == 2)
	{
		mAxisNode->setPosition(oriPos.x, oriPos.y, pos.z);
	}
	if(mBindObject != nullptr)
	{
  		mBindObject->updatePosition(mAxisNode->getPosition());
	}

	mXAxis->setVisible(false);
	mYAxis->setVisible(false);
	mZAxis->setVisible(false);
	mAxisLineNode->setVisible(true);

 	return true;
}
bool  AxisEntity::notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset)
{
	if(entry.movable->getName() == "XAxis")
	{
		mAxisLine->beginUpdate(0); 
		mAxisLine->position(-100,0,0);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
		mAxisLine->position( 100,0,0);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
		mAxisLine->end();
		mPickAxis = 0;
 	}
	else if(entry.movable->getName() == "YAxis")
	{
		mAxisLine->beginUpdate(0); 
		mAxisLine->position(0, 100,0);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
		mAxisLine->position(0,-100,0);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
		mAxisLine->end();
		mPickAxis = 1;
 	}
	else if(entry.movable->getName() == "ZAxis")
	{
		mAxisLine->beginUpdate(0); 
		mAxisLine->position(0, 0, 100);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
		mAxisLine->position(0, 0, -100);
		mAxisLine->colour(1.0f, 0.0f, 0.0f);		
		mAxisLine->end();
		mPickAxis = 2;
 	}
	mMouseOffset = offset;
	mDistance = entry.distance;
	return true;
}

