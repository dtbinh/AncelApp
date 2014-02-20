#include "gm.h"

#include "embedppca.h"
#include "debugprint.h"
#include "scriptparser.h"
#include "datareader.h"
#include "datareaderamc.h"
#include "datareaderann.h"
#include "datareaderbvh.h"
#include "datareaderbvhquat.h"
#include "matreader.h"

GenerativeModel::GenerativeModel(GPCMOptions &inOptions,
			bool bLoadTrainedModel, bool bRunHighDimensionalOptimization)
{
	GPCMOptions options;
	// initialization_script
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

    GenerativeModel *initModel = NULL;
    mHighDimensionalOptimization = bRunHighDimensionalOptimization;
    
	if (!bRunHighDimensionalOptimization &&
        !inOptions["embedding"]["training_latent_dimensions"].empty() &&
        options["data"]["initialization_file"].empty())
    {
        // Launch high-dimensional pre-training.
        DBPRINTLN("Launching high dimensional optimization...");
        initModel = new GenerativeModel(options, false, true);
        initModel->optimize();
        DBPRINTLN("High dimensional optimization complete.");
    }
 	
	//create data reader
	std::string datatype = options["data"]["type"][0];
    std::vector<std::string> data = options["data"]["path"];
   
	GPCMDataReader *datareader = NULL;
    if (!datatype.compare("bvh_motion"))			// Read BVH motion data.
        datareader = new GPCMDataReaderBVH();
    else if (!datatype.compare("bvh_motion_quat"))  // Read BVH motion data but use quaternion representation.
        datareader = new GPCMDataReaderBVHQuat();
	else if (!datatype.compare("amc_motion"))
		datareader = new GPCMDataReaderAMC();
    else if (!datatype.compare("annotation"))		// Read ANN annotation data.
        datareader = new GPCMDataReaderANN();
    else											// Unknown data.
        DBERROR("Unknown data type " << datatype << " specified!");

	std::vector<double> noise(data.size());
    for (unsigned i = 0; i < data.size(); i++)
    {
        if (options["data"]["noise"].size() > i)
            noise[i] = atof(options["data"]["noise"][i].c_str());
        else
            noise[i] = 0.0;
    }
	// append absloute path
	for (std::vector<std::string>::iterator itr = data.begin(); itr != data.end(); itr++)
    {
        itr->insert(0,options["dir"]["dir"][0]);
    }

	datareader->load(data,noise);

	mDataMatrix = datareader->getYMatrix();
	mSequence   = datareader->getSequence();
	mSupplementary = datareader->getSupplementary();

	for(std::size_t i = 0; i < mSequence.size(); i++)
		std::cout << mSequence[i] << std::endl;
	delete datareader;

	// to makesure wheather to validate the gradient
	bool bValidate = !options["optimization"]["validate_gradients"][0].compare("true");

	int maxIterations;
    
	if (!options["optimization"]["iterations_lowdim"].empty() && !bRunHighDimensionalOptimization)
        maxIterations = atoi(options["optimization"]["iterations_lowdim"][0].c_str());
    else
        maxIterations = atoi(options["optimization"]["iterations"][0].c_str());

	bool bUseEM = !options["model"]["learn_scales"][0].compare("em");
 
	int outerIterations = 1;
    if (bUseEM)
        outerIterations = atoi(options["optimization"]["outer_iterations"][0].c_str());
	
	if (!options["data"]["initialization_file"].empty())
        mRunOptimization = false;
    else
        mRunOptimization = true;
	
	mOptimization = new GPCMOptimization( bValidate, bUseEM, options["optimization"]["algorithm"][0],maxIterations,false);
	mLatDim  = 1;

	if (bRunHighDimensionalOptimization)
		mLatDim = atoi(options["embedding"]["training_latent_dimensions"][0].c_str());
    else
        mLatDim = atoi(options["embedding"]["latent_dimensions"][0].c_str());
    if (mLatDim > mDataMatrix.cols()) mLatDim = mDataMatrix.cols();
    
	mX.resize(mDataMatrix.rows(),mLatDim);
    mXGrad.resize(mDataMatrix.rows(),mLatDim);

	
//	std::string inittype = options["initialization"]["method"][0];
    // Optionally filter the data matrix.
    MatrixXd filteredDataMatrix;
    /*if (!options["initialization"]["prefiltering"].empty())
        filteredDataMatrix = filterData(mDataMatrix,mSequence,atof(options["initialization"]["prefiltering"][0].c_str()));
    else
        filteredDataMatrix = mDataMatrix;*/

    mBackConstraint = GPCMBackConstraint::createBackConstraint(options["back_constraints"],options,
        mOptimization,mDataMatrix,mX);
	 
 
	GPCMEmbedPPCA(mX,mDataMatrix);
 	 
	MatrixXd initScales = mSupplementary->getScale();
    MatrixXd fullDataMatrix = mDataMatrix;

	MatrixXd velocityMatrix;
    MatrixXd initVelocityScales;

    bool bHasVelocity = !options["velocity"]["type"].empty() && options["velocity"]["type"][0].compare("none");
    if (bHasVelocity)
    {
        // Split data and velocity components.
        mSupplementary->splitVelocity(mDataMatrix,velocityMatrix);
        // If using scales, also split scales.
        if (initScales.cols() > 0)
            mSupplementary->splitVelocity(initScales,initVelocityScales);
        // Continue splitting velocity if we actually have entries.
        bHasVelocity = velocityMatrix.cols() > 0;
    }

 	
	if(mBackConstraint!= nullptr)
		mBackConstraint->initialize();

	if (mBackConstraint == nullptr)
 		mOptimization->addVariable(VarXformNone,&mX,&mXGrad,"X");

	    // Create dynamics.
    mDynamics = GPCMDynamics::createDynamics(options["dynamics"],options,mOptimization,
        mX,mXGrad,fullDataMatrix,this,mSequence,mSupplementary->getFrameTime());

	if (bRunHighDimensionalOptimization && !options["model"]["rank_prior_wt"].empty())
        mRankPrior = new GPCMRankPrior(atof(options["model"]["rank_prior_wt"][0].c_str()),mX,mXGrad);
    else 
        mRankPrior = nullptr;
  
  	
	mVelocityTerm = nullptr;
    if (bHasVelocity)
    {
        mVelocityTerm = new GPCMVelocityTerm(options["velocity"],options,mOptimization,mX,mXGrad,
            mSequence,velocityMatrix);
    }

	MatrixXd *Xptr = &mX;
    MatrixXd *Xgradptr = &mXGrad;
	mReconstructionGP = new GPCMGaussianProcess(options["model"],options,
		mOptimization,NULL,mDataMatrix,mY,&Xptr,&Xgradptr,1,false,true);

	if (initScales.cols() > 0 && !options["model"]["initial_scales"].empty()
        && !options["model"]["initial_scales"][0].compare("length"))
        mReconstructionGP->getScale() = initScales;
    if (initVelocityScales.cols() > 0 && !options["model"]["initial_scales"].empty()
        && !options["model"]["initial_scales"][0].compare("length"))
        mVelocityTerm->getGaussianProcess()->getScale() = initVelocityScales;

	//----------------------------------------------------------------------------------------------
	if (initModel)
    {
        this->copySettings(*initModel);
        delete initModel;
    }

    else if (!options["data"]["initialization_file"].empty())
    {
        // If we have an initialization file, load that now.
        GPCMMatReader *reader = new GPCMMatReader(options["data"]["initialization_file"][0]);
        load(reader->getStruct("model"));
        delete reader;
    }

	recompute(true);
}
GenerativeModel::~GenerativeModel()
{
 	if (mRankPrior)				delete mRankPrior;
	if (mOptimization)			delete mOptimization;
	if (mReconstructionGP) 		delete mReconstructionGP;
	if (mBackConstraint)		delete mBackConstraint;
	if (mVelocityTerm)			delete mVelocityTerm;
}

