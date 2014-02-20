// Multi-layer perceptron back constraint function.
#pragma once

#include "backconstraint.h"

// Forward declarations.
class GPCMOptAlgorithm;

// Possible types of activation functions.
enum GPCMMLPFunction
{
    Linear,
    Logistic
};

class GPCMBackConstraintMLP : public GPCMBackConstraint
{
protected:
    // Activation function.
    GPCMMLPFunction activationFunction;
    // First layer weights.
    MatrixXd W1;
    // First layer biases.
    MatrixXd b1;
    // Second layer weights.
    MatrixXd W2;
    // Second layer biases.
    MatrixXd b2;
    // First layer weights gradient.
    MatrixXd W1grad;
    // First layer biases gradient.
    MatrixXd b1grad;
    // Second layer weights gradient.
    MatrixXd W2grad;
    // Second layer biases gradient.
    MatrixXd b2grad;
    // Temporary storage for last evaluation summed inputs.
    MatrixXd A;
    // Temporary storage for last evaluation hidden unit activations.
    MatrixXd Z;
    // Target X positions during initialization.
    MatrixXd Xtarget;
    // Temporary storage for error gradient during initialization.
    MatrixXd Xerror;
    // Number of iterations for initialization.
    int maxIterations;
    // Optimization algorithm to use for initialization.
    GPCMOptAlgorithm *algorithm;
public:
    // Construct the back constraint function.
    GPCMBackConstraintMLP(GPCMParams &params, GPCMOptions &options, GPCMOptimization *optimization,
        MatrixXd &dataMatrix, MatrixXd &X);
    // Copy any settings from another back constraint function that we can.
    virtual void copySettings(GPCMBackConstraint *other);
    // Take a step for the initialization optimization procedure.
    double initializationStep(const double *x, double *g);
    // Initialize the back constraint function by optimizing the parameters for an initial latent matrix.
    virtual void initialize();
    // Compute the gradients of this back constraint function using the gradient of the latent coordinates.
    virtual double updateGradient(MatrixXd &Xgrad, bool bNeedGradient);
    // Update the latent coordinates based on the current parameters.
    virtual void updateLatentCoords();
    // Write back constraint to file.
    virtual void write(GPCMMatWriter *writer);
    // Load parameters from specified MAT file reader.
    virtual void load(GPCMMatReader *reader);
    // Destructor.
    virtual ~GPCMBackConstraintMLP();
};
