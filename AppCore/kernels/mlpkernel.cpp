// MLP kernel function.

#include "mlpkernel.h"
#include "mathutils.h"
#include "matwriter.h"
#include "matreader.h"
#include "optimization.h"
#include "prior.h"
#include "debugprint.h"

// This disables a really obnoxious "template argument too long" warning on VC++
#pragma warning(disable : 4503)

// Construct the kernel.
GPCMMLPKernel::GPCMMLPKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    int dims                                // Number of input dimensions
    ) : GPCMKernel(dims), wt(1,1), bias(1,1), var(1,1), wtgrad(1,1), biasgrad(1,1), vargrad(1,1)
{
    wt << atof(params["weight"][0].c_str());
    bias << atof(params["bias"][0].c_str());
    var << atof(params["variance"][0].c_str());
    wtprior = GPCMPrior::createPrior(params["weight_prior"][0],
        atof(params["weight_prior_weight"][0].c_str()),&wt,&wtgrad);
    biasprior = GPCMPrior::createPrior(params["bias_prior"][0],
        atof(params["bias_prior_weight"][0].c_str()),&bias,&biasgrad);
    varprior = GPCMPrior::createPrior(params["variance_prior"][0],
        atof(params["variance_prior_weight"][0].c_str()),&var,&vargrad);
    name = "mlp";
    if (optimization)
    {
        optimization->addVariable(VarXformExp,&wt,&wtgrad,"MLP weight");
        optimization->addVariable(VarXformExp,&bias,&biasgrad,"MLP bias");
        optimization->addVariable(VarXformExp,&var,&vargrad,"MLP variance");
    }
    this->params = 3;
    this->type = KernelTypeMLP;
}

// Copy any settings from another kernel that we can.
void GPCMMLPKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMMLPKernel *othercst = dynamic_cast<GPCMMLPKernel*>(other);
        this->wt = othercst->wt;
        this->bias = othercst->bias;
        this->var = othercst->var;
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return covariance of a single set of points.
MatrixXd GPCMMLPKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.

    innerProducts.noalias() = (*X[0])*X[0]->transpose();
    numerator.noalias() = innerProducts*wt(0,0) + MatrixXd::Constant(X[0]->rows(),X[0]->rows(),bias(0,0));
    vec.noalias() = numerator.diagonal() + VectorXd::Constant(X[0]->rows(),1.0);
    denominator.noalias() = (vec*vec.transpose()).cwiseSqrt();
    arg.noalias() = numerator.cwiseQuotient(denominator);
    kmat.noalias() = arg.array().asin().matrix();

    return var(0,0)*kmat;
}

