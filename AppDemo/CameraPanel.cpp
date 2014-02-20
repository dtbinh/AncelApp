#include "AppDemo.h"
#include "CameraPanel.h"
#include "MousePicker.h"
#include <MyGUI.h>
using namespace AncelApp;

template<> CameraPanel* Ogre::Singleton<CameraPanel>::msSingleton = 0;

CameraPanel::CameraPanel(Ogre::Camera *cam)
	:mCamera(cam),
	 mCamMode(1)
{
//	MyGUI::LayoutManager::getInstance().loadLayout("ControlPanel.layout");
	MyGUI::VectorWidgetPtr mWidgets = MyGUI::LayoutManager::getInstance().loadLayout("CameModeLayout.layout");

	mWidgets.at(0)->setAlign(MyGUI::Align::Top);
	mBtnFixed       = (MyGUI::Button*)mWidgets.at(0)->findWidget("RBCamFixed");
	mBtnManipulable = (MyGUI::Button*)mWidgets.at(0)->findWidget("RBCamManipulable");
	mBtnTracking    = (MyGUI::Button*)mWidgets.at(0)->findWidget("RBCamTracking");

	mBtnFixed->setStateSelected(true);
	mBtnFixed->eventMouseButtonClick       += MyGUI::newDelegate(this, &CameraPanel::notifyMouseButtonClick);
	mBtnManipulable->eventMouseButtonClick += MyGUI::newDelegate(this, &CameraPanel::notifyMouseButtonClick);
	mBtnTracking->eventMouseButtonClick    += MyGUI::newDelegate(this, &CameraPanel::notifyMouseButtonClick);
}
void CameraPanel::notifyMouseButtonClick(MyGUI::Widget* _sender)
{
	MyGUI::Button* button = _sender->castType<MyGUI::Button>();
	bool pressed = button->getStateSelected();
	
	if(!pressed)
	{
		mBtnFixed->setStateSelected(false);
		mBtnManipulable->setStateSelected(false);
		mBtnTracking->setStateSelected(false);
		button->setStateSelected(!pressed);
		
		std::string btName = button->getName();
 		if(btName == "RBCamFixed")
			mCamMode = 0;
		else if(btName == "RBCamFixed")
			mCamMode = 1;
		else 
			mCamMode = 2;
	}
}
CameraPanel::~CameraPanel()
{
 
}
bool CameraPanel::mouseMoved(const OIS::MouseEvent &evt)
{
 
	if(mCamMode == 1 || mCamMode == 2)
	{
		if (evt.state.buttonDown(OIS::MB_Middle))
		{
		/*	Ogre::Real yaw   = -mCamera->getOrientation().getYaw().valueDegrees() + evt.state.X.rel * 0.2;
			Ogre::Real pitch = -mCamera->getOrientation().getPitch().valueDegrees()+ evt.state.Y.rel * 0.2;*/
			
			static float pitchValue = 15;
			static float yawValue = 0; 
			
	/*		std::cout << "++++++++++++++++++++++++++++++++++++++++-----" << std::endl;
			std::cout << yawValue << std::endl;
			std::cout << pitchValue << std::endl;
			std::cout << "----------------------------------------" << std::endl;

			std::cout << -mCamera->getRealOrientation().getYaw().valueDegrees() << std::endl;
			std::cout << -mCamera->getRealOrientation().getPitch().valueDegrees() << std::endl;*/

	/*		Ogre::Vector3 vp(0,0,1.0);
			Ogre::Vector3 rot(evt.state.X.rel*0.2,evt.state.Y.rel*0.2,50.0);
	 
			rot.normalise();
			double cosTheta = vp.dotProduct(rot);
			Ogre::Vector3 axis = vp.crossProduct(rot);
			double theta = acos(cosTheta);
			Ogre::Quaternion tempOri(Ogre::Radian(theta),-axis);
			Ogre::Quaternion camOri = mCamera->getOrientation();
			camOri = camOri *tempOri;*/
			yawValue += evt.state.X.rel * 0.2;
			pitchValue += evt.state.Y.rel * 0.2;
	
			//std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
			//std::cout << yawValue << std::endl;
			//std::cout << pitchValue << std::endl;
			//std::cout << "#####################################################" << std::endl;

			//std::cout << yaw << std::endl;
			//std::cout << pitch << std::endl;

	 	 	mCamera->moveRelative(Ogre::Vector3(0, 0, -50));
	 	 	mCamera->setOrientation(Ogre::Quaternion::IDENTITY);
 			mCamera->pitch(Ogre::Degree(-pitchValue));
			mCamera->yaw(Ogre::Degree(-yawValue));
		 //	mCamera->setOrientation(camOri);
			mCamera->moveRelative(Ogre::Vector3(0.0, 0.0, 50));
  		}

		if (evt.state.buttonDown(OIS::MB_Right))
		{
			mCamera->moveRelative(Ogre::Vector3(-evt.state.X.rel * 0.5, evt.state.Y.rel * 0.5, 0));
 		}

		if (evt.state.Z.rel != 0)
		{
			float movRel = evt.state.Z.rel / 50.f;
			mCamera->moveRelative(Ogre::Vector3(0.0f, 0.0f, -100));
			Ogre::Vector3 target = mCamera->getPosition();
			mCamera->moveRelative(Ogre::Vector3(0.0f, 0.0f, 100));

			Ogre::Vector3 v = (mCamera->getPosition() - target).normalisedCopy() * movRel;
  			Ogre::Vector3 newPos = mCamera->getPosition() - v;
			Ogre::Vector3 camPos = mCamera->getPosition();
 	
  			float val = newPos.dotProduct(camPos);
 
			if (val > 0)
	 			mCamera->move(-v);
 		}
	}
 	return true;
}

bool CameraPanel::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{ 
  	return false;
}
bool CameraPanel::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
  	return false;
}