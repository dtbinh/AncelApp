// First order autoregressive gaussian process dynamics prior.

#include "debugprint.h"
#include "dynamicsgp.h"
#include "matwriter.h"
#include "matreader.h"
#include "gp.h"
#include "prior.h"
#include "tensorkernel.h"
#include "compoundkernel.h"

// Constructor.
GPCMDynamicsGP::GPCMDynamicsGP(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    MatrixXd &dataMatrix,                   // Pointer to data matrix.
    GPCMModel *model,                       // Pointer to model.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    double frameLength,                     // Length of a single frame.
    bool bPriorsOnly                        // Create only priors.
    ) : GPCMDynamics(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength)
{
    // Decide if we're using difference mode.
    if (!params["difference_mode"][0].compare("true") ||
        !params["difference_mode"][0].compare("1"))
        bDifference = true;
    else
        bDifference = false;

    // Decide if we're using velocity mode.
    if (!params["use_velocity"].empty() &&
        (!params["use_velocity"][0].compare("true") ||
         !params["use_velocity"][0].compare("1")))
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

    // Can't use tensor product without velocity.
    assert((bTensorProduct && bUseVelocity) || !bTensorProduct);

    if (bUseVelocity)
    {
        firstPointLocation.resize(2*sequence.size());
        firstPointGrad.resize(2*sequence.size());
        firstPointPrior.resize(2*sequence.size());
    }
    else
    {
        firstPointLocation.resize(sequence.size());
        firstPointGrad.resize(sequence.size());
        firstPointPrior.resize(sequence.size());
    }

    // Get weight (scale).
    weight = atof(params["weight"][0].c_str());

    // Initialize input and output matrices.
    int cnt = bUseVelocity ? 2 : 1;
    gaussianProcess = NULL;
    if (bTensorProduct)
    {
        inputGrad.resize(X.rows()-sequence.size()*cnt,X.cols());
        inputMatrix.resize(X.rows()-sequence.size()*cnt,X.cols());
        velGrad.resize(X.rows()-sequence.size()*cnt,X.cols());
        velMatrix.resize(X.rows()-sequence.size()*cnt,X.cols());
    }
    else
    {
        inputGrad.resize(X.rows()-sequence.size()*cnt,X.cols()*cnt);
        inputMatrix.resize(X.rows()-sequence.size()*cnt,X.cols()*cnt);
    }
    outputMatrix.resize(X.rows()-sequence.size()*cnt,X.cols());
    outputMatrixScaled.resize(X.rows()-sequence.size()*cnt,X.cols());
    setInputOutput();

    // Create a prior on the first point in each trajectory.
    int start = 0;
    for (unsigned i = 0; i < sequence.size(); i++)
    {
        for (int j = 0; j < cnt; j++)
        {
            firstPointLocation[i*cnt+j] = X.block(start+j,0,1,X.cols());
            firstPointGrad[i*cnt+j] = Xgrad.block(start+j,0,1,X.cols());
            firstPointPrior[i*cnt+j] = GPCMPrior::createPrior(params["first_point_prior"][0],
                atof(params["first_point_prior_weight"][0].c_str()),&firstPointLocation[i*cnt+j],&firstPointGrad[i*cnt+j]);
        }
        start = sequence[i];
    }

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

    // Create the Gaussian process.
    if (!bPriorsOnly)
        createGaussianProcess(params,options,optimization);

    // Set type.
    this->type = DynamicsTypeGP;
}

// Get underlying Gaussian process.
GPCMGaussianProcess *GPCMDynamicsGP::getGaussianProcess()
{
    return gaussianProcess;
}

// Copy any settings from another model that we can.
void GPCMDynamicsGP::copySettings(
    GPCMDynamics *other                     // Dynamics object to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMDynamicsGP *othercst = dynamic_cast<GPCMDynamicsGP*>(other);
        this->gaussianProcess->copySettings(othercst->gaussianProcess);
    }
    else
    {
        DBWARNING("Dynamics type mismatch when initializing dynamics parameters from another kernel!");
    }
}

// Helper function for creating the Gaussian process.
void GPCMDynamicsGP::createGaussianProcess(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization          // Optimization object to add new variables to.
    )
{
    // Set up input matrices.
    int inputs = 1;
    MatrixXd *inputMatrixPtr[2];
    MatrixXd *inputGradPtr[2];
    inputMatrixPtr[0] = &inputMatrix;
    inputGradPtr[0] = &inputGrad;

    // If using tensor product, set up necessary inputs here.
    if (bTensorProduct)
    {
        inputs = 2;
        inputMatrixPtr[1] = &velMatrix;
        inputGradPtr[1] = &velGrad;
    }

    // Create Gaussian process.
    gaussianProcess = new GPCMGaussianProcess(params,options,optimization,tensorKernel,
        outputMatrix,outputMatrixScaled,inputMatrixPtr,inputGradPtr,inputs,false,false);
    gaussianProcess->getScale() *= weight;
}

// Set input and output matrices from current X matrix.
void GPCMDynamicsGP::setInputOutput()
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

    // Clear gradient.
    inputGrad.setZero(inputGrad.rows(),inputGrad.cols());
    if (bTensorProduct)
        velGrad.setZero(velGrad.rows(),velGrad.cols());
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMDynamicsGP::recomputeClosedForm()
{
    gaussianProcess->recomputeClosedForm();
}

