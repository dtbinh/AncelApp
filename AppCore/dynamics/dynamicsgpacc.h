// Second order dynamical system with passive acceleration field given by GP.
#pragma once

#include "dynamics.h"

// Forward declarations.
class GPCMGaussianProcess;
class GPCMPrior;

class GPCMDynamicsAccelerationGP : public GPCMDynamics
{
protected:
    // Gaussian process.
    GPCMGaussianProcess *gaussianProcess;
    // Gradient of GP input matrix.
    MatrixXd inputGrad;
    // GP input matrix.
    MatrixXd inputMatrix;
    // GP output matrix.
    MatrixXd outputMatrix;
    // GP output matrix after scaling (if any).
    MatrixXd outputMatrixScaled;
    // Length of a single frame.
    double frameLength;
    // Locations of the first point in each sequence.
    std::vector<MatrixXd> firstPointLocation;
    // Locations of the second point in each sequence.
    std::vector<MatrixXd> secondPointLocation;
    // Gradients of the first point in each sequence.
    std::vector<MatrixXd> firstPointGrad;
    // Gradients of the second point in each sequence.
    std::vector<MatrixXd> secondPointGrad;
    // Priors on the first point in each sequence.
    std::vector<GPCMPrior*> firstPointPrior;
    // Priors on the second point in each sequence.
    std::vector<GPCMPrior*> secondPointPrior;
    // Helper function for creating the Gaussian process.
    virtual void createGaussianProcess(GPCMParams &params, GPCMOptions &options,
        GPCMOptimization *optimization);
    // Set input and output matrices from current X matrix.
    virtual void setInputOutput();
public:
    // Constructor.
    GPCMDynamicsAccelerationGP(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, MatrixXd &dataMatrix, GPCMModel *model,
        std::vector<int> &sequence, double frameLength, bool bPriorsOnly = false);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMDynamics *other);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMDynamicsAccelerationGP();
};
