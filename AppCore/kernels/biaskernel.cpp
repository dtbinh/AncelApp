// Bias kernel function.

#include "biaskernel.h"
#include "matwriter.h"
#include "matreader.h"
#include "optimization.h"
#include "prior.h"
#include "debugprint.h"

// Construct the kernel.
GPCMBiasKernel::GPCMBiasKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    int dims                                // Number of input dimensions
    ) : GPCMKernel(dims), var(1,1), vargrad(1,1)
{
    var << atof(params["variance"][0].c_str());
    varprior = GPCMPrior::createPrior(params["variance_prior"][0],
        atof(params["variance_prior_weight"][0].c_str()),&var,&vargrad);
    name = "bias";
    if (optimization)
    {
        optimization->addVariable(VarXformExp,&var,&vargrad,"kernel bias");
    }
    this->params = 1;
    this->type = KernelTypeBias;
}

// Copy any settings from another kernel that we can.
void GPCMBiasKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMBiasKernel *othercst = dynamic_cast<GPCMBiasKernel*>(other);
        this->var = othercst->var;
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return covariance of a single set of points.
MatrixXd GPCMBiasKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    return MatrixXd::Constant(X[0]->rows(),X[0]->rows(),var(0,0));
}

// Return coviarance of two sets of points.
MatrixXd GPCMBiasKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    assert(X1[0] != NULL); // Make sure we have at least one element.
    assert(X2[0] != NULL); // Make sure we have at least one element.

    return MatrixXd::Constant(X1[0]->rows(),X2[0]->rows(),var(0,0));
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMBiasKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    Xgrad.setZero(X[0]->rows(),dims);

    return MatrixXd::Constant(X[0]->rows(),X[0]->rows(),var(0,0));
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMBiasKernel::covarianceGrad(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2,              // Second data matrix.
    MatrixXd &X2grad                        // Gradient.
    )
{
    assert(X1[0] != NULL); // Make sure we have at least one element.
    assert(X2[0] != NULL); // Make sure we have at least one element.

    X2grad.setZero(X1[0]->rows(),dims);

    return MatrixXd::Constant(X1[0]->rows(),X2[0]->rows(),var(0,0));
}

// Recompute priors on all kernel parameters and return the likelihood.
double GPCMBiasKernel::recomputePriors()
{
    return varprior->recompute();
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMBiasKernel::recompute(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    // No change to latent coordinates.
    // Compute change with respect to variance.
    vargrad(0,0) += gKd.sum();
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMBiasKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    return MatrixXd::Constant(X[0]->rows()*X[0]->cols(),X[0]->rows()*X[0]->cols(),var(0,0));
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMBiasKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    // No change to latent coordinates.
    // Compute change with respect to variance.
    vargrad(0,0) += gKd.sum();
}

// Write kernel data to file.
void GPCMBiasKernel::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    GPCMKernel::write(writer); // Let superclass write first.

    // Write parameters.
    writer->writeDouble(var(0,0),"variance");

    // Write priors.
    GPCMMatWriter *cellWriter = writer->writeCell("priors",1,1);
    GPCMMatWriter *priorStruct = cellWriter->writeStruct("",1,1);
    priorStruct->writeDouble(1.0,"index");
    varprior->write(priorStruct);
    cellWriter->closeStruct();
    writer->closeCell();

    // Write transforms struct.
    GPCMMatWriter *xformStruct = writer->writeStruct("transforms",1,1);
    xformStruct->writeDouble(1.0,"index");
    xformStruct->writeString("exp","type");
    writer->closeStruct();
}

// Load kernel from specified MAT file reader.
void GPCMBiasKernel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Load superclass.
    GPCMKernel::load(reader);

    // Load variance.
    var = reader->getVariable("variance");
}

// Print out kernel parameters.
void GPCMBiasKernel::printParams()
{
    DBPRINTLN("Bias variance: " << var(0,0));
}

// Destructor.
GPCMBiasKernel::~GPCMBiasKernel()
{
    delete varprior;
}
