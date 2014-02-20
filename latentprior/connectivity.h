// Term that encourages connectivity.
#pragma once

#include "latentprior.h"

// List of available transition cost type.
enum CostType
{
    CostTypeGaussian,
    CostTypeInverse
};

// List of available objective types.
enum ObjectiveType
{
    ObjectiveLog,
    ObjectiveEntropy
};

class GPCMConnectivity : public GPCMLatentPrior
{
protected:
    // Whether to print kernel statistics.
    bool bPrintKernelInfo;
    // Type of transition cost to use.
    CostType costType;
    // Type of objective to use.
    ObjectiveType objectiveType;
    // Overall weight on this term.
    double weight;
    // Distance kernel width (Gaussian).
    double width;
    // Distance kernel power (inverse).
    MatrixXd power;
    // Diffusion beta value.
    double beta;
    // Temporary distance matrix.
    MatrixXd dists;
    // Temporary costs matrix.
    MatrixXd costs;
    // Temporary matrix for general use.
    MatrixXd temp;
    // Temporary storage for cost derivatives.
    MatrixXd dCdX;
    // Normalizing diagonal matrix.
    VectorXd D;
    // Inverse of normalizing diagonal matrix.
    VectorXd invD;
    // Inverse square root of normalizing diagonal matrix.
    VectorXd invSqrtD;
    // Negative graph Laplacian.
    MatrixXd H;
    // Eigenvectors of negative Laplacian.
    MatrixXd U;
    // Eigenvalues of negative Laplacian.
    VectorXd Lambda;
    // Exponentiated eigenvalues.
    VectorXd betaLambdaExp;
    // Temporary storage for Q matrix for computing kernel derivative.
    MatrixXd Q;
    // Heat kernel matrix.
    MatrixXd K;
    // Derivative of objective with respect to K.
    MatrixXd dLdK;
    // Intermediate matrix M.
    MatrixXd M;
    // Factor in front of dD/dX.
    MatrixXd MD;
    // Factor in front of dC/dX.
    MatrixXd MC;
    // Constant factors to multiply costs by to discount transitions to temporal neighbors.
    MatrixXd temporalNeighborFactor;
public:
    // Constructor.
    GPCMConnectivity(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &X, MatrixXd &Xgrad, std::vector<int> &sequence, GPCMTask *task, GPCMController *controller);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Write GP data to file.
    virtual void write(GPCMMatWriter *writer);
    // Destructor.
    virtual ~GPCMConnectivity();
};
