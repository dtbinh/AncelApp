#include "HMGPM.h"
#include "PeriodKernel.h"
#include <sstream>
using namespace ResModel;

HMGPModel::HMGPModel(ResUtil::MocapData &mocapData)
	:mFeaVec(mocapData.Y),
	mSegmentLabels(mocapData.segmentLabel),
	mSegments(mocapData.segments),
	mFactor2Ls(mocapData.actionLs),
	mFactor1Ls(mocapData.actorLs),
	mInitFeaVec(mocapData.initY),
	mIsUpdated(false),
	mTtrained(false),
	mDimState(3)
{
 	initStorge();
	initKernel(mNumData);
	initOptiParams();
	initScaleFactor(true);
}

HMGPModel::HMGPModel(std::string modelName)
{
  	loadModelFromFile(modelName);
	initStorge();
  	initScaleFactor(false);
	constructStepPrior();
}

HMGPModel::~HMGPModel()
{
	delete  mKernel;
	delete  mStateKernel;
	for(size_t i = 0; i < mFactors.size(); i++)
	{
		delete mFakeFactors[i];
 		delete mFactors[i];
	}

	for(size_t i = 0; i < mGx.size(); i++)
		delete mGx[i];
	for(size_t i = 0; i < mGt.size(); i++)
		delete mGt[i];
	
}

void HMGPModel::initOptiParams()
{
	mFactors.push_back(new MMatrix(mSegments.size() - 1 , 2));
	mFactors.push_back(new MMatrix(mNumFactor1, mNumFactor1, 'I'));
	mFactors.push_back(new MMatrix(mNumFactor2, mNumFactor2, 'I'));

	mFakeFactors.push_back(new MMatrix(mNumData, 3));
	mFakeFactors.push_back(new MMatrix(mNumData, mNumFactor1));
	mFakeFactors.push_back(new MMatrix(mNumData, mNumFactor2));

	mW.resize(mDimData, 1);
	mW.oneElements();
	
	mWs.resize(mScaleX.sizeCol(), 1);
	mWs.oneElements();

	calcuateOffset();
}

void HMGPModel::initKernel(std::size_t dataNum)
{
	mKernel = new MGPKernel(dataNum);
	mStateKernel = new ResCore::CompoundKernel();
	mStateKernel->addKernel(new ResCore::PeriodKernel());
	mStateKernel->addKernel(new ResCore::WhiteNoiseKernel());

	//mStateKernel->setParam(1e-6, mStateKernel->getNumParams() - 1);
}

void HMGPModel::initStorge()
{
	mDimData = mFeaVec.sizeCol();
	mNumData = mFeaVec.sizeRow();
	
	mNumFactor1 = mFactor1Ls.size();
	mNumFactor2  = mFactor2Ls.size();

	mDimLatent = mNumFactor1 + mNumFactor2 + 3;
	mNumSegments = mSegmentLabels.size();

	//----------------Init Y--------------------------------------------
	mMeanFeaVec = mFeaVec.meanCol();
 
	mCentredFeaVec = mFeaVec;
	for(size_t i = 0; i < mNumData; i++)
	{
		mCentredFeaVec.axpyRowRow(i,mMeanFeaVec,0,-1.0);
	}
 
	mVarFeaVec = mCentredFeaVec.varCol2();
	 
 	for(size_t i = 0; i < mDimData; i++)
	{
		if(mVarFeaVec.get(i) != 0.0f)
			mCentredFeaVec.scaleCol(i, 1.0/sqrt(mVarFeaVec.get(i)));
	}
  	 	
	//--------------Init Factors----------------------------------------
	
	//add the total number of frame  to mSegments for convenience
	//TODO
	mSegments.push_back(mNumData);
 
	mPoseProperties.resize(mNumData);
 
  	//size_t numLabels = 2;

	for (size_t i = 1; i < mSegments.size(); i++)
	{
		for (size_t t = mSegments[i-1]; t < mSegments[i]; t++)
		{
	 		mPoseProperties[t].push_back(mSegmentLabels[i-1][0]);
			mPoseProperties[t].push_back(mSegmentLabels[i-1][1]);
		}
	}
  
	//------Init storeage------------------------------------------------------------------------------------
 
	mK.resize(     mNumData, mNumData);
 	mKs.resize(    mNumData, mNumData);

	mInvK.resize(  mNumData, mNumData);
	mInvKs.resize( mNumData, mNumData);

	mScaleX.resize(mNumData, 3);

	mInvKFeaVec.resize( mNumData, mDimData);
	mScaleFeaVec.resize(mNumData, mDimData);
 	
 	mGradientK.resize(mNumData, mNumData);
	
	mStatesFactor.resize(mNumData, 1);
      
	for (size_t i = 0; i < mDimLatent;i++)
		mGx.push_back(new MMatrix(mNumData,mNumData));
	
	mGt.push_back(new MMatrix(mNumData, mNumData));
 
}

