#ifndef LINEARKERNEL_H
#define LINEARKERNEL_H
#include "Kernel.h"
#include <assert.h>

namespace ResCore
{
	// Linear Kernel k(X,X') = alpha * X*X'
	class LinearKernel: public Kernel
	{
	public:
		LinearKernel();
		~LinearKernel();
		LinearKernel(const LinearKernel&);
		LinearKernel* Clone() const;
		
	  
 	 	double computeDiagElement(const MMatrix &X,size_t index) const;
		
		void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG = false) const;
		void   getGradientX(MMatrix& g, const MMatrix& X, size_t uRow, const MMatrix& X2, bool addG = false) const;
		void   getDiagGradientX(MMatrix& g, const MMatrix& X, bool addG = false) const;
		double getWhiteNoise() const;

	 	void   computeKernel(MMatrix& K, const MMatrix& X) const;
		void   computeKernel(MMatrix& K, const MMatrix& X, const MMatrix& X2) const;
		void   computeKernel(MMatrix& K, const MMatrix& X, const MMatrix& X2, size_t row) const;
		double computeElement(const MMatrix& X1, size_t Index1, const MMatrix& X2, size_t Index2) const;
		
		double getGradientParam(size_t index, const MMatrix& X, const MMatrix& X2, const MMatrix& cvGrd)const;
		double getGradientParam(size_t index, const MMatrix& X, const MMatrix& cvGrd) const;

 	protected:
		void initKernel();
 	};
	//---------------------------------------------inline function-------------------------------------------
	inline LinearKernel::LinearKernel()
 	{
		initKernel();
	}

	inline LinearKernel::~LinearKernel()
	{

	}
	inline LinearKernel::LinearKernel(const LinearKernel& kern)
	{
		initKernel();
	 	mhps = kern.mhps;
 	}
	inline LinearKernel* LinearKernel::Clone() const
	{
		return new LinearKernel(*this);
	}
 	inline double LinearKernel::computeDiagElement(const MMatrix & X,size_t index) const
	{
		return mhps[0].second * X.rowNorm2(index);
	}
	
	inline double LinearKernel::getWhiteNoise() const
	{
		return 0.0;
	}
};


#endif