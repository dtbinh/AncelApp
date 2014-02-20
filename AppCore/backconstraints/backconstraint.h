// Abstract back constraint function.
#pragma once

#include "options.h"

#include <string>

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;
class GPCMMatReader;
class GPCMOptimization;

// Possible types of back constraints.
enum GPCMBackConstraintType
{
    BackConstraintMLP,
    BackConstraintKBR,
    BackConstraintGP
};

class GPCMBackConstraint
{
protected:
    // Back constraint type for export.
    std::string name;
    // Back constraint type for internal use.
    GPCMBackConstraintType type;
    // Stored data matrix.
    MatrixXd &dataMatrix;
    // Stored latent positions matrix.
    MatrixXd &X;
public:
    // Global function for creating a new back constraint function.
    static GPCMBackConstraint *createBackConstraint(GPCMParams &params, GPCMOptions &options,
        GPCMOptimization *optimization, MatrixXd &dataMatrix, MatrixXd &X);
    // Construct the back constraint function.
    GPCMBackConstraint(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &dataMatrix, MatrixXd &X);
    // Copy any settings from another back constraint function that we can.
    virtual void copySettings(GPCMBackConstraint *other);
    // Get the type of this back constraint function.
    virtual GPCMBackConstraintType getType();
    // Initialize the back constraint function by optimizing the parameters for an initial latent matrix.
    virtual void initialize() = 0;
    // Compute the gradients of this back constraint function using the gradient of the latent coordinates.
    virtual double updateGradient(MatrixXd &Xgrad, bool bNeedGradient) = 0;
    // Update the latent coordinates based on the current parameters.
    virtual void updateLatentCoords() = 0;
    // Write back constraint to file.
    virtual void write(GPCMMatWriter *writer);
    // Load parameters from specified MAT file reader.
    virtual void load(GPCMMatReader *reader) = 0;
    // Destructor.
    virtual ~GPCMBackConstraint();
};
