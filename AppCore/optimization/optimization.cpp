// The object that keeps track of optimization variables and coordinates the optimization.

#include "debugprint.h"
#include "optimization.h"
#include "optimizable.h"
#include "optalgnlopt.h"

#include <iomanip>
#include <boost/lexical_cast.hpp>

// Small value for taking finite differences.
#define FD_EPSILON      1e-6

// Optimization callback.
double objectiveCallback(
    unsigned n,                             // Dimensionality of problem.
    const double *xdata,                    // Pointer to new parameter values.
    double *grad,                           // Pre-allocated buffer to return data to.
    void *opt                               // Pointer to optimization object.
    )
{
    // Simply call the optimization object.
    return ((GPCMOptimization*)opt)->objective(n,xdata,grad);
}

// Constraint callback.
double constraintCallback(
    unsigned n,                             // Dimensionality of problem.
    const double *xdata,                    // Pointer to new parameter values.
    double *grad,                           // Pre-allocated buffer to return data to.
    void *opt                               // Pointer to optimization object.
    )
{
    // Simply call the optimization object.
    return ((GPCMOptimization*)opt)->constraint(n,xdata,grad);
}

// Clear all gradients to be zero.
void GPCMOptimization::clearGradients()
{
    for (std::vector<GPCMOptVariable>::iterator itr = variables.begin();
         itr != variables.end(); ++itr)
    {
        itr->clearGradient();
    }
}

// Fill all gradients from their current model values.
void GPCMOptimization::packGradients(
    const VectorXd &params,                 // Parameter vector to read from.
    VectorXd &grad                          // Gradient vector to write to.
    )
{
    for (std::vector<GPCMOptVariable>::iterator itr = variables.begin();
         itr != variables.end(); ++itr)
    {
        itr->packGradient(params,grad);
    }
}

// Fill all optimization variables from their current model values.
void GPCMOptimization::packVariables(
    VectorXd &params                        // Parameter vector to write to.
    )
{
    for (std::vector<GPCMOptVariable>::iterator itr = variables.begin();
         itr != variables.end(); ++itr)
    {
        itr->packVariable(params);
    }
}

// Fill all optimization variables from their current optimization values.
void GPCMOptimization::unpackVariables(
    const VectorXd &params                  // Parameter vector to read from.
    )
{
    for (std::vector<GPCMOptVariable>::iterator itr = variables.begin();
         itr != variables.end(); ++itr)
    {
        itr->unpackVariable(params);
    }
}

// Constructor.
GPCMOptimization::GPCMOptimization(
    bool bValidate,                         // Whether to validate gradients with finite differences.
    bool bAlternating,                      // Whether to alternate between iterative and closed form solves.
    std::string algorithm,                  // Desired optimization algorithm.
    int maxIterations,                      // How many iterations to optimize for.
    int outerIterations,                    // How many outer iterations to use, if applicable.
    bool bSilent                            // Whether to suppress printouts.
    ) : bValidate(bValidate), bAlternating(bAlternating), bSilent(bSilent),
        maxIterations(maxIterations), outerIterations(outerIterations), params(0)
{
    // Create algorithm.
    this->algorithm = GPCMOptAlgorithm::createAlgorithm(algorithm,bSilent);
}

// Add a new optimization variable.
void GPCMOptimization::addVariable(
    OptVariableXform xform,                 // The transformation of the new variable.
    MatrixXd *variable,                     // Pointer to new variable data.
    MatrixXd *gradient,                     // Pointer to new variable gradient.
    std::string name                        // Name of new variable.
    )
{
    variables.push_back(GPCMOptVariable(xform,variable,gradient,params,name));
    params += variables.back().getParamCount();
}

// Tie additional data with existing variable.
void GPCMOptimization::tieVariable(
    MatrixXd *prevVariable,                 // Pointer to previous variable to tie to.
    MatrixXd *variable,                     // Pointer to new variable data.
    MatrixXd *gradient                      // Pointer to new variable gradient.
    )
{
    for (unsigned i = 0; i < variables.size(); i++)
    {
        if (variables[i].getData() == prevVariable)
        {
            variables[i].addTiedData(variable,gradient);
            break;
        }
    }
}

