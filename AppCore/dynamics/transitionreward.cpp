// Wrapper for GP dynamics term that is used to compute a transition reward.

#include "transitionreward.h"
#include "optimization.h"
#include "gp.h"
#include "dynamics.h"
#include "dynamicsgp.h"
#include "model.h"
#include "matwriter.h"
#include "matreader.h"
#include "debugprint.h"
#include "mathutils.h"
#include "dynamicstimegp.h"
#include "tensorkernel.h"
#include "compoundkernel.h"

// Global function for creating new transition reward.
GPCMTransitionReward *GPCMTransitionReward::createTransitionReward(
    GPCMParams &params,                     // Parameters for this object.
    GPCMOptions &options,                   // All options.
    MatrixXd &X,                            // Reference to latent positions.
    GPCMModel *model,                       // Refernece to model.
    std::vector<int> &sequence,             // Trajectory sequence indices.
    double frameLength                      // Length of each frame in seconds.
    )
{
    // Simply construct a new transition reward for now.
    return new GPCMTransitionReward(params,options,X,model,sequence,frameLength);
}

    // Constructor.
GPCMTransitionReward::GPCMTransitionReward(
    GPCMParams &params,                     // Parameters for this object.
    GPCMOptions &options,                   // All options.
    MatrixXd &X,                            // Reference to latent positions.
    GPCMModel *model,                       // Refernece to model.
    std::vector<int> &sequence,             // Trajectory sequence indices.
    double frameLength                      // Length of each frame in seconds.
    ) : X(X), model(model), sequence(sequence), frameLength(frameLength)
{
    // Decide if we're using difference mode.
    if (!params["difference_mode"][0].compare("true") ||
        !params["difference_mode"][0].compare("1"))
        bDifference = true;
    else
        bDifference = false;

    // Decide if we're using velocity.
    if (!params["use_velocity"][0].compare("true") ||
        !params["use_velocity"][0].compare("1"))
        bUseVelocity = true;
    else
        bUseVelocity = false;

    // Decide if we're using a tensor product kernel for velocity.
    if (!params["tensor_product_kernel"].empty() &&
        (!params["tensor_product_kernel"][0].compare("true") ||
         !params["tensor_product_kernel"][0].compare("!")))
        bTensorProduct = true;
    else
        bTensorProduct = false;

    // Initialize input and output matrices.
    int cnt = bUseVelocity ? 2 : 1;
    if (bTensorProduct)
    {
        inputMatrix.resize(X.rows()-sequence.size()*cnt,X.cols());
        velMatrix.resize(X.rows()-sequence.size()*cnt,X.cols());
    }
    else
    {
        inputMatrix.resize(X.rows()-sequence.size()*cnt,X.cols()*cnt);
    }
    outputMatrix.resize(X.rows()-sequence.size()*cnt,X.cols());
    outputMatrixScaled.resize(X.rows()-sequence.size()*cnt,X.cols());

    // Get optimization options.
    bool bValidate = !params["validate_gradients"][0].compare("true");
    int maxIterations = atoi(params["iterations"][0].c_str());

    // Create optimization.
    optimization = new GPCMOptimization(bValidate,false,options["optimization"]["algorithm"][0],
        maxIterations,1);

    // Set up the kernel.
    if (bTensorProduct)
    {
        // Set up the tensor product kernel.
        std::string kernelFirstName = params["kernel_first_pt"][0];
        std::string kernelSecondName = params["kernel_second_pt"][0];
        std::string kernelNoiseName = params["kernel_noise"][0];
        GPCMKernel *tensorComponents[2];
        GPCMKernel *compoundComponents[2];
        tensorComponents[0] = GPCMKernel::createKernel(options[kernelFirstName],options,optimization,X.cols());
        tensorComponents[1] = GPCMKernel::createKernel(options[kernelSecondName],options,optimization,X.cols());
        compoundComponents[0] = new GPCMTensorKernel(options[kernelFirstName],optimization,tensorComponents,2,2*X.cols());
        compoundComponents[1] = GPCMKernel::createKernel(options[kernelNoiseName],options,optimization,2*X.cols());
        tensorKernel = new GPCMCompoundKernel(options[kernelFirstName],optimization,compoundComponents,2,2*X.cols());
    }
    else
    {
        tensorKernel = NULL; // Let the GP set up the kernel.
    }

    // Create input and output matrices.
    gaussianProcess = NULL;
    setInputOutput();

    // Set up Gaussian process.
    createGaussianProcess(params,options);
}

