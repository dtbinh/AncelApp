// GPCM model.

#include "debugprint.h"
#include "model.h"
#include "optimization.h"
#include "mathutils.h"
#include "matfile.h"
#include "matreader.h"
#include "datareaderbvh.h"
#include "datareaderbvhquat.h"
#include "datareaderann.h"
#include "supplementary.h"
#include "task.h"
#include "gp.h"
#include "dynamics.h"
#include "latentprior.h"
#include "embedppca.h"
#include "rankprior.h"
#include "backconstraint.h"
#include "velocityterm.h"
#include "transitionreward.h"
#include "scriptparser.h"

// Constructor, creates model from script.
GPCMModel::GPCMModel(
    GPCMOptions &inOptions,                 // Script to use to construct the model.
    bool bLoadTrainedModel,                 // Load the trained model corresponding to the specified script.
    bool bRunHighDimensionalOptimization    // Whether this is the high dimensional initialization.
    )
{
    // Check if we simply want to initialize the model from another script.
    GPCMOptions options;
    if (!inOptions["model"]["initialization_script"].empty())
    {
        // Options are replaced with the options of the initialization script.
        GPCMScriptParser parser(inOptions["model"]["initialization_script"][0],inOptions["dir"]["dir"][0]);
        options = parser.getOptions();

        // We necessarily want to load the trained model.
        bLoadTrainedModel = true;
        options["data"]["initialization_file"] = options["result"]["mat_file"];
        options["data"]["initialization_file"][0].insert(0,options["dir"]["dir"][0]);

        // Override options.
        for (GPCMOptions::iterator itr = inOptions.begin(); itr != inOptions.end(); itr++)
        {
            for (GPCMParams::iterator pitr = itr->second.begin(); pitr != itr->second.end(); pitr++)
            {
                options[itr->first][pitr->first] = pitr->second;
            }
        }
    }
    else if (bLoadTrainedModel)
    { // Set initialization file if we are loading a trained model.
        options = inOptions;
        options["data"]["initialization_file"] = options["result"]["mat_file"];
        options["data"]["initialization_file"][0].insert(0,options["dir"]["dir"][0]);
    }
    else
    {
        options = inOptions;
    }

    // Get model name.
    this->name = inOptions["dir"]["filename"][0];

    // Check if there is an initialization script or a high-dimensional pre-training step.
    GPCMModel *initModel = NULL;
    bHighDimensionalOptimization = bRunHighDimensionalOptimization;
    if (!bRunHighDimensionalOptimization &&
        !options["embedding"]["training_latent_dimensions"].empty() &&
        options["data"]["initialization_file"].empty())
    {
        // Launch high-dimensional pre-training.
        DBPRINTLN("Launching high dimensional optimization...");
        initModel = new GPCMModel(options,false,true);
        initModel->optimize();
        DBPRINTLN("High dimensional optimization complete.");
    }

    // Create task specification.
    task = GPCMTask::createTask(options["data"],options);
    taskName = options["data"]["task"][0];

    // Create data reader.
    std::string datatype = options["data"]["type"][0];
    std::vector<std::string> data = options["data"]["path"];
    GPCMDataReader *datareader = NULL;
    if (!datatype.compare("bvh_motion")) // Read BVH motion data.
        datareader = new GPCMDataReaderBVH();
    else if (!datatype.compare("bvh_motion_quat")) // Read BVH motion data but use quaternion representation.
        datareader = new GPCMDataReaderBVHQuat();
    else if (!datatype.compare("annotation")) // Read ANN annotation data.
        datareader = new GPCMDataReaderANN();
    else // Unknown data.
        DBERROR("Unknown data type " << datatype << " specified!");

    // Get any data noise settings.
    std::vector<double> noise(data.size());
    for (unsigned i = 0; i < data.size(); i++)
    {
        if (options["data"]["noise"].size() > i)
            noise[i] = atof(options["data"]["noise"][i].c_str());
        else
            noise[i] = 0.0;
    }

    // Load the data.
    for (std::vector<std::string>::iterator itr = data.begin(); itr != data.end(); itr++)
    {
        itr->insert(0,options["dir"]["dir"][0]);
    }
    datareader->load(data,noise);

    // Read back the data.
    dataMatrix = datareader->getYMatrix();
    auxData = datareader->getAuxMatrix();
    sequence = datareader->getSequence();
    supplementary = datareader->getSupplementary();

    // Print number of frames.
    DBPRINTLN("Embedding " << dataMatrix.rows() << " points.");

    // Clean up.
    delete datareader;

    // Check whether we want to validate gradients and create the optimization.
    bool bValidate = !options["optimization"]["validate_gradients"][0].compare("true") ||
                     !options["optimization"]["validate_gradients"][0].compare("1") ||
                     (!options["optimization"]["validate_gradients_lowdim"].empty() && !bRunHighDimensionalOptimization &&
                      (!options["optimization"]["validate_gradients_lowdim"][0].compare("true") ||
                       !options["optimization"]["validate_gradients_lowdim"][0].compare("1")));
    bool bUseEM = !options["model"]["learn_scales"][0].compare("em");
    int maxIterations;
    if (!options["optimization"]["iterations_lowdim"].empty() && !bRunHighDimensionalOptimization)
        maxIterations = atoi(options["optimization"]["iterations_lowdim"][0].c_str());
    else
        maxIterations = atoi(options["optimization"]["iterations"][0].c_str());
    int outerIterations = 1;
    if (bUseEM)
        outerIterations = atoi(options["optimization"]["outer_iterations"][0].c_str());

    // If we are loading from a file, no need to run initial optimization.
    if (!options["data"]["initialization_file"].empty())
        bRunOptimization = false;
    else
        bRunOptimization = true;

    // Create optimization.
    optimization = new GPCMOptimization(bValidate,bUseEM,options["optimization"]["algorithm"][0],
        maxIterations,outerIterations);

    // Set latent dimension and allocate matrix.
    if (bRunHighDimensionalOptimization)
        q = atoi(options["embedding"]["training_latent_dimensions"][0].c_str());
    else
        q = atoi(options["embedding"]["latent_dimensions"][0].c_str());
    if (q > dataMatrix.cols()) q = dataMatrix.cols();
    X.resize(dataMatrix.rows(),q);
    Xgrad.resize(dataMatrix.rows(),q);

    // Get filtering method.
    std::string inittype = options["initialization"]["method"][0];
    // Optionally filter the data matrix.
    MatrixXd filteredDataMatrix;
    if (!options["initialization"]["prefiltering"].empty())
        filteredDataMatrix = filterData(dataMatrix,sequence,atof(options["initialization"]["prefiltering"][0].c_str()));
    else
        filteredDataMatrix = dataMatrix;

    // If we have hard-coded initial scales, get them now.
    MatrixXd initScales = supplementary->getScale();
    MatrixXd fullDataMatrix = dataMatrix;

    // Check if we should tease apart velocity and position components.
    MatrixXd velocityMatrix;
    MatrixXd initVelocityScales;
    bool bHasVelocity = !options["velocity"]["type"].empty() && options["velocity"]["type"][0].compare("none");
    if (bHasVelocity)
    {
        // Split data and velocity components.
        supplementary->splitVelocity(dataMatrix,velocityMatrix);
        // If using scales, also split scales.
        if (initScales.cols() > 0)
            supplementary->splitVelocity(initScales,initVelocityScales);
        // Continue splitting velocity if we actually have entries.
        bHasVelocity = velocityMatrix.cols() > 0;
    }

    // Create back constraints.
    backConstraint = GPCMBackConstraint::createBackConstraint(options["back_constraints"],options,
        optimization,dataMatrix,X);

    // Initialize X.
    // Initialize.
    if (!inittype.compare("ppca"))
        GPCMEmbedPPCA(X,filteredDataMatrix);
    else
        DBERROR("Unkown initialization " << inittype << " requested!");

    // Initialize back constraints.
    if (backConstraint)
        backConstraint->initialize();

    // Create rank prior term.
    if (bRunHighDimensionalOptimization && !options["model"]["rank_prior_wt"].empty())
        rankPrior = new GPCMRankPrior(atof(options["model"]["rank_prior_wt"][0].c_str()),X,Xgrad);
    else
        rankPrior = NULL;

    // Register X if there are no back constraints.
    if (!backConstraint)
        optimization->addVariable(VarXformNone,&X,&Xgrad,"X");

    // Check if we should create a velocity GP.
    velocityGP = NULL;
    if (bHasVelocity)
    {
        velocityGP = new GPCMVelocityTerm(options["velocity"],options,optimization,X,Xgrad,
            sequence,velocityMatrix);
    }

    // Create reconstruction GP.
    MatrixXd *Xptr = &X;
    MatrixXd *Xgradptr = &Xgrad;
    reconstructionGP = new GPCMGaussianProcess(options["model"],options,
        optimization,NULL,dataMatrix,Y,&Xptr,&Xgradptr,1,true,true);

    // Set scales if necessary.
    if (initScales.cols() > 0 && !options["model"]["initial_scales"].empty()
        && !options["model"]["initial_scales"][0].compare("length"))
        reconstructionGP->getScale() = initScales;
    if (initVelocityScales.cols() > 0 && !options["model"]["initial_scales"].empty()
        && !options["model"]["initial_scales"][0].compare("length"))
        velocityGP->getGaussianProcess()->getScale() = initVelocityScales;

    // Create dynamics.
    dynamics = GPCMDynamics::createDynamics(options["dynamics"],options,optimization,
        X,Xgrad,fullDataMatrix,this,sequence,supplementary->getFrameTime());

    // Create latent prior term.
	// Note that scripts refer to it as "value term" for historical reasons.
    latentPrior = GPCMLatentPrior::createLatentPrior(options["value_term"],options,
        optimization,X,Xgrad,sequence,task,NULL);

    // Give dynamics a pointer to the value function.
    if (dynamics)
        dynamics->setValueTerm(latentPrior);

    // Create transition reward.
    if (!options["transition_reward"].empty())
        transitionReward = GPCMTransitionReward::createTransitionReward(options["transition_reward"],
            options,X,this,sequence,supplementary->getFrameTime());
    else
        transitionReward = NULL;

    // If we have an initialization model, get as many parameters from it as possible.
    if (initModel)
    {
        this->copySettings(initModel);
        delete initModel;
    }
    else if (!options["data"]["initialization_file"].empty())
    {
        // If we have an initialization file, load that now.
        GPCMMatReader *reader = new GPCMMatReader(options["data"]["initialization_file"][0]);
        load(reader->getStruct("model"));
        delete reader;
    }

    // Precompute temporaries.
    recompute(true);
}

