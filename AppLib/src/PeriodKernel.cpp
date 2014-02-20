#include "PeriodKernel.h"

using namespace ResCore;

void PeriodKernel::initKernel()
{
	mKernelType = "PeriodKernel";
	setName("Kernel_Period");
 	mhps.push_back(std::make_pair("theta",1));
	mhps.push_back(std::make_pair("gamma",1e-3));
	mhps.push_back(std::make_pair("lambda",3.141592653589/5));

 	addTransform(CTransform::DefaultPositive(),0);
	addTransform(CTransform::DefaultPositive(),1);
	addTransform(CTransform::DefaultPositive(),2);
}

void  PeriodKernel::getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG) const
{
	assert(gX.size() == X.sizeCol());
  	
	for(size_t i = 0; i < X.sizeRow(); i++)
	{
		for(size_t j = 0; j < i; j++)
		{
			double val = 0.0;
			for (std::size_t t = 0; t < X.sizeCol(); t++)
			{
				double theta = mhps[2].second * (X.get(i, t) - X.get(j, t));
				double dis = sin(theta);
 				val += dis*dis;
			}
 
			double kxx = mhps[0].second * exp(-0.5 * mhps[1].second * val);

			for(size_t t = 0; t < X.sizeCol(); t++)
			{
				double tempval = mhps[2].second * (X.get(i, t) - X.get(j, t));
				double grad = - kxx * mhps[1].second * sin(tempval) * cos(tempval) * mhps[2].second;
				if(addG)
				{
					gX[t]->add(grad, i, j);
					gX[t]->add(-grad, j, i);
				}
				else 
				{
					gX[t]->assign( grad, i, j);
					gX[t]->assign(-grad, j, i);
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

void  PeriodKernel::getGradientX(MMatrix& g, const MMatrix& X, size_t row, bool addG) const
{
	assert(g.rowsMatch(X));
 	 
	for(size_t k = 0; k < X.sizeRow(); k++)
	{
		double val = 0.0;
		for (std::size_t t = 0; t < X.sizeCol(); t++)
		{
			double theta = mhps[2].second * (X.get(row, t) - X.get(k, t));
			double dis = sin(theta);
 			val += dis*dis;
		}
		
		double kxx = mhps[0].second * exp(-0.5 * mhps[1].second * val);

  		for(size_t t = 0; t < X.sizeCol(); t++)
		{
			double tempval = mhps[2].second * (X.get(row, t) - X.get(k, t));
			double Grad = - kxx * mhps[1].second * sin(tempval) * cos(tempval) * mhps[2].second;
			if(addG)
				g.add(Grad, k, t);
			else 
				g.assign(Grad, k, t);
		}
	} 
}

void  PeriodKernel::getGradientX(MMatrix& g, const MMatrix& X, size_t uRow, const MMatrix& X2, bool addG) const
{
	assert(g.rowsMatch(X2) && X.colsMatch(X2));
 	 
	for(size_t k = 0; k < X2.sizeRow(); k++)
	{
		double val = 0.0;
		for (std::size_t t = 0; t < X.sizeCol(); t++)
		{
			double theta = mhps[2].second * (X.get(uRow, t) - X2.get(k, t));
			double dis = sin(theta);
 			val += dis*dis;
		}
		
		double kxx = mhps[0].second * exp(-0.5 * mhps[1].second * val);

  		for(size_t t = 0; t < X2.sizeCol(); t++)
		{
			double tempval = mhps[2].second * (X.get(uRow, t) - X2.get(k, t));
			double Grad = - kxx * mhps[1].second * sin(tempval) * cos(tempval) * mhps[2].second;
			if(addG)
				g.add(Grad, k, t);
			else 
				g.assign(Grad, k, t);
		}
	} 

}
		
void  PeriodKernel::getDiagGradientX(MMatrix& g, const MMatrix& X, bool addG) const
{
	if(!addG)
		g.zeroElements();
}

void PeriodKernel::getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& covGrad, bool regularise) const
{
 	double gTheta = 0,gGamma = 0,gLambda = 0;
	
	for (std::size_t i = 0; i < X.sizeRow(); i++)
	{
		gTheta += covGrad.get(i, i);
 		
		for (std::size_t j = 0; j < i; j++)
		{
			double val = 0.0, tempVal = 0.0;			// chian gradient for ladmbda
			
			for (std::size_t t = 0; t < X.sizeCol(); t++)
			{
				double theta = mhps[2].second * (X.get(i, t) - X.get(j, t));
			
				double dis = sin(theta);
				
				tempVal += 2 * dis * cos(theta) * (X.get(i, t) - X.get(j, t));
				
				val += dis*dis;
			}
			
			double exp_val = exp(-0.5 * mhps[1].second * val);

			gTheta  += 2 * exp_val * covGrad.get(i, j);
			gGamma  -= mhps[0].second * exp_val * val * covGrad.get(i, j);
 			gLambda -= mhps[0].second * exp_val * mhps[1].second * tempVal * covGrad.get(i, j);
		}
	}

	g.assign(gTheta,0);
	g.assign(gGamma,1);
	g.assign(gLambda,2);
}
void PeriodKernel::getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& X2, const MMatrix& covGrad, bool regularise) const
{
	assert(X.sizeCol() == X2.sizeCol());

	double gTheta = 0,gGamma = 0,gLambda = 0;
	for (std::size_t i = 0; i < X.sizeRow(); i++)
	{
		for (std::size_t j = 0; j < X2.sizeRow(); j++)
		{
			double val = 0.0, tempVal = 0.0;			// chian gradient for ladmbda

			for (std::size_t t = 0; t < X.sizeCol(); t++)
			{
				double theta = mhps[2].second * (X.get(i, t) - X2.get(j, t));
				double dis = sin(theta);
				
				tempVal += 2 * dis * cos(theta) * (X.get(i, t) - X2.get(j, t));
				
				val += dis*dis;
			}
			gTheta  += exp(-0.5 * mhps[1].second * val) * covGrad.get(i, j);
			gGamma  -= mhps[0].second * exp(-0.5 * mhps[1].second * val) * 0.5 * val * covGrad.get(i, j);
 			gLambda -= mhps[0].second * exp(-0.5 * mhps[1].second * val) * 0.5 * mhps[1].second * tempVal * covGrad.get(i, j);
		}
	}

	g.assign(gTheta,0);
	g.assign(gGamma,1);
	g.assign(gLambda,2);
}

double PeriodKernel::computeElement(const MMatrix& X1, size_t index1, const MMatrix& X2, size_t index2) const
{
	double val = 0.0;
	for(std::size_t i = 0; i < X1.sizeCol(); i++)
	{
		double dis = sin(mhps[2].second*(X1.get(index1, i) - X2.get(index2, i)));
		val += dis*dis;
	}
	return mhps[0].second * exp(-0.5 * mhps[1].second * val);
}
