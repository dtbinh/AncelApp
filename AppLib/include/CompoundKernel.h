#ifndef COMPOUNDKERNEL_H
#define COMPOUNDKERNEL_H

#include "Kernel.h"

namespace ResCore
{
	class CompoundKernel: public Kernel
	{
	public:
		CompoundKernel();
		~CompoundKernel();

		CompoundKernel(const CompoundKernel&);
		CompoundKernel* Clone() const;
		
		size_t getNumParams() const;
		
		virtual size_t addKernel(const Kernel * kern);

		virtual void   setParam(double val, size_t index);
		virtual double getParam(size_t index) const;

		virtual std::string getParamName(size_t index) const;
	 
		virtual void AddPrior(Distribution* prior, size_t index);

		virtual double PriorLogProb() const;

		virtual size_t GetKernsNum() const;
 
	 	virtual double getWhiteNoise()   const;

	 
		virtual double computeDiagElement(const MMatrix & X,size_t index) const;
		virtual void   computeDiag(MMatrix &d, const MMatrix &X) const;

		virtual void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG = false) const;
		virtual void   getGradientX(MMatrix &g, const MMatrix& X, size_t uRow, const MMatrix&X2,bool addG = false) const;
		
		virtual void   getDiagGradientX(MMatrix &g, const MMatrix &X,bool addG = false) const;
	
		virtual double computeElement(const MMatrix &X1,size_t uIndex1,const MMatrix &X2,size_t uIndex2) const;

		virtual void   getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &covGrad,bool regularise = true) const;
		virtual void   getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &X2,const MMatrix &covGrad,bool regularise = true) const;

		virtual double getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & cvGrd) const ;
		virtual double getGradientParam(size_t index, const MMatrix &X, const MMatrix & cvGrd) const;

		virtual void SetLog(std::ostream *pLog);
		virtual void WriteParamsToLog();
	protected:
		void initKernel();
	private:
		size_t m_ParamsNum;
		std::vector<Kernel*> m_Components;
  	};

	inline size_t CompoundKernel::getNumParams() const
	{
		return m_ParamsNum;
	}
};

#endif