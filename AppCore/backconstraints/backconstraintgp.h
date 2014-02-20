// Gaussian process back constraint function.
#pragma once

#include "backconstraint.h"

// Forward declarations.
class GPCMKernel;
class GPCMGaussianProcess;

class GPCMBackConstraintGP : public GPCMBackConstraint
{
protected:
    // Gaussian process.
    GPCMGaussianProcess *gaussianProcess;
    // Input matrix.
    MatrixXd gpInput;
    // Post-transform X matrix.
    MatrixXd Xpts;
    // Scaled X matrix (identical to post-transform X).
    MatrixXd XptsScaled;
    // Gradient of X matrix.
    MatrixXd XptsGrad;
    // Diagonal kernel matrix.
    MatrixXd K;
    // Kernel gradient matrix.
    MatrixXd gK;
    // Kernel gradient matrix, diagonal.
    MatrixXd gKd;
    // Temporary storage for alpha matrix.
    MatrixXd tempAlpha;
public:
    // Construct the back constraint function.
    GPCMBackConstraintGP(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
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
    virtual ~GPCMBackConstraintGP();
};
