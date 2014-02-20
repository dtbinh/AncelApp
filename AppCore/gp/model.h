// GPCM model.
#pragma once

#include "optimizable.h"
#include "options.h"

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMSupplementaryData;
class GPCMOptimization;
class GPCMTask;
class GPCMGaussianProcess;
class GPCMVelocityTerm;
class GPCMBackConstraint;
class GPCMDynamics;
class GPCMLatentPrior;
class GPCMRankPrior;
class GPCMTransitionReward;
class GPCMMatReader;
class GPCMMatWriter;
class GPCMController;

class GPCMModel : public GPCMOptimizable
{
protected:
    // The name of the script.
    std::string name;
    // The name of the task.
    std::string taskName;
    // Sequence array.
    std::vector<int> sequence;
    // Matrix of data entries (before scale and bias).
    MatrixXd dataMatrix;
    // Matrix of data entries (after scale and bias).
    MatrixXd Y;
    // Auxiliary data matrix (usually theta).
    MatrixXd auxData;
    // Latent positions matrix.
    MatrixXd X;
    // Gradient of latent positions.
    MatrixXd Xgrad;
    // Latent dimensionality.
    int q;
    // The current total log likelihood.
    double loglikelihood;
    // Base GP model.
    GPCMGaussianProcess *reconstructionGP;
    // Velocity term.
    GPCMVelocityTerm *velocityGP;
    // Back constraints.
    GPCMBackConstraint *backConstraint;
    // Latent dynamics prior.
    GPCMDynamics *dynamics;
    // Nondynamic prior term (usually connectivity).
    GPCMLatentPrior *latentPrior;
    // Rank prior term.
    GPCMRankPrior *rankPrior;
    // Transition reward term.
    GPCMTransitionReward *transitionReward;
    // Supplementary data.
    GPCMSupplementaryData *supplementary;
    // Task specification.
    GPCMTask *task;
    // Optimization manager.
    GPCMOptimization *optimization;
    // Whether to run the optimization.
    bool bRunOptimization;
    // Whether this model is running a high dimensional optimization with rank prior.
    bool bHighDimensionalOptimization;
    // Debugging gradient saved with the model.
    VectorXd debugGradient;
    // Debugging likelihood saved with the model.
    double debugLoglike;
    // Helper function for filtering the data matrix.
    static MatrixXd filterData(MatrixXd &dataMatrix, std::vector<int> sequence, double variance);
public:
    // Constructor, creates model from script.
    GPCMModel(GPCMOptions &options, bool bLoadTrainedModel, bool bRunHighDimensionalOptimization = false);
    // Copy any settings from another model that we can.
    void copySettings(GPCMModel *other);
    // Recompute closed-form MAP estimates when doing alternating optimization.
    virtual void recomputeClosedForm();
    // Recompute all stored temporaries when variables change.
    virtual double recompute(bool bNeedGradient);
    // Recompute constraint, assuming temporaries are up to date.
    virtual double recomputeConstraint(bool bNeedGradient);
    // Get data matrix.
    virtual MatrixXd &getDataMatrix();
    // Get training data.
    virtual MatrixXd getTrainingData();
    // Get latent positions.
    virtual MatrixXd &getLatentPoints();
    // Get sequence.
    virtual std::vector<int> &getSequence();
    // Get reconstruction GP pointer.
    virtual GPCMGaussianProcess *getGaussianProcess();
    // Get velocity term pointer.
    virtual GPCMVelocityTerm *getVelocityTerm();
    // Get transition reward pointer.
    virtual GPCMTransitionReward *getTransitionReward();
    // Get dynamics pointer.
    virtual GPCMDynamics *getDynamics();
    // Get task pointer.
    virtual GPCMTask *getTask();
    // Get supplementary data information.
    virtual GPCMSupplementaryData *getSupplementary();
    // Set new controller.
    void setController(GPCMController *controller);
    // Reconstruct a pose using a trained model.
    virtual void getPose(const MatrixXd *X1, const MatrixXd *X2, MatrixXd *Y,
        MatrixXd *Yvar, MatrixXd *Vvar, MatrixXd *dYnzdX = NULL, MatrixXd *dVnzdX = NULL,
        MatrixXd *dYdX = NULL, MatrixXd *dYvardX = NULL, MatrixXd *dVvardX = NULL,
        MatrixXd *Ynz_out = NULL, MatrixXd *Vnz_out = NULL, MatrixXd *dVnzdX1 = NULL);
    // Check if a constraint exists.
    virtual bool hasConstraint();
    // Train the model.
    void optimize();
    // Save gradient for debugging purposes.
    virtual void setDebugGradient(const VectorXd &dbg, double ll);
    // Get the name of this model.
    std::string getName();
    // Save model output as specified in the script.
    void write(GPCMMatWriter *writer);
    // Load model from specified MAT file reader.
    void load(GPCMMatReader *reader);
    // Destructor.
    ~GPCMModel();
};
