// Abstract prior for an optimization variable.

#include "debugprint.h"
#include "noprior.h"
#include "logprior.h"
#include "gaussprior.h"
#include "prior.h"
#include "matwriter.h"

// Create a prior.
GPCMPrior *GPCMPrior::createPrior(
    std::string type,                       // Desired prior type.
    double weight,                          // Desired prior weight.
    MatrixXd *data,                         // Variables to put the prior on.
    MatrixXd *gradient                      // Prior gradient.
    )
{
    // Create and return the desired prior.
    if (!type.compare("none"))
        return new GPCMNoPrior(data,gradient);
    else if (!type.compare("log"))
        return new GPCMLogPrior(data,gradient,weight);
    else if (!type.compare("gauss"))
        return new GPCMGaussPrior(data,gradient,weight);
    else
        DBERROR("Unknown prior " << type << " requested.");
    return NULL;
}

// Write the prior into the specific writer.
void GPCMPrior::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    writer->writeDouble((double)(data->cols()*data->rows()),"nParams");
}

// Constructor.
GPCMPrior::GPCMPrior(
    MatrixXd *data,                         // Variable to put a prior on.
    MatrixXd *gradient                      // Where to store the prior gradient.
    ) : data(data), gradient(gradient)
{
}
