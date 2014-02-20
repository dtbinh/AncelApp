// A single variable in the optimization.

#include "optvariable.h"

// Maximum allowed logarithm.
#define MAX_LOG 128.0

// Minimum allowed logarithm.
#define MIN_LOG -128.0

// Create a new variable.
GPCMOptVariable::GPCMOptVariable(
    OptVariableXform xform,                 // The transformation of the new variable.
    MatrixXd *variable,                     // Pointer to new variable data.
    MatrixXd *gradient,                     // Pointer to new variable gradient.
    int index,                              // Index in parameter vector at which variable begins.
    std::string name                        // Variable name.
    )
{
    this->xform = xform;
    this->tiedVariables = 1;
    this->data[0] = variable;
    this->gradient[0] = gradient;
    this->index = index;
    this->name = name;

    // Get number of parameters.
    this->params = data[0]->rows()*data[0]->cols();

    // Create max and min matrices.
    if (xform == VarXformExp)
    {
        minMat = MatrixXd::Constant(this->params,1,MIN_LOG);
        maxMat = MatrixXd::Constant(this->params,1,MAX_LOG);
    }
}

// Add tied variable.
void GPCMOptVariable::addTiedData(
    MatrixXd *variable,                     // Pointer to new variable data.
    MatrixXd *gradient                      // Pointer to new variable gradient.
    )
{
    assert(tiedVariables < MAX_TIED_VARS);
    this->data[tiedVariables] = variable;
    this->gradient[tiedVariables] = gradient;
    assert((*(this->data[tiedVariables])-*(this->data[0])).array().abs().maxCoeff() < 1.0e-8);
    tiedVariables++;
}

// Get pointers to variables.
MatrixXd *GPCMOptVariable::getData()
{
    return this->data[0];
}

// Get number of parameters.
int GPCMOptVariable::getParamCount()
{
    return this->params;
}

// Get starting index.
int GPCMOptVariable::getIndex()
{
    return this->index;
}

// Get name.
std::string GPCMOptVariable::getName()
{
    return this->name;
}

// Clear the gradient to be zero.
void GPCMOptVariable::clearGradient()
{
    for (int i = 0; i < tiedVariables; i++)
        this->gradient[i]->setZero(this->gradient[i]->rows(),this->gradient[i]->cols());
}

// Pack the gradient.
void GPCMOptVariable::packGradient(
    const VectorXd &params,                 // Parameter vector to read from.
    VectorXd &grad                          // Gradient vector to write to.
    )
{
    // Sum up the gradients.
    grad.block(index,0,this->params,1) = Map<MatrixXd>(this->gradient[0]->data(),this->params,1);
    for (int i = 1; i < tiedVariables; i++)
        grad.block(index,0,this->params,1) += Map<MatrixXd>(this->gradient[i]->data(),this->params,1);

    switch (xform)
    {
    case VarXformNone:
        // Nothing to do.
        break;
    case VarXformExp:
        // Multiply.
        grad.block(index,0,this->params,1) =
            grad.block(index,0,this->params,1).cwiseProduct(
                Map<MatrixXd>(this->data[0]->data(),this->params,1));
        break;
    }
}

// Pack the variable.
void GPCMOptVariable::packVariable(
    VectorXd &params                        // Parameter vector to write to.
    )
{
    switch (xform)
    {
    case VarXformNone:
        params.block(index,0,this->params,1) =
            Map<MatrixXd>(this->data[0]->data(),this->params,1);
        break;
    case VarXformExp:
        params.block(index,0,this->params,1) =
            Map<MatrixXd>(this->data[0]->data(),this->params,1).array().log();
        break;
    }
}

// Unpack the variable.
void GPCMOptVariable::unpackVariable(
    const VectorXd &params                  // Parameter vector to read from.
    )
{
    switch (xform)
    {
    case VarXformNone:
        for (int i = 0; i < tiedVariables; i++)
            Map<MatrixXd>(this->data[i]->data(),this->params,1) =
                params.block(index,0,this->params,1);
        break;
    case VarXformExp:
        Map<MatrixXd>(this->data[0]->data(),this->params,1) =
            params.block(index,0,this->params,1).array().min(maxMat.array()).max(minMat.array()).exp();
        for (int i = 1; i < tiedVariables; i++)
            *(this->data[i]) = *(this->data[0]);
        break;
    }
}
