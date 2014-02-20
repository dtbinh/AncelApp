#include "gpwithrankprior.h"
#include "datareader.h"
#include "datareaderbvh.h"
#include "datareaderann.h"
#include "datareaderbvhquat.h"
#include "debugprint.h"
#include "embedppca.h"
#include "scriptparser.h"
#include "matreader.h"
#include "backconstraint.h"
#pragma warning(disable:4503) 
#include <assert.h>
#include <Eigen\Eigen>
 


GPWithRankPrior::GPWithRankPrior(GPCMOptions &inOptions, bool bLoadTrainedModel, bool bRunHighDimensionalOptimization)
{
 	GPCMOptions options;
	//#########  initialization_script
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

    GPWithRankPrior *initModel = NULL;
    mbHighDimensionalOptimization = bRunHighDimensionalOptimization;
    
	if (!bRunHighDimensionalOptimization &&
        !inOptions["embedding"]["training_latent_dimensions"].empty() &&
        options["data"]["initialization_file"].empty())
    {
        // Launch high-dimensional pre-training.
        DBPRINTLN("Launching high dimensional optimization...");
        initModel = new GPWithRankPrior(options, false, true);
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

	delete datareader;

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
        mbRunOptimization = false;
    else
        mbRunOptimization = true;
	
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
    if (!options["initialization"]["prefiltering"].empty())
        filteredDataMatrix = filterData(mDataMatrix,mSequence,atof(options["initialization"]["prefiltering"][0].c_str()));
    else
        filteredDataMatrix = mDataMatrix;

    mBackConstraint = GPCMBackConstraint::createBackConstraint(options["back_constraints"],options,
        mOptimization,mDataMatrix,mX);
	 
	MatrixXd *Xptr = &mX;

    MatrixXd *Xgradptr = &mXGrad;
	mReconstructionGP = new GPCMGaussianProcess(options["model"],options,
		mOptimization,NULL,mDataMatrix,mY,&Xptr,&Xgradptr,1,false,true);
//	if(!mbHighDimensionalOptimization)
	GPCMEmbedPPCA(mX,mDataMatrix);
//	else 
//		mX = mY;

	if(mBackConstraint!= nullptr)
		mBackConstraint->initialize();

	if (mBackConstraint == nullptr)
 		mOptimization->addVariable(VarXformNone,&mX,&mXGrad,"X");

	if (bRunHighDimensionalOptimization && !options["model"]["rank_prior_wt"].empty())
        mRankPrior = new GPCMRankPrior(atof(options["model"]["rank_prior_wt"][0].c_str()),mX,mXGrad);
    else 
        mRankPrior = nullptr;
  
  	
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
GPWithRankPrior::~GPWithRankPrior()
{
	delete mOptimization;
}
void GPWithRankPrior::recomputeClosedForm()
{
	 mReconstructionGP->recomputeClosedForm();
}
    // Recompute all stored temporaries when variables change.
double GPWithRankPrior::recompute(bool bNeedGradient)
{
	if (mBackConstraint)
    {
        mXGrad.setZero(mX.rows(),mX.cols());
        mBackConstraint->updateLatentCoords();
    }

	mLogLikelihood =  mReconstructionGP->recompute(bNeedGradient);
	if(mRankPrior)
		mLogLikelihood += mRankPrior->recompute(bNeedGradient);
	 
	return mLogLikelihood;
}
    // Recompute constraint, assuming temporaries are up to date.
double GPWithRankPrior::recomputeConstraint(bool bNeedGradient)
{
	double constraintValue = 0.0;
	if(mRankPrior)
		constraintValue = mRankPrior->recomputeConstraint(bNeedGradient);
   if (mBackConstraint)
        mBackConstraint->updateGradient(mXGrad,false);

	return constraintValue;
}
    // Check if a constraint exists.
bool GPWithRankPrior::hasConstraint()
{
	return mRankPrior != nullptr;
}
    // Save gradient for debugging purposes.
void GPWithRankPrior::setDebugGradient(const VectorXd &dbg, double ll)
{

}

void GPWithRankPrior::optimize()
{
	if (!mbRunOptimization) return;

    // First train the entire model.
    mOptimization->optimize(this);
}

MatrixXd GPWithRankPrior::filterData(
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

MatrixXd GPWithRankPrior::getLatentVariable()
{
	return mX;
}

void GPWithRankPrior::copySettings(const GPWithRankPrior & model)
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

		std::cout << mX << std::endl;
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

void GPWithRankPrior::load(GPCMMatReader *reader)
{

}