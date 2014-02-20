// Abstract supplementary data structure for data.
#pragma once

#include <fstream>
#include <vector>

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;

class GPCMSupplementaryData
{
protected:
    // Optional matrix containing scales.
    MatrixXd scales;
    // Constant entries in data matrix.
    MatrixXd constantEntries;
    // Indices of constant entries.
    std::vector<int> constantIndices;
    // Indices of variable entries.
    std::vector<int> variableIndices;
    // Indices of position entries in the full pose matrix.
    std::vector<int> positionIndices;
    // Indices of variable entries in the full pose matrix.
    std::vector<int> velocityIndices;
    // Indicates whether specified entry comes from the velocity GP.
    std::vector<int> isVelocity;
    // Indicates where specified entry comes from in split pose/velocity vectors.
    std::vector<int> fullToSplit;
    // Total number of entries.
    int totalIndices;
    // Seconds per frame.
    double frameTime;
public:
    // Create supplementary data.
    GPCMSupplementaryData();
    // Write supplementary data.
    virtual void write(GPCMMatWriter *file);
    // Set constant info.
    virtual void setConstant(MatrixXd &constantEntries, std::vector<int> &constantIndices,
        std::vector<int> &variableIndices, int totalIndices);
    // Get number of position indices.
    virtual int positionCount();
    // Get number of velocity indices.
    virtual int velocityCount();
    // Get total number of indices.
    virtual int getTotalIndices();
    // Get length of a single frame.
    virtual double getFrameTime();
    // Get reference to scales.
    virtual MatrixXd &getScale();
    // Split apart the position and velocity portions of the data matrix.
    virtual void splitVelocity(MatrixXd &positions, MatrixXd &velocities);
    // Split apart the position and velocity portions of the input matrix.
    virtual void splitMatrix(const MatrixXd &matrixIn, MatrixXd *positions, MatrixXd *velocities);
    // Return vector of indices corresponding to position entries in the data matrix (after removing constant entries).
    virtual std::vector<int> getPositionIndices();
    // Return vector of indices corresponding to position entries in the full pose matrix (with constant entries).
    virtual std::vector<int> getPositionIndicesInFullPose();
    // Return vector of indices corresponding to velocity entries in the data matrix.
    virtual std::vector<int> getVelocityIndices();
    // Return vector of indices corresponding to velocity entries in the full pose matrix (with constant entries).
    virtual std::vector<int> getVelocityIndicesInFullPose();
    // Fill full pose term.
    virtual void fillFullY(const MatrixXd &Yin, const MatrixXd &Vin, MatrixXd &Y);
    // Destructor.
    virtual ~GPCMSupplementaryData();
};
