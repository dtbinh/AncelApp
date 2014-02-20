#include "_Skeleton.h"
#include <fstream>
#include <string>
#include "AppDemo.h"


using namespace AncelApp;
//------------------------------------------------------------------------------------------

Skeleton::Skeleton(Ogre::String skelName)
	:mSkeletonName(skelName),
	mRootID(0),
 	mBelongTo(NULL),
	mAnimState(NULL),
	mTimeCount(0),
	mTimeInterval(100),
	mTheme("Skeleton/Bone/Default")

{
}
Skeleton::~Skeleton()
{
 	if(mBoneSet.size())
	{
		if(mBelongTo != NULL)
			removeFromSceneMgr();
		for(size_t i = 0; i < mBoneSet.size(); i++)
			delete mBoneSet[i];
	}
}

bool Skeleton::loadSkeleton(Ogre::String fileName)
{
	std::ifstream loader(fileName);
	if(loader.fail())
		return false;

	//--------------------------------------
	// Bone format description:
	// ID Name Lenght Positon Oriention Root? childNum child1 child2 child3.............
	// ID Name Lenght Positon Oriention Root? childNum child1 child2 ...................
	//--------------------------------------

	int totalBones;
	loader >> totalBones;

	for(int i = 0; i < totalBones; i++)
	{
		BoneNode *node = new BoneNode();
		
		//TODO
		//bool root;
		int childNum;

		loader >> node->mBoneID;
	 	loader >> node->mBoneName;
		loader >> node->mBoneLength;
		
		loader >> node->mPostion.x;
		loader >> node->mPostion.y;
		loader >> node->mPostion.z;

		loader >> node->mMeshOri.w;
		loader >> node->mMeshOri.x;
		loader >> node->mMeshOri.y;
		loader >> node->mMeshOri.z;
 		loader >> node->mMeshName;

		loader >> node->mLocalAxis.w;
		loader >> node->mLocalAxis.x;
		loader >> node->mLocalAxis.y;
		loader >> node->mLocalAxis.z;
 
		//	loader >>  root;
	//	if(root) mRootID = node->mBoneID;
		
		loader >> childNum;
		for(int j = 0; j < childNum;j++)
		{
			int child;
			loader >> child;
			node->mChildID.push_back(child);
		}
		mBoneSet.push_back(node);
 	}

	for(size_t i = 0; i < mBoneSet.size(); i++)
	{
		for(size_t j = 0; j < mBoneSet[i]->mChildID.size(); j++)
		{
			mBoneSet[mBoneSet[i]->mChildID[j]]->mParentID = mBoneSet[i]->mBoneID;
		}
	}
	mRootID = 0;
	return true;
}

void Skeleton::attachToSceneMgr(Ogre::SceneManager *sceneMgr)
{
	if(mBelongTo != NULL)
	{
		removeFromSceneMgr(mRootID);
		mBelongTo->destroySceneNode(mRootNode);
	}

	mBelongTo = sceneMgr;
 	mRootNode = mBelongTo->getRootSceneNode()->createChildSceneNode(mSkeletonName + "/RootNode");
	createHierarchicalSkeleton(mRootID);
	Ogre::Vector3 pos = getBonePosition("rfoot");
	mBoneSet[mRootID]->mHierarchicalNode->setPosition(Ogre::Vector3(0,-pos.y,0));
}

