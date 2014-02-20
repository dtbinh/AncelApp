#include "MGPM.h"
#include <cassert>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <fstream>
#include "ResUtility.h"

using namespace ResModel;
using namespace ResUtil;
 
MGPModel::MGPModel(ResUtil::MocapData &mocapData)
	:mYIn(&mocapData.Y),
	 mInitY(mocapData.initY),
	 mSegments(mocapData.segments),
	 mSegmentLabels(mocapData.segmentLabel),
	 mCycles(mocapData.numCycles),
	 mActorLs(mocapData.actorLs),
	 mActionLs(mocapData.actionLs),
	 mModelName(mocapData.modelName),
	 mMotionNameLs(mocapData.mMotionNameLs),
 	 mScaleflag(true),
	 mIsUpdated(false),
	 mTtrained(false)
{
 	initModel();
}
MGPModel::~MGPModel()
{
 	delete  mKernel;
 
	for(size_t i = 0; i < mFactors.size(); i++)
	{
		delete mFakeFactors[i];
 		delete mFactors[i];
	}

	for(size_t i = 0; i < mGx.size(); i++)
		delete mGx[i];

	for(size_t i = 0; i < mGradTs.size(); i++)
		delete mGradTs[i];	
}

void MGPModel::initModel()
{
 	mDimData = mYIn->sizeCol();
	mNumData = mYIn->sizeRow();
	
	mNumIdentity = mActorLs.size();
	mNumContent  = mActionLs.size();

	mDimLatent = mNumContent + mNumIdentity + 2;
	mNumSegments = mSegmentLabels.size();

	//----------------Init Y--------------------------------------------
	// centredY = (Y - mu) / segma
	mMeanY = mYIn->meanCol();
 
	mCentredY = (*mYIn);
	for(size_t i = 0; i < mNumData; i++)
	{
		mCentredY.axpyRowRow(i, mMeanY, 0, -1.0);
	}
 
	mVarY = mCentredY.varCol2();

 	for(size_t i = 0; i < mDimData; i++)
	{
		if(mVarY.get(i) != 0.0f)
			mCentredY.scaleCol(i,1.0/sqrt(mVarY.get(i)));
	}
  	 	
	//--------------Init Factors----------------------------------------
	
	//add the total number of frame  to mSegments just for convenience
	mSegments.push_back(mNumData);
 
	mFacIdx.resize(mNumData);
 
  	size_t numLabels = 2;

	for (size_t i = 1; i < mSegments.size(); i++)
	{
		for (size_t t = mSegments[i-1]; t < mSegments[i]; t++)
		{
			mFacIdx[t].push_back(i-1);
 			mFacIdx[t].push_back(mSegmentLabels[i-1][0]);
			mFacIdx[t].push_back(mSegmentLabels[i-1][1]);
		}
	}
     
	//------Init storeage------------------------------------------------------------------------------------
	
 	mW.resize(mDimData,1);
	mW.oneElements();

	mK.resize(mNumData,mNumData);
 	mInvK.resize(mNumData,mNumData);

	mScaleY.resize(mNumData,mDimData);
 	mInvKY.resize( mNumData,mDimData);
 	mGraCov.resize(mNumData,mNumData);

	mFactors.push_back(new MMatrix(mSegmentLabels.size(), 2));
	mFactors.push_back(new MMatrix(mNumIdentity,mNumIdentity, 'I'));
	mFactors.push_back(new MMatrix(mNumContent, mNumContent,  'I'));

	mFakeFactors.push_back(new MMatrix(mNumData,2));
	mFakeFactors.push_back(new MMatrix(mNumData,mNumIdentity));
	mFakeFactors.push_back(new MMatrix(mNumData,mNumContent));

	mGradTs.push_back(new MMatrix(mNumData,2));
	mGradTs.push_back(new MMatrix(mNumData,2));
	
	calcuateOffset();

	for (size_t i = 0; i < mDimLatent;i++)
		mGx.push_back(new MMatrix(mNumData,mNumData));
	
	mKernel = new MGPKernel(mNumData);
  
	initScaleFactor();
	 
}
void MGPModel::initScaleFactor()
{
	computeFakeFactor();
 
	mKernel->computeKernel(mK,mFakeFactors);
 
	mInvK = mK;
	mLnDK = MMatrix::invertMMatrix(mInvK);

	mInvKY = mInvK * mCentredY;   //(mInvK,mCentredY,1.0,0.0,"N","N");
	mScaleY.copy(mCentredY);
	for (size_t i = 0; i < mDimData; i++)
	{
		double denom = mCentredY.dotColCol(i, mInvKY, i);
		if(fabs(denom) > (1e-8))
			mW.assign(sqrt(mNumData/denom), i);
		else
			mW.assign(1, i);
		mScaleY.scaleCol(i, mW.get(i));
 	}
}
void MGPModel::calcuateOffset()
{
	for (size_t i = 1; i < mSegments.size();i++)
	{
		size_t numRow = mSegments[i]-mSegments[i-1];

		MMatrix curY;
 		curY.submat(mCentredY, mSegments[i-1], numRow, 0, mCentredY.sizeCol());

	//---------------------PCA ---------------------------------------------------------
  		 
		MMatrix eigenValues,eigenVectors;
		curY.pca(eigenVectors, eigenValues);
		 
	//---------------------------------------------------------------------------------
		 
		MMatrix curX(numRow,2);
		MMatrix EigV2(eigenVectors.sizeRow(), 2);
		
		EigV2.copyColCol(0,eigenVectors,eigenVectors.sizeCol() - 1);
		EigV2.copyColCol(1,eigenVectors,eigenVectors.sizeCol() - 2);

		//curX = curY * eigV2 
		curX.gemm(curY, EigV2, 1.0, 0.0, "N", "N");
		curX.scaleCol(0,1.0/sqrt(eigenValues.get(eigenValues.sizeCol() - 1)));
		curX.scaleCol(1,1.0/sqrt(eigenValues.get(eigenValues.sizeCol() - 2)));
	  
		std::vector<double> difAngles;
		double preAngles,curAngles;
		
		for (size_t j = 0; j < numRow; j++)
		{
			curAngles = atan(curX.get(j, 1)/curX.get(j, 0));
			if (j != 0)
				difAngles.push_back(abs(curAngles - preAngles));
			preAngles = curAngles;
		}
		std::sort(difAngles.begin(),difAngles.end());
		//使用各个帧之间的间隔的中间值作为step
		mFactors[0]->assign(0.0, i-1, 0);
		mFactors[0]->assign(difAngles[difAngles.size()/2], i-1, 1);
   	}

	MMatrix templateMat;
  	size_t len = mSegments[1];
 	templateMat.submat(mCentredY, 0, len, 0, mCentredY.sizeCol());
	mFactors[0]->assign(0,0,0);
	
	for(size_t i = 1; i < mSegments.size()-1; i++)
	{
		MMatrix rowI,temp;
		rowI.submat(mCentredY,mSegments[i], 1, 0, mCentredY.sizeCol());
		temp.repmat(rowI, len, 1);
 	 
		temp.axpy(templateMat, -1.0);
		temp.pow(1);
		MMatrix sumRow = temp.sumRow();
		int index = sumRow.minIndex();
		mFactors[0]->assign(index * mFactors[0]->get(0,1), i, 0);
   	}
//// Versison assume all data  are aligned
//	for(std::size_t i = 1; i < mSegments.size(); i++)
//	{
//		double step = (mCycles[i-1] * 2 * PI) / double(mSegments[i] - mSegments[i-1]);
//		mFactors[0]->assign(step, i-1, 1);
//	}
//
//	MMatrix templateMat;
//  	size_t len = mSegments[1];
// 	templateMat.submat(mCentredY, 0, len, 0, mCentredY.sizeCol());
//	mFactors[0]->assign(0,0,0);
//	for(size_t i = 1; i < mSegments.size()-1; i++)
//	{
//		MMatrix rowI,temp;
//		rowI.submat(mCentredY,mSegments[i], 1, 0, mCentredY.sizeCol());
//		temp.repmat(rowI, len, 1);
// 	 
//		temp.axpy(templateMat, -1.0);
//		temp.pow(1);
//		MMatrix sumRow = temp.sumRow();
//		int index = sumRow.minIndex();
//		mFactors[0]->assign(0/*index * mFactors[0]->get(0,1)*/, i, 0);
//   	}
}

