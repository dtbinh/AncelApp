// Gaussian process back constraint function.

#include "debugprint.h"
#include "backconstraintgp.h"
#include "matwriter.h"
#include "matreader.h"
#include "options.h"
#include "optimization.h"
#include "kernel.h"
#include "gp.h"

#include <Eigen/Eigen>

// Construct the back constraint function.
GPCMBackConstraintGP::GPCMBackConstraintGP(
    GPCMParams &params,                     // Parameters of this kernel.
    GPCMOptions &options,                   // Loaded options used for creating other kernels.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &dataMatrix,                   // Matrix of Y values.
    MatrixXd &X                             // Matrix of X values.
    ) : GPCMBackConstraint(params,options,optimization,dataMatrix,X)
{
    // Set up input and output matrices.
    gpInput = dataMatrix;
    Xpts = X;
    XptsScaled = Xpts;
    XptsGrad.resize(Xpts.rows(),Xpts.cols());
    gK.resize(Xpts.rows(),Xpts.rows());
    gKd.resize(Xpts.rows(),Xpts.rows());
    optimization->addVariable(VarXformNone,&Xpts,&XptsGrad,"GP Back X");

    // Create gaussian process.
    MatrixXd *Inptr = &gpInput;
    MatrixXd *Ingradptr = NULL;
    gaussianProcess = new GPCMGaussianProcess(params,options,
        optimization,NULL,Xpts,XptsScaled,&Inptr,&Ingradptr,1,false,false);

    // Set type.
    name = "gp";
    type = BackConstraintGP;
}

// Copy any settings from another back constraint function that we can.
void GPCMBackConstraintGP::copySettings(
    GPCMBackConstraint *other               // Function to copy parameters from.
    )
{
    if (other->getType() == getType())
    {
        GPCMBackConstraintGP *othercst = dynamic_cast<GPCMBackConstraintGP*>(other);
        this->gaussianProcess->copySettings(othercst->gaussianProcess);
        if (this->Xpts.cols() == othercst->Xpts.cols())
        {
            this->Xpts = othercst->Xpts;
            this->XptsScaled = othercst->XptsScaled;
            this->XptsGrad = othercst->XptsGrad;
            this->K = othercst->K;
        }
        else
        {
            initialize();
        }
    }
    else
    {
        DBWARNING("Back constraints type mismatching when initializing from another back constraint function!");
    }
}

// Initialize the back constraint function by optimizing the parameters for an initial latent matrix.
void GPCMBackConstraintGP::initialize()
{
    // Initialize variables.
    MatrixXd *Inptr[MAX_KERNEL_GROUPS];
    memset(Inptr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    Inptr[0] = &gpInput;
    Xpts = X;
    XptsScaled = Xpts;
    K = this->gaussianProcess->getKernel()->covariance(Inptr,Inptr);

    // Update X.
    updateLatentCoords();
}

// Compute the gradients of this back constraint function using the gradient of the latent coordinates.
double GPCMBackConstraintGP::updateGradient(
    MatrixXd &Xgrad,                        // Current gradient with respect to latent coordinates.
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // Compute additional gK component from Xgrad.
    tempAlpha.noalias() = gaussianProcess->getKinv()*Xpts;
    gK.noalias() = ((-1.0)*(K*gaussianProcess->getKinv()*tempAlpha))*Xgrad.transpose();
    gKd.noalias() = gK + tempAlpha*Xgrad.transpose();
    gaussianProcess->addGKDiag(gKd);
    gaussianProcess->addGK(gK);

    // Get likelihood.
    double likelihood = gaussianProcess->recomputeLikelihood(bNeedGradient);

    // Compute gradient with respect to GP output.
    XptsGrad = K*gaussianProcess->getKinv()*Xgrad;

    // Add gradient with respect to GP objective.
    XptsGrad -= gaussianProcess->getAlpha();

    // Return score.
    return likelihood;
}

// Update the latent coordinates based on the current parameters.
void GPCMBackConstraintGP::updateLatentCoords()
{
    // Compute kernel matrix.
    gaussianProcess->recomputeKernel();

    // Update K.
    MatrixXd *Inptr[MAX_KERNEL_GROUPS];
    memset(Inptr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    Inptr[0] = &gpInput;
    K = this->gaussianProcess->getKernel()->covariance(Inptr,Inptr);

    X = K*gaussianProcess->getKinv()*Xpts;
    XptsScaled = Xpts;
}

// Write back constraint to file.
void GPCMBackConstraintGP::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write type.
    GPCMBackConstraint::write(writer);

    // Write Gaussian process.
    gaussianProcess->write(writer);
}

// Load parameters from specified MAT file reader.
void GPCMBackConstraintGP::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read guassian process.
    this->gaussianProcess->load(reader);

    // Construct K.
    MatrixXd *Inptr[MAX_KERNEL_GROUPS];
    memset(Inptr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    Inptr[0] = &gpInput;
    K = this->gaussianProcess->getKernel()->covariance(Inptr,Inptr);
}

// Destructor.
GPCMBackConstraintGP::~GPCMBackConstraintGP()
{
    delete gaussianProcess;
}
