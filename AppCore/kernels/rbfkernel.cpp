// RBF kernel function.

#include "rbfkernel.h"
#include "mathutils.h"
#include "matwriter.h"
#include "matreader.h"
#include "optimization.h"
#include "prior.h"
#include "debugprint.h"

// Construct the kernel.
GPCMRBFKernel::GPCMRBFKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    int dims                                // Number of input dimensions
    ) : GPCMKernel(dims), iw(1,1), var(1,1), iwgrad(1,1), vargrad(1,1)
{
    // Construct standard parameters.
    iw << atof(params["inverse_width"][0].c_str());
    var << atof(params["variance"][0].c_str());
    iwprior = GPCMPrior::createPrior(params["inverse_width_prior"][0],
        atof(params["inverse_width_prior_weight"][0].c_str()),&iw,&iwgrad);
    varprior = GPCMPrior::createPrior(params["variance_prior"][0],
        atof(params["variance_prior_weight"][0].c_str()),&var,&vargrad);

    // Check if we should have ARD parameters.
    if (!params["ard"].empty() && (!params["ard"][0].compare("true") || !params["ard"][0].compare("1")))
    { // Create scale.
        scale.setOnes(1,dims);
        scalegrad.resize(1,dims);
        scaleprior = GPCMPrior::createPrior(params["ard_prior"][0],
            atof(params["ard_prior_weight"][0].c_str()),&scale,&scalegrad);
    }

    // Determine if we want to learn the parameters.
    bool bLearn = true;
    if (!params["learn"].empty() && !params["learn"][0].compare("false"))
        bLearn = false;
    bool bLearnVar = true;
    if (!params["learn_var"].empty() && !params["learn_var"][0].compare("false"))
        bLearnVar = false;

    name = "rbf";
    if (optimization && bLearn)
    {
        optimization->addVariable(VarXformExp,&iw,&iwgrad,"RBF inverse width");
        if (scale.cols() > 0)
            optimization->addVariable(VarXformExp,&scale,&scalegrad,"RBF scale");
        if (bLearnVar)
            optimization->addVariable(VarXformExp,&var,&vargrad,"RBF variance");
    }
    this->params = 2 + scale.cols();
    this->type = KernelTypeRBF;
}

// Get inverse width of this kernel.
double GPCMRBFKernel::getInverseWidth()
{
    return this->iw(0,0);
}

// Copy any settings from another kernel that we can.
void GPCMRBFKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMRBFKernel *othercst = dynamic_cast<GPCMRBFKernel*>(other);
        this->iw = othercst->iw;
        this->var = othercst->var;
        this->scale = othercst->scale;
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return covariance of a single set of points.
MatrixXd GPCMRBFKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.
    assert(scale.cols() == 0 && "ARD kernel currently unsupported!");

    dists.noalias() = pairwiseDistance(*X[0]);
    kmat.noalias() = var(0,0)*(dists*(-0.5*iw(0,0))).array().exp().matrix();
    return kmat;
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMRBFKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.
    assert(X[0]->rows() == 1); // This function only supports single query points.

    // Compute gradient.
    Xgrad.setZero(X[0]->rows(),X[0]->cols());

    // Return result.
    return MatrixXd::Constant(1,1,var(0,0));
}

// Return coviarance of two sets of points.
MatrixXd GPCMRBFKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    assert(X1[0] != NULL && X1[1] == NULL); // Make sure we have only one element.
    assert(X2[0] != NULL && X2[1] == NULL); // Make sure we have only one element.

    return var(0,0)*(pairwiseDistance(*X1[0],*X2[0])*(-0.5*iw(0,0))).array().exp();
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMRBFKernel::covarianceGrad(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2,              // Second data matrix.
    MatrixXd &X2grad                        // Gradient.
    )
{
    assert(X1[0] != NULL && X1[1] == NULL); // Make sure we have only one element.
    assert(X2[0] != NULL && X2[1] == NULL); // Make sure we have only one element.
    assert(X2[0]->rows() == 1); // This only supports single query points.

    // Compute result.
    MatrixXd K = var(0,0)*(pairwiseDistance(*X1[0],*X2[0])*(-0.5*iw(0,0))).array().exp();

    // Compute gradient.
    X2grad = iw(0,0)*(*X1[0]-X2[0]->replicate(X1[0]->rows(),1)).cwiseProduct(K.replicate(1,X1[0]->cols()));

    // Return result.
    return K;
}

