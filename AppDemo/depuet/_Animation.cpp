#include "_Animation.h"
#include <OgreMatrix3.h>
#include <OgreMath.h>
#include <ResUtility.h>
#include "AppUtility.h"
#include "_Skeleton.h"
#include "AppDemo.h"
#include <Quaternion.h>

using namespace AncelApp;

AnimationTrack::AnimationTrack(AnimationTrack& track)
{
 	mRotation = track.mRotation;
	mTransition = track.mTransition;
	mRo = track.mRo;
	mTr = track.mTr;
}
AnimationTrack::AnimationTrack(bool Tr,bool Ro)
	:mTr(Tr),mRo(Ro)
{
}
AnimationTrack::~AnimationTrack()
{

}
void AnimationTrack::pushBackKeyframe(const Ogre::Vector3 &euler,const Ogre::Vector3& trans)
{
	if(mTr) mTransition.push_back(trans);
	if(mRo)
	{
		Ogre::Matrix3 mat;
		mat.FromEulerAnglesZYX(Ogre::Radian(Ogre::Degree(euler.z).valueRadians()),
							   Ogre::Radian(Ogre::Degree(euler.y).valueRadians()),
							   Ogre::Radian(Ogre::Degree(euler.x).valueRadians()));
		Ogre::Quaternion Q;
		Q.FromRotationMatrix(mat);
		mRotation.push_back(Q);
	}
}
void AnimationTrack::pushBackKeyframe(const Ogre::Quaternion &rotate,Ogre::Vector3 trans)
{
	if(mTr) mTransition.push_back(trans);
	if(mRo) mRotation.push_back(rotate);
}


const Ogre::Quaternion& AnimationTrack::getRotation(size_t frameNum) const
{
	assert(mRo);
 	assert(frameNum < mRotation.size());
	return mRotation[frameNum];
}
const Ogre::Vector3&  AnimationTrack::getTransition(size_t frameNum) const
{
	assert(mTr);
	assert(frameNum < mRotation.size());
	return mTransition[frameNum];
}

Animation::Animation(const ResUtil::Matrix *motion, const Ogre::String &name)
	:mAnimtionName(name),mRawmotion(*motion)
{

#define NUMBONE 21
 
	static string boneName[NUMBONE] = {"root","lowerback","upperback","thorax","lowerneck","upperneck","head","rclavicle","rhumerus","rradius",
									   "rwrist","lclavicle","lhumerus","lradius","lwrist","rfemur","rtibia","rfoot","lfemur","ltibia","lfoot"};
	static int DOF[NUMBONE] = {0x111111,0x000111,0x000111,0x000111,0x000111,0x000111,0x000111,0x000000,0x000111,0x000100,
							   0x000010,0x000000,0x000111,0x000100,0x000010,0x000111,0x000100,0x000101,0x000111,0x000100,0x000101};
 
	std::size_t index = 0;

	for (int i = 0; i < NUMBONE; i++)
	{
		bool tr =   (DOF[i]&0x111000) > 0 ? true:false;
		bool ro =   (DOF[i]&0x000111) > 0 ? true:false;
		bool tr_x = (DOF[i]&0x100000) > 0 ? true:false;
		bool tr_y = (DOF[i]&0x010000) > 0 ? true:false;
		bool tr_z = (DOF[i]&0x001000) > 0 ? true:false;
		bool ro_x = (DOF[i]&0x000100) > 0 ? true:false;
		bool ro_y = (DOF[i]&0x000010) > 0 ? true:false;
		bool ro_z = (DOF[i]&0x000001) > 0 ? true:false;

		AnimationTrack* track = createAnimationTrack(boneName[i],ro,tr);
		assert(track != NULL);

		std::size_t curIndex = 0;
 		for (size_t j = 0; j < motion->sizeRow(); j++)
		{
			curIndex = index;

			Ogre::Vector3 trans(0.0,0.0,0.0);
			Ogre::Vector3 euler(0.0,0.0,0.0);

			if(tr_x) trans.x = (float)motion->get(j,curIndex++);
			if(tr_y) trans.y = (float)motion->get(j,curIndex++);
			if(tr_z) trans.z = (float)motion->get(j,curIndex++);
		
			if(ro_x) euler.x = (float)motion->get(j,curIndex++);
			if(ro_y) euler.y = (float)motion->get(j,curIndex++);
			if(ro_z) euler.z = (float)motion->get(j,curIndex++);
 			track->pushBackKeyframe(euler,trans);
		}
		index = curIndex;
 	}
}

std::string Animation::getAnimName() const
{
	return mAnimtionName;
}

Animation::~Animation()
{
	std::map<std::string,AnimationTrack*>::iterator it = mTrackList.begin();
	for(;it != mTrackList.begin(); it++)
		delete it->second;
}

AnimationTrack* Animation::createAnimationTrack(const std::string &boneName,bool ro,bool tr)
{
	if(mTrackList.find(boneName) == mTrackList.end())
	{
		AnimationTrack *track  = new AnimationTrack(tr,ro);
		mTrackList[boneName] = track;
		return track;
	}
	else
		return NULL;
}

