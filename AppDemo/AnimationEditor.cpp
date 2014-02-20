#include "AnimationEditor.h"
#include "AxisEntity.h"
#include "AppDemo.h"
#include "AnimationEditorPanel.h"
#include "MotionGraphs.h"

using namespace AncelApp;

AnimationEditor::AnimationEditor(Animation* anim)
	:mAnimation(anim),
	mPickedPath(nullptr),
	mRootShift(Ogre::Vector3(0.0f,0.0f,0.0f)),
	mRotateAngel(0),
	mRootModify(false),
	mRootPath(nullptr),
	//mPlayPath(nullptr),
	mFixXAxis(true),
	mFixYAxis(true),
	mFixZAxis(true)
{
	mSkeleton = mAnimation->getSkeleton();
	mMotion = mAnimation->getMotion();
	mSkeleton->setPickEventHandle(static_cast<PickableObject*>(this));

	mAuxSkeletonPtr = std::shared_ptr<Skeleton>(new Skeleton("Aux"));
	mAuxSkeletonPtr->cloneFrom(*mSkeleton);
	mAuxSkeletonPtr->setSkeletonType(1);

	::SetCurrentDirectoryA(AppDemo::getSingletonPtr()->mWorkDirectory.c_str());
	mAuxSkeletonPtr->attachSkeletonToScene(AppDemo::getSingletonPtr()->mSceneMgr);
	mAuxSkeletonPtr->setVisibility(false);

	mSolver.initSlover(mAuxSkeletonPtr.get());
	
	initAvailableRoot();
}

AnimationEditor::~AnimationEditor()
{
	MousePicker::getSingletonPtr()->resetPicker();
	
	if(mRootPath) delete mRootPath;
	for(std::size_t i = 0; i < mPathList.size(); i++)
		delete mPathList[i];
}

bool  AnimationEditor::notifyReleased()
{
	AnimationEditorPanel::getSingletonPtr()->bindAnimationEditor(nullptr);
	AnimationEditorPanel::getSingletonPtr()->setVisible(false);
 
	mPickedNode->showBoundingBox(false);
	mPickedPath = nullptr;

 	AxisEntity::getSingletonPtr()->setVisible(false);

	return true;
}

bool  AnimationEditor::notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
{
	if(mPickedPath && evt.state.buttonDown(OIS::MB_Left))
	{
		Ogre::Vector3 position = ray.getPoint(mCollsionDepth) + mPickedNodeOffset;
	//	Ogre::Vector3 pos = position - mPickedNode->getPosition();

	  // 	float dis = pos.length();
		//if(dis < 1)
		//{
			if(mFixXAxis) position.x = mPickedNode->getPosition().x;
			if(mFixYAxis) position.y = mPickedNode->getPosition().y;
			if(mFixZAxis) position.z = mPickedNode->getPosition().z;
			
			position.x = int(position.x*1e+6)/double(1e+6);
			position.y = int(position.y*1e+6)/double(1e+6);
			position.z = int(position.z*1e+6)/double(1e+6);

			mPickedPath->update_(mPickedIndex, position);
			AxisEntity::getSingletonPtr()->setPosition(mPickedNode->getPosition());
			AxisEntity::getSingletonPtr()->setVisible(true);
 		//}
			if(mPickedPath == mRootPath)
			{
  				modifyRootPath(std::pair<int,int>(0,0));
				if(mFixXAxis || mFixYAxis || mFixZAxis)
				{
					for(std::size_t i = 0; i < mChainNameList.size(); i++)
					{
						bool visible = mPathList[i]->getVisbility();
						delete mPathList[i];
						mPathList[i] = createPath(mChainNameList[i].second);
						mPathList[i]->setVisbility(visible);
					}
				}
 			}
			else
     			updateEndEffector();
			 
			upateAuxSkeleton();
	}
	return true;
}

