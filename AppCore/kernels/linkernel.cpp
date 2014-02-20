// Linear kernel function.

#include "linkernel.h"
#include "mathutils.h"
#include "matwriter.h"
#include "matreader.h"
#include "optimization.h"
#include "prior.h"
#include "debugprint.h"

// Construct the kernel.
GPCMLinearKernel::GPCMLinearKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    int dims                                // Number of input dimensions
    ) : GPCMKernel(dims), var(1,1), vargrad(1,1)
{
    // Construct standard parameters.
    var << atof(params["variance"][0].c_str());
    varprior = GPCMPrior::createPrior(params["variance_prior"][0],
        atof(params["variance_prior_weight"][0].c_str()),&var,&vargrad);

    name = "lin";
    if (optimization)
    {
        optimization->addVariable(VarXformExp,&var,&vargrad,"Linear kernel variance");
    }
    this->params = 1;
    this->type = KernelTypeLinear;
}

// Copy any settings from another kernel that we can.
void GPCMLinearKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMLinearKernel *othercst = dynamic_cast<GPCMLinearKernel*>(other);
        this->var = othercst->var;
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return covariance of a single set of points.
MatrixXd GPCMLinearKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.

    kmat.noalias() = (*X[0])*(X[0]->transpose());
    return var(0,0)*kmat;
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMLinearKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.

    kmat.noalias() = (*X[0])*(X[0]->transpose());

    // Compute gradient.
    // Note that this is the diagonal gradient.
    Xgrad = (2.0*var(0,0))*(*X[0]);

    // Return result.
    return var(0,0)*kmat;
}

// Return coviarance of two sets of points.
MatrixXd GPCMLinearKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    assert(X1[0] != NULL && X1[1] == NULL); // Make sure we have only one element.
    assert(X2[0] != NULL && X2[1] == NULL); // Make sure we have only one element.

    return var(0,0)*(*X1[0])*(X2[0]->transpose());
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMLinearKernel::covarianceGrad(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2,              // Second data matrix.
    MatrixXd &X2grad                        // Gradient.
    )
{
    assert(X1[0] != NULL && X1[1] == NULL); // Make sure we have only one element.
    assert(X2[0] != NULL && X2[1] == NULL); // Make sure we have only one element.
    assert(X2[0]->rows() == 1); // This only supports single query points.

    // Compute gradient.
    X2grad = var(0,0)*(*X1[0]);

    // Return result.
    return var(0,0)*(*X1[0])*(X2[0]->transpose());
}

// Recompute priors on all kernel parameters and return the likelihood.
double GPCMLinearKernel::recomputePriors()
{
    return varprior->recompute();
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMLinearKernel::recompute(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.
    assert(Xgrad[1] == NULL); // Make sure we have only one element.

    // Constants.
    int N = gKd.rows();

    // Compute hyperparameter gradients.
    vargrad(0,0) += (kmat.cwiseProduct(gKd)).sum();

    if (Xgrad[0])
    {
        // Next, compute the gradient for each dimension of X.
        for (int d = 0; d < X[0]->cols(); d++)
        {
            Xgrad[0]->col(d) += (2.0*var(0,0)*(X[0]->col(d).transpose().replicate(N,1).cwiseProduct(gKd))).rowwise().sum();
        }
    }
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMLinearKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(false && "Unsupported");
    return var;
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMLinearKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(false && "Unsupported");
}

// Write kernel data to file.
void GPCMLinearKernel::write(
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
void GPCMLinearKernel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Load superclass.
    GPCMKernel::load(reader);

    // Load variance.
    var = reader->getVariable("variance");
}

// Print out kernel parameters.
void GPCMLinearKernel::printParams()
{
    DBPRINTLN("Linear variance: " << var(0,0));
}

// Destructor.
GPCMLinearKernel::~GPCMLinearKernel()
{
    delete varprior;
}
