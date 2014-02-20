#ifndef __Motion_h
#define __Motion_h

#include <OgreVector3.h>
#include <OgreVector4.h>
#include <Eigen\Eigen>

using namespace Eigen;

namespace AncelApp
{
	//TODO: a animation  naming mechanism need be created 
	//Sloution is to using it this pointer as a prefix_ ?
	class Motion
	{
	public:
		Motion(const Motion&);
		Motion(const MatrixXd &data, const std::string& animName);
		void      writeToFile(const std::string & fileName) ;//const;
 		void      loadMotion(const std::string& filename);
		Motion&   createMotion(const MatrixXd& animData, const std::string& animName);
 		void      rotateMotion(int index, const double& angle, const Ogre::Vector3 &axis = Ogre::Vector3(0.0f,1.0f,0.0f));
		void      spliceWithMotion(const Motion* anim);
		void      updateBlock(Eigen::MatrixXd chanels,Eigen::MatrixXi index);
		void      updateRoot(Eigen::MatrixXd chanels, std::pair<int,int> range);
		void      updateFrame(int frameIndex,const std::vector<double>& theta);
		//void      update
		Motion	  upSampleMotion(std::size_t scale)   const;
		Motion	  downSampleMotion(float scale) const;

		void      shiftMotion(Ogre::Vector3 shiftValue);
		
		void      rotateFrame(int frameIndex, float rotateAngle, Ogre::Vector3 &posShift, Ogre::Vector3 &oriention);
		Ogre::Vector3  getRootPosition(std::size_t frameIndex);
		
		void setRootPosition(std::size_t frameIndex, const Ogre::Vector3& pos);

		Ogre::Vector3  getRootOriention(std::size_t frameIndex);
		std::size_t         getTotalFrameNum() const;
		
		std::vector<double> getFrame(int frameIndex);
		Eigen::MatrixXd getMotionPath();
		Eigen::MatrixXd getMotionData() {return mAnimData;}
		std::size_t  getFreedomNum() const {return mAnimData.cols();}
		const std::string& getName() const {return mAnimName;}
 	private:
 		
		bool operator = (const Motion&);

		std::string mAnimName;
		MatrixXd mAnimData;
	};
};

#endif