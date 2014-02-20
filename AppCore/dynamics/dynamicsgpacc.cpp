// Second order dynamical system with passive acceleration field given by GP.

#include "debugprint.h"
#include "dynamicsgpacc.h"
#include "matwriter.h"
#include "matreader.h"
#include "gp.h"
#include "prior.h"

// Constructor.
GPCMDynamicsAccelerationGP::GPCMDynamicsAccelerationGP(
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
    ) : GPCMDynamics(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength),
        firstPointLocation(sequence.size()),
        firstPointGrad(sequence.size()),
        firstPointPrior(sequence.size()),
        secondPointLocation(sequence.size()),
        secondPointGrad(sequence.size()),
        secondPointPrior(sequence.size())
{
    // Initialize input and output matrices.
    inputGrad.resize(X.rows()-sequence.size()*2,X.cols());
    inputMatrix.resize(X.rows()-sequence.size()*2,X.cols());
    outputMatrix.resize(X.rows()-sequence.size()*2,X.cols());
    outputMatrixScaled.resize(X.rows()-sequence.size()*2,X.cols());
    gaussianProcess = NULL;
    setInputOutput();

    // Set frame length.
    this->frameLength = frameLength;

    // Create a prior on the first and second points in each trajectory.
    int start = 0;
    for (unsigned i = 0; i < sequence.size(); i++)
    {
        firstPointLocation[i] = X.block(start,0,1,X.cols());
        secondPointLocation[i] = X.block(start+1,0,1,X.cols());
        firstPointGrad[i] = Xgrad.block(start,0,1,X.cols());
        secondPointGrad[i] = Xgrad.block(start+1,0,1,X.cols());
        firstPointPrior[i] = GPCMPrior::createPrior(params["first_point_prior"][0],
            atof(params["first_point_prior_weight"][0].c_str()),&firstPointLocation[i],&firstPointGrad[i]);
        secondPointPrior[i] = GPCMPrior::createPrior(params["first_point_prior"][0],
            atof(params["first_point_prior_weight"][0].c_str()),&secondPointLocation[i],&secondPointGrad[i]);
        start = sequence[i];
    }

    // Create the Gaussian process.
    if (!bPriorsOnly)
        createGaussianProcess(params,options,optimization);

    // Set type.
    this->type = DynamicsTypeGP;
}

// Copy any settings from another model that we can.
void GPCMDynamicsAccelerationGP::copySettings(
    GPCMDynamics *other                     // Dynamics object to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMDynamicsAccelerationGP *othercst = dynamic_cast<GPCMDynamicsAccelerationGP*>(other);
        this->gaussianProcess->copySettings(othercst->gaussianProcess);
    }
    else
    {
        DBWARNING("Dynamics type mismatch when initializing dynamics parameters from another kernel!");
    }
}

// Helper function for creating the Gaussian process.
void GPCMDynamicsAccelerationGP::createGaussianProcess(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization          // Optimization object to add new variables to.
    )
{
    MatrixXd *inputMatrixPtr = &inputMatrix;
    MatrixXd *inputGradPtr = &inputGrad;
    gaussianProcess = new GPCMGaussianProcess(params,options,optimization,NULL,
        outputMatrix,outputMatrixScaled,&inputMatrixPtr,&inputGradPtr,1,false,false);
    gaussianProcess->getScale() *= 1.0/(frameLength*frameLength);
}