Path* AnimationEditor::createPath(std::string boneName)
{
	
	Eigen::MatrixXd position(mMotion->getTotalFrameNum(),4);

 	 for(int i = 0; i < position.rows(); i++)
	 {
		 mSkeleton->update(mMotion->getFrame(i),false);
 		 Ogre::Vector3 pos = mSkeleton->getBone(boneName)->getGlobalPos();
 		 position(i, 0) = int(pos.x*1e+6)/double(1e+6);
		 position(i, 1) = int(pos.y*1e+6)/double(1e+6);
		 position(i, 2) = int(pos.z*1e+6)/double(1e+6);
		 position(i, 3) = (i == 0 || i == position.rows()-1) ? 1: 0;
 	 }
	//averageSmooth(position,3);
	 	 
	 calculateHandlePoint(position, 1);
	  
 	 Path* path = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	 path->setInit3DPath(position);
	 path->createPath(MyGUI::utility::toString(this) + "_Path_" + MyGUI::utility::toString(path),"Dru_Bezh", false, true);

	 return path;
}

// for motiongraphs add 2013-4-19
Path* AnimationEditor::createPlayPath(Point Ppath[], int PpCount)
{
	Eigen::MatrixXd position(PpCount,4);
	Point pointTemp;
	Path*  mPlayPath;

	for (int i = 0; i < PpCount; i++)
	{
		pointTemp = Ppath[i];
		//cout << (pointTemp.x * 8 - 400) << ":" << (pointTemp.y * 8 - 400) << endl;
		position(i, 0) = pointTemp.x;//int(pointTemp.x * 8 - 400);
		position(i, 1) = 15;
		position(i, 2) = pointTemp.y;//int(pointTemp.y * 8 - 400);
		position(i, 3) = (i == 0 || i == PpCount-1 || i% 5 == 0) ? 1: 0;
	}

	//averageSmooth(position,1);
	calculateHandlePoint(position, 1);
	mPlayPath = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	mPlayPath->setInit3DPath(position);
	mPlayPath->createPath(MyGUI::utility::toString(this) + "_PlayPath_", "OrangePath", false, true);
	mPlayPath->setVisbility(true);

	return mPlayPath;
}

Path* AnimationEditor::createPlayPPath(PPoint Ppath[], int PpCount)
{
	Eigen::MatrixXd position(PpCount,4);
	PPoint pointTemp;
	Path*  mPlayPath;

	for (int i = 0; i < PpCount; i++)
	{
		pointTemp = Ppath[i];
		//cout << (pointTemp.x * 8 - 400) << ":" << (pointTemp.y * 8 - 400) << endl;
		position(i, 0) = pointTemp.x;//int(pointTemp.x * 8 - 400);
		position(i, 1) = 15;
		position(i, 2) = pointTemp.y;//int(pointTemp.y * 8 - 400);
		position(i, 3) = (i == 0 || i == PpCount-1 || i% 5 == 0) ? 1: 0;
	}

	//averageSmooth(position,1);
	calculateHandlePoint(position, 1);
	mPlayPath = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	mPlayPath->setInit3DPath(position);
	mPlayPath->createPath(MyGUI::utility::toString(this) + "_PlayPath_", "OrangePath", false, true);
	mPlayPath->setVisbility(true);

	return mPlayPath;
}

//add 2013-4-24  ”√path ’πœ÷ goSomeWhere
Path* AnimationEditor::createGoWherePath(PPoint pathGo[], int cCount)
{
	Eigen::MatrixXd position(cCount,4);
	PPoint pointTemp;
	Path*  mPlayPath;

	for (int i = 0; i < cCount; i++)
	{
		pointTemp = pathGo[i];
		position(i, 0) = pointTemp.x;
		position(i, 1) = 15;
		position(i, 2) = pointTemp.y; 
		position(i, 3) = (i == 0 || i == cCount-1 || i% 5 == 0) ? 1: 0;

	}
	//averageSmooth(position,1);
	calculateHandlePoint(position, 1);		
	
	mPlayPath = new Path(AppDemo::getSingletonPtr()->mSceneMgr,static_cast<PickableObject*>(this), 6, 0.1f, 12, 12, 0.4f);
	mPlayPath->setInit3DPath(position);
	mPlayPath->createPath(MyGUI::utility::toString(this) + "_PlayPath_", "OrangePath", false, true);
	mPlayPath->setVisbility(true);

	return mPlayPath;	
}

