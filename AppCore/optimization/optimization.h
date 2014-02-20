// The object that keeps track of optimization variables and coordinates the optimization.
#ifndef _Optimization_h_
#define _Optimization_h_
#include "optvariable.h"

#include <vector>

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMOptimizable;
class GPCMOptAlgorithm;

class GPCMOptimization
{
protected:
    // Whether to suppress printouts.
    bool bSilent;
    // Whether to validate gradients before optimization.
    bool bValidate;
    // Whether to alternate between closed form and iterative solves.
    bool bAlternating;
    // Optimization algorithm wrapper.
    GPCMOptAlgorithm *algorithm;
    // Total number of iterations.
    int maxIterations;
    // Number of outer iterations.
    int outerIterations;
    // Number of iterations elapsed.
    int iterations;
    // Optimization variables.
    std::vector<GPCMOptVariable> variables;
    // Number of parameters.
    int params;
    // Current parameter vector.
    VectorXd x;
    // Current gradient vector.
    VectorXd g;
    // Model for current optimization.
    GPCMOptimizable *model;
    // Clear all gradients to be zero.
    void clearGradients();
    // Fill all gradients from their current model values.
    void packGradients(const VectorXd &params, VectorXd &grad);
    // Fill all optimization variables from their current model values.
    void packVariables(VectorXd &params);
    // Fill all optimization variables from their current optimization values.
    void unpackVariables(const VectorXd &params);
    // Validate gradients with finite differences.
    void validateGradients(GPCMOptimizable *model);
public:
    // Constructor.
    GPCMOptimization(bool bValidate, bool bAlternating, std::string algorithm,
        int maxIterations, int outerIterations, bool bSilent = false);
    // Add a new optimization variable.
    void addVariable(OptVariableXform xform, MatrixXd *variable,
        MatrixXd *gradient, std::string name);
    // Tie additional data with existing variable.
    void tieVariable(MatrixXd *prevVariable, MatrixXd *variable,
        MatrixXd *gradient);
    // Get the dimensionality of the optimization.
    int getDims();
    // Callback to compute objective.
    double objective(int n, const double *xdata, double *grad);
    // Callback to compute constraint.
    double constraint(int n, const double *xdata, double *grad);
    // Run the optimization.
    void optimize(GPCMOptimizable *model);
    // Destructor.
    ~GPCMOptimization();
};
#endif