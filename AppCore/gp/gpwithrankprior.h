#ifndef __GPWithRankPrior_h
#define __GPWithRankPrior_h

#include "gp.h"
#include "optimizable.h"
#include "optimization.h"
#include "rankprior.h"
#include "backconstraint.h"

class GPWithRankPrior: public GPCMOptimizable
{
public:
	GPWithRankPrior(GPCMOptions &inOptions,bool bLoadTrainedModel, bool bRunHighDimensionalOptimization);
	~GPWithRankPrior();

 	virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Recompute constraint, assuming temporaries are up to date.
    virtual double recomputeConstraint(bool bNeedGradient);
    // Check if a constraint exists.
    virtual bool hasConstraint();
    // Save gradient for debugging purposes.
    virtual void setDebugGradient(const VectorXd &dbg, double ll);
	// Train the model.
	void optimize();

	MatrixXd getLatentVariable();
	void copySettings(const GPWithRankPrior & model);
	void load(GPCMMatReader *reader);
protected:

	MatrixXd GPWithRankPrior::filterData( MatrixXd &dataMatrix,  std::vector<int> sequence, double variance);
protected:
	bool			        mbRunOptimization;
	int				        mLatDim;
	double					mLogLikelihood;
	MatrixXd			    mDataMatrix;
	MatrixXd				mY;
	MatrixXd				mX;
	MatrixXd				mXGrad;
	std::vector<int>		mSequence;
	GPCMRankPrior*			mRankPrior;
	GPCMOptimization*		mOptimization;
	GPCMGaussianProcess*	mReconstructionGP;
	GPCMBackConstraint*     mBackConstraint;
	bool					mbHighDimensionalOptimization;
};

#endif