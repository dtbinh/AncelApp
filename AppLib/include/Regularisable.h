#ifndef REGULARISABLE_H
#define REGULARISABLE_H

#include "ParamPriors.h"

namespace ResCore
{
	class Regularisable
	{
	public:
		virtual ~Regularisable() {};
		virtual size_t getNumParams()		        const = 0;
		virtual double getParam(size_t index)       const = 0;
		virtual void   setParam(double val, size_t index) = 0;
		
		virtual void   getGradientParams(MMatrix& g) const = 0;

		virtual void getParams(MMatrix& params) const;
		virtual void setParams(const MMatrix& params);
		virtual void AddPriorGrad(MMatrix& g)   const;
		
		virtual double PriorLogProb() const;

	 	Distribution * GetPrior(size_t index) const;
	
		size_t GetNumPriors() const;
 		std::string GetPriorType (size_t index) const;
		

		void   ClearPriors();
		void   AddPrior(Distribution * dist);
 		double GetPriorGradInput(double val, size_t ind) const;
 	private:
		ParamPriors m_ParamPriors;
	};


//-------------------------------------------inline function ---------------------------------------------------------- 
	inline size_t Regularisable::GetNumPriors() const
    {
		return m_ParamPriors.getDistNum();
    }
	inline Distribution * Regularisable::GetPrior(size_t index) const
    {
		return m_ParamPriors.getDist(index);
    }
	inline std::string Regularisable::GetPriorType(size_t index) const
    {
		return m_ParamPriors.getDistType(index);
    }
	inline double Regularisable::GetPriorGradInput(double val, size_t index) const
    {
      return m_ParamPriors.getDist(index)->getGradInput(val);
    }
	inline void Regularisable::AddPrior(Distribution * dist)
    {
		m_ParamPriors.addDist(dist);
    }
 	inline void Regularisable::ClearPriors()
    {
		m_ParamPriors.clearDist();
    }
//---------------------------------------------------------------------------------------------------------------
};
#endif