void HMGPModel::optimize(std::size_t iter)
{
	SetminIters(iter);
	RunDefaultOptimiser();
	mTtrained = true;
	//for(std::size_t i = 0; i < mFactors.size(); i++)
	//{
	//	std::cout << "Factor 1 " << std::endl;
	//	std::cout << *mFactors[i] << std::endl;
	//}

	//std::cout << "Factor X " << std::endl;
	//std::cout << *mFakeFactors[0] << std::endl;

	//std::cout << "W: " << std::endl;
	//std::cout << mW << std::endl;

	//std::cout << "W: " << std::endl;
	//std::cout << mWs << std::endl;
 // 
	//ResUtil::MMatrix parms(1, mKernel->getNumParams());
	//mKernel->getPrams(parms);

	//std::cout << "Params: " << std::endl;
	//std::cout << parms << std::endl;
	//
	//parms.resize(1, mStateKernel->getNumParams());
	//mStateKernel->getParams(parms);
	//std::cout << "Params2:" << std::endl;
	//std::cout << parms << std::endl;
}

void HMGPModel::calcuateOffset()
{
	for (size_t i = 1; i < mSegments.size();i++)
	{
		size_t numRow = mSegments[i] - mSegments[i-1];

		MMatrix curY;
 		curY.submat(mCentredFeaVec, mSegments[i-1], numRow, 0, mCentredFeaVec.sizeCol());

	//---------------------PCA ---------------------------------------------------------
  		 
		MMatrix eigenValues,eigenVectors;
		curY.pca(eigenVectors, eigenValues);
		 
	//---------------------------------------------------------------------------------
		 
		MMatrix curX(numRow,3);
		MMatrix EigV2(eigenVectors.sizeRow(), 3);
		
		EigV2.copyColCol(0,eigenVectors,eigenVectors.sizeCol() - 1);
		EigV2.copyColCol(1,eigenVectors,eigenVectors.sizeCol() - 2);
		EigV2.copyColCol(2,eigenVectors,eigenVectors.sizeCol() - 3);

		//curX = curY * eigV2 
		curX.gemm(curY, EigV2, 1.0, 0.0, "N", "N");
		curX.scaleCol(0, 1.0/sqrt(eigenValues.get(eigenValues.sizeCol() - 1)));
		curX.scaleCol(1, 1.0/sqrt(eigenValues.get(eigenValues.sizeCol() - 2)));
		curX.scaleCol(2, 1.0/sqrt(eigenValues.get(eigenValues.sizeCol() - 3)));

		MMatrix meanX = curX.meanCol();

		for(size_t t = 0; t < curX.sizeRow(); t++)
			curX.axpyRowRow(t, meanX, 0, -1.0);
 	
		mFakeFactors[0]->setMMatrix(mSegments[i-1], 0, curX);
  
		mFactors[0]->assign(0.0, i - 1, 0);
		mFactors[0]->assign(10.0 / numRow, i - 1, 1);
   	}

 	MMatrix templateMat;
  	size_t len = mSegments[1];
 	templateMat.submat(mCentredFeaVec, 0, len, 0, mCentredFeaVec.sizeCol());
	mFactors[0]->assign(0.1, 0, 0);

	for(size_t i = 1; i < mSegments.size() - 1; i++)
	{
		MMatrix rowI,temp;
		rowI.submat(mCentredFeaVec,mSegments[i], 1, 0, mCentredFeaVec.sizeCol());
		temp.repmat(rowI, len, 1);
 	 
		temp.axpy(templateMat, -1.0);
		temp.pow(1);
		MMatrix sumRow = temp.sumRow();
		int index = sumRow.minIndex();
		mFactors[0]->assign(/*index * mFactors[0]->get(0,1) + */mFactors[0]->get(0, 0), i, 0);
   	}
}

void HMGPModel::update()  const
{
	if(!mIsUpdated)
	{
		updateFakeFactor();
		mKernel->computeKernel(mK,mFakeFactors);
 		
  		mInvK = mK;
	 	mLnDK = MMatrix::invertMMatrix(mInvK);
	 	mScaleFeaVec.copy(mCentredFeaVec);
		for(size_t i = 0; i < mDimData; i++)
		{
			mScaleFeaVec.scaleCol(i, mW.get(i));
		}
   		mInvKFeaVec.gemm(mInvK, mScaleFeaVec, 1.0, 0.0, "N", "N");
 
		//state
		mStateKernel->computeKernel(mKs, mStatesFactor);
		mInvKs = mKs;
		mLnDKs = MMatrix::invertMMatrix(mInvKs);

		mScaleX.copy(*mFakeFactors[0]);
		for(size_t i = 0; i < mScaleX.sizeCol(); i++)
		{
			mScaleX.scaleCol(i, mWs.get(i));
		}
 		
		mIsUpdated = true;
 	}
	
}

void HMGPModel::updateCovGradient() const
{
	// update cov of Kernel K
	mGradientK = mInvK;				
	mGradientK.scale(mDimData);		 
	mGradientK.gemm(mInvKFeaVec, mInvKFeaVec, -1.0, 1.0, "N", "T"); //DL/DK =  D*invK - InvKY*Y'(invK)'
	mGradientK.setSymmetric(true);
	mGradientK.scale(0.5);

	//update cov of Kernel Ks
	mGradientKs = mInvKs;
	mGradientKs.scale(mDimState);
	MMatrix invKsX = mInvKs * mScaleX;
	mGradientKs.gemm(invKsX, invKsX, -1.0, 1.0, "N", "T");
	mGradientKs.setSymmetric(true);
	mGradientKs.scale(0.5);
}

