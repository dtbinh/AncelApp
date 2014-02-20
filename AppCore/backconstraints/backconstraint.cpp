// Abstract back constraint function.

#include "debugprint.h"
#include "backconstraintmlp.h"
#include "backconstraintkbr.h"
#include "backconstraintgp.h"
#include "matwriter.h"
#include "matreader.h"
#include "options.h"

// Global function for creating a new back constraint function.
GPCMBackConstraint *GPCMBackConstraint::createBackConstraint(
    GPCMParams &params,                     // Parameters of this kernel.
    GPCMOptions &options,                   // Loaded options used for creating other kernels.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &dataMatrix,                   // Matrix of Y values.
    MatrixXd &X                             // Dimensionality of latent space.
    )
{
    std::string type = params["type"][0];
    // Create and return the desired dynamics.
    if (!type.compare("none"))
        return NULL;
    else if (!type.compare("mlp"))
        return new GPCMBackConstraintMLP(params,options,optimization,dataMatrix,X);
    else if (!type.compare("kbr"))
        return new GPCMBackConstraintKBR(params,options,optimization,dataMatrix,X);
    else if (!type.compare("gp"))
        return new GPCMBackConstraintGP(params,options,optimization,dataMatrix,X);
    else
        DBERROR("Unknown back constraint " << type << " requested.");
    return NULL;
}

// Construct the back constraint function.
GPCMBackConstraint::GPCMBackConstraint(
    GPCMParams &params,                     // Parameters of this kernel.
    GPCMOptions &options,                   // Loaded options used for creating other kernels.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &dataMatrix,                   // Matrix of Y values.
    MatrixXd &X                             // Dimensionality of latent space.
    ) : dataMatrix(dataMatrix), X(X)
{
    // Nothing to do here.
}

// Copy any settings from another back constraint function that we can.
void GPCMBackConstraint::copySettings(
    GPCMBackConstraint *other               // Function to copy parameters from.
    )
{
}

// Get the type of this back constraint function.
GPCMBackConstraintType GPCMBackConstraint::getType()
{
    return type;
}

// Write back constraint to file.
void GPCMBackConstraint::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write type.
    writer->writeString(name,"type");
}

// Destructor.
GPCMBackConstraint::~GPCMBackConstraint()
{
}
