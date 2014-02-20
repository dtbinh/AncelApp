#include "Path2DManipulator.h"
#include <cblaswrap.h>
#include <OgreStringConverter.h>
using namespace AncelApp;

Eigen::MatrixXd multiplyM1(const Eigen::MatrixXd &m1, const Eigen::MatrixXd &m2)
{
 	Eigen::MatrixXd ret = Eigen::MatrixXd::Zero(m1.rows(),m2.cols());
	for(int i = 0; i < m1.rows(); i++)
	{
		if(i >= 4)
		{
			for(int j = 0; j <= i; j++)
			{
 				ret(i,j) += m1(i, (i/2)*2 - 4) * m2((i/2)*2 - 4, j);
				ret(i,j) += m1(i, (i/2)*2 - 3) * m2((i/2)*2 - 3, j);
 			}
		}
	 	if(i >= 2 && ((i/2)*2 - 1) < m1.cols())
		{
			for(int j = 0; j <=  i; j++)
			{
				ret(i,j) += m1(i, (i/2)*2 - 2) * m2((i/2)*2 - 2, j);
				ret(i,j) += m1(i, (i/2)*2 - 1) * m2((i/2)*2 - 1, j);
 			}
		}
		if((i/2)*2 < m1.cols())
		{
			for(int j = 0; j <=  i; j++)
			{
				ret(i,j) += m1(i, (i/2)*2) * m2((i/2)*2, j);
				ret(i,j) += m1(i, (i/2)*2 + 1) * m2((i/2)*2 +1 , j);
  			}
  		}
		for(int j = 0; j < i; j++)
		{
			ret(j,i) = ret(i,j);
		}
	}
 	return ret;
}

Eigen::MatrixXd multiplyM2(const Eigen::MatrixXd &m1, const Eigen::MatrixXd &m2)
{
 	Eigen::MatrixXd ret = Eigen::MatrixXd::Zero(m1.rows(),m2.cols());
	for(int i = 0; i < m1.rows(); i++)
	{
 	 	if(i >= 2 && (i - 2) < m1.cols())
		{
			for(int j = 0; j <= i; j++)
			{
				ret(i,j) += m1(i, i - 2) * m2(i - 2, j);
 			}
		}
		if(i < m1.cols())
		{
			for(int j = 0; j <= i; j++)
			{
				ret(i,j) += m1(i, i) * m2(i, j);
 			}
  		}
		for(int j = 0; j < i; j++)
		{
			ret(j,i) = ret(i,j);
		}
	}
 	return ret;
}
Eigen::MatrixXd LUSlover(Eigen::MatrixXd A, Eigen::MatrixXd B)
{
 	int	lwork = 3*B.rows();
	int info;
	std::vector<int> ipivv(B.rows());
	int *ipiv = &ipivv[0];
	
	double *work = new double[3 * B.rows()];

	dsysv_("u", B.rows(), B.cols(), A.data(), A.rows(), ipiv, B.data(), B.rows(), work, lwork, info);
	assert(info == 0);
	return B;
}
Path2DManipulator::Path2DManipulator()
{

}
Path2DManipulator::~Path2DManipulator()
{

}

