#include "Distribution.h"

using namespace ResCore;


void  Distribution::getGradInput(MMatrix& g, const MMatrix& x)
{
	if(g.sizeCol() == x.sizeCol() && g.sizeRow() == x.sizeRow())
	{
		size_t tnRows = g.sizeRow();
		size_t tnCols = g.sizeCol();
		for(size_t i = 0; i < tnRows; ++i)
			for(size_t j = 0; j < tnCols; ++j)
			{
				g.assign(getGradInput(x.get(i,j)),i,j);
 			}
	}
}

double Distribution::logProb(const MMatrix& x) const
{
	double tSumLog = 0.0;
	
	size_t uRows = x.sizeRow();
	size_t uCols = x.sizeCol();

	for(size_t i = 0; i < uRows; ++i)
	{
		for(size_t j = 0; j < uCols; ++j)
		{
			tSumLog += logProb(x.get(i,j));
		}
	}
	return tSumLog;
}
