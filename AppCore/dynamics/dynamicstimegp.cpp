// Temporally regressive gaussian process dynamics prior.

#include "debugprint.h"
#include "dynamicstimegp.h"
#include "matwriter.h"
#include "matreader.h"
#include "gp.h"
#include "prior.h"
#include "mathutils.h"
#include "optimization.h"
#include "kernel.h"
#include "compoundkernel.h"
#include "distkernel.h"
#include "model.h"
#include "velocityterm.h"
#include "supplementary.h"

// Constructor.
GPCMDynamicsTimeGP::GPCMDynamicsTimeGP(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    MatrixXd &dataMatrix,                   // Pointer to data matrix.
    GPCMModel *model,                       // Pointer to model.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    double frameLength                      // Length of a single frame.
    ) : GPCMDynamics(params,options,optimization,X,Xgrad,dataMatrix,model,sequence,frameLength)
{
    // Get weight (scale).
    weight = atof(params["weight"][0].c_str());

    // Store data matrix.
    this->dataMatrix = dataMatrix;

    // Initialize matrices.
    timeMat.resize(X.rows(),1);
    dists[0].setConstant(X.rows(),X.rows(),LARGE_DISTANCE);

    // Create matrices and gaussian processes.
    int start = 0;
    for (unsigned i = 0; i < sequence.size(); i++)
    {
        int end = sequence[i];

        // Write time values.
        for (int t = start; t < end; t++)
        {
            timeMat(t,0) = ((double)(t-start))*frameLength;
        }

        // Compute temporal distances.
        dists[0].block(start,start,end-start,end-start) = pairwiseDistance(timeMat.block(start,0,end-start,1));

        start = end;
    }

    // Create matrix of pairwise distances, scaled according to ground truth scales.
    // Get default scales.
    scales = model->getSupplementary()->getScale();
    scaleGrads.resize(scales.rows(),scales.cols());
    assert(scales.cols() > 0);
    // Scale data.
    MatrixXd scaledData = dataMatrix.cwiseProduct(scales.replicate(dataMatrix.rows(),1));
    // Compute pairwise distances.
    dists[1] = pairwiseDistance(scaledData);
    // Compute average distance to neighbors.
    start = 0;
    double totalDist = 0.0;
    for (unsigned i = 0; i < sequence.size(); i++)
    {
        int end = sequence[i];
        for (int t = start; t < end-1; t++)
        {
            totalDist += sqrt(dists[1](t,t+1));
        }
        start = end;
    }
    totalDist /= ((double)(dataMatrix.rows()-sequence.size()));
    // Scale distances so that neighbors are on average frameLength apart.
    //dists[1] *= pow(frameLength/totalDist,2);
    
    // Decide if we want to optimize the GP hyperparameters.
    if (params["learn"][0].compare("true") && params["learn"][0].compare("1"))
    {
        bLearnGP = false;
        optimization = NULL; // If we're not learning the parameters, set optimization to NULL now.
    }
    else
    {
        bLearnGP = true;
    }

    // Create gaussian process.
    MatrixXd *inputMatrixPtr = &timeMat;
    MatrixXd *inputGradPtr = NULL;
    gaussianProcess = new GPCMGaussianProcess(params,options,optimization,NULL,
        X,X,&inputMatrixPtr,&inputGradPtr,1,false,false,false,dists);

    if (!params["learn_pose_scales"].empty() && !params["learn_pose_scales"][0].compare("true"))
    {
        // Set variables.
        bLearnScales = true;

        // Decide if we're tying scales.
        if (!params["tied_pose_scales"].empty() &&
            !params["tied_pose_scales"][0].compare("true"))
            bTiedScales = true;
        else
            bTiedScales = false;

        // Get scale indices.
        positionScaleIndices = model->getSupplementary()->getPositionIndices();
        velocityScaleIndices = model->getSupplementary()->getVelocityIndices();

        // Resize scale variables.
        positionScales.resize(1,positionScaleIndices.size());
        positionScaleGrads.resize(1,positionScaleIndices.size());
        velocityScales.resize(1,velocityScaleIndices.size());
        velocityScaleGrads.resize(1,velocityScaleIndices.size());

        // Construct scales from constituent pieces.
        for (unsigned i = 0; i < positionScaleIndices.size(); i++)
            positionScales(0,i) = scales(0,positionScaleIndices[i]);
        for (unsigned i = 0; i < velocityScaleIndices.size(); i++)
            velocityScales(0,i) = scales(0,velocityScaleIndices[i]);

        if (bTiedScales)
        {
            // Get pointers to position and velocity scales.
            MatrixXd *ptrScalesPosition = &(model->getGaussianProcess()->getScale());
            MatrixXd *ptrScalesVelocity = NULL;
            if (model->getVelocityTerm())
                ptrScalesVelocity = &(model->getVelocityTerm()->getGaussianProcess()->getScale());
            optimization->tieVariable(ptrScalesPosition,&positionScales,&positionScaleGrads);
            if (ptrScalesVelocity)
                optimization->tieVariable(ptrScalesVelocity,&velocityScales,&velocityScaleGrads);
        }
        else
        {
            // Register scales variables.
            optimization->addVariable(VarXformExp,&positionScales,&positionScaleGrads,"dynamics position scale");
            if (velocityScales.cols() > 0)
                optimization->addVariable(VarXformExp,&velocityScales,&velocityScaleGrads,"dynamics velocity scale");
        }

        // Modify poses kernel.
        GPCMKernel *kernel = gaussianProcess->getKernel();
        if (kernel->getType() == KernelTypeCompound)
        {
            GPCMCompoundKernel *cmpd = dynamic_cast<GPCMCompoundKernel*>(kernel);
            for (int i = 0; i < cmpd->getComponentCount(); i++)
            {
                if (cmpd->getComponent(i)->getType() == KernelTypeDist)
                {
                    GPCMDistanceKernel *dst = dynamic_cast<GPCMDistanceKernel*>(cmpd->getComponent(i));
                    if (!dst->getDistanceType().compare("pose"))
                    {
                        dst->setLearnScales(&(this->dataMatrix),&(this->scales),&(this->scaleGrads));
                    }
                }
            }
        }
    }

    // Recompute gaussian process.
    gaussianProcess->recompute(false);

    // Set type.
    this->type = DynamicsTypeTimeGP;
}