// Get the dimensionality of the optimization.
int GPCMOptimization::getDims()
{
    return params;
}

// Callback to compute objective.
double GPCMOptimization::objective(
    int n,                                  // Dimensionality of problem (should match!).
    const double *xdata,                    // Pointer to new parameter values.
    double *grad                            // Pre-allocated buffer to return data to.
    )
{
    // Make sure dimensionality matches.
    assert(params == n);

    // Copy out the parameter values.
    memcpy(x.data(),xdata,sizeof(double)*params);

    // Unpack variables.
    unpackVariables(x);

    // Zero out the gradient.
    if (grad)
        clearGradients();

    // Compute gradients and objective.
    double ll = model->recompute(grad != NULL);

    if (grad)
    { // Only do this if grad is not NULL.
        // Pack gradients.
        packGradients(x,g);

        // Copy over the gradients.
        memcpy(grad,g.data(),sizeof(double)*params);

        // Print iteration.
        iterations++;
        if (!bSilent)
            DBPRINTLN("Evaluation " << iterations << ": " << ll);
    }
    else if (!bSilent)
    {
        DBPRINTLN("Linesearch: " << ll);
    }

    return ll;
}

// Callback to compute constraint.
double GPCMOptimization::constraint(
    int n,                                  // Dimensionality of problem (should match!).
    const double *xdata,                    // Pointer to new parameter values.
    double *grad                            // Pre-allocated buffer to return data to.
    )
{
    // Make sure dimensionality matches.
    assert(params == n);

    // Make sure parameters match.
    bool bXChanged = !(Map<VectorXd>((double*)xdata,params,1).array() == x.array()).all();

    if (bXChanged)
    { // If parameters changed, must update the model.
        DBWARNING("Constraint function called with X vector that differs from last objective call, reevaluating.");

        // Copy out the parameter values.
        memcpy(x.data(),xdata,sizeof(double)*params);

        // Unpack variables.
        unpackVariables(x);
        
        // Update the model.
        model->recompute(false);
    }

    // Zero out the gradient.
    if (grad)
        clearGradients();

    // Compute gradients and constraint value.
    double constraintValue = model->recomputeConstraint(grad != NULL);

    if (grad)
    { // Only do this if grad is not NULL.
        // Pack gradients.
        packGradients(x,g);

        // Copy over the gradients.
        memcpy(grad,g.data(),sizeof(double)*params);

        // Print message
        if (!bSilent)
            DBPRINTLN("Constraint gradient evaluation: " << constraintValue);
    }
    else
    {
        if (!bSilent)
            DBPRINTLN("Constraint value evaluation: " << constraintValue);
    }

    return constraintValue;
}

// Run the optimization.
void GPCMOptimization::optimize(
    GPCMOptimizable *model                  // Model to optimize.
    )
{
    if (maxIterations <= 0) return; // Nothing to optimize.

    // Create initial value.
    x.resize(params);
    g.resize(params);
    packVariables(x);
    iterations = 0;
	 
    // Optionally validate gradients.
    if (bValidate)
        validateGradients(model);

    // Store model.
    this->model = model;

    // Perform optimization.
    int outerIterations = 1;
    if (bAlternating)
        outerIterations = this->outerIterations; // If alternating, use a number of outer iterations.
    for (int itr = 0; itr < outerIterations; itr++)
    {
        // Run optimization.
        if (model->hasConstraint())
            algorithm->initialize(params,maxIterations,
                objectiveCallback,constraintCallback,this);
        else
            algorithm->initialize(params,maxIterations,
                objectiveCallback,NULL,this);
        algorithm->setStart(x);
        algorithm->run(x);

        // Read back result.
        unpackVariables(x);

        if (bAlternating)
        { // If doing alternating optimization, perform maximization here.
            model->recompute(false);
            model->recomputeClosedForm();
            packVariables(x);
        }
    }

    clearGradients();
    double l = model->recompute(true);
    packGradients(x,g);
    model->setDebugGradient(g,l);
    if (!bSilent)
        DBPRINTLN("Final likelihood: " << l);
    if (bValidate) validateGradients(model);
    return;
}