// Return coviarance of two sets of points.
MatrixXd GPCMMLPKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    assert(X1[0] != NULL && X1[1] == NULL); // Make sure we have only one element.
    assert(X2[0] != NULL && X2[1] == NULL); // Make sure we have only one element.

    return var(0,0)*(((((*X1[0])*X2[0]->transpose())*wt(0,0) + MatrixXd::Constant(X1[0]->rows(),X1[0]->rows(),bias(0,0))).cwiseQuotient(
        ((X1[0]->rowwise().squaredNorm()*wt(0,0) + MatrixXd::Constant(X1[0]->rows(),1,bias(0,0)+1.0))*
         (X2[0]->rowwise().squaredNorm()*wt(0,0) + MatrixXd::Constant(X1[0]->rows(),1,bias(0,0)+1.0)).transpose()).cwiseSqrt())).array().asin().matrix());
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMMLPKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    assert(false && "Unsupported!");

    // Return result.
    return MatrixXd();
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMMLPKernel::covarianceGrad(
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
double GPCMMLPKernel::recomputePriors()
{
    return varprior->recompute()+wtprior->recompute()+biasprior->recompute();
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMMLPKernel::recompute(
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

    // Compute gradient of variance.
    vargrad(0,0) += kmat.cwiseProduct(gKd).sum();

    // Compute gradient of weight and bias.
    denominatorCubed.noalias() = denominator.array().pow(3).matrix();
    baseCovGrad.noalias() = var(0,0)*gKd.cwiseQuotient((MatrixXd::Ones(arg.rows(),arg.cols()) - arg.cwiseProduct(arg)).cwiseSqrt());
    vec.noalias() = innerProducts.diagonal();
    wtgrad(0,0) += (innerProducts.cwiseQuotient(denominator) -
        0.5*(numerator.cwiseQuotient(denominatorCubed)).cwiseProduct(
            (wt(0,0)*vec + MatrixXd::Constant(vec.rows(),vec.cols(),bias(0,0)+1.0))*vec.transpose() +
            vec*(wt(0,0)*vec + MatrixXd::Constant(vec.rows(),vec.cols(),bias(0,0)+1.0)).transpose())).cwiseProduct(baseCovGrad).sum();
    biasgrad(0,0) += (denominator.array().inverse().matrix() -
        0.5*(numerator.cwiseQuotient(denominatorCubed)).cwiseProduct(
        (wt(0,0)*vec + MatrixXd::Constant(vec.rows(),vec.cols(),2.0*bias(0,0)+2.0)).replicate(1,vec.rows()) +
        (wt(0,0)*vec.transpose()).replicate(vec.rows(),1))).cwiseProduct(baseCovGrad).sum();

    // Compute X gradients.
    if (Xgrad[0])
    {
        for (int d = 0; d < X[0]->cols(); d++)
        {
            MatrixXd b = (X[0]->rowwise().squaredNorm()*wt(0,0) + VectorXd::Constant(N,bias(0,0)+1.0)).replicate(1,N).cwiseProduct(
                     numerator).cwiseProduct((X[0]->col(d)*wt(0,0)*2.0).transpose().replicate(N,1)).cwiseQuotient(denominatorCubed);
            Xgrad[0]->col(d) += ((X[0]->col(d)*(2.0*wt(0,0))).replicate(1,N).cwiseQuotient(denominator) -
                ((X[0]->rowwise().squaredNorm()*wt(0,0) + VectorXd::Constant(N,bias(0,0)+1.0)).replicate(1,N).cwiseProduct(
                     numerator).cwiseProduct((X[0]->col(d)*wt(0,0)*2.0).transpose().replicate(N,1)).cwiseQuotient(denominatorCubed))).cwiseProduct(
                     baseCovGrad).transpose().rowwise().sum();
        }
    }
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMMLPKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(false && "Unsupported!");
    return MatrixXd::Zero(1,1);
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMMLPKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(false && "Unsupported!");
}

// Write kernel data to file.
void GPCMMLPKernel::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    GPCMKernel::write(writer); // Let superclass write first.

    // Write parameters.
    writer->writeDouble(wt(0,0),"weightVariance");
    writer->writeDouble(bias(0,0),"biasVariance");
    writer->writeDouble(var(0,0),"variance");

    // Write priors.
    GPCMMatWriter *cellWriter = writer->writeCell("priors",1,3);
    GPCMMatWriter *priorStruct = cellWriter->writeStruct("",1,1);
    priorStruct->writeDouble(1.0,"index");
    wtprior->write(priorStruct);
    priorStruct = cellWriter->closeStruct();
    priorStruct = cellWriter->writeStruct("",1,1);
    priorStruct->writeDouble(2.0,"index");
    biasprior->write(priorStruct);
    priorStruct = cellWriter->closeStruct();
    priorStruct = cellWriter->writeStruct("",1,1);
    priorStruct->writeDouble(3.0,"index");
    varprior->write(priorStruct);
    cellWriter->closeStruct();
    writer->closeCell();

    // Write transforms struct.
    GPCMMatWriter *xformStruct = writer->writeStruct("transforms",1,3);
    xformStruct->writeDouble(1.0,"index");
    xformStruct->writeString("exp","type");
    xformStruct = writer->closeStruct();
    xformStruct->writeDouble(2.0,"index");
    xformStruct->writeString("exp","type");
    xformStruct = writer->closeStruct();
    xformStruct->writeDouble(3.0,"index");
    xformStruct->writeString("exp","type");
    writer->closeStruct();
}

// Load kernel from specified MAT file reader.
void GPCMMLPKernel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Load superclass.
    GPCMKernel::load(reader);

    // Load variance.
    var = reader->getVariable("variance");

    // Load weight.
    wt = reader->getVariable("weightVariance");

    // Load bias.
    bias = reader->getVariable("biasVariance");
}

// Print out kernel parameters.
void GPCMMLPKernel::printParams()
{
    DBPRINTLN("MLP variance: " << var(0,0));
    DBPRINTLN("MLP weight:");
    DBPRINTMAT(wt);
    DBPRINTLN("MLP bias:");
    DBPRINTMAT(bias);
}

// Destructor.
GPCMMLPKernel::~GPCMMLPKernel()
{
    delete varprior;
    delete wtprior;
    delete biasprior;
}
