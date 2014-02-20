#include "LinearKernel.h"
#include <assert.h>
using namespace ResCore;

void LinearKernel::initKernel()
{
	mKernelType = "Linear";
	setName("kernel_LINEAR");
	 
	mhps.push_back(std::make_pair("variance",1.0));
	addTransform(CTransform::DefaultPositive(),0);	 
}
void LinearKernel::getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG) const
{
	assert(X.sizeCol() == gX.size());

	for(size_t i = 0; i < X.sizeRow(); i++)
	{
		for(size_t j = 0; j < i; j++)
		{
			for(size_t t = 0; t < X.sizeCol(); t++)
			{
				if(addG)
				{
					gX[t]->add(X.get(j, t) * mhps[0].second, i, j);
					gX[t]->add(X.get(i, t) * mhps[0].second, j, i);
 				}
				else
				{
					gX[t]->assign(X.get(j, t) * mhps[0].second, i, j);
					gX[t]->assign(X.get(i, t) * mhps[0].second, j, i);
 				}
 			}
		}
		for(size_t t = 0; t < X.sizeCol(); t++)
		{
			if(addG)
				gX[t]->add(2 * X.get(i, t) * mhps[0].second, i, i);
			else
 				gX[t]->assign(2 * X.get(i, t) * mhps[0].second, i, i);
 		}
	}
	//std::cout << *gX[0] << std::endl;
}
void LinearKernel::getGradientX(MMatrix &g, const MMatrix& X, size_t uRow,const MMatrix&X2 ,bool addG) const
{
	assert(g.sizeRow() == X2.sizeRow());
	assert(uRow < X.sizeRow());
	assert(X.sizeCol() == X2.sizeCol());
 /*
	for(size_t t = 0; t < X2.sizeRow(); t++)
	{
		for(size_t k = 0; k < X2.sizeCol(); k++)
		{
			double val = 0.0;
			if(addG) 
				val = g.get(t,k);
			val += m_Variance * X2.get(t,k);
			g.assign(val,t,k);
		}
	}
*/
	if(!addG)
	{
		g = X2;
		g.scale(mhps[0].second);
	}
	else {
		g.axpy(X2,mhps[0].second);
	}
}
void LinearKernel::getDiagGradientX(MMatrix &g, const MMatrix &X,bool addG) const
{
	assert(g.dimensionsMatch(X));
	if(!addG)
	{
		g = X;
		g.scale(2.0*mhps[0].second);
	}
	else 
	{
		g.axpy(X,2.0*mhps[0].second);
	}
}
double LinearKernel::computeElement(const MMatrix &X1,size_t Index1,const MMatrix &X2,size_t Index2) const
{
	assert(X1.colsMatch(X2));
	return mhps[0].second * X1.dotRowRow(Index1,X2,Index2);
}
void LinearKernel::computeKernel(MMatrix &K, const MMatrix &X) const
{
	assert(K.isSquare());
	K.setSymmetric(true);
	K.syrk(X, mhps[0].second, 0.0, "u", "n");
}
void LinearKernel::computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2) const
{
	assert(K.rowsMatch(X));
	assert(K.sizeCol()==X2.sizeRow());
	K.gemm(X, X2, mhps[0].second, 0.0, "n", "t");
} 
void LinearKernel::computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2, size_t row) const
{
	assert(K.rowsMatch(X));
	assert(K.sizeCol() == 1);
	K.gemvRowRow(0, X, X2, row, mhps[0].second, 0.0, "n");
}
double LinearKernel::getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & covGrd)const
{
	assert(index == 0);
	assert(X.sizeRow() == covGrd.sizeRow());
	assert(X2.sizeRow() == covGrd.sizeCol());
 	 
	double g1 = 0.0;
 
	for(size_t i = 0; i < X.sizeRow(); i++)
		for(size_t j = 0; j < X2.sizeRow(); j++)
    {
      double dot = X.dotRowRow(i, X2, j);
      g1 += dot * covGrd.get(i, j);
    }
	return g1;
}
double LinearKernel::getGradientParam(size_t index, const MMatrix &X, const MMatrix & cvGrd) const
{
	assert(index == 0);
	assert(X.rowsMatch(cvGrd));
	assert(cvGrd.isSquare());
  	 
	double g1 = 0.0;
	for(size_t i=0; i < X.sizeRow(); i++)
	{
		for(size_t j = 0; j <X.sizeRow(); j++)
		{	
			double dot = X.dotRowRow(i, X, j);
			g1 += dot*cvGrd.get(i, j);
		}
	}
	return g1;
}
