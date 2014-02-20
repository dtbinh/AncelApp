#ifndef __basicskeleton_h_
#define __basicskeleton_h_
#include <map>
#include <vector>
#include <string>


template<class bone_type>
class GPCMBasicSkeleton
{
public:
	GPCMBasicSkeleton(const std::string& skeletonName);
	~GPCMBasicSkeleton();
	bool load(std::string filename);
protected:
	bone_type*		           mRoot;
	std::vector<bone_type*>	   mBones;
	std::map<std::string, int> mBoneNameToID;
   
	std::string			       mSkeletonName;
	bool				       mIsBoneCreated;  
};


template<class bone_type>
GPCMBasicSkeleton<bone_type>::GPCMBasicSkeleton(const std::string& skeletonName)
{

}
template<class bone_type>
bool GPCMBasicSkeleton<bone_type>::load(std::string filename)
{
	return true;
}

#endif
