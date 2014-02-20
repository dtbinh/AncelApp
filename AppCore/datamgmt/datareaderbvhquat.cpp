// Data loader for BVH motion capture files that uses quaternions to represent rotations.

#include "debugprint.h"
#include "datareaderbvhquat.h"
#include "mathutils.h"

#include <Eigen/Geometry>

// Scaling constants.
#define QUAT_SCALE  1.0
#define ANG_SCALE   (1.0/180.0)

// Create the data reader.
GPCMDataReaderBVHQuat::GPCMDataReaderBVHQuat()
{
    rotationParam = "quat";
}

// Flip a single joint at each time step.
MatrixXd GPCMDataReaderBVHQuat::flipJoint(
    const MatrixXd &unflipped
    )
{
    return -unflipped; // Simply negate the quaternion.
}

// Convert rotations to exponential maps.
void GPCMDataReaderBVHQuat::convertRotations(
    const GPCMSkeletonData &skeleton,       // Current skeleton.
    MatrixXd &channels                      // Untransformed, relative-root channels.
    )
{
    // Create new channels structure for output.
    MatrixXd newChannels(channels.rows(),((channels.cols()-3)*4)/3+4);
    newChannels.block(0,0,channels.rows(),4) = channels.block(0,0,channels.rows(),4);

    // Set number of entries.
    positionEntries = 4;
    velocityEntries = 3;
    rotationScale = QUAT_SCALE;
    rotationVelocityScale = ANG_SCALE;

    for (int t = 0; t < channels.rows(); t++)
    {
        for (int j = 0; j < skeleton.jointCount; j++)
        {
            const int *order = skeleton.joints[j].getOrder();
            const int *rotInd = skeleton.joints[j].getRotInd();
            if (rotInd)
            { // If this joint has a rotation, convert it.
                Vector3d euler;
                if (j == 0)
                { // Use zero for yaw.
                    // Note: we use standard order here to maintain compatibility
                    // with the Matlab implementation. It would be correct to
                    // simply use "order" here, using the correct (different)
                    // order for the root joint, but this will make the Matlab
                    // version unable to read the result correctly.
                    euler << 0,
                             channels(t,rotInd[order[1]]),
                             channels(t,rotInd[order[2]]);
                }
                else
                { // Simply convert in place.
                    euler << channels(t,rotInd[order[0]]),
                             channels(t,rotInd[order[1]]),
                             channels(t,rotInd[order[2]]);
                }
                Quaterniond quat = eulerToQuat(euler,order);
                // Determine the correct output indices.
                assert(rotInd[order[1]] == rotInd[order[0]]+1 && rotInd[order[2]] == rotInd[order[0]]+2);
                int jntIdx = ((rotInd[order[0]]-3)/3)*4 + 4;
                newChannels(t,jntIdx+0) = quat.x();
                newChannels(t,jntIdx+1) = quat.y();
                newChannels(t,jntIdx+2) = quat.z();
                newChannels(t,jntIdx+3) = quat.w();
                // Check if we need to flip.
                if (t > 0)
                {
                    double thisDist = 0.0;
                    double flipDist = 0.0;
                    for (int k = 0; k < 4; k++)
                    {
                        thisDist += pow(newChannels(t-1,jntIdx+k)-newChannels(t,jntIdx+k),2);
                        flipDist += pow(newChannels(t-1,jntIdx+k)+newChannels(t,jntIdx+k),2);
                    }
                    if (flipDist < thisDist)
                    { // Flip quaternion to face the other way.
                        for (int k = 0; k < 4; k++)
                            newChannels(t,jntIdx+k) = -newChannels(t,jntIdx+k);
                    }
                }
            }
        }
    }

    // Return result.
    channels = newChannels;
}

// Augment vector of positions with their velocities.
MatrixXd GPCMDataReaderBVHQuat::computeVelocities(
    const GPCMSkeletonData &skeleton,       // Current skeleton.
    MatrixXd &channels,                     // Transformed, relative-root channels.
    double frameTime                        // Length of each frame.
    )
{
    // Create new channels structure for output.
    MatrixXd newChannels(channels.rows(),channels.cols() + ((channels.cols()-4)*3)/4);
    newChannels.block(0,0,channels.rows(),channels.cols()) = channels;

    for (int t = 0; t < channels.rows(); t++)
    {
        for (int j = 0; j < skeleton.jointCount; j++)
        {
            const int *order = skeleton.joints[j].getOrder();
            const int *rotInd = skeleton.joints[j].getRotInd();
            if (rotInd)
            { // If this joint has a rotation, differentiate it.
                // Figure out indices.
                int curridx = t;
                int previdx = t-1;
                if (previdx < 0)
                { // Don't have a previous frame, so use the next one.
                    curridx = t+1;
                    previdx = t;
                }

                // Get indices.
                assert(rotInd[order[1]] == rotInd[order[0]]+1 && rotInd[order[2]] == rotInd[order[0]]+2);
                int jntIdx = ((rotInd[order[0]]-3)/3)*4 + 4;

                // Compute angular velocity.
                Vector4d prev, curr;
                prev << newChannels(previdx,jntIdx+0),
                        newChannels(previdx,jntIdx+1),
                        newChannels(previdx,jntIdx+2),
                        newChannels(previdx,jntIdx+3);
                curr << newChannels(curridx,jntIdx+0),
                        newChannels(curridx,jntIdx+1),
                        newChannels(curridx,jntIdx+2),
                        newChannels(curridx,jntIdx+3);

                // Compute difference between two quaternions.
                Quaterniond prevQuat(prev);
                Quaterniond currQuat(curr);
                Quaterniond diffQuat = prevQuat.inverse()*currQuat;
                diffQuat.w() = fabs(diffQuat.w());

                // Convert difference to exponential map.
                Vector3d diff = quatToExp(diffQuat);
                assert(diff.norm() <= 180.0);

                // Divide by frameTime.
                diff /= frameTime;

                // Compute index and store.
                jntIdx = ((jntIdx-4)*3)/4 + channels.cols();
                newChannels(t,jntIdx+0) = diff(0);
                newChannels(t,jntIdx+1) = diff(1);
                newChannels(t,jntIdx+2) = diff(2);
            }
        }
    }

    // Return result.
    return newChannels;
}

// Destructor.
GPCMDataReaderBVHQuat::~GPCMDataReaderBVHQuat()
{
}
