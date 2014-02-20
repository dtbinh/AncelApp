
#include "MGPKernel.h"

using namespace ResCore;

MGPKernel::MGPKernel(size_t numData)
	:mNumData(numData),mIsUpdate(false)
{
 	mKernels.push_back(new RBFKernel());
  	mKernels.push_back(new LinearKernel());
  	mKernels.push_back(new LinearKernel());
 
	mNoiseKernel = new WhiteNoiseKernel();
	for(size_t i = 0; i < mKernels.size(); i++)
	{
 		mUnits.push_back(new MMatrix(numData,numData));
		mComMult.push_back(new MMatrix(numData,numData));
	}
}
MGPKernel::~MGPKernel()
{
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		delete mKernels[i];
		delete mUnits[i];
	}
	delete mNoiseKernel;
}

void  MGPKernel::getGradientX(std::vector<MMatrix*>&g,const std::vector<MMatrix*> &factor)
{
	if(!mIsUpdate)
	{
		for(size_t i = 0; i < mKernels.size(); i++)
		{
   			mKernels[i]->computeKernel(*mUnits[i],*factor[i]);
		}
  	}	
 
	size_t colIndex = 0;
 	
    for(size_t i = 0; i < mKernels.size(); i++)
	{
		std::vector<MMatrix*> gX;
		mComMult[i]->oneElements();
		mComMult[i]->setSymmetric(true);

		for(size_t t = 0; t < factor[i]->sizeCol(); t++)
		{	
 			gX.push_back(g[colIndex + t]);
		}
		mKernels[i]->getGradientX(gX, *factor[i]);
		 
		for(size_t t = 0; t < mKernels.size(); t++)
		{
			if(t != i)
				mComMult[i]->dotMul(*mUnits[t]);
	  	}
		for(size_t t = 0; t < factor[i]->sizeCol(); t++)
		{
			gX[t]->dotMul(*mComMult[i]);
		}
		colIndex += factor[i]->sizeCol();
	}
	mIsUpdate = false;
}
void  MGPKernel::computeKernel(MMatrix &K, const std::vector<MMatrix*> &factor) const
{
	K.oneElements();
	for(size_t i = 0; i < mKernels.size(); i++)
	{
   		mKernels[i]->computeKernel(*mUnits[i],*factor[i]);
  		K.dotMul(*mUnits[i]);
 	}
	for(size_t i = 0; i < K.sizeCol(); i++)
	{
		K.add(mNoiseKernel->getParam(0),i,i);
	}
	 
 	mIsUpdate = true;
}
//----------------------------------------------------------------------------------------------------------
void   MGPKernel::getGradientParams(MMatrix &g, const std::vector<MMatrix*> &factor, const MMatrix &covGrad) 
{
	std::size_t index = 0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		mComMult[i]->dotMul(covGrad);
		MMatrix subG(1,mKernels[i]->getNumParams());
		mKernels[i]->getGradientParams(subG,*factor[i],*mComMult[i]);
		g.setMMatrix(0,index,subG);
		index += mKernels[i]->getNumParams();
	}
	g.assign(covGrad.trace(),g.getElemNum()-1);
}
//------------------------------------------------------------------------------------------------------
void   MGPKernel::getGradTransParams(MMatrix &g, const std::vector<MMatrix*> &factor, const MMatrix &covGrd) 
{
 	getGradientParams(g,factor,covGrd);
   
	std::size_t index = 0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{	
		
 		for(size_t j = 0;j < mKernels[i]->getNumParams(); j++)
		{
			double val = g.get(index);
			double param = mKernels[i]->getParam(j);
			g.assign(val * mKernels[i]->getGradientTransform(param, j),index++);
		}
  	}
	double val = g.get(index);
	double param = mNoiseKernel->getParam(0);
  	g.assign(mNoiseKernel->getGradientTransform(param,0),index);
}
//-----------------------------------------------------------------------------
size_t MGPKernel::getNumParams() const
{
	int numParams = 0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		numParams += mKernels[i]->getNumParams();
	}
	numParams += mNoiseKernel->getNumParams();
	return numParams;
}
//-----------------------------------------------------------------------------
double  MGPKernel::getTransParam(size_t index)
{
	size_t curIndex = 0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		if(curIndex + mKernels[i]->getNumParams() > index)
			return mKernels[i]->getTransParam(index - curIndex);
		else 
			curIndex += mKernels[i]->getNumParams();
	}
	return mNoiseKernel->getTransParam(0);
}
void  MGPKernel::getPrams(MMatrix &params)
{
	assert(params.getElemNum() == getNumParams());
	for(std::size_t i = 0; i < params.getElemNum(); i++)
	{
		params.assign(getParam(i),i);
	}
}
double MGPKernel::getParam(size_t index)
{
	size_t curIndex = 0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		if(curIndex + mKernels[i]->getNumParams() > index)
			return mKernels[i]->getParam(index - curIndex);
		else 
			curIndex += mKernels[i]->getNumParams();
	}
	return mNoiseKernel->getParam(0);
}

void MGPKernel::setPrams(MMatrix & params)
{
	assert(params.getElemNum() == getNumParams());
	for(std::size_t i = 0; i < params.getElemNum(); i++)
	{
		setParam(params.get(i),i);
 	}
}

void MGPKernel::setParam(double val, size_t index)
{
	size_t curIndex = 0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		if(curIndex + mKernels[i]->getNumParams() > index)
		{
			mKernels[i]->setParam(val,index - curIndex);
			return;
		}
		else 
			curIndex += mKernels[i]->getNumParams();
	}
	mNoiseKernel->setParam(val,0);
}

//------------------------------------------------------------------------------
void MGPKernel::setTransParam(double val,size_t index)
{
	size_t curIndex = 0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		if(curIndex + mKernels[i]->getNumParams() > index)
		{
			mKernels[i]->setTransParam(val,index - curIndex);
			return;
		}
		else 
			curIndex += mKernels[i]->getNumParams();
	}
	mNoiseKernel->setTransParam(val,0);
}
double MGPKernel::computeKernel(const std::vector<MMatrix*> &factor) const
{
	double val = 1.0;
	for(size_t i = 0; i < mKernels.size(); i++)
	{
		val *= mKernels[i]->computeDiagElement(*factor[i],0);
	}
	val += mNoiseKernel->getParam(0);
	return val;
}
void MGPKernel::computeKernel(MMatrix &Kx, const std::vector<const MMatrix*> &X,const std::vector<MMatrix*> &X2) const
{
	assert(X.size() == 3);
	size_t numX = X[0]->sizeRow();

 	for(size_t i = 0; i < numX; i++)
	{
		for(size_t j = 0; j < mNumData; j++)
		{
			double val  = 1.0;
			for(size_t t = 0; t < mKernels.size(); t++)
			{
				val *= mKernels[t]->computeElement(*X[t],i,*X2[t],j);
			}
			Kx.assign(val,i,j);
		}
 	}
}