#ifndef BIASKERNEL_H
#define BIASKERNEL_H

#include "Kernel.h"
#include <assert.h>
namespace ResCore
{
	class BiasKernel: public Kernel
	{
	public:
		BiasKernel();
	 	BiasKernel(const BiasKernel& Kern);
		~BiasKernel();
		
		BiasKernel* Clone() const;

 		double computeDiagElement(const MMatrix & X,size_t index) const;
		void   computeDiag(MMatrix &d, const MMatrix &X) const;
	 
		void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG = false) const {};
	 	void   getGradientX(MMatrix &g, const MMatrix& X, size_t ptNum,const MMatrix&X2,bool addG = false) const;
		void   getDiagGradientX(MMatrix &g, const MMatrix &X,bool addG = false) const;
  
		double getWhiteNoise() const;
		double computeElement(const MMatrix &X1,size_t Index1,const MMatrix &X2,size_t Index2) const;
 		void   computeKernel(MMatrix &K, const MMatrix &X) const;
		void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2) const;
		void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2, size_t row) const;

		double getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & cvGrd)const;
		double getGradientParam(size_t index, const MMatrix &X, const MMatrix & cvGrd) const;
		
		void   WriteParamsToLog();

	protected:
 		virtual void initKernel();
 	};
	//----------------------------------------------inline function-------------------------------------------
	inline BiasKernel::BiasKernel() 
	{
		initKernel();
 	}
	
	inline BiasKernel::BiasKernel(const BiasKernel& Kern)
	{
		initKernel();
 		mhps = Kern.mhps;
	}
	inline BiasKernel::~BiasKernel()
	{
	}

	 
	inline BiasKernel* BiasKernel::Clone() const
	{
		return new BiasKernel(*this);
	}
	inline double BiasKernel::computeDiagElement(const MMatrix & X,size_t index) const
	{
		return mhps[0].second;
	}
	inline void BiasKernel::computeDiag(MMatrix &d, const MMatrix &X) const
	{
		d.assign(mhps[0].second);
	}
	
	inline double BiasKernel::getWhiteNoise() const
	{
		return 0.0;
	}
	
   	inline double BiasKernel::computeElement(const MMatrix &X1,size_t Index1,const MMatrix &X2,size_t Index2) const
	{
		return mhps[0].second;
	}
	inline void BiasKernel::WriteParamsToLog()
	{
		*m_pLog << "BiasKernel:" << std::endl;
		*m_pLog << "m_Variance  : " << mhps[0].second << std::endl;
	}
};


#endif