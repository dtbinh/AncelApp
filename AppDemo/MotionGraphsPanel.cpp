#include "MotionGraphsPanel.h"
#include "MotionManager.h"
#include "AppDemo.h"
#include "AppUtility.h"
#include "CommandManager.h"
#include "StatePanel.h"
#include <MyGUI.h>
#include "AnimationManager.h"
#include "SkeletonManager.h"
#include "SceneEntityManager.h"
#include "Path.h"
#include "MousePicker.h"
#include "MotionGraphs.h"
#include "AnimationEditor.h"
#include <algorithm>
#include <fstream>

using namespace AncelApp;

template<> MotionGraphsPanel* Ogre::Singleton<MotionGraphsPanel>::msSingleton = nullptr;


//#define FOR_DEBUG
#define FOR_PLAY
//#define KNOW_TIME

#define NUMBONE 19
#define WINDOW_SIZE 5
#define THRESHOLD_IN_WALK 2050.0
#define THRESHOLD_IN_RUN  5065.0
#define THRESHOLD_IN_JUMP 930.0
#define THRESHOLD_NOT_LOC_WALK  20*WINDOW_SIZE
#define THRESHOLD_NOT_LOC_RUN   19*WINDOW_SIZE
#define THRESHOLD_JUMP_BOT 10*WINDOW_SIZE
#define THRESHOLD_JUMP_TOP  48*WINDOW_SIZE
#define THRESHOLD_OUT  9.65
#define WINDOW_M_IN 5
#define WINDOW_M_OUT 5
#define FRAME_NUM_FOR_PLAY 2000


#define CLAMP(x , min , max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : x))


//权重实验
//static double boneWeight[NUMBONE]={0.3,0.15,0.1,0.1,0.125,0.125,0.2,0.15,0.1,
//	0.05,0.15,0.1,0.05,0.15,0.1,0.05,0.15,0.1,0.05};
static double boneWeight[NUMBONE]={0.1,0.05,0.05,0.025,0.0125,0.0125,0.05,0.1,0.05,
	0.025,0.1,0.05,0.025,0.1,0.05,0.025,0.1,0.05,0.025};

static std::string boneName[NUMBONE] = {"root","lowerback","upperback","thorax","lowerneck","upperneck","head","rhumerus","rradius",
	"rwrist","lhumerus","lradius","lwrist","rfemur","rtibia","rfoot","lfemur","ltibia","lfoot"};

// x y z
static int boneAxisUse[NUMBONE] = {111,111,111,111,111,111,111,111,100,010,111,100,010,111,100,101,111,100,101};

MotionGraphsPanel::MotionGraphsPanel() :
		mBtnAddMotion(nullptr),
		mBtnRemoveMotion(nullptr),
		mBtnGen(nullptr),
		mListsContent(nullptr),
		mMotion(nullptr),
		mAnimation(nullptr),
		mSkeleton(nullptr),
		playPath(nullptr),
		mEditor(nullptr),
		transIndex(0),
		transEdge(0),
		newMotionIndex(0),
		realIndex(0),
		realEdge(0),
		destX(0),
		destZ(0),
		enablePoint(false),
		clickNum(0),
		nowFrame(0),
		initK(0.0),
		doubleK(0.0),
		temp1X(0.0),
		temp1Z(0.0),
		temp2X(0.0),
		temp2Z(0.0),
		pathWhereCount(0),
	wraps::BaseLayout("MotionGraphsPanel.layout")
{
	//MotionList.clear();
	//memset(transNode,0,sizeof(transNode));
	memset(realNode,0,sizeof(realNode));
	memset(newMotionMap,0,sizeof(newMotionMap));
	memset(isExist,false,sizeof(isExist));

	memset(pathWherePoint,0,sizeof(pathWherePoint));
	//newMotionMap.clear();

	CommandManager::getInstance().registerCommand("Command_ShowMGPanel",MyGUI::newDelegate(this,&MotionGraphsPanel::showPanel));
	mMainWidget->setVisible(false);
	MyGUI::WindowPtr winptr = static_cast<MyGUI::WindowPtr>(mMainWidget);
	winptr->eventWindowButtonPressed += MyGUI::newDelegate(this,&MotionGraphsPanel::windowButtonPressed);
	
	assignWidget(mBtnAddMotion, "MG_ADD_MOTION");
	assignWidget(mBtnRemoveMotion, "MG_REMOVE_MOTION");
	assignWidget(mBtnGen, "MG_GEN_MG");
	assignWidget(mBtnPlay, "MG_PLAY");

	assignWidget(mBtnGoTwoAttribute, "MG_GO_TWOATTRIBUTE");
	assignWidget(mBtnGetPoint, "MG_GET_POINT");
	assignWidget(mBtnGoWhere, "MG_GO_WHERE");
	assignWidget(mBtnGoZone, "MG_GO_ZONE");

	assignWidget(mListsContent, "MG_CHAIN");

	mBtnAddMotion->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);
	mBtnRemoveMotion->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);
	mBtnGen->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);
	mBtnPlay->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);
	mBtnGoTwoAttribute->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);
	mBtnGetPoint->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);
	mBtnGoWhere->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);
	mBtnGoZone->eventMouseButtonClick += MyGUI::newDelegate(this,&MotionGraphsPanel::notifyMouseButtonClick);

	mSkeleton = new AncelApp::Skeleton("testskel");
	mSkeleton->loadSkeletonFromXML("actor\\113.xml");
}

MotionGraphsPanel::~MotionGraphsPanel()
{
	for(std::size_t i = 0; i < MotionList.size(); i++)
			delete MotionList[i];
	MotionList.clear();
	for (int i = 0; i < newMotionIndex; i++)
	{
		delete newMotionList[i];
	}
//	delete newMotionList;
	delete mMotion;
	delete mAnimation;
}

//----------------------------------------------------------------------
/*Author ruan.answer
  Date  4.1 - 4.8
  Des   主要是界面显示和动作等，如添加删除文件
*/
//----------------------------------------------------------------------
void MotionGraphsPanel::showPanel(const MyGUI::UString& commandName,bool& result)
{
	mMainWidget->setVisible(!mMainWidget->getVisible());
}

void MotionGraphsPanel::initialise()
{

}

void MotionGraphsPanel::shutdown()
{

}

void MotionGraphsPanel::windowButtonPressed(MyGUI::Window* _sender, const std::string& _name)
{
	mMainWidget->setVisible(false);
}

void MotionGraphsPanel::notifyMouseButtonClick(MyGUI::Widget* _sender)
{
	MyGUI::ButtonPtr btn = static_cast<MyGUI::ButtonPtr>(_sender);
	std::string btnName = btn->getName();
	btnName = btnName.substr(mPrefix.length(),btnName.length()-mPrefix.length());
	bool pressed = btn->getStateSelected();
	bool ret = false;
//	if (!pressed)
	{
		//btn->setStateSelected(!pressed);

		if (btnName == "MG_GEN_MG")
		{
			generateMotion("", ret);
		}
		else if (btnName == "MG_PLAY")
		{
			goRandom("", ret);
		}
		else if (btnName == "MG_GO_TWOATTRIBUTE")
		{
			goTwoAttribute("", ret);
		}
		else if (btnName == "MG_GO_WHERE")
		{
			goSomeWhere("", ret);
		}
		else if (btnName == "MG_GO_ZONE")
		{
			playMotion("", ret);
		}
		else if(btnName == "MG_ADD_MOTION")
		{
			loadMotionFile("", ret);
		}
		else if (btnName == "MG_REMOVE_MOTION")
		{
			removeMotionFile("", ret);
		}
		else if (btnName == "MG_GET_POINT")
		{
			collectPoint("", ret);
		}
	}

}

void MotionGraphsPanel::addMotion(const Motion *anim)
{
	std::string	motionName = anim->getName();
	if (motionName != "")
	{
		//MotionManager::getSingletonPtr()->addMotion(anim);
		MotionList.push_back(const_cast<Motion*>(anim));
		mListsContent->addItem(motionName);
		mListsContent->setIndexSelected(mListsContent->getItemCount() - 1);
	}
}

const Motion* MotionGraphsPanel::getMotion(const std::string& animName) const
{
	for (std::size_t i = 0; i < MotionList.size(); i++)
	{
		if (MotionList[i]->getName() == animName)
		{
			return MotionList[i];
		}
	}
	return NULL;
}

Motion* MotionGraphsPanel::getMotion(const std::string& animName)
{
	for (std::size_t i = 0; i < MotionList.size(); i++)
	{
		if (MotionList[i]->getName() == animName)
		{
			return MotionList[i];
		}
	}
	return NULL;
}

bool MotionGraphsPanel::removeMotion(const std::string animName)
{
	std::vector<Motion*>::iterator it;
	for (it = MotionList.begin(); it != MotionList.end(); it++)
	{
		if ((*it)->getName() == animName)
		{
		//	if (!AnimationManager::getSingletonPtr()->isMotionUsed(*it))
			{
				delete *it;
				MotionList.erase(it);
				return true;
			}
			return false;
		}
	}
	return false;
}

const std::size_t MotionGraphsPanel::size() const
{
	return MotionList.size();
}

const std::string& MotionGraphsPanel::getMotionName(std::size_t index)
{
	assert(index < MotionList.size());
	return MotionList[index]->getName();
}

std::string MotionGraphsPanel::loadMotion(const std::string& filename)
{
	MatrixXd mat = loadData(filename);
	if (mat.rows() > 0)
	{
		int pos = filename.find_last_of('\\') + 1;
		std::string motionName = filename.substr(pos);
		Motion *anim = new Motion(mat, motionName);
		
		//MotionManager::getSingletonPtr()->addMotion(anim);
		//MotionList.push_back(const_cast<Motion*>(anim));
		MotionList.push_back(anim);
		return anim->getName();
	}
	return "";
}

bool MotionGraphsPanel::saveMotion(const std::string &animName, const std::string& fileName)
{
	Motion* anim = getMotion(animName);
	if (anim)
	{
		anim->writeToFile(fileName);
		return true;
	}
	return false;
}

void MotionGraphsPanel::loadMotionFile(const MyGUI::UString& commandName,bool& result)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW",static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("dat",HWND(hWnd));
	if (filename != "")
	{
		std::string motionName = loadMotion(filename);

		if (motionName != "")
		{
			mListsContent->addItem(motionName);	
			mListsContent->setIndexSelected(mListsContent->getItemCount() - 1);
		}
	}
}

void MotionGraphsPanel::removeMotionFile(const MyGUI::UString& commandName,bool& result)
{
	std::size_t temp = mListsContent->getIndexSelected();
	if (temp != MyGUI::ITEM_NONE)
	{
		std::string moName = mListsContent->getItemNameAt(temp);
		if (removeMotion(moName))
		{
			mListsContent->removeItemAt(temp);
			mListsContent->clearIndexSelected();
			if (mListsContent->getItemCount() > temp)
			{
				mListsContent->setIndexSelected(temp);
			}
		}

	}
}

//----------------------------------------------------------------------
/*Author ruan.answer
  Date   4.1 - 4.10
  Des    运动图生成
*/
//----------------------------------------------------------------------
void MotionGraphsPanel::generateMotion(const MyGUI::UString& commandName, bool& result)
{
	std::size_t i, j;
	std::vector<double> frameTemp1,frameTemp2;
#ifdef KNOW_TIME
// cal the time of generate motion graphs 
	clock_t startTime, finishTime;
	startTime = clock();
#endif
	if (!MotionList.empty()) // && MotionList.size() > 1)
	{
		for(i = 0; i < MotionList.size(); i++)
		{
			for (j = 0; j < MotionList.size(); j++)
			{
				std::string motionName1 = MotionList.at(i)->getName();
				std::string motionName2 = MotionList.at(j)->getName();
				Motion* mo1 = getMotion(motionName1);
				Motion* mo2 = getMotion(motionName2);

				calculWindowDistance(*mo1,*mo2,i,j);
			}
		}
		if (realIndex > 0)
			realAddNewEdge();
#ifdef KNOW_TIME
		finishTime = clock();
		cout << "the time of generate motion graphs:" << (double)(finishTime - startTime) / CLOCKS_PER_SEC << "seconds" << endl;
#endif
#ifdef FOR_DEBUG
		cout << "realIndex:" << realIndex - 1 << endl;
		cout << "realEdge:" << realEdge - 1 << endl;
#endif
		mMotionGraphs.solve(realIndex);
	}
	else
	{
		printf("MotionList is empty !");
	}

}

void MotionGraphsPanel::realAddNewEdge()
{
	int temp1 = 0,temp2 = 0;
	qsort(realNode,realIndex,sizeof(realNode[0]),rrcmp);
	for (int i = 0; i < realIndex-1; i++)
	{
		temp1 = realNode[i].positionZ / 10000;
		temp2 = realNode[i+1].positionZ / 10000;
		if (temp1 == temp2)
		{
			mMotionGraphs.addEdge(realNode[i].ccount, realNode[i+1].ccount);
			//printf("=%d->%d",realNode[i].positionZ,realNode[i+1].positionZ);
			realEdge++;
		}
	}
}

