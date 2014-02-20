// Optimization object for finding the projection of a pose into a model's latent space.
#pragma once

#include "options.h"
#include "optimizable.h"

// Forward declarations.
class GPCMModel;
class GPCMOptimization;

class GPCMBackProjection : public GPCMOptimizable
{
protected:
    // Optimization manager.
    GPCMOptimization *optimization;
    // Pointer to model.
    GPCMModel *model;
    // Temporary storage for pose distance matrix.
    MatrixXd ydist;
    // Current latent position.
    MatrixXd X2;
    // Current latent gradient.
    MatrixXd X2grad;
    // Previous latent position.
    MatrixXd X1;
    // Previous latent position gradient.
    MatrixXd X1grad;
    // Data matrix for computing flips.
    MatrixXd flipMatrix;
    // Scales on channels.
    MatrixXd scalesY;
    MatrixXd scalesV;
    // Various gradients and temporaries.
    MatrixXd dYnzdX;
    MatrixXd dVnzdX;
    MatrixXd newYnz;
    MatrixXd newVnz;
    MatrixXd dVnzdX1;
    MatrixXd newY;
    MatrixXd targetYnz;
    MatrixXd targetVnz;
public:
    // Create projection optimizer.
    GPCMBackProjection(GPCMModel *model);
    // Compute back projection.
    virtual void backProject(const MatrixXd &pose, MatrixXd &X, MatrixXd &V);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Destructor.
    virtual ~GPCMBackProjection();
};
