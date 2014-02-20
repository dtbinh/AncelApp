#ifndef GPLVM_H
#define GPLVM_H

#include "Kernel.h"
#include "RBFKernel.h"
#include "BiasKernel.h"
#include "LinearKernel.h"
#include "CompoundKernel.h"
#include "WhiteNoiseKernel.h"
#include "MMatrix.h"
#include "Optimization.h"
#include <vector>
#include <string>

namespace ResModel 
{
	using namespace ResCore;
	//-----------------------------------------------------------------------------------------------
	class Model_GPLVM
	{
 		MMatrix mPx;
		MMatrix mKernel;
		std::pair<std::string,double> mParams;
	};
	//-----------------------------------------------------------------------------------------------
 	class CGPLVM : public Optimisable
	{
	public:
		explicit CGPLVM(Kernel* pKernel,MMatrix *pYin,size_t LatDim = 2);
		explicit CGPLVM(Kernel* pKernel,MMatrix *pYin,Kernel *pBackKernel,size_t LatDim = 2);
 		void InitVals();
		void InitXByPCA();
		void InitStoreage();
		
		void UpdateX();
		void Update() const;
		void UpdateK() const;
		void UpdateInvK() const;
		void UpdateCovGradient() const;

		void SetInputscaleLearnt(bool val);
		void LearnModel();

		virtual double LogLikelihood() const;
		double LogLikelihoodGradient(MMatrix& g) const;

		void Optimize(size_t iter);
		virtual void getOptiParams(MMatrix& param) const;
		virtual void setOptiParams(const MMatrix& param);
		virtual size_t getOptiParamsNum()     const;

		virtual double computeObjectiveVal();
		virtual double computeObjectiveGradParams(MMatrix & g);

		void SetLatentVals(MMatrix *pX);
		MMatrix* GetLatentVals() const;
		MMatrix* GetDataSpaceDist() const;
 	//	virtual double PredictY(MMatrix x);
	protected:
		
		MMatrix     *m_pX;
		MMatrix     *m_pYIn;
 		MMatrix		m_BackK;
		MMatrix		m_ParamA;
		MMatrix      m_CentredY;
		MMatrix		m_MeanY;
		MMatrix		m_scale;
	 

		mutable MMatrix      m_K;
		mutable MMatrix      m_InvK;
		mutable MMatrix      m_InvKY;
		mutable MMatrix      m_LcholK;
		mutable MMatrix      m_DiagX;
		mutable MMatrix      m_CovGradient;
		mutable std::vector<MMatrix*> m_vGX;
  		
		Kernel			*m_pKernel;
		Kernel			*m_pBackKernel;
//      CscaleNoise		*m_pNoise;

		size_t  m_uLatDim;
		size_t  m_uDataNum;
		size_t  m_uDataDim;

 		mutable double	m_LogDetK;
		 
		bool    m_bIsBackConstrained;
		bool	m_bIsInputscaleLearnt;
		bool	m_bIsLatentRegularised;

		mutable bool    m_bIsKtoUpdate;
  	};
}
#endif