// Get the model name.
std::string GPCMModel::getName()
{
    return name;
}

// Set new controller.
void GPCMModel::setController(
    GPCMController *controller              // New controller.
    )
{
    if (latentPrior) latentPrior->setController(controller);
}

// Copy any settings from another model that we can.
void GPCMModel::copySettings(
    GPCMModel *other                        // Model to copy from.
    )
{
    // Make sure number of data points is equal.
    assert(other->X.rows() == X.rows());

    // Copy parameters that don't require special processing.
    this->sequence = other->sequence;
    this->dataMatrix = other->dataMatrix;
    this->Y = other->Y;

    // Copy latent positions or convert them to desired dimensionality.
    if (other->X.cols() == this->X.cols())
    {
        this->X = other->X;
    }
    else
    {
        // Pull out the largest singular values to keep.
        JacobiSVD<MatrixXd> svd(other->X, ComputeThinU | ComputeThinV);
        VectorXd S = svd.singularValues();
        this->X = svd.matrixU().block(0,0,this->X.rows(),this->X.cols())*S.head(this->X.cols()).asDiagonal();

        // Report on the singular values that are kept and discarded.
        DBPRINTLN("Largest singular value discarded: " << S(this->X.cols()));
        DBPRINTLN("Smallest singular value kept: " << S(this->X.cols()-1));
        DBPRINTLN("Average singular value kept: " << ((1.0/((double)this->X.cols()))*S.head(this->X.cols()).sum()));
        DBPRINT("Singular values: ");
        DBPRINTMAT(S);
    }

    // Copy model components.
    this->reconstructionGP->copySettings(other->reconstructionGP);
    if (velocityGP) velocityGP->copySettings(other->velocityGP);
    if (dynamics) this->dynamics->copySettings(other->dynamics);
    if (latentPrior) this->latentPrior->copySettings(other->latentPrior);
    if (backConstraint) this->backConstraint->copySettings(other->backConstraint);
    if (transitionReward) this->transitionReward->copySettings(other->transitionReward);
}