void Path2DManipulator::computeRelativeCoordinate()
{
	mRelativeCoordinate.clear();
//	std::ofstream fout(Ogre::StringConverter::toString(reinterpret_cast<int>(this)) + "relout.txt");
	//put the frist point: assume the first point's relative coordination is itself
	mRelativeCoordinate.push_back(Ogre::Vector2((float)(mInit2DPath(0,0)), (float)(mInit2DPath(0,1)))); 

	//start from the second
	for (int i = 1; i < mInit2DPath.rows() - 1; i++)
	{
 		double v1x = mInit2DPath(i,0) - mInit2DPath(i-1,0);
		double v1y = mInit2DPath(i,1) - mInit2DPath(i-1,1);
		
		double v2x = mInit2DPath(i+1,0) - mInit2DPath(i-1,0);
		double v2y = mInit2DPath(i+1,1) - mInit2DPath(i-1,1);
		
		double len  = sqrt(v1x * v1x + v1y * v1y);
		double len2 = sqrt(v2x * v2x + v2y * v2y);
 	 
		//assert(len > 1e-08 && len2 > 1e-08);
		//std::cout << i << " " << len << " " << len2 << std::endl;
		//Ogre::Vector2 v1(v1x,v1y*10000);
		//Ogre::Vector2 v2(v2x,v2y*10000);
		//v1.normalise();
		//v2.normalise();

		//v1x = v1.x;
		//v1y = v1.y;

		//v2x = v2.x;
		//v2y = v2.y;

		v1x = v1x/len;
		v1y = v1y/len;

		v2x = v2x/len2;
		v2y = v2y/len2;
  	 	 
	
		double u = v1x*v2x + v1y*v2y;
		std::swap(v2y, v2x);
		v2y = -v2y;
		double v = v1x*v2x + v1y*v2y;
  
	 	mRelativeCoordinate.push_back(Ogre::Vector2((float)(len * u / len2), (float)(len * v/len2)));
	 }
}
void  Path2DManipulator::setInit2DPath(const Eigen::MatrixXd &init2DPath)
{
 	mInit2DPath = init2DPath;
	mManipulated2DPath = mInit2DPath;

	computeRelativeCoordinate();
	computeMatrixMs();
	computeMatrixHs();

	computeMatrixMA();
 	mMB1.resize(mMA1.rows(), 1);
 
	mMB2.resize(mMA2.rows(), 1);
}

void Path2DManipulator::computeMatrixHs()
{
	mNumHandlePoint = 0;
	for (int i = 0; i < mInit2DPath.rows(); i++)
	{
		if (mInit2DPath(i,2)  > 0)
		{
			mNumHandlePoint ++;
		}
	}
	
	mHc = Eigen::MatrixXd::Zero(mNumHandlePoint * 2, mInit2DPath.rows() * 2);
	
	std::size_t cnt = 0;
	for (int i = 0; i < mInit2DPath.rows(); i++)
	{
		if (mInit2DPath(i,2)  > 0)
		{
			mHc(cnt * 2, i * 2) = 1.0;
			mHc(cnt * 2 + 1, i * 2 + 1) = 1.0;
			cnt++;
		}
	}
}
/*
*	p_i = p_(i-1) + x_i(p_(i+1) - p(i-1)) + y_i*R*(p_(i+1) - p(i-1)) ;    where R = |0  1|
*   p_(i-1) + x_i(p_(i+1) - p(i-1)) + y_i*R*(p_(i+1) - p(i-1)) - p_i = 0;           |-1 0|
*   Ap = 0   
*
*
*/
void Path2DManipulator::computeMatrixMs()
{
	mM1 = Eigen::MatrixXd::Zero((mInit2DPath.rows()-2) * 2, (mInit2DPath.rows() * 2));
	mM2 = Eigen::MatrixXd::Zero((mInit2DPath.rows()-1) * 2, (mInit2DPath.rows() * 2));
 
	for (int i = 1; i < mInit2DPath.rows() - 1; i++)
	{
		//Ogre::Vector2 v(mInit2DPath(i+1,0) - mInit2DPath(i-1,0), mInit2DPath(i+1,1) - mInit2DPath(i-1,1));
		double w = 1.0;// / v.length();
		double x = mRelativeCoordinate[i].x;
		double y = mRelativeCoordinate[i].y;
 	 
		mM1((i - 1) * 2, (i * 2) - 2) = w * (x - 1);
		mM1((i - 1) * 2, (i * 2) - 1) = w * (    y);
		mM1((i - 1) * 2, (i * 2))	  = w * (    1);
		mM1((i - 1) * 2, (i * 2) + 1) = w * (    0);
		mM1((i - 1) * 2, (i * 2) + 2) = w * (   -x);
		mM1((i - 1) * 2, (i * 2) + 3) = w * (   -y);

		mM1((i - 1) * 2 + 1, (i * 2) - 2) = w * (   -y);
		mM1((i - 1) * 2 + 1, (i * 2) - 1) = w * (x - 1);
		mM1((i - 1) * 2 + 1, (i * 2))	  = w * (    0);
		mM1((i - 1) * 2 + 1, (i * 2) + 1) = w * (    1);
		mM1((i - 1) * 2 + 1, (i * 2) + 2) = w * (    y);
		mM1((i - 1) * 2 + 1, (i * 2) + 3) = w * (   -x);
   	}

	for (int i = 1; i < mInit2DPath.rows(); i++)
	{
		Ogre::Vector2 v(float(mInit2DPath(i,0) - mInit2DPath(i-1,0)), float(mInit2DPath(i,1) - mInit2DPath(i-1,1)));
		double w = 1.0 / v.length();

 		mM2(2 * (i - 1), 2 * i - 2) = -w;
		mM2(2 * (i - 1), 2 * i) =  w;
		mM2(2 * (i - 1) + 1, 2 * i - 1) = -w;
		mM2(2 * (i - 1) + 1, 2 * i + 1) = w;
 	}

	mTransM1M1 =  multiplyM1(mM1.transpose(), mM1);
	mTransM2M2 =  multiplyM2(mM2.transpose(), mM2);
 	/*std::ofstream fout("check.txt");
	fout << mM1.block(0,0,50,50) << std::endl;*/
}

