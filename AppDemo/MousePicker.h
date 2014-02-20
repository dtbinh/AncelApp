#ifndef __MousePicker_h_
#define __MousePicker_h_
 
#include <OgreSceneQuery.h>
#include <OgreSingleton.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OIS.h>

#include "SelectionBox.h"
#include <ostream>

namespace AncelApp
{
	enum ENTITY_QUERY_MASK 
	{
		EQM_NO_MASK = 1,
		EQM_TUBE_MASK = (1 << 1),
		EQM_BONE_MASK = (1 << 2),
		EQM_ENTITY_MASK = 6
 	};
	
	struct UserAnyPair
	{
		UserAnyPair(Ogre::Any _first, Ogre::Any _second)
			:first(_first),
			second(_second),
			third(Ogre::Any())
		{
 		}
		
		UserAnyPair(Ogre::Any _first, Ogre::Any _second, Ogre::Any _third)
			:first(_first),
			second(_second),
			third(_third)
		{
 		}
	 	friend	std::ostream& operator << (std::ostream& o, const UserAnyPair& v)
		{
			o << v.first;
			o << v.second;
			o << v.third;
			return o;
		}

		Ogre::Any first;
		Ogre::Any second;
		Ogre::Any third;
	};

	class PickableObject
	{
	public:

		PickableObject(){}
		PickableObject(ENTITY_QUERY_MASK &mask):mObjMask(mask){}
		virtual ~PickableObject(){}
		virtual bool  notifyReleased() = 0;
		virtual bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray) = 0;
		virtual bool  mousePressed(const OIS::MouseEvent &evt, const Ogre::Ray &ray) {return true;};
		virtual bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset) = 0;
		virtual bool  updatePosition(const Ogre::Vector3 &pos) {return true;}
		
		virtual bool  updateOperation(int operation) {return true;}
		virtual bool  applyOperation(int operation) {return true;}
		ENTITY_QUERY_MASK getQueryMask() const {return mObjMask;}

	protected:
		ENTITY_QUERY_MASK mObjMask;
	};

	class MousePicker:public Ogre::Singleton<MousePicker>
	{
	public:
		MousePicker(Ogre::Camera* cam, Ogre::SceneManager* sceneMgr);

		virtual ~MousePicker(){};
		virtual bool mouseMoved(const OIS::MouseEvent &evt);
		virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		void resetPicker() {mSelectObj = nullptr;}
 		bool setEnablePick(bool flag);

		void deselectObjects();
		void selectObject(Ogre::MovableObject* obj);

		void performSelection(const Ogre::Vector2& first, const Ogre::Vector2& second);
	private:
		bool					   mEnablePick;
		bool					   mIsObjectPicked;
		Ogre::SceneNode*           mSelectedNode;       
		Ogre::Camera*              mCamera;
		Ogre::SceneManager*        mSceneMgr;
		
		Ogre::MovableObject*       mMovableObj;
		PickableObject*			   mSelectObj;
		
		Ogre::RaySceneQuery*       mRayScnQuery;

		Ogre::Vector2 mStart, mStop;
		Ogre::PlaneBoundedVolumeListSceneQuery* mVolQuery;
		std::list<Ogre::MovableObject*> mSelected;
		bool mSelecting;
 		SelectionBox* mSelectionBox;  
   	};

}


#endif