void HMGPModel::initScaleFactor(bool flag)
{
 	updateFakeFactor();
 
	mKernel->computeKernel(mK, mFakeFactors);
 
	mInvK = mK;
	mLnDK = MMatrix::invertMMatrix(mInvK);

	mInvKFeaVec = mInvK * mCentredFeaVec;   //(mInvK,mCentredY,1.0,0.0,"N","N");
	mScaleFeaVec.copy(mCentredFeaVec);

	for (size_t i = 0; i < mDimData; i++)
	{
		if(flag)
		{
			double denom = mCentredFeaVec.dotColCol(i, mInvKFeaVec, i);
		
			if(fabs(denom) > (1e-8))
				mW.assign(sqrt(mNumData/denom), i);
			else
				mW.assign(1, i);
 		}		
		mScaleFeaVec.scaleCol(i, mW.get(i));
 	}

	//----------------------------------------------------------------------------------------
//	std::cout << mStatesFactor << std::endl;
	mStateKernel->computeKernel(mKs, mStatesFactor);
//	std::cout << mKs << std::endl;
	mInvKs = mKs;
	mLnDKs = MMatrix::invertMMatrix(mInvKs);
	
	mScaleX = *mFakeFactors[0];
	for(std::size_t i = 0; i < mScaleX.sizeCol(); i++)
	{
		if(flag)
		{
			MMatrix invKsX = mInvKs * (*mFakeFactors[0]);
			double denom = mFakeFactors[0]->dotColCol(i, invKsX, i);
		
			if(fabs(denom) > (1e-8))
				mWs.assign(sqrt(mNumData/denom), i);
			else
				mWs.assign(1, i);
		}
		mScaleX.scaleCol(i, mWs.get(i));
	}
}

void HMGPModel::updateFakeFactor() const
{
	//TODO modify
	for (size_t i = 1; i < mSegments.size(); i++)
	{
		for (size_t t = mSegments[i-1]; t < mSegments[i]; t++)
		{
			mStatesFactor.assign(mFactors[0]->get(i-1, 0) + (t - mSegments[i-1]) * mFactors[0]->get(i-1, 1), t);
		}
	}
 
	for(size_t i = 1; i < mFactors.size(); i++)
	{
		for(size_t j = 0; j < mNumData; j++)
		{
	 		mFakeFactors[i]->copyRowRow(j, *mFactors[i], mPoseProperties[j][i-1]);
 		}
 	}
}

double HMGPModel::logLikelihood() const
{
	//log likelihood  ln p(phi|Y)_MAP = ln p(Y|phi) + ln p(phi)
	//ln p(phi) = ln p(X,S,Theta)
	//assume X,S and Theta independent of each other 
	//then ln p(phi) = ln p(X) + ln p(S) + ln p(Theta)   
	//ln p(X)_map = ln p(X|t) + ln p(t)
	update();
	double L = 0;
	//----------------------------------------ln p(Y|theta)-----------------------------------------------
	// ln p(Y|phi) = N*segma ln w_j - ND/2*ln(2PI) - D/2ln|K| - 1/2*segma w_j^2*(Y_:,j)^T*K^-1*Y_:,j 
	L = -0.5 * mDimData * mLnDK;

	//N * segma ln w_j 
	for(std::size_t i = 0; i < mDimData; i++)
	{
		L += mNumData * log(mW.get(i));
	}

	// - 1/2*segma w_j^2*(Y_:,j)^T*K^-1*Y_:,j 
	MMatrix Ytrans = mScaleFeaVec;
 	Ytrans.transpose();
 	MMatrix tmp = mInvKFeaVec * Ytrans;    //tmp = inv(K)*YW WY'
 
	L -= 0.5 * tmp.trace();  // L += tr(temp)
    //-------------------------------------ln p(X|t)----------------------------------------------------------------
  
	L -= 0.5 * mDimState * mLnDKs;

	//std::cout << mWs << std::endl;
	for(std::size_t i = 0; i < mDimState; i++)
	{
		L +=  mNumData * log(mWs.get(i));
	}

	MMatrix xTrans = mScaleX;
	xTrans.transpose();
	MMatrix xtmp = mInvKs * mScaleX * xTrans;

	L -=  0.5 * xtmp.trace();

	//-------------------------------------ln p(theta)----------------------------------------------------------------
	
	return L;
}



