// Gaussian prior.
#pragma once

#include "prior.h"

class GPCMGaussPrior : public GPCMPrior
{
protected:
    // Coefficient in front of prior.
    double c;
public:
    // Constructor.
    GPCMGaussPrior(MatrixXd *data, MatrixXd *gradient, double c);
    // Write the prior into the specific writer.
    virtual void write(GPCMMatWriter *writer);
    // Recompute gradient and return log likelihood.
    virtual double recompute();
    // Get precision.
    virtual double getPrecision();
};
