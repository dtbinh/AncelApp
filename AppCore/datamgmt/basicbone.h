#ifndef __basicbone_h_
#define __basicbone_h_
 
#include <string>
#include <Eigen\Eigen>
#include <vector>

using namespace Eigen;
class GPCMBasicSkeleton;

class GPCMBasicBone
{
public:
		GPCMBasicBone(const GPCMBasicBone* parent = NULL);
		virtual ~GPCMBasicBone();


		bool isRoot() const		{return (!mParent);}
		bool isLeaf() const		{return mChildren.empty();}

		std::string& name()		{return mBoneName;}
protected:
		 
	 	GPCMBasicSkeleton*				mBelongTo;			//the skeleton pointer which the bone belong to
		const GPCMBasicBone*			mParent;			//the bone's parent bone pointer
		std::vector<GPCMBasicBone*>     mChildren;			//the children pointer array

		std::string						mBoneName;			//bone name
		Vector3d						mRelativePostion;
		Eigen::Quaterniond	        	mLocalAxis;
		
		Vector3i						mRotOrder;
};

#endif