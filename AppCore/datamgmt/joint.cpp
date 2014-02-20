// Definition of joint and skeleton structures.

#include "joint.h"
#include "mathutils.h"
#include "debugprint.h"

// Default constructor.
GPCMJointData::GPCMJointData()
{
    this->bHasRotation = false;
    this->bHasTranslation = false;
}

// Constructor.
GPCMJointData::GPCMJointData(
    std::string name,                       // The name of the new joint.
    int parent,                             // The joint parent's index.
    Vector3d offset,                        // Offset of joint.
    int *orderPtr,                          // Rotation order.
    int *rotIndPtr,                         // Indices of rotation DoFs.
    int *posIndPtr                          // Indices of translation DoFs.
    )
{
    this->name = name;
    this->parent = parent;
    this->offset = offset;
    this->bHasRotation = false;
    this->bHasTranslation = false;
    if (orderPtr)
    {
        this->bHasRotation = true;
        memcpy(this->order,orderPtr,sizeof(int)*3);
    }
    if (rotIndPtr)
    {
        this->bHasRotation = true;
        memcpy(this->rotInd,rotIndPtr,sizeof(int)*3);
    }
    if (posIndPtr)
    {
        this->bHasTranslation = true;
        memcpy(this->posInd,posIndPtr,sizeof(int)*3);
    }
}

// Get name of joint.
std::string GPCMJointData::getName()
{
    return name;
}

// Get parent.
int GPCMJointData::getParent()
{
    return parent;
}

// Get offset.
Vector3d GPCMJointData::getOffset()
{
    return offset;
}

// Get entire rotational index array.
const int *GPCMJointData::getRotInd()
{
    if (this->bHasRotation)
        return this->rotInd;
    else
        return NULL;
}

// Get entire positional index array.
const int *GPCMJointData::getPosInd()
{
    if (this->bHasTranslation)
        return this->posInd;
    else
        return NULL;
}

// Get index of rotational joint.
int GPCMJointData::getRotIndex(
    int axis                                // Axis for which to return the rotation.
    )
{
    if (this->bHasRotation)
        return this->rotInd[axis];
    else
        return INT_MAX;
}

// Get joint order.
const int *GPCMJointData::getOrder()
{
    return this->order;
}

// Set new joint order.
void GPCMJointData::switchOrder(
    const int *order                        // New order to use.
    )
{
    if (order)
    {
        // Rearrange rotInd.
        if (this->bHasRotation)
        {
            int newInd[3];
            for (int k = 0; k < 3; k++)
            {
                newInd[order[k]] = this->rotInd[this->order[k]];
            }
            memcpy(this->rotInd,newInd,sizeof(int)*3);
        }
        // Switch to new order.
        memcpy(this->order,order,sizeof(int)*3);
    }
    else
    {
        this->bHasRotation = false;
    }
}


void GPCMSkeletonData::printJointNames()
{
    for (int i = 0; i < jointCount; i++)
    {
        DBPRINTLN("" << i << "\t" << joints[i].getName());
    }
}
