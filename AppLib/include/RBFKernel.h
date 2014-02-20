#ifndef RBFKERNEL_H
#define RBFKERNEL_H

#include <assert.h>
#include "Kernel.h"
 
namespace ResCore
{
	class RBFKernel : public Kernel
	{
	public:
		RBFKernel();
 	 	RBFKernel(const RBFKernel&);
		~RBFKernel();

		RBFKernel* Clone() const;
  		
	 	double getLengthscale()  const;
		double getInverseWidth() const;
		
		void   setVariance(double Variance);
		void   setLengthscale(double val);
		void   setInverseWidth(double InverseWidth);
		
		void   setInitParams();
	
		void   computeDiag(MMatrix& d, const MMatrix& X) const;
		double computeDiagElement(const MMatrix& X, size_t index) const;

		void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG = false) const;

		void   getGradientX(MMatrix& g, const MMatrix& X, size_t row, bool addG = false) const;
 		void   getGradientX(MMatrix& g, const MMatrix& X, size_t uRow, const MMatrix& X2, bool addG = false) const;
		
		void   getDiagGradientX(MMatrix& g, const MMatrix& X, bool addG = false) const;
		
		double getWhiteNoise() const {return 0;}
		
		double computeElement(const MMatrix& X1, size_t Index1, const MMatrix& X2, size_t Index2) const;
	
		void   getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& covGrad, bool regularise = true) const;
		void   getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& X2, const MMatrix& covGrad, bool regularise = true) const;
	
		double getGradientParam(size_t index, const MMatrix& X, const MMatrix& X2, const MMatrix& covGrd) const { return 0;}
		double getGradientParam(size_t index, const MMatrix& X, const MMatrix& covGrd) const {return 0;};
	
	protected:
		void initKernel();
  	};
	//-------------------------------------------inline functions---------------------------------------
	inline RBFKernel* RBFKernel::Clone() const
	{
		return new RBFKernel(*this);
	}
	inline double RBFKernel::computeDiagElement(const MMatrix & X,size_t index) const
	{
		return mhps[1].second;
	}
	inline void RBFKernel::computeDiag(MMatrix &d, const MMatrix &X) const // e^0 = 1;
	{
		assert(d.sizeCol() == 1 && X.rowsMatch(d));
		d.assign(mhps[1].second);
	}
};

#endif

