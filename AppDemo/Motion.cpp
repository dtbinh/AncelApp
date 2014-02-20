#include "Motion.h"
#include "AppUtility.h"
#include <OgreMatrix3.h>
#include <OgreQuaternion.h>
#include <MyGUI.h>
using namespace AncelApp;

Motion::Motion(const MatrixXd &data, const std::string& animName)
{
	mAnimData = data;
	mAnimName =  MyGUI::utility::toString(this, "_") + animName;
}

Motion::Motion(const Motion& mo)
{
	//TODO: update animation name;
	this->mAnimData = mo.mAnimData;
	this->mAnimName = mo.mAnimName;
}
void  Motion::loadMotion(const std::string& filename)
{
	mAnimData = loadData(filename);

	mAnimName = filename;
}
void Motion::writeToFile(const std::string & fileName) //const
{
 	//MatrixXd data = mAnimData.block(407,0,1110-407,mAnimData.cols());

	std::size_t row = mAnimData.rows();
	std::size_t col = mAnimData.cols();
	std::ofstream writer(fileName,std::ios::binary|std::ios::out);
		
	/*for(std::size_t i = 0; i < row; i++)
	{
		Ogre::Vector3 s;
		Ogre::Vector3 v;
		if(i < 200)
			rotateMotion(i, -0.1 - i * 0.001);
		else
			rotateMotion(i, 0.1 + i * 0.001);
	}
*/
	writer.write((char*)(&row), sizeof(std::size_t));
	writer.write((char*)(&col), sizeof(std::size_t));
	writer.write((char*)(mAnimData.data()), sizeof(double)*row*col);
	writer.close();

	
	//Motion mo = downSampleMotion(0.25);
 // 
	//std::size_t row = mo.mAnimData.rows();
	//std::size_t col = mo.mAnimData.cols();
	//std::ofstream writer(fileName,std::ios::binary|std::ios::out);
	//		
	//writer.write((char*)(&row), sizeof(std::size_t));
	//writer.write((char*)(&col), sizeof(std::size_t));
	//writer.write((char*)(mo.mAnimData.data()), sizeof(double)*row*col);
}
Motion&  Motion::createMotion(const MatrixXd& animData, const std::string& animName)
{
	mAnimData = animData;
	mAnimName = animName;
	return *this; 
}
 		
std::size_t Motion::getTotalFrameNum() const
{
	return mAnimData.rows();
}

std::vector<double> Motion::getFrame(int frameIndex)
{
	if(frameIndex >= mAnimData.rows())
		throw std::invalid_argument("Invalid Index number!!");
	
	std::vector<double> frame;
	for(int i = 0; i < mAnimData.cols(); i++)
		frame.push_back(mAnimData(frameIndex, i));
	return frame;
}

