// Wrapper for NLopt optimization routines.
#pragma once

#include "optalgorithm.h"

#include <vector>

// Possible modes for NLopt optimization.
enum GPCMNLOPTMode
{
    GPCM_OPTNL_LBFGS,
    GPCM_OPTNL_SLSQP,
    GPCM_OPTNL_AUGLAG
};

// Forward declarations.
namespace nlopt { class opt; }

class GPCMOptAlgorithmNLOPT : public GPCMOptAlgorithm
{
protected:
#ifndef NO_F2C
    // Desired optimizer type.
    GPCMNLOPTMode mode;
    // Pointer to currently active optimization.
    nlopt::opt *opt;
    // Pointer to currently active local optimization.
    nlopt::opt *local_opt;
    // Current input vector.
    std::vector<double> inputVector;
#endif
public:
    // Constructor.
    GPCMOptAlgorithmNLOPT(bool bSilent, GPCMNLOPTMode mode);
    // Set up optimization parameters.
    virtual void initialize(int params, int iterations,
        GPCMOptCallback callback, GPCMOptCallback constraintCallback,
        void *callbackPtr);
    // Set up starting point.
    virtual void setStart(const VectorXd &start);
    // Run the optimization return the result.
    virtual void run(VectorXd &result);
    // Destructor.
    virtual ~GPCMOptAlgorithmNLOPT();
};
