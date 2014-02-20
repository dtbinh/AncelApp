#ifndef __MotionSyn_h
#define __MotionSyn_h

#include "MMatrix.h"
#include "MGPKernel.h"
#include "GaussianProcessModel.h"
#include "Vector3D.h"

namespace ResModel
{
	class MotionSyn
	{
	public:
		explicit MotionSyn(std::string modelName);
		~MotionSyn();
		ResUtil::MMatrix synthesis(std::size_t identity,std::size_t content,std::size_t length);
		ResUtil::MMatrix generate(std::size_t idetity,vector<std::size_t> contents,std::size_t interval);
		ResUtil::MMatrix reconstruct(std::size_t motionIndex);
		ResUtil::MMatrix testTransitionSyn();
		ResUtil::MMatrix synTrainsiton(const std::size_t identity, const std::size_t content1, const std::size_t content2,
									  const std::size_t length, CVector3D<double> initPos, double &curState);
		/**
		* this function is used for initialize the variable that is necessary for motion synthesis
		*/
		const std::string& getModelName() const;
		const std::vector<std::string>& getFactorAList() const;
		const std::vector<std::string>& getFactorBList() const;
		const std::vector<std::string>& getMotionNameList() const;
		
		MMatrix syc();
		MMatrix syc(size_t subject, size_t style);
  		void initSynthesis();
		ResUtil::MMatrix evaluateFactors(ResUtil::MMatrix unknownMotion);
		std::size_t getMotionSemgemtnNum();
	protected:
		/**
		* use for loading the trained model
		*@param modelName the name of the trained model
		*/
		bool loadModel(std::string modelName);
		void computeFakeFactor();
		void toCircle(ResUtil::MMatrix & mat) const;
		void initStoreage();
		void computeBilinearMapping();
		void calcFactorsPrior();
		ResUtil::MMatrix meanPrediction(const std::vector<MMatrix> &X, CVector3D<double> initPos);
		ResUtil::MMatrix calcMean(const std::vector<ResUtil::MMatrix*> &X);

  	private:

		std::size_t mCurrentContent;

		std::size_t mNumDim;
		std::size_t mNumData;											
		std::size_t mNumFactorDim;

		std::string mModelName;

		ResUtil::MMatrix  mVarY;										//
		ResUtil::MMatrix  mMeanY;									//
		ResUtil::MMatrix  mInitY;

  		ResUtil::MMatrix mW;											//
		ResUtil::MMatrix mY;											//
		ResUtil::MMatrix mK;											//
		ResUtil::MMatrix mInvK;										//					
		ResUtil::MMatrix mCentredY;								    //
		ResUtil::MMatrix mBMapping;
		ResUtil::MMatrix mFactorsInvCov;
		ResUtil::MMatrix mFactorsMean;					
		ResUtil::MMatrix mFactorsSample;					
		ResCore::MGPKernel *mKernel;								//

		std::vector<ResUtil::MMatrix*> mGradTs;						//
		std::vector<ResUtil::MMatrix*> mFactors;						//
		std::vector<ResUtil::MMatrix*> mFakeFactors;					//
		
		std::vector<std::size_t> mSegments;							//
		std::vector<std::vector<size_t>> mFacIndex;					//
		std::vector<std::vector<size_t>> mSegmentLabel;				//

		std::vector<std::string> mActorLs;
		std::vector<std::string> mActionLs;
		std::vector<std::string> mMotionNameLs;

		ResModel::GuaasinProcessModel *mGPM;
	};
};

#endif