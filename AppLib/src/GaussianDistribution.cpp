#include "GaussianDistribution.h"
#include <cassert>
using namespace ResCore;


CGaussianDist::CGaussianDist()
{
	initDist();
 
}
CGaussianDist::CGaussianDist(const CGaussianDist& dist)
	:mPrecision(dist.mPrecision)
{
	initDist();
}
CGaussianDist::~CGaussianDist()
{

}
void CGaussianDist::initDist()
{
	mDistType = "Gaussian";
	mDistName = "Gaussian Prior";
	mhps.push_back(std::make_pair("precision",1.0));
  	addTransform(new CNegLogLogitTransform, 0);
}
CGaussianDist* CGaussianDist::Clone() const
{
	return new CGaussianDist(*this);
}
 
double CGaussianDist::getGradInput(double x) const
{
	return -mPrecision * x;							// d log(L)/dx;
}
double CGaussianDist::logProb(double val) const
{
	return -0.5 *( mPrecision *val * val + log(2*M_PI) - log(mPrecision));
}