bool  AnimationEditor::notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset)
{
	mPickedIndex = Ogre::any_cast<int>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).third);
	mPickedPath = Ogre::any_cast<Path*>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).second);

	if(AppDemo::getSingletonPtr()->mKeyboard->isKeyDown(OIS::KC_LCONTROL))
	{
		mPickedPath = Ogre::any_cast<Path*>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).second);
		mPickedIndex = Ogre::any_cast<int>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).third);
		mPickedPath->updateControlPoint(mPickedIndex);
		mPickedPath = nullptr;
		mPickedIndex = 0;
		return false;
 	}
	else if(mPickedPath->isHandle(mPickedIndex))
	{
    	AnimationEditorPanel::getSingletonPtr()->bindAnimationEditor(this);
		AnimationEditorPanel::getSingletonPtr()->setVisible(true);

		mCollsionDepth = entry.distance;
		mPickedNodeOffset = offset;
		mPickedNode = entry.movable->getParentSceneNode();
		mPickedPath = Ogre::any_cast<Path*>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).second);
		mPickedIndex = Ogre::any_cast<int>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).third);
  	
		mPickedNode->showBoundingBox(true);
		AxisEntity::getSingletonPtr()->setPosition(mPickedNode->getPosition());
		AxisEntity::getSingletonPtr()->setVisible(true);
		mAnimation->setEnabled(false);
		return true;
	}
 	else 
	{
		MousePicker::getSingletonPtr()->resetPicker();
		mPickedPath = nullptr;
 	}
  	return false;
}
 
void AnimationEditor::showMotionPath(const int index, bool visbility)
{

}

void AnimationEditor::calculateHandlePoint(Eigen::MatrixXd &position, int traceNum)
{
		for(int i = 0; i < traceNum; i++)
	{
		std::vector<std::size_t> handlePt;
		for(int j = 0; j < position.rows(); j++)
		{
			if(j == 0 || j == position.rows() - 1)
			{
 				handlePt.push_back(j);
 			}
			else 
			{
				if(position(j, i * 4 + 1) < position(j-1, i * 4 + 1) && position(j, i * 4 + 1) < position(j+1, i*4 + 1))
	 				handlePt.push_back(j);
  	 			else if(position(j, i * 4 + 1) > position(j-1, i * 4 + 1) && position(j, i * 4 + 1) > position(j+1, i*4 + 1)) 
	 				handlePt.push_back(j);
  	 		}
	    	position(j, i * 4 + 3) = 0;
  		}

		std::vector<std::vector<int>> cluster;
		for(std::size_t k = 0; k <  handlePt.size(); k++)
		{
			bool flag = false;
			for(std::size_t t = 0; t < cluster.size(); t++)
			{
 				for(std::size_t w = 0; w < cluster[t].size(); w++)
				{
					Ogre::Vector3 v0 = Ogre::Vector3(position(handlePt[k], i * 4),position(handlePt[k], i * 4 + 1),position(handlePt[k], i * 4 + 2));
					Ogre::Vector3 v1 = Ogre::Vector3(position(cluster[t][w], i * 4),position(cluster[t][w], i * 4 + 1),position(cluster[t][w], i * 4 + 2));

					v1 = v1-v0;
					if(v1.length() < 2)
					{
						flag = true;
						cluster[t].push_back(handlePt[k]);
						break;
					}
				}
				if(flag) break;
			}
			if(!flag)
			{
				std::vector<int> temp;
				temp.push_back(handlePt[k]);
				cluster.push_back(temp);
			}
		}
		for(std::size_t t = 0; t < cluster.size(); t++)
		{
			bool findclustercenter = false;
			
			int maxVal = -1;
			int minVal = 0x7fffff;
			int index = -1;
 			for(std::size_t w = 0; w < cluster[t].size(); w++)
			{
				if(cluster[t][w] == 0 || cluster[t][w] == position.rows() - 1)
				{
					findclustercenter = true;
					position(cluster[t][w], i * 4 + 3) = 1;
					break;
				}
				if(position(cluster[t][w], i * 4 + 1) < position(cluster[t][w] - 1, i * 4 + 1) 
				 && position(cluster[t][w], i * 4 + 1) < position(cluster[t][w] + 1, i * 4 + 1))
				{
					if(position(cluster[t][w], i * 4 + 1) < minVal)
					{
						index = cluster[t][w];
						minVal = position(cluster[t][w], i * 4 + 1);
					}
				}
				else if(position(cluster[t][w], i * 4 + 1) > position(cluster[t][w] - 1, i * 4 + 1) 
				 && position(cluster[t][w], i * 4 + 1) > position(cluster[t][w] + 1, i * 4 + 1))
				{
					if(position(cluster[t][w], i * 4 + 1) > maxVal)
					{
						index = cluster[t][w];
						maxVal = position(cluster[t][w], i * 4 + 1);
					}
				}
   			}
			if(!findclustercenter)
				position(index, i * 4 + 3) = 1;
		}
		
 	}
}

