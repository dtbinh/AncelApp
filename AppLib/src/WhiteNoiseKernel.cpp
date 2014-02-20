#include <cassert>
#include "WhiteNoiseKernel.h"

using namespace ResCore;

void WhiteNoiseKernel::initKernel()
{
	mKernelType = "WHITE";
    setName("Kernel_WHITE");
	mhps.push_back(std::make_pair("Variance",exp(-1.0)));
	addTransform(CTransform::DefaultPositive(), 0);
}
 
double WhiteNoiseKernel::computeDiagElement(const MMatrix & X,size_t index) const
{
	 return mhps[0].second;
}
void   WhiteNoiseKernel::computeDiag(MMatrix &d, const MMatrix &X) const
{
	 assert(d.sizeCol()==1);
	 assert(X.rowsMatch(d));
	 d.assign(mhps[0].second);
}
		
void   WhiteNoiseKernel::setParam(double paramVal,size_t index)
{
	assert(index == 0);
	mhps[0].second = paramVal;
}
double WhiteNoiseKernel::getParam(size_t index) const
{
	assert(index == 0);
	return mhps[0].second;
}
 		 
void   WhiteNoiseKernel::getGradientX(MMatrix &g, const MMatrix& X, size_t uRow,const MMatrix&X2,bool addG) const
{
	assert(g.sizeRow() == X2.sizeRow());
	assert(uRow < X.sizeRow());
	assert(X.sizeCol()==X2.sizeCol());
	if(!addG)
		g.zeroElements();
}
void   WhiteNoiseKernel::getDiagGradientX(MMatrix &g, const MMatrix &X,bool addG) const
{
	assert(g.dimensionsMatch(X));
	if(!addG)
		g.zeroElements();
}

double WhiteNoiseKernel::getWhiteNoise() const
{
	return mhps[0].second;
}

double WhiteNoiseKernel::computeElement(const MMatrix &X1, size_t uIndex1,const MMatrix &X2, size_t uIndex2) const
{
	return 0.0;
}
void   WhiteNoiseKernel::computeKernel(MMatrix &K, const MMatrix &X) const
{
	assert(K.rowsMatch(X));
	assert(K.isSquare());
	K.zeroElements();
	for(size_t i = 0; i < K.sizeRow(); i++)
		K.assign(mhps[0].second,i,i);
	K.setSymmetric(true);
}
void   WhiteNoiseKernel::computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2) const
{
	assert(K.rowsMatch(X));
	assert(K.sizeCol() == X2.sizeRow());
	K.zeroElements();
}

void   WhiteNoiseKernel::computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2, size_t row) const
{
	assert(K.rowsMatch(X));
	assert(K.sizeCol() == 1);
	K.zeroElements();
}
double WhiteNoiseKernel::getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & cvGrd) const
{
	return 0.0;
}
double WhiteNoiseKernel::getGradientParam(size_t index, const MMatrix &X, const MMatrix &cvGrd) const
{
	return cvGrd.trace();
}
