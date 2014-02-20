// RBF kernel function.
#pragma once

#include "kernel.h"
#include "options.h"

// Forward declarations.
class GPCMPrior;

class GPCMRBFKernel : public GPCMKernel
{
protected:
    // Inverse length scale.
    MatrixXd iw;
    // Inverse length scale gradient.
    MatrixXd iwgrad;
    // Inverse length prior.
    GPCMPrior *iwprior;
    // Variance.
    MatrixXd var;
    // Variance gradient.
    MatrixXd vargrad;
    // Variance prior.
    GPCMPrior *varprior;
    // Scale to apply to each channel.
    MatrixXd scale;
    // Scale gradient.
    MatrixXd scalegrad;
    // Scale prior.
    GPCMPrior *scaleprior;
    // Temporary storage for pairwise distances.
    MatrixXd dists;
    // Temporary storage for entire RBF kernel matrix.
    MatrixXd kmat;
    // Temporary storage for fat identity matrix used in gradient GP kernel computation.
    MatrixXd fatIdentity;
    // Temporary storage for delta rectangular matrix used in gradient GP kernel computation.
    MatrixXd delta;
    // Temporary storage for delta product square matrix used in gradient GP kernel computation.
    MatrixXd deltaMat;
    // Temporary storage for gradient GP kernel.
    MatrixXd gradientKmat;
    // Temporary storage for sum of grad-K components when computing gradient in gradient GP mode.
    MatrixXd dmat;
    // Temporary storage for product of kernel gradient and kernel.
    MatrixXd gKk;
    // Temporary storage for X-independent matrix.
    MatrixXd Xtemp;
    // Temporary storage for X-independent vector.
    MatrixXd XRtemp;
public:
    // Construct the kernel.
    GPCMRBFKernel(GPCMParams &params, GPCMOptimization *optimization, int dims);
    // Get inverse width of this kernel.
    double getInverseWidth();
    // Copy any settings from another kernel that we can.
    virtual void copySettings(GPCMKernel *other);
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
    virtual ~GPCMRBFKernel();
};
