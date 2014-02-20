#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H


#include <cassert>
#include <iostream>
#include <string>

#include "MMatrix.h"
namespace ResCore
{
	enum OPTIMIZER_TYPE
	{
		TYPE_CG, 
		TYPE_SCG, 
		TYPE_GD, 
		TYPE_BFGS,
		TYPE_LBFGS
	};
	
	using namespace ResUtil;

 	class Optimisable
	{
	public:
		Optimisable();
		~Optimisable();

		void SetDefaultOptimiser(OPTIMIZER_TYPE OpType);
		OPTIMIZER_TYPE GetDefaultOptimiser() const;

		virtual size_t getOptiParamsNum()     const = 0;
		virtual void getOptiParams(MMatrix& param) const = 0;
		virtual void setOptiParams(const MMatrix& param) = 0;

		virtual double computeObjectiveVal() = 0;
		virtual double computeObjectiveGradParams(MMatrix & g)  = 0;
	   	
		void	CGOptimise();
		void	SCGOptimise();

		void	SetDirection(const MMatrix& vals);
		void	GetDirection(MMatrix& vals) const;
		void	CheckGradients();  

		void	SetLearnRate(double val);
		double	GetLearnRate() const;

		void	SetMomentum(double val);
		double	GetMomentum() const;

		void	SetFuncEvalTerminate(bool val);
		bool	IsFuncEvalTerminate() const;

		void	SetIterTerminate(bool val);
		bool	IsIterTerminate() const;

		void	SetminFuncEvals(size_t val);
		size_t GetminFuncEvals() const;

		void	SetminIters(size_t val);
		size_t	GetminIters() const;

		void    SetObjectiveTol(double val);
		double  GetObjectiveTol() const;

		void    SetParamTol(double val);
		double  GetParamTol() const;

		void    RunDefaultOptimiser();

		void    WriteToLog(std::string strContent) const;
		void    SetLog(std::ostream *pLog);

	protected:

		OPTIMIZER_TYPE m_DefaultOptimiser;
		bool m_bfnEvalTerminate;
		bool m_bIterTerminate;
	
		double m_Momentum;
		double m_LearnRate;
 	
		double m_ObjectiveTol;
		double m_ParameterTol;
	
		
		size_t m_uIter;
		size_t	m_uFuncVal;		
		size_t m_uminIter;
		size_t	m_uminFuncVal;		

		std::ostream *m_pLog;

		MMatrix m_Direction; // direction for 1-D optimisation.
		MMatrix m_ParamStoreOne;
		MMatrix m_ParamStoreTwo;
	};
	//-------------------------------------inline function------------------------------------------
	inline void Optimisable::SetDefaultOptimiser(OPTIMIZER_TYPE OpType)
	{
		m_DefaultOptimiser = OpType;
	}
	inline	OPTIMIZER_TYPE Optimisable::GetDefaultOptimiser() const
	{
		return m_DefaultOptimiser;
	}

	inline void	Optimisable::SetDirection(const MMatrix& vals)
	{
		 assert(vals.sizeCol() == getOptiParamsNum());
		 assert(vals.sizeRow() == 1);
		 m_Direction = vals;
	}
	inline void	Optimisable::GetDirection(MMatrix& vals) const
	{
		 assert(vals.sizeCol() == getOptiParamsNum());
		 assert(vals.sizeRow() == 1);
		 vals = m_Direction;
	}
	
	inline void	Optimisable::SetLearnRate(double val)
	{
		m_LearnRate = val;
	}
	inline double Optimisable::GetLearnRate() const
	{
		return m_LearnRate;
	}

	inline void	Optimisable::SetMomentum(double val)
	{
		m_Momentum = val;
	}
	inline double Optimisable::GetMomentum() const
	{
		return m_Momentum;
	}

	inline void	Optimisable::SetFuncEvalTerminate(bool val)
	{
		m_bfnEvalTerminate = val;
	}
	inline bool	Optimisable::IsFuncEvalTerminate() const
	{
		return m_bfnEvalTerminate;
	}

	inline void	Optimisable::SetIterTerminate(bool val)
	{
		m_bIterTerminate = val;
	}
	inline bool	Optimisable::IsIterTerminate() const
	{
		return m_bIterTerminate;
	}

	inline void	Optimisable::SetminFuncEvals(size_t val)
	{
		m_uminFuncVal = val;
	}
	inline size_t Optimisable::GetminFuncEvals() const
	{
		return m_uminFuncVal;
	}

	inline void	Optimisable::SetminIters(size_t val)
	{
		m_uminIter = val;
	}
	inline size_t Optimisable::GetminIters() const
	{
		return m_uminIter;
	}

	inline void Optimisable::SetObjectiveTol(double val)
	{
		m_ObjectiveTol  = val;
	}
	inline double Optimisable::GetObjectiveTol() const
	{
		return m_ObjectiveTol;
	}

	inline void  Optimisable::SetParamTol(double val)
	{
		m_ParameterTol = val;
	}
	inline double Optimisable::GetParamTol() const
	{
		return m_ParameterTol;
	}

	inline void  Optimisable::RunDefaultOptimiser()
	{
		switch(m_DefaultOptimiser)
		{
		case TYPE_CG:
			break;
		case TYPE_SCG:
				SCGOptimise();
			break;
		default:
			break;
		}
	}
	inline void Optimisable::WriteToLog(std::string strContent) const
	{
	#ifdef _DEBUG
		(*m_pLog) << strContent << std::endl;
	#endif
	}
 	inline void Optimisable::SetLog(std::ostream *pLog)
	{
		m_pLog = pLog;
	}
};

#endif