bool AnimationEditor::addChain(std::string root, std::string leaf)
{
	bool isExisted = false;
	
	for(std::size_t i = 0; i < mChainNameList.size(); i++)
	{
		if(mChainNameList[i].second == leaf)
		{
			isExisted = true;
			break;
		}
	}
	if(isExisted) 
		return false;

	mChainNameList.push_back(std::make_pair(root,leaf));
	mSolver.addChain(root,leaf);

	mPathList.push_back(createPath(leaf));

	return true;
}

void AnimationEditor::removeChain(const std::size_t index)
{
	if(index >= 0 && index < mChainNameList.size())
	{
		mChainNameList.erase(mChainNameList.begin() + index);
		mSolver.removeChain(index);
		
		MousePicker::getSingletonPtr()->resetPicker();
		delete mPathList[index];
		mPathList.erase(mPathList.begin() + index);
	}
}
  
void AnimationEditor::modifyRootPath(std::pair<int,int> range)
{
 	range.first = 0;
	range.second = mAnimation->getTotalFrame();

	Eigen::MatrixXd path = mRootPath->getUpdatedPath();
 	Eigen::MatrixXd rootFreedom(range.second - range.first, 6);
 	rootFreedom.block(0, 0, range.second - range.first, 3) = path.block(range.first, 0,range.second - range.first, 3);
	
	Eigen::MatrixXd adjustAngle(path.rows(),1);
	for(int i = range.first; i < range.second; i++)
 		adjustAngle(i,0) = calculateRotateAngle(path,i).valueRadians();

	/*std::ofstream fout("angle.txt");
	for(int i = range.first; i < range.second; i++)
	{
		fout << adjustAngle(i,0) << " ";
	}
	fout << std::endl;*/
	averageSmooth(adjustAngle,40);
	 

	for(int i = 0; i < adjustAngle.rows(); i++)
	{
		Ogre::Vector3 ori = calculateOrientation(Ogre::Radian(adjustAngle(i,0)),i);
 		rootFreedom(i-range.first,3) = ori.x;
		rootFreedom(i-range.first,4) = ori.y;
		rootFreedom(i-range.first,5) = ori.z;
 	}
	
	//for(std::size_t i = 0; i < range.second; i++)
	//{
	//	std::cout << i << ": Ori(" << mMotion->getRootPosition(i).x << " " << mMotion->getRootPosition(i).z <<") Now:(";
	//	std::cout <<  rootFreedom(i,0) << " " << rootFreedom(i,2)<< ")"  << std::endl;
	//}
	
	mMotion->updateRoot(rootFreedom, range);
}
void AnimationEditor::updateEndEffector()
{
	std::size_t index = 0;
 	for(std::size_t i = 0; i < mPathList.size(); i++)
		if(mPickedPath == mPathList[i])
		{
			index = i;
			break;
		}
	
	for(int i = 0; i < mAnimation->getTotalFrame(); i++)
	{
		mAuxSkeletonPtr->update(mMotion->getFrame(i));
		for(std::size_t j = 0; j < mPathList.size(); j++)
		{
			Ogre::Vector3 v(mPathList[j]->getUpdatedPath()(i,0),mPathList[j]->getUpdatedPath()(i,1),mPathList[j]->getUpdatedPath()(i,2));
			mSolver.setEndEffectorGoal(j, v);
		}
		mSolver.solve();
		std::vector<double> theta(mMotion->getFreedomNum(), 0);
		mAuxSkeletonPtr->getSkeletonParameters(theta);
		mMotion->updateFrame(i, theta);
	}
}
Eigen::MatrixXd AnimationEditor::getBonePosition(const std::string &name)
{
	 Eigen::MatrixXd position(mMotion->getTotalFrameNum(),3);
 
	 for(int i = 0; i < position.rows(); i++)
	 {
		 mAuxSkeletonPtr->update(mMotion->getFrame(i),false);
		 Ogre::Vector3 pos = mAuxSkeletonPtr->getBone(name)->getGlobalPos();
		 
		 position(i,0) = pos.x;
		 position(i,1) = pos.y;
		 position(i,2) = pos.z;
	 }
	 return position;
}

