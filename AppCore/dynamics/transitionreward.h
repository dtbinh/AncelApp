// Wrapper for GP dynamics term that is used to compute a transition reward.
#pragma once

#include "options.h"
#include "optimizable.h"

// Forward declarations.
class GPCMGaussianProcess;
class GPCMModel;
class GPCMOptimization;
class GPCMMatWriter;
class GPCMMatReader;
class GPCMKernel;

class GPCMTransitionReward : public GPCMOptimizable
{
protected:
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
    // GP input matrix.
    MatrixXd inputMatrix;
    // GP velocity matrix.
    MatrixXd velMatrix;
    // GP output matrix.
    MatrixXd outputMatrix;
    // Scaled GP output matrix.
    MatrixXd outputMatrixScaled;
    // Reference to latent positions matrix.
    MatrixXd &X;
    // Reference to model.
    GPCMModel *model;
    // Refernece to sequence.
    std::vector<int> &sequence;
    // Frame length.
    double frameLength;
    // Optimization manager.
    GPCMOptimization *optimization;
    // Temporary storage for inputs during evaluation.
    MatrixXd tempInputs;
    // Temporary storage for kernalized inputs during evaluation.
    MatrixXd tempKstar;
    // Temporary storage for diagonal kernels during evaluation.
    MatrixXd tempKdiag;
    // Temporary storage for a single row.
    MatrixXd tempSingleRow;
    // Temporary storage for a single velocity row.
    MatrixXd tempVSingleRow;
    // Temporary storage for means.
    MatrixXd tempMeans;
    // Temporary storage for variances.
    MatrixXd tempVars;
    // Helper function for creating the Gaussian process.
    virtual void createGaussianProcess(GPCMParams &params, GPCMOptions &options);
    // Set input and output matrices from current X matrix.
    virtual void setInputOutput();
public:
    // Global function for creating new transition reward.
    static GPCMTransitionReward *createTransitionReward(GPCMParams &params, GPCMOptions &options,
        MatrixXd &X, GPCMModel *model, std::vector<int> &sequence, double frameLength);
    // Constructor.
    GPCMTransitionReward(GPCMParams &params, GPCMOptions &options, MatrixXd &X, GPCMModel *model,
        std::vector<int> &sequence, double frameLength);
    // Evaluate the probability of a given transition given the distributions.
    virtual void evaluateProbability(const MatrixXd &means, const MatrixXd &vars, const MatrixXd *XA,
        int actions, MatrixXd *TProb);
    // Get the probability distribution for transitions from a given point.
    virtual void getDistribution(const MatrixXd &X, const MatrixXd &V, MatrixXd &means, MatrixXd &vars);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMTransitionReward *other);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Train the model.
    void optimize();
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMTransitionReward();
};
