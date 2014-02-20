#ifndef _ActorBone_h
#define _ActorBone_h

#include <string>
#include <Ogre.h>
#include <skeleton\skeleton_default_bone_traits.h>

namespace AncelApp
{
	template<typename math_types_>
  	class EXBoneTraits: public OpenTissue::skeleton::DefaultBoneTraits<math_types_>
	{
	public:
		EXBoneTraits(){};
		~EXBoneTraits(){};
   	public:
	
		double					mBoneLength;
 		Ogre::Entity		   *mEntity;
		Ogre::Quaternion		mMeshOri;
		Ogre::String			mMeshName;
 		Ogre::String			mMaterialName;

		Ogre::SceneNode		   *mObjectNode;
		Ogre::SceneNode		   *mHierarchicalNode;
   	};
}
#endif