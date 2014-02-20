// Optimization object for finding the projection of a pose into a model's latent space.

#include "backprojection.h"
#include "model.h"
#include "velocityterm.h"
#include "gp.h"
#include "supplementary.h"
#include "supplementarybvh.h"
#include "optimization.h"
#include "mathutils.h"

// Create projection optimizer.
GPCMBackProjection::GPCMBackProjection(
    GPCMModel *model
    )
{
    // Set variables.
    this->model = model;

    // Create and set up the optimization.
    bool bValidate = false;
    int iterations = 100;
    optimization = new GPCMOptimization(bValidate,false,"lbfgs_nlopt",iterations,1,!bValidate);

    // Resize optimization variables.
    int q = this->model->getLatentPoints().cols();
    X1.resize(1,q);
    X1grad.resize(1,q);
    X2.resize(1,q);
    X2grad.resize(1,q);

    // Resize temporary variables.
    int nY = model->getGaussianProcess()->getScale().cols();
    int nV = model->getVelocityTerm()->getGaussianProcess()->getScale().cols();
    dYnzdX.resize(q,nY);
    dVnzdX.resize(q,nV);
    newYnz.resize(1,nY);
    newVnz.resize(1,nV);
    dVnzdX1.resize(q,nV);
    newY.resize(1,this->model->getSupplementary()->getTotalIndices());
    targetYnz.resize(1,nY);
    targetVnz.resize(1,nV);

    // Load scales.
    scalesY = model->getGaussianProcess()->getScale();
    scalesV = model->getVelocityTerm()->getGaussianProcess()->getScale();
    //scalesY.setOnes(scalesY.rows(),scalesY.cols());
    //scalesV.setOnes(scalesV.rows(),scalesV.cols());
    //scalesY = model->getSupplementary()->getScale();
    //model->getSupplementary()->splitVelocity(scalesY,scalesV);

    // Get the full data matrix.
    MatrixXd fullDataMatrix = model->getTrainingData();

    // Pick out a number of random rows.
    std::vector<int> randomList;
    for (int i = 0; i < fullDataMatrix.rows(); i++)
        randomList.push_back(i);
    std::random_shuffle(randomList.begin(),randomList.end());
    flipMatrix.resize(50,fullDataMatrix.cols());
    for (int i = 0; i < 50; i++)
        flipMatrix.row(i) = fullDataMatrix.row(randomList[i]);

    // Register optimization variables.
    optimization->addVariable(VarXformNone,&X1,&X1grad,"X1");
    optimization->addVariable(VarXformNone,&X2,&X2grad,"X2");
}

// Compute back projection.
void GPCMBackProjection::backProject(
    const MatrixXd &pose,                   // Input pose.
    MatrixXd &Xout,                         // Output X position.
    MatrixXd &Vout                          // Output V position.
    )
{
    // Get scales and dimensions.
    MatrixXd &dataY = model->getGaussianProcess()->getDataMatrix();
    MatrixXd &dataV = model->getVelocityTerm()->getGaussianProcess()->getDataMatrix();
    int NY = model->getGaussianProcess()->getDataMatrix().rows();
    int NV = model->getVelocityTerm()->getGaussianProcess()->getDataMatrix().rows();
    std::vector<int> &seq = model->getSequence();
    MatrixXd flippedPose = pose;

    // First flip the joints to be similar to the maximum number of data points.
    GPCMSupplementaryBVH * bvh = dynamic_cast<GPCMSupplementaryBVH*>(model->getSupplementary());
    int jnts = (bvh->getConfigChannels()-4)/4;
    for (int i = 0; i < jnts; i++)
    {
        double dist = pairwiseDistance(flippedPose.block(0,i*4+4,1,4),flipMatrix.block(0,i*4+4,flipMatrix.rows(),4)).sum();
        double fdist = pairwiseDistance(-flippedPose.block(0,i*4+4,1,4),flipMatrix.block(0,i*4+4,flipMatrix.rows(),4)).sum();
        if (fdist < dist)
            flippedPose.block(0,i*4+4,1,4) *= -1.0;
    }

    // Split pose into Y and V components.
    model->getSupplementary()->splitMatrix(flippedPose,&targetYnz,&targetVnz);

    // Set initial X position to the pose most resembling the desired one in the data.
    int startIndex;
    ydist = pairwiseDistance(targetYnz.cwiseProduct(scalesY),dataY.cwiseProduct(scalesY.replicate(NY,1)));
    ydist.row(0).minCoeff(&startIndex);
    Xout = model->getLatentPoints().row(startIndex);

    // Determine velocity from temporal neighbors.
    int otherIndex = startIndex-1;
    double rev = 1.0;
    if (otherIndex < 0)
    {
        otherIndex = startIndex+1;
        rev = -1.0;
    }
    else
    {
        for (unsigned i = 0; i < seq.size(); i++)
        {
            if (startIndex == seq[i])
            {
                otherIndex = startIndex+1;
                rev = -1.0;
                break;
            }
        }
    }
    Vout = rev*(Xout - model->getLatentPoints().row(otherIndex));
    X1 = Xout-Vout;
    X2 = Xout;

    // Now run the optimization.
    optimization->optimize(this);

    // Return result.
    Xout = X2;
    Vout = X2-X1;
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMBackProjection::recomputeClosedForm()
{
    // Not implemented.
}

// Recompute all stored temporaries when variables change.
double GPCMBackProjection::recompute(
    bool bNeedGradient                      // Indicates whether the gradient is required.
    )
{
    // Compute pose and gradients.
    model->getPose(&X1,&X2,&newY,NULL,NULL,&dYnzdX,&dVnzdX,NULL,NULL,NULL,&newYnz,&newVnz,&dVnzdX1);

    // Compute squared difference.
    double score = 0.5*((newVnz-targetVnz).cwiseProduct(scalesV).squaredNorm() + (newYnz-targetYnz).cwiseProduct(scalesY).squaredNorm());

    // Add up gradients.
    X1grad = -(dVnzdX1.cwiseProduct((newVnz - targetVnz).cwiseProduct(scalesV.array().square().matrix()).replicate(X1.cols(),1))).rowwise().sum().transpose();
    X2grad = -(dYnzdX.cwiseProduct((newYnz - targetYnz).cwiseProduct(scalesY.array().square().matrix()).replicate(X1.cols(),1)) +
               dVnzdX.cwiseProduct((newVnz - targetVnz).cwiseProduct(scalesV.array().square().matrix()).replicate(X1.cols(),1))).rowwise().sum().transpose();

    return -score;
}

// Destructor.
GPCMBackProjection::~GPCMBackProjection()
{
}
