// Abstract data loader.
#pragma once

#include <Eigen/Core>

#include <string>
#include <vector>

using namespace Eigen;

// Forward declarations.
class GPCMSupplementaryData;
class GPCMReward;

class GPCMDataReader
{
protected:
    // Loaded design matrix.
    MatrixXd Y;
    // Loaded auxiliary matrix.
    MatrixXd auxData;
    // Loaded sequence vector.
    std::vector<int> sequence;
    // Supplementary data.
    GPCMSupplementaryData *supplementary;
    // Remove constant entries from data matrix.
    int removeConstantEntries(MatrixXd &channels, MatrixXd &constantEntries,
        std::vector<int> &constantIndices, std::vector<int> &variableIndices);
    // Apply whatever post-processing is required to the data.
    virtual void postProcessData();
    // Add noise to a data matrix.
    virtual void addDataNoise(MatrixXd &data, double noise);
public:
    // Create the data reader.
    GPCMDataReader();
    // Load the data.
    virtual void load(std::vector<std::string> filelist, std::vector<double> noiselist);
    // Load a single file.
    virtual MatrixXd loadFile(std::string filename) = 0;
    // Load a corresponding auxiliary data file.
    virtual MatrixXd loadAuxFile(std::string filename);
    // Get back the design matrix.
    virtual MatrixXd getYMatrix();
    // Get back the auxiliary data matrix.
    virtual MatrixXd getAuxMatrix();
    // Get back the sequence list.
    virtual std::vector<int> getSequence();
    // Get back supplementary data.
    virtual GPCMSupplementaryData *getSupplementary();
    // Destructor.
    virtual ~GPCMDataReader();
};
