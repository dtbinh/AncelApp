#include "RBFKernel.h"
#include <assert.h>
using namespace ResCore;

//
//	RBF Kernel a * exp(-r/2*(x-x')T*(x-x'))
//
RBFKernel::RBFKernel() 
{
	 initKernel();
}

RBFKernel::RBFKernel(const RBFKernel& kern) : Kernel(kern)
{
	initKernel();
	mhps =  kern.mhps;
}
// Class destructor
RBFKernel::~RBFKernel()
{
}

void RBFKernel::initKernel()
{
	mKernelType = "RBF";
	setName("Kernel_RBF");
 	mhps.push_back(std::make_pair("InverseWidth",1));
	mhps.push_back(std::make_pair("Variance",1));

 	addTransform(CTransform::DefaultPositive(),0);
	addTransform(CTransform::DefaultPositive(),1);
}
void  RBFKernel::getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG) const
{
	assert(gX.size() == X.sizeCol());
	
	double halfInvWid = mhps[0].second * 0.5;
	double mulVarInvMid = mhps[1].second * mhps[0].second;
	double n2(0),Kxx(0),grad(0);
	
	for(size_t i = 0; i < X.sizeRow(); i++)
	{
		for(size_t j = 0; j < i; j++)
		{
 			n2 = X.rowDist2(i, X, j);
			Kxx = exp(-n2 * halfInvWid);
 
			for(size_t t = 0; t < X.sizeCol(); t++)
			{
 				grad = mulVarInvMid * (X.get(j,t) - X.get(i,t)) * Kxx;
				if(addG)
				{
	 				gX[t]->add( grad,i,j);
					gX[t]->add(-grad,j,i);
 				}
				else
				{
	 				gX[t]->assign( grad,i,j);
					gX[t]->assign(-grad,j,i);
 				}
 			}
		}

		if (!addG)
		{
			for (size_t t = 0; t < X.sizeCol(); t++)
	  			gX[t]->assign(0,i,i);
		}
 	}
}
void  RBFKernel::getGradientX(MMatrix &g, const MMatrix& X, size_t row, bool addG) const
{
	assert(g.rowsMatch(X));
	
	double halfInvWid = mhps[0].second * 0.5;
	double mulVarInvMid = mhps[1].second * mhps[0].second;

	for(size_t k = 0; k < X.sizeRow(); k++)
	{
		if(k == row)
		{
			for(size_t t = 0; t < X.sizeCol(); t++)
			{
				if(!addG) g.assign(0.0,k,t);
			}
		}
		else 
		{
			double n2 = X.rowDist2(row, X, k);
			double Kxx = exp(-n2 * halfInvWid);

			for(size_t t = 0; t < X.sizeCol(); t++)
			{
				double Grad = mulVarInvMid * (X.get(k,t) - X.get(row,t)) * Kxx;
				if (addG)
						g.add(Grad,k,t);
				else 
						g.assign(Grad,k,t);
			}
		}
	} 
}
//-r(x-x')k(x,x')  dK/dX
void RBFKernel::getGradientX(MMatrix& g, const MMatrix& X, size_t uRow, const MMatrix& X2, bool addG) const
{
	assert(g.rowsMatch(X2) && X.colsMatch(X2));
	
	double halfInvWid = mhps[0].second * 0.5;
	double mulVarInvMid = mhps[1].second * mhps[0].second;

	for(size_t k = 0; k < X2.sizeRow(); k++)
	{
		double n2 = X.rowDist2(uRow, X2, k);
		double Kxx = exp(-n2 * halfInvWid);

		for(size_t t = 0; t < X2.sizeCol(); t++)
		{
			double Grad = mulVarInvMid * (X2.get(k,t) - X.get(uRow,t)) * Kxx;
			if(addG)
				g.add(Grad, k, t);
			else 
				g.assign(Grad, k, t);
		}
	} 
}
void RBFKernel::getDiagGradientX(MMatrix& g, const MMatrix& X, bool addG) const
{
	assert(g.rowsMatch(X));
 	if(!addG)
			g.zeroElements();
}

double RBFKernel::computeElement(const MMatrix& X1, size_t index1, const MMatrix& X2, size_t index2) const
{
	double Dist2 = X1.rowDist2(index1, X2, index2);
	//std::cout << Dist2 << std::endl;
	return mhps[1].second * exp(-0.5 * Dist2 * mhps[0].second);
}
 
void RBFKernel::getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& covGrad, bool regularise) const
{
	assert(g.sizeRow() == 1 && g.sizeCol() == mhps.size() && covGrad.isSymmetric());
	
	double g1(0.0),g2(0.0);
	double halfInvWid = 0.5 * mhps[0].second;
	double halfVariance = 0.5 * mhps[1].second;
	size_t uRows = X.sizeRow();

	for(size_t j = 0; j < uRows; j++)
	{
		g2 += covGrad.get(j,j);
		for(size_t i = 0; i < j; i++)
		{
 			double dist2 =  X.rowDist2(i,X,j);
			double k = exp(-dist2*halfInvWid);
 
			g1 -=  2.0 * k * halfVariance * dist2 * covGrad.get(i, j);  // for k is a symmetrical matrix
			g2 +=  2.0 * k * covGrad.get(i, j);
		}
	}
	g.assign(g1, 0);
	g.assign(g2, 1);
	//if(regularise)
	//	AddPriorGrad(g);
}
// d k(x,x')/d alpha(m_Variance)
// d k(x,x')/d gamma(m_InverseWidth)
void RBFKernel::getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& X2,const MMatrix& covGrad,bool regularise) const
{
	assert(g.sizeRow() == 1);
	assert(g.sizeCol() == mhps.size());
	assert(X.sizeRow() == covGrad.sizeRow());
	assert(X2.sizeRow() == covGrad.sizeCol());

	double g1 = 0.0;
	double g2 = 0.0;
	double halfInvWid = 0.5 * mhps[0].second;
	double halfVariance = 0.5 * mhps[1].second;
	size_t nRows = X.sizeRow();
	
	for(size_t j = 0; j < nRows; j++)
	{
		for(size_t i  = 0; i < X2.sizeRow(); i++)
		{
 			double dist2 = X2.rowDist2(i,X,j);
			double k = exp(-dist2*halfInvWid);

			g1 -=  k*halfVariance*dist2*covGrad.get(i, j);
			g2 +=  k*covGrad.get(i,j);
		}
 	}
	g.assign(g1,0);
	g.assign(g2,1);
	/*if(regularise)
		AddPriorGrad(g);*/
}