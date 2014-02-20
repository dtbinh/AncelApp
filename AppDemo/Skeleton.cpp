#define TIXML_USE_STL
#define _SCL_SECURE_NO_WARNINGS

#include <tinyxml.h>
#include <OgreSceneNode.h>
#include "Skeleton.h"
#include "MyGUI.h"
#include "AnimationEditorPanel.h"
using namespace AncelApp;
 

Skeleton::Skeleton(const std::string& skeletonName, const std::size_t type)
   :mRoot(NULL),
	mBelongTo(NULL),
	mSkeletonType(type),
	mIsBoneCreated(false),
	mActorNode(nullptr), 
	mPickEventHandle(nullptr)
	
{
	mSkeletonName = MyGUI::utility::toString(this) + "_" +  skeletonName;
}
Skeleton::~Skeleton()
{
	removeSkeletonFromScene();
	std::vector<Bone*>::iterator it = mBones.begin();
	for(it; it != mBones.end(); it++)
 	{
		delete *it;
	}
}
bool Skeleton::attachSkeletonToScene(Ogre::SceneManager *sceneMgr)
{
	if(!mBelongTo)
	{
		mBelongTo = sceneMgr;
		mRoot->attatchBoneToScene(mBelongTo, mSkeletonType);
		mActorNode = mBelongTo->getRootSceneNode()->createChildSceneNode(mSkeletonName + "RootNode");
		mActorNode->setPosition(mRoot->getGlobalPos());
 		return true;
	}
	return false;
}

void Skeleton::removeSkeletonFromScene()
{
	if(mBelongTo)
	{
		mRoot->removeFromScene(mBelongTo);
		mActorNode->detachAllObjects();
 		mBelongTo->destroySceneNode(mActorNode);
 		mBelongTo = NULL;
 	}
}

Bone*  Skeleton::createBone(Bone *parent)
{
	Bone* bone = NULL;

	if(!parent)
	{
		if(mRoot) throw std::invalid_argument("root bone already existed");
		bone = new Bone();
		mRoot = bone;
	}
	else
	{
 		bone = new Bone(parent);
		parent->addChild(bone);
	}

	bone->setBelongTo(this);
	bone->id() = mBones.size();
	mBones.push_back(bone);
	
	mIsBoneCreated = true;
	return bone;
}

