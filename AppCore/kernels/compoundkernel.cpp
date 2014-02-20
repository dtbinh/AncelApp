// Compound kernel function.

#include "compoundkernel.h"
#include "mathutils.h"
#include "matwriter.h"
#include "matreader.h"
#include "debugprint.h"

// Construct the kernel.
GPCMCompoundKernel::GPCMCompoundKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    GPCMKernel **components,                // Input array of components.
    int componentCount,                     // Size of array.
    int dims                                // Number of input dimensions
    ) : GPCMKernel(dims)
{
    memcpy(this->components,components,sizeof(GPCMKernel*)*componentCount);
    this->componentCount = componentCount;
    this->params = 0;
    for (int i = 0; i < componentCount; i++)
    {
        this->params = this->params + components[i]->getParamCount();
    }
    name = "cmpnd";
    this->type = KernelTypeCompound;
}

// Get number of components.
int GPCMCompoundKernel::getComponentCount()
{
    return componentCount;
}

// Get a specific component.
GPCMKernel *GPCMCompoundKernel::getComponent(
    int idx                                 // Which component to get.
    )
{
    return components[idx];
}

// Copy any settings from another kernel that we can.
void GPCMCompoundKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMCompoundKernel *othercst = dynamic_cast<GPCMCompoundKernel*>(other);
        if (othercst->componentCount == this->componentCount)
        {
            for (int i = 0; i < componentCount; i++)
            {
                this->components[i]->copySettings(othercst->components[i]);
            }
        }
        else
        {
            DBWARNING("Compound kernel component count mismatch when initializing kernel parameters from another kernel!");
        }
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return white noise from this kernel.
double GPCMCompoundKernel::getNoise()
{
    double noise = 0.0;
    for (int i = 0; i < componentCount; i++)
    {
        noise += this->components[i]->getNoise();
    }
    return noise;
}

// Return covariance of a single set of points.
MatrixXd GPCMCompoundKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    MatrixXd result;
    for (int i = 0; i < componentCount; i++)
    {
        if (i == 0)
            result = this->components[i]->covariance(X);
        else
            result += this->components[i]->covariance(X);
    }
    return result;
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMCompoundKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    int q = 0;
    for (int i = 0; i < MAX_KERNEL_GROUPS; i++)
    {
        if (X[i] == NULL)
            break;
        else
            q += X[i]->cols();
    }

    MatrixXd result;
    MatrixXd tempGrad(X[0]->rows(),q);
    Xgrad.setZero(Xgrad.rows(),Xgrad.cols());
    for (int i = 0; i < componentCount; i++)
    {
        if (i == 0)
            result = this->components[i]->covarianceGrad(X,tempGrad);
        else
            result += this->components[i]->covarianceGrad(X,tempGrad);
        Xgrad += tempGrad;
    }
    return result;
}

// Return coviarance of two sets of points.
MatrixXd GPCMCompoundKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    MatrixXd result;
    for (int i = 0; i < componentCount; i++)
    {
        if (i == 0)
            result = this->components[i]->covariance(X1,X2);
        else
            result += this->components[i]->covariance(X1,X2);
    }
    return result;
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMCompoundKernel::covarianceGrad(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2,              // Second data matrix.
    MatrixXd &X2grad                        // Gradient.
    )
{
    int q = 0;
    for (int i = 0; i < MAX_KERNEL_GROUPS; i++)
    {
        if (X1[i] == NULL)
            break;
        else
            q += X1[i]->cols();
    }

    MatrixXd result;
    MatrixXd tempGrad(X1[0]->rows(),q);
    X2grad.setZero(X2grad.rows(),X2grad.cols());
    for (int i = 0; i < componentCount; i++)
    {
        if (i == 0)
            result = this->components[i]->covarianceGrad(X1,X2,tempGrad);
        else
            result += this->components[i]->covarianceGrad(X1,X2,tempGrad);
        X2grad += tempGrad;
    }
    return result;
}

// Recompute priors on all kernel parameters and return the likelihood.
double GPCMCompoundKernel::recomputePriors()
{
    double result = 0;
    for (int i = 0; i < componentCount; i++)
    {
        result += this->components[i]->recomputePriors();
    }
    return result;
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMCompoundKernel::recompute(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    for (int i = 0; i < componentCount; i++)
    {
        this->components[i]->recompute(gK,gKd,X,Xgrad);
    }
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMCompoundKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    MatrixXd result;
    for (int i = 0; i < componentCount; i++)
    {
        if (i == 0)
            result = this->components[i]->covarianceGradientGP(X);
        else
            result += this->components[i]->covarianceGradientGP(X);
    }
    return result;
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMCompoundKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    for (int i = 0; i < componentCount; i++)
    {
        this->components[i]->recomputeGradientGP(gK,gKd,X,Xgrad);
    }
}

// Write kernel data to file.
void GPCMCompoundKernel::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    GPCMKernel::write(writer); // Let superclass write first.

    // Create a cell array containing the structures for each component.
    GPCMMatWriter *cellWriter = writer->writeCell("comp",1,componentCount);
    for (int i = 0; i < componentCount; i++)
    {
        GPCMMatWriter *compStruct = cellWriter->writeStruct("",1,1);
        compStruct->writeBlank("index"); // All dimensions participate in component.
        this->components[i]->write(compStruct);
        cellWriter->closeStruct();
    }
    writer->closeCell();

    // Some general stuff that is necessary for the Matlab code to work.
    writer->writeMatrix(MatrixXd::Identity(params,params),"paramGroups");
    writer->writeBlank("transforms");
}

// Load kernel from specified MAT file reader.
void GPCMCompoundKernel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
    // Load superclass.
    GPCMKernel::load(reader);

    // Get components.
    GPCMMatReader *components = reader->getStruct("comp");
    for (int i = 0; i < components->structCount(); i++)
    {
        this->components[i]->load(components->getStruct(i));
    }
}

// Print out kernel parameters.
void GPCMCompoundKernel::printParams()
{
    DBPRINTLN("Compound kernel:");
    for (int i = 0; i < componentCount; i++)
    {
        DBPRINTLN("Component " << i << ":");
        components[i]->printParams();
    }
    DBPRINTLN("Compound kernel end");
}

// Destructor.
GPCMCompoundKernel::~GPCMCompoundKernel()
{
    // Delete the components.
    for (int i = 0; i < componentCount; i++)
    {
        delete components[i];
    }
}