// Validate gradients with finite differences.
void GPCMOptimization::validateGradients(
    GPCMOptimizable *model                  // Model to validate gradients for.
    )
{
    // Compute gradient.
    clearGradients();
    double center = model->recompute(true);
    double centerc = 0.0;
    packGradients(x,g);

//	std::cout << x << std::endl;
    // Optionally compute the constraint gradient.
    VectorXd cg(g.rows());
    if (model->hasConstraint())
    {
        clearGradients();
        centerc = model->recomputeConstraint(true);
        packGradients(x,cg);
    }

    // Take samples to evaluate finite differences.
    VectorXd pt = x;
    VectorXd fdg(params);
    VectorXd fdgc(params);
    for (int i = 0; i < params; i++)
    {
        // Evaluate upper and lower values.
        pt.noalias() = x + VectorXd::Unit(params,i)*FD_EPSILON;
        unpackVariables(pt);
        double valp = model->recompute(false);
        double valpc = model->recomputeConstraint(false);
        pt.noalias() = x - VectorXd::Unit(params,i)*FD_EPSILON;
        unpackVariables(pt);
        double valm = model->recompute(false);
        double valmc = model->recomputeConstraint(false);
        fdg(i) = 0.5*(valp-valm)/FD_EPSILON;
        fdgc(i) = 0.5*(valpc-valmc)/FD_EPSILON;
        DBPRINTLN("Computed finite difference for dimension " << i << " of " << params << ": " << fdg(i));
    }
//	std::cout << x << std::endl;
    // Reset variables.
    unpackVariables(x);

    // Construct gradient names.
    std::vector<std::string> varname(x.rows());
    for (std::vector<GPCMOptVariable>::iterator itr = variables.begin();
         itr != variables.end(); itr++)
    {
        for (int i = itr->getIndex(); i < itr->getIndex()+itr->getParamCount(); i++)
        {
            varname[i] = itr->getName();
            if (itr->getParamCount() > 1)
                varname[i] += std::string(" ") +
                    boost::lexical_cast<std::string>(i-itr->getIndex());
        }
    }

    // Print gradients.
    int idx;
    DBPRINTLN("True gradient / finite-difference gradient:");
    for (int i = 0; i < params; i++)
    {
        if (model->hasConstraint())
        {
            DBPRINTLN(std::setw(10) << g(i) << " " <<
                      std::setw(10) << fdg(i) <<
                      std::setw(10) << cg(i) << " " <<
                      std::setw(10) << fdgc(i) <<
                      std::setw(10) << "(" << x(i) << ")" << "   " << varname[i]);
        }
        else
        {
            DBPRINTLN(std::setw(10) << g(i) << " " <<
                      std::setw(10) << fdg(i) <<
                      std::setw(10) << "(" << x(i) << ")" << "   " << varname[i]);
        }
    }

    // Check objective gradient.
    double maxDiff = (g-fdg).array().abs().matrix().maxCoeff(&idx);
    if (maxDiff >= 0.1)
        DBWARNING("Gradients appear significantly different!");
    DBPRINTLN("Max difference: " << maxDiff);
    DBPRINTLN("Max difference at index " << idx << ":" << std::endl << std::setw(10) << g(idx)
        << " " << std::setw(10) << fdg(idx) << "   " << varname[idx]);

    if (model->hasConstraint())
    {
        // Check constraint gradient.
        maxDiff = (cg-fdgc).array().abs().matrix().maxCoeff(&idx);
        if (maxDiff >= 0.1)
            DBWARNING("Constraint gradients appear significantly different!");
        DBPRINTLN("Max constraint difference: " << maxDiff);
        DBPRINTLN("Max constraint difference at index " << idx << ":" << std::endl << std::setw(10) << cg(idx)
            << " " << std::setw(10) << fdgc(idx) << "   " << varname[idx]);
    }
}

// Destructor.
GPCMOptimization::~GPCMOptimization()
{
    delete algorithm;
}