Ogre::Vector3 AnimationEditor::calculateOrientation(const Ogre::Radian theta, const int frameNum)
{
	Ogre::Vector3 v = mMotion->getRootOriention(frameNum);
 
	Ogre::Matrix3 rotMat;
	rotMat.FromEulerAnglesZYX(Ogre::Radian(Ogre::Degree(v.z).valueRadians()),
							  Ogre::Radian(Ogre::Degree(v.y).valueRadians()),
							  Ogre::Radian(Ogre::Degree(v.x).valueRadians()));
  	Ogre::Matrix3 hMatrix;
    hMatrix.FromAngleAxis(Ogre::Vector3(0, 1, 0), -theta);
 
 	Ogre::Quaternion Q1,Q2;
	Q1.FromRotationMatrix(hMatrix);
	Q2.FromRotationMatrix(rotMat);
	
	Q1 = Q1 * Q2;
	Q1.ToRotationMatrix(hMatrix);
  	Ogre::Radian ax, ay, az;
	hMatrix.ToEulerAnglesZYX(az, ay, ax);
  	return Ogre::Vector3(ax.valueDegrees(), ay.valueDegrees(), az.valueDegrees());
}
Ogre::Radian  AnimationEditor::calculateRotateAngle(const Eigen::MatrixXd &path, const int frameNum)
{
	Ogre::Vector2 hNow, hPast;
	if(frameNum == 0)
	{
 		hNow = Ogre::Vector2(path(1,0) - path(0,0), path(1,2) - path(0,2));
		Ogre::Vector3 v0 = mMotion->getRootPosition(0);
		Ogre::Vector3 v1 = mMotion->getRootPosition(1);
 		hPast = Ogre::Vector2(v1.x - v0.x, v1.z - v0.z);
 	}
	else if(frameNum == path.rows()-1)
	{
		hNow = Ogre::Vector2(path(frameNum,0)-path(frameNum-1,0),path(frameNum,2)-path(frameNum-1,2));
		Ogre::Vector3 v0 = mMotion->getRootPosition(frameNum-1);
		Ogre::Vector3 v1 = mMotion->getRootPosition(frameNum);
 		hPast = Ogre::Vector2(v1.x - v0.x, v1.z - v0.z);
  	}
	else
	{
		Ogre::Vector3 v0 = mMotion->getRootPosition(frameNum - 1);
		Ogre::Vector3 v1 = mMotion->getRootPosition(frameNum + 1);
		hPast = Ogre::Vector2(v1.x - v0.x, v1.z - v0.z);
	 	hNow = Ogre::Vector2((path(frameNum+1,0) - path(frameNum-1,0))/2, (path(frameNum+1,2) - path(frameNum - 1,2))/2);
	
		/*if(frameNum - 2 >= 0 && frameNum + 2 < path.rows())
		{
			hNow += Ogre::Vector2(path(frameNum+2,0) - path(frameNum-2,0), path(frameNum+2,2) - path(frameNum-2,2)).normalisedCopy();

			Ogre::Vector3 v0 = mMotion->getRootPosition(frameNum - 2);
			Ogre::Vector3 v1 = mMotion->getRootPosition(frameNum + 2);
			hPast += Ogre::Vector2(v1.x - v0.x, v1.z - v0.z).normalise();
		}
		if(frameNum - 3 >= 0 && frameNum + 3 < path.rows())
		{
			hNow += Ogre::Vector2(path(frameNum+3,0) - path(frameNum-3,0), path(frameNum+3,2) - path(frameNum-3,2)).normalisedCopy();
	
			Ogre::Vector3 v0 = mMotion->getRootPosition(frameNum - 3);
			Ogre::Vector3 v1 = mMotion->getRootPosition(frameNum + 3);
 			hPast += Ogre::Vector2(v1.x - v0.x, v1.z - v0.z).normalise();
		}
		if(frameNum - 4 >= 0 && frameNum + 4 < path.rows())
		{
			hNow += Ogre::Vector2(path(frameNum+4,0) - path(frameNum-4,0), path(frameNum+4,2) - path(frameNum-4,2)).normalisedCopy();

			Ogre::Vector3 v0 = mMotion->getRootPosition(frameNum - 4);
			Ogre::Vector3 v1 = mMotion->getRootPosition(frameNum + 4);
 			hPast += Ogre::Vector2(v1.x - v0.x, v1.z - v0.z).normalise();
		}*/
 	}
	hPast.normalise();
	hNow.normalise();
	float crossProduct = hNow.crossProduct(hPast);
 	Ogre::Radian theta = Ogre::Math::ACos(hNow.dotProduct(hPast));
	return (crossProduct > 0) ? -theta:theta;
}