// Helper function for filtering the data matrix.
MatrixXd GPCMModel::filterData(
    MatrixXd &dataMatrix,                   // Data matrix to filter.
    std::vector<int> sequence,              // Sequence.
    double variance                         // Filtering variance.
    )
{
    MatrixXd result = dataMatrix;
    result.setZero(result.rows(),result.cols());

    int start = 0;
    for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
    { // Step over each entry in sequence.
        int send = *itr;
        for (int t = start; t < send; t++)
        { // Step over each frame.
            int fstart = (int)floor((double)t-variance*3.0);
            int fend = (int)ceil((double)t+variance*3.0);
            if (fstart < start) fstart = start;
            if (fend >= send) fend = send-1;
            double totalw = 0.0;

            // Step over each frame to add in here.
            for (int tt = fstart; tt < fend; tt++)
            {
                double w = exp(-(1.0/variance)*pow((double)tt - (double)t,2));
                totalw += w;
                result.block(t,0,1,result.cols()) +=
                    dataMatrix.block(tt,0,1,result.cols())*w;
            }
            
            // Normalize.
            result.block(t,0,1,result.cols()) *= (1.0/totalw);
        }
        start = send;
    }

    // Return filtered result.
    return result;
}

// Check if a constraint exists.
bool GPCMModel::hasConstraint()
{
    return rankPrior != NULL;
}

