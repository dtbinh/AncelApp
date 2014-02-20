#ifndef GAUSSIANDISTRIBUTION_H
#define GAUSSIANDISTRIBUTION_H

#include "Distribution.h"

namespace ResCore
{
	class CGaussianDist :public Distribution
	{
	public:
		CGaussianDist();
		CGaussianDist(const CGaussianDist&); 
		
		~CGaussianDist();
   
		CGaussianDist* Clone() const;
	 
		double getGradInput(double x) const;
 		double logProb(double val) const;
	protected:
		void initDist();
	private:
 		double mPrecision;
	};
};
#endif