double HMGPModel::logLikelihoodGradient(MMatrix& g) const
{
	double L = logLikelihood();
	g.zeroElements();

	updateCovGradient();
 
 	//d_K / d_X;
	mKernel->getGradientX(mGx, mFakeFactors);
 
	for(std::size_t i = 0; i < mGx.size(); i++)
	{
		mGx[i]->scaleDiag(0.5);
		mGx[i]->dotMul(mGradientK);
 		 
		MMatrix sumRow = mGx[i]->sumRow();
		sumRow.scale(2.0);
		
		for(size_t j = 0; j < mNumData; j++)
 		{
			if(i < 3)  // grad Factor X;
			{
		 		g.assign(sumRow.get(j), i * sumRow.getElemNum() + j);
		  	}
			else if(i < mNumFactor1 + 3)  //grad Factor1
			{
				int index = mFakeFactors[0]->getElemNum() + mPoseProperties[j][0] + (i - 3) * mNumFactor1;
				g.add(sumRow.get(j), index);
			} 
			else  //grad Factor2
			{
				int index = mFakeFactors[0]->getElemNum()  + mFactors[1]->getElemNum() + mPoseProperties[j][1] + (i - (mNumFactor1 + 3)) * mNumFactor2;
				g.add(sumRow.get(j), index);
			}
		}
	}

	//------------------------------------------------------dlp(X|t)/dx -----------------------------------------------------
	MMatrix grad_x = mInvKs * mScaleX;
  
	for(std::size_t i = 0; i < grad_x.sizeCol(); i++)
		grad_x.scaleCol(i, mWs.get(i));
	 
	for(std::size_t i = 0; i < grad_x.getElemNum(); i++)
		g.add(grad_x.get(i), i);
	
	//---------------------------------------------------------------------------------------------------------------------- 
	std::size_t offset = mFakeFactors[0]->getElemNum() + mFactors[1]->getElemNum() + mFactors[2]->getElemNum();
	
	//-------------------------------------------------------DL/DW ----------------------------------------------------------
 	for(size_t i = 0; i < mDimData; i++)
	{
		double wi = mInvKFeaVec.dotColCol(i, mCentredFeaVec, i) - mNumData / mW.get(i);
 		g.assign(wi * mPostiveTransform.Gradient(mW.get(i)), offset + i);
	}
	offset += mW.getElemNum();

	//-------------------------------------------------------DL/Dhyper_layer1 ----------------------------------------------------------
	MMatrix tempG(1,mKernel->getNumParams());
	mKernel->getGradTransParams(tempG, mFakeFactors, mGradientK);
	g.setMMatrix(0, offset, tempG);
	offset += tempG.getElemNum();


	//-------------------------------------------------------DL/DWs ----------------------------------------------------------
  	for(size_t i = 0; i < mFakeFactors[0]->sizeCol(); i++)
	{
		MMatrix invksScaleX = mInvKs * mScaleX;
		double wi = invksScaleX.dotColCol(i, *mFakeFactors[0], i) - mNumData / mWs.get(i);
		g.assign(wi * mPostiveTransform.Gradient(mWs.get(i)), offset + i);
	}
	offset += mWs.getElemNum();
 
	//-------------------------------------------------------DL/Dhyper_layer2 ----------------------------------------------------------
 	MMatrix grad_hyperparams(1,mStateKernel->getNumParams()); 
	mStateKernel->getGradTransParams(grad_hyperparams, mStatesFactor, mGradientKs);
	g.setMMatrix(0, offset, grad_hyperparams);
	offset += grad_hyperparams.getElemNum();

	//-------------------------------------------------------DL/Dt----------------------------------------------------------
 //	mGt[0]->zeroElements();
 //	mStateKernel->getGradientX(mGt, mStatesFactor);
 //
	//mGt[0]->scaleDiag(0.5);
	//mGt[0]->dotMul(mGradientKs);
 //		 
	//MMatrix sumCol = mGt[0]->sumRow();
	//sumCol.scale(2.0);
 //
	////gL / gt
	//for (size_t i = 1; i < mSegments.size(); i++)
	//{
	//	for (size_t t = mSegments[i-1]; t < mSegments[i]; t++)
	//	{
	//		g.add(sumCol.get(t),offset + i - 1);
	//		g.add(sumCol.get(t) * (t - mSegments[i-1]), offset + i - 2 + mSegments.size());
	//	}
	//}

	//for (std::size_t i = 0; i < mFactors[0]->getElemNum(); i++)
	//{
	//	g.assign(g.get(offset + i) * mFactors[0]->get(i), offset + i);
	//}
 
//	std::cout << g << std::endl;
	return L;
}

void HMGPModel::getOptiParams(MMatrix& param) const
{
	std::size_t index = 0;


	for (std::size_t i = 0; i < mFakeFactors[0]->getElemNum(); i++)
 		param.assign(mFakeFactors[0]->get(i), index++);
	
	for (std::size_t i = 0; i < mFactors[1]->getElemNum(); i++)
 		param.assign(mFactors[1]->get(i), index++); 
	
	for (std::size_t i = 0; i < mFactors[2]->getElemNum(); i++)
 		param.assign(mFactors[2]->get(i), index++);  
  	
	for (std::size_t i = 0; i < mW.getElemNum(); i++)
		param.assign(mPostiveTransform.XtoA(mW.get(i)), index++);  

	for (std::size_t i = 0; i < mKernel->getNumParams(); i++)
		param.assign(mKernel->getTransParam(i), index++); 

	//-------------------------------------------------------------------------
 	 
	for (std::size_t i = 0; i < mWs.getElemNum(); i++)
 		param.assign(mPostiveTransform.XtoA(mWs.get(i)), index++);  
 	
	for (std::size_t i = 0; i < mStateKernel->getNumParams(); i++)
		param.assign(mStateKernel->getTransParam(i), index++); 
	//t
	//for (std::size_t i = 0; i < mFactors[0]->getElemNum(); i++)
	//	param.assign(mPostiveTransform.XtoA(mFactors[0]->get(i)), index++);

}

