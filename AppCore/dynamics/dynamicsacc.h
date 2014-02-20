// Second order finite differences dynamics prior.
#pragma once

#include "dynamics.h"

class GPCMDynamicsAcceleration : public GPCMDynamics
{
protected:
    // Weight on the dynamics term.
    double weight;
public:
    // Constructor.
    GPCMDynamicsAcceleration(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, MatrixXd &dataMatrix, GPCMModel *model,
        std::vector<int> &sequence, double frameLength);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMDynamics *other);
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMDynamicsAcceleration();
};
