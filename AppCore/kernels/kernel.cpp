// Abstract kernel function.

#include "debugprint.h"
#include "distkernel.h"
#include "linkernel.h"
#include "rbfkernel.h"
#include "mlpkernel.h"
#include "whitekernel.h"
#include "biaskernel.h"
#include "compoundkernel.h"
#include "tensorkernel.h"
#include "matwriter.h"
#include "matreader.h"
#include "options.h"

#include <boost/tokenizer.hpp>

// Global function for creating a new kernel.
GPCMKernel *GPCMKernel::createKernel(
    GPCMParams &params,                     // Parameters of this kernel.
    GPCMOptions &options,                   // Loaded options used for creating other kernels.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    int dims,                               // Number of input dimensions.
    MatrixXd *dists                         // Optional distances matrix.
    )
{
    GPCMKernel *kernelArray[MAX_COMPONENTS];
    int i = 0;

    // Get the kernel category.
    std::string type = params["type"][0];

    if (!type.compare("compound") || !type.compare("tensor"))
    { // Create component kernels.
        std::vector<std::string> components = params["component"];
        for (std::vector<std::string>::iterator itr = components.begin();
             itr != components.end(); ++itr)
        {
            assert(i < MAX_COMPONENTS);
            GPCMParams comp_params = options[*itr];
            kernelArray[i] = createKernel(comp_params,options,optimization,dims,dists);
            i++;
        }
    }

    // Create and return the desired kernel.
    if (!type.compare("rbf"))
        return new GPCMRBFKernel(params,optimization,dims);
    else if (!type.compare("mlp"))
        return new GPCMMLPKernel(params,optimization,dims);
    else if (!type.compare("lin"))
        return new GPCMLinearKernel(params,optimization,dims);
    else if (!type.compare("white"))
        return new GPCMWhiteKernel(params,optimization,dims);
    else if (!type.compare("bias"))
        return new GPCMBiasKernel(params,optimization,dims);
    else if (!type.compare("compound"))
        return new GPCMCompoundKernel(params,optimization,kernelArray,i,dims);
    else if (!type.compare("tensor"))
        return new GPCMTensorKernel(params,optimization,kernelArray,i,dims);
    else if (!type.compare("dist"))
        return new GPCMDistanceKernel(params,optimization,dims,dists);
    else
        DBERROR("Unknown kernel " << type << " requested!");

    return NULL;
}

// Copy any settings from another kernel that we can.
void GPCMKernel::copySettings(
    GPCMKernel *other                       // Kernel to copy from.
    )
{
}

// Get the type of this kernel.
GPCMKernelType GPCMKernel::getType()
{
    return type;
}

// Construct the kernel.
GPCMKernel::GPCMKernel(
    int dims                                // Number of input dimensions
    ) : dims(dims), params(0), type(KernelTypeUnknown)
{
}

// Return white noise from this kernel.
double GPCMKernel::getNoise()
{
    return 0.0; // Most kernels have no noise.
}

// Return the number of dimensions the kernel can handle.
int GPCMKernel::getDims()
{
    return dims;
}

// Get number of parameters.
int GPCMKernel::getParamCount()
{
    return params;
}

// Write kernel data to file.
void GPCMKernel::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    writer->writeString(name,"type");
    writer->writeDouble((double)params,"nParams");
}

// Load kernel from specified MAT file reader.
void GPCMKernel::load(
    GPCMMatReader *reader                   // Reader to read model data from.
    )
{
}

// Print out kernel parameters.
void GPCMKernel::printParams()
{
    DBPRINTLN("No printout for this kernel type.");
}

// Destructor.
GPCMKernel::~GPCMKernel()
{
}
