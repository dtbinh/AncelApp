#ifndef _gm_h_
#define _gm_h_

#include "gp.h"
#include "optimizable.h"
#include "optimization.h"
#include "rankprior.h"
#include "backconstraint.h"
#include "velocityterm.h"
#include "supplementary.h"
#include "dynamics.h"
#include <Eigen/Eigen>

using namespace Eigen;

class GenerativeModel:public GPCMOptimizable
{
public:
	GenerativeModel(GPCMOptions &inOptions,bool bLoadTrainedModel, 
								bool bRunHighDimensionalOptimization);
	~GenerativeModel();

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
	//copy settings ..........
	void copySettings(const GenerativeModel & model);
	//load to use
	void load(GPCMMatReader *reader);
	// Writer to use.
	void write(GPCMMatWriter *writer);
protected:

	int				        mLatDim;
	
	bool			        mRunOptimization;

	double					mLogLikelihood;

	MatrixXd			    mDataMatrix;
	MatrixXd				mY;
	MatrixXd				mX;
	MatrixXd				mXGrad;
	std::vector<int>		mSequence;
	
	GPCMRankPrior*			mRankPrior;
	GPCMOptimization*		mOptimization;
	GPCMGaussianProcess*	mReconstructionGP;
	GPCMBackConstraint*	    mBackConstraint;
	GPCMVelocityTerm*		mVelocityTerm;
	GPCMSupplementaryData*  mSupplementary;
    GPCMDynamics *          mDynamics;

	bool					mHighDimensionalOptimization;
};
#endif