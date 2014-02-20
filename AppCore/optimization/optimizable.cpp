// Abstract optimizable object.

#include "optimizable.h"

// Recompute constraint, assuming temporaries are up to date.
double GPCMOptimizable::recomputeConstraint(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    return 0.0; // No constraint by default.
}

// Check if a constraint exists.
bool GPCMOptimizable::hasConstraint()
{
    return false;
}

// Save gradient for debugging purposes.
void GPCMOptimizable::setDebugGradient(
    const VectorXd &dbg,                    // Gradient.
    double ll                               // Log likelihood.
    )
{
    // Do nothing.
}
