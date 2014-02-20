// Tensor product kernel function.
#pragma once

#include "kernel.h"
#include "options.h"

class GPCMTensorKernel : public GPCMKernel
{
protected:
    // Number of "sub-kernels".
    int componentCount;
    // Array of "sub-kernels".
    GPCMKernel *components[MAX_KERNEL_GROUPS];
    // Array of "sub-kernel" input indices.
    int componentInputs[MAX_KERNEL_GROUPS];
    // Temporary storage for leave-one-out product matrices.
    MatrixXd Kslash[MAX_KERNEL_GROUPS];
    // Additional temporary storage for leave-one-out product matrices.
    MatrixXd KgradSlash[MAX_KERNEL_GROUPS];
    // Temporary storage for computing Krbf.
    MatrixXd KrbfNew;
    MatrixXd KrbfComp;
public:
    // Construct the kernel.
    GPCMTensorKernel(GPCMParams &params, GPCMOptimization *optimization,
        GPCMKernel **components, int componentCount, int dims);
    // Copy any settings from another kernel that we can.
    virtual void copySettings(GPCMKernel *other);
    // Return white noise from this kernel.
    virtual double getNoise();
    // Return covariance of a single set of points.
    virtual MatrixXd covariance(const MatrixXd* const *X);
    // Return coviarance of two sets of points.
    virtual MatrixXd covariance(const MatrixXd* const *X1, const MatrixXd* const *X2);
    // Return covariance of a single set of points and the gradient.
    virtual MatrixXd covarianceGrad(const MatrixXd* const *X, MatrixXd &Xgrad);
    // Return coviarance of two sets of points.
    virtual MatrixXd covarianceGrad(const MatrixXd* const *X1, const MatrixXd* const *X2, MatrixXd &X2grad);
    // Recompute priors on all kernel parameters and return the likelihood.
    virtual double recomputePriors();
    // Recompute gradients of hyperparameters and latent coordinates.
    virtual void recompute(const MatrixXd &gK, const MatrixXd &gKd, const MatrixXd* const *X, MatrixXd **Xgrad);
    // Return covariance of a single set of points in gradient observation mode.
    virtual MatrixXd covarianceGradientGP(const MatrixXd* const *X);
    // Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
    virtual void recomputeGradientGP(const MatrixXd &gK, const MatrixXd &gKd, const MatrixXd* const *X, MatrixXd **Xgrad);
    // Write kernel data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load kernel from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Print out kernel parameters.
    virtual void printParams();
    // Destructor.
    virtual ~GPCMTensorKernel();
};
