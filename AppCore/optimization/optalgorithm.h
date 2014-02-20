// Abstract wrapper around an optimization algorithm.
#pragma once

#include <Eigen/Core>

using namespace Eigen;

// Callback function.
typedef double (*GPCMOptCallback) (unsigned n, const double *x, double *g, void *ptr);

class GPCMOptAlgorithm
{
protected:
    // Whether to suppress all printouts.
    bool bSilent;
public:
    // Create optimization algorithm.
    static GPCMOptAlgorithm *createAlgorithm(std::string algorithm, bool bSilent = false);
    // Constructor.
    GPCMOptAlgorithm(bool bSilent);
    // Set up optimization parameters.
    virtual void initialize(int params, int iterations,
        GPCMOptCallback callback, GPCMOptCallback constraintCallback,
        void *callbackPtr) = 0;
    // Set up starting point.
    virtual void setStart(const VectorXd &start) = 0;
    // Run the optimization return the result.
    virtual void run(VectorXd &result) = 0;
    // Destructor.
    virtual ~GPCMOptAlgorithm();
};