void GenerativeModel::recomputeClosedForm()
{
	mReconstructionGP->recomputeClosedForm();
	if (mVelocityTerm) mVelocityTerm->recomputeClosedForm();
 
	//if (dynamics)	dynamics->recomputeClosedForm();
 //   if (latentPrior) latentPrior->recomputeClosedForm();
}
// Recompute all stored temporaries when variables change.
double GenerativeModel::recompute(bool bNeedGradient)
{
	mLogLikelihood = mReconstructionGP->recompute(bNeedGradient);

	if (mVelocityTerm)
		mLogLikelihood += mVelocityTerm->recompute(bNeedGradient);

	if (mRankPrior)
		mLogLikelihood += mRankPrior->recompute(bNeedGradient);

	if (mBackConstraint)
		mLogLikelihood += mBackConstraint->updateGradient(mXGrad,bNeedGradient);

	return mLogLikelihood;
}
    // Recompute constraint, assuming temporaries are up to date.
double GenerativeModel::recomputeConstraint(bool bNeedGradient)
{
	double constraintValue = 0.0;

    // If we have back constraints, reset X gradient.
	if (mBackConstraint)
        mXGrad.setZero(mX.rows(),mX.cols());

    // Compute rank prior constraint.
    if (mRankPrior)
        constraintValue += mRankPrior->recomputeConstraint(bNeedGradient);

    // If we have back constraints, update their gradient.
    if (mBackConstraint)
        mBackConstraint->updateGradient(mXGrad, false);

    return constraintValue;
}
    // Check if a constraint exists.
