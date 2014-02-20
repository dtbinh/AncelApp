#include "Regularisable.h"

using namespace ResCore;

void Regularisable::getParams(MMatrix& params) const
{
	for(size_t i = 0; i < params.sizeCol(); i++)
		params.assign(getParam(i), i);
}
void Regularisable::setParams(const MMatrix& params)
{
	for(size_t i = 0; i < params.sizeCol(); i++)
		setParam(params.get(i), i);
}
void Regularisable::AddPriorGrad(MMatrix& g) const
{
 	for(size_t i = 0; i < m_ParamPriors.getDistNum(); i++)
	{
		double param = getParam(i);
		g.add(GetPriorGradInput(param, i),i);
	}  
}
double Regularisable::PriorLogProb() const
{
	double L = 0.0;
	double param=0.0;
	for(size_t i = 0; i < m_ParamPriors.getDistNum(); i++)
	{
		param = getParam(i);
		L += m_ParamPriors.getDist(i)->logProb(param);
	}
	return L;
}