void MotionGraphsPanel::calculWindowDistance(Motion &mo1, Motion &mo2, int molist1, int molist2)
{
	int totalNum1 = mo1.getTotalFrameNum();
	int totalNum2 = mo2.getTotalFrameNum();
	int i,j,k = 0,t = 0;
	double sum = 0.0;
	int realtemp1 = -1, realtemp2 = -1;

	if (molist1 == molist2)
	{
/* for debug 为了看窗口距离到底多远
		for (k = 0; k <WINDOW_SIZE; k++)
		{
			sum += calculTheDistance(mo1.getFrame(20+k),mo2.getFrame(200+WINDOW_SIZE+k))* (1.0 - (double)k/(double)WINDOW_SIZE);
		}
		printf("*%lf*\n", sum);*/
		if (molist1 == 0)
		{
			for(i = 0; i < totalNum1 - WINDOW_SIZE; i += WINDOW_SIZE)
			{
				for (j = 0; j < totalNum2 - WINDOW_SIZE; j += WINDOW_SIZE )
				{
					sum = 0.0;
					if (i - j >= 0)
					{
						if (i -j <= THRESHOLD_NOT_LOC_WALK)
						{
							continue;
						}
					}
					else if (i - j < 0)
					{
						if (j - i <= THRESHOLD_NOT_LOC_WALK)
						{
							continue;
						}
					}
					for (k = 0; k < WINDOW_SIZE; k++)
					{
						if(sum > THRESHOLD_IN_WALK)
							break;
						sum +=  calculTheDistance(mo1.getFrame(i+k),mo2.getFrame(j+k)) * (1.0 - (double)k/(double)WINDOW_SIZE);
					}
#ifdef FOR_DEBUG
					if (sum > 1.0)
					{
						cout << "+" << sum ;
					}
#endif
					if (sum < THRESHOLD_IN_WALK && sum > 1.0)
					{
						realtemp1 = -1;
						if (isExist[molist1 * 10000 + i])
						{
							realtemp1 = returnRealIndex(molist1 * 10000 + i);
						}
						else
						{
							isExist[molist1 * 10000 + i] = true;
							if (realtemp1 == -1)
							{
								realNode[realIndex].positionZ = molist1 * 10000 + i;
								realNode[realIndex].ccount = realIndex;
								realIndex++;
								realtemp1 = realIndex - 1;
							}

						}
						realtemp2 = -1;
						if (isExist[molist2 * 10000 + j + WINDOW_SIZE - 1])
						{
							realtemp2 = returnRealIndex( molist2 * 10000 + j + WINDOW_SIZE - 1);
						}
						else
						{
							isExist[molist2 * 10000 + j + WINDOW_SIZE - 1] = true;
							if (realtemp2 == -1)
							{
								realNode[realIndex].positionZ = molist2 * 10000 + j + WINDOW_SIZE - 1;
								realNode[realIndex].ccount = realIndex;
								realIndex++;
								realtemp2 = realIndex - 1;
							}
						}
					
						if (realNode[realtemp2].positionZ > realNode[realtemp1].positionZ)
						{
							mMotionGraphs.addEdge(realNode[realtemp2].ccount, realNode[realtemp1].ccount);
							//printf(":%d->%d",realNode[realtemp2].positionZ,realNode[realtemp1].positionZ);
							realEdge++;
							interpolateAnMotion(realNode[realtemp2].ccount, realNode[realtemp1].ccount);
						}
					}
				}
			}
		}
		else if (molist1 == 1)
		{
			for(i = 0; i < totalNum1 - WINDOW_SIZE; i += WINDOW_SIZE)
			{
				for (j = 0; j < totalNum2 - WINDOW_SIZE; j += WINDOW_SIZE )
				{
					sum = 0.0;
					if (i - j >= 0)
					{
						if (i -j <= THRESHOLD_NOT_LOC_RUN)
						{
							continue;
						}
					}
					else if (i - j < 0)
					{
						if (j - i <= THRESHOLD_NOT_LOC_RUN)
						{
							continue;
						}
					}
					for (k = 0; k < WINDOW_SIZE; k++)
					{
						if(sum > THRESHOLD_IN_RUN)
							break;
						sum +=  calculTheDistance(mo1.getFrame(i+k),mo2.getFrame(j+k)) * (1.0 - (double)k/(double)WINDOW_SIZE);
					}
#ifdef FOR_DEBUG
					if (sum > 1.0)
					{
						cout << "+" << sum ;
					}
#endif
					if (sum < THRESHOLD_IN_RUN && sum > 1.0)
					{
						realtemp1 = -1;
						if (isExist[molist1 * 10000 + i])
						{
							realtemp1 = returnRealIndex(molist1 * 10000 + i);
						}
						else
						{
							isExist[molist1 * 10000 + i] = true;
							if (realtemp1 == -1)
							{
								realNode[realIndex].positionZ = molist1 * 10000 + i;
								realNode[realIndex].ccount = realIndex;
								realIndex++;
								realtemp1 = realIndex - 1;
							}

						}
						realtemp2 = -1;
						if (isExist[molist2 * 10000 + j + WINDOW_SIZE - 1])
						{
							realtemp2 = returnRealIndex( molist2 * 10000 + j + WINDOW_SIZE - 1);
						}
						else
						{
							isExist[molist2 * 10000 + j + WINDOW_SIZE - 1] = true;
							if (realtemp2 == -1)
							{
								realNode[realIndex].positionZ = molist2 * 10000 + j + WINDOW_SIZE - 1;
								realNode[realIndex].ccount = realIndex;
								realIndex++;
								realtemp2 = realIndex - 1;
							}
						}
					
						if (realNode[realtemp2].positionZ > realNode[realtemp1].positionZ)
						{
							mMotionGraphs.addEdge(realNode[realtemp2].ccount, realNode[realtemp1].ccount);
							//printf(":%d->%d",realNode[realtemp2].positionZ,realNode[realtemp1].positionZ);
							realEdge++;
							interpolateAnMotion(realNode[realtemp2].ccount, realNode[realtemp1].ccount);
						}

					}
				}
			}
		}
		else if (molist1 == 2)
		{
			for(i = THRESHOLD_JUMP_BOT ; i < THRESHOLD_JUMP_BOT + 4 * WINDOW_SIZE; i += WINDOW_SIZE)
			{
				for (j = THRESHOLD_JUMP_TOP; j < THRESHOLD_JUMP_TOP + 4 * WINDOW_SIZE ; j += WINDOW_SIZE )
				{
					sum = 0.0;
					for (k = 0; k < WINDOW_SIZE; k++)
					{
						if(sum > THRESHOLD_IN_JUMP)
							break;
						sum +=  calculTheDistance(mo1.getFrame(i+k),mo2.getFrame(j+k)) * (1.0 - (double)k/(double)WINDOW_SIZE);
					}
#ifdef FOR_DEBUG
					if (sum > 1.0)
					{
						cout << "+" << sum ;
					}
#endif
					if (sum < THRESHOLD_IN_JUMP && sum > 1.0)
					{
						realtemp1 = -1;
						if (isExist[molist1 * 10000 + i])
						{
							realtemp1 = returnRealIndex(molist1 * 10000 + i);
						}
						else
						{
							isExist[molist1 * 10000 + i] = true;
							if (realtemp1 == -1)
							{
								realNode[realIndex].positionZ = molist1 * 10000 + i;
								realNode[realIndex].ccount = realIndex;
								realIndex++;
								realtemp1 = realIndex - 1;
							}

						}
						realtemp2 = -1;
						if (isExist[molist2 * 10000 + j + WINDOW_SIZE - 1])
						{
							realtemp2 = returnRealIndex( molist2 * 10000 + j + WINDOW_SIZE - 1);
						}
						else
						{
							isExist[molist2 * 10000 + j + WINDOW_SIZE - 1] = true;
							if (realtemp2 == -1)
							{
								realNode[realIndex].positionZ = molist2 * 10000 + j + WINDOW_SIZE - 1;
								realNode[realIndex].ccount = realIndex;
								realIndex++;
								realtemp2 = realIndex - 1;
							}
						}
					
						if (realNode[realtemp2].positionZ > realNode[realtemp1].positionZ)
						{
							mMotionGraphs.addEdge(realNode[realtemp2].ccount, realNode[realtemp1].ccount);
							//printf(":%d->%d",realNode[realtemp2].positionZ,realNode[realtemp1].positionZ);
							realEdge++;
							interpolateAnMotion(realNode[realtemp2].ccount, realNode[realtemp1].ccount);
						}

					}
				}
			}
		}
	}
	else
	{
		for (i = 0; i < totalNum1-WINDOW_SIZE; i += WINDOW_SIZE)
		{

			for (j = 0; j < totalNum2-WINDOW_SIZE; j += WINDOW_SIZE)
			{
				sum = 0.0;

				for (k = 0; k < WINDOW_SIZE; k++)
				{
					if (sum > THRESHOLD_OUT)
						break;
					sum += calculTheDistance(mo1.getFrame(i+k),mo2.getFrame(j+k)) * (1.0 - (double)k/(double)WINDOW_SIZE);
				}
#ifdef FOR_DEBUG
				if (sum > 1.0)
				{
					cout << "+" << sum ;
				}
#endif
				if (sum < THRESHOLD_OUT && sum > 1.0)
				{
					realtemp1 = -1;
					if (isExist[molist1 * 10000 + i + WINDOW_SIZE - 1])
					{
						realtemp1 = returnRealIndex(molist1 * 10000 + i + WINDOW_SIZE - 1);
					}
					else
					{
						isExist[molist1 * 10000 + i + WINDOW_SIZE - 1] = true;
						if (realtemp1 == -1)
						{
							realNode[realIndex].positionZ = molist1 * 10000 + i + WINDOW_SIZE - 1;
							realNode[realIndex].ccount = realIndex;
							realIndex++;
							realtemp1 = realIndex - 1;
						}
					}
					realtemp2 = -1;
					if (isExist[molist2 * 10000 + j])
					{
						realtemp2 = returnRealIndex(molist2 * 10000 + j);
					}
					else
					{
						isExist[molist2 * 10000 + j] = true;
						if (realtemp2 == -1)
						{
							realNode[realIndex].positionZ = molist2 * 10000 + j;
							realNode[realIndex].ccount = realIndex;
							realIndex++;
							realtemp2 = realIndex - 1;
						}
					}
					mMotionGraphs.addEdge(realNode[realtemp1].ccount, realNode[realtemp2].ccount);
					//printf(":%d->%d",realNode[realtemp1].positionZ,realNode[realtemp2].positionZ);
					realEdge++;
					interpolateAnMotion(realNode[realtemp1].ccount, realNode[realtemp2].ccount);
				}
			}
		}
	}
	return;
}