bool GenerativeModel::hasConstraint()
{
	return (mRankPrior != nullptr);
}
    // Save gradient for debugging purposes.
void GenerativeModel::setDebugGradient(const VectorXd &dbg, double ll)
{
}
 
// Train the model.
void GenerativeModel::optimize()
{
	if (!mRunOptimization) return;
     // First train the entire model.
    mOptimization->optimize(this);
}

MatrixXd GenerativeModel::getLatentVariable()
{
	return mX;
}
void GenerativeModel::copySettings(const GenerativeModel & model)
{
	//TODO  Need Update One Data Changed
	// Make sure number of data points is equal.
    assert(model.mX.rows() == mX.rows());

    // Copy parameters that don't require special processing.
    this->mSequence = model.mSequence;
    this->mDataMatrix = model.mDataMatrix;
    this->mY = model.mY;

    // Copy latent positions or convert them to desired dimensionality.
    if (model.mX.cols() == mX.cols())
    {
        mX = model.mX;
    }
    else
    {
 	         // Pull out the largest singular values to keep.
        JacobiSVD<MatrixXd> svd(model.mX, ComputeThinU | ComputeThinV);

		VectorXd S = svd.singularValues();
        this->mX = svd.matrixU().block(0,0,this->mX.rows(),this->mX.cols())*S.head(this->mX.cols()).asDiagonal();
 	 
        // Report on the singular values that are kept and discarded. 
        DBPRINTLN("Largest singular value discarded: " << S(this->mX.cols()));
        DBPRINTLN("Smallest singular value kept: " << S(this->mX.cols()-1));
        DBPRINTLN("Average singular value kept: " << ((1.0/((double)this->mX.cols()))*S.head(this->mX.cols()).sum()));
        DBPRINT("Singular values: ");
        DBPRINTMAT(S); 
    }
 
	if (mReconstructionGP)  mReconstructionGP->copySettings(model.mReconstructionGP);
	if (mBackConstraint)    mBackConstraint->copySettings(model.mBackConstraint);
}

void GenerativeModel::load(GPCMMatReader *reader)
{

}
 
void GenerativeModel::write(GPCMMatWriter *writer)
{

}
