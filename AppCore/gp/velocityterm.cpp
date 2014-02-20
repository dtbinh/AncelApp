// Velocity reconstruction term.

#include "debugprint.h"
#include "velocityterm.h"
#include "matwriter.h"
#include "matreader.h"
#include "gp.h"
#include "kernel.h"
#include "tensorkernel.h"
#include "compoundkernel.h"

// Constructor.
GPCMVelocityTerm::GPCMVelocityTerm(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    MatrixXd &dataMatrix                    // Data matrix containing velocity component.
    ) : X(X), Xgrad(Xgrad), sequence(sequence)
{
    // Decide if we're using difference mode.
    if (!params["difference_mode"][0].compare("true") ||
        !params["difference_mode"][0].compare("1"))
        bDifference = true;
    else
        bDifference = false;

    // Decide if we're using midpoint mode.
    if (!params["midpoint_mode"][0].compare("true") ||
        !params["midpoint_mode"][0].compare("1"))
        bMidpoint = true;
    else
        bMidpoint = false;
    
    // Store data matrix.
    this->dataMatrix.resize(dataMatrix.rows()-sequence.size(),dataMatrix.cols());
    int start = 0;
    int k = 1;
    for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
    {
        int end = *itr;

        // Copy data.
        this->dataMatrix.block(start-k+1,0,end-start-1,dataMatrix.cols()) = dataMatrix.block(start+1,0,end-start-1,dataMatrix.cols());          

        // Copy end to start.
        k++;
        start = end;
    }

    // Initialize first and second point matrices and gradients.
    firstPoints.resize(X.rows()-sequence.size(),X.cols());
    secondPoints.resize(X.rows()-sequence.size(),X.cols());
    firstPointGrads.resize(X.rows()-sequence.size(),X.cols());
    secondPointGrads.resize(X.rows()-sequence.size(),X.cols());

    // Create tensor product kernel.
    std::string kernelFirstName = params["kernel_first_pt"][0];
    std::string kernelSecondName = params["kernel_second_pt"][0];
    std::string kernelNoiseName = params["kernel_noise"][0];
    GPCMKernel *tensorComponents[2];
    GPCMKernel *compoundComponents[2];

    tensorComponents[0] = GPCMKernel::createKernel(options[kernelFirstName],options,optimization,X.cols());
    tensorComponents[1] = GPCMKernel::createKernel(options[kernelSecondName],options,optimization,X.cols());
    compoundComponents[0] = new GPCMTensorKernel(options[kernelFirstName],optimization,tensorComponents,2,2*X.cols());
    compoundComponents[1] = GPCMKernel::createKernel(options[kernelNoiseName],options,optimization,2*X.cols());
    GPCMKernel *kernel = new GPCMCompoundKernel(options[kernelFirstName],optimization,compoundComponents,2,2*X.cols());

    // Create the Gaussian process.
    MatrixXd *inputMatrixPtr[2] = { &firstPoints, &secondPoints };
    MatrixXd *inputGradPtr[2] = { &firstPointGrads, &secondPointGrads };
    setFirstSecond();
    gaussianProcess = new GPCMGaussianProcess(params,options,
        optimization,kernel,this->dataMatrix,Y,inputMatrixPtr,inputGradPtr,2,false,true);
}

// Set first and second points from current X matrix.
void GPCMVelocityTerm::setFirstSecond()
{
    int start = 0;
    int k = 1;
    for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
    {
        int end = *itr;

        // Copy X into input matrix.
        firstPoints.block(start-k+1,0,end-start-1,X.cols()) = X.block(start,0,end-start-1,X.cols());
        if (bMidpoint)
        {
            firstPoints.block(start-k+1,0,end-start-1,X.cols()) += X.block(start+1,0,end-start-1,X.cols());
            firstPoints.block(start-k+1,0,end-start-1,X.cols()) *= 0.5;
        }
        secondPoints.block(start-k+1,0,end-start-1,X.cols()) = X.block(start+1,0,end-start-1,X.cols());
        if (bDifference)
            secondPoints.block(start-k+1,0,end-start-1,X.cols()) -= X.block(start,0,end-start-1,X.cols());

        // Copy end to start.
        k++;
        start = end;
    }

    // Clear gradient.
    firstPointGrads.setZero(firstPointGrads.rows(),firstPointGrads.cols());
    secondPointGrads.setZero(secondPointGrads.rows(),secondPointGrads.cols());
}