void  MotionGraphsPanel::interpolateAnMotion(int temp1,int temp2)
{
	int t1 = returnCountRealIndex(temp1);
	int t2 = returnCountRealIndex(temp2);
	int tt1 = realNode[t1].positionZ / 10000;
	int tt2 = realNode[t2].positionZ / 10000;
	int tt3 = realNode[t1].positionZ % 10000;
	int tt4 = realNode[t2].positionZ % 10000;
	std::vector<double> frame1, frame2, frametemp;

	int i, j;
	double a = 0.0;

	if (tt2 == tt1)  //同一运动间插值
	{
		Eigen::MatrixXd motionNew(WINDOW_M_IN, 46);
		Ogre::Vector3 rootPosition[WINDOW_M_IN],r1[WINDOW_M_IN],r2[WINDOW_M_IN];
		Ogre::Quaternion rotation[WINDOW_M_IN][19],rota1[19], rota2[19], ttt;
		EulerAngle te;

   //计算root相对位置
		frametemp = MotionList.at(tt1)->getFrame(tt3 - WINDOW_M_IN + 1);
		for (j = 1; j <= WINDOW_M_IN; j++)
		{
			frame1 = MotionList.at(tt1)->getFrame(tt3 - WINDOW_M_IN + 1 + j);
			r1[j-1][0] = frame1[0] - frametemp[0];   r1[j-1][1] = frame1[1] - frametemp[1];   r1[j-1][2] = frame1[2] - frametemp[2];
			frametemp = frame1;
		}
		frametemp = MotionList.at(tt2)->getFrame(tt4);
		for (j = 1; j <= WINDOW_M_IN; j++)
		{
			frame2 = MotionList.at(tt2)->getFrame(tt4 + j);
			r2[j-1][0] = frame2[0] - frametemp[0];   r2[j-1][1] = frame2[1] - frametemp[1];   r2[j-1][2] = frame2[2] - frametemp[2];
			frametemp = frame2;
		}

		for (i = 0; i< WINDOW_M_IN; i++)
		{
			a = 2.0 * ((double)(i+1)/(double)WINDOW_M_IN) * ((double)(i+1)/(double)WINDOW_M_IN) * ((double)(i+1)/(double)WINDOW_M_IN) - 3.0 * ((double)(i+1)/(double)WINDOW_M_IN) * ((double)(i+1)/(double)WINDOW_M_IN) + 1;
			frame1 = MotionList.at(tt1)->getFrame(tt3 - WINDOW_M_IN + 1 + i);
			frame2 = MotionList.at(tt2)->getFrame(tt4 + i);
			
			for (j = 0; j < NUMBONE; j++)         //得到每个骨头的朝向   存起来  rota1是个四元数组
			{
				rota1[j] = getRelativeQuaternion(frame1, boneName[j]);//mSkeleton->getBone(boneName[j])->getGlobalOri();
				//tty = QuaternionToEulerAngle(rota1[j]);
			}

			for (j = 0; j < NUMBONE; j++)
			{
				rota2[j] = getRelativeQuaternion(frame2, boneName[j]);//mSkeleton->getBone(boneName[j])->getGlobalOri();
				rotation[i][j] = slerpBy(rota2[j], rota1[j], a);   //球面插值  rotation[i][j]表示第i帧第j个骨骼
			}
			rootPosition[i] = a * r1[i] + (1.0 - a) * r2[i];   //root节点线性插值
			int tt = 0;

			motionNew(i, tt++) = rootPosition[i][0]; motionNew(i, tt++) = rootPosition[i][1]; motionNew(i, tt++) = rootPosition[i][2];

			for (j = 0; j < NUMBONE; j++)
			{
				te = QuaternionToEulerAngle(rotation[i][j]);  //r2
			//	ttt = EulerAngleToQuaternion(te);
				if (boneAxisUse[j] == 111)
				{
					motionNew(i, tt++) = te.m_fRoll;
					motionNew(i, tt++) = te.m_fPitch;
					motionNew(i, tt++) = te.m_fYaw;
				}
				else if (boneAxisUse[j] == 100)
				{
					motionNew(i, tt++) = te.m_fRoll;
				}
				else if (boneAxisUse[j] == 010)
				{
					motionNew(i, tt++) = te.m_fPitch;
				}
				else if (boneAxisUse[j] == 101)
				{
					motionNew(i, tt++) = te.m_fRoll;
					motionNew(i, tt++) = te.m_fYaw;
				}
			}

/*  for debug
			printf("%d\n",i+1);
			printf("root %lf %lf %lf %lf %lf %lf\n", motionNew(i,0),motionNew(i,1),motionNew(i,2),motionNew(i,3),motionNew(i,4),motionNew(i,5));
			printf("lowerback %lf %lf %lf\n", motionNew(i,6),motionNew(i,7),motionNew(i,8));
			printf("upperback %lf %lf %lf\n", motionNew(i,9),motionNew(i,10),motionNew(i,11));
			printf("thorax %lf %lf %lf\n", motionNew(i,12),motionNew(i,13),motionNew(i,14));
			printf("lowerneck %lf %lf %lf\n", motionNew(i,15),motionNew(i,16),motionNew(i,17));
			printf("upperneck %lf %lf %lf\n", motionNew(i,18),motionNew(i,19),motionNew(i,20));
			printf("head %lf %lf %lf\n", motionNew(i,21),motionNew(i,22),motionNew(i,23));
			printf("rclavicle 0.0 0.0\n");
			printf("rhumerus %lf %lf %lf\n", motionNew(i,24),motionNew(i,25),motionNew(i,26));
			printf("rradius %lf\n", motionNew(i,27));
			printf("rwrist %lf\n", motionNew(i,28));
			printf("rhand 0.0 0.0\n");
			printf("rfingers 0.0\n");
			printf("rthumb 0.0 0.0\n");
			printf("lclavicle 0.0 0.0\n");
			printf("lhumerus %lf %lf %lf\n", motionNew(i,29),motionNew(i,30),motionNew(i,31));
			printf("lradius %lf\n", motionNew(i,32));
			printf("lwrist %lf\n", motionNew(i,33));
			printf("lhand 0.0 0.0\n");
			printf("lfingers 0.0\n");
			printf("lthumb 0.0 0.0\n");
			printf("rfemur %lf %lf %lf\n", motionNew(i,34),motionNew(i,35),motionNew(i,36));
			printf("rtibia %lf\n", motionNew(i,37));
			printf("rfoot %lf %lf\n", motionNew(i,38),motionNew(i,39));
			printf("rtoes 0.0\n");
			printf("lfemur %lf %lf %lf\n", motionNew(i,40),motionNew(i,41),motionNew(i,42));
			printf("ltibia %lf\n", motionNew(i,43));
			printf("lfoot %lf %lf\n", motionNew(i,44),motionNew(i,45));
			printf("ltoes 0.0\n");*/
		}
		int chan = temp1 * 10000 + temp2;
		char buf1[17];
		itoa(chan, buf1, 10);
		//map and create
		newMotionMap[newMotionIndex] = temp1 * 10000 + temp2;
		newMotionList[newMotionIndex++] = new Motion(motionNew, string(buf1));
	}
	else
	{
		Eigen::MatrixXd motionNew(WINDOW_M_OUT, 46);
		Ogre::Vector3 rootPosition[WINDOW_M_OUT],r1[WINDOW_M_OUT],r2[WINDOW_M_OUT];
		Ogre::Quaternion rotation[WINDOW_M_OUT][19],rota1[19], rota2[19], ttt;
		EulerAngle te;

		//计算root相对位置
		frametemp = MotionList.at(tt1)->getFrame(tt3 - WINDOW_M_OUT + 1);
		for (j = 1; j <= WINDOW_M_OUT; j++)
		{
			frame1 = MotionList.at(tt1)->getFrame(tt3 - WINDOW_M_OUT + 1 + j);
			r1[j-1][0] = frame1[0] - frametemp[0];   r1[j-1][1] = frame1[1] - frametemp[1];   r1[j-1][2] = frame1[2] - frametemp[2];
			frametemp = frame1;
		}
		frametemp = MotionList.at(tt2)->getFrame(tt4);
		for (j = 1; j <= WINDOW_M_OUT; j++)
		{
			frame2 = MotionList.at(tt2)->getFrame(tt4 + j);
			r2[j-1][0] = frame2[0] - frametemp[0];   r2[j-1][1] = frame2[1] - frametemp[1];   r2[j-1][2] = frame2[2] - frametemp[2];
			frametemp = frame2;
		}

		for (i = 0; i< WINDOW_M_OUT; i++)
		{
			a = 2 * ((i+1)/WINDOW_M_OUT) * ((i+1)/WINDOW_M_OUT) * ((i+1)/WINDOW_M_OUT) - 3 * ((i+1)/WINDOW_M_OUT) * ((i+1)/WINDOW_M_OUT) + 1;
			frame1 = MotionList.at(tt1)->getFrame(tt3 - WINDOW_M_OUT + 1 + i);
			frame2 = MotionList.at(tt2)->getFrame(tt4 + i);
			for (j = 0; j < NUMBONE; j++)
			{
				rota1[j] = getRelativeQuaternion(frame1, boneName[j]);//mSkeleton->getBone(boneName[j])->getGlobalOri();
			}
			for (j = 0; j < NUMBONE; j++)
			{
				rota2[j] = getRelativeQuaternion(frame2, boneName[j]);//mSkeleton->getBone(boneName[j])->getGlobalOri();
				rotation[i][j] = slerpBy(rota2[j], rota1[j], a);
			}
			rootPosition[i] = a * r1[i] + (1.0 - a) * r2[i];
			int tt = 0;
			motionNew(i, tt++) = rootPosition[i][0]; motionNew(i, tt++) = rootPosition[i][1]; motionNew(i, tt++) = rootPosition[i][2];
		//	motionNew(i,3) = rotation[i]
			
			for (j = 0; j < NUMBONE; j++)
			{
				te = QuaternionToEulerAngle(rotation[i][j]);
				if (boneAxisUse[j] == 111)
				{
					motionNew(i, tt++) = te.m_fRoll;
					motionNew(i, tt++) = te.m_fPitch;
					motionNew(i, tt++) = te.m_fYaw;
				}
				else if (boneAxisUse[j] == 100)
				{
					motionNew(i, tt++) = te.m_fRoll;
				}
				else if (boneAxisUse[j] == 010)
				{
					motionNew(i, tt++) = te.m_fPitch;
				}
				else if (boneAxisUse[j] == 101)
				{
					motionNew(i, tt++) = te.m_fRoll;
					motionNew(i, tt++) = te.m_fYaw;
				}
			}
		}
		//cout << motionNew;
		int chan = temp1 * 10000 + temp2;
		char buf1[17];
		itoa(chan, buf1, 10);
		//map and create
		newMotionMap[newMotionIndex] = temp1 * 10000 + temp2;

		newMotionList[newMotionIndex++] = new Motion(motionNew, string(buf1));
	}
}

double MotionGraphsPanel::calculTheDistance(const std::vector<double> &theta1,const std::vector<double> &theta2)
{
	mSkeleton->update2(theta1,false);
	double xave1 = 0.0, zave1 = 0.0, xave2 = 0.0, zave2 = 0.0;
	double temp1[NUMBONE] = {}, temp2[NUMBONE] = {};
	std::size_t i,j;
	Ogre::Vector3 pos, allBonePos1[NUMBONE] = {}, allBonePos2[NUMBONE] = {};
	double angle[NUMBONE] = {}, x0[NUMBONE] = {}, z0[NUMBONE] = {}, dis[NUMBONE] = {}, sum = 0.0;
//	Eigen::MatrixXd  trans(4,4), rotat(4,4), tt(4,4);
//	Eigen::MatrixXd P1(4,1), P2(4,1);
	Ogre::Matrix4 trans, rotat, tt;
	Ogre::Vector4 P1, P2;

	for (i = 0; i < NUMBONE; i++)
	{
		pos = mSkeleton->getBone(boneName[i])->getGlobalPos();
		allBonePos1[i] = pos;
		xave1 +=  boneWeight[i] * pos[0];
		zave1 +=  boneWeight[i] * pos[2];
	}

	mSkeleton->update2(theta2,false);
	for (i = 0; i < NUMBONE; i++)
	{
		pos = mSkeleton->getBone(boneName[i])->getGlobalPos();
		allBonePos2[i] = pos;
		xave2 +=  boneWeight[i] * pos[0];
		zave2 +=  boneWeight[i] * pos[2];
	}

	for (i = 0; i < NUMBONE; i++)
	{
		temp1[i] = 0.0;
		temp2[i] = 0.0;
		for(j = 0; j < NUMBONE; j++)
		{
			temp1[i] += boneWeight[j] * (allBonePos1[j][0] * allBonePos2[j][2] - allBonePos2[j][0] * allBonePos1[j][2]);
			temp2[i] += boneWeight[j] * (allBonePos1[j][0] * allBonePos2[j][0] + allBonePos1[j][2] * allBonePos2[j][2]);
		}
	}

	for (i = 0; i < NUMBONE; i++)
	{
		angle[i] = atan((temp1[i] - (xave1 * zave2 - xave2 * zave1)/getBoneTotalWeight()) / (temp2[i] - (xave1 * xave2 + zave1 * zave2)/getBoneTotalWeight()));
		x0[i] = (xave1 - xave2 * cos(angle[i]) - zave2 * sin(angle[i])) / getBoneTotalWeight();
		z0[i] = (zave1 + xave2 * sin(angle[i]) - zave2 * cos(angle[i])) / getBoneTotalWeight();
	}


	trans[0][0] = 1; trans[0][1] = 0; trans[0][2] = 0; trans[0][3] = 0;
	trans[1][0] = 0; trans[1][1] = 1; trans[1][2] = 0; trans[1][3] = 0;
	trans[2][0] = 0; trans[2][1] = 0; trans[2][2] = 1; trans[2][3] = 0; trans[3][3] = 1;
	rotat[0][1] = 0; rotat[0][3] = 0; rotat[1][0] = 0; rotat[1][1] = 1; rotat[1][2] = 0; rotat[1][3] = 0;
	rotat[2][1] = 0; rotat[2][3] = 0; rotat[3][0] = 0; rotat[3][1] = 0; rotat[3][2] = 0; rotat[3][3] = 1; 

	for (i = 0; i < NUMBONE; i++)
	{
		P1[0] = allBonePos1[i][0], P1[1] = allBonePos1[i][1], P1[2] = allBonePos1[i][2]; P1[3] = 1;
		P2[0] = allBonePos2[i][0], P2[1] = allBonePos2[i][1], P2[2] = allBonePos2[i][2]; P2[3] = 1;
		trans[3][0] = x0[i]; trans[3][1] = 0; trans[3][2] = z0[i];
		rotat[0][0] = cos(angle[i]); rotat[0][2] = -sin(angle[i]); rotat[2][0] = -rotat[0][2]; rotat[2][2] = rotat[0][0];

		P2 = calMul(trans, calMul(rotat, P2));

		dis[i] = boneWeight[i] * ((P1[0] - P2[0]) * (P1[0] - P2[0]) + (P1[1] - P2[1]) * (P1[1] - P2[1]) + (P1[2] - P2[2]) * (P1[2] - P2[2])); 

		sum += dis[i];

	}

	return sum;
}

