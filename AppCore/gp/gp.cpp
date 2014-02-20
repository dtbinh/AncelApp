// Gaussian process model.

#include "debugprint.h"
#include "gp.h"
#include "matfile.h"
#include "matreader.h"
#include "prior.h"
#include "gaussprior.h"
#include "optimization.h"
#include "mathutils.h"

#include <Eigen/Eigen>

// Constructor, creates Gaussian process from parameters.
GPCMGaussianProcess::GPCMGaussianProcess(
    GPCMParams &params,                     // Parameters of this kernel.
    GPCMOptions &options,                   // Loaded options used for creating other kernels.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    GPCMKernel *kernel,                     // Optional kernel.
    MatrixXd &dataMatrix,                   // Matrix of data entries.
    MatrixXd &Y,                            // Matrix of scaled and biased entries.
    MatrixXd **X,                           // Matrices of input coordinates.
    MatrixXd **Xgrad,                       // Gradients of input coordinates.
    int inputMats,                          // Number of input matrices.
    bool bBiasData,                         // Whether to remove the mean of the data matrix.
    bool bScaleData,                        // Whether to initialize scales that rescale data matrix to unit variance.
    bool bDontLearnScales,                  // Override scale learning.
    MatrixXd *dists                         // Optional input distances matrix.
    ) : dataMatrix(dataMatrix), Y(Y)
{
    // Copy input matrices.
    memset(this->X,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    memset(this->Xgrad,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    memcpy(this->X,X,sizeof(MatrixXd*)*inputMats);
    memcpy(this->Xgrad,Xgrad,sizeof(MatrixXd*)*inputMats);
    this->inputMats = inputMats;

    // Initialize bias and scale, as well as Y matrix.
    int N = dataMatrix.rows();

    // Compute bias.
    if (bBiasData)
        bias = dataMatrix.colwise().sum()/N;
    else
        bias = MatrixXd::Zero(1,dataMatrix.cols());

    // Compute scale.
    if (bScaleData)
        scale = ((dataMatrix - bias.replicate(N,1)).colwise().squaredNorm()/N).array().sqrt().inverse();
    else
        scale = MatrixXd::Ones(1,dataMatrix.cols());

    // Initialize scale gradient.
    scaleGrad.resize(1,scale.cols());

    // Initialize scaled and biased values.
    Y = (dataMatrix - bias.replicate(N,1)).cwiseProduct(scale.replicate(N,1));

    // Compute input dimensionality.
    q = 0;
    for (int i = 0; i < this->inputMats; i++)
        q += X[i]->cols();

    // Register scale variables in the optimization.
    if ((!params["learn_scales"][0].compare("true") ||
         !params["learn_scales"][0].compare("1")) && !bDontLearnScales)
    {
        this->bLearnScales = true;
        this->bEMScales = false;
        optimization->addVariable(VarXformExp,&scale,&scaleGrad,"scale");
    }
    else if (!params["learn_scales"][0].compare("em") && !bDontLearnScales)
    {
        this->bLearnScales = true;
        this->bEMScales = true;
    }
    else
    {
        this->bLearnScales = false;
        this->bEMScales = false;
    }

    // Create the kernel function.
    if (kernel)
    {
        this->kernel = kernel;
    }
    else
    {
        std::string kernel_name = params["kernel"][0];
        this->kernel = GPCMKernel::createKernel(options[kernel_name],options,optimization,q,dists);
    }

    // Create priors.
    if (this->bLearnScales)
        this->scalePrior = GPCMPrior::createPrior(params["scale_prior"][0],
            atof(params["scale_prior_weight"][0].c_str()),&scale, bEMScales ? NULL : &scaleGrad);
    else
        this->scalePrior = NULL;
}

// Copy any settings from another model that we can.
void GPCMGaussianProcess::copySettings(
    GPCMGaussianProcess *other              // Model to copy from.
    )
{
    // Copy individual parameters.
    if (other->bias.cols() == this->bias.cols())
    { // Only copy scale and bias if they have the same dimensionality.
        this->bias = other->bias;
        this->scale = other->scale;
    }

    // Copy the kernel settings.
    this->kernel->copySettings(other->kernel);
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMGaussianProcess::recomputeClosedForm()
{
    // Compute scales in closed form if necessary.
    if (this->bLearnScales && this->bEMScales)
    {
        int N = dataMatrix.rows();

        // Note that this assumes a Gaussian prior on the scales.
        double precision = dynamic_cast<GPCMGaussPrior*>(this->scalePrior)->getPrecision();

        // Re-estimate scales.
        scale = sqrt((double)N)*(innerProducts.array()/scale.array().square() +
            ArrayXXd::Constant(1,scale.cols(),precision)).inverse().sqrt();
        
        // Recompute Y.
        Y.noalias() = (dataMatrix - bias.replicate(N,1)).cwiseProduct(scale.replicate(N,1));
    }
}

// Recompute the kernel.
void GPCMGaussianProcess::recomputeKernel()
{
    // Constants.
    int N = dataMatrix.rows();
    int D = dataMatrix.cols();

    // Initialize the covariance matrix.
    K.noalias() = kernel->covariance(X);

    // Compute inverse of kernel matrix.
    LLT<MatrixXd> cholesky = K.llt();
    logDetK = cholesky.matrixLLT().diagonal().array().log().sum()*2.0;
    invK = cholesky.solve(MatrixXd::Identity(N,N));

    // Clear gK matrices.
    gK.setZero(N,N);
    gKd.setZero(N,N);
}

// Recompute the likelihood.
double GPCMGaussianProcess::recomputeLikelihood(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // Constants.
    int N = dataMatrix.rows();
    int D = dataMatrix.cols();

    // Compute Y from the data matrix under the current scales.
    if (this->bLearnScales && !this->bEMScales)
        Y.noalias() = (dataMatrix - bias.replicate(N,1)).cwiseProduct(scale.replicate(N,1));

    // Initialize the log likelihood.
    loglikelihood = 0;

    // Compute alpha.
    alpha.noalias() = invK*Y;

    // Compute inner products.
    innerProducts.noalias() = Y.cwiseProduct(alpha).colwise().sum();

    // Add GP likelihood.
    loglikelihood += -0.5*innerProducts.sum() - (0.5*(double)D)*logDetK;

    // Add scale normalization component.
    loglikelihood += ((double)N)*scale.array().log().sum();

    // Compute gradients for scales.
    if (this->bLearnScales)
    {
        loglikelihood += scalePrior->recompute(); // This sets scaleGrad to be the prior gradient.
        if (!this->bEMScales && bNeedGradient)
            scaleGrad.noalias() += (((double)N)*scale.array().inverse() - innerProducts.array()/scale.array()).matrix();
    }

    // Recompute all kernel priors.
    loglikelihood += kernel->recomputePriors();

    // Compute gradient with respect to the kernel matrix K.
    gKtemp.noalias() = -((double)D)*invK;
    for (int k = 0; k < alpha.cols(); k++)
        gKtemp += alpha.col(k)*alpha.col(k).transpose();
    gKtemp *= 0.5;
    gK += gKtemp;
    gKd += gKtemp;

    // Compute gradients for X and kernel hyperparameters.
    if (bNeedGradient)
        kernel->recompute(gK,gKd,X,Xgrad);

    // Subtract constant from log likelihood.
    loglikelihood -= ((double)(D*N))*0.5*log(2.0*PI);
    return loglikelihood;
}

// Add to the gK matrix.
void GPCMGaussianProcess::addGK(
    MatrixXd &addedGK                       // gK component to add.
    )
{
    gK += addedGK;
}

// Add to the gKd matrix.
void GPCMGaussianProcess::addGKDiag(
    MatrixXd &addedGKd                      // gKd component to add.
    )
{
    gKd += addedGKd;
}

// Recompute all stored temporaries when variables change.
double GPCMGaussianProcess::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // Compute the kernel.
    recomputeKernel();

    // Return the likelihood.
    return recomputeLikelihood(bNeedGradient);
}

// Load model from specified MAT file reader.
void GPCMGaussianProcess::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read design matrix.
    dataMatrix = reader->getVariable("y");

    // Read biases and scales.
    bias = reader->getVariable("bias");
    scale = reader->getVariable("scale").array().inverse().matrix();

    // Read inner products.
    innerProducts = reader->getVariable("innerProducts");

    // Read scaled & biased data.
    Y = reader->getVariable("m");

    // Read the full input matrix and distribute it among the components.
    MatrixXd fullX = reader->getVariable("X");
    int s = 0;
    for (int i = 0; i < inputMats; i++)
    {
        (*X[i]) = fullX.block(0,s,Y.rows(),X[i]->cols());
        s += X[i]->cols();
    }

    // Read covariance matrix.
    K = reader->getVariable("K_uu");

    // Read inverse covariance.
    invK = reader->getVariable("invK_uu");

    // Read alpha vector.
    alpha = reader->getVariable("alpha");

    // Read log determinant.
    MatrixXd logDetKMat = reader->getVariable("logDetK_uu");
    logDetK = logDetKMat(0,0);

    // Read the kernel.
    kernel->load(reader->getStruct("kern"));
}

// Write GP data to file.
void GPCMGaussianProcess::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Currently, only the full training conditional is supported.
    writer->writeString("ftc","approx");

    // No missing data.
    writer->writeDouble(0.0,"isMissingData");

    // Not optimizing beta.
    writer->writeDouble(0.0,"optimiseBeta");

    // Write whether we learn scales.
    if (this->bLearnScales)
        writer->writeDouble(1.0,"learnScales");
    else
        writer->writeDouble(0.0,"learnScales");

    // Store the kernel.
    GPCMMatWriter *kernStruct = writer->writeStruct("kern",1,1);
    kernel->write(kernStruct);
    writer->closeStruct();

    // Store design matrix.
    writer->writeMatrix(dataMatrix,"y");

    // Store biases.
    writer->writeMatrix(bias,"bias");

    // Store scales.
    // Note that the MATLAB code assumes *inverse* scales, so we invert them here.
    writer->writeMatrix(scale.array().inverse().matrix(),"scale");

    // Store scale transform.
    writer->writeString("exp","scaleTransform");

    // Store inner products.
    writer->writeMatrix(innerProducts,"innerProducts");

    // Write biased, scaled data matrix.
    writer->writeMatrix(Y,"m");

    // Create the full input matrix.
    int q = 0;
    int s = 0;
    for (int i = 0; i < inputMats; i++)
        q += X[i]->cols();
    MatrixXd fullX(Y.rows(),q);
    for (int i = 0; i < inputMats; i++)
    {
        fullX.block(0,s,Y.rows(),X[i]->cols()) = *X[i];
        s += X[i]->cols();
    }

    // Store the input matrix.
    writer->writeMatrix(fullX,"X");

    // Store the input dimensionality.
    writer->writeDouble(q,"q");

    // Store the full dimensionality.
    writer->writeDouble(Y.cols(),"d");

    // Store number of points.
    writer->writeDouble(Y.rows(),"N");

    // Store the covariance matrix.
    writer->writeMatrix(K,"K_uu");

    // Store the inverse covariance matrix.
    writer->writeMatrix(invK,"invK_uu");

    // Store the log determinant.
    writer->writeDouble(logDetK,"logDetK_uu");

    // Store the alpha vector.
    writer->writeMatrix(alpha,"alpha");
}