// Recompute constraint, assuming temporaries are up to date.
double GPCMModel::recomputeConstraint(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    double constraintValue = 0.0;

    // If we have back constraints, reset X gradient.
    if (backConstraint)
        Xgrad.setZero(X.rows(),X.cols());

    // Compute rank prior constraint.
    if (rankPrior)
        constraintValue += rankPrior->recomputeConstraint(bNeedGradient);

    // If we have back constraints, update their gradient.
    if (backConstraint)
        backConstraint->updateGradient(Xgrad,false);

    return constraintValue;
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMModel::recomputeClosedForm()
{
    reconstructionGP->recomputeClosedForm();
    if (velocityGP) velocityGP->recomputeClosedForm();
    if (dynamics) dynamics->recomputeClosedForm();
    if (latentPrior) latentPrior->recomputeClosedForm();
}

// Recompute all stored temporaries when variables change.
double GPCMModel::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // If we have back constraints, use them to update X.
    if (backConstraint)
    {
        Xgrad.setZero(X.rows(),X.cols());
        backConstraint->updateLatentCoords();
    }

    // Compute gradient and likelihood of reconstrunction GP.
    loglikelihood = reconstructionGP->recompute(bNeedGradient);

    // Compute gradient and likelihood of velocity term.
    if (velocityGP)
        loglikelihood += velocityGP->recompute(bNeedGradient);

    // Compute gradient and likelihood of dynamics.
    if (dynamics)
        loglikelihood += dynamics->recompute(bNeedGradient);

    // Compute gradient and likelihood of value term.
    if (latentPrior)
        loglikelihood += latentPrior->recompute(bNeedGradient);

    // Compute gradient and likelihood of rank prior.
    if (rankPrior)
        loglikelihood += rankPrior->recompute(bNeedGradient);

    // If we have back constraints, update their gradient.
    if (backConstraint)
        loglikelihood += backConstraint->updateGradient(Xgrad,bNeedGradient);

    // Return objective value.
    return loglikelihood;
}

