// Abstract dynamics prior.

#include "debugprint.h"
#include "dynamics.h"
#include "dynamicsgp.h"
#include "dynamicsgpact.h"
#include "dynamicsgpacc.h"
#include "dynamicsacc.h"
#include "dynamicstimegp.h"
#include "matwriter.h"
#include "matreader.h"

// Global function for creating new dynamics.
GPCMDynamics *GPCMDynamics::createDynamics(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    MatrixXd &dataMatrix,                   // Pointer to data matrix.
    GPCMModel *model,                       // Pointer to model.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    double frameLength                      // Length of a single frame.
    )
{
    std::string type = params["type"][0];
    // Create and return the desired dynamics.
    if (!type.compare("none"))
        return NULL;
    else if (!type.compare("second_order_finite_differences"))
        return new GPCMDynamicsAcceleration(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength);
    else if (!type.compare("autoregressive_gaussian_process"))
        return new GPCMDynamicsGP(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength);
    else if (!type.compare("autoregressive_gaussian_process_latent_actions"))
        return new GPCMDynamicsActionGP(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength);
    else if (!type.compare("gaussian_process_acceleration"))
        return new GPCMDynamicsAccelerationGP(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength);
    else if (!type.compare("regressive_gaussian_process"))
        return new GPCMDynamicsTimeGP(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength);
    else
        DBERROR("Unknown dynamics " << type << " requested.");
    return NULL;
}

// Constructor.
GPCMDynamics::GPCMDynamics(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    MatrixXd &dataMatrix,                   // Pointer to data matrix.
    GPCMModel *model,                       // Pointer to model.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    double frameLength                      // Length of a single frame.
    ) : X(X), Xgrad(Xgrad), sequence(sequence), type(DynamicsTypeUnknown)
{
}

// Copy any settings from another model that we can.
void GPCMDynamics::copySettings(
    GPCMDynamics *other                     // Dynamics object to copy from.
    )
{
}

// Get the type of this dynamics object.
GPCMDynamicsType GPCMDynamics::getType()
{
    return type;
}

// Set value term.
void GPCMDynamics::setValueTerm(
    GPCMLatentPrior *value					// New value term to use.
    )
{
    // Generally this is not required.
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMDynamics::recomputeClosedForm()
{
}

// Write dynamics data to file.
void GPCMDynamics::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write sequence array.
    MatrixXd seqMat(1,sequence.size());
    int k = 0;
    for (std::vector<int>::iterator itr = sequence.begin();
         itr != sequence.end(); itr++, k++)
    {
        seqMat(0,k) = *itr;
    }
    writer->writeMatrix(seqMat,"seq");
}

// Load model from specified MAT file reader.
void GPCMDynamics::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read in the sequence data.
    MatrixXd seqMat = reader->getVariable("seq");
    sequence.clear();
    for (int i = 0; i < seqMat.cols(); i++)
        sequence.push_back((int)(seqMat(0,i)));
}

// Destructor.
GPCMDynamics::~GPCMDynamics()
{
}