bool  Skeleton::loadSkeletonFromXML(const std::string &skeletonFileName)
{
	TiXmlDocument* skelDoc = new TiXmlDocument();
	if(!skelDoc->LoadFile(skeletonFileName))
		throw std::logic_error("Load Skeleton File Failed !!");

	TiXmlElement *rootEle = skelDoc->FirstChildElement();
		
	TiXmlElement *bonesElement = static_cast<TiXmlElement*>(rootEle->FirstChild("bones")); 

	if(!bonesElement)
	 	throw std::invalid_argument("Invalid Skeleton File !!!");

	TiXmlElement *boneElement = bonesElement->FirstChildElement();

	while(boneElement)
	{
		TiXmlAttribute *attributes = boneElement->FirstAttribute();

		int parentID, boneID;
 		std::string boneName;
		float length = 0.0;

		boneElement->QueryIntAttribute(    "id", &boneID);
		boneElement->QueryIntAttribute("parent", &parentID);
		boneElement->QueryFloatAttribute( "length", &length);
		boneElement->QueryStringAttribute(  "name", &boneName);
  
		Bone *bone = NULL;
		if(parentID == boneID)
		 	bone = createBone();
		else
		{
			Bone *parent = getBone(parentID);
			bone = createBone(parent);
		}
		bone->name() = boneName;
		mBoneNameToID[boneName] = bone->id();
		
		bone->boneLength() = length;
		//transition
		
 		TiXmlElement *sibElement = boneElement->FirstChildElement();
		sibElement->QueryFloatAttribute("x", &bone->initPose().T.x);
		sibElement->QueryFloatAttribute("y", &bone->initPose().T.y);
		sibElement->QueryFloatAttribute("z", &bone->initPose().T.z);

	 	//orientation
 	 	sibElement = sibElement->NextSiblingElement();
		sibElement->QueryFloatAttribute("w", &bone->initPose().Q.w);
		sibElement->QueryFloatAttribute("x", &bone->initPose().Q.x);
		sibElement->QueryFloatAttribute("y", &bone->initPose().Q.y);
		sibElement->QueryFloatAttribute("z", &bone->initPose().Q.z);
  
	 	//mesh ori
 		sibElement = sibElement->NextSiblingElement();
		sibElement->QueryStringAttribute("name", &bone->meshName());
		sibElement->QueryFloatAttribute("w", &bone->meshOrientation().w);
		sibElement->QueryFloatAttribute("x", &bone->meshOrientation().x);
		sibElement->QueryFloatAttribute("y", &bone->meshOrientation().y);
		sibElement->QueryFloatAttribute("z", &bone->meshOrientation().z);

  		sibElement = sibElement->NextSiblingElement();
		int jointtype;
		sibElement->QueryIntAttribute("jointtype",&jointtype);

		bone->type() = static_cast<Bone::JointType>(jointtype);
		TiXmlElement *limitsElement = NULL;
	
		if(jointtype &0x01)
		{
			limitsElement = sibElement->FirstChildElement();
			limitsElement->QueryFloatAttribute("low", &bone->limitsBox(0).x);
			limitsElement->QueryFloatAttribute("up",  &bone->limitsBox(0).y);
			
			/*bone->limitsBox(0).x = -bone->limitsBox(0).x;
			bone->limitsBox(0).y = -bone->limitsBox(0).y;*/
 		}

		if(jointtype &0x02)
		{
			if(!limitsElement)
				limitsElement = sibElement->FirstChildElement();
			else
				limitsElement = limitsElement->NextSiblingElement();
			limitsElement->QueryFloatAttribute("low", &bone->limitsBox(1).x);
			limitsElement->QueryFloatAttribute( "up", &bone->limitsBox(1).y);

		/*	bone->limitsBox(1).x = -bone->limitsBox(1).x;
			bone->limitsBox(1).y = -bone->limitsBox(1).y;*/

		}

		if(jointtype &0x04)
		{
			if(!limitsElement)
				limitsElement = sibElement->FirstChildElement();
			else
				limitsElement = limitsElement->NextSiblingElement();
			limitsElement->QueryFloatAttribute("low", &bone->limitsBox(2).x);
			limitsElement->QueryFloatAttribute( "up", &bone->limitsBox(2).y);

		/*	bone->limitsBox(2).x = -bone->limitsBox(2).x;
			bone->limitsBox(2).y = -bone->limitsBox(2).y;*/


		}
		
		boneElement = boneElement->NextSiblingElement();
 	}

 	delete skelDoc;
  	mIsBoneCreated = true;
	return true;
}
void Skeleton::update(const std::vector<double> &theta, bool updateSceneNode)
{
#define NUMBONE 19

	if(mIsBoneCreated)
	{
 		static std::string boneName[NUMBONE] = {"root","lowerback","upperback","thorax","lowerneck","upperneck","head","rhumerus","rradius",
									            "rwrist","lhumerus","lradius","lwrist","rfemur","rtibia","rfoot","lfemur","ltibia","lfoot"};
 		std::vector<double>::const_iterator it = theta.begin();

		for(std::size_t i = 0; i < NUMBONE; i++)
		{
			Bone* bone = getBone(boneName[i]);
			bone->update(it,updateSceneNode);
		}
		//if(!updateSceneNode)
			mRoot->computePosition();
		mActorNode->setPosition(mRoot->getGlobalPos());
 	}
}

void Skeleton::update2(const std::vector<double> &theta, bool updateSceneNode)
{
#define NUMBONE 19

	if(mIsBoneCreated)
	{
		static std::string boneName[NUMBONE] = {"root","lowerback","upperback","thorax","lowerneck","upperneck","head","rhumerus","rradius",
			"rwrist","lhumerus","lradius","lwrist","rfemur","rtibia","rfoot","lfemur","ltibia","lfoot"};
		std::vector<double>::const_iterator it = theta.begin();

		for(std::size_t i = 0; i < NUMBONE; i++)
		{
			Bone* bone = getBone(boneName[i]);
			bone->update(it,updateSceneNode);
		}
		//if(!updateSceneNode)
		mRoot->computePosition();
		//mActorNode->setPosition(mRoot->getGlobalPos());
	}
}

