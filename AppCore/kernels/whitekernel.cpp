// White noise kernel function.

#include "whitekernel.h"
#include "matwriter.h"
#include "matreader.h"
#include "optimization.h"
#include "prior.h"
#include "debugprint.h"

// Construct the kernel.
GPCMWhiteKernel::GPCMWhiteKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    int dims                                // Number of input dimensions
    ) : GPCMKernel(dims), var(1,1), vargrad(1,1)
{
    constVar = atof(params["variance_const"][0].c_str());
    var << atof(params["variance"][0].c_str());
    varprior = GPCMPrior::createPrior(params["variance_prior"][0],
        atof(params["variance_prior_weight"][0].c_str()),&var,&vargrad);
    name = "white";

    // Determine if we want to learn the parameters.
    bool bLearn = true;
    if (!params["learn"].empty() && !params["learn"][0].compare("false"))
        bLearn = false;

    if (optimization && bLearn)
    {
        optimization->addVariable(VarXformExp,&var,&vargrad,"white noise variance");
    }
    this->params = 1;
    this->type = KernelTypeWhite;
}

// Copy any settings from another kernel that we can.
void GPCMWhiteKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMWhiteKernel *othercst = dynamic_cast<GPCMWhiteKernel*>(other);
        this->var = othercst->var;
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return white noise from this kernel.
double GPCMWhiteKernel::getNoise()
{
    return var(0,0)+constVar;
}

// Return covariance of a single set of points.
MatrixXd GPCMWhiteKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    return MatrixXd::Identity(X[0]->rows(),X[0]->rows())*(var(0,0)+constVar);
}

// Return coviarance of two sets of points.
MatrixXd GPCMWhiteKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    assert(X1[0] != NULL); // Make sure we have at least one element.
    assert(X2[0] != NULL); // Make sure we have at least one element.

    return MatrixXd::Zero(X1[0]->rows(),X2[0]->rows())*(var(0,0)+constVar);
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMWhiteKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    Xgrad.setZero(X[0]->rows(),dims);

    return MatrixXd::Identity(X[0]->rows(),X[0]->rows())*(var(0,0)+constVar);
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMWhiteKernel::covarianceGrad(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2,              // Second data matrix.
    MatrixXd &X2grad                        // Gradient.
    )
{
    assert(X1[0] != NULL); // Make sure we have at least one element.
    assert(X2[0] != NULL); // Make sure we have at least one element.

    X2grad.setZero(X1[0]->rows(),dims);

    return MatrixXd::Zero(X1[0]->rows(),X2[0]->rows())*(var(0,0)+constVar);
}

// Recompute priors on all kernel parameters and return the likelihood.
double GPCMWhiteKernel::recomputePriors()
{
    return varprior->recompute();
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMWhiteKernel::recompute(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    // No change to latent coordinates.
    // Compute change with respect to variance.
    vargrad(0,0) += gK.diagonal().sum();
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMWhiteKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    return MatrixXd::Identity(X[0]->rows()*X[0]->cols(),X[0]->rows()*X[0]->cols())*(var(0,0)+constVar);
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMWhiteKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(X[0] != NULL); // Make sure we have at least one element.

    // No change to latent coordinates.
    // Compute change with respect to variance.
    vargrad(0,0) += gK.diagonal().sum();
}

// Write kernel data to file.
void GPCMWhiteKernel::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    GPCMKernel::write(writer); // Let superclass write first.

    // Write parameters.
    writer->writeDouble(var(0,0)+constVar,"variance");

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
void GPCMWhiteKernel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Load superclass.
    GPCMKernel::load(reader);

    // Load variance.
    var = reader->getVariable("variance");
    var(0,0) -= constVar;
}

// Print out kernel parameters.
void GPCMWhiteKernel::printParams()
{
    DBPRINTLN("White noise variance: " << var(0,0));
}

// Destructor.
GPCMWhiteKernel::~GPCMWhiteKernel()
{
    delete varprior;
}