// Return alpha matrix.
MatrixXd &GPCMGaussianProcess::getAlpha()
{
    return alpha;
}

// Get scale vector.
MatrixXd &GPCMGaussianProcess::getScale()
{
    return scale;
}

// Compute noise.
double GPCMGaussianProcess::getNoise()
{
    return kernel->getNoise();
}

// Get kernel.
GPCMKernel *GPCMGaussianProcess::getKernel()
{
    return kernel;
}

// Get inverted kernel matrix.
const MatrixXd &GPCMGaussianProcess::getKinv()
{
    return invK;
}

// Get X matrices array.
const MatrixXd* const *GPCMGaussianProcess::getX()
{
    return X;
}

// Get X gradients array.
MatrixXd **GPCMGaussianProcess::getXGrad()
{
    return Xgrad;
}

// Get data matrix.
MatrixXd &GPCMGaussianProcess::getDataMatrix()
{
    return dataMatrix;
}

// Compute posterior variance at specified point.
MatrixXd GPCMGaussianProcess::posteriorVariance(
    const MatrixXd* const *Xstar            // Sample point.
    )
{
    // Equation for posterior variance:
    // v = k(Xstar,Xstar) - k*' * K^-1 * k
    // where k = k(Xstar,X)
    MatrixXd k = kernel->covariance(X,Xstar);
    MatrixXd kd = kernel->covariance(Xstar);
    return kd - k.transpose()*invK*k;
}

