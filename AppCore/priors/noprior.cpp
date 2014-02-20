// No prior.

#include "noprior.h"

// Constructor.
GPCMNoPrior::GPCMNoPrior(
    MatrixXd *data,                         // Variable to put a prior on.
    MatrixXd *gradient                      // Where to store the prior gradient.
    ) : GPCMPrior(data,gradient)
{
}

// Recompute gradient and return log likelihood.
double GPCMNoPrior::recompute()
{
    return 0;
}