double& Skeleton::getTimeInterval()
{
	return mTimeInterval;
}
Ogre::Vector3 Skeleton::getBonePosition(Ogre::String BoneName)
{
	std::vector<BoneNode*>::iterator it = mBoneSet.begin();
	for (it; it != mBoneSet.end(); it++)
	{
		if((*it)->mBoneName == BoneName)
		{
			Ogre::Vector3 pos =   Ogre::Vector3(0.0f,-0.7f,-0.3f);
			                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
			pos = (*it)->mObjectNode->convertLocalToWorldPosition(pos);
			return pos;
		}
	}
	return  Ogre::Vector3(0,0,0);
}
void Skeleton::createHierarchicalSkeleton(int boneID)
{
	if(boneID == mRootID)
	{
		mBoneSet[boneID]->mHierarchicalNode = mBelongTo->getRootSceneNode()->createChildSceneNode(mSkeletonName + "/" + mBoneSet[boneID]->mBoneName);
 		mRootNode->setPosition(mBoneSet[boneID]->mHierarchicalNode->getPosition());
	}
  	else 
	 	mBoneSet[boneID]->mHierarchicalNode = mBoneSet[mBoneSet[boneID]->mParentID]->mHierarchicalNode->createChildSceneNode(mSkeletonName + "/" + mBoneSet[boneID]->mBoneName);
  	
	mBoneSet[boneID]->mHierarchicalNode->setPosition(mBoneSet[boneID]->mPostion);
  	mBoneSet[boneID]->mHierarchicalNode->setOrientation(mBoneSet[boneID]->mLocalAxis);
   	mBoneSet[boneID]->mHierarchicalNode->setInitialState();
	//Ogre::Entity *ent = mBelongTo->createEntity(mBoneSet[boneID]->mBoneName + "Axis","Cube.mesh");
	//mBoneSet[boneID]->mHierarchicalNode->attachObject(ent);
	
	mBoneSet[boneID]->mObjectNode = mBoneSet[boneID]->mHierarchicalNode->createChildSceneNode(mSkeletonName + "/" + mBoneSet[boneID]->mBoneName+ "/Obj");
	mBoneSet[boneID]->mObjectNode->setOrientation(mBoneSet[boneID]->mMeshOri);
 
	mBoneSet[boneID]->mEntity = mBelongTo->createEntity(mSkeletonName + "/" + mBoneSet[boneID]->mBoneName + "/Entity",mBoneSet[boneID]->mMeshName); //mBoneSet[boneID]->mBoneName+".mesh");
	mBoneSet[boneID]->mEntity->setMaterialName(mTheme);
	mBoneSet[boneID]->mEntity->setCastShadows(true);
	mBoneSet[boneID]->mEntity->setQueryFlags(EQM_SKLETON_MASK);
 	
	mBoneSet[boneID]->mObjectNode->attachObject(mBoneSet[boneID]->mEntity);
	mBoneSet[boneID]->mObjectNode->scale(mBoneSet[boneID]->mBoneLength,mBoneSet[boneID]->mBoneLength,mBoneSet[boneID]->mBoneLength);

	//if(mBoneSet[boneID]->mBoneLength > 0);
	//	mBoneSet[boneID]->mObjectNode->scale(mBoneSet[boneID]->mBoneLength,mBoneSet[boneID]->mBoneLength,mBoneSet[boneID]->mBoneLength);
	for(size_t i = 0; i < mBoneSet[boneID]->mChildID.size(); i++)
	{
		createHierarchicalSkeleton(mBoneSet[boneID]->mChildID[i]);
	}
}

void Skeleton::removeFromSceneMgr(int boneID)
{
	if(mBoneSet[boneID]->mChildID.size())
	{
		for(size_t i = 0; i < mBoneSet[boneID]->mChildID.size(); i++)
		{
			removeFromSceneMgr(mBoneSet[boneID]->mChildID[i]);
		}
 	}
	if(mBoneSet[boneID]->mHierarchicalNode != NULL)
	{
		mBoneSet[boneID]->mObjectNode->detachObject(mBoneSet[boneID]->mEntity);
	
		mBelongTo->destroyEntity(mBoneSet[boneID]->mEntity->getName());
		mBelongTo->destroySceneNode(mBoneSet[boneID]->mObjectNode);
 		mBelongTo->destroySceneNode(mBoneSet[boneID]->mHierarchicalNode);
	
		mBoneSet[boneID]->mEntity = NULL;
		mBoneSet[boneID]->mHierarchicalNode = NULL;
	}
}

void Skeleton::attachAnimState(AnimationState *animState)
{
	if(mAnimState != NULL)
		delete mAnimState;
	mAnimState = animState;
}

Ogre::SceneNode* Skeleton::getRootNode()
{
	if(mBelongTo != NULL)
	{
		return mRootNode;
	}
	return NULL;
}

void Skeleton::update(double timeSinceLastFrame)
{
  	mTimeCount += timeSinceLastFrame*1000;
	if(mAnimState != NULL && mAnimState->getEnabled()&& mTimeCount > mTimeInterval)
	{ 
		update(mRootID);
		mAnimState->updataState();
		mTimeCount = 0;
		mRootNode->setPosition(mBoneSet[mRootID]->mHierarchicalNode->getPosition());
	}
}

