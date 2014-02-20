#ifndef WHITENOISEKERNEL_H
#define WHITENOISEKERNEL_H

#include "Kernel.h"

namespace ResCore
{
	class WhiteNoiseKernel :public Kernel
	{
	public:
		WhiteNoiseKernel();
 		WhiteNoiseKernel(const MMatrix& X);
		WhiteNoiseKernel(const WhiteNoiseKernel &);
		
		~WhiteNoiseKernel();
		WhiteNoiseKernel* Clone() const;
 
		virtual double computeDiagElement(const MMatrix & X,size_t index) const;
		virtual void   computeDiag(MMatrix &d, const MMatrix &X) const;
		
		virtual void   setParam(double paramVal,size_t index);
		virtual double getParam(unsigned int index) const;
 		
		virtual void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG = false) const {};

		virtual void   getGradientX(MMatrix& g, const MMatrix& X, size_t uRow, const MMatrix& X2, bool addG = false) const;
		virtual void   getDiagGradientX(MMatrix& g, const MMatrix& X,bool addG = false) const;

		virtual double getWhiteNoise() const;

		virtual double computeElement(const MMatrix &X1, size_t uIndex1,const MMatrix &X2, size_t uIndex2) const;
		virtual void   computeKernel(MMatrix &K, const MMatrix &X) const;
		virtual void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2) const;
 		virtual void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2, size_t row) const;


		virtual double getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & cvGrd) const;
		virtual double getGradientParam(size_t index, const MMatrix &X, const MMatrix &cvGrd) const;
 
	protected:
		void initKernel();
   	};
	//--------------------------------------inline function---------------------------------------------------------
	inline WhiteNoiseKernel::WhiteNoiseKernel()
 	{
		initKernel();
 	}
	 
	inline WhiteNoiseKernel::WhiteNoiseKernel(const WhiteNoiseKernel &kern)
	{
		initKernel();
  		mhps = kern.mhps;
 	}
	inline WhiteNoiseKernel::~WhiteNoiseKernel()
	{
	}
	inline WhiteNoiseKernel* WhiteNoiseKernel::Clone() const
	{
		return new WhiteNoiseKernel(*this);
	}
};

#endif