// Definition of joint and skeleton structures.
#pragma once

#include <Eigen/Core>

#include <string>

// Forward declarations.
class GPCMJointData;

// Skeleton data structure.
struct GPCMSkeletonData
{
    // Total number of channels.
    int channelCount;
    // Number of joints.
    int jointCount;
    // Pointer to array of joints.
    GPCMJointData *joints;
    // Print all joint names with indices.
    void printJointNames();
};

using namespace Eigen;

class GPCMJointData
{
protected:
    // The joint's name.
    std::string name;
    // Parent index.
    int parent;
    // Offset from parent.
    Vector3d offset;
    // Indicates whether rotation is present.
    bool bHasRotation;
    // Indicates whether translation is present.
    bool bHasTranslation;
    // Actual order of rotations.
    int order[3];
    // Actual indices.
    int rotInd[3];
    // Actual indices.
    int posInd[3];
public:
    // Default constructor.
    GPCMJointData();
    // Constructor.
    GPCMJointData(std::string name, int parent, Vector3d offset,
        int *orderPtr, int *rotIndPtr, int *posIndPtr);
    // Get name of joint.
    std::string getName();
    // Get parent.
    int getParent();
    // Get offset.
    Vector3d getOffset();
    // Get entire rotational index array.
    const int *getRotInd();
    // Get entire positional index array.
    const int *getPosInd();
    // Get index of rotational joint.
    int getRotIndex(int axis);
    // Get joint order.
    const int *getOrder();
    // Set new joint order.
    void switchOrder(const int *order);
};