// Set input and output matrices from current X matrix.
void GPCMDynamicsAccelerationGP::setInputOutput()
{
    int start = 0;
    int k = 1;
    for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
    {
        int end = *itr;

        // Copy X into input matrix, which holds the points at which accelerations are evaluated.
        inputMatrix.block(start-k*2+2,0,end-start-2,X.cols()) = X.block(start+1,0,end-start-2,X.cols());

        // Copy X into output matrix, which holds accelerations.
        outputMatrix.block(start-k*2+2,0,end-start-2,X.cols()) = X.block(start+2,0,end-start-2,X.cols())-
                                                             2.0*X.block(start+1,0,end-start-2,X.cols())+
                                                                 X.block(start+0,0,end-start-2,X.cols());

        // Copy end to start.
        k++;
        start = end;
    }

    // Set scaled matrix.
    if (gaussianProcess)
        outputMatrixScaled = outputMatrix.cwiseProduct(gaussianProcess->getScale().replicate(outputMatrix.rows(),1));
    else
        outputMatrixScaled = outputMatrix;

    // Clear gradient.
    inputGrad.setZero(inputGrad.rows(),inputGrad.cols());
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMDynamicsAccelerationGP::recomputeClosedForm()
{
    gaussianProcess->recomputeClosedForm();
}

// Recompute all stored temporaries when variables change.
double GPCMDynamicsAccelerationGP::recompute(
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
        for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
        {
            int end = *itr;

            // Add input gradients to Xgrad.
            Xgrad.block(start+1,0,end-start-2,X.cols()) += inputGrad.block(start-k*2+2,0,end-start-2,X.cols());

            // Compute gradients with respect to "Y".
            Xgrad.block(start+2,0,end-start-2,X.cols()) -=
                alpha.block(start-k*2+2,0,end-start-2,X.cols()).cwiseProduct(scale.replicate(end-start-2,1));
            Xgrad.block(start+1,0,end-start-2,X.cols()) += 2.0*
                alpha.block(start-k*2+2,0,end-start-2,X.cols()).cwiseProduct(scale.replicate(end-start-2,1));
            Xgrad.block(start+0,0,end-start-2,X.cols()) -=
                alpha.block(start-k*2+2,0,end-start-2,X.cols()).cwiseProduct(scale.replicate(end-start-2,1));

            // Add any priors on the first and second point in the trajectory.
            firstPointLocation[k-1]  = X.block(start+0,0,1,X.cols());
            secondPointLocation[k-1] = X.block(start+1,0,1,X.cols());
            firstPointGrad[k-1].setZero(1,X.cols());
            secondPointGrad[k-1].setZero(1,X.cols());
            loglikelihood += firstPointPrior[k-1]->recompute();
            loglikelihood += secondPointPrior[k-1]->recompute();
            Xgrad.block(start+0,0,1,X.cols()) += firstPointGrad[k-1];
            Xgrad.block(start+1,0,1,X.cols()) += secondPointGrad[k-1];

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
        for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
        {
            int end = *itr;

            // Add any priors on the first point in the trajectory.
            firstPointLocation[k-1] = X.block(start,0,1,X.cols());
            secondPointLocation[k-1] = X.block(start+1,0,1,X.cols());
            firstPointGrad[k-1].setZero(1,X.cols());
            secondPointGrad[k-1].setZero(1,X.cols());
            loglikelihood += firstPointPrior[k-1]->recompute();
            loglikelihood += secondPointPrior[k-1]->recompute();

            // Copy end to start.
            k++;
            start = end;
        }

    }

    // Return result.
    return loglikelihood;
}

// Write dynamics data to file.
void GPCMDynamicsAccelerationGP::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write superclass data.
    GPCMDynamics::write(writer);

    // Write type.
    writer->writeString("gpAccDynamics","type");

    // Write GP info.
    gaussianProcess->write(writer);

    // Write prior info.
    GPCMMatWriter *priorStruct = writer->writeStruct("prior",1,1);
    firstPointPrior[0]->write(priorStruct);
    writer->closeStruct();
}

// Load model from specified MAT file reader.
void GPCMDynamicsAccelerationGP::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read superclass data.
    GPCMDynamics::load(reader);

    // Read GP info.
    gaussianProcess->load(reader);
}

// Destructor.
GPCMDynamicsAccelerationGP::~GPCMDynamicsAccelerationGP()
{
    for (unsigned i = 0; i < firstPointPrior.size(); i++)
    {
        delete firstPointPrior[i];
        delete secondPointPrior[i];
    }
    delete gaussianProcess;
}
