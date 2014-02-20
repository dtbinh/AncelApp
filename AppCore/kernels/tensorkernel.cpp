// Tensor product kernel function.

#include "tensorkernel.h"
#include "mathutils.h"
#include "matwriter.h"
#include "matreader.h"
#include "debugprint.h"

// Construct the kernel.
GPCMTensorKernel::GPCMTensorKernel(
    GPCMParams &params,                     // Kernel parameters.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    GPCMKernel **components,                // Input array of components.
    int componentCount,                     // Size of array.
    int dims                                // Number of input dimensions
    ) : GPCMKernel(dims)
{
    // Set up parameter count and components.
    memcpy(this->components,components,sizeof(GPCMKernel*)*componentCount);
    this->componentCount = componentCount;
    this->params = 0;
    for (int i = 0; i < componentCount; i++)
    {
        this->params = this->params + components[i]->getParamCount();
    }

    // Set up input indices.
    // If this is an actual tensor kernel created in the script, its type should be tensor.
    if (!params["type"][0].compare("tensor"))
    { // Set all components to 0.
        for (int i = 0; i < componentCount; i++)
            this->componentInputs[i] = 0;
    }
    else
    { // Set components in order.
        for (int i = 0; i < componentCount; i++)
            this->componentInputs[i] = i;
    }

    // Set name.
    name = "tensor";
}

// Copy any settings from another kernel that we can.
void GPCMTensorKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
    if (other->getType() == getType())
    {
        GPCMTensorKernel *othercst = dynamic_cast<GPCMTensorKernel*>(other);
        if (othercst->componentCount == this->componentCount)
        {
            for (int i = 0; i < componentCount; i++)
            {
                this->components[i]->copySettings(othercst->components[i]);
            }
        }
        else
        {
            DBWARNING("Tensor kernel component count mismatch when initializing kernel parameters from another kernel!");
        }
    }
    else
    {
        DBWARNING("Kernel type mismatch when initializing kernel parameters from another kernel!");
    }
}

// Return white noise from this kernel.
double GPCMTensorKernel::getNoise()
{
    double noise = 0.0;
    for (int i = 0; i < componentCount; i++)
    {
        double cnoise = this->components[i]->getNoise();
        if (cnoise > noise)
            noise = cnoise;
    }
    return noise;
}

