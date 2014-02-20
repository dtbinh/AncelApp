// Abstract kernel function.
#pragma once

#include "options.h"

#include <string>


#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;
class GPCMMatReader;
class GPCMOptimization;

// Possible types of kernels.
enum GPCMKernelType
{
    KernelTypeUnknown,
    KernelTypeDist,
    KernelTypeCompound,
    KernelTypeTensor,
    KernelTypeWhite,
    KernelTypeBias,
    KernelTypeLinear,
    KernelTypeRBF,
    KernelTypeMLP
};

// Maximum number of channel groups in a kernel (for tensor products).
#define MAX_KERNEL_GROUPS 4

class GPCMKernel
{
protected:
    // Kernel name for export.
    std::string name;
    // Number of parameters for export.
    int params;
    // Number of input dimensions.
    int dims;
    // The type of this kernel.
    GPCMKernelType type;
public:
    // Global function for creating a new kernel.
    static GPCMKernel *createKernel(GPCMParams &params, GPCMOptions &options,
        GPCMOptimization *optimization, int dims, MatrixXd *dists = NULL);
    // Construct the kernel.
    GPCMKernel(int dims);
    // Copy any settings from another kernel that we can.
    virtual void copySettings(GPCMKernel *other);
    // Get the type of this kernel.
    virtual GPCMKernelType getType();
    // Return the number of dimensions the kernel can handle.
    virtual int getDims();
    // Return white noise from this kernel.
    virtual double getNoise();
    // Return covariance of a single set of points.
    virtual MatrixXd covariance(const MatrixXd* const *X) = 0;
    // Return coviarance of two sets of points.
    virtual MatrixXd covariance(const MatrixXd* const *X1, const MatrixXd* const *X2) = 0;
    // Return covariance of a single set of points and the gradient.
    virtual MatrixXd covarianceGrad(const MatrixXd* const *X, MatrixXd &Xgrad) = 0;
    // Return coviarance of two sets of points.
    virtual MatrixXd covarianceGrad(const MatrixXd* const *X1, const MatrixXd* const *X2, MatrixXd &X2grad) = 0;
    // Recompute priors on all kernel parameters and return the likelihood.
    virtual double recomputePriors() = 0;
    // Recompute gradients of hyperparameters and latent coordinates.
    virtual void recompute(const MatrixXd &gK, const MatrixXd &gKd, const MatrixXd* const *X, MatrixXd **Xgrad) = 0;
    // Return covariance of a single set of points in gradient observation mode.
    virtual MatrixXd covarianceGradientGP(const MatrixXd* const *X) = 0;
    // Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
    virtual void recomputeGradientGP(const MatrixXd &gK, const MatrixXd &gKd, const MatrixXd* const *X, MatrixXd **Xgrad) = 0;
    // Get number of parameters.
    int getParamCount();
    // Write kernel data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load kernel from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Print out kernel parameters.
    virtual void printParams();
    // Destructor.
    virtual ~GPCMKernel();
};