void Skeleton::update(int boneID)
{
	mAnimState->update(mBoneSet[boneID]->mHierarchicalNode,mBoneSet[boneID]->mBoneName);
	if(mBoneSet[boneID]->mChildID.size())
	{
		for(size_t i = 0; i < mBoneSet[boneID]->mChildID.size(); i++)
		{
			update(mBoneSet[boneID]->mChildID[i]);
		}
 	}
}
void Skeleton::update(const Animation *anim,std::size_t frameNum)
{
	assert(frameNum < anim->getTotalFrame());
	update(mRootID,anim,frameNum);
}
void Skeleton::update(int boneID, const Animation *anim, std::size_t frameNum)
{
	AnimationTrack *animTrack = anim->getAnimationTrack(mBoneSet[boneID]->mBoneName);
	if(animTrack != NULL)
	{
		mBoneSet[boneID]->mHierarchicalNode->resetToInitialState();
		if(animTrack->trsition())
			mBoneSet[boneID]->mHierarchicalNode->setPosition(animTrack->getTransition(frameNum));
 		if(animTrack->rotation())
		{
 			mBoneSet[boneID]->mHierarchicalNode->rotate(animTrack->getRotation(frameNum));
		}
	}

	if(mBoneSet[boneID]->mChildID.size())
	{
		for(size_t i = 0; i < mBoneSet[boneID]->mChildID.size(); i++)
		{
			update(mBoneSet[boneID]->mChildID[i],anim,frameNum);
		}
	}
}
void Skeleton::removeFromSceneMgr()
{
	removeFromSceneMgr(mRootID);
	mBelongTo->destroySceneNode(mRootNode);
	mBelongTo = NULL;
}

bool Skeleton::isAttachToScene()
{
	return (mBelongTo != NULL);
}

void Skeleton::setTheme(const Ogre::String &theme)
{
	mTheme = theme;
	std::vector<BoneNode*>::iterator it = mBoneSet.begin();
	for (it ; it != mBoneSet.end(); it++)
	{
		(*it)->mEntity->setMaterialName(theme);
	}
}

Ogre::String Skeleton::getTheme() const
{
	return mTheme;
}

//------------------------------------------------------------------------------------------
 
template<> SkeletonManager* Ogre::Singleton<SkeletonManager>::ms_Singleton = 0;
  