void AnimationEditor::averageSmooth(Eigen::MatrixXd& path, int scale)
{
	for(int i = 0; i < path.rows(); i++){
		
		std::vector<double> val(path.cols(),0);
 
		int s = i - scale > 0 ?  i - scale: 0;
		int t = (i + scale <= path.rows()) ? i + scale : path.rows();
		for(int j = s ; j < t; j++){
			for(int k = 0; k < path.cols(); k++)
			{
				double v = path(j,k);
				val[k] += v;
  			}
 		}
		for(int k = 0; k < path.cols(); k++)
			path(i,k) = val[k]/(t-s);
	}
}

void AnimationEditor::createAuxSkeleton()
{
 	int interval = 10;
	for(int i = 0; i < mAnimation->getTotalFrame(); i+=interval)
	{
		std::shared_ptr<Skeleton> ptr(new Skeleton(mAnimation->getName()));
		ptr->cloneFrom(*mSkeleton);
		ptr->setSkeletonType(0);
 		ptr->attachSkeletonToScene(AppDemo::getSingletonPtr()->mSceneMgr);
		
		std::vector<double> theta = mMotion->getFrame(i);
	
		Ogre::Vector3 posShift(0,0,0);
		Ogre::Vector3 oriention;
		mMotion->rotateFrame(i, mRotateAngel, posShift, oriention);
	
		theta[0] = posShift.x + mRootShift.x;
		theta[1] = posShift.y + mRootShift.y;
		theta[2] = posShift.z + mRootShift.z;

		theta[3] = oriention.x;
		theta[4] = oriention.y;
		theta[5] = oriention.z;
	    ptr->update(theta);
		ptr->setVisibility(true);
		mMotionTrackPtr.push_back(ptr);
 	}
}
void AnimationEditor::upateAuxSkeleton()
{
	int interval = 10; 
	for(std::size_t i = 0; i < mMotionTrackPtr.size(); i++)
	{
		std::vector<double> theta = mMotion->getFrame(i*interval);
	
		Ogre::Vector3 posShift(0,0,0);
		Ogre::Vector3 oriention;
		mMotion->rotateFrame(i*interval, mRotateAngel, posShift, oriention);
	
		theta[0] = posShift.x + mRootShift.x;
		theta[1] = posShift.y + mRootShift.y;
		theta[2] = posShift.z + mRootShift.z;

		theta[3] = oriention.x;
		theta[4] = oriention.y;
		theta[5] = oriention.z;
		mMotionTrackPtr[i]->update(theta);
 	}
}

