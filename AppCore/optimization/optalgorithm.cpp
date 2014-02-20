// Abstract wrapper around an optimization algorithm.

#include "debugprint.h"
#include "optalgorithm.h"
#include "optalgnlopt.h"

#include <string>

// Create optimization algorithm.
GPCMOptAlgorithm *GPCMOptAlgorithm::createAlgorithm(
    std::string algorithm,                  // Desired algorithm type.
    bool bSilent                            // Whether to suppress all printouts.
    )
{
    // Create optimization algorithm.
    if (!algorithm.compare("lbfgs_nlopt"))
        return new GPCMOptAlgorithmNLOPT(bSilent,GPCM_OPTNL_LBFGS);
    else if (!algorithm.compare("slsqp_nlopt"))
        return new GPCMOptAlgorithmNLOPT(bSilent,GPCM_OPTNL_SLSQP);
    else if (!algorithm.compare("auglag_nlopt"))
        return new GPCMOptAlgorithmNLOPT(bSilent,GPCM_OPTNL_AUGLAG);
    else
        DBERROR("Unknown optimization algorithm " << algorithm << " requested!");
    return NULL;
}

// Constructor.
GPCMOptAlgorithm::GPCMOptAlgorithm(
    bool bSilent                            // Whether to suppress all printouts.
    ) : bSilent(bSilent)
{
}

// Destructor.
GPCMOptAlgorithm::~GPCMOptAlgorithm()
{
}
