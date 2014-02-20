// A single variable in the optimization.

#include <Eigen/Core>

using namespace Eigen;

// Different types of variable transformations.
enum OptVariableXform
{
    VarXformNone,
    VarXformExp
};

// Maximum tied variables.
#define MAX_TIED_VARS   4

class GPCMOptVariable
{
protected:
    // Number of tied variables.
    int tiedVariables;
    // Pointer to variable data.
    MatrixXd *data[MAX_TIED_VARS];
    // Pointer to variable gradient.
    MatrixXd *gradient[MAX_TIED_VARS];
    // Start index.
    int index;
    // Number of variables.
    int params;
    // Variable transformation.
    OptVariableXform xform;
    // Matrix of minimum values.
    MatrixXd minMat;
    // Matrix of maximum values.
    MatrixXd maxMat;
    // Variable name.
    std::string name;
public:
    // Create a new variable.
    GPCMOptVariable(OptVariableXform xform, MatrixXd *variable,
        MatrixXd *gradient, int index, std::string name);
    // Add tied variable.
    void addTiedData(MatrixXd *data, MatrixXd *gradient);
    // Get pointers to variables.
    MatrixXd *getData();
    // Get number of parameters.
    int getParamCount();
    // Get starting index.
    int getIndex();
    // Get name.
    std::string getName();
    // Clear the gradient to be zero.
    void clearGradient();
    // Pack the gradient.
    void packGradient(const VectorXd &params, VectorXd &grad);
    // Pack the variable.
    void packVariable(VectorXd &params);
    // Unpack the variable.
    void unpackVariable(const VectorXd &params);
};
