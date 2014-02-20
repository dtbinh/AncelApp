// Multi-layer perceptron back constraint function.

#include "debugprint.h"
#include "backconstraintmlp.h"
#include "matwriter.h"
#include "matreader.h"
#include "options.h"
#include "optimization.h"
#include "optalgorithm.h"

// Optimization callback.
double mlpCallback(
    unsigned n,                             // Dimensionality of problem.
    const double *xdata,                    // Pointer to new parameter values.
    double *grad,                           // Pre-allocated buffer to return data to.
    void *opt                               // Pointer to optimization object.
    )
{
    // Simply call the annotator object.
    return ((GPCMBackConstraintMLP*)opt)->initializationStep(xdata,grad);
}

// Construct the back constraint function.
GPCMBackConstraintMLP::GPCMBackConstraintMLP(
    GPCMParams &params,                     // Parameters of this kernel.
    GPCMOptions &options,                   // Loaded options used for creating other kernels.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &dataMatrix,                   // Matrix of Y values.
    MatrixXd &X                             // Matrix of X values.
    ) : GPCMBackConstraint(params,options,optimization,dataMatrix,X)
{
    // Get number of hidden and observed units.
    int inputUnits = dataMatrix.cols();
    int outputUnits = X.cols();
    int hiddenUnits = atoi(params["hidden_units"][0].c_str());
    if (!params["activation_function"][0].compare("linear"))
        activationFunction = Linear;
    else if (!params["activation_function"][0].compare("logistic"))
        activationFunction = Logistic;
    else
        DBERROR("Unknown activation function " << params["activation_function"][0] << " requested for MLP!");

    // Initialize matrices.
    W1grad.resize(inputUnits,hiddenUnits);
    W1.setRandom(inputUnits,hiddenUnits);
    W1.array() -= 0.5;
    W1 *= 1.0/sqrt((double)(inputUnits+1));
    b1grad.resize(1,hiddenUnits);
    b1.setRandom(1,hiddenUnits);
    b1.array() -= 0.5;
    b1 *= 1.0/sqrt((double)(inputUnits+1));
    W2grad.resize(hiddenUnits,outputUnits);
    W2.setRandom(hiddenUnits,outputUnits);
    W2.array() -= 0.5;
    W2 *= 1.0/sqrt((double)(hiddenUnits+1));
    b2grad.resize(1,outputUnits);
    b2.setRandom(1,outputUnits);
    b2.array() -= 0.5;
    b2 *= 1.0/sqrt((double)(hiddenUnits+1));

    // Register variables.
    optimization->addVariable(VarXformNone,&W1,&W1grad,"Layer 1 weight");
    optimization->addVariable(VarXformNone,&b1,&b1grad,"Layer 1 bias");
    optimization->addVariable(VarXformNone,&W2,&W2grad,"Layer 2 weight");
    optimization->addVariable(VarXformNone,&b2,&b2grad,"Layer 2 bias");

    // Create initialization algorithm.
    algorithm = GPCMOptAlgorithm::createAlgorithm(params["init_optimizer"][0]);
    // Set number of iterations.
    maxIterations = atoi(params["init_optimizer_iterations"][0].c_str());

    // Set type.
    name = "mlp";
    type = BackConstraintMLP;
}

// Copy any settings from another back constraint function that we can.
void GPCMBackConstraintMLP::copySettings(
    GPCMBackConstraint *other               // Function to copy parameters from.
    )
{
    if (other->getType() == getType())
    {
        GPCMBackConstraintMLP *othercst = dynamic_cast<GPCMBackConstraintMLP*>(other);
        if (othercst->W2.cols() == this->W2.cols() && othercst->W2.rows() == this->W2.rows())
        {
            this->W1 = othercst->W1;
            this->W2 = othercst->W2;
            this->b1 = othercst->b1;
            this->b2 = othercst->b2;
        }
        else
        {
            // Reinitialize the back constraints.
            initialize();
        }
    }
    else
    {
        DBWARNING("Back constraints type mismatching when initializing from another back constraint function!");
    }
}

// Take a step for the initialization optimization procedure.
double GPCMBackConstraintMLP::initializationStep(
    const double *x,                        // Current parameter offset.
    double *g                               // Returned gradient
    )
{
    double value;

    // Pull out parameters.
    int cnt = 0;
    memcpy(W1.data(),&x[cnt],sizeof(double)*W1.rows()*W1.cols());
    cnt += W1.rows()*W1.cols();
    memcpy(b1.data(),&x[cnt],sizeof(double)*b1.rows()*b1.cols());
    cnt += b1.rows()*b1.cols();
    memcpy(W2.data(),&x[cnt],sizeof(double)*W2.rows()*W2.cols());
    cnt += W2.rows()*W2.cols();
    memcpy(b2.data(),&x[cnt],sizeof(double)*b2.rows()*b2.cols());

    // Update coordinates.
    updateLatentCoords();

    // Compute value and gradient.
    Xerror = X-Xtarget;
    value = 0.5*Xerror.array().square().sum();
    
    // Compute gradients with respect to variables.
    updateGradient(Xerror,true);

    // Pack the gradient.
    if (g)
    {
        cnt = 0;
        memcpy(&g[cnt],W1grad.data(),sizeof(double)*W1.rows()*W1.cols());
        cnt += W1.rows()*W1.cols();
        memcpy(&g[cnt],b1grad.data(),sizeof(double)*b1.rows()*b1.cols());
        cnt += b1.rows()*b1.cols();
        memcpy(&g[cnt],W2grad.data(),sizeof(double)*W2.rows()*W2.cols());
        cnt += W2.rows()*W2.cols();
        memcpy(&g[cnt],b2grad.data(),sizeof(double)*b2.rows()*b2.cols());
        for (int i = 0; i < cnt+b2.rows()*b2.cols(); i++)
            g[i] = -g[i];
    }

    // Return result.
    //DBPRINTLN("Initialization error: " << value);
    return -value;
}