void HMGPModel::setOptiParams(const MMatrix& param)
{
	// Factor 1,Factor 2, Factor X, Factor theta0, step, hyperparameter, W,
	std::size_t index = 0;
	
	for(std::size_t i = 0; i < mFakeFactors[0]->getElemNum(); i++)
 		mFakeFactors[0]->assign(param.get(index++), i);
	
	for(std::size_t i = 0; i < mFactors[1]->getElemNum(); i++)
 		mFactors[1]->assign(param.get(index++), i);
	
	for(std::size_t i = 0; i < mFactors[2]->getElemNum(); i++)
 		mFactors[2]->assign(param.get(index++), i);
 	
	for(std::size_t i = 0; i < mW.getElemNum(); i++)
 		mW.assign(mPostiveTransform.AtoX(param.get(index++)), i);

	for (size_t i = 0; i < mKernel->getNumParams(); i++)
		mKernel->setTransParam(param.get(index++), i);

	//TODO
	//-------------------------------------------------------------------------
 	for(std::size_t i = 0; i < mWs.getElemNum(); i++)
 		mWs.assign(mPostiveTransform.AtoX(param.get(index++)), i);

	for (size_t i = 0; i < mStateKernel->getNumParams(); i++)
		mStateKernel->setTransParam(param.get(index++), i); 

	//for(std::size_t i = 0; i < mFactors[0]->getElemNum(); i++)
 //		mFactors[0]->assign(mPostiveTransform.AtoX(param.get(index++)), i);
 
	mIsUpdated = false;
}

std::size_t HMGPModel::getOptiParamsNum() const
{
	std::size_t numParams = 0;
 
	numParams += mFakeFactors[0]->getElemNum();

	numParams += mFactors[1]->getElemNum();
	numParams += mFactors[2]->getElemNum();
 
	numParams += mW.getElemNum();
	numParams += mKernel->getNumParams();

	//------------------------------------------------------------
	numParams += mWs.getElemNum();
 	numParams += mStateKernel->getNumParams();
 //	numParams += mFactors[0]->getElemNum();
 
	return numParams;
}

double HMGPModel::computeObjectiveVal()
{
	return -logLikelihood();
}

double HMGPModel::computeObjectiveGradParams(MMatrix & g)
{
	return -logLikelihoodGradient(g);
}