Ogre::Vector2 Path2DManipulator::getPoint(const int& index) const
{
	assert(index < mManipulated2DPath.rows());
	return Ogre::Vector2((float)mManipulated2DPath(index,0), (float)mManipulated2DPath(index,1));
}

Ogre::Vector2  Path2DManipulator::getInitPoint(const int &index) const
{
	assert(index < mInit2DPath.rows());
	return Ogre::Vector2((float)mInit2DPath(index,0), (float)mInit2DPath(index,1));
}
/*     |M1^*M1  H^|
* MA =  
*      |H        0|
*/
void Path2DManipulator::computeMatrixMA()
{
 	mMA1 = Eigen::MatrixXd::Zero(mM1.cols() + mHc.rows(), mM1.cols() + mHc.rows());
 
	mMA1.block(0, 0, mM1.cols(), mM1.cols()).noalias() =  mTransM1M1;
	mMA1.block(0, mM1.cols(), mHc.cols(), mHc.rows()) = mHc.transpose();
 	mMA1.block(mM1.cols(), 0, mHc.rows(), mHc.cols()) = mHc;	

	mMA2 = Eigen::MatrixXd::Zero(mM2.cols() + mHc.rows(), mM2.cols() + mHc.rows());
	
	mMA2.block(0, 0, mM2.cols(),mM2.cols()) = mTransM2M2;
	mMA2.block(mM2.cols(), 0, mHc.rows(), mHc.cols()) = mHc;
	mMA2.block(0, mM2.cols(), mHc.cols(), mHc.rows()) = mHc.transpose();
}
Eigen::MatrixXd& Path2DManipulator::update(const Ogre::Vector2 & pos, const int index)
{
	if(index < mInit2DPath.rows())
	{
		mManipulated2DPath(index, 0) = pos.x;
		mManipulated2DPath(index, 1) = pos.y;
	}

	update();
 	return mManipulated2DPath;
}

void Path2DManipulator::updateControlPoint(const int index)
{
	if(index <= 0 || index >= mInit2DPath.rows() - 1) return;

//	std::cout << index << " " <<  mInit2DPath(index,2) << std::endl;
	
	mInit2DPath(index,2) = mInit2DPath(index,2) > 0 ? 0 : 1;

//	std::cout << index << " "  << mInit2DPath(index,2) << std::endl;
	mManipulated2DPath(index,2) = mInit2DPath(index,2);

	computeMatrixHs();
	computeMatrixMA();

	mMB1.resize(mMA1.rows(), 1);
	mMB2.resize(mMA2.rows(), 1);
}

Eigen::MatrixXd& Path2DManipulator::update(const std::vector<Ogre::Vector2> &vPos, const std::vector<int>& vIndex)
{
	for(std::size_t i = 0; i < vIndex.size(); i++)
	{
		if(vIndex[i] >= mManipulated2DPath.rows())
			throw std::invalid_argument("index large than the point size of path !!!");
		mManipulated2DPath(vIndex[i],0) = vPos[i].x;
		mManipulated2DPath(vIndex[i],1) = vPos[i].y;
	}
 //	update();
 	return mManipulated2DPath;
}

