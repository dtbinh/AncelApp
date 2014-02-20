// No prior.
#pragma once

#include "prior.h"

class GPCMNoPrior : public GPCMPrior
{
protected:
public:
    // Constructor.
    GPCMNoPrior(MatrixXd *data, MatrixXd *gradient);
    // Recompute gradient and return log likelihood.
    virtual double recompute();
};
