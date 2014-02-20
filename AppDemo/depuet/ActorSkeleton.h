#ifndef __ActorSkeleton_h
#define __ActorSkeleton_h

#include <inverse.h>
#include <skeleton\skeleton_types.h>
#include <math\math_basic_types.h>
#include "ActorBone.h"

namespace AncelApp
{
	typedef OpenTissue::math::default_math_types								 math_types;
	typedef EXBoneTraits<math_types>											 base_bone_traits;
	typedef OpenTissue::kinematics::inverse::BoneTraits<base_bone_traits>	     bone_traits;
	typedef OpenTissue::skeleton::Types<math_types,bone_traits>					 actor_types;
	typedef actor_types::skeleton_type                                           skeleton_type;
	typedef actor_types::bone_type                                               bone_type;
	
	/*enum ENTITY_QUERY_MASK 
	{
		EQM_NO_MASK = 1,
		EQM_TUBE_MASK = (1 << 1),
		EQM_SKLETON_MASK = (1 << 2)
 	};*/

	class ActorSkeleton: public skeleton_type
	{
	public:
		typedef math_types::quaternion_type	quaternion_type;
	public:
		ActorSkeleton(std::string skeletonName);
		~ActorSkeleton();

		void attachToScene(Ogre::SceneManager *sceneMgr);
		void removeFromScene();

		bool loadSkeletonFromXML(std::string skeletonFileName);
	protected:
		void createSkin(bone_type* bone);
	private:
		ActorSkeleton(const ActorSkeleton&);
		ActorSkeleton& operator= (const ActorSkeleton&);
 	protected:

 		bool mIsSkeletonBuilt;
		std::string mSkeletonName;
 		Ogre::SceneNode*		mRootNode;
		Ogre::SceneManager*     mBelongTo;
	};

	typedef OpenTissue::kinematics::inverse::NonlinearSolver<ActorSkeleton>	     solver_type;

}

#endif