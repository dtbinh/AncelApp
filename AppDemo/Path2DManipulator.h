#ifndef __Path2DManipulator_h_
#define __Path2DManipulator_h_


#include <OgreMatrix4.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include <vector>
#include <Eigen\Eigen>

namespace AncelApp
{
	
	class Path2DManipulator
	{
	public:	
		
		Path2DManipulator();
		Path2DManipulator(const Eigen::MatrixXd &init2DPath);
		~Path2DManipulator();
		
		void  setInit2DPath(const Eigen::MatrixXd &init2DPath);
		const Eigen::MatrixXd& getInit2DPath() {return mInit2DPath;}

		Ogre::Vector2 getPoint(const int &index) const;
		Ogre::Vector2 getInitPoint(const int &index) const;

		Eigen::MatrixXd &update(const Ogre::Vector2 & pos, const int index);
		Eigen::MatrixXd &update(const std::vector<Ogre::Vector2> &vPos, const std::vector<int>& vIndex);
		void updateControlPoint(const int index);
 	protected:
		
		void update();
 		void computeMatrixHs();
		void computeMatrixMs();
		void computeMatrixMA();
		
		void updateMatrixMs();
		void computeRelativeCoordinate();
 	private:
		typedef Eigen::Matrix<double, 2, 1> Vector2d;
		typedef Eigen::Matrix<double, 3, 1> Vector3d;

		Eigen::MatrixXd mM1;
		Eigen::MatrixXd mTransM1M1;
		Eigen::MatrixXd mM2;
		Eigen::MatrixXd mTransM2M2;
		Eigen::MatrixXd mHc;
		
		Eigen::MatrixXd mMA1;		//stage 1
		Eigen::MatrixXd mMA2;		//stage 2
		Eigen::MatrixXd mMB1;		//stage 1
		Eigen::MatrixXd mMB2;       //stage 2

		std::size_t				   mNumHandlePoint;
 		Eigen::MatrixXd			   mInit2DPath;
		Eigen::MatrixXd	           mManipulated2DPath;
		std::vector<Ogre::Vector2> mRelativeCoordinate;
	};
}

#endif