void HMGPModel::writeModelToFile(std::string name)
{
	if(!mTtrained) return;

	std::ofstream archiver(name, std::ios::binary|std::ios::out);
	
	std::size_t numFactor1 = mFactor1Ls.size();
	archiver.write((char*)(&numFactor1), sizeof(size_t));
	for(std::size_t i = 0; i < numFactor1; i++)
	{
		char factor[100];
		std::stringstream converter(mFactor1Ls[i].c_str());
		converter >> factor;
		archiver.write(factor, 100);
	}

	//action
	std::size_t numFactor2 = mFactor2Ls.size();
	archiver.write((char*)(&numFactor2), sizeof(size_t));
	for(std::size_t i = 0; i < numFactor2; i++)
	{
		char factor[100];
		std::stringstream converter(mFactor2Ls[i].c_str());
		converter >> factor;
		archiver.write(factor, 100);
	}

	//FeaVec
	archiver.write((char*)(&mNumData), sizeof(size_t));
	archiver.write((char*)(&mDimData), sizeof(size_t));
	archiver.write((char*)(mFeaVec.gets()), sizeof(double) * mFeaVec.getElemNum());

	std::size_t dim = 1;
	//mW
	archiver.write((char*)(&dim), sizeof(size_t));
	archiver.write((char*)(&mDimData), sizeof(size_t));
	archiver.write((char*)(mW.gets()), sizeof(double) * mW.getElemNum());
	//mWs
	archiver.write((char*)(&dim), sizeof(size_t));
	archiver.write((char*)(&mDimState), sizeof(size_t));
	archiver.write((char*)(mWs.gets()), sizeof(double) * mWs.getElemNum());

	std::size_t col = mInitFeaVec.sizeCol();
	std::size_t row = mInitFeaVec.sizeRow();

	//InitFeaVec
	archiver.write((char*)(&row), sizeof(size_t));
	archiver.write((char*)(&col), sizeof(size_t));
	archiver.write((char*)(mInitFeaVec.gets()), sizeof(double)*mInitFeaVec.getElemNum());

	//Factors
	row = mFactors.size();
	archiver.write((char*)(&row), sizeof(size_t));
	for(std::size_t i = 0; i < mFactors.size(); i++)
	{
		std::size_t col = mFactors[i]->sizeCol();
		std::size_t row = mFactors[i]->sizeRow();

		archiver.write((char*)(&row), sizeof(size_t));
		archiver.write((char*)(&col), sizeof(size_t));
		archiver.write((char*)(mFactors[i]->gets()), sizeof(double)*mFactors[i]->getElemNum());
  	}

	col = mFakeFactors[0]->sizeCol();
	row = mFakeFactors[0]->sizeRow();

	archiver.write((char*)(&row), sizeof(size_t));
	archiver.write((char*)(&col), sizeof(size_t));
	archiver.write((char*)(mFakeFactors[0]->gets()), sizeof(double)*mFakeFactors[0]->getElemNum());

	//params
	MMatrix hyparams(1,mKernel->getNumParams());
	mKernel->getPrams(hyparams);
	col = hyparams.sizeCol();
	row = hyparams.sizeRow();
	archiver.write((char*)(&row), sizeof(size_t));
	archiver.write((char*)(&col), sizeof(size_t));
	archiver.write((char*)(hyparams.gets()), sizeof(double)*hyparams.getElemNum());
  
	//state params
	hyparams.resize(1,mStateKernel->getNumParams());
	mStateKernel->getParams(hyparams);
	col = hyparams.sizeCol();
	row = hyparams.sizeRow();
	archiver.write((char*)(&row), sizeof(size_t));
	archiver.write((char*)(&col), sizeof(size_t));
	archiver.write((char*)(hyparams.gets()), sizeof(double)*hyparams.getElemNum());

	//-------------------------------------------------------------------------------
	mSegments.pop_back();
	std::size_t size = mSegments.size();
	archiver.write((char*)(&size), sizeof(size_t));
	archiver.write((char*)(&mSegments[0]), sizeof(size_t) * size);
	mSegments.push_back(mFeaVec.sizeRow());

	row = mSegmentLabels.size();
	col = mSegmentLabels[0].size();
 	archiver.write((char*)(&row), sizeof(size_t));
 	archiver.write((char*)(&col), sizeof(size_t));
	
	for(std::size_t i = 0; i < row;i++)
 	{
		archiver.write((char*)(&mSegmentLabels[i][0]), sizeof(size_t) * col);
 	}
}

void HMGPModel::loadModelFromFile(std::string name)
{
	std::ifstream loader(name,std::ios::binary|std::ios::in);
	
	if(loader.fail()) 
		return;

	std::size_t numFactor1;
	loader.read((char*)(&numFactor1),sizeof(size_t));
	mFactor1Ls.resize(numFactor1);
	for(std::size_t i = 0; i < numFactor1; i++)
	{
		char factor[100];
		loader.read(factor,100);
		std::stringstream converter(factor);
 		converter >> mFactor1Ls[i];
	}

	std::size_t numFactor2;
	loader.read((char*)(&numFactor2),sizeof(size_t));
	mFactor2Ls.resize(numFactor2);
	for(std::size_t i = 0; i < numFactor2; i++)
	{
		char factor[100];
		loader.read(factor,100);
		std::stringstream converter(factor);
 		converter >> mFactor2Ls[i];
	}

	std::size_t row, col;

	//FeaVec
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mFeaVec.resize(row,col);
	loader.read((char*)(mFeaVec.gets()),sizeof(double) * mFeaVec.getElemNum());

	//W
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mW.resize(row,col);
	loader.read((char*)(mW.gets()),sizeof(double) * mW.getElemNum());

	//Ws
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mWs.resize(row,col);
	loader.read((char*)(mWs.gets()),sizeof(double) * mWs.getElemNum());
	
	//InitFeaVec
	loader.read((char*)(&row), sizeof(size_t));
	loader.read((char*)(&col), sizeof(size_t));
	mInitFeaVec.resize(row, col);
	loader.read((char*)(mInitFeaVec.gets()), sizeof(double)*mInitFeaVec.getElemNum());
	
	//factors
	std::size_t numFactors = 0;
	loader.read((char*)(&numFactors), sizeof(size_t));
	mFactors.resize(numFactors);
	for(std::size_t i = 0; i < numFactors; i++)
	{
		loader.read((char*)(&row), sizeof(size_t));
		loader.read((char*)(&col), sizeof(size_t));
		mFactors[i] = new MMatrix(row,col);
		loader.read((char*)(mFactors[i]->gets()),sizeof(double)*mFactors[i]->getElemNum());
 	}

	//Factor X
	loader.read((char*)(&row), sizeof(size_t));
	loader.read((char*)(&col), sizeof(size_t));
	mFakeFactors.resize(mFactors.size());
	mFakeFactors[0] = new MMatrix(row,col);
	mFakeFactors[1] = new MMatrix(row, mFactors[1]->sizeCol());
	mFakeFactors[2] = new MMatrix(row, mFactors[2]->sizeCol());
 	loader.read((char*)(mFakeFactors[0]->gets()), sizeof(double)*mFakeFactors[0]->getElemNum());

	//--------------------------------------------------------------------------------------------
 	initKernel(mFeaVec.sizeRow());
	//----------------------------------Load Hyperparameters----------------------------------------------------------

	loader.read((char*)(&row), sizeof(size_t));
	loader.read((char*)(&col), sizeof(size_t));
	ResUtil::MMatrix hyparams(row, col);
	loader.read((char*)(hyparams.gets()), sizeof(double) * hyparams.getElemNum());
	mKernel->setPrams(hyparams);
	
	loader.read((char*)(&row), sizeof(size_t));
	loader.read((char*)(&col), sizeof(size_t));
	hyparams.resize(row, col);
	loader.read((char*)(hyparams.gets()), sizeof(double) * hyparams.getElemNum());
	mStateKernel->setParams(hyparams);
	
	//-----------------------------------------------------------------------------------------------
	//mSegments
	std::size_t size;
	loader.read((char*)(&size),sizeof(size_t));
	mSegments.resize(size);
	loader.read((char*)(&mSegments[0]),sizeof(size_t)*size);
   
	//mSegments
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mSegmentLabels.resize(row);
 	for(std::size_t i = 0; i < row; i++)
	{
		mSegmentLabels[i].resize(col);
		loader.read((char*)(&mSegmentLabels[i][0]),sizeof(size_t)*col);
 	}
}