// Train the model.
void GPCMModel::optimize()
{
    if (!bRunOptimization) return;

    // First train the entire model.
    optimization->optimize(this);

    // Now build the transition reward term.
    if (transitionReward && !bHighDimensionalOptimization) transitionReward->optimize();
}

// Get data matrix.
MatrixXd &GPCMModel::getDataMatrix()
{
    return dataMatrix;
}

// Get training data.
MatrixXd GPCMModel::getTrainingData()
{
    MatrixXd fullData;
    MatrixXd resizedDataMatrix(dataMatrix.rows()-sequence.size(),dataMatrix.cols());
    int start = 0;
    int k = 1;
    for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
    {
        int end = *itr;
        
        // Copy data.
        resizedDataMatrix.block(start-k+1,0,end-start-1,dataMatrix.cols()) = dataMatrix.block(start+1,0,end-start-1,dataMatrix.cols());          
        
        // Copy end to start.
        k++;
        start = end;
    }

    supplementary->fillFullY(resizedDataMatrix,velocityGP->getDataMatrix(),fullData);
    return fullData;
}

// Get latent positions.
MatrixXd &GPCMModel::getLatentPoints()
{
    return X;
}

// Get sequence.
std::vector<int> &GPCMModel::getSequence()
{
    return sequence;
}

// Get reconstruction GP pointer.
GPCMGaussianProcess *GPCMModel::getGaussianProcess()
{
    return reconstructionGP;
}

// Get velocity term pointer.
GPCMVelocityTerm *GPCMModel::getVelocityTerm()
{
    return velocityGP;
}

// Get transition reward pointer.
GPCMTransitionReward *GPCMModel::getTransitionReward()
{
    return transitionReward;
}

// Get dynamics pointer.
GPCMDynamics *GPCMModel::getDynamics()
{
    return dynamics;
}

// Get task pointer.
GPCMTask *GPCMModel::getTask()
{
    return task;
}

// Get supplementary data information.
GPCMSupplementaryData *GPCMModel::getSupplementary()
{
    return supplementary;
}

