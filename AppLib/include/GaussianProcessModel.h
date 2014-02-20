/**
*-----------------------------------------------------------------------------
*Filename:  GaussianProcessModel.h
*-----------------------------------------------------------------------------
*File Description: the 
*-----------------------------------------------------------------------------
*Author: Ancel         2011/11/21               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __GaussianProcessModel_H
#define __GaussianProcessModel_H

#include "kernel.h"
#include "MMatrix.h"
#include "Optimization.h"


namespace ResModel
{
	using namespace ResUtil;
	class GuaasinProcessModel: public ResCore::Optimisable
	{
	public:
		GuaasinProcessModel(ResUtil::MMatrix& Y,ResUtil::MMatrix &X,ResCore::Kernel* kernel = NULL);
		~GuaasinProcessModel();
		ResUtil::MMatrix predict(ResUtil::MMatrix &X);

		virtual void getOptiParams(ResUtil::MMatrix& param) const;
		virtual void setOptiParams(const ResUtil::MMatrix& param);
		virtual size_t getOptiParamsNum()     const;

		virtual double computeObjectiveVal();
		virtual double computeObjectiveGradParams(ResUtil::MMatrix & g);

		void optimize(size_t iterNum);
	protected:
		void initValues();
		void updateCovGradient() const;
		void update() const;
	private:

		mutable bool mIsNeedUpdate;
		std::size_t mNumDim;
		std::size_t mNumData;
 
		MMatrix mX;
		MMatrix mY;
		mutable MMatrix mK;
		mutable MMatrix mInvK;

		MMatrix mMeanY;
		MMatrix mVarY;
		mutable MMatrix mCentredY;

		mutable double  mLnDK;
		mutable MMatrix  mGraCov;
		ResCore::Kernel *mKernel;
	};
};

#endif