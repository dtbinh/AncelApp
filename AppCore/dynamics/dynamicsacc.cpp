// Second order finite differences dynamics prior.

#include "debugprint.h"
#include "dynamicsacc.h"
#include "matwriter.h"
#include "matreader.h"

// Constructor.
GPCMDynamicsAcceleration::GPCMDynamicsAcceleration(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    MatrixXd &dataMatrix,                   // Pointer to data matrix.
    GPCMModel *model,                       // Pointer to model.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    double frameLength                      // Length of a single frame.
    ) : GPCMDynamics(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength)
{
    // Read the weight.
    weight = atof(params["weight"][0].c_str());

    // Set type.
    this->type = DynamicsTypeAcceleration;
}

// Copy any settings from another model that we can.
void GPCMDynamicsAcceleration::copySettings(
    GPCMDynamics *other                     // Dynamics object to copy from.
    )
{
    if (other->getType() == getType())
    {
        // Nothing to copy.
    }
    else
    {
        DBWARNING("Dynamics type mismatch when initializing dynamics parameters from another kernel!");
    }
}

// Recompute all stored temporaries when variables change.
double GPCMDynamicsAcceleration::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    int start = 0;
    int q = X.cols();
    double loglikelihood = 0.0;
    for (std::vector<int>::iterator itr = sequence.begin();
         itr != sequence.end(); itr++)
    {
        // Get the ending index for this sequence.
        int end = *itr;

        // Compute second derivatives with central differencing.
        MatrixXd sd = X.block(start+2,0,end-start-2,q)-
                      X.block(start+1,0,end-start-2,q)*2.0+
                      X.block(start+0,0,end-start-2,q);

        // Subtract square second derivatives from likelihood.
        loglikelihood -= sd.array().square().sum()*0.5*weight;

        // Subtract square priors on first and last elements.
        loglikelihood -= X.block(start,0,1,q).array().square().sum()*0.5*weight;
        loglikelihood -= X.block(end-1,0,1,q).array().square().sum()*0.5*weight;

        // Compute gradients.
        if (bNeedGradient)
        {
            // Subtract derivatives from next entries.
            Xgrad.block(start+2,0,end-start-2,q) -= sd*weight;

            // Subtract derivatives from previous entries.
            Xgrad.block(start+0,0,end-start-2,q) -= sd*weight;

            // Add double derivatives to central entries.
            Xgrad.block(start+1,0,end-start-2,q) += sd*2.0*weight;

            // Subtract derivatives from first and last entries.
            Xgrad.block(start,0,1,q) -= X.block(start,0,1,q)*weight;
            Xgrad.block(end-1,0,1,q) -= X.block(end-1,0,1,q)*weight;
        }

        // Move on to the next sequence.
        start = end;
    }

    // Return result.
    return loglikelihood;
}

// Write dynamics data to file.
void GPCMDynamicsAcceleration::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write superclass data.
    GPCMDynamics::write(writer);

    // Write type.
    writer->writeString("fd2Dynamics","type");

    // Write weight.
    writer->writeDouble(weight,"weight");
}

// Load model from specified MAT file reader.
void GPCMDynamicsAcceleration::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read superclass data.
    GPCMDynamics::load(reader);

    // Read weight.
    MatrixXd wtMat = reader->getVariable("weight");
    weight = wtMat(0,0);
}

// Destructor.
GPCMDynamicsAcceleration::~GPCMDynamicsAcceleration()
{
}
