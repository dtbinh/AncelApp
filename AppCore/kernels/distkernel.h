// Nondifferentiable RBF kernel that uses a user supplied pairwise distance matrix.
#pragma once

#include "kernel.h"
#include "options.h"

// Forward declarations.
class GPCMPrior;

class GPCMDistanceKernel : public GPCMKernel
{
protected:
    // Type of distance used.
    std::string distType;
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
    // Whether to learn scales on data matrix entries.
    bool bLearnScales;
    // Pointer to scales.
    MatrixXd *scales;
    // Pointer to scale gradients.
    MatrixXd *scaleGrads;
    // Distances matrix.
    MatrixXd dists;
    // Scaled data matrix.
    MatrixXd scaledData;
    // Data matrix to use for calculating distances matrix.
    MatrixXd *dataMatrix;
    // Temporary storage for entire RBF kernel matrix.
    MatrixXd kmat;
    // Temporary storage for product of kernel gradient and kernel.
    MatrixXd gKk;
public:
    // Construct the kernel.
    GPCMDistanceKernel(GPCMParams &params, GPCMOptimization *optimization, int dims, MatrixXd *dists);
    // Get type of distance.
    std::string &getDistanceType();
    // Set data matrix and scales.
    void setLearnScales(MatrixXd *dataMatrix, MatrixXd *scales, MatrixXd *scaleGrads);
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
    virtual ~GPCMDistanceKernel();
};
