#include "GaussianProcessModel.h"
#include "CompoundKernel.h"
#include "RBFKernel.h"
#include "LinearKernel.h"
#include "WhiteNoiseKernel.h"
#include "ResUtility.h"

using namespace ResModel;

GuaasinProcessModel::GuaasinProcessModel(MMatrix& Y,MMatrix &X,ResCore::Kernel* kernel)
	:mX(X),mY(Y),mIsNeedUpdate(false)
{
	if(kernel != NULL)
	{
		mKernel = kernel;
	}
	else
	{
		ResCore::CompoundKernel *pKernel = new ResCore::CompoundKernel();
		pKernel->addKernel(new ResCore::RBFKernel());
		pKernel->addKernel(new ResCore::LinearKernel());
 		pKernel->addKernel(new ResCore::WhiteNoiseKernel());
		int t= pKernel->getNumParams();
		pKernel->setParam(0.00001,t-1);
 		mKernel = pKernel;
	}

	mNumDim  = Y.sizeCol();
	mNumData = Y.sizeRow();
	initValues();
}
GuaasinProcessModel::~GuaasinProcessModel()
{
	if(mKernel != NULL)
		delete mKernel;
}
void GuaasinProcessModel::initValues()
{
	mMeanY = mY.meanCol();
	MMatrix tempMat(mY.sizeRow(), mY.sizeCol());
	tempMat.repmat(mMeanY, mY.sizeRow(), 1);
	mCentredY = mY;
	mCentredY.axpy(tempMat, -1.0);
	mVarY = mCentredY.varCol();

	for(std::size_t i = 0; i < mVarY.sizeCol(); i++)
	{
		if(mVarY.get(i) != 0.0f)
			mCentredY.scaleCol(i, 1.0/sqrt(mVarY.get(i)));
	}
	//std::cout  << mX << std::endl;
	mK.resize(mNumData, mNumData);
 	mKernel->computeKernel(mK, mX);

	//std::cout << mK << std::endl;

	mInvK = mK;
	mLnDK = MMatrix::invertMMatrix(mInvK);

}
ResUtil::MMatrix GuaasinProcessModel::predict(MMatrix &X)
{
	MMatrix kX(X.sizeRow(), mX.sizeRow());
	mKernel->computeKernel(kX, X, mX);
	MMatrix val = kX * mInvK * mCentredY;
	
	for (std::size_t i = 0; i < val.sizeCol();i++)
		val.scaleCol(i, sqrt(mVarY.get(i)));
	MMatrix temp(val.sizeRow(), val.sizeCol());
	temp.repmat(mMeanY,val.sizeRow(),1);
	
	val += temp;

	return val;
}
void GuaasinProcessModel::updateCovGradient() const
{
	mGraCov = mInvK;				
	mGraCov.scale(mNumDim);		 
	MMatrix invKY = mInvK * mCentredY;
	mGraCov.gemm(invKY,invKY,-1.0,1.0,"N","T"); //DL/DK =  D*invK - InvKY*Y'(invK)'
	mGraCov.setSymmetric(true);
	mGraCov.scale(0.5);
}
void GuaasinProcessModel::update() const
{
	if(mIsNeedUpdate)
	{
		mKernel->computeKernel(mK,mX);
		mInvK = mK;
		mLnDK = MMatrix::invertMMatrix(mInvK);
		mIsNeedUpdate = false;
	}
}

void GuaasinProcessModel::optimize(size_t iterNum)
{
	SetminIters(iterNum);
	RunDefaultOptimiser();
	MMatrix mat(1, mKernel->getNumParams());
	mKernel->getParams(mat);
	std::cout << mat << std::endl;
}
void GuaasinProcessModel::getOptiParams(MMatrix& param) const
{
	mKernel->getTransParams(param);
}

void GuaasinProcessModel::setOptiParams(const MMatrix& param)
{
	mKernel->setTransParams(param);
	mIsNeedUpdate = true;
}

size_t GuaasinProcessModel::getOptiParamsNum()     const
{
	return mKernel->getNumParams();
}

double GuaasinProcessModel::computeObjectiveVal()	    
{
	update();

	double L = 0.5 * ((mNumData * mNumDim * log(2 * PI)) + (mNumDim * mLnDK));

	MMatrix YT = mCentredY;
	YT.transpose();
	MMatrix temp = mInvK * mCentredY * YT;

	L += 0.5 * temp.trace();
 
	return L;
}
double GuaasinProcessModel::computeObjectiveGradParams(ResUtil::MMatrix & g)  
{
	double L = computeObjectiveVal();
	updateCovGradient();
	mKernel->getGradTransParams(g,mX,mGraCov,false);
 	return L;
}