// Initialize the back constraint function by optimizing the parameters for an initial latent matrix.
void GPCMBackConstraintMLP::initialize()
{
    // Store target X positions.
    Xtarget = X;

    // Run nonlinear optimization to most closely match desired latent positions.
    int numVars = W1.rows()*W1.cols()+b1.rows()*b1.cols()+W2.rows()*W2.cols()+b2.rows()*b2.cols();
    VectorXd params(numVars);
    params << Map<VectorXd>(W1.data(),W1.rows()*W1.cols(),1),Map<VectorXd>(b1.data(),b1.rows()*b1.cols(),1),
              Map<VectorXd>(W2.data(),W2.rows()*W2.cols(),1),Map<VectorXd>(b2.data(),b2.rows()*b2.cols(),1);
    algorithm->initialize(params.rows(),maxIterations,mlpCallback,NULL,this);
    algorithm->setStart(params);
    algorithm->run(params);

    // Pull out the result.
    double finalVal = initializationStep(params.data(),NULL);
    //DBPRINTLN("Final initialization error: " << finalVal);

    // Return the new X values.
    updateLatentCoords();
}

// Compute the gradients of this back constraint function using the gradient of the latent coordinates.
double GPCMBackConstraintMLP::updateGradient(
    MatrixXd &Xgrad,                        // Current gradient with respect to latent coordinates.
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // Note that this assumes that Z and A are up to date.
    // Constants.
    int q = X.cols();
    int N = X.rows();
    int d = dataMatrix.cols();
    int h = W1.cols();

    // Layer two weights are just linear.
    assert(activationFunction == Linear);
    W2grad = Z.transpose()*Xgrad;
    b2grad = MatrixXd::Ones(1,X.rows())*Xgrad;

    // Layer one weights are a bit more complicated.
    W1grad = dataMatrix.transpose()*((Xgrad*W2.transpose()).cwiseProduct((-Z.array().square()+1.0).matrix()));
    b1grad = MatrixXd::Ones(1,N)*((Xgrad*W2.transpose()).cwiseProduct((-Z.array().square()+1.0).matrix()));

    // Return score.
    return 0.0;
}

// Update the latent coordinates based on the current parameters.
void GPCMBackConstraintMLP::updateLatentCoords()
{
    // Compute hidden unit activations.
    Z = dataMatrix*W1 + MatrixXd::Ones(dataMatrix.rows(),1)*b1;
    Z = Z.unaryExpr(std::ptr_fun<double,double>(tanh));

    // Compute summed inputs into output units.
    A = Z*W2 + MatrixXd::Ones(dataMatrix.rows(),1)*b2;

    // Apply transformation.
    if (activationFunction == Linear)
    {
        X = A;
    }
    else if (activationFunction == Logistic)
    {
        assert(false && "Unsupported!");
        X = ((-A).array().exp() + 1.0).inverse();
    }
}

// Write back constraint to file.
void GPCMBackConstraintMLP::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write type.
    GPCMBackConstraint::write(writer);

    // Compute total number of parameters.
    int nparams = (W1.rows() + 1)*W1.cols() + (W1.cols()+1)*W2.cols();

    // Write MLP NETLAB parameters.
    writer->writeDouble((double)W1.rows(),"nin");
    writer->writeDouble((double)W1.rows(),"inputDim");
    writer->writeDouble((double)W1.cols(),"nhidden");
    writer->writeDouble((double)W1.cols(),"hiddenDim");
    writer->writeDouble((double)W2.cols(),"outputDim");
    writer->writeDouble((double)nparams,"nwts");
    writer->writeDouble((double)nparams,"numParams");
    if (activationFunction == Linear)
        writer->writeString("linear","outfn");
    else if (activationFunction = Logistic)
        writer->writeString("logistic","outfn");
    writer->writeMatrix(W1,"w1");
    writer->writeMatrix(b1,"b1");
    writer->writeMatrix(W2,"w2");
    writer->writeMatrix(b2,"b2");
}

// Load parameters from specified MAT file reader.
void GPCMBackConstraintMLP::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read matrices.
    W1 = reader->getVariable("w1");
    W2 = reader->getVariable("w2");
    b1 = reader->getVariable("b1");
    b2 = reader->getVariable("b2");
}

// Destructor.
GPCMBackConstraintMLP::~GPCMBackConstraintMLP()
{
}