// Copy any settings from another model that we can.
void GPCMVelocityTerm::copySettings(
    GPCMVelocityTerm *other                 // Object to copy from.
    )
{
    // Simply copy the GP.
    this->gaussianProcess->copySettings(other->gaussianProcess);
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMVelocityTerm::recomputeClosedForm()
{
    gaussianProcess->recomputeClosedForm();
}

// Recompute all stored temporaries when variables change.
double GPCMVelocityTerm::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{
    // Set first and second point matrices.
    setFirstSecond();

    // Compute GP likelihood and gradients.
    double loglikelihood = gaussianProcess->recompute(bNeedGradient);

    if (bNeedGradient)
    {
        int start = 0;
        int k = 1;
        for (std::vector<int>::iterator itr = sequence.begin(); itr != sequence.end(); ++itr)
        {
            int end = *itr;

            // Add first and second point gradients to Xgrad.
            if (bMidpoint)
            {
                Xgrad.block(start,0,end-start-1,X.cols()) += 0.5*firstPointGrads.block(start-k+1,0,end-start-1,X.cols());
                Xgrad.block(start+1,0,end-start-1,X.cols()) += 0.5*firstPointGrads.block(start-k+1,0,end-start-1,X.cols());
            }
            else
            {
                Xgrad.block(start,0,end-start-1,X.cols()) += firstPointGrads.block(start-k+1,0,end-start-1,X.cols());
            }
            Xgrad.block(start+1,0,end-start-1,X.cols()) += secondPointGrads.block(start-k+1,0,end-start-1,X.cols());
            if (bDifference)
                Xgrad.block(start,0,end-start-1,X.cols()) -= secondPointGrads.block(start-k+1,0,end-start-1,X.cols());

            // Copy end to start.
            k++;
            start = end;
        }
    }

    // Return result.
    return loglikelihood;
}

// Get pointer to underlying Gaussian process.
GPCMGaussianProcess *GPCMVelocityTerm::getGaussianProcess()
{
    return gaussianProcess;
}

// Compute posterior variance between specified points.
MatrixXd GPCMVelocityTerm::posteriorVariance(
    const MatrixXd &X1,                     // First point.
    const MatrixXd &X2                      // Second point.
    )
{
    // Construct array of matrices.
    MatrixXd XDiff;
    MatrixXd XMid;
    const MatrixXd *XArr[MAX_KERNEL_GROUPS];
    memset(XArr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    XArr[0] = &X1;
    if (this->bDifference)
    {
        XDiff = X2-X1;
        XArr[1] = &XDiff;
    }
    else
    {
        XArr[1] = &X2;
    }
    if (this->bMidpoint)
    {
        XMid = 0.5*(X2+X1);
        XArr[0] = &XMid;
    }
    else
    {
        XArr[0] = &X1;
    }

    return gaussianProcess->posteriorVariance(XArr);
}

// Compute posterior mean velocity between specified points.
MatrixXd GPCMVelocityTerm::posteriorMean(
    const MatrixXd &X1,                     // First point.
    const MatrixXd &X2,                     // Second point.
    MatrixXd *var,                          // Optional variance output.
    MatrixXd *dMdXstar,                     // Change in mean with respect to Xstar.
    MatrixXd *dVdXstar,                     // Change in variance with respect to Xstar.
    MatrixXd *dMdX1                         // Change in mean with respect to first point.
    )
{
    // Construct array of matrices.
    MatrixXd XDiff;
    MatrixXd XMid;
    const MatrixXd *XArr[MAX_KERNEL_GROUPS];
    memset(XArr,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    XArr[0] = &X1;
    if (this->bDifference)
    {
        XDiff = X2-X1;
        XArr[1] = &XDiff;
    }
    else
    {
        XArr[1] = &X2;
    }
    if (this->bMidpoint)
    {
        XMid = 0.5*(X2+X1);
        XArr[0] = &XMid;
    }
    else
    {
        XArr[0] = &X1;
    }

    if (dMdXstar || dVdXstar)
    {
        assert(X1.rows() == 1);
        assert(dMdXstar && dVdXstar);

        // Get result from GP.
        MatrixXd dMdXfull(X1.cols()*2,gaussianProcess->getAlpha().cols());
        MatrixXd dVdXfull(X1.cols()*2,1);
        MatrixXd pm = gaussianProcess->posteriorMean(XArr,var,&dMdXfull,&dVdXfull);

        if (dMdX1)
        {
            *dMdX1 = MatrixXd::Zero(X1.cols(),dMdXfull.cols());
            if (this->bMidpoint)
                *dMdX1 += dMdXfull.block(0,0,X1.cols(),dMdXfull.cols())*0.5;
            else
                *dMdX1 += dMdXfull.block(0,0,X1.cols(),dMdXfull.cols());
            if (this->bDifference)
                *dMdX1 -= dMdXfull.block(X1.cols(),0,X1.cols(),dMdXfull.cols());
        }

        // Now combine the gradients.
        if (this->bMidpoint)
        {
            dMdXfull.block(0,0,X1.cols(),dMdXfull.cols()) *= 0.5;
            dVdXfull.block(0,0,X1.cols(),dVdXfull.cols()) *= 0.5;
        }
        else
        {
            dMdXfull.block(0,0,X1.cols(),dMdXfull.cols()) *= 0.0;
            dVdXfull.block(0,0,X1.cols(),dVdXfull.cols()) *= 0.0;
        }
        *dMdXstar = dMdXfull.block(0,0,X1.cols(),dMdXfull.cols()) + dMdXfull.block(X1.cols(),0,X1.cols(),dMdXfull.cols());
        if (dVdXstar)
            *dVdXstar = dVdXfull.block(0,0,X1.cols(),dVdXfull.cols()) + dVdXfull.block(X1.cols(),0,X1.cols(),dVdXfull.cols());

        // Return result.
        return pm;
    }
    else
    {
        // Call posterior mean on GP.
        return gaussianProcess->posteriorMean(XArr,var);
    }
}


// Get Data Matrix
MatrixXd& GPCMVelocityTerm::getDataMatrix()
{
    return dataMatrix;
}

// Write GP data to file.
void GPCMVelocityTerm::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    // Write type.
    writer->writeString("gp","type");

    // Write difference mode.
    if (bDifference)
        writer->writeDouble(1.0,"diff");
    else
        writer->writeDouble(0.0,"diff");

    // Write midpoint mode.
    if (bMidpoint)
        writer->writeDouble(1.0,"mid");
    else
        writer->writeDouble(0.0,"mid");

    // Write GP info.
    gaussianProcess->write(writer);
}

// Load model from specified MAT file reader.
void GPCMVelocityTerm::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Read GP info.
    gaussianProcess->load(reader);
}

// Destructor.
GPCMVelocityTerm::~GPCMVelocityTerm()
{
    delete gaussianProcess;
}
