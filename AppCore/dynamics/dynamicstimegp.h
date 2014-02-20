// Temporally regressive gaussian process dynamics prior.
#pragma once

#include "dynamics.h"

// Forward declarations.
class GPCMGaussianProcess;
class GPCMPrior;

// Number of different kinds of distances we support.
#define DIST_TYPES      2

class GPCMDynamicsTimeGP : public GPCMDynamics
{
protected:
    // Weight (scale) to use on the dynamics term.
    double weight;
    // Indicates whether we are learning the GP or if the parameters are fixed.
    bool bLearnGP;
    // Indicates whether we are learning scales for poses.
    bool bLearnScales;
    // Indicates whether these scales are tied to reconstruction GP scales.
    bool bTiedScales;
    // Gaussian process.
    GPCMGaussianProcess *gaussianProcess;
    // Time matrix.
    MatrixXd timeMat;
    // Distances matrices.
    MatrixXd dists[DIST_TYPES];
    // Stored data matrix.
    MatrixXd dataMatrix;
    // Stored scales.
    MatrixXd scales;
    // Scale gradients.
    MatrixXd scaleGrads;
    // Position scales.
    MatrixXd positionScales;
    // Position scale gradients.
    MatrixXd positionScaleGrads;
    // Velocity scales.
    MatrixXd velocityScales;
    // Velocity scale gradients.
    MatrixXd velocityScaleGrads;
    // Indices into full scales from position scales.
    std::vector<int> positionScaleIndices;
    // Indices into full scales from velocity scales.
    std::vector<int> velocityScaleIndices;
public:
    // Constructor.
    GPCMDynamicsTimeGP(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, MatrixXd &dataMatrix, GPCMModel *model,
        std::vector<int> &sequence, double frameLength);
    // Get Gaussian process pointer.
    virtual GPCMGaussianProcess *getGaussianProcess();
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
    virtual ~GPCMDynamicsTimeGP();
};
