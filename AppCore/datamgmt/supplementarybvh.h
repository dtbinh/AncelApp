// Supplementary data structure for BVH data.
#pragma once

#include "supplementary.h"
#include "joint.h"

// Positions of joints in the data vector.
#define HX_DOF      0
#define HY_DOF      1
#define HZ_DOF      2
#define YAW_DOF     3

class GPCMSupplementaryBVH : public GPCMSupplementaryData
{
protected:
    // Skeleton associated with BVH file.
    GPCMSkeletonData skeleton;
    // Number of channels that specify the configuration (as opposed to velocity).
    int configChannels;
    // Rotation parameterization.
    std::string rotationParam;
    // Rotate a position by a joint's rotation for computing global position.
    void rotateByJoint(MatrixXd &out, MatrixXd &oldPos, MatrixXd &angles, VectorXd &ce, VectorXd &se,
        VectorXd &norm, const MatrixXd &data, int i, int jntIdx, MatrixXd *dPdY);
    // Compute rotation matrix for joint.
    void rotationMatrixFromJoint(VectorXd &norm, const MatrixXd &data, int i, int jntIdx, Matrix3d &rot);
    // Helper function for determining joint global position.
    void getJointGlobalHelper(MatrixXd &out, MatrixXd &oldPos, MatrixXd &angles, VectorXd &ce, VectorXd &se,
        VectorXd &norm, const MatrixXd &data, int i, bool bUseRoot, bool bUseRelative, MatrixXd *dPdY,
        Matrix3d &rotParent, Matrix3d &rot);
public:
    // Create supplementary data.
    GPCMSupplementaryBVH(GPCMSkeletonData skeleton, double frameTime, int configChannels,
        std::string rotationParam);
    // Number of position indices.
    virtual int positionCount();
    // Number of velocity indices.
    virtual int velocityCount();
    // Write supplementary data.
    virtual void write(GPCMMatWriter *file);
    // Split apart the position and velocity portions of the data matrix.
    virtual void splitVelocity(MatrixXd &positions, MatrixXd &velocities);
    // Get global position for a joint.
    virtual void getJointGlobal(MatrixXd &out, const MatrixXd &data, int joint, bool bUseRoot,
        bool bUseRelative = false, MatrixXd *dPdY = NULL);
    // Evaluate forward kinematics for a pose.
    virtual void forwardKinematics(Matrix4d *transforms, const MatrixXd &pose, bool bUseRoot, bool bUseRelative = false);
    // Transform the new pose to have the correct global root
    virtual void makeRootAbsolute(const MatrixXd &oldPose, MatrixXd *newPose);
    // Destructor.
    virtual ~GPCMSupplementaryBVH();
    // Get frame length.
    double getFrameTime();
    // Get skeleton.
    GPCMSkeletonData &getSkeleton();
    // Get number of configuration channels.
    int getConfigChannels();
    // Split apart the position and velocity portions of the input matrix.
    virtual void splitMatrix(const MatrixXd &matrixIn, MatrixXd *positions, MatrixXd *velocities);
};