// Return covariance of a single set of points.
MatrixXd GPCMTensorKernel::covariance(
    const MatrixXd* const *X                // Data matrix.
    )
{
    MatrixXd result;
    MatrixXd K;
    const MatrixXd *XArray[MAX_KERNEL_GROUPS];
    memset(XArray,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    
    // Compute the total covariance as an elementwise product of the component
    // kernels. As we go, also compute the leave-one-out products.
    Kslash[0].setOnes(X[0]->rows(),X[0]->rows());
    for (int i = 0; i < componentCount; i++)
    {
        assert(X[this->componentInputs[i]] != NULL);
        XArray[0] = X[this->componentInputs[i]];

        K = this->components[i]->covariance(XArray);
        if (i == 0)
            result = K;
        else
            result = result.cwiseProduct(K);

        // Update leave-one-out products.
        if (i < componentCount-1)
            Kslash[i+1] = result;
        for (int j = 0; j < i; j++)
            Kslash[j] = Kslash[j].cwiseProduct(K);
    }
    return result;
}

// Return covariance of a single set of points along with gradients.
MatrixXd GPCMTensorKernel::covarianceGrad(
    const MatrixXd* const *X,               // Data matrix.
    MatrixXd &Xgrad                         // Gradient.
    )
{
    MatrixXd result;
    MatrixXd K;
    const MatrixXd *XArray[MAX_KERNEL_GROUPS];
    memset(XArray,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    
    // Compute the total covariance as an elementwise product of the component
    // kernels. As we go, also compute the leave-one-out products.
    Kslash[0].setOnes(X[0]->rows(),X[0]->rows());
    for (int i = 0; i < componentCount; i++)
    {
        assert(X[this->componentInputs[i]] != NULL);
        XArray[0] = X[this->componentInputs[i]];

        K = this->components[i]->covarianceGrad(XArray,KgradSlash[i]);
        if (i == 0)
            result = K;
        else
            result = result.cwiseProduct(K);

        // Update leave-one-out products.
        if (i < componentCount-1)
            Kslash[i+1] = result;
        for (int j = 0; j < i; j++)
            Kslash[j] = Kslash[j].cwiseProduct(K);
    }

    // Now that leave-one-out products are computed, compute gradients.
    int strt = 0;
    for (int i = 0; i < componentCount; i++)
    {
        int len = X[i]->cols();
        Xgrad.block(0,strt,Xgrad.rows(),len) = KgradSlash[i].cwiseProduct(Kslash[i].replicate(1,KgradSlash[i].cols()));
        strt += len;
    }

    return result;
}

// Return coviarance of two sets of points.
MatrixXd GPCMTensorKernel::covariance(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2               // Second data matrix.
    )
{
    MatrixXd result;
    const MatrixXd *X1Array[MAX_KERNEL_GROUPS];
    const MatrixXd *X2Array[MAX_KERNEL_GROUPS];
    memset(X1Array,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    memset(X2Array,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);

    // Compute the total covariance as an elementwise product.
    for (int i = 0; i < componentCount; i++)
    {
        assert(X1[this->componentInputs[i]] != NULL);
        assert(X2[this->componentInputs[i]] != NULL);
        X1Array[0] = X1[this->componentInputs[i]];
        X2Array[0] = X2[this->componentInputs[i]];

        if (i == 0)
            result = this->components[i]->covariance(X1Array,X2Array);
        else
            result = result.cwiseProduct(this->components[i]->covariance(X1Array,X2Array));
    }

    // Return result.
    return result;
}

// Return covariance of two sets of points along with gradients.
MatrixXd GPCMTensorKernel::covarianceGrad(
    const MatrixXd* const *X1,              // First data matrix.
    const MatrixXd* const *X2,              // Second data matrix.
    MatrixXd &X2grad                        // Gradient.
    )
{
    MatrixXd result;
    MatrixXd K;
    const MatrixXd *X1Array[MAX_KERNEL_GROUPS];
    const MatrixXd *X2Array[MAX_KERNEL_GROUPS];
    memset(X1Array,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    memset(X2Array,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);

    // Compute the total covariance as an elementwise product.
    Kslash[0].setOnes(X1[0]->rows(),X2[0]->rows());
    for (int i = 0; i < componentCount; i++)
    {
        assert(X1[this->componentInputs[i]] != NULL);
        assert(X2[this->componentInputs[i]] != NULL);
        X1Array[0] = X1[this->componentInputs[i]];
        X2Array[0] = X2[this->componentInputs[i]];

        KgradSlash[i].resize(X1Array[0]->rows(),X1Array[0]->cols());
        K = this->components[i]->covarianceGrad(X1Array,X2Array,KgradSlash[i]);
        if (i == 0)
            result = K;
        else
            result = result.cwiseProduct(K);

        // Update leave-one-out products.
        if (i < componentCount-1)
            Kslash[i+1] = result;
        for (int j = 0; j < i; j++)
            Kslash[j] = Kslash[j].cwiseProduct(K);
    }

    // Now that leave-one-out products are computed, compute gradients.
    int strt = 0;
    X2grad.setZero(X2grad.rows(),X2grad.cols());
    for (int i = 0; i < componentCount; i++)
    {
        int len = X1[this->componentInputs[i]]->cols();
        X2grad.block(0,strt,KgradSlash[i].rows(),len) += KgradSlash[i].cwiseProduct(Kslash[i].replicate(1,KgradSlash[i].cols()));
        if (i < componentCount-1 && this->componentInputs[i] != this->componentInputs[i+1])
            strt += len;
    }

    // Return result.
    return result;
}

// Recompute priors on all kernel parameters and return the likelihood.
double GPCMTensorKernel::recomputePriors()
{
    double result = 0;
    for (int i = 0; i < componentCount; i++)
    {
        result += this->components[i]->recomputePriors();
    }
    return result;
}

// Recompute gradients of hyperparameters and latent coordinates.
void GPCMTensorKernel::recompute(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    const MatrixXd *XArray[MAX_KERNEL_GROUPS];
    MatrixXd *XgradArray[MAX_KERNEL_GROUPS];
    memset(XArray,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    memset(XgradArray,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);

    for (int i = 0; i < componentCount; i++)
    {
        assert(X[this->componentInputs[i]] != NULL);
        XArray[0] = X[this->componentInputs[i]];
        XgradArray[0] = Xgrad[this->componentInputs[i]];
        this->components[i]->recompute(gK.cwiseProduct(Kslash[i]),gKd.cwiseProduct(Kslash[i]),XArray,XgradArray);
    }
}

// Return covariance of a single set of points in gradient observation mode.
MatrixXd GPCMTensorKernel::covarianceGradientGP(
    const MatrixXd* const *X                // Data matrix.
    )
{
    MatrixXd result;
    MatrixXd K;
    const MatrixXd *XArray[MAX_KERNEL_GROUPS];
    memset(XArray,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    
    // Compute the total covariance as an elementwise product of the component
    // kernels. As we go, also compute the leave-one-out products.
    Kslash[0].setOnes(X[0]->rows(),X[0]->rows());
    for (int i = 0; i < componentCount; i++)
    {
        assert(X[this->componentInputs[i]] != NULL);
        XArray[0] = X[this->componentInputs[i]];

        K = this->components[i]->covarianceGradientGP(XArray);
        if (i == 0)
            result = K;
        else
            result = result.cwiseProduct(K);

        // Update leave-one-out products.
        if (i < componentCount-1)
            Kslash[i+1] = result;
        for (int j = 0; j < i; j++)
            Kslash[j] = Kslash[j].cwiseProduct(K);
    }
    return result;
}

// Recompute gradients of hyperparameters and latent coordinates in gradient observation mode.
void GPCMTensorKernel::recomputeGradientGP(
    const MatrixXd &gK,                     // Gradient of objective with respect to kernel.
    const MatrixXd &gKd,                    // Gradient of objective with respect to diagonal kernel.
    const MatrixXd* const *X,               // Current latent positions.
    MatrixXd **Xgrad                        // Latent position gradient.
    )
{
    const MatrixXd *XArray[MAX_KERNEL_GROUPS];
    MatrixXd *XgradArray[MAX_KERNEL_GROUPS];
    memset(XArray,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);
    memset(XgradArray,0,sizeof(MatrixXd*)*MAX_KERNEL_GROUPS);

    for (int i = 0; i < componentCount; i++)
    {
        assert(X[this->componentInputs[i]] != NULL);
        XArray[0] = X[this->componentInputs[i]];
        XgradArray[0] = Xgrad[this->componentInputs[i]];
        this->components[i]->recomputeGradientGP(gK.cwiseProduct(Kslash[i]),gKd.cwiseProduct(Kslash[i]),XArray,XgradArray);
    }
}

// Write kernel data to file.
void GPCMTensorKernel::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    GPCMKernel::write(writer); // Let superclass write first.

    // First, determine the start index of each sub-component.
    int strt[MAX_KERNEL_GROUPS];
    strt[0] = 0;
    for (int i = 0; i < componentCount; i++)
    {
        if (this->componentInputs[i] < MAX_KERNEL_GROUPS)
            strt[this->componentInputs[i] + 1] = strt[this->componentInputs[i]] + this->components[i]->getDims();
    }

    // Create a cell array containing the structures for each component.
    GPCMMatWriter *cellWriter = writer->writeCell("comp",1,componentCount);
    for (int i = 0; i < componentCount; i++)
    {
        GPCMMatWriter *compStruct = cellWriter->writeStruct("",1,1);
        int d = this->components[i]->getDims();
        MatrixXd index(1,d);
        for (int j = 0; j < d; j++)
            index(0,j) = j+strt[this->componentInputs[i]]+1;
        compStruct->writeMatrix(index,"index");
        this->components[i]->write(compStruct);
        cellWriter->closeStruct();
    }
    writer->closeCell();

    // Some general stuff that is necessary for the Matlab code to work.
    writer->writeMatrix(MatrixXd::Identity(params,params),"paramGroups");
    writer->writeBlank("transforms");
}

// Load kernel from specified MAT file reader.
void GPCMTensorKernel::load(
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
void GPCMTensorKernel::printParams()
{
    DBPRINTLN("Tensor kernel:");
    for (int i = 0; i < componentCount; i++)
    {
        DBPRINTLN("Component " << i << ":");
        components[i]->printParams();
    }
    DBPRINTLN("Tensor kernel end");
}

// Destructor.
GPCMTensorKernel::~GPCMTensorKernel()
{
    // Delete the components.
    for (int i = 0; i < componentCount; i++)
    {
        delete components[i];
    }
}