void MGPModel::update()  const
{
	if(!mIsUpdated)
	{
		computeFakeFactor();
		mKernel->computeKernel(mK,mFakeFactors);
 		
  		mInvK = mK;
	 	mLnDK = MMatrix::invertMMatrix(mInvK);
	 	mScaleY.copy(mCentredY);
		for(size_t i = 0; i < mDimData; i++)
		{
			mScaleY.scaleCol(i,mW.get(i));
		}
   		mInvKY.gemm(mInvK,mScaleY,1.0,0.0,"N","N");
 
		mIsUpdated = true;
 	}
}
void MGPModel::computeFakeFactor() const
{
	for(size_t i = 0; i < mFactors.size();i++)
	{
		if(i == 0)
			toCircle(*mFakeFactors[i]);
		else 
		{
			for(size_t j = 0; j < mNumData; j++)
			{
				for(size_t k = 0; k < mFactors[i]->sizeCol(); k++)
					mFakeFactors[i]->assign(mFactors[i]->get(mFacIdx[j][i],k),j,k);
 	 		}
		}
	//	std::cout << *mFakeFactors[i] << std::endl;
	}
}
void MGPModel::optimize(std::size_t iter)
{
	time_t tt = clock();
  
	SetminIters(iter);
 
	RunDefaultOptimiser(); 
  
	time_t te = clock();
	mTtrained = true;
	std::cout << "Total  Time : " << te - tt << std::endl;
}
void MGPModel::toCircle(MMatrix & mat) const
{
	for (size_t i = 1; i < mSegments.size(); i++)
	{
		double theta = mFactors[0]->get(i-1, 0);
		double delta = mFactors[0]->get(i-1, 1);

		for (size_t j = mSegments[i-1]; j < mSegments[i]; j++)
		{
			double cosTheta = cos(theta + (j - mSegments[i-1])*delta);
			double sinTheta = sin(theta + (j - mSegments[i-1])*delta);

			mat.assign(cosTheta,j,0);
			mat.assign(sinTheta,j,1);

			//precompute for saving time
			mGradTs[0]->assign(-sinTheta, j, 0);
			mGradTs[0]->assign( cosTheta, j, 1);
			mGradTs[1]->assign(-sinTheta*(j - mSegments[i-1]), j, 0);
			mGradTs[1]->assign( cosTheta*(j - mSegments[i-1]), j, 1);

//			mGradTs[1]->assign(-sinTheta*(j - mSegments[i-1]), j, 0);
//			mGradTs[1]->assign( cosTheta*(j - mSegments[i-1]), j, 1);
		}
	}
}

