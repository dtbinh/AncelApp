// Abstract dynamics prior.
#pragma once

#include "options.h"

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;
class GPCMMatReader;
class GPCMOptimization;
class GPCMLatentPrior;
class GPCMModel;

// Possible types of dynamics.
enum GPCMDynamicsType
{
    DynamicsTypeUnknown,
    DynamicsTypeAcceleration,
    DynamicsTypeGP,
    DynamicsTypeActionGP,
    DynamicsTypeAccelerationGradientGP,
    DynamicsTypeLMDP,
    DynamicsTypeTimeGP
};

class GPCMDynamics
{
protected:
    // Latent positions matrix.
    MatrixXd &X;
    // Gradient of latent positions.
    MatrixXd &Xgrad;
    // Sequence array.
    std::vector<int> &sequence;
    // Type of dynamics object.
    GPCMDynamicsType type;
public:
    // Global function for creating new dynamics.
    static GPCMDynamics *createDynamics(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, MatrixXd &dataMatrix, GPCMModel *model,
        std::vector<int> &sequence, double frameLength);
    // Constructor.
    GPCMDynamics(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, MatrixXd &dataMatrix, GPCMModel *model,
        std::vector<int> &sequence, double frameLength);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMDynamics *other);
    // Get the type of this dynamics object.
    virtual GPCMDynamicsType getType();
    // Set value term.
    virtual void setValueTerm(GPCMLatentPrior *value);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient) = 0;
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMDynamics();
};
