// Data loader for BVH motion capture files that uses quaternions to represent rotations.
#pragma once

#include "datareaderbvh.h"

class GPCMDataReaderBVHQuat : public GPCMDataReaderBVH
{
protected:
    // Convert rotations to exponential maps.
    virtual void convertRotations(const GPCMSkeletonData &skeleton, MatrixXd &channels);
    // Augment vector of positions with their velocities.
    virtual MatrixXd computeVelocities(const GPCMSkeletonData &skeleton, MatrixXd &channels, double frameTime);
    // Flip a single joint at each time step.
    virtual MatrixXd flipJoint(const MatrixXd &unflipped);
public:
    // Create the data reader.
    GPCMDataReaderBVHQuat();
    // Destructor.
    virtual ~GPCMDataReaderBVHQuat();
};
