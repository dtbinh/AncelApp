#include "Kernel.h"
#include <cassert>
using namespace ResCore;

void Kernel::computeDiag(MMatrix &d, const MMatrix &X) const
{
 	assert(X.rowsMatch(d) && d.sizeCol()== 1);
 	size_t nRows = X.sizeRow(); 
	for(size_t t = 0; t < nRows; t++)
	 	 d.assign(computeDiagElement(X,t),t);  
}
 
void Kernel::getGradientX(std::vector<MMatrix*>&gX, const MMatrix &X,const MMatrix &X2,bool addG) const
{
	for(size_t t = 0; t < X.sizeRow(); t++)
		getGradientX(*gX[t],X,t,X2,addG);
}
void Kernel::computeKernel(MMatrix &K, const MMatrix &X) const
{
	assert(K.rowsMatch(X) && K.isSquare());

	for(size_t i = 0; i < K.sizeRow(); i++)
	{
		for(size_t j = 0; j < i; j++)
		{
			double k = computeElement(X, i, X, j);
	 		K.assign(k,i,j);
			K.assign(k,j,i);
		}
		K.assign(computeDiagElement(X,i), i, i);
	}
	
}
void Kernel::computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2) const
{
	assert(K.rowsMatch(X) && K.sizeCol() == X2.sizeRow());
 	
	for(size_t i = 0; i < K.sizeRow(); i++)
	{
		for(size_t j = 0; j < K.sizeCol(); j++)
		{
			K.assign(computeElement(X, i, X2, j), i, j);
		}
	}
}
void Kernel::computeKernel(MMatrix& K, const MMatrix& X, const MMatrix& X2, size_t row) const
{
	assert(K.rowsMatch(X) && K.sizeCol() == 1);
 	
	for(size_t i = 0; i < K.sizeRow(); i++)
	{
 		K.assign(computeElement(X,i,X2,row),i, 0);
	}
}

void Kernel::getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &cvGrad, bool regularise) const
{
	assert(g.sizeRow() == 1 && g.sizeCol() == mhps.size());
	 
	for(size_t t = 0; t < mhps.size(); t++)
	{
		g.assign(getGradientParam(t,X,cvGrad),t);
	}
	if(regularise)
	{
	//	AddPriorGrad(g);
	}
}
void Kernel::getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &X2,const MMatrix &cvGrad,bool regularise) const
{
	assert(g.sizeRow() == 1 && g.sizeCol() == mhps.size());

 	for(size_t t = 0; t < mhps.size(); t++)
	{
		g.assign(getGradientParam(t,X,X2,cvGrad),t);
	}
	if(regularise)
	{
	//	AddPriorGrad(g);
	}
}
void  Kernel::getDiagGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &covGrad,bool regularise) const
{
 	MMatrix xi(1, X.sizeCol());
    MMatrix cvGradi(1, 1);
    MMatrix gtemp(1, g.sizeCol());
	g.zeroElements();
	for(size_t i = 0; i < X.sizeRow(); i++)
    {
      xi.copyRowRow(0, X, i);
      cvGradi.copyRowRow(0, covGrad, i);
      cvGradi.setSymmetric(true);
      getGradientParams(gtemp, xi, cvGradi, regularise);
      g.axpy(gtemp, 1.0);
    }
}

void Kernel::setParams(const MMatrix &parmas)
{
	for(size_t i = 0; i < mhps.size(); i++)
	{
		setParam(parmas.get(i),i);
	}
}

void Kernel::getParams(MMatrix &parmas) const
{
	for(size_t i = 0; i < mhps.size(); i++)
	{
		parmas.assign(getParam(i),i);
 	}
}

void Kernel::getGradTransParams(MMatrix &g,const MMatrix &X,const MMatrix& X2, const MMatrix& cvGrd, bool regularise) const
{
	assert(g.sizeRow()==1);
	assert(g.sizeCol()== mhps.size());
 
	getGradientParams(g, X, X2, cvGrd, regularise);
 
	for(size_t i = 0; i < getNumTransform(); i++)
	{
		double val = g.get(i);
		double param = getParam(i);

		g.assign(val * getGradientTransform(param, i),i);
	}
}
void Kernel::getGradTransParams(MMatrix& g, const MMatrix& X, const MMatrix& cvGrd, bool regularise) const
{
	assert(g.sizeRow()==1);
	assert(g.sizeCol()== mhps.size());
 
	getGradientParams(g, X,cvGrd, regularise);
   
	for(size_t i = 0; i < getNumTransform(); i++)
	{
		double val = g.get(i);
 		double param = getParam(i);
 		g.assign(val * getGradientTransform(param, i),i);
	}
}
void Kernel::getDiagGradTransParams(MMatrix& g, const MMatrix& X, const MMatrix& cvGrd, bool regularise) const
{
	assert(g.sizeRow()==1);
	assert(g.sizeCol()== mhps.size());
 
	getDiagGradientParams(g, X,cvGrd, regularise);
 
	for(size_t i = 0; i < getNumTransform(); i++)
	{
		double val = g.get(i);
		double param = getParam(i);

		g.assign(val * getGradientTransform(param, i),i);
	}
}
bool Kernel::Equals(const Kernel& kern, double tol) const
{
	if(getType()!= kern.getType())
		return false;
	if(mhps.size() != kern.getNumParams())
	    return false;
	
	MMatrix thisParams(1,mhps.size());
	MMatrix kernParams(1,mhps.size());

	getParams(thisParams);
 	kern.getParams(kernParams);

	if(!thisParams.Equals(kernParams, tol))
		return false;
	return true;
}
