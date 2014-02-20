// Velocity reconstruction term.
#pragma once

#include "options.h"

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;
class GPCMMatReader;
class GPCMOptimization;
class GPCMGaussianProcess;

class GPCMVelocityTerm
{
protected:
    // Whether difference mode is on.
    bool bDifference;
    // Whether midpoint mode is on.
    bool bMidpoint;
    // Latent positions matrix.
    MatrixXd &X;
    // Gradient of latent positions.
    MatrixXd &Xgrad;
    // Velocity data matrix.
    MatrixXd dataMatrix;
    // Scaled velocity data matrix.
    MatrixXd Y;
    // Sequence array.
    std::vector<int> &sequence;
    // Processed first point matrix.
    MatrixXd firstPoints;
    // Porcessed second point matrix.
    MatrixXd secondPoints;
    // Gradient of first point matrix.
    MatrixXd firstPointGrads;
    // Gradient of second point matrix.
    MatrixXd secondPointGrads;
    // Reconstruction gaussian process.
    GPCMGaussianProcess *gaussianProcess;
    // Set first and second points from current X matrix.
    virtual void setFirstSecond();
public:
    // Constructor.
    GPCMVelocityTerm(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, std::vector<int> &sequence, MatrixXd &dataMatrix);
    // Copy any settings from another model that we can.
    virtual void copySettings(GPCMVelocityTerm *other);
    // Get data matrix.
    virtual MatrixXd &getDataMatrix();
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Get pointer to underlying Gaussian process.
    GPCMGaussianProcess *getGaussianProcess();
    // Compute posterior mean velocity between specified points.
    virtual MatrixXd posteriorMean(const MatrixXd &X1, const MatrixXd &X2, MatrixXd *var,
        MatrixXd *dMdXstar = NULL, MatrixXd *dVdXstar = NULL, MatrixXd *dMdX1 = NULL);
    // Compute posterior variance between specified points.
    virtual MatrixXd posteriorVariance(const MatrixXd &X1, const MatrixXd &X2);
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMVelocityTerm();
};