// Reconstruct a pose using a trained model.
void GPCMModel::getPose(
    const MatrixXd *X1,                     // Optional previous point.
    const MatrixXd *X2,                     // Current point.
    MatrixXd *Y,                            // Reconstructed pose.
    MatrixXd *Yvar,                         // Optional pose variance.
    MatrixXd *Vvar,                         // Optional velocity variance.
    MatrixXd *dYnzdX,                       // Change in non-zero Y entries with respect to X.
    MatrixXd *dVnzdX,                       // Change in non-zero V entries with respect to X.
    MatrixXd *dYdX,                         // Change in pose with respect to X.
    MatrixXd *dYvardX,                      // Change in Y variance with respect to X.
    MatrixXd *dVvardX,                      // Change in V variance with respect to X.
    MatrixXd *Ynz_out,                      // Optional returned Ynz
    MatrixXd *Vnz_out,                      // Optional returned Vnz
    MatrixXd *dVnzdX1                       // Change in velocity with respect to first point.
    )
{
    // Get mean from reconstruction GP.
    const MatrixXd *X2Arr[MAX_KERNEL_GROUPS];
    memset(X2Arr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    X2Arr[0] = X2;
    MatrixXd Ynz = reconstructionGP->posteriorMean(X2Arr,Yvar,dYnzdX,dYvardX);
    MatrixXd Vnz;
    if (velocityGP)
    { // If there is a velocity GP, call it now.
        if (X1) Vnz = velocityGP->posteriorMean(*X1,*X2,Vvar,dVnzdX,dVvardX,dVnzdX1);
        else Vnz.setZero(Ynz.rows(),velocityGP->getGaussianProcess()->getAlpha().cols());
    }

    // Now reconstruct the entire pose, including constant entries.
    supplementary->fillFullY(Ynz,Vnz,*Y);

    // Also reconstruct the gradients.
    if (dYdX)
        supplementary->fillFullY(*dYnzdX,*dVnzdX,*dYdX);
    
    if(Ynz_out)
        *Ynz_out = Ynz;
    if(Vnz_out)
        *Vnz_out = Vnz;
}

// Save gradient for debugging purposes.
void GPCMModel::setDebugGradient(
    const VectorXd &dbg,                    // The new debugging gradient.
    double ll                               // The new debugging log likelihood.
    )
{
    this->debugGradient = dbg;
    this->debugLoglike = ll;
}

// Save the model to the specified file.
void GPCMModel::write(
    GPCMMatWriter *writer                   // Writer to use.
    )
{
    // Create model struct.
    GPCMMatWriter *modelStruct = writer->writeStruct("model",1,1);

    // Write model name.
    modelStruct->writeString("fgplvm","type");

    // Don't need rotation reparameterization structure.
    modelStruct->writeBlank("rotation_reparam");

    // Write task name.
    modelStruct->writeString(taskName,"task_name");

    // Write reconstruction GP parameters.
    reconstructionGP->write(modelStruct);

    // Create velocity GP structure.
    if (velocityGP)
    {
        GPCMMatWriter *velocityStruct = modelStruct->writeStruct("velocity",1,1);
        velocityGP->write(velocityStruct);
        modelStruct->closeStruct();
    }

    // Create dynamics structure.
    if (dynamics)
    {
        GPCMMatWriter *dynamicsStruct = modelStruct->writeStruct("dynamics",1,1);
        dynamics->write(dynamicsStruct);
        modelStruct->closeStruct();
    }

    // Create value term structure.
    if (latentPrior)
    {
        GPCMMatWriter *valueStruct = modelStruct->writeStruct("value_term",1,1);
        latentPrior->write(valueStruct);
        modelStruct->closeStruct();
    }

    // Create the back constraints structure.
    if (backConstraint)
    {
        GPCMMatWriter *backStruct = modelStruct->writeStruct("back",1,1);
        backConstraint->write(backStruct);
        modelStruct->closeStruct();
    }

    // Create transition reward structure.
    if (transitionReward)
    {
        GPCMMatWriter *transitionStruct = modelStruct->writeStruct("transition_reward",1,1);
        transitionReward->write(transitionStruct);
        modelStruct->closeStruct();
    }

    // Convert and store sequence.
    MatrixXd seqMat(1,sequence.size());
    int k = 0;
    for (std::vector<int>::iterator itr = sequence.begin();
         itr != sequence.end(); itr++, k++)
    {
        seqMat(0,k) = *itr;
    }
    modelStruct->writeMatrix(seqMat,"seq");

    // Save the supplementary data.
    supplementary->write(modelStruct);

    // Save the auxiliary data.
    modelStruct->writeMatrix(auxData,"Theta");

    // Close the struct.
    writer->closeStruct();

    // Write debugging stuff.
    writer->writeMatrix(debugGradient,"last_gradient");
    writer->writeDouble(debugLoglike,"last_loglike");
}

// Load model from specified MAT file reader.
void GPCMModel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read in the sequence data.
    MatrixXd seqMat = reader->getVariable("seq");
    sequence.clear();
    for (int i = 0; i < seqMat.cols(); i++)
        sequence.push_back((int)(seqMat(0,i)));

    // Read in data matrix.
    dataMatrix = reader->getVariable("y");

    // Read in data matrix after scaling.
    Y = reader->getVariable("m");

    // Read in latent positions.
    X = reader->getVariable("X");

    // Copy model components.
    reconstructionGP->load(reader);
    if (velocityGP) velocityGP->load(reader->getStruct("velocity"));
    if (dynamics) dynamics->load(reader->getStruct("dynamics"));
    if (latentPrior) latentPrior->load(reader->getStruct("value_term")); // Note that the latent prior is called a value term for historical reasons.
    if (transitionReward) transitionReward->load(reader->getStruct("transition_reward"));
}

// Destructor.
GPCMModel::~GPCMModel()
{
    delete reconstructionGP;
    delete velocityGP;
    delete dynamics;
    delete latentPrior;
    delete rankPrior;
    delete transitionReward;
    delete supplementary;
    delete optimization;
    delete task;
}
