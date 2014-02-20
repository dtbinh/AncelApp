// Logarithmic prior.
#pragma once

#include "prior.h"

class GPCMLogPrior : public GPCMPrior
{
protected:
    // Coefficient in front of prior.
    double c;
public:
    // Constructor.
    GPCMLogPrior(MatrixXd *data, MatrixXd *gradient, double c);
    // Write the prior into the specific writer.
    virtual void write(GPCMMatWriter *writer);
    // Recompute gradient and return log likelihood.
    virtual double recompute();
};
