#ifndef KERNEL_H
#define KERNEL_H

#include <string>
#include <vector>
#include <iostream>

#include "MMatrix.h"
#include "Transformable.h"
#include "Regularisable.h"
 
namespace ResCore
{
	using namespace ResUtil;
	 
 	class Kernel : public Transformable //, public Regularisable
	{
	public:
		Kernel();
	 	Kernel(const Kernel &kern); 
 	 	 
 		std::string getType()     const;
		std::string getName()	  const;
	 
 	 	virtual size_t getNumParams() const;

		void setName(const std::string name);
	 
		void   setParams(const MMatrix &parmas);
		void   getParams(MMatrix &parmas) const;
		void   setParam(double paramVal,size_t index);
		double getParam(unsigned int index) const;

 		virtual std::string getParamName(size_t index)   const;
  
		virtual ~Kernel(); 
		virtual Kernel* Clone() const = 0;
	  		
		virtual double computeDiagElement(const MMatrix & X,size_t index) const = 0;
		virtual void   computeDiag(MMatrix &d, const MMatrix &X) const;
 	 
		virtual void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG = false) const = 0;
		virtual void   getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, const MMatrix& X2, bool addG = false)   const;
		virtual void   getGradientX(MMatrix& g, const MMatrix& X, size_t uRow,const MMatrix& X2, bool addG = false) const = 0;
		virtual void   getDiagGradientX(MMatrix& g, const MMatrix& X,bool addG = false) const = 0;
		virtual void   getGradientX(MMatrix& g, const MMatrix& X, size_t row, bool addG = false){};
		virtual double getWhiteNoise() const;
 
		virtual double computeElement(const MMatrix &X1,size_t uIndex1,const MMatrix &X2,size_t uIndex2) const = 0;
 	 		
		virtual void   computeKernel(MMatrix &K, const MMatrix &X) const;
		virtual void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2) const;
		virtual void   computeKernel(MMatrix &K, const MMatrix &X, const MMatrix &X2, size_t row) const;
		
		virtual void   getGradientParams(MMatrix &g) const;
		virtual void   getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &covGrad,bool regularise = true) const;
		virtual void   getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &X2,const MMatrix &covGrad,bool regularise = true) const;
		
		virtual void   getDiagGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &covGrad,bool regularise = true) const;
		
		virtual double getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & cvGrd) const = 0;
		virtual double getGradientParam(size_t index, const MMatrix &X, const MMatrix & cvGrd) const = 0;
	 	
		//covGrad = d(L)/dk
		void getGradTransParams(MMatrix &g, const MMatrix &X, const MMatrix &covGrd, bool regularise = true)    const;
		void getGradTransParams(MMatrix &g, const MMatrix &X, const MMatrix &X2, const MMatrix &covGrd, bool regularise = true) const;
 		void getDiagGradTransParams(MMatrix&g, const MMatrix &X, const MMatrix &cvGrd, bool regularise = true) const;
		bool Equals(const Kernel& kern, double tol) const;

		void WriteToLog(std::string strContent) const;
		virtual void SetLog(std::ostream *pLog);
		virtual void WriteParamsToLog();
	protected:
		
		virtual void   initKernel() = 0;
 	protected:
 	 
		std::string mKernelName;
		std::string mKernelType;

		typedef std::pair<std::string,double> PARAMS;
 		std::vector<PARAMS> mhps;  //hyper paramaters

		std::ostream *m_pLog;
  	};
	//------------------------------------------inline function -------------------------------------------
	inline Kernel::Kernel()
 	{
  	}
 	inline Kernel::Kernel(const Kernel &kern)
	 
	{
 	}
	inline Kernel::~Kernel() 
	{
	}
    inline double Kernel::getWhiteNoise() const
	{
		return 0;
	}
	inline void Kernel::getGradientParams(MMatrix &g) const
	{
	}
 	inline void Kernel::setName(const std::string name)
	{
		mKernelName = name;
	}
	inline std::string Kernel::getType() const
	{
		return mKernelType;
	}
	inline std::string Kernel::getName() const
	{
		return mKernelName;
	}
	inline size_t addKernel(const Kernel* kern)
	{ 
		return 0;
	}
    
	inline size_t Kernel::getNumParams() const
	{
		return mhps.size();
	}
	 
	inline std::string Kernel::getParamName(size_t index) const
	{
		return mhps[index].first;
	}
	inline void Kernel::WriteToLog(std::string strContent) const
	{
		(*m_pLog) << strContent << std::endl;
	}
 	inline void Kernel::SetLog(std::ostream *pLog)
	{
		m_pLog = pLog;
	}
	inline void Kernel::WriteParamsToLog()
	{
		for(std::size_t i = 0; i < mhps.size(); i++)
			(*m_pLog) << mhps[i].first << " " << mhps[i].second << std::endl;
	}
	inline void   Kernel::setParam(double paramVal,size_t index)
	{
		assert(index < mhps.size());
		mhps[index].second = paramVal;
	}
	inline double Kernel::getParam(unsigned int index) const
	{
		assert(index < mhps.size());
		return mhps[index].second;;
	}

};
#endif