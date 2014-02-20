#ifndef __HMGPM_h_
#define __HMGPM_h_

#include "Kernel.h"
#include "Optimization.h"
#include "MGPKernel.h"
#include "ResUtility.h"
#include "CompoundKernel.h"
#include "Transform.h"
#include "Vector3D.h"
#include "GaussianProcessModel.h"
namespace ResModel
{
	using namespace ResCore;

	//Hierarchical Multi-factor Gaussian Process Model
 	class HMGPModel: public ResCore::Optimisable
	{
	public:
		explicit HMGPModel(std::string modelName);
		explicit HMGPModel(ResUtil::MocapData &mocapData);
		~HMGPModel();

		double logLikelihood() const;
		double logLikelihoodGradient(MMatrix& g)   const;

		void optimize(std::size_t iter);

		virtual void getOptiParams(MMatrix& param) const;
		virtual void setOptiParams(const MMatrix& param);
		
		virtual size_t getOptiParamsNum()     const;
		
		virtual double computeObjectiveVal();
		virtual double computeObjectiveGradParams(MMatrix & g);

		
		void writeModelToFile(std::string name);
		void loadModelFromFile(std::string name);
		
		//Experiment For Reconstruction Test
		MMatrix reconstruction(std::size_t index);
		//TODO
		MMatrix generate(std::size_t indexF1, std::size_t indexF2, std::size_t length);
		//Experiment For Interpolation
		//TODO
		//Experiment For Transition

		//functions for visable convenience
		MMatrix getX() const {return *mFakeFactors[0];}
		std::vector<size_t> getSegments() const {return mSegments;}

		std::string getModelName(){return mModelName;}
		
 		const std::vector<std::string>& getFactorAList() {return mFactor1Ls;}
		const std::vector<std::string>& getFactorBList() {return mFactor2Ls;}


		MMatrix predictX(std::size_t indexF1, std::size_t indexF2, std::size_t length) const;
 	protected:
		void constructStepPrior();
		
		//Function For prediction
		
		MMatrix meanPrediction(const std::vector<MMatrix> &X, CVector3D<double> initPos);


		//Function for learning
		void initKernel(std::size_t dataNum);
		void initOptiParams();
		void initStorge();
	 	void initScaleFactor(bool flag = false);
		void calcuateOffset();
		void update()			 const;
		void updateCovGradient() const;
		void updateFakeFactor()  const;
 	private:
		
		MMatrix  mFeaVec;					//short for feacture vector
		
		MMatrix  mVarFeaVec;
		MMatrix  mMeanFeaVec;
 		MMatrix  mCentredFeaVec;
		MMatrix  mInitFeaVec;

		mutable MMatrix  mK;
 		mutable MMatrix  mW;
		mutable MMatrix  mInvK;
		mutable MMatrix  mScaleFeaVec;		//W*Y
		mutable MMatrix  mInvKFeaVec;		//K^-1*Y
  		mutable MMatrix  mGradientK;
		mutable MMatrix  mScaleX;

		mutable MMatrix  mKs;
		mutable MMatrix  mInvKs;
		mutable MMatrix  mWs;
		mutable MMatrix  mGradientKs;
		mutable double  mLnDKs;
		std::size_t     mDimState;
 		CompoundKernel *mStateKernel;		

		mutable std::vector<MMatrix*> mGx;   //Gradient X
		mutable std::vector<MMatrix*> mGt;   //Gradient X
		
		size_t  mDimData;
		size_t  mNumData;
		size_t  mNumSegments;
		size_t  mDimLatent;
		size_t  mNumFactor1;
		size_t  mNumFactor2;
 
		bool    mTtrained;
		
		mutable double  mLnDK;           //ln|K|
		mutable bool    mIsUpdated;
		
 		MGPKernel *mKernel;
		
		CExpTransform mPostiveTransform;  	 		
	 	
		std::vector<MMatrix*> mFactors;
	 	std::vector<MMatrix*> mFakeFactors;
		mutable MMatrix       mStatesFactor; 
		
		std::string mModelName;
		std::vector<std::string> mFactor1Ls;
		std::vector<std::string> mFactor2Ls;

	 	std::vector<size_t> mSegments;
		
		ResModel::GuaasinProcessModel *mGPM;
		
		Array2D mPoseProperties;
 		Array2D mSegmentLabels;
	};
}
#endif