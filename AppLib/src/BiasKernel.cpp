#include "BiasKernel.h"

using namespace ResCore;
void BiasKernel::initKernel()
{
	mKernelType = "BIAS";
	setName("Kernel_Bias");

	mhps.push_back(std::make_pair("variance", exp(-2.0)));
	addTransform(CTransform::DefaultPositive(), 0);
}
void BiasKernel::getGradientX(MMatrix& g, const MMatrix& X, size_t ptNum,const MMatrix& X2,bool addG) const
{
	if(!addG)
		g.zeroElements();
}
void BiasKernel::getDiagGradientX(MMatrix& g, const MMatrix& X, bool addG) const
{
	if(!addG)
		g.zeroElements();
}
void BiasKernel::computeKernel(MMatrix& K, const MMatrix& X) const
{
	assert(K.rowsMatch(X));
	assert(K.isSquare());
	K.assign(mhps[0].second);
	K.setSymmetric(true);
}
void BiasKernel::computeKernel(MMatrix& K, const MMatrix& X, const MMatrix& X2) const
{
	assert(K.rowsMatch(X));
	assert(K.sizeCol() == X2.sizeRow());
	K.assign(mhps[0].second);
}
void BiasKernel::computeKernel(MMatrix& K, const MMatrix& X, const MMatrix& X2, size_t row) const
{
	assert(K.rowsMatch(X));
	assert(K.sizeCol() == 1);
	K.assign(mhps[0].second);
}
double BiasKernel::getGradientParam(size_t index, const MMatrix& X, const MMatrix& X2, const MMatrix& covGrd) const
{
	assert(index==0);
	return covGrd.sumElement();
}

double BiasKernel::getGradientParam(size_t index, const MMatrix& X, const MMatrix& covGrd) const
{
	assert(index == 0);
	return covGrd.sumElement();
}