/*
date: 2013-4-1
des: 通过菜单栏调出画板，在画板中添加运动数据,然后构建运动图
*/

#ifndef  __MotionGraphsPanel_h_
#define  __MotionGraphsPanel_h_


#include <vector>
#include <OgreSingleton.h>
#include <PanelView/BasePanelViewItem.h>
#include <MotionSyn.h>
#include <BaseLayout/BaseLayout.h>
#include <OgreVector3.h>
#include "Motion.h"
#include "Skeleton.h"
#include "Animation.h"
#include "StatePanel.h"     //调用 updateActorTheme 
#include "MotionGraphs.h"
#include "AnimationEditor.h"
#include <map>

#define TRANSNODE 50000

namespace AncelApp
{
	//typedef std::map<int, int, less<int> > M_TYPE;

	struct EulerAngle
	{
		double m_fYaw, m_fPitch, m_fRoll;
	};

	struct RNode
	{
		int ccount;
		int positionZ;
	};

	class MotionGraphsPanel:public Ogre::Singleton<MotionGraphsPanel>,public wraps::BaseLayout
	{
	public:
		MotionGraphsPanel();
		~MotionGraphsPanel();
		
		virtual void initialise();
		virtual void shutdown();
		bool setVisible(bool visibility);

		void loadMotionFile(const MyGUI::UString& commandName,bool& result);
		void removeMotionFile(const MyGUI::UString& commandName,bool& result);
		void generateMotion(const MyGUI::UString& commandName, bool& result);
		void playMotion(const MyGUI::UString& commandName, bool& result);

		void  addMotion(const Motion *anim);

		std::string loadMotion(const std::string& filename);
		bool  saveMotion(const std::string &animName, const std::string& fileName) ;//const;

		const Motion* getMotion(const std::string& animName) const;
		Motion* getMotion(const std::string& animName);
		bool  removeMotion(const std::string animName);
		const std::string&  getMotionName(std::size_t index);
		const std::size_t size() const; //return the total number of avaliable motion;

		double calculTheDistance(const std::vector<double> &theta1,const std::vector<double> &theta2);
		void calculWindowDistance(Motion &mo1, Motion &mo2, int molist1, int molist2);

		void realAddNewEdge();
		static int rrcmp(const void *a, const void *b)
		{
			if(((RNode*)a)->positionZ != ((RNode*)b)->positionZ)
				return ((RNode*)a)->positionZ - ((RNode*)b)->positionZ;
			return ((RNode*)a)->ccount - ((RNode*)b)->ccount;
		}

		const std::size_t  returnCountRealIndex(const std::size_t index);
		const std::size_t  returnRealIndex(const std::size_t index);
		int returnFrontCount(int molistnum,int framenum);
		int returnBackCount(int molistnum,int framenum);

		void interpolateAnMotion(int temp1, int temp2);

		const std::size_t returnIndex(const std::size_t index);
		const int returnMap(const int index);

		double getBoneTotalWeight();
		Ogre::Vector4 calMul(Ogre::Matrix4 &mat, Ogre::Vector4 &vec);
		//返回局部四元数
		Ogre::Quaternion getRelativeQuaternion(const std::vector<double> & frame, std::string boneFName);

		Ogre::Quaternion EulerAngleToQuaternion(const EulerAngle &EA);
		EulerAngle QuaternionToEulerAngle(Ogre::Quaternion &q);

		Ogre::Quaternion slerpBy(const Ogre::Quaternion &q0, const Ogre::Quaternion &q1, float t);
		Ogre::Quaternion calTempQuaternion(const Ogre::Quaternion &q0, const Ogre::Quaternion &q1,const Ogre::Quaternion &q2);
		Ogre::Quaternion calMulQuaternion(const Ogre::Quaternion &q0, const Ogre::Quaternion &q1);
		Ogre::Quaternion calNormalQuaternion(Ogre::Quaternion &q);
		bool intersectPlane(const Ogre::Vector3& orig, const Ogre::Vector3& dir, Ogre::Vector3& v0, Ogre::Vector3& v1, Ogre::Vector3& v2, float* t, float* u, float* v);

		void showPanel(const MyGUI::UString& commandName,bool& result);
		void windowButtonPressed(MyGUI::Window* _sender,const std::string& _name);
		void notifyMouseButtonClick(MyGUI::Widget* _sender);
		void windowResized(Ogre::RenderWindow* rw);
		virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);


		void goRandom(const MyGUI::UString& commandName, bool& result);
		void goTwoAttribute(const MyGUI::UString& commandName, bool& result);
		void goWhere();
		void goSomeWhere(const MyGUI::UString& commandName, bool& result);
		void collectPoint(const MyGUI::UString& commandName, bool& result);
		void pathWhere();
	//	void calculateHandlePoint(Eigen::MatrixXd &position, int traceNum);

	private:
		MyGUI::ButtonPtr mBtnAddMotion;
		MyGUI::ButtonPtr mBtnRemoveMotion;
		MyGUI::ButtonPtr mBtnGen;
		MyGUI::ButtonPtr mBtnPlay;
		MyGUI::ButtonPtr mBtnGoTwoAttribute;
		MyGUI::ButtonPtr mBtnGetPoint;
		MyGUI::ButtonPtr mBtnGoWhere;
		MyGUI::ButtonPtr mBtnGoZone;

		MyGUI::ListBox* mListsContent;

		std::vector<Motion*> MotionList;
//for construct
		std::size_t transIndex;
		std::size_t transEdge;
		std::size_t transNode[TRANSNODE];
//for real graphs
		std::size_t realIndex;
		std::size_t realEdge;
		RNode realNode[TRANSNODE];


		Skeleton *mSkeleton;
		MotionGraphs mMotionGraphs;
		
		int newMotionIndex;
		Motion *newMotionList[TRANSNODE];
		Motion *change;

		int newMotionMap[TRANSNODE];
		//M_TYPE newMotionMap;

		Motion *mMotion;

		Animation *mAnimation;
		AnimationEditor* mEditor;

		bool enablePoint;
		double destX, destZ;
		bool isExist[TRANSNODE];

		//第二次点击点时，应该怎样走
		int clickNum;    //点击次数
		int nowFrame;    //现在已经走到的帧
		double initK, doubleK;
		double temp1X, temp1Z, temp2X, temp2Z;

		PPoint pathWherePoint[10];
		int pathWhereCount;

		Path* playPath;
	};
}


#endif