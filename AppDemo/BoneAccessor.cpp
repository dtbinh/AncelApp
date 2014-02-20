#include "BoneAccessor.h"

using namespace AncelIK;

void BoneAccessor::computeJacobian(const IKChain* chain,const AncelApp::Bone* bone,IKSolver::real_matrix_iter col_iter, IKSolver::real_vector_iter row_iter)
{
	if(bone->type() == AncelApp::Bone::JT_NR) return;
	
	Ogre::Vector3 pos   = chain->getEndEffector()->getGlobalOri() * chain->localPosition() + chain->getEndEffector()->mAbsolute.T;
	Ogre::Vector3 axisX = chain->getEndEffector()->getGlobalOri() * chain->localAxisX();
	Ogre::Vector3 axisY = chain->getEndEffector()->getGlobalOri() * chain->localAxisY();
	
//	std::cout << pos << std::endl;

	pos = pos - bone->mAbsolute.T;
 
 	int distance = std::distance(col_iter->begin(), row_iter);
 
	IKSolver::real_matrix_iter cIter = col_iter;
	IKSolver::real_vector_iter rIter = row_iter;

	if(bone->mJointType & 1)
	{
 		Ogre::Vector3 w(1, 0, 0);
		if(bone->mJointType & 2)
		{
			Ogre::Matrix3 r;
			r.FromAngleAxis(Ogre::Vector3(0, 1, 0), Ogre::Radian(Ogre::Degree(bone->mTheta[4])));
 			w = r * w;
 		}
		
		if(bone->mJointType & 4)
		{
			Ogre::Matrix3 r;
			r.FromAngleAxis(Ogre::Vector3(0, 0, 1), Ogre::Radian(Ogre::Degree(bone->mTheta[5])));
 			w = r * w;
 		}
 
		if(bone->getParent())
 			w = bone->getParent()->getGlobalOri() * w;
 
		w.normalise();
 		cIter = col_iter;
		rIter = cIter->begin() + distance;
		
		w = bone->mInitPose.Q * w;

		Ogre::Vector3 wxp = w.crossProduct(pos);
 		*rIter++ = wxp.x;
		*rIter++ = wxp.y;
		*rIter++ = wxp.z;
   
  		if(!chain->onlyPosition())
		{
			Ogre::Vector3 wxi = w.crossProduct(axisX);
			*rIter++ = wxi.x;
			*rIter++ = wxi.y;
			*rIter++ = wxi.z;
			
			Ogre::Vector3 wxj = w.crossProduct(axisY);
 			*rIter++ = wxj.x;
			*rIter++ = wxj.y;
			*rIter++ = wxj.z;
		}
 	}

	if(bone->mJointType & 2)
	{
	 	Ogre::Vector3 v(0, 1, 0);
		
		if(bone->mJointType & 4)
		{
			Ogre::Matrix3 r;
			r.FromAngleAxis(Ogre::Vector3(0, 0, 1), Ogre::Radian(Ogre::Degree(bone->mTheta[5])));
 			v = r * v;
 		}
 
		if(bone->getParent())
 			v = bone->getParent()->getGlobalOri() * v;
 
		v.normalise();
	 	
		cIter = col_iter;
  		if(bone->mJointType & 1) cIter++;
 		rIter = cIter->begin() + distance;

		v = bone->mInitPose.Q * v;

		Ogre::Vector3 vxp = v.crossProduct(pos);
 		*rIter++ = vxp.x;
		*rIter++ = vxp.y;
		*rIter++ = vxp.z;
   
		if(!chain->onlyPosition())
		{
			Ogre::Vector3 vxi = v.crossProduct(axisX);
			*rIter++ = vxi.x;
			*rIter++ = vxi.y;
			*rIter++ = vxi.z;
			
			Ogre::Vector3 vxj = v.crossProduct(axisY);
 			*rIter++ = vxj.x;
			*rIter++ = vxj.y;
			*rIter++ = vxj.z;
		}
  	 }

	if(bone->mJointType & 4)
	{
 		Ogre::Vector3 u(0, 0, 1);
	
		if(bone->getParent())
	 		u = bone->getParent()->getGlobalOri() * u;
		
		u.normalise();
		
		u = bone->mInitPose.Q * u;
		Ogre::Vector3 uxp = u.crossProduct(pos);

		cIter = col_iter;
		if(bone->mJointType & 1) cIter ++;
		if(bone->mJointType & 2) cIter ++;
 
		rIter = cIter->begin() + distance;
		*rIter++ = uxp.x;
		*rIter++ = uxp.y;
		*rIter++ = uxp.z;
  	 
		if(!chain->onlyPosition())
		{
			Ogre::Vector3 uxi = u.crossProduct(axisX);
			*rIter++ = uxi.x;
			*rIter++ = uxi.y;
			*rIter++ = uxi.z;
			
			Ogre::Vector3 uxj = u.crossProduct(axisY);
 			*rIter++ = uxj.x;
			*rIter++ = uxj.y;
			*rIter++ = uxj.z;
		}
   }
}