// Evaluate the probability of a given transition.
void GPCMTransitionReward::evaluateProbability(
    const MatrixXd &means,                  // Transition means.
    const MatrixXd &vars,                   // Transition variances.
    const MatrixXd *XA,                     // Destination positions.
    int actions,                            // Number of destinations per source.
    MatrixXd *TProb                         // Probabilities to output.
    )
{
    // Now step over each action and compute probability.
    int D = X.cols();
    double C = pow(2.0*PI,-0.5*((double)D));
    for (int a = 0; a < actions; a++)
    {
        TProb[a] = (XA[a]-means).array().square().rowwise().sum();
        TProb[a] = ((-0.5*TProb[a]).cwiseQuotient(vars)).array().exp();
        //TProb[a] = TProb[a].cwiseProduct(C*(tempVars.array().pow(-0.5*((double)D)).matrix()));
    }
}

// Evaluate the probability of a given transition given the distributions.
void GPCMTransitionReward::getDistribution(
    const MatrixXd &X,                      // Input positions.
    const MatrixXd &V,                      // Input velocities.
    MatrixXd &means,                        // Means.
    MatrixXd &vars                          // Variances.
    )
{
    // Construct input matrix.
    if (bUseVelocity && !bTensorProduct)
    {
        tempInputs.resize(X.rows(),2*X.cols());
        tempInputs << X,V;
    }
    else
    {
        tempInputs = X;
    }

    // Kernalize all inputs.
    const MatrixXd *XuPtr[MAX_KERNEL_GROUPS];
    const MatrixXd *XPtr[MAX_KERNEL_GROUPS];
    memset(XuPtr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    memset(XPtr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    XPtr[0] = &tempInputs;
    XuPtr[0] = &inputMatrix;
    if (bTensorProduct)
    {
        XPtr[1] = &V;
        XuPtr[1] = &velMatrix;
    }
    tempKstar = gaussianProcess->getKernel()->covariance(XuPtr,XPtr);
    tempKdiag.resize(X.rows(),1);
    for (int i = 0; i < X.rows(); i++)
    {
        tempSingleRow = tempInputs.block(i,0,1,tempInputs.cols());
        XPtr[0] = &tempSingleRow;
        if (bTensorProduct)
        {
            tempVSingleRow = V.block(i,0,1,tempInputs.cols());
            XPtr[1] = &tempVSingleRow;
        }
        tempKdiag.block(i,0,1,1) = gaussianProcess->getKernel()->covariance(XPtr);
    }

    // Compute means and variances.
    means = tempKstar.transpose()*gaussianProcess->getAlpha();
    vars = tempKdiag - tempKstar.cwiseProduct(gaussianProcess->getKinv()*tempKstar).colwise().sum().transpose();
    if (bDifference)
        means = means + X; // If using difference mode, add X now.
}

// Helper function for creating the Gaussian process.
void GPCMTransitionReward::createGaussianProcess(
    GPCMParams &params,                     // Parameters for this object.
    GPCMOptions &options                    // All options.
    )
{
    // Set up input matrices.
    int inputs = 1;
    MatrixXd *inputMatrixPtr[2];
    MatrixXd *inputGradPtr[2];
    inputMatrixPtr[0] = &inputMatrix;
    inputGradPtr[0] = NULL;

    // If using tensor product, set up necessary inputs here.
    if (bTensorProduct)
    {
        inputs = 2;
        inputMatrixPtr[1] = &velMatrix;
        inputGradPtr[1] = NULL;
    }

    gaussianProcess = new GPCMGaussianProcess(params,options,optimization,tensorKernel,
        outputMatrix,outputMatrixScaled,inputMatrixPtr,inputGradPtr,inputs,false,false);
}

// Set input and output matrices from current X matrix.
void GPCMTransitionReward::setInputOutput()
{
    int start = 0;
    int k = 1;
    int cnt = bUseVelocity ? 2 : 1;
    for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
    {
        int end = *itr;

        // Copy X into input matrix.
        inputMatrix.block(start-k*cnt+cnt,0,end-start-cnt,X.cols()) = X.block(start+cnt-1,0,end-start-cnt,X.cols());

        // Copy V into input matrix.
        if (bUseVelocity)
        {
            if (bTensorProduct)
                velMatrix.block(start-k*cnt+cnt,0,end-start-cnt,X.cols()) = X.block(start+1,0,end-start-cnt,X.cols()) -
                                                                            X.block(start,0,end-start-cnt,X.cols());
            else
                inputMatrix.block(start-k*cnt+cnt,X.cols(),end-start-cnt,X.cols()) = X.block(start+1,0,end-start-cnt,X.cols()) -
                                                                                     X.block(start,0,end-start-cnt,X.cols());
        }

        // Copy X into output matrix.
        if (bDifference)
            outputMatrix.block(start-k*cnt+cnt,0,end-start-cnt,X.cols()) = X.block(start+cnt,0,end-start-cnt,X.cols())-
                                                                     X.block(start+cnt-1,0,end-start-cnt,X.cols());
        else
            outputMatrix.block(start-k*cnt+cnt,0,end-start-cnt,X.cols()) = X.block(start+cnt,0,end-start-cnt,X.cols());

        // Copy end to start.
        k++;
        start = end;
    }

    // Set scaled matrix.
    if (gaussianProcess)
        outputMatrixScaled = outputMatrix*gaussianProcess->getScale()(0,0);
    else
        outputMatrixScaled = outputMatrix;
}

// Copy any settings from another model that we can.
void GPCMTransitionReward::copySettings(
    GPCMTransitionReward *other             // Other transition reward to copy settings from.
    )
{
    this->gaussianProcess->copySettings(other->gaussianProcess);
    recompute(false); // Must recompute ourselves, since model doesn't recompute us.
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMTransitionReward::recomputeClosedForm()
{
    // Nothing to do here.
}

// Recompute all stored temporaries when variables change.
double GPCMTransitionReward::recompute(
    bool bNeedGradient                      // Whether to compute the gradient.
    )
{
    // Call into GP.
    return gaussianProcess->recompute(bNeedGradient);
}

// Train the model.
void GPCMTransitionReward::optimize()
{
    // If the model has autoregressive dynamics, copy its parameters here.
    if (model->getDynamics()->getType() == DynamicsTypeGP)
        gaussianProcess->getKernel()->copySettings(
            (dynamic_cast<GPCMDynamicsGP*>(model->getDynamics()))->getGaussianProcess()->getKernel());

    // Create input and output matrices.
    setInputOutput();

    // Recompute here.
    recompute(false);

    // Run optimization.
    optimization->optimize(this);
}

// Write GP data to file.
void GPCMTransitionReward::write(
    GPCMMatWriter *writer
    )
{
    // Write type.
    writer->writeString("gp","type");

    // Write difference setting.
    if (bDifference)
        writer->writeDouble(1.0,"diff");
    else
        writer->writeDouble(0.0,"diff");

    // Write velocity setting.
    if (bUseVelocity)
        writer->writeDouble(1.0,"usevel");
    else
        writer->writeDouble(0.0,"usevel");

    // Write gaussian process.
    gaussianProcess->write(writer);
}

// Load model from specified MAT file reader.
void GPCMTransitionReward::load(
    GPCMMatReader *reader
    )
{
    // Read difference setting.
    MatrixXd diffMat = reader->getVariable("diff");
    if (diffMat(0,0) == 0.0)
        bDifference = false;
    else
        bDifference = true;

    // Read velocity setting.
    MatrixXd velMat = reader->getVariable("usevel");
    if (velMat(0,0) == 0.0)
        bUseVelocity = false;
    else
        bUseVelocity = true;

    // Read gaussian process.
    gaussianProcess->load(reader);
    recompute(false); // Must recompute ourselves, since model doesn't recompute us.
}

// Destructor.
GPCMTransitionReward::~GPCMTransitionReward()
{
    delete gaussianProcess;
    delete optimization;
}
