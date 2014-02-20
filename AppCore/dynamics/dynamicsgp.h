// First order autoregressive gaussian process dynamics prior.
#pragma once

#include "dynamics.h"

// Forward declarations.
class GPCMGaussianProcess;
class GPCMPrior;
class GPCMKernel;

class GPCMDynamicsGP : public GPCMDynamics
{
protected:
    // Weight (scale) to use on the dynamics term.
    double weight;
    // Whether the output matrix is an output location or an offset.
    bool bDifference;
    // Whether to include velocity in the input.
    bool bUseVelocity;
    // Whether to use tensor product kernel with velocity.
    bool bTensorProduct;
    // Tensor product kernel.
    GPCMKernel *tensorKernel;
    // Gaussian process.
    GPCMGaussianProcess *gaussianProcess;
    // Gradient of GP input matrix.
    MatrixXd inputGrad;
    // GP input matrix.
    MatrixXd inputMatrix;
    // Gradient of GP velocity input matrix.
    MatrixXd velGrad;
    // GP velocity input matrix.
    MatrixXd velMatrix;
    // GP output matrix.
    MatrixXd outputMatrix;
    // GP output matrix after scaling (if any).
    MatrixXd outputMatrixScaled;
    // Locations of the first point in each sequence.
    std::vector<MatrixXd> firstPointLocation;
    // Gradients of the first point in each sequence.
    std::vector<MatrixXd> firstPointGrad;
    // Priors on the first point in each sequence.
    std::vector<GPCMPrior*> firstPointPrior;
    // Helper function for creating the Gaussian process.
    virtual void createGaussianProcess(GPCMParams &params, GPCMOptions &options,
        GPCMOptimization *optimization);
    // Set input and output matrices from current X matrix.
    virtual void setInputOutput();
public:
    // Constructor.
    GPCMDynamicsGP(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, MatrixXd &dataMatrix, GPCMModel *model,
        std::vector<int> &sequence, double frameLength, bool bPriorsOnly = false);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMDynamics *other);
    // Get underlying Gaussian process.
    virtual GPCMGaussianProcess *getGaussianProcess();
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMDynamicsGP();
};
