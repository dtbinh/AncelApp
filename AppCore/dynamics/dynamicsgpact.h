// First order autoregressive gaussian process dynamics prior with latent actions.
#pragma once

#include "dynamicsgp.h"

class GPCMDynamicsActionGP : public GPCMDynamicsGP
{
protected:
    // Gradient of latent actions.
    MatrixXd Agrad;
    // Latent actions matrix.
    MatrixXd A;
    // Action prior.
    GPCMPrior *actionPrior;
    // Helper function for creating the Gaussian process.
    virtual void createGaussianProcess(GPCMParams &params, GPCMOptions &options,
        GPCMOptimization *optimization);
public:
    // Constructor.
    GPCMDynamicsActionGP(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, MatrixXd &dataMatrix, GPCMModel *model,
        std::vector<int> &sequence, double frameLength);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMDynamics *other);
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Destructor.
    virtual ~GPCMDynamicsActionGP();
};