// Compute posterior mean at specified point.
MatrixXd GPCMGaussianProcess::posteriorMean(
    const MatrixXd* const *Xstar,           // Sample point.
    MatrixXd *var,                          // Optional variance output.
    MatrixXd *dMdXstar,                     // Change in mean with respect to Xstar.
    MatrixXd *dVdXstar                      // Change in variance with respect to Xstar.
    )
{
    // Kernalize the input.
    MatrixXd k;
    MatrixXd kGrad(X[0]->rows(),q);
    
    // If we want the mean gradient, compute gradient here.
    if (dMdXstar)
        k = kernel->covarianceGrad(X,Xstar,kGrad);
    else
        k = kernel->covariance(X,Xstar);

    // Check if we want the variance.
    if (var)
    {
        MatrixXd kd;
        MatrixXd kdGrad(Xstar[0]->rows(),q);
        
        // If we want the variance gradient, compute mean here.
        if (dVdXstar)
            kd = kernel->covarianceGrad(Xstar,kdGrad);
        else
            kd = kernel->covariance(Xstar);

        if (var->cols() > 0 && var->cols() == var->rows())
        { // If we are passed a non-empty square matrix, we want the full covariance.
            *var = kd - k.transpose()*invK*k;
        }
        else
        {
            var->resize(k.cols(),1);
            for (int i = 0; i < k.cols(); i++)
                var->block(i,0,1,1) = kd.block(i,i,1,1) - k.col(i).transpose()*invK*k.col(i);
        }

        // Check if we want the gradient of the variance.
        if (dVdXstar)
        {
            assert(var->rows() == 1); // Only supports single query points.
            *dVdXstar = kdGrad.transpose() - 2.0*kGrad.transpose()*invK*k;
        }
    }

    // Compute scaled mean gradient.
    if (dMdXstar)
    {
        *dMdXstar = (kGrad.transpose()*alpha).cwiseQuotient(scale.replicate(kGrad.cols(),1));
    }

    // Return mean.
    return (k.transpose()*alpha).cwiseQuotient(scale.replicate(k.cols(),1)) + bias.replicate(k.cols(),1);
}

// Destructor.
GPCMGaussianProcess::~GPCMGaussianProcess()
{
    delete kernel;
    delete scalePrior;
}
