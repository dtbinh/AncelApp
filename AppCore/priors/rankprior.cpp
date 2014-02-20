// Rank prior term.

#include "rankprior.h"
#include "debugprint.h"

#include <Eigen/Eigen>

#include <cmath>
// Beta value in sparsity function.
#define BETA                10.0

// Factor to multiply constraint by.
#define CONSTRAINT_FACTOR   1.0

// Constructor.
GPCMRankPrior::GPCMRankPrior(
    double weight,                          // Weight to set on term.
    MatrixXd &X,                            // Reference to latent positions.
    MatrixXd &Xgrad                         // Reference to latent position gradients.
    ) : weight(weight), X(X), Xgrad(Xgrad)
{
    // Allocate matrices.
    S.resize(X.cols());
    dS.resize(X.cols());
    U.resize(X.rows(),X.cols());
    V.resize(X.cols(),X.cols());

    // Compute initial energy for constraint.
    JacobiSVD<MatrixXd> svd(X, ComputeThinU | ComputeThinV);
    S = svd.singularValues();
    dataEnergy = S.array().square().sum();
}

// Recompute constraint, assuming temporaries are up to date.
double GPCMRankPrior::recomputeConstraint(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    double diff = S.array().square().sum() - dataEnergy;
    double constraintValue = (CONSTRAINT_FACTOR*0.25/((double)(X.rows()-1)))*pow(diff,2);

    // Compute gradient.
    if (bNeedGradient)
    {
        // Compute gradient.
        Xgrad += (diff*CONSTRAINT_FACTOR/((double)(X.rows()-1)))*(U*S.asDiagonal()*V.transpose());
    }

    return constraintValue;
}

// Recompute all stored temporaries when variables change.
double GPCMRankPrior::recompute(
    bool bNeedGradient                      // Indicates whether we need to compute the gradient.
    )
{
    double likelihood = 0.0;
    double fac = BETA/sqrt((double)(X.rows()-1)); // The square root term normalizes the singular values.

    // Compute singular value decomposition.
    JacobiSVD<MatrixXd> svd(X, ComputeThinU | ComputeThinV);

    // Add up singular value penalties.
    S = svd.singularValues();
    likelihood += -weight*(S.array().square()*fac + VectorXd::Ones(S.rows()).array()).log().sum();

	//std::cout << X << std::endl;
	assert((fabs(X(0,0)) < 1e+10));
//	std::cout << "#######################################################################" << std::endl;
//	std::cout << S << std::endl;
//	std::cout << "#######################################################################" << std::endl;
    // Get U and V matrices.
    U = svd.matrixU();
    V = svd.matrixV();

    if (bNeedGradient)
    {
        // Compute gradient with respect to singular values.
        dS = -2.0*fac*S.array()/(S.array().square()*fac + VectorXd::Ones(S.rows()).array());

        // Compute gradient of rank prior.
        Xgrad += weight*(U*dS.asDiagonal()*V.transpose());
    }

    // Return result.
    return likelihood;
}

// Destructor.
GPCMRankPrior::~GPCMRankPrior()
{
}