// Recompute priors on all kernel parameters and return the likelihood.
double GPCMRBFKernel::recomputePriors()
{
    return varprior->recompute()+iwprior->recompute();
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMRBFKernel::recompute(
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
    gKk.noalias() = gKd.cwiseProduct(kmat);
    iwgrad(0,0) += -0.5*gKk.cwiseProduct(dists).sum();
    vargrad(0,0) += (1.0/var(0,0))*gKk.sum();

    if (Xgrad[0])
    {
        // Compute latent coordinate gradients.
        // First precompute matrix common to all X dimensions.
        Xtemp.noalias() = gKk.cwiseProduct(MatrixXd::Constant(N,N,2.0)-MatrixXd::Identity(N,N));
        
        // Next, compute the gradient for each dimension of X.
        for (int d = 0; d < X[0]->cols(); d++)
        {
            Xgrad[0]->col(d) += iw(0,0)*Xtemp.cwiseProduct(
                X[0]->col(d).transpose().replicate(N,1) - X[0]->col(d).replicate(1,N)
                    ).rowwise().sum();
        }
    }
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMRBFKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.
    int D = X[0]->cols();
    int N = X[0]->rows();

    // Compute kmat, which contains the standard kernel.
    kmat.noalias() = covariance(X);

    // Initialize fat identity if necessary.
    if (fatIdentity.rows() == 0)
    {
        fatIdentity = MatrixXd::Zero(N*D,N*D);
        for (int d = 0; d < D; d++)
            fatIdentity.block(d*N,d*N,N,N).setOnes();
        delta.resize(N,N*D);
    }

    // Compute the delta matrices.
    for (int d = 0; d < D; d++)
    {
        delta.block(0,d*N,N,N) = X[0]->col(d).replicate(1,N) - X[0]->col(d).transpose().replicate(N,1);
    }

    // Compute delta matrix.
    deltaMat.noalias() = (-iw(0,0))*delta.replicate(D,1).cwiseProduct(delta.transpose().replicate(1,D));

    // Compute final result.
    gradientKmat.noalias() = iw(0,0)*(fatIdentity - deltaMat).cwiseProduct(kmat.replicate(D,D));
    return gradientKmat;
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMRBFKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    assert(X[0] != NULL && X[1] == NULL); // Make sure we have only one element.
    assert(Xgrad[1] == NULL); // Make sure we have only one element.

    // Constants.
    int D = X[0]->cols();
    int N = X[0]->rows();

    // Compute hyperparameter gradients.
    gKk.noalias() = gKd.cwiseProduct(gradientKmat);
    iwgrad(0,0) += ((1/iw(0,0))*gKk -
                    0.5*gKk.cwiseProduct(dists.replicate(D,D)) -
                    deltaMat.cwiseProduct(kmat.replicate(D,D)).cwiseProduct(gKd)).sum();
    vargrad(0,0) += (1.0/var(0,0))*gKk.sum();

    if (Xgrad[0])
    {        
        // Compute latent coordinate gradients.
        // Precompute matrix common to all X dimensions.
        Xtemp.setZero(N,N);
        for (int dh = 0; dh < D; dh++)
        {
            for (int dv = 0; dv < D; dv++)
            {
                Xtemp += gKk.block(N*dv,N*dh,N,N) + gKk.block(N*dv,N*dh,N,N).transpose();
            }
        }
        
        // Next, compute the gradient for each dimension of X.
        for (int d = 0; d < D; d++)
        {
            // Sum up the gK matrix across dimensions.
            dmat.setZero(N,N*D);
            
            for (int dh = 0; dh < D; dh++)
            {
                dmat.block(0,dh*N,N,N) += gKd.block(N*d,N*dh,N,N)+gKd.block(N*dh,N*d,N,N)+
                    gKd.block(N*d,N*dh,N,N).transpose()+gKd.block(N*dh,N*d,N,N).transpose();
            }

            // Add the exponential derivative component.
            // TODO: something about this term is incorrect.
            Xgrad[0]->col(d) -= pow(iw(0,0),2)*(delta.cwiseProduct(dmat.cwiseProduct(kmat.replicate(1,D))).rowwise().sum());

            // Add the outer difference derivative component.
            Xgrad[0]->col(d) -= iw(0,0)*(delta.block(0,d*N,N,N).cwiseProduct(Xtemp).rowwise().sum());
        }
    }
}

// Write kernel data to file.
void GPCMRBFKernel::write(
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
void GPCMRBFKernel::load(
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
void GPCMRBFKernel::printParams()
{
    DBPRINTLN("RBF variance: " << var(0,0));
    DBPRINTLN("RBF inverse width: " << iw(0,0));
}

// Destructor.
GPCMRBFKernel::~GPCMRBFKernel()
{
    delete varprior;
    delete iwprior;
}
