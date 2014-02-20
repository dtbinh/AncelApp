// Gaussian process model.
#pragma once

#include "optimizable.h"
#include "options.h"
#include "kernel.h"

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;
class GPCMOptimization;
class GPCMPrior;

class GPCMGaussianProcess : public GPCMOptimizable
{
protected:
    // Matrix of data entries (before scale and bias).
    MatrixXd &dataMatrix;
    // Data matrix bias.
    MatrixXd bias;
    // Data matrix scale, so that dataMatrix = Y/scale+bias.
    MatrixXd scale;
    // Gradient of scale.
    MatrixXd scaleGrad;
    // Whether scales are learned.
    bool bLearnScales;
    // Whether scales are learned with expectation maximization.
    bool bEMScales;
    // Design matrix.
    MatrixXd &Y;
    // Input matrices.
    MatrixXd *X[MAX_KERNEL_GROUPS];
    // Gradients of input matrices.
    MatrixXd *Xgrad[MAX_KERNEL_GROUPS];
    // Number of input matrices.
    int inputMats;
    // Covariance matrix.
    MatrixXd K;
    // Inverse of the covariance matrix.
    MatrixXd invK;
    // The log determinant of K.
    double logDetK;
    // Alpha vector.
    MatrixXd alpha;
    // The current log likelihood.
    double loglikelihood;
    // Current inner products vector.
    MatrixXd innerProducts;
    // Current gradient with respect to the kernel.
    MatrixXd gK;
    // Current gradient with respect to diagonal kernel only.
    MatrixXd gKd;
    // Temporary storage for kernel gradient.
    MatrixXd gKtemp;
    // Kernel.
    GPCMKernel *kernel;
    // Prior on the scale parameters.
    GPCMPrior *scalePrior;
    // Input dimensionality.
    int q;
public:
    // Constructor, creates Gaussian process from parameters.
    GPCMGaussianProcess(GPCMParams &params, GPCMOptions &options,
        GPCMOptimization *optimization, GPCMKernel *kernel, MatrixXd &dataMatrix,
        MatrixXd &Y, MatrixXd **X, MatrixXd **Xgrad, int inputMats,
        bool bBiasData, bool bScaleData, bool bDontLearnScales = false,
        MatrixXd *dists = NULL);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMGaussianProcess *other);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute the kernel.
    virtual void recomputeKernel();
    // Recompute the likelihood.
    virtual double recomputeLikelihood(bool bNeedGradient);
    // Recompute all stored temporaries when variables change.
    // This function calls recomputeKernel and recomputeLikelihood.
    virtual double recompute(bool bNeedGradient);
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Add to the gK matrix.
    virtual void addGK(MatrixXd &addedGK);
    // Add to the gKd matrix.
    virtual void addGKDiag(MatrixXd &addedGKd);
    // Return alpha matrix.
    virtual MatrixXd &getAlpha();
    // Get scale vector.
    virtual MatrixXd &getScale();
    // Get data matrix.
    virtual MatrixXd &getDataMatrix();
    // Compute noise.
    virtual double getNoise();
    // Get kernel.
    virtual GPCMKernel *getKernel();
    // Get inverted kernel matrix.
    virtual const MatrixXd &getKinv();
    // Get X matrices array.
    virtual const MatrixXd* const *getX();
    // Get X gradients array.
    virtual MatrixXd **getXGrad();
    // Compute posterior variance at specified point.
    virtual MatrixXd posteriorVariance(const MatrixXd* const *Xstar);
    // Compute posterior mean at specified point.
    virtual MatrixXd posteriorMean(const MatrixXd* const *Xstar, MatrixXd *var,
        MatrixXd *dMdXstar = NULL, MatrixXd *dVdXstar = NULL);
    // Destructor.
    virtual ~GPCMGaussianProcess();
};
