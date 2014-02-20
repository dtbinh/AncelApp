#ifndef __MGPKERNEL_H_
#define __MGPKERNEL_H_

#include "Kernel.h"
#include "RBFKernel.h"
#include "LinearKernel.h"
#include "WhiteNoiseKernel.h"

#include <vector>

namespace ResCore
{
	using namespace ResUtil;

	typedef std::vector<std::vector<size_t>> Array2D;
	 
	class MGPKernel
	{
	public:
		/**
		*Constructor,
		*/
		explicit MGPKernel(size_t numData);			
		virtual ~MGPKernel();

		virtual size_t getNumParams() const;
		/**
		*calculate Kernel matrix
		*@param K kernel matrix
		*@param factor multifactor
		*/
		void     computeKernel(MMatrix &K, const std::vector<MMatrix*> &factor) const;
		double   computeKernel(const std::vector<MMatrix*> &factor) const;
		void     computeKernel(MMatrix &Kx, const std::vector<const MMatrix*> &X,const std::vector<MMatrix*> &X2) const;
		/**
		*calculate D(K)/D(factor)
		*/
		double getTransParam(size_t index);
		double getParam(size_t index);


		void   getPrams(MMatrix & params);
		void   setPrams(MMatrix & params);
		void   setParam(double val, size_t index);
		void   setTransParam(double val,size_t index);

		void   getGradientX(std::vector<MMatrix*>&g, const std::vector<MMatrix*> &factor);
		void   getGradientParams(MMatrix &g, const std::vector<MMatrix*> &factor, const MMatrix &covGrad);			
		void   getGradTransParams(MMatrix &g, const std::vector<MMatrix*> &factor, const MMatrix &covGrd);
	private:
   		
  	 	Kernel*                 mNoiseKernel;
		std::vector<Kernel*>    mKernels;

		size_t	                mNumData;
		mutable bool	        mIsUpdate;
		mutable vector<MMatrix*> mUnits;
		mutable vector<MMatrix*> mComMult;
	};
}
#endif