size_t MGPModel::getOptiParamsNum() const
{
	size_t optiNum = 0;
	
	optiNum += mFactors[0]->getElemNum();
 	optiNum += mFactors[1]->getElemNum();
	optiNum += mFactors[2]->getElemNum();

	optiNum += mDimData;
	optiNum += mKernel->getNumParams();

  	return optiNum;
}

void MGPModel::getOptiParams(MMatrix& param) const
{
	size_t curIndex = 0;

	for (size_t i = 0; i < mFactors[0]->getElemNum(); i++)
		param.assign(mFactors[0]->get(i), curIndex++);

	for (size_t i = 0; i < mFactors[1]->getElemNum(); i++)
		param.assign(mFactors[1]->get(i), curIndex++);

	for (size_t i = 0; i < mFactors[2]->getElemNum(); i++)
		param.assign(mFactors[2]->get(i), curIndex++);
	
	
	/**scale factor*/
	for (size_t i = 0; i < mDimData; i++)
		param.assign(mW.get(i), curIndex++);
	
	/**hyper parameter*/
	for (size_t i = 0; i < mKernel->getNumParams(); i++)
		param.assign(mKernel->getTransParam(i), curIndex++);
}

void MGPModel::setOptiParams(const MMatrix& param)
{
  	size_t curIndex = 0;

	for (size_t i = 0; i < mFactors[0]->getElemNum(); i++)
		mFactors[0]->assign(param.get(curIndex++), i);

	for (size_t i = 0; i < mFactors[1]->getElemNum(); i++)
		mFactors[1]->assign(param.get(curIndex++), i);

	for (size_t i = 0; i < mFactors[2]->getElemNum(); i++)
		mFactors[2]->assign(param.get(curIndex++), i);

	/**scale factor*/
	for (size_t i = 0; i < mDimData; i++)
		mW.assign(param.get(curIndex++), i);

	/**hyper parameter*/	
	for (size_t i = 0; i < mKernel->getNumParams(); i++)
		mKernel->setTransParam(param.get(curIndex++), i);
	
	mIsUpdated = false;
}

void MGPModel::updateCovGradient() const
{
	mGraCov = mInvK;				
	mGraCov.scale(mDimData);		 
	mGraCov.gemm(mInvKY,mInvKY,-1.0,1.0,"N","T"); //DL/DK =  D*invK - InvKY*Y'(invK)'
	mGraCov.setSymmetric(true);
	mGraCov.scale(0.5);
}

