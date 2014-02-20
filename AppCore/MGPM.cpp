#include "MGPM.h"
#include "embedppca.h"
#include "debugprint.h"
#include "scriptparser.h"
#include "datareader.h"
#include "datareaderamc.h"
#include "datareaderann.h"
#include "datareaderbvh.h"
#include "datareaderbvhquat.h"
#include "matreader.h"

#include <sstream>

GPMGPModel::GPMGPModel(GPCMOptions &inOptions, bool bLoadTrainedModel, bool bRunHighDimensionalOptimization)
{
	GPCMOptions options;
	//#########  initialization_script
	if (!inOptions["model"]["initialization_script"].empty())
    {
        // Options are replaced with the options of the initialization script.
        GPCMScriptParser parser(inOptions["model"]["initialization_script"][0], inOptions["dir"]["dir"][0]);
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

 //   GPMGPModel *initModel = NULL;
 //   mbHighDimensionalOptimization = bRunHighDimensionalOptimization;
 //   
	//if (!bRunHighDimensionalOptimization &&
 //       !inOptions["embedding"]["training_latent_dimensions"].empty() &&
 //       options["data"]["initialization_file"].empty())
 //   {
 //       // Launch high-dimensional pre-training.
 //       DBPRINTLN("Launching high dimensional optimization...");
 //       initModel = new GPMGPModel(options, false, true);
 //       initModel->optimize();
 //       DBPRINTLN("High dimensional optimization complete.");
 //   }
 	
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

	//验证梯度计算有效性
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
	

	//LatDim calculate
	//mLatDim  = 1;
	//if (bRunHighDimensionalOptimization)
	//	mLatDim = atoi(options["embedding"]["training_latent_dimensions"][0].c_str());
 //   else
 //       mLatDim = atoi(options["embedding"]["latent_dimensions"][0].c_str());
 //   if (mLatDim > mDataMatrix.cols()) mLatDim = mDataMatrix.cols();
   
   // mX.resize(mDataMatrix.rows(),mLatDim);
   // mXGrad.resize(mDataMatrix.rows(),mLatDim);
 
   // std::string inittype = options["initialization"]["method"][0];
   // Optionally filter the data matrix.

    //MatrixXd filteredDataMatrix;
    //if (!options["initialization"]["prefiltering"].empty())
    //    filteredDataMatrix = filterData(mDataMatrix,mSequence,atof(options["initialization"]["prefiltering"][0].c_str()));
    //else
    //    filteredDataMatrix = mDataMatrix;

 /*   mBackConstraint = GPCMBackConstraint::createBackConstraint(options["back_constraints"],options,
        mOptimization,mDataMatrix,mX);*/
	    
	
//	if(!mbHighDimensionalOptimization)
//	GPCMEmbedPPCA(mX,mDataMatrix);
//	else 
//		mX = mY;

	//if(mBackConstraint!= nullptr)
	//	mBackConstraint->initialize();

	//if (mBackConstraint == nullptr)
 //		mOptimization->addVariable(VarXformNone,&mX,&mXGrad,"X");

	//if (bRunHighDimensionalOptimization && !options["model"]["rank_prior_wt"].empty())
 //       mRankPrior = new GPCMRankPrior(atof(options["model"]["rank_prior_wt"][0].c_str()),mX,mXGrad);
 //   else 
 //       mRankPrior = nullptr;
   	
	//----------------------------------------------------------------------------------------------
	//if (initModel)
 //   {
 //       this->copySettings(*initModel);
 //       delete initModel;
 //   }

    if (!options["data"]["initialization_file"].empty())
    {
        // If we have an initialization file, load that now.
        GPCMMatReader *reader = new GPCMMatReader(options["data"]["initialization_file"][0]);
        load(reader->getStruct("model"));
        delete reader;
    }
	
	initLatentVariable(options);

	mOptimization->addVariable(VarXformNone, mLatentVariable[0], mGradientLatent[0],"latent_0");
	mOptimization->addVariable(VarXformNone, mLatentVariable[1], mGradientLatent[1],"latent_1");
	mOptimization->addVariable(VarXformNone, mLatentVariable[2], mGradientLatent[2],"latent_2");

	mReconstructionGP = new GPCMGaussianProcess(options["model"], options,
		mOptimization, NULL, mDataMatrix, mY, mX, mXGrad, 3, false, true);

	recompute(true);

	/*fout << std::endl;
	fout << "#latent variable 0" << std::endl;
	fout << *mLatentVariable[0];
	fout << std::endl;
	fout << *mGradientLatent[0];
	fout << std::endl;

	fout << "#latent variable 1" << std::endl;
 	fout << *mLatentVariable[1];
	fout << std::endl;
	fout << *mGradientLatent[1];
	fout << std::endl;

	fout << "#latent variable 2" << std::endl;
	fout << *mLatentVariable[2];
	fout << std::endl;
	fout << *mGradientLatent[2];
	fout << std::endl;

	fout << "#gradient X variable 0" << std::endl;
 	fout << *mXGrad[0] << std::endl;
	fout << std::endl;
	
	fout << "#gradient X variable 1" << std::endl;
 	fout << *mXGrad[1] << std::endl;
	fout << std::endl;

	fout << "#gradient X variable 2" << std::endl;
 	fout << *mXGrad[2] << std::endl;
	fout << std::endl;
 
	mXGrad[0]->setConstant(0);
	mXGrad[1]->setConstant(0);
	mXGrad[2]->setConstant(0);

	recompute(true);

	fout << std::endl;
	fout << "#latent variable 0" << std::endl;
	fout << *mLatentVariable[0];
	fout << std::endl;
	fout << *mGradientLatent[0];
	fout << std::endl;

	fout << "#latent variable 1" << std::endl;
 	fout << *mLatentVariable[1];
	fout << std::endl;
	fout << *mGradientLatent[1];
	fout << std::endl;

	fout << "#latent variable 2" << std::endl;
	fout << *mLatentVariable[2];
	fout << std::endl;
	fout << *mGradientLatent[2];
	fout << std::endl;

	fout << "#gradient X variable 0" << std::endl;
 	fout << *mXGrad[0] << std::endl;
	fout << std::endl;
	
	fout << "#gradient X variable 1" << std::endl;
 	fout << *mXGrad[1] << std::endl;
	fout << std::endl;

	fout << "#gradient X variable 2" << std::endl;
 	fout << *mXGrad[2] << std::endl;
	fout << std::endl;*/
}

GPMGPModel::~GPMGPModel()
{
	if(mReconstructionGP)
		delete mReconstructionGP;
	if(mOptimization)
		delete mOptimization;

	if(mCDMGradient)
	{
		delete mCDMGradient[0];
		delete mCDMGradient[1];
		delete []mCDMGradient;
	}

	if(mX)
	{
		delete mX[0];
		delete mX[1];
		delete mX[2];
		delete []mX;
	}

	if(mXGrad)
	{
		delete mXGrad[0];
		delete mXGrad[1];
		delete mXGrad[2];
		delete []mX;
	}

	if(mLatentVariable.size())
	{
		for(std::size_t i = 0; i < mLatentVariable.size(); i++)
		{
			delete mLatentVariable[i];
			delete mGradientLatent[i];
		}
	}
}

void GPMGPModel::recomputeClosedForm()
{

}
// Recompute all stored temporaries when variables change.
double GPMGPModel::recompute(bool bNeedGradient)
{
	recomputeX();
	
	if(bNeedGradient)
	{
		mXGrad[0]->setConstant(0);
		mXGrad[1]->setConstant(0);
		mXGrad[2]->setConstant(0);
 	}
	mLogLikelihood = mReconstructionGP->recompute(bNeedGradient);
//	std::cout << "likelihood-------------------:" << mLogLikelihood << std::endl;
	
	
	if(bNeedGradient)
		recomputeGradient();

	return mLogLikelihood;
}

// Recompute constraint, assuming temporaries are up to date.
double GPMGPModel::recomputeConstraint(bool bNeedGradient)
{
	return 0;
}
    // Check if a constraint exists.
bool GPMGPModel::hasConstraint()
{
	return false;
}
    // Save gradient for debugging purposes.
void GPMGPModel::setDebugGradient(const VectorXd &dbg, double ll)
{
	std::cout << dbg << std::endl;
}
	// Train the model.
void GPMGPModel::optimize()
{
	//if (!mRunOptimization) return;
     // First train the entire model.
    mOptimization->optimize(this);
	std::cout << *mLatentVariable[0] << std::endl;
	std::cout << std::endl;
	std::cout << *mLatentVariable[1] << std::endl;
	std::cout << std::endl;
	std::cout << *mLatentVariable[2] << std::endl;
}

void GPMGPModel::getLatentVariable(std::vector<MatrixXd*> &latentVar)
{
	latentVar = mLatentVariable;
}

void GPMGPModel::copySettings(const GPMGPModel & model)
{

}

void GPMGPModel::load(GPCMMatReader *reader)
{

}

MatrixXd GPMGPModel::filterData( MatrixXd &dataMatrix,  std::vector<int> sequence, double variance)
{
	return dataMatrix;
}

void GPMGPModel::initLatentVariable(GPCMOptions &inOptions)
{
  	std::vector<std::string> prior_info  = inOptions["data"]["priorinfo"];
 
 	//mX is the function of mLatentVariable, here memory is allocated for both of them

	mX = new Eigen::MatrixXd*[prior_info.size() + 1];
	mXGrad = new Eigen::MatrixXd*[prior_info.size() + 1];

	mLatentVariable.resize(prior_info.size() + 1);
	mGradientLatent.resize(prior_info.size() + 1);

	int type_cnt;
	for(std::size_t i = 0; i < prior_info.size(); i++)
	{
		std::string strcnt = inOptions["data"][prior_info[i] + "_cnt"][0];
		std::stringstream ss(strcnt);
 		ss >> type_cnt;
		
		mX[i] = new Eigen::MatrixXd(mDataMatrix.rows(), type_cnt);
		mXGrad[i] = new Eigen::MatrixXd(mDataMatrix.rows(), type_cnt);
		
		mLatentVariable[i] = new Eigen::MatrixXd(type_cnt, type_cnt);
		mGradientLatent[i] = new Eigen::MatrixXd(type_cnt, type_cnt);
		mLatentVariable[i]->setIdentity(type_cnt, type_cnt);
	}
	

	mX[prior_info.size()] = new Eigen::MatrixXd(mDataMatrix.rows(), 2);
	mXGrad[prior_info.size()] = new Eigen::MatrixXd(mDataMatrix.rows(), 2);

	mLatentVariable[prior_info.size()] = new Eigen::MatrixXd(mSequence.size(), 2);
	mGradientLatent[prior_info.size()] = new Eigen::MatrixXd(mSequence.size(), 2);

	mLatentIndex.resize(prior_info.size() + 1, mDataMatrix.rows());

	//init latent index
 	int prior_type;
	mSequence.insert(mSequence.begin(),0);

	for(std::size_t i = 1; i < mSequence.size(); i++)
	{
		for(int j = mSequence[i-1]; j < mSequence[i]; j++)
		{
			for(std::size_t k = 0; k < prior_info.size(); k++)
			{
				std::string strcnt = inOptions["data"][prior_info[k]][i-1];
				std::stringstream ss(strcnt);
 				ss >> prior_type;
				mLatentIndex(k, j) = prior_type;
			}
			mLatentIndex(prior_info.size(), j) = i - 1;
		}
	}
	
	mCDMGradient = new Eigen::MatrixXd*[prior_info.size() + 2];
	mCDMGradient[0] = new Eigen::MatrixXd(mDataMatrix.rows(),2);
	mCDMGradient[1] = new Eigen::MatrixXd(mDataMatrix.rows(),2);

	initStateFactor(mLatentVariable[prior_info.size()]);

	recomputeX();

	/*std::cout << *mLatentVariable[0] << std::endl;
	std::cout << *mLatentVariable[1] << std::endl;
	std::cout << *mLatentVariable[2] << std::endl;*/

}
void GPMGPModel::initStateFactor(MatrixXd* factor)
{
	for (size_t i = 1; i < mSequence.size();i++)
	{
		size_t numRow = mSequence[i] - mSequence[i-1];

		Eigen::MatrixXd curY(numRow, mDataMatrix.cols());

		curY = mDataMatrix.block(mSequence[i-1], 0, numRow, mDataMatrix.cols());
  
	 	Eigen::MatrixXd curX(numRow,2);
		GPCMEmbedPPCA::GPCMEmbedPPCA(curX, curY);
 	  
		std::vector<double> difAngles;
		double preAngles,curAngles;
		
		for (size_t j = 0; j < numRow; j++)
		{
			curAngles = atan(curX(j, 1)/curX(j, 0));
			if (j != 0)
				difAngles.push_back(abs(curAngles - preAngles));
			preAngles = curAngles;
		}
		std::sort(difAngles.begin(),difAngles.end());
		//使用各个帧之间的间隔的中间值作为step
		(*factor)(i-1, 0) = 0.0;
		(*factor)(i-1, 1) = difAngles[difAngles.size()/2];
    }

	
	size_t len = mSequence[1];
	Eigen::MatrixXd templateMat = mDataMatrix.block(0,0,len,mDataMatrix.cols());
 
	(*factor)(0,0) = 0;
	int index = 0;
	int minRows,minCols;
	for(size_t i = 2; i < mSequence.size(); i++)
	{
		//MatrixXd rowfirst = 
	 	MatrixXd duplicatedMat = mDataMatrix.block(mSequence[i-1], 0, 1, mDataMatrix.cols()).replicate(len, 1);
	 		
		duplicatedMat = duplicatedMat - templateMat;
		duplicatedMat = duplicatedMat.cwiseProduct(duplicatedMat);
 		MatrixXd sumRow = duplicatedMat.rowwise().sum();
		sumRow.minCoeff(&minRows, &minCols);
 		(*factor)(i-1,0) = minRows*(*factor)(0, 1);
   	}
}
void GPMGPModel::recomputeGradient()
{
	for(std::size_t i = 0; i < mGradientLatent.size(); i++)
	{
		mGradientLatent[i]->setConstant(0);
 	}

	for (std::size_t j = 1; j < mSequence.size(); j++)
	{
  		for (size_t k = mSequence[j-1]; k < mSequence[j]; k++)
		{
  			double cosTheta = (*mX[2])(k, 0); 
			double sinTheta = (*mX[2])(k, 1); 
				 
			(*mCDMGradient[0])(k, 0) = -sinTheta;
			(*mCDMGradient[0])(k, 1) =  cosTheta;
			
			(*mCDMGradient[1])(k, 0) = -sinTheta * (k - mSequence[j-1]);
			(*mCDMGradient[1])(k, 1) =  cosTheta * (k - mSequence[j-1]);
 		}
	}
	 
	for(int i = 0; i < mLatentIndex.rows(); i++)
	{
		for(int j = 0; j < mDataMatrix.rows(); j++)
		{
			if(i == mLatentIndex.cols() - 1)
			{
				int index = mLatentIndex(i, j);
  
				(*mGradientLatent[i])(index, 0) += (*mXGrad[i])(j, 0) * (*mCDMGradient[0])(j, 0);
				(*mGradientLatent[i])(index, 0) += (*mXGrad[i])(j, 1) * (*mCDMGradient[0])(j, 1);

				(*mGradientLatent[i])(index, 1) += (*mXGrad[i])(j, 0) * (*mCDMGradient[1])(j, 0);
 				(*mGradientLatent[i])(index, 1) += (*mXGrad[i])(j, 1) * (*mCDMGradient[1])(j, 1);
 			}
			else
			{
 				int index = mLatentIndex(i, j);
				mGradientLatent[i]->row(index) += mXGrad[i]->row(j);
			}
		}
	}
	//std::ofstream fout("text.txt");
	//fout << *mX[0] <<std::endl;
	//fout << std::endl;
	//fout << *mX[1] <<std::endl;
	//fout << std::endl;
	//fout << *mX[2] <<std::endl;
	//fout << std::endl;
	//fout << *mXGrad[0] << std::endl;
	//fout << std::endl;
	//fout << *mXGrad[1] << std::endl;
	//fout << std::endl;
	//fout << *mXGrad[2] << std::endl;
	//fout << std::endl;
	//std::cout << *mGradientLatent[0] << std::endl;
	//fout << std::endl;
	//std::cout << *mGradientLatent[1] << std::endl;
	//fout << std::endl;
	//std::cout << *mGradientLatent[2] << std::endl;
}
void GPMGPModel::recomputeX()
{
	for(size_t i = 0; i < mLatentIndex.rows(); i++)
	{
		if(i == mLatentIndex.rows() - 1)
		{
//			toCircle(*mFakeFactors[i]);
 
			for (std::size_t j = 1; j < mSequence.size(); j++)
			{
				double theta = mLatentVariable[i]->operator()(j - 1, 0);
				double delta = mLatentVariable[i]->operator()(j - 1, 1);

				for (size_t k = mSequence[j-1]; k < mSequence[j]; k++)
				{
					double cosTheta = cos(theta + (k - mSequence[j-1])*delta);
					double sinTheta = sin(theta + (k - mSequence[j-1])*delta);

					(*mX[i])(k, 0) = cosTheta; 
					(*mX[i])(k, 1) = sinTheta; 
				 
					//(*mCDMGradient[0])(k, 0) = -sinTheta;
					//(*mCDMGradient[0])(k, 1) =  cosTheta;

					//(*mCDMGradient[1])(k, 0) = -sinTheta * (k - mSequence[j-1]);
					//(*mCDMGradient[1])(k, 1) =  cosTheta * (k - mSequence[j-1]);
 				}
			}
		}
		else 
		{
			for(size_t j = 0; j < mDataMatrix.rows(); j++)
			{
				mX[i]->row(j) = mLatentVariable[i]->row(mLatentIndex(i, j));
 				//mX[i]->block(j, 0, 1, mX[i]->cols()) = mLatentVariable[i]->block(mLatentIndex(i,j),0, 1, mX[i]->cols()); 
 	 		}
		}
	//	std::cout << *mFakeFactors[i] << std::endl;
	}
}