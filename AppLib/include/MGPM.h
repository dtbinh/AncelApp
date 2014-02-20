#ifndef __MGPM_H
#define __MGPM_H
 
#include "Kernel.h"
#include "Optimization.h"
#include "MGPKernel.h"
#include "ResUtility.h"

namespace ResModel
{
	using namespace ResCore;
 	 
	class MGPModel: public Optimisable
	{
	public:
		explicit MGPModel(ResUtil::MocapData &mocapData);
  		~MGPModel();
  
		void optimize(std::size_t iter);
 		
		bool writeModelToFile(std::string name);
//		void loadModelFromFile(std::string name);

		double LogLikelihood() const;
		double LogLikelihoodGradient(MMatrix& g) const;
		virtual void getOptiParams(MMatrix& param) const;
		virtual void setOptiParams(const MMatrix& param);
		virtual size_t getOptiParamsNum()     const;
		virtual double computeObjectiveVal();
		virtual double computeObjectiveGradParams(MMatrix & g);
		
	private:
		void initModel();
	 	void initScaleFactor();
		void calcuateOffset();
		void toCircle(MMatrix & mat) const;
		void update() const;
		void updateCovGradient() const;
		void computeFakeFactor() const;
   	private:
			
		MMatrix *mYIn;
		
		MMatrix  mVarY;
		MMatrix  mMeanY;
 		MMatrix  mCentredY;
		MMatrix  mInitY;

		mutable MMatrix  mK;
		mutable MMatrix  mW;
		mutable MMatrix  mInvK;
		mutable MMatrix  mScaleY;
		mutable MMatrix  mInvKY;
  		mutable MMatrix  mGraCov;
		
		mutable std::vector<MMatrix*> mGx;

		size_t  mDimData;
		size_t  mNumData;
		size_t  mNumSegments;
		size_t  mDimLatent;
		size_t  mNumContent;
		size_t  mNumIdentity;
 
		bool    mScaleflag;
		bool    mTtrained;
		
		mutable double  mLnDK;
		mutable bool    mIsUpdated;
		
 		MGPKernel *mKernel;
  		std::vector<MMatrix*> mGradTs;
		
	 	std::vector<MMatrix*> mFactors;
	 	std::vector<MMatrix*> mFakeFactors;
		
		std::vector<std::string> mActorLs;
		std::vector<std::string> mActionLs;
		std::vector<std::string> mMotionNameLs;

		std::vector<double> mCycles;
		std::string         mModelName;
  		std::vector<size_t> mSegments;
 		Array2D mFacIdx;
 		Array2D mSegmentLabels;
	};
};

#endif