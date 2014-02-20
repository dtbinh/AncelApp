// Logarithmic prior.

#include "logprior.h"
#include "matwriter.h"

// Constructor.
GPCMLogPrior::GPCMLogPrior(
    MatrixXd *data,                         // Variable to put a prior on.
    MatrixXd *gradient,                     // Where to store the prior gradient.
    double c                                // Prior coefficient.
    ) : GPCMPrior(data,gradient), c(c)
{
}

// Write the prior into the specific writer.
void GPCMLogPrior::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    writer->writeString("wang","type");
    writer->writeDouble(c,"M");

    // Write transforms struct.
    GPCMMatWriter *xformStruct = writer->writeStruct("transforms",1,1);
    xformStruct->writeDouble(1.0,"index");
    xformStruct->writeString("exp","type");
    writer->closeStruct();
}

// Recompute gradient and return log likelihood.
double GPCMLogPrior::recompute()
{
    if (gradient)
        *gradient += -c*data->array().inverse().matrix();
    return -c*data->array().log().sum();
}