void Path2DManipulator::update()
{
	//stage one scale free slove Mp^- = h1
 	
	//init MB's value
 	mMB1.setConstant(0);
  
	std::size_t handleCnt = 0;
	for(int i = 0; i < mManipulated2DPath.rows(); i++)
	{
		if(mInit2DPath(i,2)  > 0)
		{
			mMB1(mManipulated2DPath.rows() * 2 + handleCnt * 2, 0) = mManipulated2DPath(i,0);
			mMB1(mManipulated2DPath.rows() * 2 + handleCnt * 2 + 1, 0) = mManipulated2DPath(i,1);
			handleCnt++;
		}
	}
 
	Eigen::MatrixXd Pv = LUSlover(mMA1 ,mMB1);
 
	/*for(std::size_t i = 0; i < mManipulated2DPath.rows(); i++)
	{
		mManipulated2DPath(i,0)  = Pv(i*2, 0);
		mManipulated2DPath(i,1)  = Pv(i*2 + 1, 0);
	}
	return; */
	
	
	//stage two scaled-adjust
 	mMB2.setConstant(0);
 
	handleCnt = 0;
 	for(int i = 0; i < mManipulated2DPath.rows(); i++)
	{
		if(mInit2DPath(i,2)  > 0)
		{
			mMB2(mManipulated2DPath.rows() * 2 + handleCnt * 2, 0) = mManipulated2DPath(i,0);
			mMB2(mManipulated2DPath.rows() * 2 + handleCnt * 2 + 1, 0) = mManipulated2DPath(i,1);
			handleCnt++;
		}
	}
 
	//Automatic scale factor 
	/*std::vector<Ogre::Real> scaleFactor;
	std::size_t last_handle_point = 0;
	std::size_t arc_length = 0.0;
	for(std::size_t i = 1; i < mManipulated2DPath.rows(); i++)
	{
		 
		Ogre::Vector2 v;
	  	v.x = (Ogre::Real)Pv(i * 2, 0) - (Ogre::Real)Pv((i-1) * 2, 0);
		v.y = (Ogre::Real)Pv(i * 2 + 1, 0) - (Ogre::Real)Pv((i-1) * 2 + 1, 0);
		arc_length += v.length();
		if(mManipulated2DPath(i,2) > 0)
		{
 			scaleFactor.push_back(arc_length/mArcLength[scaleFactor.size()]);
			arc_length = 0.0;
			last_handle_point = i;
		}
 	}*/
 
	Eigen::MatrixXd matA(2 * (mManipulated2DPath.rows() - 1), 1);  
	std::size_t cnt = 0;
 	for (int i = 1; i < mManipulated2DPath.rows() ; i++)
	{
 		Vector2d v2;
	
//		Eigen::Array2d v2;

	    //Ogre::Vector2 v2;
		//v2.x  = mInit2DPath(i, 0) - mInit2DPath(i-1, 0);
		//v2.y  = mInit2DPath(i, 1) - mInit2DPath(i-1, 1);
 	//	double w = sqrt(v2(0,0)*v2(0,0) + v2(0,1)*v2(0,1));

		v2(0,0) = (Ogre::Real)Pv(i * 2, 0) - (Ogre::Real)Pv((i-1) * 2, 0);
		v2(1,0) = (Ogre::Real)Pv(i * 2 + 1, 0) - (Ogre::Real)Pv((i-1) * 2 + 1, 0);

		double v2Len = sqrt(v2(0,0)*v2(0,0) + v2(1,0)*v2(1,0));
		matA(2 * (i - 1), 0) = v2(0,0)/ v2Len;// w;
		matA(2 * (i - 1) + 1, 0) = v2(1,0) /v2Len;// * w;
		if(mManipulated2DPath(i,2) > 0)
			cnt++;
   	}
 
	mMB2.block(0, 0, mM2.cols(),matA.cols()).noalias() = mM2.transpose()*matA;
 
	//Eigen::MatrixXd PDesired = mMA2.lu().solve(mMB2);
	Eigen::MatrixXd PDesired = LUSlover(mMA2, mMB2);

	for(int i = 0; i < mManipulated2DPath.rows(); i++)
	{
		mManipulated2DPath(i,0)  = PDesired(i*2, 0);
		mManipulated2DPath(i,1)  = PDesired(i*2 + 1, 0);
	}
}