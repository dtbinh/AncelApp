// Nondifferentiable RBF kernel that uses a user supplied pairwise distance matrix.

#include "distkernel.h"
#include "mathutils.h"
#include "matwriter.h"
#include "matreader.h"
#include "optimization.h"
#include "prior.h"
#include "debugprint.h"

// Construct the kernel.
GPCMDistanceKernel::GPCMDistanceKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    int dims,                               // Number of input dimensions
    MatrixXd *dists                         // Distances matrix
    ) : GPCMKernel(dims), iw(1,1), var(1,1), iwgrad(1,1), vargrad(1,1)
{
    // Copy the desired distances matrix.
    if (!params["distance_type"][0].compare("temporal"))
        this->dists = dists[0];
    else if (!params["distance_type"][0].compare("pose"))
        this->dists = dists[1];
    distType = params["distance_type"][0];

    // Construct standard parameters.
    iw << atof(params["inverse_width"][0].c_str());
    var << atof(params["variance"][0].c_str());
    iwprior = GPCMPrior::createPrior(params["inverse_width_prior"][0],
        atof(params["inverse_width_prior_weight"][0].c_str()),&iw,&iwgrad);
    varprior = GPCMPrior::createPrior(params["variance_prior"][0],
        atof(params["variance_prior_weight"][0].c_str()),&var,&vargrad);

    // Don't learn the scales for now.
    bLearnScales = false;

    name = "dist";
    if (optimization)
    {
        optimization->addVariable(VarXformExp,&iw,&iwgrad,"Distance kernel inverse width");
        optimization->addVariable(VarXformExp,&var,&vargrad,"Distance kernel variance");
    }
    this->params = 2;
    this->type = KernelTypeDist;
}

// Get type of distance.
std::string &GPCMDistanceKernel::getDistanceType()
{
    return distType;
}

// Set data matrix and scales.
void GPCMDistanceKernel::setLearnScales(
    MatrixXd *dataMatrix,                   // Data matrix to use.
    MatrixXd *scales,                       // Scale matrix to use.
    MatrixXd *scaleGrads                    // Gradients of scale matrix.
    )
{
    this->dataMatrix = dataMatrix;
    this->scales = scales;
    this->scaleGrads = scaleGrads;
    this->bLearnScales = true;
}

// Copy any settings from another kernel that we can.
void GPCMDistanceKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMDistanceKernel *othercst = dynamic_cast<GPCMDistanceKernel*>(other);
        this->iw = othercst->iw;
        this->var = othercst->var;
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return covariance of a single set of points.
MatrixXd GPCMDistanceKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.

    // Verify that X is what we expect.
    if (X[0]->rows() == dists.rows() && X[0]->cols() == dims)
    {
        if (bLearnScales)
        { // Recompute distances.
            scaledData = dataMatrix->cwiseProduct(scales->replicate(dataMatrix->rows(),1));
            dists = pairwiseDistance(scaledData,scaledData);
        }

        kmat.noalias() = var(0,0)*((dists*(-0.5*iw(0,0))).array()).exp().matrix();
        return kmat;
    }
    else
    { // Otherwise, check if we are being passed in a distance matrix directly.
        assert(X[0]->rows() == X[0]->cols());
        return var(0,0)*(((*X[0])*(-0.5*iw(0,0))).array()).exp().matrix();
    }
}

// Return coviarance of two sets of points.
MatrixXd GPCMDistanceKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    assert(X1[0] != NULL && X1[1] == NULL); // Make sure we have only one element.
    assert(X2[0] != NULL && X2[1] == NULL); // Make sure we have only one element.

    // Make sure we are being passed in distance matrices.
    assert(X1[0]->rows() == X2[0]->cols() && X1[0]->cols() == X2[0]->rows());
    return var(0,0)*(((*X1[0])*(-0.5*iw(0,0))).array()).exp().matrix();
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMDistanceKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    assert(false && "Unsupported!");

    // Return result.
    return MatrixXd();
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMDistanceKernel::covarianceGrad(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2,              // Second data matrix.
    MatrixXd &X2grad                        // Gradient.
    )
{
    assert(false && "Unsupported!");

    // Return result.
    return MatrixXd();
}

// Recompute priors on all kernel parameters and return the likelihood.
double GPCMDistanceKernel::recomputePriors()
{
    return varprior->recompute()+iwprior->recompute();
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMDistanceKernel::recompute(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.
    assert(Xgrad[1] == NULL); // Make sure we have only one element.
    assert(Xgrad[0] == NULL); // Use of this kernel under variable inputs is impossible.

    // Constants.
    int N = gKd.rows();

    // Compute hyperparameter gradients.
    gKk.noalias() = gKd.cwiseProduct(kmat);
    iwgrad(0,0) += -0.5*gKk.cwiseProduct(dists).sum();
    vargrad(0,0) += (1.0/var(0,0))*gKk.sum();

    // If necessary, compute scale gradients.
    if (bLearnScales)
    {
        for (int i = 0; i < dataMatrix->cols(); i++)
        {
            (*scaleGrads)(0,i) += (*scales)(0,i)*(-((gKk*(dataMatrix->col(i).array().square().matrix())).sum() -
                                                    (dataMatrix->col(i).transpose()*gKk*dataMatrix->col(i))(0,0))*(2.0*iw(0,0)));
        }
    }
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMDistanceKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(false && "Unsupported!");

    return MatrixXd();
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMDistanceKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(false && "Unsupported!");
}

// Write kernel data to file.
void GPCMDistanceKernel::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    GPCMKernel::write(writer); // Let superclass write first.

    // Write parameters.
    writer->writeDouble(iw(0,0),"inverseWidth");
    writer->writeDouble(var(0,0),"variance");

    // Write priors.
    GPCMMatWriter *cellWriter = writer->writeCell("priors",1,2);
    GPCMMatWriter *priorStruct = cellWriter->writeStruct("",1,1);
    priorStruct->writeDouble(1.0,"index");
    iwprior->write(priorStruct);
    priorStruct = cellWriter->closeStruct();
    priorStruct = cellWriter->writeStruct("",1,1);
    priorStruct->writeDouble(2.0,"index");
    varprior->write(priorStruct);
    cellWriter->closeStruct();
    writer->closeCell();

    // Write transforms struct.
    GPCMMatWriter *xformStruct = writer->writeStruct("transforms",1,2);
    xformStruct->writeDouble(1.0,"index");
    xformStruct->writeString("exp","type");
    xformStruct = writer->closeStruct();
    xformStruct->writeDouble(2.0,"index");
    xformStruct->writeString("exp","type");
    writer->closeStruct();
}

// Load kernel from specified MAT file reader.
void GPCMDistanceKernel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Load superclass.
    GPCMKernel::load(reader);

    // Load variance.
    var = reader->getVariable("variance");

    // Load inverse width.
    iw = reader->getVariable("inverseWidth");
}

// Print out kernel parameters.
void GPCMDistanceKernel::printParams()
{
    DBPRINTLN("RBF dist variance: " << var(0,0));
    DBPRINTLN("RBF dist inverse width: " << iw(0,0));
}

// Destructor.
GPCMDistanceKernel::~GPCMDistanceKernel()
{
    delete varprior;
    delete iwprior;
}