// Recompute all stored temporaries when variables change.
double GPCMDynamicsGP::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // Set input and output matrices.
    setInputOutput();

    // Compute GP likelihood and gradients.
    double loglikelihood = gaussianProcess->recompute(bNeedGradient);

    if (bNeedGradient)
    {
        const MatrixXd &alpha = gaussianProcess->getAlpha();
        const MatrixXd &scale = gaussianProcess->getScale();
        int start = 0;
        int k = 1;
        int cnt = bUseVelocity ? 2 : 1;
        for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
        {
            int end = *itr;

            // Add input gradients to Xgrad.
            Xgrad.block(start+cnt-1,0,end-start-cnt,X.cols()) += inputGrad.block(start-k*cnt+cnt,0,end-start-cnt,X.cols());

            // Add velocity gradient to Xgrad.
            if (bUseVelocity)
            {
                if (bTensorProduct)
                {
                    Xgrad.block(start+1,0,end-start-cnt,X.cols()) += velGrad.block(start-k*cnt+cnt,0,end-start-cnt,X.cols());
                    Xgrad.block(start,0,end-start-cnt,X.cols()) -= velGrad.block(start-k*cnt+cnt,0,end-start-cnt,X.cols());
                }
                else
                {
                    Xgrad.block(start+1,0,end-start-cnt,X.cols()) += inputGrad.block(start-k*cnt+cnt,X.cols(),end-start-cnt,X.cols());
                    Xgrad.block(start,0,end-start-cnt,X.cols()) -= inputGrad.block(start-k*cnt+cnt,X.cols(),end-start-cnt,X.cols());
                }
            }

            // Compute gradients with respect to "Y".
            if (bDifference)
            {
                Xgrad.block(start+cnt,0,end-start-cnt,X.cols()) -=
                    alpha.block(start-k*cnt+cnt,0,end-start-cnt,X.cols()).cwiseProduct(scale.replicate(end-start-cnt,1));
                Xgrad.block(start+cnt-1,0,end-start-cnt,X.cols()) +=
                    alpha.block(start-k*cnt+cnt,0,end-start-cnt,X.cols()).cwiseProduct(scale.replicate(end-start-cnt,1));
            }
            else
            {
                Xgrad.block(start+cnt,0,end-start-cnt,X.cols()) -=
                    alpha.block(start-k*cnt+cnt,0,end-start-cnt,X.cols()).cwiseProduct(scale.replicate(end-start-cnt,1));
            }

            // Add any priors on the first point in the trajectory.
            for (int j = 0; j < cnt; j++)
            {
                firstPointLocation[(k-1)*cnt+j] = X.block(start+j,0,1,X.cols());
                firstPointGrad[(k-1)*cnt+j].setZero(1,X.cols());
                loglikelihood += firstPointPrior[(k-1)*cnt+j]->recompute();
                Xgrad.block(start+j,0,1,X.cols()) += firstPointGrad[(k-1)*cnt+j];
            }

            // Copy end to start.
            k++;
            start = end;
        }
    }
    else
    {
        // Only compute prior magnitude.
        int start = 0;
        int k = 1;
        int cnt = bUseVelocity ? 2 : 1;
        for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
        {
            int end = *itr;

            // Add any priors on the first point in the trajectory.
            for (int j = 0; j < cnt; j++)
            {
                firstPointLocation[(k-1)*cnt+j] = X.block(start+j,0,1,X.cols());
                firstPointGrad[(k-1)*cnt+j].setZero(1,X.cols());
                loglikelihood += firstPointPrior[(k-1)*cnt+j]->recompute();
            }

            // Copy end to start.
            k++;
            start = end;
        }

    }

    // Return result.
    return loglikelihood;
}

// Write dynamics data to file.
void GPCMDynamicsGP::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write superclass data.
    GPCMDynamics::write(writer);

    // Write type.
    writer->writeString("gpDynamics","type");

    // Write diff.
    if (bDifference)
        writer->writeDouble(1.0,"diff");
    else
        writer->writeDouble(0.0,"diff");

    // Write velocity.
    if (bUseVelocity)
        writer->writeDouble(1.0,"vel");
    else
        writer->writeDouble(0.0,"vel");

    // Write GP info.
    gaussianProcess->write(writer);

    // Write prior info.
    GPCMMatWriter *priorStruct = writer->writeStruct("prior",1,1);
    firstPointPrior[0]->write(priorStruct);
    writer->closeStruct();
}

// Load model from specified MAT file reader.
void GPCMDynamicsGP::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read superclass data.
    GPCMDynamics::load(reader);

    // Read diff.
    MatrixXd diffMat = reader->getVariable("diff");
    if (diffMat(0,0) == 0.0)
        bDifference = false;
    else
        bDifference = true;

    // Read velocity.
    MatrixXd velMat = reader->getVariable("vel");
    if (velMat(0,0) == 0.0)
        bUseVelocity = false;
    else
        bUseVelocity = true;

    // Read GP info.
    gaussianProcess->load(reader);
}

// Destructor.
GPCMDynamicsGP::~GPCMDynamicsGP()
{
    for (unsigned i = 0; i < firstPointPrior.size(); i++)
    {
        delete firstPointPrior[i];
    }
    delete gaussianProcess;
}
