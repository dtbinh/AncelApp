#include "MousePicker.h"
#include "AxisEntity.h"

using namespace AncelApp;

template<> MousePicker* Ogre::Singleton<MousePicker>::msSingleton = 0;

MousePicker::MousePicker(Ogre::Camera* cam, Ogre::SceneManager* sceneMgr)
	:mCamera(cam),
	mSceneMgr(sceneMgr),
	mEnablePick(true),
	mIsObjectPicked(false),
	mSelectObj(nullptr),
	mSelectedNode(nullptr)
{
 	mRayScnQuery = mSceneMgr->createRayQuery(Ogre::Ray());
	mRayScnQuery->setQueryMask(EQM_ENTITY_MASK);
}

bool MousePicker::setEnablePick(bool flag)
{
	mEnablePick = flag;
	return true;
}

bool MousePicker::mouseMoved(const OIS::MouseEvent &evt)
{
	if(mSelectObj != NULL)
	{
		Ogre::Vector2 mousePos(Ogre::Real(evt.state.X.abs), Ogre::Real(evt.state.Y.abs));
		Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.x/float(evt.state.width), mousePos.y/float(evt.state.height));
		mSelectObj->notifyMoved(evt, mouseRay);
   		return true;
	}
	return false;
}

bool MousePicker::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if (id == OIS::MB_Left && mEnablePick)
	{
		Ogre::Vector2 mousePos(Ogre::Real(evt.state.X.abs), Ogre::Real(evt.state.Y.abs));
		
		Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.x/float(evt.state.width), mousePos.y/float(evt.state.height));
	 
		mRayScnQuery->setRay(mouseRay);
 
		Ogre::RaySceneQueryResult& result = mRayScnQuery->execute();
		Ogre::RaySceneQueryResult::iterator it = result.begin();
	 
		if(mSelectObj)
			mSelectObj->mousePressed(evt,mouseRay);
		
		Ogre::RaySceneQueryResult::iterator mit = result.begin();
		
		while(it != result.end() && it->movable)
		{
			if(it->distance < mit->distance)
				mit= it;
			it++;
		}
		it = mit;
		
		if(it != result.end() && it->movable)
		{
			Ogre::Vector3 offset = it->movable->getParentSceneNode()->getPosition() - mouseRay.getPoint(it->distance);
		
		//	std::cout << it->movable->getName() << std::endl;
			
			if(!it->movable->getMovableType().compare("Entity"))
			{
 				Ogre::Any any = it->movable->getUserAny();
				UserAnyPair any_pair = Ogre::any_cast<UserAnyPair>(any);
				PickableObject *obj = Ogre::any_cast<PickableObject*>(any_pair.first);
				
				if(obj != nullptr)
				{
					if(mSelectObj) 
					{
						mSelectObj->notifyReleased();
 					}
					
					if(obj->notifyPicked(*it, offset))
					{
 						mSelectObj = obj;
						mMovableObj = it->movable;
						mSelectedNode = it->movable->getParentSceneNode();
						mSelectObj->mousePressed(evt,mouseRay);
 					}
					else
					{
						mMovableObj = nullptr;
						mSelectObj = nullptr;
						mSelectedNode = nullptr;
					}
				}
 			}
  	 		return true;
		}
		
 	}
	else if(id == OIS::MB_Right && mSelectObj)
	{
		mSelectedNode->showBoundingBox(false);
		mSelectObj->notifyReleased();
		mSelectObj = nullptr;
		mSelectedNode = nullptr;
		mMovableObj = nullptr;
	}
	return false;
}
void MousePicker::performSelection(const Ogre::Vector2& first, const Ogre::Vector2& second)
{
	float left = first.x, right = second.x;
	float top =  first.y, buttom = second.y;
	if(left > right)
		std::swap(left,right);
	if(top > buttom)
		std::swap(top,buttom);
	if((right - left)*(buttom - top) < 0.0001)
		return;
	Ogre::Ray topLeft = mCamera->getCameraToViewportRay(left,top);
	Ogre::Ray topRight = mCamera->getCameraToViewportRay(right,top);
	Ogre::Ray bottomLeft = mCamera->getCameraToViewportRay(left,buttom);
	Ogre::Ray bottomRight = mCamera->getCameraToViewportRay(right,buttom);

	Ogre::PlaneBoundedVolume vol;
	vol.planes.push_back(Ogre::Plane(topLeft.getPoint(3), topRight.getPoint(3), bottomRight.getPoint(3)));         // front plane
	vol.planes.push_back(Ogre::Plane(topLeft.getOrigin(), topLeft.getPoint(100), topRight.getPoint(100)));         // top plane
	vol.planes.push_back(Ogre::Plane(topLeft.getOrigin(), bottomLeft.getPoint(100), topLeft.getPoint(100)));       // left plane
	vol.planes.push_back(Ogre::Plane(bottomLeft.getOrigin(), bottomRight.getPoint(100), bottomLeft.getPoint(100)));   // bottom plane
	vol.planes.push_back(Ogre::Plane(topRight.getOrigin(), topRight.getPoint(100), bottomRight.getPoint(100)));     // right plane
	
	Ogre::PlaneBoundedVolumeList volList;
	volList.push_back(vol);
	mVolQuery->setVolumes(volList);
	Ogre::SceneQueryResult result = mVolQuery->execute();
	deselectObjects();
  
	Ogre::SceneQueryResultMovableList::iterator iter;
	for(iter = result.movables.begin(); iter != result.movables.end(); ++iter)
		 selectObject(*iter);
}
bool MousePicker::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	/*if(id == OIS::MB_Left && mSelectObj != NULL)
	{
		mSelectObj->notifyReleased();
		mSelectObj = nullptr;
		return true;
	}*/
	return false;
}
void MousePicker::deselectObjects()
{
	std::list<Ogre::MovableObject*>::iterator iter = mSelected.begin();
	for(iter; iter != mSelected.end(); iter++)
	{
		(*iter)->getParentSceneNode()->showBoundingBox(false);
	}

}
void MousePicker::selectObject(Ogre::MovableObject* obj)
{
	obj->getParentSceneNode()->showBoundingBox(true);
	mSelected.push_back(obj);
}