//---------------------------------------------------------------------------------------------------
AnimationTrack* Animation::getAnimationTrack(const std::string &boneName) const
{
	std::map<std::string,AnimationTrack*>::const_iterator it = mTrackList.find(boneName);
	if(it != mTrackList.end())
	{
		return it->second;
 	}
	else
		return NULL;
}

size_t Animation::getTotalFrame() const
{
	return mTrackList.begin()->second->getTotalFrame();
}

const ResUtil::Matrix& Animation::getRawMotion() const
{
	return mRawmotion;
}
//---------------------------------------------------------------------------------------------------
AnimationState::AnimationState(const Animation* anim)
	:mAnimPointer(anim)
{
	mTotalFrame = mAnimPointer->getTotalFrame();
	mCurrentFrame = 0;
	mLoop = true;
	mEnabled = true;
}

void   AnimationState::attachAnimation(const Animation* anim)
{
	mAnimPointer = anim;
	mTotalFrame = anim->getTotalFrame();
	mCurrentFrame = 0;
	mLoop = true;
	mEnabled = true;
}

void  AnimationState::update(Ogre::SceneNode *node,std::string boneName)
{
	assert(mAnimPointer != NULL);
	const AnimationTrack* track = mAnimPointer->getAnimationTrack(boneName);
	if(track != NULL)
	{
		node->resetToInitialState();
		if(track->trsition())
			node->setPosition(track->getTransition(mCurrentFrame));
 		if(track->rotation())
		{
 			node->rotate(track->getRotation(mCurrentFrame));
		}
 	}
}

void  AnimationState::updataState()
{
	++mCurrentFrame;
	mTotalFrame = mAnimPointer->getTotalFrame();
	if(mCurrentFrame >= mTotalFrame)
	{
		if(mLoop)
			mCurrentFrame = 0;
		else
			--mCurrentFrame;
	}
}
template<> AnimationManager* Ogre::Singleton<AnimationManager>::ms_Singleton = 0;


AnimationManager::AnimationManager()
	:UIComponent("AnimationUI")
{
	OgreBites::TrayLocation tLoc = OgreBites::TL_BOTTOMRIGHT;
	mTrayMgr->createLabel(tLoc, "AnimMGR_TITLE" ,"AnimtionUI", 150);
	mTrayMgr->createSeparator(tLoc, "AnimMGR_SEPA1", 200);
	mTrayMgr->createThickSelectMenu(tLoc, "AnimMGR_Motion", "Motion", 250, 20);
	mTrayMgr->createButton(tLoc,"AnimMGR_Load","Load Motion",250);
	mTrayMgr->createButton(tLoc,"AnimMGR_Play","Play Motion",250);
	mTrayMgr->createButton(tLoc,"AnimMGR_Trajactory","Show Trajactory",250);
	mTrayMgr->createThickSlider(tLoc,"AnimMGR_Slider","degree",250,50,1,180,180);
	mTrayMgr->createButton(tLoc,"AnimMGR_Rotate","Rotate Motion",250);
	mTrayMgr->createButton(tLoc,"AnimMGR_Save","Save Motion",250);
  	mTrayMgr->createSeparator(tLoc, "AnimMGR_SEPA2", 150);
}
 
void AnimationManager::createAnimtion(ResUtil::Matrix *motion,std::string name)
{
 	Animation *anim = new Animation(motion,name);
 	 
	std::map<std::string,Animation*>::iterator it = mAvaliableAnimtion.find(name);
	if (it != mAvaliableAnimtion.end())
	{
		delete mAvaliableAnimtion[name];
		mAvaliableAnimtion.erase(it);
	}
	mAvaliableAnimtion[name] = anim;

	Ogre::StringVector sv = getAllAnimtion();
	OgreBites::SelectMenu * selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("AnimMGR_Motion");
	selMenu->setItems(sv);
}

AnimationManager::~AnimationManager()
{
	std::map<std::string,Animation*>::iterator it = mAvaliableAnimtion.begin();
	while (it != mAvaliableAnimtion.end())
	{
		delete it->second;
		it++;
	}
}

const Animation* AnimationManager::getAnimationByName(std::string name) const
{
	std::map<std::string,Animation*>::const_iterator it = mAvaliableAnimtion.find(name);
	if (it != mAvaliableAnimtion.end())
		return it->second;
	else 
		return NULL;
}

