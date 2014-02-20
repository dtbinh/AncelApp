// Abstract data loader.

#include "datareader.h"
#include "supplementary.h"
#include "mathutils.h"

#include <boost/tokenizer.hpp>

#include <Eigen/Dense>

// Do not look for nearest frames when this close.
#define FIND_NEAREST_TOLERANCE      10

// Create the data reader.
GPCMDataReader::GPCMDataReader()
{
	supplementary = nullptr;
}

// Apply whatever post-processing is required to the data.
void GPCMDataReader::postProcessData()
{
    // Nothing to do in the generic case.
}

// Load a corresponding auxiliary data file.
MatrixXd GPCMDataReader::loadAuxFile(
    std::string filename                    // By default, there is no auxiliary data.
    )
{
    return MatrixXd();
}

// Add noise to a data matrix.
void GPCMDataReader::addDataNoise(
    MatrixXd &data,                         // Data matrix to add noise to.
    double noise                            // How much noise to add.
    )
{
    if (noise > 0.0)
    {
        // First, measure the variance along each channel and scale by noise.
        int N = data.rows();
        MatrixXd bias = data.colwise().sum()/N;
        MatrixXd var = (data - bias.replicate(N,1)).colwise().squaredNorm()*(noise/N);
        
        // Now get normally distributed samples.
        MatrixXd samples(data.rows(),1);
        VectorXd gcov(1);
        for (int i = 0; i < data.cols(); i++)
        {
            gcov(0) = var(0,i);
            sampleGaussian(gcov,samples);
            data.col(i) += samples;
        }
    }
}

// Load the data.
void GPCMDataReader::load(
    std::vector<std::string> filelist,      // List of files from which to load the data.
    std::vector<double> noiselist           // Amount of noise to add to each file.
    )
{
    // Read data files.
    int i = 0;
    for (std::vector<std::string>::iterator itr = filelist.begin();
         itr != filelist.end(); ++itr)
    {
        // Load the file.
        MatrixXd data = loadFile(*itr);

        // Load auxiliary data.
        MatrixXd aux = loadAuxFile(*itr);

        // Check for blank auxiliary data.
        if (aux.cols() == 0)
            aux.setZero(data.rows(),1);

        // Add noise if desired.
        addDataNoise(data,noiselist[i]);

        // Concatenate into Y.
        MatrixXd newY(data.rows()+Y.rows(),data.cols());
        MatrixXd newAux(data.rows()+Y.rows(),aux.cols());

        // Store data.
        if (Y.rows() > 0)
            newY << Y,data;
        else
            newY = data;

        // Store auxiliary.
        if (auxData.rows() > 0)
            newAux << auxData,aux;
        else
            newAux = aux;

        // Switch over to the new matrices.
        Y = newY;
        auxData = newAux;

        // Concatenate into sequence.
        sequence.push_back(Y.rows());

        // Increment index.
        i++;
    }

    // Do any additional processing here, such as computing velocities and building supplementary
    // data structures.
    postProcessData();

    // Remove constant entries.
    MatrixXd constantEntries;
    std::vector<int> constantIndices;
    std::vector<int> variableIndices;
    int totalIndices = removeConstantEntries(Y,constantEntries,
        constantIndices,variableIndices);

    // Remove constant entries from scales.
    if (supplementary->getScale().cols() > 0)
    {
        MatrixXd newScale(1,variableIndices.size());
        for (unsigned i = 0; i < variableIndices.size(); i++)
        {
            newScale(0,i) = supplementary->getScale()(0,variableIndices[i]);
        }
        supplementary->getScale() = newScale;
    }

    // Pass duplicate entry information to supplement.
    supplementary->setConstant(constantEntries,constantIndices,variableIndices,totalIndices);
}

// Remove constant entries from data matrix.
int GPCMDataReader::removeConstantEntries(
    MatrixXd &channels,                     // Final channels, returns only variable entries.
    MatrixXd &constantEntries,              // Constant entries to return.
    std::vector<int> &constantIndices,      // Constant indices to return.
    std::vector<int> &variableIndices       // Variable indices to return.
    )
{
    // Compute variance in each column.
    int T = channels.rows();
    MatrixXd means = channels.colwise().sum()/T;
    MatrixXd vars = (channels - means.replicate(T,1)).colwise().squaredNorm()/T;

    // Step over all vars and decide where they go.
    for (int i = 0; i < channels.cols(); i++)
    {
        if (vars(0,i) < EPS_ANGLE)
            constantIndices.push_back(i);
        else
            variableIndices.push_back(i);
    }

    // Distribute results.
    MatrixXd newChannels(channels.rows(),variableIndices.size());
    constantEntries.resize(1,constantIndices.size());
    int k = 0;
    for (std::vector<int>::iterator itr = constantIndices.begin();
         itr != constantIndices.end(); itr++)
    {
        constantEntries(0,k) = means(0,*itr);
        k++;
    }
    k = 0;
    for (std::vector<int>::iterator itr = variableIndices.begin();
         itr != variableIndices.end(); itr++)
    {
        newChannels.block(0,k,channels.rows(),1) = channels.block(0,*itr,channels.rows(),1);
        k++;
    }

    // Copy over new channels structure.
    channels = newChannels;
    
    // Return total elements.
    return constantIndices.size()+variableIndices.size();
}

// Get back supplementary data.
GPCMSupplementaryData *GPCMDataReader::getSupplementary()
{
    return supplementary;
}

// Get back the design matrix.
MatrixXd GPCMDataReader::getYMatrix()
{
    return Y;
}

// Get back the auxiliary matrix.
MatrixXd GPCMDataReader::getAuxMatrix()
{
    return auxData;
}

// Get back the sequence list.
std::vector<int> GPCMDataReader::getSequence()
{
    return sequence;
}

// Destructor.
GPCMDataReader::~GPCMDataReader()
{
	 
}
