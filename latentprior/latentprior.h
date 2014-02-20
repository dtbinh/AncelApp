// Abstract GPCM latent prior.
#pragma once

#include "options.h"

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;
class GPCMMatReader;
class GPCMOptimization;
class GPCMGaussianProcess;
class GPCMTask;
class GPCMController;

// Possible types of value terms.
enum GPCMLatentPriorType
{
    ValueTypeConnectivity,
    ValueTypeUnknown
};

class GPCMLatentPrior
{
protected:
    // Latent positions matrix.
    MatrixXd &X;
    // Gradient of latent positions.
    MatrixXd &Xgrad;
    // Sequence array.
    std::vector<int> &sequence;
    // Type of value object.
    GPCMLatentPriorType type;
    // Pointer to controller.
    GPCMController *controller;
public:
    // Global function for creating new value function term.
    static GPCMLatentPrior *createLatentPrior(GPCMParams &params, GPCMOptions &options,
        GPCMOptimization *optimization, MatrixXd &X, MatrixXd &Xgrad, std::vector<int> &sequence,
        GPCMTask *task, GPCMController *controller);
    // Constructor.
    GPCMLatentPrior(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, std::vector<int> &sequence, GPCMTask *task, GPCMController *controller);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMLatentPrior *other);
    // Get the type of this value function object.
    virtual GPCMLatentPriorType getType();
    // Set new controller.
    virtual void setController(GPCMController *controller);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm() = 0;
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient) = 0;
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMLatentPrior();
};