void AnimationEditor::updateShiftValue(const Ogre::Vector3 &shift)
{
	mRootShift += shift;
	upateAuxSkeleton();
	AnimationEditorPanel::getSingletonPtr()->updateUICaption();
}
void AnimationEditor::setAuxVisibility(bool visible)
{
	if(!mMotionTrackPtr.size()) 
		createAuxSkeleton();
	for(std::size_t i = 0; i < mMotionTrackPtr.size(); i++)
		mMotionTrackPtr[i]->setVisibility(visible);
}


//void Animation::initIKChain()
//{
// 	std::vector<Bone*>::const_iterator it = mAuxSkeletonPtr->begin();
//
//	for (it; it != mAuxSkeletonPtr->end(); it++)
//    {
//		if(!(*it)->isLeaf())
//          continue;
//
//        AncelIK::IKChain* chain = new AncelIK::IKChain();
//		chain->initChain(mAuxSkeletonPtr->getRoot(), *it); 
//		mAuxSkelChain.push_back(chain);
//    }
//}


void AnimationEditor::setRootPathVisibility()
{
	if(!mRootPath)
	{
		mAnimation->applyShift();
		mAnimation->applyRotation();
 		mRootPath = createPath("root");
	}
	else 
		mRootPath->setVisbility(!mRootPath->getVisbility());
}
		
 
void AnimationEditor::setPathVisibility(const int index)
{
	if((int)mPathList.size() > index && index >= 0)
	{
		mPathList[index]->setVisbility(!mPathList[index]->getVisbility());
	}
}

bool AnimationEditor::getRootPathVisibility()
{
	if(mRootPath)
		return mRootPath->getVisbility();
	return false;
}
bool AnimationEditor::getPathVisibility(const int index)
{
	if((int)mPathList.size() > index && index >= 0)
 		return mPathList[index]->getVisbility();
 	return false;
}
void AnimationEditor::applyShift() 
{
	mAnimation->applyShift();
	mRootShift = Ogre::Vector3(0,0,0);
	if(mRootPath)
	{
		delete mRootPath;
		mRootPath = createPath("root");
	}
	upateAuxSkeleton();
}
void AnimationEditor::applyRotation() 
{
	mAnimation->applyRotation();

	if(mRotateAngel > 0 && mRootPath)
	{
		delete mRootPath;
		mRootPath = createPath("root");
	}
 
	mRotateAngel = 0;
}

void AnimationEditor::initAvailableRoot()
{
	std::vector<Bone*>::const_iterator it = mAuxSkeletonPtr->begin();
	for (it; it != mAuxSkeletonPtr->end(); it++)
	{
		if(!(*it)->isRoot())
		{
			Bone* parent = const_cast<Bone*>((*it)->getParent());
			std::vector<std::string> ancestors;
			while(parent)
			{
				ancestors.push_back(parent->name());
				parent = const_cast<Bone*>(parent->getParent());
			}
			mBoneNameList.push_back((*it)->name());
			mMapAvailableRoot[(*it)->name()] = ancestors;
		}
    }
}
void AnimationEditor::colorChain(std::size_t index, std::string materialname)
{
	if(index < mChainNameList.size())
	{
		static int last_index = -1;
		static std::string last_materialname;
 
		if(last_index >= 0)
		{
			colorChain(mChainNameList[last_index].first, mChainNameList[last_index].second, last_materialname);
		}

		if(materialname.length())
		{
 			last_index = index;
			last_materialname = mSkeleton->getRoot()->getMaterialName();
 			colorChain(mChainNameList[index].first, mChainNameList[index].second, materialname);
 		}
		else
		{
			last_index = -1;
 		}
	}
}
void AnimationEditor::colorChain(std::string root, std::string leaf, std::string materialname)
{
	Bone* leafBone = mSkeleton->getBone(leaf);
	Bone* rootBone = mSkeleton->getBone(root);
	while(leafBone != rootBone)
	{
		leafBone->updateMaterial(materialname);
		leafBone = const_cast<Bone*>(leafBone->getParent());
	}
	rootBone->updateMaterial(materialname);
}
void AnimationEditor::pauseAnimation()
{
	mAnimation->setEnabled(false);
};