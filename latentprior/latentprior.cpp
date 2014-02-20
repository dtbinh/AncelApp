// GPCM latent prior term.

#include "debugprint.h"
#include "latentprior.h"
#include "matwriter.h"
#include "task.h"
#include "connectivity.h"

// Global function for creating new latent prior term.
GPCMLatentPrior *GPCMLatentPrior::createLatentPrior(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    GPCMTask *task,                         // Pointer to the task.
    GPCMController *controller              // Pointer to the controller object.
    )
{
    std::string type = params["type"][0];
    // Create and return the desired dynamics.
    if (!type.compare("none"))
        return NULL;
    else if (!type.compare("connectivity"))
        return new GPCMConnectivity(params,options,optimization,X,Xgrad,sequence,task,controller);
    else
        DBERROR("Unknown value term " << type << " requested.");
    return NULL;
}

// Constructor.
GPCMLatentPrior::GPCMLatentPrior(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    GPCMTask *task,                         // Pointer to the task.
    GPCMController *controller              // Pointer to the controller object.
    ) : X(X), Xgrad(Xgrad), sequence(sequence), type(ValueTypeUnknown), controller(controller)
{
}

// Copy any settings from another model that we can.
void GPCMLatentPrior::copySettings(
    GPCMLatentPrior *other                    // Model to copy from.
    )
{
}

// Get the type of this latent prior object.
GPCMLatentPriorType GPCMLatentPrior::getType()
{
    return type;
}

// Set new controller.
void GPCMLatentPrior::setController(
    GPCMController *controller              // New controller.
    )
{
    this->controller = controller;
}

// Load model from specified MAT file reader.
void GPCMLatentPrior::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
}

// Write dynamics data to file.
void GPCMLatentPrior::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
}

// Destructor.
GPCMLatentPrior::~GPCMLatentPrior()
{
}
