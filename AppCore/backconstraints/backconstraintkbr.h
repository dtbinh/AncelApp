// Kernel-based regression back constraint function.
#pragma once

#include "backconstraint.h"

// Forward declarations.
class GPCMKernel;

class GPCMBackConstraintKBR : public GPCMBackConstraint
{
protected:
    // Kernel.
    GPCMKernel *kernel;
    // Kernel matrix.
    MatrixXd K;
    // Kernel matrix for inference (without noise term).
    MatrixXd Kd;
    // Alpha matrix.
    MatrixXd A;
    // Gradient of alpha matrix.
    MatrixXd Agrad;
public:
    // Construct the back constraint function.
    GPCMBackConstraintKBR(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &dataMatrix, MatrixXd &X);
    // Copy any settings from another back constraint function that we can.
    virtual void copySettings(GPCMBackConstraint *other);
    // Initialize the back constraint function by optimizing the parameters for an initial latent matrix.
    virtual void initialize();
    // Compute the gradients of this back constraint function using the gradient of the latent coordinates.
    virtual double updateGradient(MatrixXd &Xgrad, bool bNeedGradient);
    // Update the latent coordinates based on the current parameters.
    virtual void updateLatentCoords();
    // Write back constraint to file.
    virtual void write(GPCMMatWriter *writer);
    // Load parameters from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMBackConstraintKBR();
};