MMatrix HMGPModel::meanPrediction(const std::vector<MMatrix> &X ,CVector3D<double> initPos)
{
	std::vector<const MMatrix*> tempX;
	
	for(std::size_t t = 0; t < X.size(); t++)
		tempX.push_back(&X[t]);

	std::size_t motionLen = X[0].sizeRow();
	MMatrix Kx(motionLen, mNumData);
	mKernel->computeKernel(Kx, tempX, mFakeFactors);
	 
	MMatrix &Ymean = Kx * mInvK * mCentredFeaVec;
	
	for(std::size_t i = 0; i < Ymean.sizeCol(); i++)
	{
		Ymean.scaleCol(i, sqrt(mVarFeaVec.get(i)));
 	}

	MMatrix meanData;
	meanData.repmat(mMeanFeaVec, motionLen, 1);
 	
 	Ymean += meanData; 
 
	
	MMatrix motion(Ymean.sizeRow(), mInitFeaVec.sizeCol());
	motion.copyMMatrix(0, 0, Ymean, 0, Ymean.sizeRow(), 0, mInitFeaVec.sizeCol());
	
	//TODO
	motion.assign(initPos.x, 0, 0);
	motion.assign(initPos.y, 0, 1);
	motion.assign(initPos.z, 0, 2);
	 
	for (std::size_t i = 1; i < motionLen; i++)
	{
	/*	motion.assign(motion.get(i, 0), i, 0);
		motion.assign(motion.get(i, 1), i, 1);
		motion.assign(motion.get(i, 2), i, 2);*/

		motion.add(motion.get(i-1, 0), i, 0);
		motion.add(motion.get(i-1, 1), i, 1);
		motion.add(motion.get(i-1, 2), i, 2);
	}
  	
	mocapToEulerAngle(motion);
  
	for(std::size_t i = 3; i < motion.sizeCol(); i++)
	{
		motion.scaleCol(i, 180.0/PI);
 	}

	//for(std::size_t i = 0; i < motion.sizeRow(); i++)
	//{
	//	std::cout << motion.get(i,0) <<  " ,";
	//	std::cout << motion.get(i,1) <<  " ,";
	//	std::cout << motion.get(i,2) <<  " ,";
	//	std::cout << motion.get(i,3) <<  " ,";
	//	std::cout << motion.get(i,4) <<  " ,";
	//	std::cout << motion.get(i,5) <<  std::endl;
 //	}
	//
	return motion;
}
void HMGPModel::constructStepPrior()
{
	MMatrix A(mSegments.size() - 1, mFactors[1]->sizeCol() * mFactors[2]->sizeCol());
	MMatrix B(mSegments.size() - 1, 1);
	
	size_t col1 = mFactors[1]->sizeRow();
	size_t col2 = mFactors[2]->sizeRow();

	int curIndex = 0;
	for (size_t i = 0; i < col1; i++)
	{
		for (size_t j = 0; j < col2; j++)
		{
			int  index = -1;
			for (std::size_t t = 0; t < mSegmentLabels.size(); t++)
			{
				if (i == mSegmentLabels[t][0] && j == mSegmentLabels[t][1])
				{
					index = int(t);
					break;
				}
 			}
 			if (index >= 0)
			{
				for (size_t t = 0; t < mFactors[1]->sizeCol(); t++)
				{
					for (size_t k = 0; k < mFactors[2]->sizeCol(); k++)
					{
						double val = mFactors[1]->get(i, t) * mFactors[2]->get(j, k);
						A.assign(val, curIndex, t * mFactors[2]->sizeCol() + k);
					}
				}
		 		B.assign(mFactors[0]->get(index, 1), curIndex);
				curIndex++;
			}
		}
	}
//	std::cout << A << std::endl; 
	mGPM  = new GuaasinProcessModel(B,A);
	mGPM->optimize(100);
	MMatrix ret = mGPM->predict(A);
	/*std::cout << A << std::endl;
	std::cout << ret << std::endl;
	std::cout << B << std::endl;*/
}
MMatrix HMGPModel::predictX(std::size_t indexF1, std::size_t indexF2, std::size_t length) const
{
	MMatrix mat(1, mFactors[1]->sizeCol() * mFactors[2]->sizeCol());

	for(size_t t = 0; t < mFactors[1]->sizeCol(); t++)
	{
		for(size_t k = 0; k < mFactors[2]->sizeCol(); k++)
		{
			double val = mFactors[1]->get(indexF1, t) * mFactors[2]->get(indexF2, k);
			mat.assign(val,t *  mFactors[2]->sizeCol() + k);
		}
	}

	MMatrix val = mGPM->predict(mat);
	double step  = val.get(0);

	MMatrix tempS(length, 1);
	for(std::size_t i = 0; i < length; i++)
	{
		tempS.assign(i* step, i, 0);
	}
	std::ofstream fv("txt.txt");
	std::streambuf *oldbuf  = std::cout.rdbuf(fv.rdbuf());

	MMatrix Kx(length, mNumData);
	mStateKernel->computeKernel(Kx, tempS, mStatesFactor);

	MMatrix Xmean = Kx * mInvKs * (*mFakeFactors[0]);
	std::cout << mW << std::endl;
	std::cout << mWs << std::endl;
	//std::cout << mKs << std::endl;
	//std::cout << mStatesFactor << std::endl;
 	//std::cout << Xmean << std::endl;
	/*std::cout << "Params" << std::endl;
	for(std::size_t i = 0; i < mStateKernel->getNumParams(); i++)
		std::cout << mStateKernel->getParam(i) << " " << std::endl;*/

	MMatrix m(mSegments[1], 3);
 	for (std::size_t i = mSegments[0]; i < mSegments[1]; i++)
		  m.copyRowRow(i, *mFakeFactors[0], i);  //std::cout << mFakeFactors[0]->get(i,0) << " " <<  mFakeFactors[0]->get(i,1) <<  mFakeFactors[0]->get(i,2) << std::endl;
	//m-Xmean
	//std::cout << *mFactors[0] << std::endl;
	//std::cout << mWs << std::endl;
	//std::cout << *mFakeFactors[0] << std::endl;
	std::cout.rdbuf(oldbuf);
	return Xmean;
}
MMatrix HMGPModel::reconstruction(std::size_t index)
{
	assert(index < mSegmentLabels.size());
	std::size_t identity =  mSegmentLabels[index][0];
	std::size_t content  =  mSegmentLabels[index][1];

	std::size_t length = mSegments[index + 1] - mSegments[index];

	double step = mFactors[0]->get(index, 1);

	std::vector<MMatrix> X(3);
	X[0].resize(length,3);
	X[1].resize(length, mFactors[1]->sizeCol());
	X[2].resize(length, mFactors[2]->sizeCol());

	for(size_t i = 0; i < length; i++)
	{
//		X[0].assign(cos(mFactors[0]->get(motionIndex,0) + double(i * step)), i, 0);
//		X[0].assign(sin(mFactors[0]->get(motionIndex,0) + double(i * step)), i, 1);
		X[0].copyRowRow(i, *mFakeFactors[0], mSegments[index] + i);
   		X[1].copyRowRow(i, *mFactors[1], identity);
		X[2].copyRowRow(i, *mFactors[2], content);
	}
	//std::cout << X[0] << std::endl;
	return meanPrediction(X,CVector3D<double>(0, mInitFeaVec.get(index, 1), 0));
}
MMatrix HMGPModel::generate(std::size_t indexF1, std::size_t indexF2, std::size_t length)
{
	//assert(index < mSegmentLabels.size());
//	std::size_t identity =  mSegmentLabels[index][0];
//	std::size_t content  =  mSegmentLabels[index][1];

//	std::size_t length = mSegments[index + 1] - mSegments[index];

//	double step = mFactors[0]->get(index,1);

	std::vector<MMatrix> X(3);
	X[0].resize(length,3);
	X[1].resize(length, mFactors[1]->sizeCol());
	X[2].resize(length, mFactors[2]->sizeCol());

	for(size_t i = 0; i < length; i++)
	{
//		X[0].assign(cos(mFactors[0]->get(motionIndex,0) + double(i * step)), i, 0);
//		X[0].assign(sin(mFactors[0]->get(motionIndex,0) + double(i * step)), i, 1);
	//	X[0].copyRowRow(i, *mFakeFactors[0], mSegments[index] + i);
   		X[1].copyRowRow(i, *mFactors[1], indexF1);
		X[2].copyRowRow(i, *mFactors[2], indexF2);
	}
	X[0] = predictX(indexF1, indexF2, length);
	//std::cout << X[0] << std::endl;
	return meanPrediction(X,CVector3D<double>(0, mInitFeaVec.get(indexF1, 1), 0));
}