SkeletonManager::SkeletonManager()
	:UIComponent("SkeletonUI"),mTrackingTarget("")
{
	OgreBites::TrayLocation tLoc = OgreBites::TL_TOPLEFT;
	mTrayMgr->createLabel(tLoc, "SKLEMGR_TITLE" ,"SkeletonUI", 150);
	mTrayMgr->createSeparator(tLoc, "SKLEMGR_SEPA1", 200);

	Ogre::StringVector sv;
	sv.push_back("data\\skeleton\\HDM_bdd.txt");
	sv.push_back("data\\skeleton\\HDM_bkk.txt");
	sv.push_back("data\\skeleton\\HDM_dgg.txt");
	sv.push_back("data\\skeleton\\HDM_mmm.txt");
	sv.push_back("data\\skeleton\\HDM_trr.txt");


	mTrayMgr->createThickSelectMenu(tLoc, "SKELMGR_Template", "Template", 250, 20, sv);
	mTrayMgr->createButton(tLoc,"SKELMGR_BTCreate","create skelton",250);
	mTrayMgr->createThickSelectMenu(tLoc, "SKELMGR_Actor", "Actor", 250, 20);
	mTrayMgr->createThickSlider(tLoc, "SKELMGR_Speed","speed", 250, 50, 10, 500, 50);
	
	sv.clear();
 	sv.push_back("Skeleton/Bone/Bronze");
	sv.push_back("Skeleton/Bone/Default");
	sv.push_back("Skeleton/Bone/PolishedBronze");
	sv.push_back("Skeleton/Bone/Chrome");
	sv.push_back("Skeleton/Bone/Copper");
	sv.push_back("Skeleton/Bone/PolishedCopper");
	sv.push_back("Skeleton/Bone/Gold");
	sv.push_back("Skeleton/Bone/PolishedGold");
	sv.push_back("Skeleton/Bone/Pewter");
	sv.push_back("Skeleton/Bone/Silver");
	sv.push_back("Skeleton/Bone/PolishedSilver");
	sv.push_back("Skeleton/Bone/Emerald");
	sv.push_back("Skeleton/Bone/Jade");
	sv.push_back("Skeleton/Bone/Obsidian");
	sv.push_back("Skeleton/Bone/Pearl");
	sv.push_back("Skeleton/Bone/Ruby");
	sv.push_back("Skeleton/Bone/Turquoise");
	sv.push_back("Skeleton/Bone/BlackPlastic");
	sv.push_back("Skeleton/Bone/BlackRubber");
 
	mTrayMgr->createThickSelectMenu(tLoc, "SKELMGR_Theme", "Theme", 250, 20,sv);
  	mTrayMgr->createSeparator(tLoc, "SKLEMGR_SEPA2", 150); 


	//createSkeleton("HDM_bdd","data\\skeleton\\HDM_bdd.txt");
	//createSkeleton("HDM_bkk","data\\skeleton\\HDM_bkk.txt");
	//createSkeleton("HDM_dgg","data\\skeleton\\HDM_dgg.txt");
	//createSkeleton("HDM_mmm","data\\skeleton\\HDM_mmm.txt");
	//createSkeleton("HDM_trr","data\\skeleton\\HDM_trr.txt");
	//createSkeleton("08","data\\skeleton\\08.txt");

	//createSkeleton("16", "data\\motion\\MOG\\skeleton\\16.txt");
	//createSkeleton("86", "data\\motion\\MOG\\skeleton\\86.txt");
	//createSkeleton("105","data\\motion\\MOG\\skeleton\\105.txt");
	//createSkeleton("142","data\\motion\\MOG\\skeleton\\142.txt");

	//attachSkeletonToSceneMgr("HDM_bdd",AppDemo::getSingleton().mSceneMgr);
	//attachSkeletonToSceneMgr("HDM_bkk",AppDemo::getSingleton().mSceneMgr);
	//attachSkeletonToSceneMgr("HDM_trr",AppDemo::getSingleton().mSceneMgr);
	//attachSkeletonToSceneMgr("142",AppDemo::getSingleton().mSceneMgr);

  /*attachSkeletonToSceneMgr("HDM_bkk",AppDemo::getSingleton().mSceneMgr);
  	attachSkeletonToSceneMgr("HDM_dgg",AppDemo::getSingleton().mSceneMgr);
	attachSkeletonToSceneMgr("HDM_mmm",AppDemo::getSingleton().mSceneMgr);*/
}
SkeletonManager::~SkeletonManager()
{
  	std::map<std::string,Skeleton*>::iterator it = mSkeletons.begin();
	while(it != mSkeletons.end())
	{
		delete it->second;
		it++;
 	}
}
bool SkeletonManager::createSkeleton(std::string skelName,std::string  fileName)
{
 	std::map<std::string,Skeleton*>::iterator it = mSkeletons.find(skelName);
	if(it != mSkeletons.end()) return false;

	Skeleton *skel = new Skeleton(skelName);

	if(skel->loadSkeleton(fileName))
	{
 		mSkeletons[skelName] = skel;
 	}
	else 
	{
		delete skel;
		return false;
	}
	Ogre::StringVector sv = getSkeletonName();
	OgreBites::SelectMenu * selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("SKELMGR_Actor");
	selMenu->setItems(sv);
	return true;
}
bool SkeletonManager::removeSkeleton(std::string skelName)
{
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.find(skelName);
	if(it != mSkeletons.end())
	{
		if(mTrackingTarget == skelName)
			mTrackingTarget == "";
		delete it->second;
		mSkeletons.erase(it);
		return true;
	}
 	return false;
}
//--------------------------------------------------------------------------------------
bool SkeletonManager::attachSkeletonToSceneMgr(std::string skel,Ogre::SceneManager* sceneMgr)
{
 	std::map<std::string,Skeleton*>::iterator it = mSkeletons.find(skel);
	if(it != mSkeletons.end())
	{
		it->second->attachToSceneMgr(sceneMgr);
		return true;
	}
	return false;
}
bool SkeletonManager::removeSkeletonFromSceneMgr(std::string skel)
{
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.find(skel);
	if(it != mSkeletons.end())
	{
		it->second->removeFromSceneMgr();
		return true;
	}
	return false;
}
bool SkeletonManager::removeAll()
{
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.begin();
	for (it; it != mSkeletons.end(); it++)
	{
		if(it->second->isAttachToScene())
			it->second->removeFromSceneMgr();
	}
	return false;
}

void SkeletonManager::update(const Ogre::FrameEvent& evt)
{
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.begin();
	for (it; it != mSkeletons.end(); it++)
	{
		it->second->update(evt.timeSinceLastFrame);
	}
}