bool MotionGraphsPanel::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if (id == OIS::MB_Left && enablePoint)
	{
		Ogre::Vector2 mousePos(Ogre::Real(evt.state.X.abs), Ogre::Real(evt.state.Y.abs));

		Ogre::Ray mouseRay = AppDemo::getSingleton().mCamera->getCameraToViewportRay(mousePos.x/float(evt.state.width), mousePos.y/float(evt.state.height));
		Ogre::Vector3 v0,v1,v2;
		v0[0] = -1000.0; v0[1] = 0.0; v0[2] = 1000.0;
		v1[0] = 3000.0; v1[1] = 0.0; v1[2] = 1000.0;
		v2[0] = -1000.0; v2[1] = 0.0; v2[2] = 3000.0;
		float a, b, c;
		float x, y, z;
		if (intersectPlane(mouseRay.getOrigin(),mouseRay.getDirection(),v0,v1,v2,&a,&b,&c))
		{
			v0 = mouseRay.getOrigin();
			v1 = mouseRay.getDirection();

			x = v0[0] + v1[0] * a;
			y = v0[1] + v1[1] * a;
			z = v0[2] + v1[2] * a;

			destX = x;
			destZ = z;
			cout << "(" << x << ":"  << z<< ")" << endl;
			mMotionGraphs.endPoint.x = (int)((x + 400) / 8);
			mMotionGraphs.endPoint.y = (int)((z + 400) / 8);

			pathWherePoint[pathWhereCount].x = x;
			pathWherePoint[pathWhereCount++].y = z;
		//	endPoint.x = 30;
		//	endPoint.y = 60;
		}
		else
		{
			cout << "no corss" << endl;
		}	
		return true;
	}
	return false;
}

//----------------------------------------------------------------------
/*Author ruan.answer
  Date   4.1 - 4.15
  Des   生成运动图需要的函数
*/
//----------------------------------------------------------------------
bool MotionGraphsPanel::intersectPlane(const Ogre::Vector3& orig, const Ogre::Vector3& dir, Ogre::Vector3& v0, Ogre::Vector3& v1, Ogre::Vector3& v2, float* t, float* u, float* v)
{
	// E1
	Ogre::Vector3 E1 = v1 - v0;

	// E2
	Ogre::Vector3 E2 = v2 - v0;

	// P
	Ogre::Vector3 P = dir.crossProduct(E2);

	// determinant
	float det = E1.absDotProduct(P);

	// keep det > 0, modify T accordingly
	Ogre::Vector3 T;
	if( det >0 )
	{
		T = orig - v0;
	}
	else
	{
		T = v0 - orig;
		det =-det;
	}

	// If determinant is near zero, ray lies in plane of triangle
	if( det <0.0001f )
		return false;

	// Calculate u and make sure u <= 1
	*u = T.absDotProduct(P);
	if( *u <0.0f||*u > det )
		return false;

	// Q
	Ogre::Vector3 Q = T.crossProduct(E1);

	// Calculate v and make sure u + v <= 1
	*v = dir.absDotProduct(Q);
	if( *v <0.0f||*u +*v > det )
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	*t = E2.absDotProduct(Q);

	float fInvDet =1.0f/ det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return true;
}

Ogre::Quaternion MotionGraphsPanel::slerpBy(const Ogre::Quaternion &q0, const Ogre::Quaternion &q1, float t) {

	// Check for out-of range parameter and return edge points if so

	if (t <= 0.0f) return q0;
	if (t >= 1.0f) return q1;

	// Compute "cosine of angle between quaternions" using dot product
	Ogre::Quaternion result;
	float cosOmega = q0.w*q1.w + q0.x*q1.x + q0.y*q1.y +q0.z*q1.z;

	// If negative dot, use -q1.  Two quaternions q and -q
	// represent the same rotation, but may produce
	// different slerp.  We chose q or -q to rotate using
	// the acute angle.

	float q1w = q1.w;
	float q1x = q1.x;
	float q1y = q1.y;
	float q1z = q1.z;
	if (cosOmega < 0.0f) {
		q1w = -q1w;
		q1x = -q1x;
		q1y = -q1y;
		q1z = -q1z;
		cosOmega = -cosOmega;
	}

	// We should have two unit quaternions, so dot should be <= 1.0

	assert(cosOmega < 1.1f);

	// Compute interpolation fraction, checking for quaternions
	// almost exactly the same

	float k0, k1;
	if (cosOmega > 0.9999f) {

		// Very close - just use linear interpolation,
		// which will protect againt a divide by zero

		k0 = 1.0f-t;
		k1 = t;

	} else {

		// Compute the sin of the angle using the
		// trig identity sin^2(omega) + cos^2(omega) = 1

		float sinOmega = sqrt(1.0f - cosOmega*cosOmega);

		// Compute the angle from its sin and cosine

		float omega = atan2(sinOmega, cosOmega);

		// Compute inverse of denominator, so we only have
		// to divide once

		float oneOverSinOmega = 1.0f / sinOmega;

		// Compute interpolation parameters

		k0 = sin((1.0f - t) * omega) * oneOverSinOmega;
		k1 = sin(t * omega) * oneOverSinOmega;
	}

	// Interpolate

	
	result.x = k0*q0.x + k1*q1x;
	result.y = k0*q0.y + k1*q1y;
	result.z = k0*q0.z + k1*q1z;
	result.w = k0*q0.w + k1*q1w;

	// Return it

	return result;
}

Ogre::Quaternion calTempQuaternion(const Ogre::Quaternion &q0, const Ogre::Quaternion &q1,const Ogre::Quaternion &q2)
{
//	q0 = q0.normalise();
//	q1 = q1.normalise();
//	q2 = q2.normalise();
	//q0 = calNormalQuaternion(q0);
//	q1 = calNormalQuaternion(q1);
	//q2 = calNormalQuaternion(q2);
	Ogre::Quaternion result;
	Ogre::Quaternion tempQ, temp1, temp2;
	tempQ.w	= q1.w;
	tempQ.x = -q1.x;
	tempQ.y = -q1.y;
	tempQ.z = -q1.z;
/*
//	temp1 = calMulQuaternion(tempQ,q0);
	double temp1Angle = acos(temp1.w);
	double temp1x = temp1Angle * temp1.x;
	double temp1y = temp1Angle * temp1.y;
	double temp1z = temp1Angle * temp1.z;
	temp2 = calMulQuaternion(tempQ,q2);
	double temp2Angle = acos(temp2.w);
	double temp2x = temp2Angle * temp2.x;
	double temp2y = temp2Angle * temp2.y;
	double temp2z = temp2Angle * temp2.z;

	double tx = temp1x + temp2x;
	double ty = temp1y + temp2y;
	double tz = temp1z + temp2z;*/

	return tempQ;//calMulQuaternion(q1,calMulQuaternion(.Log()+.Log()).Exp());
}

Ogre::Quaternion calMulQuaternion(const Ogre::Quaternion &q0, const Ogre::Quaternion &q1)
{
	Ogre::Quaternion result;
	result.w = q0.w*q1.w - (q0.x*q1.x + q0.y*q1.y + q0.z*q1.z);
	result.x = q0.y*q1.z - q0.z*q1.y + q0.w*q1.x + q0.x*q1.w;
	result.y = q0.z*q1.x - q0.x*q1.z + q0.w*q1.y + q0.y*q1.w;
	result.z = q0.x*q1.y - q0.y*q1.x + q0.w*q1.z + q0.z*q1.w;
	return result;
}

