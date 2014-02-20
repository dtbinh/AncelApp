#include "ImagePanel.h"
#include "MotionManager.h"
#include "AnimationManager.h"
#include "SkeletonManager.h"
#include "SceneEntityManager.h"
#include "PathEditingEvaluator.h"
#include "Path.h"
#include "MousePicker.h"
#include "AnimationEditor.h"
#include <algorithm>
#include <fstream>
#include <MyGUI.h>
#include "AppDemo.h"
#include "AppUtility.h"
#include "CommandManager.h"
#include "StatePanel.h"
#include "Skeleton.h"

using namespace AncelApp;


template<> ImagePanel* Ogre::Singleton<ImagePanel>::msSingleton = nullptr;

#define NUMBONE 19
//#define F 30
#define PI 3.1415926
#define Zscale 1
#define LengthScale 6
#define InitalZ 80.0
//double BoneLength[NUMBONE]={};


ImagePanel::ImagePanel():
		mBtnGetPoint(nullptr),
		enableGetPoint(false),
		count(0),
	wraps::BaseLayout("ImagePanel.layout")
{
	CommandManager::getInstance().registerCommand("Command_ShowIMGPanel",MyGUI::newDelegate(this,&ImagePanel::showPanel));
	mMainWidget->setVisible(false);
	MyGUI::WindowPtr winptr = static_cast<MyGUI::WindowPtr>(mMainWidget);
	winptr->eventWindowButtonPressed += MyGUI::newDelegate(this,&ImagePanel::windowButtonPressed);

	assignWidget(mBtnGetPoint,"I_GET_POINT");

	mBtnGetPoint->eventMouseButtonClick += MyGUI::newDelegate(this,&ImagePanel::notifyMouseButtonClick);
}

ImagePanel::~ImagePanel()
{

}

void ImagePanel::initialise()
{

}
void ImagePanel::shutdown()
{

}

bool ImagePanel::setVisible(bool visibility)
{
	visibility = true;
	return true;
}

void ImagePanel::showPanel(const MyGUI::UString& commandName,bool& result)
{
	mMainWidget->setVisible(!mMainWidget->getVisible());
	double RealFrame[46]={31.9968,17.3527,-1.74667,-1.5,0.414691,-1.21422,6.74386,-1.20847,-1.62689,0.797958,-1.61431,3.72571,-2.80404,-0.77102,4.92858,-14.3392,-2.24257,-8.58431,20.4642,-1.72961,2.9674,8.88727,-1.16586,2.23338,-44.1537,-23.2042,-67.3544,20.95,19.3371,-25.6329,2.85222,85.0565,37.0345,-0.212245,-17.7543,3.21648,26.5843,31.0148,-6.6902,-3.54914,-7.47355,-0.439767,-18.1862,64.3926,21.4932,-11.6105
	};

	Skeleton *mSkeleton = new AncelApp::Skeleton("testskel");
	mSkeleton->loadSkeletonFromXML("actor\\12.xml");
	Eigen::MatrixXd data(1,46);
	for (int i = 0; i < 46; i++)
	{
		data(0,i) = RealFrame[i];
	}

	Motion *mMotion = new Motion(data, "imagetest");
	Animation *mAnimation;
	::SetCurrentDirectoryA(AppDemo::getSingletonPtr()->mWorkDirectory.c_str());
	mSkeleton->attachSkeletonToScene(AppDemo::getSingletonPtr()->mSceneMgr);
	if (mMotion != nullptr && mSkeleton != nullptr)
	{
		if (!AnimationManager::getSingleton().isSkeletonUsed(mSkeleton))
		{
			std::string animname = AnimationManager::getSingleton().addAnimation(mMotion, mSkeleton);
			mAnimation = AnimationManager::getSingletonPtr()->getAnimation(animname);
			mAnimation->setEnabled(true);
		}
	}
}

void ImagePanel::windowButtonPressed(MyGUI::Window* _sender,const std::string& _name)
{
	mMainWidget->setVisible(false);
}

void ImagePanel::notifyMouseButtonClick(MyGUI::Widget* _sender)
{
	MyGUI::ButtonPtr btn = static_cast<MyGUI::ButtonPtr>(_sender);
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length()-mPrefix.length());
	bool pressed = btn->getStateSelected();
	bool ret = false;

	if (btnName == "I_GET_POINT")
	{
		//GET POINT
		EnableGetPoint("", ret);
	}
}

