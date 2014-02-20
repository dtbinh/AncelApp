#define TIXML_USE_STL
#define _SCL_SECURE_NO_WARNINGS

#include <tinyxml.h>
#include <OgreSceneNode.h>
#include "ActorSkeleton.h"

using namespace AncelApp;

ActorSkeleton::ActorSkeleton(std::string skeletonName)
	:mSkeletonName(skeletonName) ,mIsSkeletonBuilt(false)
{
}

ActorSkeleton::~ActorSkeleton()
{

}

void ActorSkeleton::attachToScene(Ogre::SceneManager *sceneMgr)
{
	if(mIsSkeletonBuilt)
	{
		mBelongTo = sceneMgr;
		createSkin(root());
	}
}
void ActorSkeleton::createSkin(bone_type* bone)
{
	if(bone->is_root())
	{
		bone->mHierarchicalNode = mBelongTo->getRootSceneNode()->createChildSceneNode(mSkeletonName + "/" + bone->get_name());
 	}
	else
		bone->mHierarchicalNode = bone->parent()->mHierarchicalNode->createChildSceneNode(mSkeletonName + "/" + bone->get_name());
	Ogre::Vector3 v(bone->bind_pose().T()[0], bone->bind_pose().T()[1], bone->bind_pose().T()[2]);
	bone->mHierarchicalNode->setPosition(v);
	Ogre::Quaternion Q(bone->bind_pose().Q().s(),bone->bind_pose().Q().v()[0], bone->bind_pose().Q().v()[1], bone->bind_pose().Q().v()[2]);
  	bone->mHierarchicalNode->setOrientation(Q);
   	bone->mHierarchicalNode->setInitialState();

	bone->mObjectNode = bone->mHierarchicalNode->createChildSceneNode(mSkeletonName + "/" + bone->get_name() + "/Obj");
	bone->mObjectNode->setOrientation(bone->mMeshOri);

	bone->mEntity = mBelongTo->createEntity(mSkeletonName + "/" + bone->get_name() + "/Entity", bone->mMeshName); //mBoneSet[boneID]->mBoneName+".mesh");
	bone->mEntity->setMaterialName("Skeleton/Bone/Default");
	bone->mEntity->setCastShadows(true);
	//bone->mEntity->setQueryFlags(EQM_SKLETON_MASK);

	bone->mObjectNode->attachObject(bone->mEntity);
	bone->mObjectNode->scale(bone->mBoneLength,bone->mBoneLength,bone->mBoneLength);

	bone_type::bone_ptr_iterator it = bone->child();

	for(int i = 0; i < bone->children(); i++)
	{
		createSkin(*it); 
		it++;
	}
}

void ActorSkeleton::removeFromScene()
{

}
bool ActorSkeleton::loadSkeletonFromXML(std::string skeletonFileName)
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
		double length = 0.0;

		boneName = attributes->Value();
		attributes = attributes->Next();
   		attributes->QueryIntValue(&parentID);
		attributes = attributes->Next();
		attributes->QueryIntValue(&boneID);
		attributes = attributes->Next();
	 	attributes->QueryDoubleValue(&length);
		attributes = attributes->Next();
	 	
		bone_type *bone = NULL;
		if(parentID == boneID)
		 	bone = create_bone();
		else
		{
			bone_type *parent = get_bone(parentID);
			bone = create_bone(parent);
		}
		bone->set_name(boneName);
		bone->mBoneLength = length;
		//transition
		math_types::vector3_type v3;
 		TiXmlElement *sibElement = boneElement->FirstChildElement();
		attributes = sibElement->FirstAttribute();
		attributes->QueryDoubleValue(&v3[0]);
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&v3[1]);
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&v3[2]);
 		bone->bind_pose().T() = v3;			
		//orientation

		double val;
		math_types::quaternion_type Q;
		math_types::vector3_type v;
		sibElement = sibElement->NextSiblingElement();
		attributes = sibElement->FirstAttribute();
	 	attributes->QueryDoubleValue(&Q.s());
 		attributes = attributes->Next();
		attributes->QueryDoubleValue(&v[0]);
 		attributes = attributes->Next();
		attributes->QueryDoubleValue(&v[1]);
 		attributes = attributes->Next();
		attributes->QueryDoubleValue(&v[2]);
 
		Q.v() = v;
		bone->bind_pose().Q() = Q;
		
		//mesh ori
		
		sibElement = sibElement->NextSiblingElement();
		attributes = sibElement->FirstAttribute();
		bone->mMeshName = attributes->Value();
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&val);
		bone->mMeshOri.w = val;
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&val);
		bone->mMeshOri.x = val;
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&val);
		bone->mMeshOri.y = val;
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&val);
		bone->mMeshOri.z = val;
	  
		sibElement = sibElement->NextSiblingElement();
		sibElement = sibElement->FirstChildElement();

 		attributes = sibElement->FirstAttribute();
		attributes->QueryDoubleValue(&bone->box_limits().min_limit(0));
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&bone->box_limits().max_limit(0));

		sibElement = sibElement->NextSiblingElement();
		attributes = sibElement->FirstAttribute();
		attributes->QueryDoubleValue(&bone->box_limits().min_limit(1));
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&bone->box_limits().max_limit(1));

		sibElement = sibElement->NextSiblingElement();
		attributes = sibElement->FirstAttribute();
		attributes->QueryDoubleValue(&bone->box_limits().min_limit(2));
		attributes = attributes->Next();
		attributes->QueryDoubleValue(&bone->box_limits().max_limit(2));
		
   		boneElement = boneElement->NextSiblingElement();
		bone->type() = bone_traits::ball_type;

	}
	OpenTissue::kinematics::inverse::set_joint_parameters(*this);
	
	delete skelDoc;
	//flag to indicate the skeleton is built
	mIsSkeletonBuilt = true;
	return true;
}