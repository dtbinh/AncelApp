// Gaussian prior.

#include "gaussprior.h"
#include "matwriter.h"
#include "mathutils.h"

// Constructor.
GPCMGaussPrior::GPCMGaussPrior(
    MatrixXd *data,                         // Variable to put a prior on.
    MatrixXd *gradient,                     // Where to store the prior gradient.
    double c                                // Prior coefficient.
    ) : GPCMPrior(data,gradient), c(c)
{
}

// Write the prior into the specific writer.
void GPCMGaussPrior::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    writer->writeString("gaussian","type");
    writer->writeDouble(c,"precision");

    // Write transforms struct.
    GPCMMatWriter *xformStruct = writer->writeStruct("transforms",1,1);
    xformStruct->writeDouble(1.0,"index");
    xformStruct->writeString("exp","type");
    writer->closeStruct();
}

// Recompute gradient and return log likelihood.
double GPCMGaussPrior::recompute()
{
    int D = data->rows()*data->cols();
    if (gradient)
        *gradient += -c*(*data);
    return -0.5*c*data->array().square().sum() - 0.5*log(2.0*PI)*D + 0.5*log(c)*D;
}

// Get precision.
double GPCMGaussPrior::getPrecision()
{
    return c;
}