// Get Gaussian process pointer.
GPCMGaussianProcess *GPCMDynamicsTimeGP::getGaussianProcess()
{
    return gaussianProcess;
}

// Copy any settings from another model that we can.
void GPCMDynamicsTimeGP::copySettings(
    GPCMDynamics *other                     // Dynamics object to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMDynamicsTimeGP *othercst = dynamic_cast<GPCMDynamicsTimeGP*>(other);
        this->scales = othercst->scales;
        this->positionScales = othercst->positionScales;
        this->velocityScales = othercst->velocityScales;
        this->gaussianProcess->copySettings(othercst->gaussianProcess);
        this->gaussianProcess->recompute(false);
    }
    else
    {
        DBWARNING("Dynamics type mismatch when initializing dynamics parameters from another kernel!");
    }
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMDynamicsTimeGP::recomputeClosedForm()
{
    if (bLearnGP)
    {
        gaussianProcess->recomputeClosedForm();
    }
}

// Recompute all stored temporaries when variables change.
double GPCMDynamicsTimeGP::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    double loglikelihood = 0.0;

    // Compute likelihood from GP.
    if (bLearnGP)
    {
        assert(weight == 1.0); // Weight is not supported right now.
        if (bLearnScales)
        {
            // Construct scales from constituent pieces.
            for (unsigned i = 0; i < positionScaleIndices.size(); i++)
                scales(0,positionScaleIndices[i]) = positionScales(0,i);
            for (unsigned i = 0; i < velocityScaleIndices.size(); i++)
                scales(0,velocityScaleIndices[i]) = velocityScales(0,i);
            scaleGrads.setZero(1,scales.cols());
        }
        loglikelihood += gaussianProcess->recompute(bNeedGradient);
        if (bLearnScales)
        {
            // Distribute derivatives into constituent pieces.
            for (unsigned i = 0; i < positionScaleIndices.size(); i++)
                positionScaleGrads(0,i) = scaleGrads(0,positionScaleIndices[i]);
            for (unsigned i = 0; i < velocityScaleIndices.size(); i++)
                velocityScaleGrads(0,i) = scaleGrads(0,velocityScaleIndices[i]);
        }
    }
    else
    {
        // If we are not learning GP parameters, simply subtract the quadratic component.
        loglikelihood -= pow(weight,2)*0.5*X.cwiseProduct(gaussianProcess->getKinv()*X).sum();
    }

    if (bNeedGradient) // Compute gradient contribution from GP.
        Xgrad -= pow(weight,2)*(gaussianProcess->getKinv()*X);

    // Return result.
    return loglikelihood;
}

// Write dynamics data to file.
void GPCMDynamicsTimeGP::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write superclass data.
    GPCMDynamics::write(writer);

    // Write GP.
    gaussianProcess->write(writer);

    // Write type.
    writer->writeString("gpDynamics","type");

    // Write distance matrices.
    for (int i = 0; i < DIST_TYPES; i++)
    {
        std::string distName;
        if (i == 0)
            distName = "temporal_distance";
        else if (i == 1)
            distName = "pose_distance";
        else
            distName = "unknown_distance";
        writer->writeMatrix(dists[i],distName);
    }

    // Write data matrix.
    writer->writeMatrix(dataMatrix,"poses");
}

// Load model from specified MAT file reader.
void GPCMDynamicsTimeGP::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read superclass data.
    GPCMDynamics::load(reader);

    // Read GP.
    gaussianProcess->load(reader);

    // Recompute.
    gaussianProcess->recompute(false);
}

// Destructor.
GPCMDynamicsTimeGP::~GPCMDynamicsTimeGP()
{
    // Delete GP.
    delete gaussianProcess;
}
