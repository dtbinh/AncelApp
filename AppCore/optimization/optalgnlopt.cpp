// Wrapper for NLopt optimization routines.

#include "debugprint.h"
#include "optalgnlopt.h"

#ifndef NO_F2C
#include "nlopt.hpp"
#endif

// Tolerance.
#define X_TOLERANCE     1.0e-4

// Constructor.
GPCMOptAlgorithmNLOPT::GPCMOptAlgorithmNLOPT(
    bool bSilent,                           // Whether to suppress all printouts.
    GPCMNLOPTMode mode                      // Optimization mode.
    ) : GPCMOptAlgorithm(bSilent)
{
#ifndef NO_F2C
    this->mode = mode;
    this->opt = NULL;
    this->local_opt = NULL;
#endif
}

// Set up optimization parameters.
void GPCMOptAlgorithmNLOPT::initialize(
    int params,                             // Number of optimization parameters.
    int iterations,                         // Desired number of iterations.
    GPCMOptCallback callback,               // Callback function.
    GPCMOptCallback constraintCallback,     // Constraint callback function.
    void *callbackPtr                       // Data to pass to callback.
    )
{
#ifndef NO_F2C
    // Delete old parameters and reinitialize input vector.
    if (opt) delete opt;
    inputVector.clear();
    inputVector = std::vector<double>(params);

    // Create NLopt optimization object and pass in callback.
    switch (this->mode)
    {
    case GPCM_OPTNL_LBFGS:
        opt = new nlopt::opt(nlopt::LD_LBFGS_NOCEDAL,params);
        break;
    case GPCM_OPTNL_SLSQP:
        opt = new nlopt::opt(nlopt::LD_SLSQP,params);
	    break;
    case GPCM_OPTNL_AUGLAG:
        opt = new nlopt::opt(nlopt::LD_AUGLAG,params);
        // Set local optimizer.
		local_opt = new nlopt::opt(nlopt::LD_LBFGS_NOCEDAL,params);
        local_opt->set_max_objective(callback,callbackPtr);
        local_opt->set_xtol_rel(X_TOLERANCE);
        if (constraintCallback)
            local_opt->set_maxeval(iterations/20);
        else
            local_opt->set_maxeval(iterations);
        opt->set_local_optimizer(*local_opt);
        // Set constraints.
        if (constraintCallback)
        {
            double tolerance = 0;
            opt->add_equality_constraint(constraintCallback,callbackPtr,tolerance);
        }
        break;
    }
	opt->set_max_objective(callback,callbackPtr);
    
    // Set options (tolerances, etc.).
    opt->set_xtol_rel(X_TOLERANCE);
    opt->set_maxeval(iterations);
#endif
}

// Set up starting point.
void GPCMOptAlgorithmNLOPT::setStart(
    const VectorXd &start                   // Optimization starting point.
    )
{
#ifndef NO_F2C
    // Stuff input into a vector.
    for (int i = 0; i < start.rows(); i++)
        inputVector[i] = start(i);
#endif
}

// Run the optimization return the result.
void GPCMOptAlgorithmNLOPT::run(
    VectorXd &result                        // Result to return.
    )
{
#ifndef NO_F2C
    nlopt::result res;
    double val = 0.0;

    // Run the optimization.
    try
    {
        res = opt->optimize(inputVector,val);
    }
    catch (std::runtime_error &e)
    {
        DBERROR("Optimization returned: " << e.what());
        //throw(e);
    }

    // Read back result.
    for (int i = 0; i < result.rows(); i++)
        result(i) = inputVector[i];
#endif
}

// Destructor.
GPCMOptAlgorithmNLOPT::~GPCMOptAlgorithmNLOPT()
{
#ifndef NO_F2C
    delete opt;
    if (local_opt) delete local_opt;
#endif
}