Ogre::Quaternion calNormalQuaternion(Ogre::Quaternion &q)
{
	Ogre::Quaternion result;
	double length = (q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
	result.w = q.w/length;
	result.x = q.x/length;
	result.y = q.y/length;
	result.z = q.z/length;
	return result;
}

Ogre::Quaternion MotionGraphsPanel::getRelativeQuaternion(const std::vector<double> & frame, std::string boneFName)
{
	int i;
	EulerAngle euler;
	Ogre::Quaternion result;
	for (i = 0; i < NUMBONE; i++)
	{
		if (boneFName == boneName[i])
			break;
	}
	switch(i)
	{
	case 0:
		euler.m_fRoll = frame[3];
		euler.m_fPitch = frame[4];
		euler.m_fYaw = frame[5];
		break;
	case 1:
		euler.m_fRoll = frame[6];
		euler.m_fPitch = frame[7];
		euler.m_fYaw = frame[8];
		break;
	case 2:
		euler.m_fRoll = frame[9];
		euler.m_fPitch = frame[10];
		euler.m_fYaw = frame[11];
		break;
	case 3:
		euler.m_fRoll = frame[12];
		euler.m_fPitch = frame[13];
		euler.m_fYaw = frame[14];
		break;
	case 4:
		euler.m_fRoll = frame[15];
		euler.m_fPitch = frame[16];
		euler.m_fYaw = frame[17];
		break;
	case 5:
		euler.m_fRoll = frame[18];
		euler.m_fPitch = frame[19];
		euler.m_fYaw = frame[20];
		break;
	case 6:
		euler.m_fRoll = frame[21];
		euler.m_fPitch = frame[22];
		euler.m_fYaw = frame[23];
		break;
	case 7:
		euler.m_fRoll = frame[24];
		euler.m_fPitch = frame[25];
		euler.m_fYaw = frame[26];
		break;
	case 8:
		euler.m_fRoll = frame[27];
		euler.m_fPitch = 0.0;
		euler.m_fYaw = 0.0;
		break;
	case 9:
		euler.m_fRoll = 0.0;
		euler.m_fPitch = frame[28];
		euler.m_fYaw = 0.0;
		break;
	case 10:
		euler.m_fRoll = frame[29];
		euler.m_fPitch = frame[30];
		euler.m_fYaw = frame[31];
		break;
	case 11:
		euler.m_fRoll = frame[32];
		euler.m_fPitch = 0.0;
		euler.m_fYaw = 0.0;
		break;
	case 12:
		euler.m_fRoll = 0.0;
		euler.m_fPitch = frame[33];
		euler.m_fYaw = 0.0;
		break;
	case 13:
		euler.m_fRoll = frame[34];
		euler.m_fPitch = frame[35];
		euler.m_fYaw = frame[36];
		break;
	case 14:
		euler.m_fRoll = frame[37];
		euler.m_fPitch = 0.0;
		euler.m_fYaw = 0.0;
		break;
	case 15:
		euler.m_fRoll = frame[38];
		euler.m_fPitch = 0.0;
		euler.m_fYaw = frame[39];
		break;
	case 16:
		euler.m_fRoll = frame[40];
		euler.m_fPitch = frame[41];
		euler.m_fYaw = frame[42];
		break;
	case 17:
		euler.m_fRoll = frame[43];
		euler.m_fPitch = 0.0;
		euler.m_fYaw = 0.0;
		break;
	case 18:
		euler.m_fRoll = frame[44];
		euler.m_fPitch = 0.0;
		euler.m_fYaw = frame[45];
		break;
	}
	result = EulerAngleToQuaternion(euler);
	return result;
}

Ogre::Vector4 MotionGraphsPanel::calMul(Ogre::Matrix4 &mat, Ogre::Vector4 &vec)
{
	Ogre::Vector4 ret;
	ret[0] = mat[0][0]*vec[0] + mat[0][1]*vec[1] + mat[0][2]*vec[2] + mat[0][3]*vec[3];
	ret[1] = mat[1][0]*vec[0] + mat[1][1]*vec[1] + mat[1][2]*vec[2] + mat[1][3]*vec[3];
	ret[2] = mat[2][0]*vec[0] + mat[2][1]*vec[1] + mat[2][2]*vec[2] + mat[2][3]*vec[3];
	ret[3] = mat[3][0]*vec[0] + mat[3][1]*vec[1] + mat[3][2]*vec[2] + mat[3][3]*vec[3];

	return ret;
}

double MotionGraphsPanel::getBoneTotalWeight()
{
	std::size_t i;
	double sum = 0.0;
	for (i = 0; i < NUMBONE; i++)
	{
		sum += boneWeight[i];
	}
	return sum;
}

void MotionGraphsPanel::windowResized(Ogre::RenderWindow* rw)
{
	mMainWidget->setPosition(0,AppDemo::getSingletonPtr()->mRenderWnd->getHeight() - mMainWidget->getHeight());
}

Ogre::Quaternion MotionGraphsPanel::EulerAngleToQuaternion(const EulerAngle &EA)
{
	double halfPhi   = EA.m_fRoll * M_PI /360;
	double halfTheta = EA.m_fPitch * M_PI /360;
	double halfPsi   = EA.m_fYaw * M_PI /360;
	double cosPhi    = cos(halfPhi);
	double sinPhi    = sin(halfPhi);
	double cosTheta  = cos(halfTheta);
	double sinTheta  = sin(halfTheta);
	double cosPsi    = cos(halfPsi);
	double sinPsi    = sin(halfPsi);

	Ogre::Quaternion Q;
	Q.w = (cosPhi * cosTheta * cosPsi) + (sinPhi * sinTheta * sinPsi);
	Q.x = (sinPhi * cosTheta * cosPsi) - (cosPhi * sinTheta * sinPsi);
	Q.y = (cosPhi * sinTheta * cosPsi) + (sinPhi * cosTheta * sinPsi);
	Q.z = (cosPhi * cosTheta * sinPsi) - (sinPhi * sinTheta * cosPsi);
	return Q;
}

EulerAngle MotionGraphsPanel::QuaternionToEulerAngle(Ogre::Quaternion &q)
{
	EulerAngle ea;
	ea.m_fRoll  = atan2(2 * (q.w * q.x + q.y * q.z) , 1 - 2 * (q.x * q.x + q.y * q.y)) * 180/M_PI;
	ea.m_fPitch = asin(2 * (q.w * q.y - q.x * q.z)) * 180/M_PI; 
	ea.m_fYaw   = atan2(2 * (q.w * q.z + q.x * q.y) , 1 - 2 * (q.y * q.y + q.z * q.z)) *180/M_PI;

	return ea;
}

const std::size_t MotionGraphsPanel::returnCountRealIndex(const std::size_t index)  //ccount ->  index
{

	std::size_t  i;
	for (i = 0; i < realIndex; i++)
	{
		if (index == realNode[i].ccount)
		{
			return i;
		}
	}
	return -1;
}

const std::size_t MotionGraphsPanel::returnRealIndex(const std::size_t index)   //positionz  -> index
{
	std::size_t  i;
	for (i = 0; i < realIndex; i++)
	{
		if (index == realNode[i].positionZ)
		{
			return i;
		}
	}
	return -1;
}

const std::size_t MotionGraphsPanel::returnIndex(const std::size_t index)
{
	std::size_t  i;
	for (i = 0; i < transIndex; i++)
	{
		if (index == transNode[i])
		{
			return i;
		}
	}
	return -1;
}

const int MotionGraphsPanel::returnMap(int index)
{
	int  i;
	for (i = 0; i < newMotionIndex; i++)
	{
		if (index == newMotionMap[i])
		{
			return i;
		}
	}
	return -1;
}

int MotionGraphsPanel::returnFrontCount(int molistnum,int framenum)
{
	int temList = 0, temFrame = 0;
	for (int i = 0; i < realIndex; i++)
	{
		temList = realNode[i].positionZ/10000;
		temFrame = realNode[i].positionZ%10000;
		if (temList == molistnum)
		{
			if (temFrame >= framenum && mMotionGraphs.nodeIn(realNode[i].ccount))
			{
				return realNode[i].ccount;
			}
		}  
	}
	return -1;
}

int MotionGraphsPanel::returnBackCount(int molistnum,int framenum)
{
	int temList = 0, temFrame = 0;
	int perget = 0, nowget = 0;
	for (int i = 0; i < realIndex; i++)
	{
		temList = realNode[i].positionZ/10000;
		temFrame = realNode[i].positionZ%10000;
		if (temList == molistnum)
		{
			if (temFrame <= framenum && mMotionGraphs.nodeIn(realNode[i].ccount))
			{
				nowget = realNode[i].ccount;
			}
		}  
		perget = nowget;
	}
	if (perget)
	{
		return perget;
	}
	return -1;
}

//----------------------------------------------------------------------
/*Author ruan.answer
  Date   4.1 - 4.15
  Des   界面点击运动展示
*/
//----------------------------------------------------------------------
void MotionGraphsPanel::goRandom(const MyGUI::UString& commandName, bool& result)
{
#ifdef KNOW_TIME
	// cal the time of generate 2000 frame motion  
	clock_t startTime, finishTime;
	startTime = clock();
#endif
		//mMotionGraphs.randomPlay(mMotionGraphs.tempNode[0]);
#ifdef FOR_DEBUG
		for (int i = 0; i < mMotionGraphs.pathPlayCount; i++)
		{
			cout << ":" <<mMotionGraphs.pathPlay[i];
		}
#endif

		Eigen::MatrixXd  data(FRAME_NUM_FOR_PLAY+1, 46);
		int ccData = 0;
		int pre = 0, now = 0;	
		int motionListNum1 = 0, motionFrame1 = 0;
		int motionListNum2 = 0, motionFrame2 = 0;
		Motion *motion;
		bool flag = false;
		double i_X = 0.0, i_Y = 0.0, i_Z = 0.0;
		double s_X = 0.0, s_Y = 0.0, s_Z = 0.0;
		mMotionGraphs.randomPlay(mMotionGraphs.tempNode[0]);
		for (int i = 0; i < mMotionGraphs.pathPlayCount - 1; i++)
		{
			motionListNum1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ / 10000;
			motionFrame1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ % 10000;
			motionListNum2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ / 10000;
			motionFrame2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ % 10000;

			motion = getMotion(MotionList.at(motionListNum1)->getName());

			if (motionListNum1 == motionListNum2)
			{
				if (motionFrame1 < motionFrame2) //前面节点到后面节点  直接用原始运动
				{	
					if(pre == -1)   //原始运动是插出来的
						motionFrame1 = motionFrame1 + WINDOW_M_IN;
					now = 1;

					for (int j = motionFrame1; j <= motionFrame2; j++)
					{
						vector<double> temp = motion->getFrame(j);
						if (ccData == 0)
						{
							i_X = temp[0];
							i_Y = temp[1];
							i_Z = temp[2];
						}
						for (int k = 0; k < 46; k++)
						{
							data(ccData,k) = temp[k];
						}
						if (j == motionFrame1 && ccData > 1)
						{
							data(ccData, 0) = 2 * data(ccData-1, 0) - data(ccData-2, 0);
							data(ccData, 1) = 2 * data(ccData-1, 1) - data(ccData-2, 1);
							data(ccData, 2) = 2 * data(ccData-1, 2) - data(ccData-2, 2);
						}
						else 
						{
							if (ccData > 0)
							{
								s_X = temp[0] - i_X;			
								s_Y = temp[1] - i_Y;
								s_Z = temp[2] - i_Z;
						
								data(ccData, 0) = s_X + data(ccData-1, 0);
								data(ccData, 1) = s_Y + data(ccData-1, 1);
								data(ccData, 2) = s_Z + data(ccData-1, 2);
							}
							else
							{
								s_X = temp[0] - i_X;			
								s_Y = temp[1] - i_Y;
								s_Z = temp[2] - i_Z;

								data(ccData, 0) = s_X + i_X;
								data(ccData, 1) = s_Y + i_Y;
								data(ccData, 2) = s_Z + i_Z;
							}
						}
#ifdef FOR_DEBUG
						cout <<ccData << ":"<< endl;
						cout<<"temp" << temp[0] <<":" << temp[1] <<":" <<temp[2] <<endl;
						cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
						i_X = temp[0];
						i_Y = temp[1];
						i_Z = temp[2];

						ccData++;
						if (ccData > FRAME_NUM_FOR_PLAY)
						{
							flag = true;
							break;
						}
					}
				}
				else    //后面节点到前面节点  用内部插值数据
				{
					if (pre == 1)
					{
						ccData = ccData - WINDOW_M_IN;
					}
					now = -1;

					int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
					tem = returnMap(tem);
				//	cout << "interdata tem:" << tem << endl;
					if (tem == -1)
					{
						printf("random has error, no data\n");
						break;
					}
					Eigen::MatrixXd interDate(newMotionList[tem]->getTotalFrameNum(), 46);
					interDate = newMotionList[tem]->getMotionData();
					//cout << interDate;
					for (int j = 0; j < interDate.rows(); j++)
					{
						if (ccData == 0)
						{
							i_X = interDate(j,0);
							i_Y = interDate(j,1);
							i_Z = interDate(j,2);
						}
						for (int k = 0; k < 46; k++)
						{
							data(ccData,k) =  interDate(j,k);
						}
						if (ccData > 0)
						{
							data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
							data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
							data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
						}						
						else
						{
							data(ccData, 0) = i_X;
							data(ccData, 1) = i_Y;
							data(ccData, 2) = i_Z;
						}
#ifdef FOR_DEBUG
						cout <<ccData << ":"<< endl;
						cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
						cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
						ccData++;
						if (ccData > FRAME_NUM_FOR_PLAY)
						{
							flag = true;
							break;
						}
					}				
				}
			}
			else   //不同运动序列      直接插值  
			{
				if (pre == 1)
				{
					ccData = ccData - WINDOW_M_OUT;
				}
				now = -1;

				int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
				tem = returnMap(tem);
				if (tem == -1)
				{
					printf("radom has error, no data\n");
					break;
				}
				Eigen::MatrixXd interDate(newMotionList[tem]->getFreedomNum(), 46);
				interDate = newMotionList[tem]->getMotionData();
				for (int j = 0; j < interDate.rows(); j++)
				{
					if (ccData == 0)
					{
						i_X = interDate(j,0);
						i_Y = interDate(j,1);
						i_Z = interDate(j,2);
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) =  interDate(j,k);
					}
					if (ccData > 0)
					{
						data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
						data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
						data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
					}						
					else
					{
						data(ccData, 0) = i_X;
						data(ccData, 1) = i_Y;
						data(ccData, 2) = i_Z;
					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}
			}
			if (flag)
			{
				break;
			}
			pre = now;
		}
		mMotion = new Motion(data,"random");
#ifdef KNOW_TIME
		finishTime = clock();
		cout << "the time of generate 2000 frames motion:" << (double)(finishTime - startTime) / CLOCKS_PER_SEC << "seconds" << endl;
#endif

#ifdef FOR_PLAY
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
#else
		mMotion->writeToFile("random.dat");
#endif
}

void MotionGraphsPanel::goTwoAttribute(const MyGUI::UString& commandName, bool& result)
{
	Eigen::MatrixXd  data(FRAME_NUM_FOR_PLAY+1, 46);

	int ccData = 0;
	int pre = 0, now = 0;
	int motionListNum1 = 0, motionFrame1 = 0;
	int motionListNum2 = 0, motionFrame2 = 0;
	double i_X = 0.0, i_Y = 0.0, i_Z = 0.0;
	double s_X = 0.0, s_Y = 0.0, s_Z = 0.0;
	Motion *motion;
	bool flag = false;
	int srccMolist, srccFrame, desttMolist, desttFrame, temp, tranTemp[1000], tra = 0;
	int srcc = 0, destt = 0;
	while(scanf("%d %d %d %d",&srccMolist,&srccFrame,&desttMolist,&desttFrame) == 4)
	{
		srcc = returnFrontCount(srccMolist,srccFrame);
		destt = returnBackCount(desttMolist,desttFrame);
		if (srcc == -1 ||  destt == -1)
		{
			cout << "input is error. or can't find this path" << endl;
		}
		else
			break;
	}

	if (srcc != -1)
	{
		int tery = returnCountRealIndex(srcc);
		temp = realNode[tery].positionZ%10000;
		motion = getMotion(MotionList.at(realNode[tery].positionZ/10000)->getName());
		for (int i = srccFrame; i < temp; i++)
		{
			vector<double> test = motion->getFrame(i);
			i_X = test[0];
			i_Y = test[1];
			i_Z = test[2];
			for (int j = 0; j < 46; j++)
				data(ccData, j) = test[j];
			if (ccData > 0)
			{
				s_X = test[0] - i_X;
				s_Y = test[1] - i_Y;
				s_Z = test[2] - i_Z;

				data(ccData, 0) = s_X + data(ccData-1, 0);
				data(ccData, 1) = s_Y + data(ccData-1, 1);
				data(ccData, 2) = s_Z + data(ccData-1, 2);
			}
			else
			{
				s_X = test[0] - i_X;			
				s_Y = test[1] - i_Y;
				s_Z = test[2] - i_Z;

				data(ccData, 0) = s_X + i_X;
				data(ccData, 1) = s_Y + i_Y;
				data(ccData, 2) = s_Z + i_Z;				
			}
			i_X = test[0];
			i_Y = test[1];
			i_Z = test[2];

			ccData++;
			if (ccData > FRAME_NUM_FOR_PLAY)
				break;
		}
		now = 1;
		pre = now;
	}

	mMotionGraphs.dijkstra(realIndex, srcc);
	cout << "distance" << mMotionGraphs.dist[destt] << endl;
	temp = destt;
	mMotionGraphs.pathPlayCount = 0;
	memset(mMotionGraphs.pathPlay, 0 ,sizeof(mMotionGraphs.pathPlay));
	tranTemp[tra++] = destt;
	while (mMotionGraphs.prevv[temp] != -1)
	{
		//cout << ":" << mMotionGraphs.prevv[temp];
		temp = mMotionGraphs.prevv[temp];
		tranTemp[tra++] = temp;
	}
	while(tra>0)
	{
		mMotionGraphs.pathPlay[mMotionGraphs.pathPlayCount++] = tranTemp[tra-1];
		tra--;
	}
#ifdef FOR_DEBUG
	for(int i = 0; i < mMotionGraphs.pathPlayCount; i++)
	{
		cout << ":" << mMotionGraphs.pathPlay[i];
	}
#endif

	for (int i = 0; i < mMotionGraphs.pathPlayCount - 1; i++)
	{
		motionListNum1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ / 10000;
		motionFrame1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ % 10000;
		motionListNum2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ / 10000;
		motionFrame2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ % 10000;

		motion = getMotion(MotionList.at(motionListNum1)->getName());

		if (motionListNum1 == motionListNum2)
		{
			if (motionFrame1 < motionFrame2) //前面节点到后面节点  直接用原始运动
			{	
				if(pre == -1)   //原始运动是插出来的
					motionFrame1 = motionFrame1 + WINDOW_M_IN;
				now = 1;

				for (int j = motionFrame1; j <= motionFrame2; j++)
				{
					vector<double> temp = motion->getFrame(j);
					if (ccData == 0)
					{
						i_X = temp[0];
						i_Y = temp[1];
						i_Z = temp[2];
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) = temp[k];
					}
					if (j == motionFrame1 && ccData > 1)
					{
						data(ccData, 0) = 2 * data(ccData-1, 0) - data(ccData-2, 0);
						data(ccData, 1) = 2 * data(ccData-1, 1) - data(ccData-2, 1);
						data(ccData, 2) = 2 * data(ccData-1, 2) - data(ccData-2, 2);
					}
					else 
					{
						if (ccData > 0)
						{
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;

							data(ccData, 0) = s_X + data(ccData-1, 0);
							data(ccData, 1) = s_Y + data(ccData-1, 1);
							data(ccData, 2) = s_Z + data(ccData-1, 2);
						}
						else
						{
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;

							data(ccData, 0) = s_X + i_X;
							data(ccData, 1) = s_Y + i_Y;
							data(ccData, 2) = s_Z + i_Z;
						}
					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"temp" << temp[0] <<":" << temp[1] <<":" <<temp[2] <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif

					i_X = temp[0];
					i_Y = temp[1];
					i_Z = temp[2];

					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}
			}
			else    //后面节点到前面节点  用内部插值数据
			{
				if (pre == 1)
				{
					ccData = ccData - WINDOW_M_IN;
				}
				now = -1;

				int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
				tem = returnMap(tem);
				//	cout << "interdata tem:" << tem << endl;
				if (tem == -1)
				{
					printf("random has error, no data\n");
					break;
				}
				Eigen::MatrixXd interDate(newMotionList[tem]->getTotalFrameNum(), 46);
				interDate = newMotionList[tem]->getMotionData();
				//cout << interDate;
				for (int j = 0; j < interDate.rows(); j++)
				{
					if (ccData == 0)
					{
						i_X = interDate(j,0);
						i_Y = interDate(j,1);
						i_Z = interDate(j,2);
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) =  interDate(j,k);
					}
					if (ccData > 0)
					{
						data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
						data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
						data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
					}						
					else
					{
						data(ccData, 0) = i_X;
						data(ccData, 1) = i_Y;
						data(ccData, 2) = i_Z;
					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}				
			}
		}
		else   //不同运动序列      直接插值  
		{
			if (pre == 1)
			{
				ccData = ccData - WINDOW_M_OUT;
			}
			now = -1;

			int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
			tem = returnMap(tem);
			if (tem == -1)
			{
				printf("radom has error, no data\n");
				break;
			}
			Eigen::MatrixXd interDate(newMotionList[tem]->getFreedomNum(), 46);
			interDate = newMotionList[tem]->getMotionData();
			for (int j = 0; j < interDate.rows(); j++)
			{
				if (ccData == 0)
				{
					i_X = interDate(j,0);
					i_Y = interDate(j,1);
					i_Z = interDate(j,2);
				}
				for (int k = 0; k < 46; k++)
				{
					data(ccData,k) =  interDate(j,k);
				}
				if (ccData > 0)
				{
					data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
					data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
					data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
				}						
				else
				{
					data(ccData, 0) = i_X;
					data(ccData, 1) = i_Y;
					data(ccData, 2) = i_Z;
				}
#ifdef FOR_DEBUG
				cout <<ccData << ":"<< endl;
				cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
				cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
				ccData++;
				if (ccData > FRAME_NUM_FOR_PLAY)
				{
					flag = true;
					break;
				}
			}
		}
		if (flag)
		{
			break;
		}
		pre = now;
	}

	if (destt != -1)
	{
		temp = returnCountRealIndex(destt);
		temp = realNode[temp].positionZ%10000;
		for (int i = temp; i <= desttFrame; i++)
		{
			vector<double> test = motion->getFrame(i);
			i_X = test[0];
			i_Y = test[1];
			i_Z = test[2];
			for (int j = 0; j < 46; j++)
				data(ccData, j) = test[j];
			if (ccData > 0)
			{
				s_X = test[0] - i_X;
				s_Y = test[1] - i_Y;
				s_Z = test[2] - i_Z;

				data(ccData, 0) = s_X + data(ccData-1, 0);
				data(ccData, 1) = s_Y + data(ccData-1, 1);
				data(ccData, 2) = s_Z + data(ccData-1, 2);
			}
			else
			{
				s_X = test[0] - i_X;			
				s_Y = test[1] - i_Y;
				s_Z = test[2] - i_Z;

				data(ccData, 0) = s_X + i_X;
				data(ccData, 1) = s_Y + i_Y;
				data(ccData, 2) = s_Z + i_Z;				
			}
			i_X = test[0];
			i_Y = test[1];
			i_Z = test[2];

			ccData++;
			if (ccData > FRAME_NUM_FOR_PLAY)
				break;
		}
	}
	Eigen::MatrixXd  tttt(ccData, 46);
	for (int i = 0; i < ccData; i++)
	{
		for (int j =0; j < 46; j++)
		{
			tttt(i,j) = data(i,j);
		}
	}
	mMotion = new Motion(tttt,"twoAttribute");
#ifdef FOR_PLAY
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
#else
	mMotion->writeToFile("twoAttr.dat");
#endif
}

void MotionGraphsPanel::goWhere()
{
	Eigen::MatrixXd  data(FRAME_NUM_FOR_PLAY+1, 46);
	int ccData = 0;
	int pre = 0, now = 0;
	int motionListNum1 = 0, motionFrame1 = 0;
	int motionListNum2 = 0, motionFrame2 = 0;
	Motion *motion;
	bool flag = false;

	mMotionGraphs.randomPlay(mMotionGraphs.tempNode[0]);
	double i_X = 0.0, i_Y = 0.0, i_Z = 0.0;
	double s_X = 0.0, s_Y = 0.0, s_Z = 0.0;
	for (int i = 0; i < mMotionGraphs.pathPlayCount - 1; i++)
	{
		motionListNum1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ / 10000;
		motionFrame1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ % 10000;
		motionListNum2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ / 10000;
		motionFrame2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ % 10000;

		motion = getMotion(MotionList.at(motionListNum1)->getName());

		if (motionListNum1 == motionListNum2)
		{
			if (motionFrame1 < motionFrame2) //前面节点到后面节点  直接用原始运动
			{	
				if(pre == -1)   //原始运动是插出来的
					motionFrame1 = motionFrame1 + WINDOW_M_IN;
				now = 1;

				for (int j = motionFrame1; j <= motionFrame2; j++)
				{
					vector<double> temp = motion->getFrame(j);
					if (ccData == 0)
					{
						if (clickNum == 0)
						{
							i_X = temp1X;
							i_Y = temp[1];
							i_Z = temp1Z;
						}
						else
						{
							i_X = temp[0];
							i_Y = temp[1];
							i_Z = temp[2];
						}
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) = temp[k];
					}
					if (j == motionFrame1 && ccData > 1)
					{
						data(ccData, 0) = 2 * data(ccData-1, 0) - data(ccData-2, 0);
						data(ccData, 1) = 2 * data(ccData-1, 1) - data(ccData-2, 1);
						data(ccData, 2) = 2 * data(ccData-1, 2) - data(ccData-2, 2);
					}
					else 
					{
						if (ccData > 0)
						{
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;

							data(ccData, 0) = s_X + data(ccData-1, 0);
							data(ccData, 1) = s_Y + data(ccData-1, 1);
							data(ccData, 2) = s_Z + data(ccData-1, 2);
						}
						else
						{
							if (clickNum != 0)
							{
								data(ccData, 0) = temp1X;
								data(ccData, 1) = s_Y + i_Y;
								data(ccData, 2) = temp1Z;
							}
							else
							{
								data(ccData, 0) = s_X + i_X;
								data(ccData, 1) = s_Y + i_Y;
								data(ccData, 2) = s_Z + i_Z;
							}
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;
						}
					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"temp" << temp[0] <<":" << temp[1] <<":" <<temp[2] <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif

					i_X = temp[0];
					i_Y = temp[1];
					i_Z = temp[2];

					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}
			}
			else    //后面节点到前面节点  用内部插值数据
			{
				if (pre == 1)
				{
					ccData = ccData - WINDOW_M_IN;
				}
				now = -1;

				int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
				tem = returnMap(tem);
				//	cout << "interdata tem:" << tem << endl;
				if (tem == -1)
				{
					printf("random has error, no data\n");
					break;
				}
				Eigen::MatrixXd interDate(newMotionList[tem]->getTotalFrameNum(), 46);
				interDate = newMotionList[tem]->getMotionData();
				//cout << interDate;
				for (int j = 0; j < interDate.rows(); j++)
				{
					if (ccData == 0)
					{
						if (clickNum == 0)
						{
							i_X = temp1X;
							i_Y = interDate(j,1);
							i_Z = temp1Z;
						}
						else
						{
							i_X = interDate(j,0);
							i_Y = interDate(j,1);
							i_Z = interDate(j,2);
						}
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) =  interDate(j,k);
					}
					if (ccData > 0)
					{
						data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
						data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
						data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
					}						
					else
					{
						if (clickNum == 0)
						{
							data(ccData, 0) = temp1X;
							data(ccData, 1) = i_Y;
							data(ccData, 2) = temp1Z;
						}
						else
						{
							data(ccData, 0) = i_X;
							data(ccData, 1) = i_Y;
							data(ccData, 2) = i_Z;
						}

					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}				
			}
		}
		else   //不同运动序列      直接插值  
		{
			if (pre == 1)
			{
				ccData = ccData - WINDOW_M_OUT;
			}
			now = -1;

			int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
			tem = returnMap(tem);
			if (tem == -1)
			{
				printf("radom has error, no data\n");
				break;
			}
			Eigen::MatrixXd interDate(newMotionList[tem]->getFreedomNum(), 46);
			interDate = newMotionList[tem]->getMotionData();
			for (int j = 0; j < interDate.rows(); j++)
			{
				if (ccData == 0)
				{
					if (clickNum == 0)
					{
						i_X = temp1X;
						i_Y = interDate(j,1);
						i_Z = temp1Z;
					}
					else	
					{
						i_X = interDate(j,0);
						i_Y = interDate(j,1);
						i_Z = interDate(j,2);
					}

				}
				for (int k = 0; k < 46; k++)
				{
					data(ccData,k) =  interDate(j,k);
				}
				if (ccData > 0)
				{
					data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
					data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
					data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
				}						
				else
				{
					if (clickNum == 0)
					{
						data(ccData, 0) = temp1X;
						data(ccData, 1) = i_Y;
						data(ccData, 2) = temp1Z;
					}
					else
					{
						data(ccData, 0) = i_X;
						data(ccData, 1) = i_Y;
						data(ccData, 2) = i_Z;
					}
				}
#ifdef FOR_DEBUG
				cout <<ccData << ":"<< endl;
				cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
				cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
				ccData++;
				if (ccData > FRAME_NUM_FOR_PLAY)
				{
					flag = true;
					break;
				}
			}
		}
		if (flag)
		{
			break;
		}
		pre = now;
	}

	//沿弧插值   要注意角色是向左还是向右
	//double initK = 0.0, doubleK = 0.0;
	double circleX = 0.0, circleZ = 0.0, circleR2 = 0.0, circleR = 0.0;
	double temp = 0.0, aziSD = 0.0, aziCS = 0.0;    //圆心弧度   起点到终点方位角  圆心到起点方位角
	double distSD = 0.0;
	double perAngle = 0.0;
	double per_X[FRAME_NUM_FOR_PLAY+1], per_Z[FRAME_NUM_FOR_PLAY+1];
	double abs_X = 0.0, abs_Z = 0.0;
	if (clickNum == 0)
	{
		temp1X = data(0,0), temp1Z = data(0,2);
	}

	//double temp2X, temp2Z;
	if (clickNum == 0)
	{
		temp2X = data(10, 0);
		temp2Z = data(10, 2);
	}


	distSD = sqrt((destX-temp1X)*(destX-temp1X)+(destZ-temp1Z)*(destZ-temp1Z));		
	if (abs(temp2X - temp1X) < 0.00001)
		initK = 10000.0f;
	else
		initK = (temp2Z - temp1Z)/(temp2X - temp1X);
	if(abs(destX - temp1X) < 0.00001)
		doubleK = 10000.0f;
	else
		doubleK = (destZ - temp1Z)/(destX - temp1X);
	if (abs(initK) < 10001.0f && abs(doubleK) < 10001.0f)
	{
		circleX = (temp1Z/2 - destZ/2 + temp1X/initK - (temp1X + destX)/(2.0*doubleK))/(1/initK-1/doubleK);
		circleZ = -(circleX - temp1X)/initK + temp1Z;
	}
	else if (abs(initK - 10000.0f) < 0.0000001f)
	{
		circleX = (temp1X + temp2X)/2.0 - doubleK*(temp2Z - temp1Z)/2.0;
		circleZ = temp1Z;
	}
	else if (abs(doubleK - 10000.0f) < 0.000001f)
	{
		circleX = temp1X - initK*(temp2Z - temp1Z)/2.0;
		circleZ = (temp1Z + temp2Z)/2.0;
	}
	circleR2 = (temp1X-circleX)*(temp1X-circleX) + (temp1Z - circleZ)*(temp1Z - circleZ);
	circleR = sqrt(circleR2);
	temp = (circleR2*2.0 - distSD*distSD)/(2.0*circleR2);
	temp = acos(temp);

	aziCS = asin((temp1X - circleX)/circleR);
	perAngle = temp/(ccData-1); 

	abs_X = circleX + circleR*sin(aziCS);
	abs_Z = circleZ + circleR*cos(aziCS);
	per_X[0] = 0.0;
	per_Z[0] = 0.0;

	int totalFrame = (distSD/0.21);//0.24);
	if (totalFrame > ccData)
	{
		printf(	"frameNumforplay is small\n");
	}

	Eigen::MatrixXd  tttt(totalFrame, 46);

	double sign = (temp1X-destX)*(temp2Z-destZ)-(temp1Z-destZ)*(temp2X-destX);
	if (sign < 0)
	{
		for (int i = 1; i < ccData;i++)
		{
			per_X[i] = circleX + circleR*sin(aziCS + i*perAngle) - abs_X;		//*M_PI/180
			per_Z[i] = circleZ + circleR*cos(aziCS + i*perAngle) - abs_Z;
			if (i > 0)
			{
				abs_X = circleX + circleR*sin(aziCS + i*perAngle*M_PI/180.0);
				abs_Z = circleZ + circleR*cos(aziCS + i*perAngle*M_PI/180.0);
			}	
		} 
	}
	else
	{
		for (int i = 1; i < ccData;i++)
		{
			per_X[i] = circleX + circleR*sin(aziCS + i*perAngle) - abs_X;		//*M_PI/180
			per_Z[i] = circleZ + circleR*cos(aziCS + i*perAngle) - abs_Z;
			if (i > 0)
			{
				abs_X = circleX + circleR*sin(aziCS + i*perAngle*M_PI/180.0);
				abs_Z = circleZ + circleR*cos(aziCS + i*perAngle*M_PI/180.0);
			}	
		} 
	}
	perAngle = perAngle*180.0/M_PI;
	if(sign > 0)
		perAngle = -perAngle;
	for (int i = 1; i <= totalFrame; i++)
	{
		data(i, 0) = data(i, 0) + per_X[i-1];
		data(i, 2) = data(i, 2) + per_Z[i-1];
		data(i, 4) = (data(i, 4) + i*perAngle);
	}

	for (int i = 0; i < totalFrame; i++)
	{
		for (int j =0; j < 46; j++)
		{
			tttt(i,j) = data(i,j);
		}
	}

	clickNum++;
	temp1X = data(totalFrame-1, 0);
	temp1Z = data(totalFrame-1, 2);
	temp2X = data(totalFrame-10, 0);
	temp2Z = data(totalFrame-10, 2);

	mMotion = new Motion(tttt,"gosomewhere");
#ifdef FOR_PLAY
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
#else
	mMotion->writeToFile("gosomewhere.dat");
#endif
}

void MotionGraphsPanel::goSomeWhere(const MyGUI::UString& commandName, bool& result)
{	
	if (pathWhereCount == 1)
	{
		goWhere();
	}
	else
		pathWhere();
/*
	mMotionGraphs.doIt();
	if (!playPath)
	{
		playPath = mEditor->createPlayPPath(mMotionGraphs.tempPpath, mMotionGraphs.tempPpCount);
	}
	else
		playPath->setVisbility(!playPath->getVisbility());*/
} 

void MotionGraphsPanel::collectPoint(const MyGUI::UString& commandName, bool& result)
{
	enablePoint = !enablePoint;
}

void MotionGraphsPanel::playMotion(const MyGUI::UString& commandName, bool& result)
{
	//先一个运动跑起来
	//1、随机漫游  
	//2、连接两个姿势
	//3、点击屏幕中一点  角色向该点运动
	//4、沿着指定轨迹的运动
	//5、在图中画区域 某一段区域内执行指定的动作 
	//6、自智能  后面再说！！还有建场景树 A*  碰撞检查等等 

	Eigen::MatrixXd  data(FRAME_NUM_FOR_PLAY+1, 46);
	int ccData = 0;
	int pre = 0, now = 0;	
	int motionListNum1 = 0, motionFrame1 = 0;
	int motionListNum2 = 0, motionFrame2 = 0;
	Motion *motion;
	bool flag = false;
	double i_X = 0.0, i_Y = 0.0, i_Z = 0.0;
	double s_X = 0.0, s_Y = 0.0, s_Z = 0.0;
	mMotionGraphs.randomPlay(mMotionGraphs.tempNode[0]);
	for (int i = 0; i < mMotionGraphs.pathPlayCount - 1; i++)
	{
		motionListNum1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ / 10000;
		motionFrame1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ % 10000;
		motionListNum2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ / 10000;
		motionFrame2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ % 10000;

		motion = getMotion(MotionList.at(motionListNum1)->getName());

		if (motionListNum1 == motionListNum2)
		{
			if (motionFrame1 < motionFrame2) //前面节点到后面节点  直接用原始运动
			{	
				if(pre == -1)   //原始运动是插出来的
					motionFrame1 = motionFrame1 + WINDOW_M_IN;
				now = 1;

				for (int j = motionFrame1; j <= motionFrame2; j++)
				{
					vector<double> temp = motion->getFrame(j);
					if (ccData == 0)
					{
						i_X = temp[0];
						i_Y = temp[1];
						i_Z = temp[2];
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) = temp[k];
					}
					if (j == motionFrame1 && ccData > 1)
					{
						data(ccData, 0) = 2 * data(ccData-1, 0) - data(ccData-2, 0);
						data(ccData, 1) = 2 * data(ccData-1, 1) - data(ccData-2, 1);
						data(ccData, 2) = 2 * data(ccData-1, 2) - data(ccData-2, 2);
					}
					else 
					{
						if (ccData > 0)
						{
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;

							data(ccData, 0) = s_X + data(ccData-1, 0);
							data(ccData, 1) = s_Y + data(ccData-1, 1);
							data(ccData, 2) = s_Z + data(ccData-1, 2);
						}
						else
						{
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;

							data(ccData, 0) = s_X + i_X;
							data(ccData, 1) = s_Y + i_Y;
							data(ccData, 2) = s_Z + i_Z;
						}
					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"temp" << temp[0] <<":" << temp[1] <<":" <<temp[2] <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
					i_X = temp[0];
					i_Y = temp[1];
					i_Z = temp[2];

					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}
			}
			else    //后面节点到前面节点  用内部插值数据
			{
				if (pre == 1)
				{
					ccData = ccData - WINDOW_M_IN;
				}
				now = -1;

				int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
				tem = returnMap(tem);
				//cout << "interdata tem:" << tem << endl;
				if (tem == -1)
				{
					printf("random has error, no data\n");
					break;
				}
				Eigen::MatrixXd interDate(newMotionList[tem]->getTotalFrameNum(), 46);
				interDate = newMotionList[tem]->getMotionData();
				//cout << interDate;
				for (int j = 0; j < interDate.rows(); j++)
				{
					if (ccData == 0)
					{
						i_X = interDate(j,0);
						i_Y = interDate(j,1);
						i_Z = interDate(j,2);
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) =  interDate(j,k);
					}
					if (ccData > 0)
					{
						data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
						data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
						data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
					}						
					else
					{
						data(ccData, 0) = i_X;
						data(ccData, 1) = i_Y;
						data(ccData, 2) = i_Z;
					}
#ifdef FOR_DEBUG
						cout <<ccData << ":"<< endl;
						cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
						cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}				
			}
		}
		else   //不同运动序列      直接插值  
		{
			if (pre == 1)
			{
				ccData = ccData - WINDOW_M_OUT;
			}
			now = -1;

			int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
			tem = returnMap(tem);
			if (tem == -1)
			{
				printf("radom has error, no data\n");
				break;
			}
			Eigen::MatrixXd interDate(newMotionList[tem]->getFreedomNum(), 46);
			interDate = newMotionList[tem]->getMotionData();
			for (int j = 0; j < interDate.rows(); j++)
			{
				if (ccData == 0)
				{
					i_X = interDate(j,0);
					i_Y = interDate(j,1);
					i_Z = interDate(j,2);
				}
				for (int k = 0; k < 46; k++)
				{
					data(ccData,k) =  interDate(j,k);
				}
				if (ccData > 0)
				{
					data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
					data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
					data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
				}						
				else
				{
					data(ccData, 0) = i_X;
					data(ccData, 1) = i_Y;
					data(ccData, 2) = i_Z;
				}
#ifdef FOR_DEBUG
				cout <<ccData << ":"<< endl;
				cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
				cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
				ccData++;
				if (ccData > FRAME_NUM_FOR_PLAY)
				{
					flag = true;
					break;
				}
			}
		}
		if (flag)
		{
			break;
		}
		pre = now;
	}

	mMotionGraphs.doIt();
//	for(int i = 0; i < mMotionGraphs.tempPpCount; i++)
//		cout << mMotionGraphs.tempPpath[i].x << ":" << mMotionGraphs.tempPpath[i].y << endl;

/*
	if (!playPath)
	{
		playPath = mEditor->createPlayPath(mMotionGraphs.tempPpath, mMotionGraphs.tempPpCount);
	}
	else
		playPath->setVisbility(!playPath->getVisbility());*/


	double add = 0.0;

	Eigen::MatrixXd  tttt(mMotionGraphs.tempPpCount, 46);

	for (int i = 0; i < mMotionGraphs.tempPpCount; i++)
	{
		for (int j =0; j < 46; j++)
		{
			tttt(i,j) = data(i,j);
		}
	}


	//if (mMotionGraphs.tempPp[0].x == mMotionGraphs.tempPp[1].x && mMotionGraphs.tempPp[0].y < mMotionGraphs.tempPp[1].y)  
	//	add -= 90;
	if (mMotionGraphs.tempPp[0].x == mMotionGraphs.tempPp[1].x && mMotionGraphs.tempPp[0].y > mMotionGraphs.tempPp[1].y)
		add += 180;
	else if (mMotionGraphs.tempPp[0].y == mMotionGraphs.tempPp[1].y && mMotionGraphs.tempPp[0].x > mMotionGraphs.tempPp[1].x) 
		add -= 90;
	else if (mMotionGraphs.tempPp[0].y == mMotionGraphs.tempPp[1].y && mMotionGraphs.tempPp[0].x < mMotionGraphs.tempPp[1].x)
		add += 90;

	double sign = 0.0;
	double addAngle = (180*0.24/LENGTH_TO_CUT)/M_PI;
	for (int i = 0; i < mMotionGraphs.tempPpCount-2; i++)
	{
		tttt(i, 0) = mMotionGraphs.tempPpath[i].x;
		tttt(i, 2) = mMotionGraphs.tempPpath[i].y;
#ifdef FOR_DEBUG
		cout << min(abs(mMotionGraphs.tempPpath[i].x - mMotionGraphs.tempPpath[i+1].x), abs(mMotionGraphs.tempPpath[i].y - mMotionGraphs.tempPpath[i+1].y) )<<endl;
		cout << (mMotionGraphs.tempPpath[i].x-mMotionGraphs.tempPpath[i+2].x)*(mMotionGraphs.tempPpath[i+1].y-mMotionGraphs.tempPpath[i+2].y)-(mMotionGraphs.tempPpath[i].y-mMotionGraphs.tempPpath[i+2].y)*(mMotionGraphs.tempPpath[i+1].x-mMotionGraphs.tempPpath[i+2].x) << endl;
#endif
		if (abs(mMotionGraphs.tempPpath[i].x - mMotionGraphs.tempPpath[i+1].x) > 0.001 && abs(mMotionGraphs.tempPpath[i].y - mMotionGraphs.tempPpath[i+1].y) > 0.001)
		{
			sign = (mMotionGraphs.tempPpath[i].x-mMotionGraphs.tempPpath[i+2].x)*(mMotionGraphs.tempPpath[i+1].y-mMotionGraphs.tempPpath[i+2].y)-(mMotionGraphs.tempPpath[i].y-mMotionGraphs.tempPpath[i+2].y)*(mMotionGraphs.tempPpath[i+1].x-mMotionGraphs.tempPpath[i+2].x);

			if(sign > 0)
			{
				add -= addAngle;
				tttt(i, 4) = data(i, 4) + add;
			}
			else
			{
				add += addAngle;
				tttt(i, 4) = data(i, 4) + add;
			}
		}
		else
			tttt(i, 4) = data(i, 4) + add;

		if (abs(tttt(i, 4) + 89.98) < 0.001)
		{
			tttt(i, 4) = tttt(i-1, 4);
		}
/*
		if (tttt(i, 3) > 1.5)
		{
			tttt(i, 3) = 1.5;
		}
		if (tttt(i,3) < -1.5)
		{
			tttt(i,3) = -1.5;
		}*/

	}
	mMotion = new Motion(tttt,"play");

/*
	fstream fp;
	char* filename = "paly.txt";
	fp.open(filename, ios::out);
	for (int i = 0; i < mMotionGraphs.tempPpCount; i++)
	{
		//fp << tttt << endl;
		for (int j = 0; j < 46; j++)
		{
			fp << tttt(i, j) << ",";
		}
		fp << endl;
	}
	fp.close();*/
#ifdef FOR_PLAY
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
#else
	mMotion->writeToFile("play.dat");
#endif
}