void ImagePanel::windowResized(Ogre::RenderWindow* rw)
{
	mMainWidget->setPosition(0,AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
}

bool ImagePanel::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left && enableGetPoint)
	{

		Ogre::Vector2 mousePos(Ogre::Real(evt.state.X.abs), Ogre::Real(evt.state.Y.abs));
		cout << "mousePos:" << mousePos.x << "," << mousePos.y << endl;

		//cout << "w h" << evt.state.width << ";" << evt.state.height << endl;

		Image2DPos[count].x = mousePos.x;
		Image2DPos[count++].y = mousePos.y;
		if (count > 21)
		{
			enableGetPoint = false;
			//double l = 5;
			//double F = sqrt(((Image2DPos[0].x - Image2DPos[1].x)*(Image2DPos[0].x - Image2DPos[1].x) + (Image2DPos[0].y - Image2DPos[1].y)*(Image2DPos[0].y - Image2DPos[1].y)))/l;
			//double dz = sqrt(l*l - ((Image2DPos[0].x - Image2DPos[1].x)*(Image2DPos[0].x - Image2DPos[1].x) + (Image2DPos[0].y - Image2DPos[1].y)*(Image2DPos[0].y - Image2DPos[1].y))/(F*F));

			double Length[21] = {2,8,6,2.75,2.75,6,8,2,1.8,1.8,1.8,1.8,3.7,3.8,4.8,2.8,3,3.8,4.8,2.8,3};   //*5
			for (int i = 0; i < 21; i++)
			{
				Length[i] = Length[i] * LengthScale;
			}
			double PN[21] = {-1.0,1.0,-1.0,-1.0,1.0,1.0,-1.0,1.0,-1.0,1.0,1.0,-1.0,1.0,-1.0,-1.0,1.0,1.0,-1.0,1.0,1.0,1.0};
			double dz[21] = {0};
			double MinF = 100,temp;
			for (int i = 0; i < 8; i++)
			{
				temp = sqrt(((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y)))/Length[i];
				if (temp > MinF)
					MinF = temp;
			}
			for (int i = 9; i < 13; i++)
			{
				if (i == 9)
				{
					temp = sqrt(((Image2DPos[4].x - Image2DPos[i].x)*(Image2DPos[4].x - Image2DPos[i].x) + (Image2DPos[4].y - Image2DPos[i].y)*(Image2DPos[4].y - Image2DPos[i].y)))/Length[i];
					if (temp > MinF)
						MinF = temp;
				}
				temp = sqrt(((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y)))/Length[i];
				if (temp > MinF)
					MinF = temp;				
			}
			for (int i = 14; i < 17; i++)
			{
				if (i == 14)
				{
					temp = sqrt(((Image2DPos[11].x - Image2DPos[i].x)*(Image2DPos[11].x - Image2DPos[i].x) + (Image2DPos[11].y - Image2DPos[i].y)*(Image2DPos[11].y - Image2DPos[i].y)))/Length[i];
					if (temp > MinF)
						MinF = temp;
				}
				temp = sqrt(((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y)))/Length[i];
				if (temp > MinF)
					MinF = temp;	
			}

			for (int i = 18; i < 21; i++)
			{
				if (i == 18)
				{
					temp = sqrt(((Image2DPos[11].x - Image2DPos[i].x)*(Image2DPos[11].x - Image2DPos[i].x) + (Image2DPos[11].y - Image2DPos[i].y)*(Image2DPos[11].y - Image2DPos[i].y)))/Length[i];
					if (temp > MinF)
						MinF = temp;
				}
				temp = sqrt(((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y)))/Length[i];
				if (temp > MinF)
					MinF = temp;	
			}
			
			
			double ChangeZ = InitalZ;
			double RootZ = InitalZ;
			double LowerNeckZ = RootZ;
			double frame[46],tep = 0;
			int frameC = 0;
			//下肢
			for (int i = 0; i < 8; i++)
			{
				tep = Length[i]*Length[i] - ((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y))/(MinF*MinF);
				if (tep < 0.01)
				{
					dz[i] = 0.0;
				}
				else
					dz[i] = sqrt(tep);
			}
			//躯干
			tep = Length[8]*Length[8] - ((Image2DPos[9].x - Image2DPos[4].x)*(Image2DPos[9].x - Image2DPos[4].x) + (Image2DPos[9].y - Image2DPos[4].y)*(Image2DPos[9].y - Image2DPos[4].y))/(MinF*MinF);
			if (tep < 0.01)
			{
				dz[8] = 0.0;
			}
			else
				dz[8] = sqrt(tep);

			for (int i = 9; i < 13; i++)
			{
				tep = Length[i]*Length[i] - ((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y))/(MinF*MinF);
				if (tep < 0.01)
				{
					dz[i] = 0.0;
				}
				else
					dz[i] = sqrt(tep);
			}
			//左上肢
			tep = Length[13]*Length[13] - ((Image2DPos[14].x - Image2DPos[11].x)*(Image2DPos[14].x - Image2DPos[11].x) + (Image2DPos[14].y - Image2DPos[11].y)*(Image2DPos[14].y - Image2DPos[11].y))/(MinF*MinF);
			if (tep < 0.01)
			{
				dz[13] = 0.0;
			}
			else
				dz[13] = sqrt(tep);

			for (int i = 14; i < 17; i++)
			{
				tep = Length[i]*Length[i] - ((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y))/(MinF*MinF);
				if (tep < 0.01)
				{
					dz[i] = 0.0;
				}
				else
					dz[i] = sqrt(tep);
			}
			//右上肢
			tep = Length[17]*Length[17] - ((Image2DPos[18].x - Image2DPos[11].x)*(Image2DPos[18].x - Image2DPos[11].x) + (Image2DPos[18].y - Image2DPos[11].y)*(Image2DPos[18].y - Image2DPos[11].y))/(MinF*MinF);
			if (tep < 0.01)
			{
				dz[13] = 0.0;
			}
			else
				dz[13] = sqrt(tep);

			for (int i = 18; i < 21; i++)
			{
				tep = Length[i]*Length[i] - ((Image2DPos[i].x - Image2DPos[i+1].x)*(Image2DPos[i].x - Image2DPos[i+1].x) + (Image2DPos[i].y - Image2DPos[i+1].y)*(Image2DPos[i].y - Image2DPos[i+1].y))/(MinF*MinF);
				if (tep < 0.01)
				{
					dz[i] = 0.0;
				}
				else
					dz[i] = sqrt(tep);
			}



			Image3DPos[0].x = (Image2DPos[0].x - evt.state.height/2) * InitalZ / MinF;
			Image3DPos[0].y = (Image2DPos[0].y - evt.state.width/2) * InitalZ / MinF;
			Image3DPos[0].z = InitalZ;				
			
/*
			for (int i = 0; i < 21; i++)
			{
				printf("dz :  %lf", dz[i]);
			}*/

			ChangeZ = InitalZ;
			for(int i = 1; i <= 8; i++)
			{
				Image3DPos[i].x = (Image2DPos[i].x - evt.state.height/2) * InitalZ / MinF;
				Image3DPos[i].y = (Image2DPos[i].y - evt.state.width/2) * InitalZ / MinF;
				Image3DPos[i].z = ChangeZ + PN[i-1] * dz[i-1] / Zscale;
				ChangeZ = Image3DPos[i].z;
				if (i == 4)
					RootZ = ChangeZ;
			}

			ChangeZ = RootZ;
			for (int i = 9; i <= 13; i++)
			{
				Image3DPos[i].x = (Image2DPos[i].x - evt.state.height/2) * InitalZ / MinF;
				Image3DPos[i].y = (Image2DPos[i].y - evt.state.width/2) * InitalZ / MinF;
				Image3DPos[i].z = ChangeZ + PN[i-1] * dz[i-1] / Zscale;
				ChangeZ = Image3DPos[i].z;
				if(i == 11)
					LowerNeckZ = ChangeZ;
			}

			ChangeZ = LowerNeckZ;
			for (int i = 14; i <= 17; i++)
			{
				Image3DPos[i].x = (Image2DPos[i].x - evt.state.height/2) * InitalZ / MinF;
				Image3DPos[i].y = (Image2DPos[i].y - evt.state.width/2) * InitalZ / MinF;
				Image3DPos[i].z = ChangeZ + PN[i-1] * dz[i-1] / Zscale;
				ChangeZ = Image3DPos[i].z;
			}
			ChangeZ = LowerNeckZ;
			for (int i = 18; i <= 21; i++)
			{
				Image3DPos[i].x = (Image2DPos[i].x - evt.state.height/2) * InitalZ / MinF;
				Image3DPos[i].y = (Image2DPos[i].y - evt.state.width/2) * InitalZ / MinF;
				Image3DPos[i].z = ChangeZ + PN[i-1] * dz[i-1] / Zscale;
				ChangeZ = Image3DPos[i].z;
			}

			for (int i = 0; i <= 21; i++)
			{
/*
				double tempLength = sqrt((Image3DPos[i].x-Image3DPos[i+1].x)*(Image3DPos[i].x-Image3DPos[i+1].x) + (Image3DPos[i].y-Image3DPos[i+1].y)*(Image3DPos[i].y-Image3DPos[i+1].y) + (Image3DPos[i].z-Image3DPos[i+1].z)*(Image3DPos[i].z-Image3DPos[i+1].z));
				frame[frameC++] = asin((Image3DPos[i].x - Image3DPos[i+1].x)/tempLength);
				frame[frameC++] = asin((Image3DPos[i].y - Image3DPos[i+1].y)/tempLength);
				frame[frameC++] = asin((Image3DPos[i].z - Image3DPos[i+1].z)/tempLength);*/
				printf("pos: %lf %lf %lf\n",Image3DPos[i].x,Image3DPos[i].y,Image3DPos[i].z);
			}
			
			frame[frameC++] = -31.9968;
			frame[frameC++] = 17.3527;
			frame[frameC++] = -1.74667;


/*
			for (int i = 0; i < frameC; i++)
			{
				printf("%lf ",frame[i]*180/PI);
			}
			printf("\n");
			*/
		}


		return true;
	}
	return false;
}

void ImagePanel::EnableGetPoint(const MyGUI::UString& commandName, bool& result)
{
	enableGetPoint = !enableGetPoint;
}