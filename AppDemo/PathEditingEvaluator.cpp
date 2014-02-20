#include "AppDemo.h"
#include "CommandManager.h"
#include "PathEditingEvaluator.h"
#include "MousePicker.h"
using namespace AncelApp;

template<> PathEditingEvaluator* Ogre::Singleton<PathEditingEvaluator>::msSingleton = nullptr;


PathEditingEvaluator::PathEditingEvaluator()
	:mDemoPath(nullptr),
	mPickedNode(nullptr),
	mVisibility(false)
{
	
	CommandManager::getInstance().registerCommand("Command_HalfCirclePath", MyGUI::newDelegate(this, &PathEditingEvaluator::createHalfCirclePath));
	CommandManager::getInstance().registerCommand("Command_StraightLinePath", MyGUI::newDelegate(this, &PathEditingEvaluator::createStraightLinePath));
	CommandManager::getInstance().registerCommand("Command_StringPath", MyGUI::newDelegate(this, &PathEditingEvaluator::createStringPath));
	CommandManager::getInstance().registerCommand("Command_SetPathVisibility", MyGUI::newDelegate(this, &PathEditingEvaluator::remove));
}
PathEditingEvaluator::~PathEditingEvaluator()
{
	if(mDemoPath) 
		delete mDemoPath;
}

void PathEditingEvaluator::remove(const MyGUI::UString& commandName, bool& result)
{
	if(mDemoPath)
	{
		delete mDemoPath;
		mDemoPath = nullptr;
	}	
}

bool  PathEditingEvaluator::notifyReleased()
{
 	 mPickedNode->showBoundingBox(false);
	 return true;
}

bool  PathEditingEvaluator::notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
{
	if(mDemoPath && mPickedNode && evt.state.buttonDown(OIS::MB_Left))
	{
 		Ogre::Vector3 position = ray.getPoint(mCollsionDepth) + mPickedNodeOffset;
		Ogre::Vector3 pos = position - mPickedNode->getPosition();
 
		mDemoPath->update_(mPickedIndex, position);
	}
	return true;
}

bool  PathEditingEvaluator::notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset)
{
 	AppDemo::getSingletonPtr()->mKeyboard->capture();
	
	mPickedIndex = Ogre::any_cast<int>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).third);
	
	if(AppDemo::getSingletonPtr()->mKeyboard->isKeyDown(OIS::KC_LCONTROL))
	{
		mPickedIndex = Ogre::any_cast<int>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).third);
		mDemoPath->updateControlPoint(mPickedIndex);

 		return false;
	}
	
	if(mDemoPath->isHandle(mPickedIndex))
	{
 		mCollsionDepth = entry.distance;
		mPickedNodeOffset = offset;
		mPickedNode = entry.movable->getParentSceneNode();
		mPickedIndex = Ogre::any_cast<int>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).third);
		return true;
 	}
  	return false;
}
		
void PathEditingEvaluator::createHalfCirclePath(const MyGUI::UString& commandName, bool& result)
{
	 MousePicker::getSingletonPtr()->resetPicker();
	 Eigen::MatrixXd position(60,4);

	 for(int i = 0; i < 60; i++)
	 {
		 double angle = 3.14 + i*3.14/60; 
 		 position(i, 0) = 10*sin(angle);
		 position(i, 1) = 15 ;
		 position(i, 2) = 10*cos(angle);
		 position(i, 3) = (i == 59 || i == 0) ? 1: 0;
 	 }  
	 if(mDemoPath) delete mDemoPath;

	 mDemoPath = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	 mDemoPath->setInit3DPath(position);
	 mDemoPath->createPath(MyGUI::utility::toString(this) + "_DemoPath_", "OrangePath", false, true);
	 mDemoPath->setVisbility(true);
}
void PathEditingEvaluator::createStraightLinePath(const MyGUI::UString& commandName, bool& result)
{
	 MousePicker::getSingletonPtr()->resetPicker();
 	 Eigen::MatrixXd position(60,4);

	 for(int i = 0; i < 60; i++)
	 {
		 double angle = 3.14 + i*3.14/40; 
 		 position(i, 0) = i;
		 position(i, 1) = 15 ;
		 position(i, 2) = 0;
		 position(i, 3) = (i == 59 || i == 0 || i% 20 == 0) ? 1: 0;
 	 }  
	 if(mDemoPath) delete mDemoPath;

	 mDemoPath = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	 mDemoPath->setInit3DPath(position);
	 mDemoPath->createPath(MyGUI::utility::toString(this) + "_DemoPath_", "OrangePath", false, true);
	 mDemoPath->setVisbility(true);
}
void PathEditingEvaluator::createStringPath(const MyGUI::UString& commandName, bool& result)
{
	 MousePicker::getSingletonPtr()->resetPicker();
	 Eigen::MatrixXd position(200,4);

	 for(int i = 0; i < 200; i++)
	 {
		 double angle = 3.14 + i*3.14/10; 
 		 position(i, 0) = 2*sin(angle);
		 position(i, 1) = 15+ 2 * cos(angle);
		 position(i, 2) = i*0.2;
		 position(i, 3) = (i == 199 || i == 0) ? 1: 0;
 	 }  
	 if(mDemoPath) delete mDemoPath;

	 mDemoPath = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	 mDemoPath->setInit3DPath(position);
	 mDemoPath->createPath(MyGUI::utility::toString(this) + "_DemoPath_", "OrangePath", false, true);
	 mDemoPath->setVisbility(true);

}
 void PathEditingEvaluator::createPointPath()
 {
	 MousePicker::getSingletonPtr()->resetPicker();
	 Eigen::MatrixXd position(1,4);

	 for(int i = 0; i < 1; i++)
	 {
		 position(i, 0) = 2;
		 position(i, 1) = 15;
		 position(i, 2) = 3;
		 position(i, 3) = 1;
	 }  
	 if(mDemoPath) delete mDemoPath;

	 mDemoPath = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	 mDemoPath->setInit3DPath(position);
	 mDemoPath->createPath(MyGUI::utility::toString(this) + "_DemoPath_", "OrangePath", false, true);
	 mDemoPath->setVisbility(true);
 }