void  Motion::rotateMotion(int index, const double& angle, const Ogre::Vector3 &axis)
{
	 Ogre::Matrix3 rotMatrix; 
	 Ogre::Quaternion Q;
  
	 Q.FromAngleAxis(Ogre::Radian(Ogre::Degree(static_cast<float>(angle)).valueRadians()),axis);
 	 Q.ToRotationMatrix(rotMatrix);

 	 Ogre::Vector3 ori;

	 ori.x = static_cast<float>(mAnimData(index,0));
	 ori.y = static_cast<float>(mAnimData(index,1));
	 ori.z = static_cast<float>(mAnimData(index,2));

	 for(int i = index; i < mAnimData.rows(); i++)
	 {
 		Ogre::Vector3 pos;
		pos.x = static_cast<float>(mAnimData(i,0));
		pos.y = static_cast<float>(mAnimData(i,1));
		pos.z = static_cast<float>(mAnimData(i,2));

		pos = pos - ori;
		pos = rotMatrix * pos;
		pos = pos + ori;

		mAnimData(i, 0) = pos.x;
		mAnimData(i, 1) = pos.y;
		mAnimData(i, 2) = pos.z;
 
		Ogre::Vector3 ro;
		ro.x = static_cast<float>(mAnimData(i,3));
		ro.y = static_cast<float>(mAnimData(i,4));
		ro.z = static_cast<float>(mAnimData(i,5));

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

		mAnimData(i,3) = ax.valueDegrees();
		mAnimData(i,4) = ay.valueDegrees();
		mAnimData(i,5) = az.valueDegrees();
	}
}
void  Motion::spliceWithMotion(const Motion* anim)
{

}
Eigen::MatrixXd Motion::getMotionPath()
{
	Eigen::MatrixXd motionpath(mAnimData.rows(), 4);

	motionpath.block(0, 0, mAnimData.rows(), 3) = mAnimData.block(0, 0, mAnimData.rows(), 3);
 
	assert(mAnimData.rows());
	motionpath(0, 3) = 1;
	int cnt = 0;
	for (int i = 1; i < mAnimData.rows() - 1; i++)
	{
 		double ldif = motionpath(i, 1) - motionpath(i-1, 1);
		double rdif = motionpath(i+1, 1) - motionpath(i, 1);
	
		if(ldif <= 0 && rdif >= 0 && cnt > 50)
		{
			motionpath(i, 3) = 1;
			cnt = 0;
		}
		else 
		{
			motionpath(i, 3) = 0;
			cnt ++;
		}
	}
	motionpath(motionpath.rows() - 1, 3) = 1;
 	return motionpath;
}
void Motion::updateBlock(Eigen::MatrixXd channels,Eigen::MatrixXi index)
{
	if(channels.cols() == index.cols() && channels.rows() <= mAnimData.rows())
	{
		for(int i = 0; i < index.cols();i++)
			mAnimData.block(0,index(0,i),channels.rows(),1) = channels.block(0,i,channels.rows(),1);
	}
}
void Motion::shiftMotion(Ogre::Vector3 shiftValue)
{
	for(int i = 0; i < mAnimData.rows(); i++)
	{
		mAnimData(i,0) += shiftValue.x;
		mAnimData(i,1) += shiftValue.y;
		mAnimData(i,2) += shiftValue.z;
	}
}
void  Motion::rotateFrame(int frameIndex, float rotateAngle, Ogre::Vector3 &posShift, Ogre::Vector3 &oriention)
{
	Ogre::Matrix3 rotMatrix; 
	Ogre::Quaternion Q;
  
	Q.FromAngleAxis(Ogre::Radian(Ogre::Degree(static_cast<float>(rotateAngle)).valueRadians()),Ogre::Vector3(0,1,0));
 	Q.ToRotationMatrix(rotMatrix);

 	Ogre::Vector3 ori;

	ori.x = static_cast<float>(mAnimData(0, 0));
	ori.y = static_cast<float>(mAnimData(0, 1));
	ori.z = static_cast<float>(mAnimData(0, 2));

	 
 	Ogre::Vector3 pos;
	pos.x = static_cast<float>(mAnimData(frameIndex, 0));
	pos.y = static_cast<float>(mAnimData(frameIndex, 1));
	pos.z = static_cast<float>(mAnimData(frameIndex, 2));

	pos = pos - ori;
	pos = rotMatrix * pos;
	pos = pos + ori;

	posShift = pos;
 
	Ogre::Vector3 ro;
	ro.x = static_cast<float>(mAnimData(frameIndex, 3));
	ro.y = static_cast<float>(mAnimData(frameIndex, 4));
	ro.z = static_cast<float>(mAnimData(frameIndex, 5));

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

	oriention = Ogre::Vector3(ax.valueDegrees(), ay.valueDegrees(), az.valueDegrees());
}
Ogre::Vector3  Motion::getRootPosition(std::size_t frameIndex)
{
	return Ogre::Vector3(float(mAnimData(frameIndex,0)), float(mAnimData(frameIndex,1)), float(mAnimData(frameIndex,2)));
}
Ogre::Vector3  Motion::getRootOriention(std::size_t frameIndex)
{
	return Ogre::Vector3(float(mAnimData(frameIndex,3)), float(mAnimData(frameIndex,4)), float(mAnimData(frameIndex,5)));
}
Motion	  Motion::upSampleMotion(std::size_t scale)   const
{
	return *this;
}
Motion  Motion::downSampleMotion(float scale) const
{
	MatrixXd data(int(mAnimData.rows() * scale), mAnimData.cols());
	for(int i = 0; i < data.rows(); i++)
		data.row(i) = mAnimData.row(int(i/scale));
	Motion mo(data, mAnimName + "DownSample");
	return mo;
}
void Motion::updateFrame(int frameIndex, const std::vector<double>& theta)
{
	if(frameIndex >= mAnimData.rows())
		throw std::invalid_argument("Invalid Index number!!");

	for(int i = 0; i < mAnimData.cols(); i++)
		mAnimData(frameIndex, i) = theta[i];
}
void  Motion::updateRoot(Eigen::MatrixXd chanels, std::pair<int,int> range)
{
	mAnimData.block(range.first,0,range.second-range.first, 6) = chanels;
}

void Motion::setRootPosition(std::size_t frameIndex, const Ogre::Vector3& pos)
{
	mAnimData(frameIndex,0) = pos.x;
	mAnimData(frameIndex,1) = pos.y;
	mAnimData(frameIndex,2) = pos.z;
}

