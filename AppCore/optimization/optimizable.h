// Abstract optimizable object.
#pragma once

#include <Eigen/Core>

using namespace Eigen;

class GPCMOptimizable
{
protected:
public:
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm() = 0;
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient) = 0;
    // Recompute constraint, assuming temporaries are up to date.
    virtual double recomputeConstraint(bool bNeedGradient);
    // Check if a constraint exists.
    virtual bool hasConstraint();
    // Save gradient for debugging purposes.
    virtual void setDebugGradient(const VectorXd &dbg, double ll);
};
