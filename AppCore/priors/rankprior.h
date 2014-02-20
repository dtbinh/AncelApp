// Rank prior term.
#pragma once

#include <Eigen/Core>

using namespace Eigen;

class GPCMRankPrior
{
protected:
    // Latent positions matrix.
    MatrixXd &X;
    // Gradient of latent positions.
    MatrixXd &Xgrad;
    // Energy constraint value.
    double dataEnergy;
    // Weight on rank prior.
    double weight;
    // Storage for derivative of phi with respect to singular values.
    VectorXd dS;
    // Storage for singular values.
    VectorXd S;
    // Storage for left matrix.
    MatrixXd U;
    // Storage for right matrix.
    MatrixXd V;
public:
    // Constructor.
    GPCMRankPrior(double weight, MatrixXd &X, MatrixXd &Xgrad);
    // Recompute constraint, assuming temporaries are up to date.
    virtual double recomputeConstraint(bool bNeedGradient);
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Destructor.
    virtual ~GPCMRankPrior();
};
