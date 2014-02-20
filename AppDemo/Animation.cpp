
#include "Animation.h"
#include "AppDemo.h"
#include "AnimationEditor.h"
using namespace AncelApp;

Animation::Animation(const Skeleton* skel, const Motion *mo)
	:mCurrentFrame(0),
  	 mUpdateSpeed(0.01),
	 mEnable(false),
	 timeSinceLastUpdate(0)
{
	mMotion = const_cast<Motion*>(mo);
	mSkeleton  = const_cast<Skeleton*>(skel);
	
	mEditor = new AnimationEditor(this);
 	mTotalFrame = mMotion->getTotalFrameNum();
	mAnimationName = mSkeleton->getName() + "_" + mMotion->getName();
}
Animation::~Animation()
{ 
	delete mEditor;
	mSkeleton->notifyReleased();
	mSkeleton->setPickEventHandle(nullptr);
}
void  Animation::update(double timeSinceLastFrame)
{
	//std::cout << mAnimationName << " " << mEnable << std::endl;
   	//-----------------------------------------------------------------------------
	std::vector<double> theta = mMotion->getFrame(mCurrentFrame);
	
	Ogre::Vector3 posShift(0,0,0);
	Ogre::Vector3 oriention;
	mMotion->rotateFrame(mCurrentFrame, mEditor->getRotationAngel(), posShift, oriention);
	
	theta[0] = posShift.x + mEditor->getRootShiftValue().x;
	theta[1] = posShift.y + mEditor->getRootShiftValue().y;
	theta[2] = posShift.z + mEditor->getRootShiftValue().z;

	theta[3] = oriention.x;
	theta[4] = oriention.y;
	theta[5] = oriention.z;

	//-----------------------------------------------------------------------------
	mSkeleton->update(theta, true);
  /*  Ogre::Vector3 pos1 = mSkeleton->getBone("lfoot")->getGlobalPos();
	Ogre::Vector3 pos2 = mSkeleton->getBone("rfoot")->getGlobalPos();
	
	Ogre::Vector3 rootPos(theta[0],theta[1],theta[2]);
	rootPos -= std::min(pos1.y,pos2.y) - 0.8;

	mMotion->setRootPosition(mCurrentFrame,rootPos);*/

 	if(mEnable)
	{
 		timeSinceLastUpdate += timeSinceLastFrame;
		if(timeSinceLastUpdate < mUpdateSpeed) 
			return;
 		timeSinceLastUpdate = 0;

		if(mCurrentFrame + 1 < mTotalFrame)  
			mCurrentFrame++;
		else if(mEnableLoop) 
			mCurrentFrame = 0;
	}
}

void  Animation::skipTo(const std::size_t& frameNum)
{
	if(frameNum < mTotalFrame)
	{
		mCurrentFrame = frameNum;
 	}
}
const std::string & Animation::getName() const
{
	return mAnimationName;
}

bool  Animation::updatePosition(const Ogre::Vector3 &pos)
{
	Ogre::Vector3 curPos = mMotion->getRootPosition(mCurrentFrame);
	mEditor->getRootShiftValue() = pos-curPos;
	mEditor->upateAuxSkeleton();
 	return true;
}

void Animation::applyShift()
{
	mMotion->shiftMotion(mEditor->getRootShiftValue());
}

void Animation::applyRotation()
{
	mMotion->rotateMotion(0,mEditor->getRotationAngel());
}
void Animation::upateActorTheme(const std::string& themeName)
{
	mSkeleton->updateMaterial(themeName);
}

