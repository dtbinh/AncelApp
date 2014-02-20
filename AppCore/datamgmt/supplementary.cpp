// Abstract supplementary data structure for data.

#include "supplementary.h"
#include "matwriter.h"

// Create supplementary data.
GPCMSupplementaryData::GPCMSupplementaryData()
{
}

// Set constant info.
void GPCMSupplementaryData::setConstant(
    MatrixXd &constantEntries,              // Constant entries.
    std::vector<int> &constantIndices,      // Constant entry indices.
    std::vector<int> &variableIndices,      // Variable entry indices.
    int totalIndices                        // Total indices.
    )
{
    this->constantEntries = constantEntries;
    this->constantIndices = constantIndices;
    this->variableIndices = variableIndices;
    this->totalIndices = totalIndices;
    // Initialize split information.
    isVelocity.resize(variableIndices.size());
    fullToSplit.resize(variableIndices.size());
    for (unsigned i = 0; i < variableIndices.size(); i++)
    {
        isVelocity[i] = 0;
        fullToSplit[i] = i;
    }
    
}

// Number of position indices.
int GPCMSupplementaryData::positionCount()
{
    return totalIndices;
}

// Number of position indices.
int GPCMSupplementaryData::velocityCount()
{
    return 0;
}

// Get total number of indices.
int GPCMSupplementaryData::getTotalIndices()
{
    return totalIndices;
}

// Get length of a single frame.
double GPCMSupplementaryData::getFrameTime()
{
    return frameTime;
}

// Split apart the position and velocity portions of the data matrix.
void GPCMSupplementaryData::splitVelocity(
    MatrixXd &positions,                    // Output is position terms, input is data matrix.
    MatrixXd &velocities                    // Output is velocity terms.
    )
{
    // By default, there are no velocity terms.
    // Do nothing.
}

// Split apart the position and velocity portions of the input matrix.
void GPCMSupplementaryData::splitMatrix(
    const MatrixXd &matrixIn,               // Input Matrix
    MatrixXd *positions,                    // Output is position terms.
    MatrixXd *velocities                    // Output is velocity terms.
    )
{
    // Do nothing
}

// Return vector of indices corresponding to position entries in the data matrix (after removing constant entries).
std::vector<int> GPCMSupplementaryData::getPositionIndices()
{
    // Construct vector from fullToSplit and isVelocity.
    int num = 0;
    for (unsigned i = 0; i < isVelocity.size(); i++)
        num += (isVelocity[i] == 0);
    std::vector<int> ret;
    ret.resize(num);
    for (unsigned i = 0; i < isVelocity.size(); i++)
    {
        if (!isVelocity[i])
            ret[fullToSplit[i]] = i;
    }
    return ret;
}

// Return vector of indices corresponding to position entries in the full pose matrix (with constant entries).
std::vector<int> GPCMSupplementaryData::getPositionIndicesInFullPose()
{
    // Construct vector from fullToSplit and isVelocity.
    int num = 0;
    for (unsigned i = 0; i < isVelocity.size(); i++)
        num += (isVelocity[i] == 0);
    std::vector<int> ret;
    ret.resize(num);
    for (unsigned i = 0; i < isVelocity.size(); i++)
    {
        if (!isVelocity[i])
            ret[fullToSplit[i]] = variableIndices[i];
    }
    return ret;
}


// Return vector of indices corresponding to velocity entries in the data matrix.
std::vector<int> GPCMSupplementaryData::getVelocityIndices()
{
    // Construct vector from fullToSplit and isVelocity.
    int num = 0;
    for (unsigned i = 0; i < isVelocity.size(); i++)
        num += (isVelocity[i] != 0);
    std::vector<int> ret;
    ret.resize(num);
    for (unsigned i = 0; i < isVelocity.size(); i++)
    {
        if (isVelocity[i])
            ret[fullToSplit[i]] = i;
    }
    return ret;
}


// Return vector of indices corresponding to velocity entries in the full pose matrix (with constant entries).
std::vector<int> GPCMSupplementaryData::getVelocityIndicesInFullPose()
{
    // Construct vector from fullToSplit and isVelocity.
    int num = 0;
    for (unsigned i = 0; i < isVelocity.size(); i++)
        num += (isVelocity[i] != 0);
    std::vector<int> ret;
    ret.resize(num);
    for (unsigned i = 0; i < isVelocity.size(); i++)
    {
        if (isVelocity[i])
            ret[fullToSplit[i]] = variableIndices[i];
    }
    return ret;
}

// Fill full pose term.
void GPCMSupplementaryData::fillFullY(
    const MatrixXd &Yin,                    // Input Y.
    const MatrixXd &Vin,                    // Input V.
    MatrixXd &Y                             // Output.
    )
{
    // Resize output.
    Y.resize(Yin.rows(),totalIndices);

    // Fill in constant entries.
    for (unsigned i = 0; i < constantIndices.size(); i++)
        Y.col(constantIndices[i]) = constantEntries.col(i).replicate(Y.rows(),1);

    // Fill in variable entries.
    for (unsigned i = 0; i < variableIndices.size(); i++)
    {
        if (isVelocity[i])
        {
            if (Vin.cols() > 0)
                Y.col(variableIndices[i]) = Vin.col(fullToSplit[i]);
            else
                Y.col(variableIndices[i]).setZero();
        }
        else
        {
            Y.col(variableIndices[i]) = Yin.col(fullToSplit[i]);
        }
    }
}

// Get reference to scales.
MatrixXd &GPCMSupplementaryData::getScale()
{
    return scales;
}

// Write supplementary data.
void GPCMSupplementaryData::write(
    GPCMMatWriter *writer                   // Stream to write supplementary data to.
    )
{
    // Write fullD, the width of the full design matrix.
    writer->writeDouble((double)totalIndices,"fullD");

    // Write Ydegen, the matrix of constant values.
    writer->writeMatrix(constantEntries,"Ydegen");

    // Write Yz, indices of constant entries.
    MatrixXd YzMat(1,constantIndices.size());
    for (unsigned i = 0; i < constantIndices.size(); i++)
        YzMat(0,i) = constantIndices[i]+1;
    writer->writeMatrix(YzMat,"Yz");

    // Write Ynz, indices of variable entries.
    MatrixXd YnzMat(1,variableIndices.size());
    for (unsigned i = 0; i < variableIndices.size(); i++)
        YnzMat(0,i) = variableIndices[i]+1;
    writer->writeMatrix(YnzMat,"Ynz");
}

// Destructor.
GPCMSupplementaryData::~GPCMSupplementaryData()
{
}
