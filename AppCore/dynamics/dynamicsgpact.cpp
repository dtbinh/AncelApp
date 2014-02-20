// First order autoregressive gaussian process dynamics prior with latent actions.

#include "debugprint.h"
#include "dynamicsgpact.h"
#include "gp.h"
#include "prior.h"
#include "optimization.h"
#include "tensorkernel.h"
#include "compoundkernel.h"
#include "matwriter.h"

// Constructor.
GPCMDynamicsActionGP::GPCMDynamicsActionGP(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    MatrixXd &dataMatrix,                   // Pointer to data matrix.
    GPCMModel *model,                       // Pointer to model.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    double frameLength                      // Length of a single frame.
    ) : GPCMDynamicsGP(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength,true)
{
    // Create the Gaussian process.
    createGaussianProcess(params,options,optimization);

    // Create action prior.
    actionPrior = GPCMPrior::createPrior(params["action_prior"][0],
            atof(params["action_prior_weight"][0].c_str()),&A,&Agrad);

    // Set type.
    this->type = DynamicsTypeActionGP;
}

// Copy any settings from another model that we can.
void GPCMDynamicsActionGP::copySettings(
    GPCMDynamics *other                     // Dynamics object to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMDynamicsActionGP *othercst = dynamic_cast<GPCMDynamicsActionGP*>(other);
        
        // Copy the gaussian process.
        this->gaussianProcess->copySettings(othercst->gaussianProcess);

        // Copy the latent action space.
        if (othercst->A.cols() == this->A.cols())
        {
            // Copy actions directly.
            this->A = othercst->A;
        }
        else if (A.cols() == inputMatrix.cols())
        {
            // Initialize from accelerations.
            int start = 0;
            int k = 1;
            for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
            {
                int end = *itr;
                
                // Write all but the first entry of A for this block, copy second entry into first.
                A.block(start-k+2,0,end-start-2,X.cols()) = X.block(start+2,0,end-start-2,X.cols())-
                                                        2.0*X.block(start+1,0,end-start-2,X.cols())+
                                                            X.block(start+0,0,end-start-2,X.cols());
                A.block(start-k+1,0,1,X.cols()) = A.block(start-k+2,0,1,X.cols());

                // Copy end to start.
                k++;
                start = end;
            }
        }
        else
        {
            assert(false);
        }
    }
    else
    {
        DBWARNING("Dynamics type mismatch when initializing dynamics parameters from another kernel!");
    }
}

// Helper function for creating the Gaussian process.
void GPCMDynamicsActionGP::createGaussianProcess(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization          // Optimization object to add new variables to.
    )
{
    // Get number of action dimensions.
    int adim;
    if (!options["embedding"]["action_dimensions"][0].compare("auto"))
        adim = inputMatrix.cols();
    else
        adim = atoi(options["embedding"]["action_dimensions"][0].c_str());

    // Create action matrix.
    A.resize(inputMatrix.rows(),adim);
    Agrad.resize(inputMatrix.rows(),adim);
    
    // Initialize action matrix from accelerations.
    if (adim != inputMatrix.cols())
        DBERROR("When initializing latent actions with accelerations, action dimensionality must equal latent dimensionality!");
    int start = 0;
    int k = 1;
    for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
    {
        int end = *itr;
        
        // Write all but the first entry of A for this block, copy second entry into first.
        A.block(start-k+2,0,end-start-2,X.cols()) = X.block(start+2,0,end-start-2,X.cols())-
                                                2.0*X.block(start+1,0,end-start-2,X.cols())+
                                                    X.block(start+0,0,end-start-2,X.cols());
        A.block(start-k+1,0,1,X.cols()) = A.block(start-k+2,0,1,X.cols());

        // Copy end to start.
        k++;
        start = end;
    }

    // Add action matrix optimization variables.
    optimization->addVariable(VarXformNone,&A,&Agrad,"A");

    // Create split kernel for latent position and actions.
    std::string kernelXName = params["kernel_x"][0];
    std::string kernelThetaName = params["kernel_action"][0];
    std::string kernelNoiseName = params["kernel_noise"][0];
    GPCMKernel *tensorComponents[2];
    GPCMKernel *compoundComponents[2];
    tensorComponents[0] = GPCMKernel::createKernel(options[kernelXName],options,optimization,X.cols());
    tensorComponents[1] = GPCMKernel::createKernel(options[kernelThetaName],options,optimization,A.cols());
    compoundComponents[0] = new GPCMTensorKernel(options[kernelXName],optimization,tensorComponents,2,X.cols()+A.cols());
    compoundComponents[1] = GPCMKernel::createKernel(options[kernelNoiseName],options,optimization,X.cols()+A.cols());
    GPCMKernel *kernel = new GPCMCompoundKernel(options[kernelXName],optimization,compoundComponents,2,X.cols()+A.cols());

    // Create Gaussian process.
    MatrixXd *inputMatrixPtr[2] = { &inputMatrix, &A };
    MatrixXd *inputGradPtr[2] = { &inputGrad, &Agrad };
    gaussianProcess = new GPCMGaussianProcess(params,options,optimization,kernel,
        outputMatrix,outputMatrix,inputMatrixPtr,inputGradPtr,2,false,false);
    gaussianProcess->getScale() *= weight;
}

// Recompute all stored temporaries when variables change.
double GPCMDynamicsActionGP::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // Recompute parent.
    double likelihood = GPCMDynamicsGP::recompute(bNeedGradient);

    // Update action prior.
    if (actionPrior)
        likelihood += actionPrior->recompute();

    // Return result.
    return likelihood;
}

// Write dynamics data to file.
void GPCMDynamicsActionGP::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write superclass data.
    GPCMDynamicsGP::write(writer);

    // Write second component type.
    writer->writeString("action","second_input");
}

// Destructor.
GPCMDynamicsActionGP::~GPCMDynamicsActionGP()
{
    delete actionPrior;
}
