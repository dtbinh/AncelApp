#ifndef __AnimationEditor_h_
#define __AnimationEditor_h_

#include "MousePicker.h"
#include "Skeleton.h"
#include "Animation.h"
#include "IKSolver.h"
#include "Path.h"
#include "MotionGraphs.h"

namespace AncelApp
{
	class AnimationEditor: public PickableObject
	{
	public:	
		AnimationEditor(Animation* animation);
		~AnimationEditor();
		bool  notifyReleased();
		bool  notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray);
		bool  notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset);
 
		void showMotionPath(const int index, bool visbility);

		bool getRootPathVisibility();
		void setRootPathVisibility();
		
		bool getPathVisibility(const int index);
 		void setPathVisibility(const int index);

		bool addChain(std::string root, std::string leaf);
		void removeChain(const std::size_t index);
		void showPath(const int index, bool visibility);

		void updateShiftValue(const Ogre::Vector3 &shift);

		float& getRotationAngel() {return mRotateAngel;}
		Ogre::Vector3& getRootShiftValue() {return mRootShift;}

		void setAuxVisibility(bool visible);

		void applyShift(); 
		void applyRotation(); 

		void colorChain(std::size_t index, std::string materialname);
 		void upateAuxSkeleton();
		void pauseAnimation();

		//add 2013-4-19
		Path* createPlayPath(Point Ppath[], int PpCount);
		Path* createPlayPPath(PPoint Ppath[], int PpCount);
		//add 2013-4-24  ”√path ’πœ÷ goSomeWhere
		Path* AnimationEditor::createGoWherePath(PPoint pathGo[], int cCount);
	private:
		void createAuxSkeleton();
		
 		
		Path* createPath(std::string boneName);

		void setPlayPathVisibility();
		void calculateHandlePoint(Eigen::MatrixXd &position, int traceNum);
	
		void modifyRootPath(std::pair<int,int> range);
		void updateEndEffector();
 		void averageSmooth(Eigen::MatrixXd& path, int scale);

		Eigen::MatrixXd getBonePosition(const std::string &name);
 		Ogre::Vector3 calculateOrientation(const Ogre::Radian theta, const int frameNum);
		Ogre::Radian  calculateRotateAngle(const Eigen::MatrixXd &path, const int frameNum);

		bool isFixXAxis() {return mFixXAxis;}
		bool isFixYAxis() {return mFixYAxis;}
		bool isFixZAxis() {return mFixZAxis;}

		void initAvailableRoot();
 		
		void colorChain(std::string root, std::string leaf, std::string materialname);

		const std::vector<std::string>& getBoneNameList() const{return mBoneNameList;}
		const std::vector<std::string>& geAvailableRoot(std::string name) const {return mMapAvailableRoot.at(name);}

		const std::vector<std::pair<std::string,std::string>>& getChainNameList() const {return mChainNameList;}
		
 	private:
		friend class AnimationEditorPanel;

  		Ogre::Real			mCollsionDepth;
		Ogre::Vector3		mPickedNodeOffset;
		Ogre::SceneNode*    mPickedNode;
		std::size_t			mPickedIndex;
		Path*               mPickedPath;
		
		//----------------------------------
		Animation          *mAnimation;
		Skeleton		   *mSkeleton;
		Motion		       *mMotion;
		
		std::shared_ptr<Skeleton>	mAuxSkeletonPtr;
		std::vector<std::shared_ptr<Skeleton>> mMotionTrackPtr;
 		//-----------------------------------------------------

		bool mFixXAxis;
		bool mFixYAxis;
		bool mFixZAxis;

		bool mRootModify;
		float mRotateAngel;
		Ogre::Vector3 mRootShift;

 		//-----------------------------------------------------
 		Path*               mRootPath;
		//Path*               mPlayPath;
		std::vector<Path*>  mPathList;
		AncelIK::IKSolver   mSolver;
 		
		std::vector<std::string> mBoneNameList;
		std::vector<std::pair<std::string,std::string>> mChainNameList;
		std::map<std::string,std::vector<std::string>> mMapAvailableRoot;

 	};

};

#endif