double MGPModel::computeObjectiveVal()	    
{
	 return LogLikelihood();			
}
double MGPModel::computeObjectiveGradParams(MMatrix & g)  
{
 	 return LogLikelihoodGradient(g);
}

double MGPModel::LogLikelihood() const
{
	update();
	// N*D*ln(2*PI) + D*ln|K|
 
	double L = 0.5 * (mNumData*mDimData*log(2*PI) + mDimData*mLnDK);
	//Y' = transpose(Y)
	// ScaleY = Y*W
	MMatrix TY = mScaleY;

	TY.transpose();
	//tmp = inv(K)*YW WY'
	MMatrix tmp = mInvKY * TY;
 	// L += tr(temp)
	L += 0.5 * tmp.trace();
 
	if(mScaleflag)
	{
		double DW = 1.0;
		for(size_t i = 0; i < mW.getElemNum(); i++)
 		    DW *= mW.get(i);
  		// L -= D*ln|W|
		//double val = log(10.0);
		L -= mNumData * log(DW);
	}
 	return L;
}

double MGPModel::LogLikelihoodGradient(MMatrix& g) const
{
	double L = LogLikelihood();
	updateCovGradient();
	
	mKernel->getGradientX(mGx,mFakeFactors);
 	
	g.zeroElements();
	
	/**
	* mGx  which is Gradient of K/Grad_Lat;
	*/
	for(size_t t = 0; t < mGx.size(); t++)
	{
		mGx[t]->scaleDiag(0.5);
		mGx[t]->dotMul(mGraCov);
 		 
		MMatrix sumCol = mGx[t]->sumRow();
		sumCol.scale(2.0);
  		
		for(size_t i = 0; i < mNumData; i++)
 		{
	 		if(t < 2)
			{
//				double val1 = sumCol.get(i)*mGradTs[0]->get(i,t);
// 				g.add(val1,mFacIdx[i][0]);
				double val1 = sumCol.get(i)*mGradTs[0]->get(i,t);
				double val2 = sumCol.get(i)*mGradTs[1]->get(i,t);
				g.add(val1, mFacIdx[i][0]);
				g.add(val2, mSegments.size()-1 + mFacIdx[i][0]);
   			}
			else if(t < mNumIdentity + 2)
			{
				int index = mFactors[0]->getElemNum() + mFacIdx[i][1] + (t - 2) * mNumIdentity;
				g.add(sumCol.get(i), index);
			}
			else 
			{
				int index = mFactors[0]->getElemNum() + mFactors[1]->getElemNum() + mFacIdx[i][2] + (t - (mNumIdentity + 2)) * mNumContent;
				g.add(sumCol.get(i),index);
			}
		}
	//}
	}

	//-----------------------------------------------------------------------------------------------
	/**gradient of scale factor*/
	int curIndex  = mFactors[0]->getElemNum();
	curIndex += mFactors[1]->getElemNum();
	curIndex += mFactors[2]->getElemNum();
 
	//for(std::size_t i = 0; i < curIndex; i++)
	//	std::cout << g.get(i) << std::endl;
	for(size_t i = 0; i < mDimData; i++)
	{
		double wi = mInvKY.dotColCol(i,mCentredY,i) - mNumData/mW.get(i);
 		g.assign(wi,curIndex++);
	}
	//-----------------------------------------------------------------------------------------------
	//TODO
	/**gradient of hyperparameter*/

	MMatrix tempG(1,mKernel->getNumParams());
	mKernel->getGradTransParams(tempG,mFakeFactors,mGraCov);
	g.setMMatrix(0,curIndex,tempG);

	//-----------------------------------------------------------------------------------------------
	//std::cout << "grad..........." << std::endl;
 //  	std::cout << g << std::endl;

 	return L;
}
bool MGPModel::writeModelToFile(std::string name)
{
	if(!mTtrained) 
		return false;

	/**
	*	File Format Description:
	*	Model Name:: char[100];
	*	Training Data Dim: N D
	*	MMatrix:  CentredY
	*	MMatrix:  MeanY
	*/
	std::ofstream archiver(name,std::ios::binary|std::ios::out);
	//
	//char modelDescription[100] = "MGP Model";
	//archiver.write(modelDescription,100);
	//RawData YIn
	
	//actor
	std::size_t numActor = mActorLs.size();
	archiver.write((char*)(&numActor),sizeof(size_t));
	for(std::size_t i = 0; i < mActorLs.size(); i++)
	{
		char actor[100];
		std::stringstream converter(mActorLs[i].c_str());
		converter >> actor;
		archiver.write(actor,100);
	}

	//action
	std::size_t numAction = mActionLs.size();
	archiver.write((char*)(&numAction),sizeof(size_t));
	for(std::size_t i = 0; i < mActionLs.size(); i++)
	{
		char action[100];
		std::stringstream converter(mActionLs[i].c_str());
		converter >> action;
		archiver.write(action,100);
	}

	std::size_t numMotion = mMotionNameLs.size();
	archiver.write((char*)(&numMotion),sizeof(size_t));
	for(std::size_t i = 0; i < mMotionNameLs.size(); i++)
	{
		char motion[100];
		std::stringstream converter(mMotionNameLs[i].c_str());
		converter >> motion;
		archiver.write(motion,100);
	}

	char modelName[100];
	std::stringstream converter(mModelName.c_str());
	converter >> modelName;
	archiver.write(modelName,100);
 
	archiver.write((char*)(&mNumData),sizeof(size_t));
	archiver.write((char*)(&mDimData),sizeof(size_t));
	archiver.write((char*)(mYIn->gets()),sizeof(double) * mYIn->getElemNum());
	//CentredY
	archiver.write((char*)(&mNumData),sizeof(size_t));
	archiver.write((char*)(&mDimData),sizeof(size_t));
	archiver.write((char*)(mCentredY.gets()),sizeof(double) * mCentredY.getElemNum());

	//MeanY
	size_t dim = 1;
	archiver.write((char*)(&dim),sizeof(size_t));
	archiver.write((char*)(&mDimData),sizeof(size_t));
	archiver.write((char*)(mMeanY.gets()),sizeof(double)*mMeanY.getElemNum());
	//mVarY
	archiver.write((char*)(&dim),sizeof(size_t));
	archiver.write((char*)(&mDimData),sizeof(size_t));
	archiver.write((char*)(mVarY.gets()),sizeof(double)*mVarY.getElemNum());
	//mW
	archiver.write((char*)(&dim),sizeof(size_t));
	archiver.write((char*)(&mDimData),sizeof(size_t));
	archiver.write((char*)(mW.gets()),sizeof(double)*mW.getElemNum());
	
	std::size_t col = mInitY.sizeCol();
	std::size_t row = mInitY.sizeRow();

	archiver.write((char*)(&row),sizeof(size_t));
	archiver.write((char*)(&col),sizeof(size_t));
	archiver.write((char*)(mInitY.gets()),sizeof(double)*mInitY.getElemNum());
	//mFactor

	for(std::size_t i = 0; i < mFactors.size(); i++)
	{
		std::size_t col = mFactors[i]->sizeCol();
		std::size_t row = mFactors[i]->sizeRow();

		archiver.write((char*)(&row),sizeof(size_t));
		archiver.write((char*)(&col),sizeof(size_t));
		archiver.write((char*)(mFactors[i]->gets()),sizeof(double)*mFactors[i]->getElemNum());

 	}

	//params
	MMatrix tempG(1,mKernel->getNumParams());
	mKernel->getPrams(tempG);
	col = tempG.sizeCol();
	row = tempG.sizeRow();
	archiver.write((char*)(&row),sizeof(size_t));
	archiver.write((char*)(&col),sizeof(size_t));
	archiver.write((char*)(tempG.gets()),sizeof(double)*tempG.getElemNum());
  
	//-------------------------------------------------------------------------------
	std::size_t size = mSegments.size();
	archiver.write((char*)(&size),sizeof(size_t));
	archiver.write((char*)(&mSegments[0]),sizeof(size_t)*size);
 
	row = mSegmentLabels.size();
	col = mSegmentLabels[0].size();
	archiver.write((char*)(&row),sizeof(size_t));
 	archiver.write((char*)(&col),sizeof(size_t));
	for(std::size_t i = 0; i < row;i++)
 	{
		archiver.write((char*)(&mSegmentLabels[i][0]),sizeof(size_t)*col);
 	}
 	
	row = mFacIdx.size();
	col = mFacIdx[0].size();
 	archiver.write((char*)(&row),sizeof(size_t));
 	archiver.write((char*)(&col),sizeof(size_t));
	for(std::size_t i = 0; i < row;i++)
 	{
		archiver.write((char*)(&mFacIdx[i][0]),sizeof(size_t)*col);
 	}
  	return true;
}
//void MGPModel::loadModelFromFile(std::string name)
//{
//	std::ifstream loader(name,std::ios::binary|std::ios::in);
//}