void MotionGraphsPanel::pathWhere()
{

	Eigen::MatrixXd  data(FRAME_NUM_FOR_PLAY+1, 46);
	int ccData = 0;
	int pre = 0, now = 0;	
	int motionListNum1 = 0, motionFrame1 = 0;
	int motionListNum2 = 0, motionFrame2 = 0;
	Motion *motion;
	bool flag = false;
	double i_X = 0.0, i_Y = 0.0, i_Z = 0.0;
	double s_X = 0.0, s_Y = 0.0, s_Z = 0.0;
	mMotionGraphs.randomPlay(mMotionGraphs.tempNode[0]);
	for (int i = 0; i < mMotionGraphs.pathPlayCount - 1; i++)
	{
		motionListNum1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ / 10000;
		motionFrame1 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i])].positionZ % 10000;
		motionListNum2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ / 10000;
		motionFrame2 = realNode[returnCountRealIndex(mMotionGraphs.pathPlay[i+1])].positionZ % 10000;

		motion = getMotion(MotionList.at(motionListNum1)->getName());

		if (motionListNum1 == motionListNum2)
		{
			if (motionFrame1 < motionFrame2) //前面节点到后面节点  直接用原始运动
			{	
				if(pre == -1)   //原始运动是插出来的
					motionFrame1 = motionFrame1 + WINDOW_M_IN;
				now = 1;

				for (int j = motionFrame1; j <= motionFrame2; j++)
				{
					vector<double> temp = motion->getFrame(j);
					if (ccData == 0)
					{
						i_X = temp[0];
						i_Y = temp[1];
						i_Z = temp[2];
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) = temp[k];
					}
					if (j == motionFrame1 && ccData > 1)
					{
						data(ccData, 0) = 2 * data(ccData-1, 0) - data(ccData-2, 0);
						data(ccData, 1) = 2 * data(ccData-1, 1) - data(ccData-2, 1);
						data(ccData, 2) = 2 * data(ccData-1, 2) - data(ccData-2, 2);
					}
					else 
					{
						if (ccData > 0)
						{
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;

							data(ccData, 0) = s_X + data(ccData-1, 0);
							data(ccData, 1) = s_Y + data(ccData-1, 1);
							data(ccData, 2) = s_Z + data(ccData-1, 2);
						}
						else
						{
							s_X = temp[0] - i_X;			
							s_Y = temp[1] - i_Y;
							s_Z = temp[2] - i_Z;

							data(ccData, 0) = s_X + i_X;
							data(ccData, 1) = s_Y + i_Y;
							data(ccData, 2) = s_Z + i_Z;
						}
					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"temp" << temp[0] <<":" << temp[1] <<":" <<temp[2] <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
					i_X = temp[0];
					i_Y = temp[1];
					i_Z = temp[2];

					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}
			}
			else    //后面节点到前面节点  用内部插值数据
			{
				if (pre == 1)
				{
					ccData = ccData - WINDOW_M_IN;
				}
				now = -1;

				int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
				tem = returnMap(tem);
				//cout << "interdata tem:" << tem << endl;
				if (tem == -1)
				{
					printf("random has error, no data\n");
					break;
				}
				Eigen::MatrixXd interDate(newMotionList[tem]->getTotalFrameNum(), 46);
				interDate = newMotionList[tem]->getMotionData();
				//cout << interDate;
				for (int j = 0; j < interDate.rows(); j++)
				{
					if (ccData == 0)
					{
						i_X = interDate(j,0);
						i_Y = interDate(j,1);
						i_Z = interDate(j,2);
					}
					for (int k = 0; k < 46; k++)
					{
						data(ccData,k) =  interDate(j,k);
					}
					if (ccData > 0)
					{
						data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
						data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
						data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
					}						
					else
					{
						data(ccData, 0) = i_X;
						data(ccData, 1) = i_Y;
						data(ccData, 2) = i_Z;
					}
#ifdef FOR_DEBUG
					cout <<ccData << ":"<< endl;
					cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
					cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
					ccData++;
					if (ccData > FRAME_NUM_FOR_PLAY)
					{
						flag = true;
						break;
					}
				}				
			}
		}
		else   //不同运动序列      直接插值  
		{
			if (pre == 1)
			{
				ccData = ccData - WINDOW_M_OUT;
			}
			now = -1;

			int tem = mMotionGraphs.pathPlay[i] * 10000 + mMotionGraphs.pathPlay[i+1];
			tem = returnMap(tem);
			if (tem == -1)
			{
				printf("radom has error, no data\n");
				break;
			}
			Eigen::MatrixXd interDate(newMotionList[tem]->getFreedomNum(), 46);
			interDate = newMotionList[tem]->getMotionData();
			for (int j = 0; j < interDate.rows(); j++)
			{
				if (ccData == 0)
				{
					i_X = interDate(j,0);
					i_Y = interDate(j,1);
					i_Z = interDate(j,2);
				}
				for (int k = 0; k < 46; k++)
				{
					data(ccData,k) =  interDate(j,k);
				}
				if (ccData > 0)
				{
					data(ccData, 0) = data(ccData-1, 0) + interDate(j, 0);
					data(ccData, 1) = data(ccData-1, 1) + interDate(j, 1);
					data(ccData, 2) = data(ccData-1, 2) + interDate(j, 2);
				}						
				else
				{
					data(ccData, 0) = i_X;
					data(ccData, 1) = i_Y;
					data(ccData, 2) = i_Z;
				}
#ifdef FOR_DEBUG
				cout <<ccData << ":"<< endl;
				cout<<"interdata" <<interDate(j,0)<<":" << interDate(j,1) <<":" << interDate(j,2) <<endl;
				cout<<"data" << data(ccData,0)<<":" << data(ccData,1) <<":" <<data(ccData,2) <<endl;
#endif
				ccData++;
				if (ccData > FRAME_NUM_FOR_PLAY)
				{
					flag = true;
					break;
				}
			}
		}
		if (flag)
		{
			break;
		}
		pre = now;
	}
	// 让跑来试试
	double dx[100],dy[100];
	//EulerAngle EA[100];
	Ogre::Quaternion QU[100];
	PPoint pathTempPoint[2000];
	int pathTempCount = 0;
	for(int i = 0; i < pathWhereCount-1; i++)
	{
		dx[i] = pathWherePoint[i+1].x - pathWherePoint[i].x;
		dy[i] = pathWherePoint[i+1].y - pathWherePoint[i].y;

/*
		if (i > 0)
		{
			EA[i].m_fPitch = (atan((pathWherePoint[i+1].y - pathWherePoint[i].y)/(pathWherePoint[i+1].x - pathWherePoint[i].x))
				+ atan((pathWherePoint[i].y - pathWherePoint[i-1].y)/(pathWherePoint[i].x - pathWherePoint[i-1].x)))/2.0;
			EA[i].m_fRoll = 0.0;
			EA[i].m_fYaw = 0.0;
		}
		else if (i == 0)
		{//取random初始朝向
			EA[i].m_fRoll = data(0, 3);
			EA[i].m_fPitch = data(0, 4);
			EA[i].m_fYaw = data(0 ,5);
		}
		QU[i] = EulerAngleToQuaternion(EA[i]);*/
	}
	double PK = 0.0, PK1 = 0.0, DPK = 0.0, DPK1 = 0.0;
	double dist = 0.0, sign = 0.0;
	double addAngle = 0.0 , dis1 = 0.0, dis2 = 0.0, dis3 = 0.0;
	for (int i = 0; i < pathWhereCount - 2; i++)
	{
		dist = 22.0/(100*sqrt((pathWherePoint[i+1].x-pathWherePoint[i].x)*(pathWherePoint[i+1].x-pathWherePoint[i].x)+(pathWherePoint[i+1].y-pathWherePoint[i].y)*(pathWherePoint[i+1].y-pathWherePoint[i].y)));
		for (double u = 0; u <= 1; u+=dist)  // 暂时用0.01   其实是长度/0.24 就是插多少帧 然后在倒
		{
			PK = (2*u*u*u - 3*u*u +1);
			PK1 = (-2*u*u*u + 3*u*u);
			DPK = (u*u*u - 2*u*u +u);
			DPK1 = (u*u*u - u*u);

			pathTempPoint[pathTempCount].x = pathWherePoint[i].x*PK	+ pathWherePoint[i+1].x*PK1 + dx[i]*DPK + dx[i+1]*DPK1;
			pathTempPoint[pathTempCount++].y = pathWherePoint[i].y*PK + pathWherePoint[i+1].y*PK1 + dy[i]*DPK + dy[i+1]*DPK1;

			if(pathTempCount > 1999)
				break;
/*
			if (pathTempCount > 1)
			{
				dis1 = sqrt((pathTempPoint[pathTempCount-2].x - pathTempPoint[pathTempCount-1].x)*(pathTempPoint[pathTempCount-2].x - pathTempPoint[pathTempCount-1].x) + (pathTempPoint[pathTempCount-2].y - pathTempPoint[pathTempCount-1].y)*(pathTempPoint[pathTempCount-2].y - pathTempPoint[pathTempCount-1].y));
				dis2 = (pathTempPoint[pathTempCount-1].x - pathTempPoint[pathTempCount-2].x)/dis1;
				cout << asin(dis2)*180.0/M_PI << endl;
			}*/
		}
	}

	Eigen::MatrixXd  tttt(pathTempCount, 46);

	for (int i = 0; i < pathTempCount; i++)
	{
		for (int j =0; j < 46; j++)
		{
			tttt(i,j) = data(i,j);
		}
	}
	double tt = 0.0;
	double preadd = 0.0, nowadd = 0.0;
	for (int i = 0; i < pathTempCount-2; i++)
	{
		tttt(i, 0) = pathTempPoint[i].x;
		tttt(i, 2) = pathTempPoint[i].y;

		dis1 = sqrt((pathTempPoint[i].x - pathTempPoint[i+1].x)*(pathTempPoint[i].x - pathTempPoint[i+1].x) + (pathTempPoint[i].y - pathTempPoint[i+1].y)*(pathTempPoint[i].y - pathTempPoint[i+1].y));

		tt = (pathTempPoint[i].x - pathTempPoint[i+1].x)/dis1;
		if (pathTempPoint[i].x > pathTempPoint[i+1].x && pathTempPoint[i].y > pathTempPoint[i+1].y)
		{
			tttt(i,4) = -180.0+asin(tt)*180.0/M_PI;
		}
		else if (pathTempPoint[i].x > pathTempPoint[i+1].x && pathTempPoint[i].y < pathTempPoint[i+1].y)
		{
			tttt(i,4) = -asin(tt)*180.0/M_PI;
		}
		else if (pathTempPoint[i].x < pathTempPoint[i+1].x && pathTempPoint[i].y > pathTempPoint[i+1].y)
		{
			tttt(i, 4) = -90.0 - asin(tt)*180.0/M_PI;
		}
		else 
		{
			tttt(i, 4) = -90.0 + asin(tt)*180.0/M_PI;
		}
	}
	mMotion = new Motion(tttt,"test");

#ifdef FOR_PLAY
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
#else
	mMotion->writeToFile("test.dat");
#endif

/*
	if (!playPath)
	{
		playPath = mEditor->createPlayPPath(pathTempPoint, pathTempCount);
	}
	else
		playPath->setVisbility(!playPath->getVisbility());*/
}