Ogre::StringVector AnimationManager::getAllAnimtion() const
{
	Ogre::StringVector sv;
	std::map<std::string,Animation*>::const_iterator it = mAvaliableAnimtion.begin();
	for (it ; it != mAvaliableAnimtion.end(); it++)
		sv.push_back(it->first);
	return sv;
}
void AnimationManager::buttonHit(OgreBites::Button* button)
{
 	if (button->getCaption() == "Load Motion")
	{
 		unsigned long hWnd;
		AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

		std::string filename = AncelApp::loadFile("dat",HWND(hWnd));

		if (filename != "")
		{
			ResUtil::Matrix *mat = ResUtil::loadData(filename);
			if (mat != NULL)
			{
				int pos = filename.find_last_of('\\') + 1;
				std::string motionName = filename.substr(pos);
				createAnimtion(mat,motionName);
			}
		}
 	}
	else if (button->getCaption() == "Play Motion")
	{
		OgreBites::SelectMenu* selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("AnimMGR_Motion");
		if (selMenu->getNumItems() > 0)
		{
 
			Skeleton *skel = SkeletonManager::getSingleton().getActivatedSkeleton();
			if(skel != NULL)
			{
				Ogre::String animationName = selMenu->getSelectedItem();
				const Animation * anim = getAnimationByName(animationName);

				AnimationState * animState = new AnimationState(anim);
				if(!skel->isAttachToScene())
				{
 					skel->attachToSceneMgr(AppDemo::getSingleton().mSceneMgr);
				}
				skel->attachAnimState(animState);
			}
			else
			{
				MessageBoxA(NULL,"Create Skeleton First","MSG",MB_OK);
			}
		}
	}
	else if(button->getCaption() == "Rotate Motion")
	{
		OgreBites::SelectMenu* selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("AnimMGR_Motion");
		if (selMenu->getNumItems() > 0)
		{
			Ogre::String animationName = selMenu->getSelectedItem();
			const Animation * anim = getAnimationByName(animationName);
			assert(anim);
			ResUtil::Matrix m = anim->getRawMotion();
			OgreBites::Slider *sl = (OgreBites::Slider*)mTrayMgr->getWidget("AnimMGR_Slider");
			
			double angle = sl->getValue();
			Ogre::Matrix3 rotMatrix; 
			Ogre::Quaternion Q;
  
			Q.FromAngleAxis(Ogre::Radian(Ogre::Degree(angle).valueRadians()),Ogre::Vector3(0,1,0));
 			Q.ToRotationMatrix(rotMatrix);

 			Ogre::Vector3 ori;
			ori.x = m.get(0,0);
			ori.y = m.get(0,1);
			ori.z = m.get(0,2);

			for(std::size_t i = 0; i < m.sizeRow(); i++)
			{
 				Ogre::Vector3 pos;
				pos.x = m.get(i,0);
				pos.y = m.get(i,1);
				pos.z = m.get(i,2);

				pos = pos - ori;
				pos = rotMatrix * pos;
				pos = pos + ori;

				m.assign(pos.x, i, 0);
				m.assign(pos.y, i, 1);
				m.assign(pos.z, i, 2);

				Ogre::Vector3 ro;
				ro.x = m.get(i,3);
				ro.y = m.get(i,4);
				ro.z = m.get(i,5);

				Ogre::Quaternion qu;
				Ogre::Matrix3 mat;
				mat.FromEulerAnglesZYX(Ogre::Radian(Ogre::Degree(ro.z).valueRadians()),
							           Ogre::Radian(Ogre::Degree(ro.y).valueRadians()),
							           Ogre::Radian(Ogre::Degree(ro.x).valueRadians()));
				qu.FromRotationMatrix(mat);

				qu = Q * qu;
				qu.ToRotationMatrix(mat);
				Ogre::Radian ax,ay,az;
				mat.ToEulerAnglesZYX(az,ay,ax);
				m.assign(ax.valueDegrees(),i,3);
				m.assign(ay.valueDegrees(),i,4);
				m.assign(az.valueDegrees(),i,5);
  			}
			Ogre::String newName =  Ogre::StringConverter::toString(int(angle));
			newName = anim->getAnimName() + "_rot_" + newName;
			createAnimtion(&m,newName);
  		}
	}
	else if(button->getCaption() == "Save Motion")
	{
		OgreBites::SelectMenu* selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("AnimMGR_Motion");
		if(selMenu->getNumItems() > 0)
		{
	 		Ogre::String animationName = selMenu->getSelectedItem();
			const Animation * anim = getAnimationByName(animationName);

			unsigned long hWnd;
			AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));
			std::string fileName = saveFile("dat",HWND(hWnd));
			if(fileName.find('.') == std::string::npos)
			{
				fileName += ".dat";
			}
			
			ResUtil::Matrix m = anim->getRawMotion();
			std::size_t row = m.sizeRow();
			std::size_t col = m.sizeCol();
			std::ofstream writer(fileName,std::ios::binary|std::ios::out);
			
			writer.write((char*)(&row), sizeof(std::size_t));
			writer.write((char*)(&col), sizeof(std::size_t));
			writer.write((char*)(m.gets()), sizeof(double)*m.getElemNum());
			
			writer.close();
  		}
 	}
	else if(button->getName() == "AnimMGR_Trajactory")
	{
		OgreBites::SelectMenu* selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("AnimMGR_Motion");
		Ogre::String animationName = selMenu->getSelectedItem();
		const Animation * anim = getAnimationByName(animationName);
		SkeletonManager::getSingleton().showTrajectory(anim);

 	}
}
void AnimationManager::sliderMoved(OgreBites::Slider * slider)
{
	/*if(slider->getCaption() == "speed")
	{
		double val = slider->getValue();
 	}*/
}