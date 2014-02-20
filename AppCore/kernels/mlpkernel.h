// MLP kernel function.
#pragma once

#include "kernel.h"
#include "options.h"

// Forward declarations.
class GPCMPrior;

class GPCMMLPKernel : public GPCMKernel
{
protected:
    // Weight.
    MatrixXd wt;
    // Weight gradient.
    MatrixXd wtgrad;
    // Weight prior.
    GPCMPrior *wtprior;
    // Bias.
    MatrixXd bias;
    // Bias gradient.
    MatrixXd biasgrad;
    // Bias prior.
    GPCMPrior *biasprior;
    // Variance.
    MatrixXd var;
    // Variance gradient.
    MatrixXd vargrad;
    // Variance prior.
    GPCMPrior *varprior;
    // Temporary storage for inner products.
    MatrixXd innerProducts;
    // Temporary storage for argument numerator.
    MatrixXd numerator;
    // Temporary storage for argument denominator.
    MatrixXd denominator;
    // Temporary storage for cube of denominator.
    MatrixXd denominatorCubed;
    // Temporary storage for arcsine derivative.
    MatrixXd baseCovGrad;
    // Temporary storage for inner products diagonal.
    MatrixXd vec;
    // Temporary storage for arcsin argument.
    MatrixXd arg;
    // Temporary storage for kernel matrix.
    MatrixXd kmat;
public:
    // Construct the kernel.
    GPCMMLPKernel(GPCMParams &params, GPCMOptimization *optimization, int dims);
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
    virtual ~GPCMMLPKernel();
};