void Skeleton::getSkeletonParameters(std::vector<double> &theta) const
{
#define NUMBONE 19
		static std::string boneName[NUMBONE] = {"root","lowerback","upperback","thorax","lowerneck","upperneck","head","rhumerus","rradius",
									            "rwrist","lhumerus","lradius","lwrist","rfemur","rtibia","rfoot","lfemur","ltibia","lfoot"};
	if(mIsBoneCreated)
	{
		std::vector<double>::iterator it = theta.begin();
		*it++ = mRoot->getGlobalPos().x;
		*it++ = mRoot->getGlobalPos().y;
		*it++ = mRoot->getGlobalPos().z;

		for(std::size_t i = 0; i < NUMBONE; i++)
		{
			const Bone* bone = getBone(boneName[i]);
			bone->getTheta(it);
		}
	}
}
void   Skeleton::computeSkeletonLimitsProjection(std::vector<double> &theta)
{
 	if(mIsBoneCreated)
	{
 	    std::vector<double>::iterator it = theta.begin();

		for(std::size_t i = 0; i < mBones.size(); i++)
		{
			mBones[i]->computeJointLimitsProjection(it);
		}
 	}
}
void  Skeleton::setThetaParameters(const std::vector<double> &theta, bool updateSceneNode)
{
	if(mIsBoneCreated)
	{
		std::vector<double>::const_iterator theta_it = theta.begin();

		std::vector<bone_type*>::iterator it = mBones.begin();

		for(it; it != mBones.end(); it++)
		{
			(*it)->setTheta(theta_it,true);
		}
		mRoot->computePosition();
	}
}
void  Skeleton::getThetaParameters(std::vector<double> &theta) const
{
	if(mIsBoneCreated)
	{
		std::vector<double>::iterator theta_it = theta.begin();

		std::vector<bone_type*>::const_iterator it = mBones.begin();

		for(it; it != mBones.end(); it++)
		{
			(*it)->getTheta(theta_it);
		}
	}
}

std::size_t Skeleton::getTotalFreedom() const
{
	if(mIsBoneCreated)
	{
		std::size_t totalFreedom = 0;
		std::vector<bone_type*>::const_iterator it = mBones.begin();
		for(it; it != mBones.end(); it++)
		{
			totalFreedom += (*it)->getActiveDofs();
	//		std::cout <<(*it)->name() << ": " << (*it)->getActiveDofs() << std::endl;
		}
 		return totalFreedom;
	}
	return 0;
}
void Skeleton::setVisibility(bool flag)
{
 	std::vector<bone_type*>::const_iterator it = mBones.begin();
	for(it; it != mBones.end(); it++)
	{
		(*it)->setVisibility(flag);
	}
}

std::string	 Skeleton::getName() const 
{
  	return mSkeletonName;
}

void Skeleton::setPickEventHandle(PickableObject *handle) 
{
	mPickEventHandle = handle;
}

bool Skeleton::cloneFrom(const Skeleton& skel)
{
	if(mIsBoneCreated)  return false;
  
	createBone(NULL);
	mRoot->cloneFrom(*skel.mRoot);
 	
	mSkeletonType = skel.mSkeletonType;
 	mSkeletonName = MyGUI::utility::toString(this) + skel.mSkeletonName.substr(skel.mSkeletonName.find_first_of('_'));
  	 	
	for(std::size_t i = 1; i < skel.mBones.size(); i++)
	{
		createBone(mBones[skel.mBones[i]->getParent()->id()]);
		mBones[i]->cloneFrom(*skel.mBones[i]);
	}
   	mBoneNameToID = skel.mBoneNameToID;
 
	return false;
}

bool  Skeleton::notifyReleased()
{
	if(mPickEventHandle != nullptr)
	{
		AnimationEditorPanel::getSingletonPtr()->bindAnimationEditor(nullptr);
		AnimationEditorPanel::getSingletonPtr()->setVisible(false);
		mRoot->showBoundingBox(false);
	}
	return true;
}
bool  Skeleton::notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
{
	if(evt.state.buttonDown(OIS::MB_Left))
	{
		Ogre::Vector3 shift = ray.getPoint(mCollsionDepth);
		
		shift -= mPressedPosition;
		
		//pos 
		if(AnimationEditorPanel::getSingletonPtr()->isFixXAxis()) shift.x = 0;
		if(AnimationEditorPanel::getSingletonPtr()->isFixYAxis()) shift.y = 0;
		if(AnimationEditorPanel::getSingletonPtr()->isFixZAxis()) shift.z = 0;
 		
		mPressedPosition += shift;

		AnimationEditor* editor = static_cast<AnimationEditor*>(mPickEventHandle);
		editor->updateShiftValue(shift);
	}
 	return true;
}
bool  Skeleton::mousePressed(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
{
	mPressedPosition = ray.getPoint(mCollsionDepth);
	return true;
}
bool  Skeleton::notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset)
{
	if(mPickEventHandle != nullptr)
	{
		mCollsionDepth = entry.distance;
		mPickedNodeOffset =  mRoot->getGlobalPos() - getBone("head")->getGlobalPos() + offset;
		AnimationEditor* editor = static_cast<AnimationEditor*>(mPickEventHandle);
		AnimationEditorPanel::getSingletonPtr()->bindAnimationEditor(editor);
		AnimationEditorPanel::getSingletonPtr()->setVisible(true);
		mRoot->showBoundingBox(true);
		editor->pauseAnimation();
		return true;
	}
	return false;
}
 		
void  Skeleton::updateMaterial(std::string materialName)
{
	for(std::size_t i = 0; i < mBones.size(); i++)
		mBones[i]->updateMaterial(materialName);
}
