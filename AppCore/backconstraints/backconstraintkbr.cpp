// Kernel-based regression back constraint function.

#include "debugprint.h"
#include "backconstraintkbr.h"
#include "matwriter.h"
#include "matreader.h"
#include "options.h"
#include "optimization.h"
#include "kernel.h"

#include <Eigen/Eigen>

// Construct the back constraint function.
GPCMBackConstraintKBR::GPCMBackConstraintKBR(
    GPCMParams &params,                     // Parameters of this kernel.
    GPCMOptions &options,                   // Loaded options used for creating other kernels.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &dataMatrix,                   // Matrix of Y values.
    MatrixXd &X                             // Matrix of X values.
    ) : GPCMBackConstraint(params,options,optimization,dataMatrix,X)
{
    // Create the kernel.
    std::string kernel_name = params["kernel"][0];
    this->kernel = GPCMKernel::createKernel(options[kernel_name],options,NULL,dataMatrix.cols());

    // Resize alpha and register it.
    A.resize(X.rows(),X.cols());
    Agrad.resize(X.rows(),X.cols());
    optimization->addVariable(VarXformNone,&A,&Agrad,"KBR A Matrix");

    // Set type.
    name = "kbr";
    type = BackConstraintKBR;
}

// Copy any settings from another back constraint function that we can.
void GPCMBackConstraintKBR::copySettings(
    GPCMBackConstraint *other               // Function to copy parameters from.
    )
{
    if (other->getType() == getType())
    {
        GPCMBackConstraintKBR *othercst = dynamic_cast<GPCMBackConstraintKBR*>(other);
        this->kernel->copySettings(othercst->kernel);
        if (this->A.cols() == othercst->A.cols())
        {
            this->Kd = othercst->Kd;
            this->K = othercst->K;
            this->A = othercst->A;
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
void GPCMBackConstraintKBR::initialize()
{
    // Constraint pointers.
    MatrixXd *Yptr[MAX_KERNEL_GROUPS];
    memset(Yptr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    Yptr[0] = &dataMatrix;

    // Compute kernel matrix.
    K = kernel->covariance(Yptr);
    Kd = kernel->covariance(Yptr,Yptr);

    // Compute alpha matrix.
    LLT<MatrixXd> cholesky = K.llt();
    A = cholesky.solve(X);

    // Update X.
    updateLatentCoords();
}

// Compute the gradients of this back constraint function using the gradient of the latent coordinates.
double GPCMBackConstraintKBR::updateGradient(
    MatrixXd &Xgrad,                        // Current gradient with respect to latent coordinates.
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // The gradient is simply Kd*Xgrad.
    Agrad = Kd*Xgrad;

    // Return score.
    return 0.0;
}

// Update the latent coordinates based on the current parameters.
void GPCMBackConstraintKBR::updateLatentCoords()
{
    X = Kd*A;
}

// Write back constraint to file.
void GPCMBackConstraintKBR::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write type.
    GPCMBackConstraint::write(writer);

    // Write kernel.
    GPCMMatWriter *kernStruct = writer->writeStruct("kern",1,1);
    kernel->write(kernStruct);
    writer->closeStruct();

    // Write matrices.
    writer->writeMatrix(MatrixXd::Zero(1,dataMatrix.cols()),"bias");
    writer->writeMatrix(K,"K");
    writer->writeMatrix(A,"A");
    writer->writeMatrix(X,"X");

    // Write values.
    writer->writeDouble((double)X.rows(),"numData");
    writer->writeDouble((double)dataMatrix.cols(),"inputDim");
    writer->writeDouble((double)X.cols(),"outputDim");
    writer->writeDouble((double)((X.rows()+1)*X.cols()),"numParams");
}

// Load parameters from specified MAT file reader.
void GPCMBackConstraintKBR::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read kernel.
    this->kernel->load(reader->getStruct("kern"));

    // Read matrices.
    K = reader->getVariable("K");
    A = reader->getVariable("A"); 

    // Construct Kd.
    MatrixXd *Yptr[MAX_KERNEL_GROUPS];
    memset(Yptr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    Yptr[0] = &dataMatrix;
    Kd = kernel->covariance(Yptr,Yptr);
}

// Destructor.
GPCMBackConstraintKBR::~GPCMBackConstraintKBR()
{
    delete kernel;
}