Skeleton* SkeletonManager::findByName(std::string skel)
{
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.find(skel);
	if(it != mSkeletons.end())
	{
		return it->second;
 	}
	return NULL;
}
bool SkeletonManager::setTrackingTarget(std::string skel)
{
 	std::map<std::string,Skeleton*>::iterator it = mSkeletons.find(skel);
	if(it != mSkeletons.end())
	{
		mTrackingTarget = skel;
		return true;
 	}
	return false;
}
Ogre::SceneNode* SkeletonManager::getTrackingTarget()
{
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.find(mTrackingTarget);
	if(it != mSkeletons.end())
	{
 		return it->second->getRootNode();
 	}
	return NULL;
}
Ogre::StringVector SkeletonManager::getSkeletonName()
{
	Ogre::StringVector nameList;
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.begin();
	for(it ;it != mSkeletons.end(); it++)
	{
 		nameList.push_back(it->first);
	}
  	return nameList;
}
void SkeletonManager::buttonHit(OgreBites::Button* button)
{
	if(button->getName() == "SKELMGR_BTCreate")
	{
		std::string templateName = getActiveTemplate();
		std::string skelName = templateName.substr(templateName.find_last_of('\\') + 1);
		::SetCurrentDirectoryA(AppDemo::getSingleton().mWorkDirectory.c_str());
		createSkeleton(skelName,templateName);
		attachSkeletonToSceneMgr(skelName,AppDemo::getSingleton().mSceneMgr);
	}
}

/**
*@return 1.if skeleton is existed,then return the selected skeleton
		 2.else return NULL, which means that no skeleton is avaliable.
*/
Skeleton* SkeletonManager::getActivatedSkeleton()
{
	OgreBites::SelectMenu *selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("SKELMGR_Actor");
	if (selMenu->getNumItems() > 0)
	{
		return findByName(selMenu->getSelectedItem());
	}
	return NULL;
}
std::string SkeletonManager::getActiveTemplate()
{
	OgreBites::SelectMenu *selMenu = (OgreBites::SelectMenu*)mTrayMgr->getWidget("SKELMGR_Template");
	if(selMenu->getNumItems() > 0)
	{
		return selMenu->getSelectedItem();
	}
	return "";
}
void SkeletonManager::sliderMoved(OgreBites::Slider * slider)
{
	if (slider->getCaption() == "speed")
	{
		double val = slider->getValue();
		OgreBites::SelectMenu* sm = (OgreBites::SelectMenu*)mTrayMgr->getWidget("SKELMGR_Actor");
		string item = sm->getSelectedItem();
		Skeleton *skel = getActivatedSkeleton();
		if (skel != NULL)
			skel->getTimeInterval() = val;
	}
}
void SkeletonManager::itemSelected(OgreBites::SelectMenu *menu)
{
	if (menu->getCaption()== "Actor")
	{
		Skeleton *skel = getActivatedSkeleton();
		OgreBites::Slider * slider = (OgreBites::Slider*)mTrayMgr->getWidget("SKELMGR_Speed");
		slider->setValue(skel->getTimeInterval(),false);
		OgreBites::SelectMenu* sm = (OgreBites::SelectMenu*)mTrayMgr->getWidget("SKELMGR_Theme");
		sm->selectItem(skel->getTheme(),false);
 	}
	else if (menu->getCaption()=="Theme")
	{
		Skeleton *skel = getActivatedSkeleton();
		if(skel != NULL)
		{
			skel->setTheme(menu->getSelectedItem());
		}
 	}
}
void SkeletonManager::showTrajectory(const Animation *anim)
{
	std::size_t totalFrame = anim->getTotalFrame();
	std::size_t step = 5;
	std::string templateName = getActiveTemplate();
	std::string skelName = templateName.substr(templateName.find_last_of('\\') + 1);
	for (std::size_t i = 0; i < totalFrame; i+= step)
	{
		std::string name = anim->getAnimName() + "_"  + skelName  + "_" + Ogre::StringConverter::toString(i);
		::SetCurrentDirectoryA(AppDemo::getSingleton().mWorkDirectory.c_str());
 		
		if(!createSkeleton(name,templateName)) return;
		
		attachSkeletonToSceneMgr(name,AppDemo::getSingleton().mSceneMgr);
		Skeleton* skel= findByName(name);
		skel->update(anim,i);
	}
}
void SkeletonManager::destoryTrajectory(const std::string name)
{
	std::map<std::string,Skeleton*>::iterator it = mSkeletons.begin();

	for(it ; it != mSkeletons.end(); it++)
	{
		if(it->first.find(name) != std::string::npos)
		{
			it = mSkeletons.erase(it);
			if(it != mSkeletons.begin())
				it--;
		}
	}
}
