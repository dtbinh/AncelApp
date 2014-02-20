#ifndef __MGPM_h
#define __MGPM_h

#include "gp.h"
#include "optimizable.h"
#include "optimization.h"
#include "rankprior.h"
#include "backconstraint.h"

//Multifactor Gaussian Model
class GPMGPModel: public GPCMOptimizable
{
public:
	GPMGPModel(GPCMOptions &inOptions,bool bLoadTrainedModel, bool bRunHighDimensionalOptimization);
	~GPMGPModel();

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

	void getLatentVariable(std::vector<MatrixXd*> &latentVar);
	void copySettings(const GPMGPModel & model);
	void load(GPCMMatReader *reader);

protected:
	void recomputeX();
	void recomputeGradient();
	void initStateFactor(MatrixXd* factor);
	void initLatentVariable(GPCMOptions &inOptions);
  	MatrixXd filterData( MatrixXd &dataMatrix,  std::vector<int> sequence, double variance);
protected:
	bool			        mbRunOptimization;
	int				        mLatDim;
	double					mLogLikelihood;
	MatrixXd			    mDataMatrix;
	MatrixXd				mY;
	
	MatrixXd**				mX;
	MatrixXd**				mXGrad;
    
	MatrixXi			    mLatentIndex;
	
	MatrixXd**				mCDMGradient;
	std::vector<MatrixXd*>	mLatentVariable;
	std::vector<MatrixXd*>  mGradientLatent;

	std::vector<int>		mSequence;
//	GPCMRankPrior*			mRankPrior;
	GPCMOptimization*		mOptimization;
	GPCMGaussianProcess*	mReconstructionGP;
//	GPCMBackConstraint*     mBackConstraint;
	bool					mbHighDimensionalOptimization;
};



#endif