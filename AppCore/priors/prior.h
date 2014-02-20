// Abstract prior for an optimization variable.
#pragma once

#include <string>

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;

class GPCMPrior
{
protected:
    // Pointer to variable.
    MatrixXd *data;
    // Pointer to gradient.
    MatrixXd *gradient;
public:
    // Create a prior.
    static GPCMPrior *createPrior(std::string type, double weight, MatrixXd *data, MatrixXd *gradient);
    // Constructor.
    GPCMPrior(MatrixXd *data, MatrixXd *gradient);
    // Write the prior into the specific writer.
    virtual void write(GPCMMatWriter *writer);
    // Recompute gradient and return log likelihood.
    virtual double recompute() = 0;
};
