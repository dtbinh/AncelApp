// Data loader for BVH motion capture files.
#pragma once

#include "datareader.h"
#include "joint.h"
#include <fstream>

// Forward declarations.
class GPCMSupplementaryData;
class GPCMSupplementaryBVH;

class GPCMDataReaderBVH : public GPCMDataReader
{
protected:
    // Number of position entries.
    int positionEntries;
    // Number of velocity entries.
    int velocityEntries;
    // Scale applied to rotation entries.
    double rotationScale;
    // Scale applied to rotation velocity entries.
    double rotationVelocityScale;
    // Rotation reparameterization.
    std::string rotationParam;
    // Compute initial scales.
    virtual void setInitScales(GPCMSkeletonData &skeleton, int config, int chans);
    // Read in data about the skeleton.
    GPCMSkeletonData loadSkeleton(std::ifstream &bvhstream);
    // Convert the root rotation to desired joint order.
    void transformRoot(GPCMSkeletonData &skeleton, MatrixXd &channels, const int *order);
    // Smooth out all rotations.
    void smoothRotations(const GPCMSkeletonData &skeleton, MatrixXd &channels);
    // Transform channels to have relative root positions.
    void makeRootRelative(const GPCMSkeletonData &skeleton, MatrixXd &channels, double frameTime);
    // Convert rotations to exponential maps.
    virtual void convertRotations(const GPCMSkeletonData &skeleton, MatrixXd &channels);
    // Augment vector of positions with their velocities.
    virtual MatrixXd computeVelocities(const GPCMSkeletonData &skeleton, MatrixXd &channels, double frameTime);
    // Check that there are no discontinuities in any of the channels.
    void validateChannels(const GPCMSkeletonData &skeleton, const MatrixXd &channels);
    // Apply whatever post-processing is required to the data.
    virtual void postProcessData();
    // Flip a single joint at each time step.
    virtual MatrixXd flipJoint(const MatrixXd &unflipped);
public:
    // Create the data reader.
    GPCMDataReaderBVH();
    // Load a single file.
    virtual MatrixXd loadFile(std::string filename);
    // Load a corresponding auxiliary data file.
    virtual MatrixXd loadAuxFile(std::string filename);
    // Destructor.
    virtual ~GPCMDataReaderBVH();
};
