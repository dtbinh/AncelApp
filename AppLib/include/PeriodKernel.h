#ifndef __PeriodKernel_
#define __PeriodKernel_
#include "Kernel.h"

namespace ResCore
{
	//simplifid version of period kernel 

	class PeriodKernel: public Kernel
	{
	public:
		PeriodKernel();
	 	PeriodKernel(const PeriodKernel& Kern);
		~PeriodKernel();
		virtual PeriodKernel* Clone() const;
 		double computeDiagElement(const MMatrix & X,size_t index) const;
		void   computeDiag(MMatrix &d, const MMatrix &X) const;
	 
		void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG = false) const;
	 	void   getGradientX(MMatrix &g, const MMatrix& X, size_t ptNum,const MMatrix&X2,bool addG = false) const;
		void   getGradientX(MMatrix& g, const MMatrix& X, size_t row, bool addG = false) const;
		void   getDiagGradientX(MMatrix &g, const MMatrix &X,bool addG = false) const;
  
		double getWhiteNoise() const {return 0;}
		double computeElement(const MMatrix &X1,size_t Index1,const MMatrix &X2,size_t Index2) const;
//		void   computeKernel(MMatrix &K, const MMatrix &X) const;
//		void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2) const;
//		void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2, size_t row) const;

		void   getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& covGrad, bool regularise = true) const;
		void   getGradientParams(MMatrix& g, const MMatrix& X, const MMatrix& X2, const MMatrix& covGrad, bool regularise = true) const;

		double getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & cvGrd) const {return 0;}
		double getGradientParam(size_t index, const MMatrix &X, const MMatrix & cvGrd) const {return 0;}
		
		void   WriteParamsToLog() {};

	protected:
 		virtual void initKernel();
 	};
	//----------------------------------------------inline function-------------------------------------------
	inline PeriodKernel::PeriodKernel() 
	{
		initKernel();
 	}
	inline PeriodKernel* PeriodKernel::Clone() const
	{
		return new PeriodKernel(*this);
	}
	inline PeriodKernel::PeriodKernel(const PeriodKernel& Kern)
	{
		initKernel();
 		mhps = Kern.mhps;
	}
	inline PeriodKernel::~PeriodKernel()
	{
	}
  
	inline double PeriodKernel::computeDiagElement(const MMatrix & X,size_t index) const
	{
		return mhps[0].second;
	}
	inline void PeriodKernel::computeDiag(MMatrix &d, const MMatrix &X) const
	{
		d.assign(mhps[0].second);
	}
 	

}
#endif