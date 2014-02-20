// Supplementary data structure for BVH data.

#include "supplementarybvh.h"
#include "matwriter.h"
#include "mathutils.h"
#include "debugprint.h"

#include <vector>

#include <Eigen/Geometry>

// Largest number of joints we expect to see.
#define MAX_JOINTS  128

// Create supplementary data.
GPCMSupplementaryBVH::GPCMSupplementaryBVH(
    GPCMSkeletonData skeleton,              // Description of BVH skeleton.
    double frameTime,                       // Number of seconds per frame.
    int configChannels,                     // Number of configuration channels.
    std::string rotationParam               // Rotation reparameterization.
    ) : skeleton(skeleton), configChannels(configChannels), rotationParam(rotationParam)
{
    this->frameTime = frameTime;
}

// Get skeleton.
GPCMSkeletonData &GPCMSupplementaryBVH::getSkeleton()
{
    return skeleton;
}

// Number of position indices.
int GPCMSupplementaryBVH::positionCount()
{
    return configChannels-3;
}

// Number of velocity indices.
int GPCMSupplementaryBVH::velocityCount()
{
    return totalIndices-configChannels+3;
}

// Evaluate forward kinematics for a pose.
void GPCMSupplementaryBVH::forwardKinematics(
    Matrix4d *transforms,                   // Array of transforms at least as large as the number of points.
    const MatrixXd &pose,                   // Pose row vector.
    bool bUseRoot,                          // Whether to add the root position and rotation.
    bool bUseRelative                       // Root is relative.
    )
{
    Matrix3d afnMat = Matrix3d::Identity();
    Matrix4d rotMat = Matrix4d::Identity();
    for (int i = 0; i < skeleton.jointCount; i++)
    { // Step over all joints.
        // Initialize.
        transforms[i] = Matrix4d::Identity();

        // Rotate by joint rotation.
        const int *order = skeleton.joints[i].getOrder();
        const int *rotInd = skeleton.joints[i].getRotInd();
        if (rotInd)
        {
            int jntIdx = ((rotInd[order[0]]-3)/3)*4 + 4;
            if (i == 0)
            {
                // For other joints, convert to Euler angles.
                double norm = pose.block(0,jntIdx,1,4).squaredNorm();
                double angle0 = asin(2.0*((pose(0,jntIdx+3)*pose(0,jntIdx+2) + pose(0,jntIdx+1)*pose(0,jntIdx+0))/norm));
                double angle1 = atan2(2.0*((pose(0,jntIdx+3)*pose(0,jntIdx+0) - pose(0,jntIdx+1)*pose(0,jntIdx+2))/norm),(pow(pose(0,jntIdx+3),2) - pow(pose(0,jntIdx+0),2) + pow(pose(0,jntIdx+1),2) - pow(pose(0,jntIdx+2),2))/norm);
                // Apply roll about X axis.
                afnMat(0,0) = 1.0;
                afnMat(0,1) = afnMat(0,2) = afnMat(1,0) = afnMat(2,0) = 0.0;
                afnMat(1,1) = cos(angle1);
                afnMat(1,2) = -sin(angle1);
                afnMat(2,1) = sin(angle1);
                afnMat(2,2) = cos(angle1);
                // Apply pitch about Z axis.
                transforms[i](0,0) = cos(angle0);
                transforms[i](0,1) = -sin(angle0);
                transforms[i](1,0) = sin(angle0);
                transforms[i](1,1) = cos(angle0);
                transforms[i].block(0,0,3,3) *= afnMat;
            }
            else
            {
                // For standard joints, just use the quaternion.
                double norm = 1.0/(pose.block(0,jntIdx,1,4).norm());
                afnMat = Quaterniond(pose(0,jntIdx+3)*norm,pose(0,jntIdx+0)*norm,pose(0,jntIdx+1)*norm,pose(0,jntIdx+2)*norm);
                transforms[i].block(0,0,3,3) = afnMat;
            }
        }

        // Apply joint translation.
        transforms[i].block(0,3,3,1) += skeleton.joints[i].getOffset();

        // Apply root transformation if necessary.
        if (i == 0 && bUseRoot)
        { // For root joints, apply root rotation and translation.
            if (bUseRelative)
            {
                afnMat = AngleAxisd(DEG_TO_RAD(pose(0,3)*frameTime),Vector3d::UnitY());
                rotMat.block(0,0,3,3) = afnMat;
                transforms[i] = rotMat*transforms[i];
                transforms[i](0,3) += frameTime * (pose(0,0) * cos(DEG_TO_RAD(pose(0,3)*frameTime)) + pose(0,2) * sin(DEG_TO_RAD(pose(0,3)*frameTime)));
                transforms[i](1,3) += pose(0,1);
                transforms[i](2,3) += frameTime * (pose(0,2) * cos(DEG_TO_RAD(pose(0,3)*frameTime)) - pose(0,0) * sin(DEG_TO_RAD(pose(0,3)*frameTime)));;

            }
            else
            {
                afnMat = AngleAxisd(DEG_TO_RAD(pose(0,3)),Vector3d::UnitY());
                rotMat.block(0,0,3,3) = afnMat;
                transforms[i] = rotMat*transforms[i];
                transforms[i](0,3) += pose(0,0);
                transforms[i](1,3) += pose(0,1);
                transforms[i](2,3) += pose(0,2);
            }
        }

        // Apply parent transformation.
        int parent = skeleton.joints[i].getParent();
        if (parent != -1)
            transforms[i] = transforms[parent]*transforms[i];
    }
}

// Rotate a position by a joint's rotation for computing global position.
void GPCMSupplementaryBVH::rotateByJoint(
    MatrixXd &out,                          // Output matrix for storing global positions.
    MatrixXd &oldPos,                       // Temporary storage for previous position.
    MatrixXd &angles,                       // Temporary angles storage.
    VectorXd &ce,                           // Temporary cosine storage.
    VectorXd &se,                           // Temporary sine storage.
    VectorXd &norm,                         // Temporary normalization storage.
    const MatrixXd &data,                   // Input poses.
    int i,                                  // Joint.
    int jntIdx,                             // First angle index.
    MatrixXd *dPdY                          // Jacobian of joint with respect to Y.
    )
{
    // Compute indices.
    int xidx = jntIdx+0;
    int yidx = jntIdx+1;
    int zidx = jntIdx+2;
    int widx = jntIdx+3;

    // Compute quaternion normalization.
    norm = data.block(0,xidx,data.rows(),4).rowwise().squaredNorm();
    oldPos = out;

    if (i == 0)
    {
        // Compute the Jacobian.
        if (dPdY)
        {
            // Compute u, v, and w.
            double a = (2.0/norm(0,0))*(data(0,widx)*data(0,zidx) + data(0,yidx)*data(0,xidx));
            double b = (2.0/norm(0,0))*(data(0,widx)*data(0,xidx) - data(0,yidx)*data(0,zidx));
            double c = (1.0/norm(0,0))*(pow(data(0,widx),2) - pow(data(0,xidx),2) + pow(data(0,yidx),2) - pow(data(0,zidx),2));

            // Compute sines and cosines.
            double c0 = sqrt(1.0-pow(a,2));
            double s0 = a;
            double c1 = c/sqrt(pow(b,2)+pow(c,2));
            double s1 = b/sqrt(pow(b,2)+pow(c,2));

            // Compute gradient of normalizing factor.
            double dNormdx = 2.0*data(0,xidx);
            double dNormdy = 2.0*data(0,yidx);
            double dNormdz = 2.0*data(0,zidx);
            double dNormdw = 2.0*data(0,widx);

            // Compute the gradients of a, b, c with respect to data and normalizing factor.
            double dadnorm = -a/norm(0,0);
            double dbdnorm = -b/norm(0,0);
            double dcdnorm = -c/norm(0,0);
            double dadx = (2.0/norm(0,0))*data(0,yidx)  + dadnorm*dNormdx;
            double dady = (2.0/norm(0,0))*data(0,xidx)  + dadnorm*dNormdy;
            double dadz = (2.0/norm(0,0))*data(0,widx)  + dadnorm*dNormdz;
            double dadw = (2.0/norm(0,0))*data(0,zidx)  + dadnorm*dNormdw;
            double dbdx = (2.0/norm(0,0))*data(0,widx)  + dbdnorm*dNormdx;
            double dbdy = (-2.0/norm(0,0))*data(0,zidx) + dbdnorm*dNormdy;
            double dbdz = (-2.0/norm(0,0))*data(0,yidx) + dbdnorm*dNormdz;
            double dbdw = (2.0/norm(0,0))*data(0,xidx)  + dbdnorm*dNormdw;
            double dcdx = (-2.0/norm(0,0))*data(0,xidx) + dcdnorm*dNormdx;
            double dcdy = (2.0/norm(0,0))*data(0,yidx)  + dcdnorm*dNormdy;
            double dcdz = (-2.0/norm(0,0))*data(0,zidx) + dcdnorm*dNormdz;
            double dcdw = (2.0/norm(0,0))*data(0,widx)  + dcdnorm*dNormdw;

            // Compute the gradients of the sines and cosines with respect to a, b, c.
            double dc0da = -a/sqrt(1.0-pow(a,2));
            double ds0da = 1;
            double dc1dc = pow(b,2)/pow(sqrt(pow(b,2)+pow(c,2)),3);
            double dc1db = -b*c/pow(sqrt(pow(b,2)+pow(c,2)),3);
            double ds1dc = -b*c/pow(sqrt(pow(b,2)+pow(c,2)),3);
            double ds1db = pow(c,2)/pow(sqrt(pow(b,2)+pow(c,2)),3);

            // Compute the gradients of the positions with respect to sines and cosines.
            double dpxdc0 = oldPos(0,0);
            double dpxds0 = -c1*oldPos(0,1) + s1*oldPos(0,2);
            double dpxdc1 = -s0*oldPos(0,1);
            double dpxds1 = s0*oldPos(0,2);
            double dpydc0 = c1*oldPos(0,1) - s1*oldPos(0,2);
            double dpyds0 = oldPos(0,0);
            double dpydc1 = c0*oldPos(0,1);
            double dpyds1 = -c0*oldPos(0,2);
            double dpzdc1 = oldPos(0,2);
            double dpzds1 = oldPos(0,1);

            // Use chain rule to compute final gradients.
            (*dPdY)(0,xidx) = (dpxdc0*dc0da + dpxds0*ds0da)*dadx +
                              dpxdc1*(dc1db*dbdx + dc1dc*dcdx) +
                              dpxds1*(ds1db*dbdx + ds1dc*dcdx);
            (*dPdY)(0,yidx) = (dpxdc0*dc0da + dpxds0*ds0da)*dady +
                              dpxdc1*(dc1db*dbdy + dc1dc*dcdy) +
                              dpxds1*(ds1db*dbdy + ds1dc*dcdy);
            (*dPdY)(0,zidx) = (dpxdc0*dc0da + dpxds0*ds0da)*dadz +
                              dpxdc1*(dc1db*dbdz + dc1dc*dcdz) +
                              dpxds1*(ds1db*dbdz + ds1dc*dcdz);
            (*dPdY)(0,widx) = (dpxdc0*dc0da + dpxds0*ds0da)*dadw +
                              dpxdc1*(dc1db*dbdw + dc1dc*dcdw) +
                              dpxds1*(ds1db*dbdw + ds1dc*dcdw);
            (*dPdY)(1,xidx) = (dpydc0*dc0da + dpyds0*ds0da)*dadx +
                              dpydc1*(dc1db*dbdx + dc1dc*dcdx) +
                              dpyds1*(ds1db*dbdx + ds1dc*dcdx);
            (*dPdY)(1,yidx) = (dpydc0*dc0da + dpyds0*ds0da)*dady +
                              dpydc1*(dc1db*dbdy + dc1dc*dcdy) +
                              dpyds1*(ds1db*dbdy + ds1dc*dcdy);
            (*dPdY)(1,zidx) = (dpydc0*dc0da + dpyds0*ds0da)*dadz +
                              dpydc1*(dc1db*dbdz + dc1dc*dcdz) +
                              dpyds1*(ds1db*dbdz + ds1dc*dcdz);
            (*dPdY)(1,widx) = (dpydc0*dc0da + dpyds0*ds0da)*dadw +
                              dpydc1*(dc1db*dbdw + dc1dc*dcdw) +
                              dpyds1*(ds1db*dbdw + ds1dc*dcdw);
            (*dPdY)(2,xidx) = dpzdc1*(dc1db*dbdx + dc1dc*dcdx) +
                              dpzds1*(ds1db*dbdx + ds1dc*dcdx);
            (*dPdY)(2,yidx) = dpzdc1*(dc1db*dbdy + dc1dc*dcdy) +
                              dpzds1*(ds1db*dbdy + ds1dc*dcdy);
            (*dPdY)(2,zidx) = dpzdc1*(dc1db*dbdz + dc1dc*dcdz) +
                              dpzds1*(ds1db*dbdz + ds1dc*dcdz);
            (*dPdY)(2,widx) = dpzdc1*(dc1db*dbdw + dc1dc*dcdw) +
                              dpzds1*(ds1db*dbdw + ds1dc*dcdw);
        }

        // If this is the root, pull out non-yaw Euler angles.
        angles.col(0) = (2.0*(data.col(widx).cwiseProduct(data.col(zidx)) + data.col(yidx).cwiseProduct(data.col(xidx))).cwiseQuotient(norm)).array().asin();
        for (int s = 0; s < data.rows(); s++)
            angles(s,1) = atan2(2.0*(data(s,widx)*data(s,xidx) - data(s,yidx)*data(s,zidx))/norm(s),(pow(data(s,widx),2) - pow(data(s,xidx),2) + pow(data(s,yidx),2) - pow(data(s,zidx),2))/norm(s));
        // Apply roll about X axis.
        ce = (angles.col(1)).array().cos();
        se = (angles.col(1)).array().sin();
        out.col(1) = oldPos.col(1).cwiseProduct(ce) - oldPos.col(2).cwiseProduct(se);
        out.col(2) = oldPos.col(1).cwiseProduct(se) + oldPos.col(2).cwiseProduct(ce);
        // Apply pitch about Z axis.
        ce = (angles.col(0)).array().cos();
        se = (angles.col(0)).array().sin();
        oldPos = out;
        out.col(0) = oldPos.col(0).cwiseProduct(ce) - oldPos.col(1).cwiseProduct(se);
        out.col(1) = oldPos.col(0).cwiseProduct(se) + oldPos.col(1).cwiseProduct(ce);
    }
    else
    {
        // Rotate vector by quaternion.
        out.col(0) = (oldPos.col(0).cwiseProduct(norm - 2.0*(data.col(yidx).array().square().matrix()) - 2.0*(data.col(zidx).array().square().matrix())) +
                      oldPos.col(1).cwiseProduct(2.0*data.col(xidx).cwiseProduct(data.col(yidx)) - 2.0*data.col(zidx).cwiseProduct(data.col(widx))) +
                      oldPos.col(2).cwiseProduct(2.0*data.col(xidx).cwiseProduct(data.col(zidx)) + 2.0*data.col(yidx).cwiseProduct(data.col(widx)))).cwiseQuotient(norm);
        out.col(1) = (oldPos.col(0).cwiseProduct(2.0*data.col(xidx).cwiseProduct(data.col(yidx)) + 2.0*data.col(zidx).cwiseProduct(data.col(widx))) +
                      oldPos.col(1).cwiseProduct(norm - 2.0*(data.col(xidx).array().square().matrix()) - 2.0*(data.col(zidx).array().square().matrix())) +
                      oldPos.col(2).cwiseProduct(2.0*data.col(yidx).cwiseProduct(data.col(zidx)) - 2.0*data.col(xidx).cwiseProduct(data.col(widx)))).cwiseQuotient(norm);
        out.col(2) = (oldPos.col(0).cwiseProduct(2.0*data.col(xidx).cwiseProduct(data.col(zidx)) - 2.0*data.col(yidx).cwiseProduct(data.col(widx))) +
                      oldPos.col(1).cwiseProduct(2.0*data.col(yidx).cwiseProduct(data.col(zidx)) + 2.0*data.col(xidx).cwiseProduct(data.col(widx))) +
                      oldPos.col(2).cwiseProduct(norm - 2.0*(data.col(xidx).array().square().matrix()) - 2.0*(data.col(yidx).array().square().matrix()))).cwiseQuotient(norm);

        // Now compute the Jacobian.
        if (dPdY)
        {
            (*dPdY)(0,xidx) = (-2.0/norm(0,0))*data(0,xidx)*out(0,0) + (2.0/norm(0,0))*( data(0,xidx)*oldPos(0,0) + data(0,yidx)*oldPos(0,1) + data(0,zidx)*oldPos(0,2));
            (*dPdY)(0,yidx) = (-2.0/norm(0,0))*data(0,yidx)*out(0,0) + (2.0/norm(0,0))*(-data(0,yidx)*oldPos(0,0) + data(0,xidx)*oldPos(0,1) + data(0,widx)*oldPos(0,2));
            (*dPdY)(0,zidx) = (-2.0/norm(0,0))*data(0,zidx)*out(0,0) + (2.0/norm(0,0))*(-data(0,zidx)*oldPos(0,0) - data(0,widx)*oldPos(0,1) + data(0,xidx)*oldPos(0,2));
            (*dPdY)(0,widx) = (-2.0/norm(0,0))*data(0,widx)*out(0,0) + (2.0/norm(0,0))*( data(0,widx)*oldPos(0,0) - data(0,zidx)*oldPos(0,1) + data(0,yidx)*oldPos(0,2));

            (*dPdY)(1,xidx) = (-2.0/norm(0,0))*data(0,xidx)*out(0,1) + (2.0/norm(0,0))*( data(0,yidx)*oldPos(0,0) - data(0,xidx)*oldPos(0,1) - data(0,widx)*oldPos(0,2));
            (*dPdY)(1,yidx) = (-2.0/norm(0,0))*data(0,yidx)*out(0,1) + (2.0/norm(0,0))*( data(0,xidx)*oldPos(0,0) + data(0,yidx)*oldPos(0,1) + data(0,zidx)*oldPos(0,2));
            (*dPdY)(1,zidx) = (-2.0/norm(0,0))*data(0,zidx)*out(0,1) + (2.0/norm(0,0))*( data(0,widx)*oldPos(0,0) - data(0,zidx)*oldPos(0,1) + data(0,yidx)*oldPos(0,2));
            (*dPdY)(1,widx) = (-2.0/norm(0,0))*data(0,widx)*out(0,1) + (2.0/norm(0,0))*( data(0,zidx)*oldPos(0,0) + data(0,widx)*oldPos(0,1) - data(0,xidx)*oldPos(0,2));

            (*dPdY)(2,xidx) = (-2.0/norm(0,0))*data(0,xidx)*out(0,2) + (2.0/norm(0,0))*( data(0,zidx)*oldPos(0,0) + data(0,widx)*oldPos(0,1) - data(0,xidx)*oldPos(0,2));
            (*dPdY)(2,yidx) = (-2.0/norm(0,0))*data(0,yidx)*out(0,2) + (2.0/norm(0,0))*(-data(0,widx)*oldPos(0,0) + data(0,zidx)*oldPos(0,1) - data(0,yidx)*oldPos(0,2));
            (*dPdY)(2,zidx) = (-2.0/norm(0,0))*data(0,zidx)*out(0,2) + (2.0/norm(0,0))*( data(0,xidx)*oldPos(0,0) + data(0,yidx)*oldPos(0,1) + data(0,zidx)*oldPos(0,2));
            (*dPdY)(2,widx) = (-2.0/norm(0,0))*data(0,widx)*out(0,2) + (2.0/norm(0,0))*(-data(0,yidx)*oldPos(0,0) + data(0,xidx)*oldPos(0,1) + data(0,widx)*oldPos(0,2));
        }
    }
}

// Compute rotation matrix for joint.
void GPCMSupplementaryBVH::rotationMatrixFromJoint(
    VectorXd &norm,                         // Temporary normalization storage.
    const MatrixXd &data,                   // Input poses.
    int i,                                  // Joint.
    int jntIdx,                             // First angle index.
    Matrix3d &rot                           // Rotation matrix to return.
    )
{
    assert(data.rows() == 1); // This method only supports single row data for now.

    // Compute indices.
    int xidx = jntIdx+0;
    int yidx = jntIdx+1;
    int zidx = jntIdx+2;
    int widx = jntIdx+3;

    // Compute quaternion normalization.
    norm = data.block(0,xidx,data.rows(),4).rowwise().squaredNorm();

    if (i == 0)
    { // Special handling for root.
        // Compute u, v, and w.
        double a = (2.0/norm(0,0))*(data(0,widx)*data(0,zidx) + data(0,yidx)*data(0,xidx));
        double b = (2.0/norm(0,0))*(data(0,widx)*data(0,xidx) - data(0,yidx)*data(0,zidx));
        double c = (1.0/norm(0,0))*(pow(data(0,widx),2) - pow(data(0,xidx),2) + pow(data(0,yidx),2) - pow(data(0,zidx),2));

        // Compute sines and cosines.
        double c0 = sqrt(1.0-pow(a,2));
        double s0 = a;
        double c1 = c/sqrt(pow(b,2)+pow(c,2));
        double s1 = b/sqrt(pow(b,2)+pow(c,2));

        // The first matrix leaves x alone.
        Matrix3d r1;
        r1(0,0) = 1.0;
        r1(1,0) = 0.0;
        r1(2,0) = 0.0;
        r1(0,1) = 0.0;
        r1(1,1) = c1;
        r1(2,1) = s1;
        r1(0,2) = 0.0;
        r1(1,2) = -s1;
        r1(2,2) = c1;

        // The second matrix leaves z alone.
        Matrix3d r0;
        r0(0,0) = c0;
        r0(1,0) = s0;
        r0(2,0) = 0.0;
        r0(0,1) = -s0;
        r0(1,1) = c0;
        r0(2,1) = 0.0;
        r0(0,2) = 0.0;
        r0(1,2) = 0.0;
        r0(2,2) = 1.0;

        // Compute result.
        rot = r0*r1;
    }
    else
    { // Simply convert the quaternion to a matrix.
        rot(0,0) = ( pow(data(0,xidx),2) - pow(data(0,yidx),2) - pow(data(0,zidx),2) + pow(data(0,widx),2))/norm(0,0);
        rot(1,0) = (2.0*data(0,xidx)*data(0,yidx) + 2.0*data(0,zidx)*data(0,widx))/norm(0,0);
        rot(2,0) = (2.0*data(0,xidx)*data(0,zidx) - 2.0*data(0,yidx)*data(0,widx))/norm(0,0);
        rot(0,1) = (2.0*data(0,xidx)*data(0,yidx) - 2.0*data(0,zidx)*data(0,widx))/norm(0,0);
        rot(1,1) = (-pow(data(0,xidx),2) + pow(data(0,yidx),2) - pow(data(0,zidx),2) + pow(data(0,widx),2))/norm(0,0);
        rot(2,1) = (2.0*data(0,yidx)*data(0,zidx) + 2.0*data(0,xidx)*data(0,widx))/norm(0,0);
        rot(0,2) = (2.0*data(0,xidx)*data(0,zidx) + 2.0*data(0,yidx)*data(0,widx))/norm(0,0);
        rot(1,2) = (2.0*data(0,yidx)*data(0,zidx) - 2.0*data(0,xidx)*data(0,widx))/norm(0,0);
        rot(2,2) = (-pow(data(0,xidx),2) - pow(data(0,yidx),2) + pow(data(0,zidx),2) + pow(data(0,widx),2))/norm(0,0);
    }
}

// Helper function for determining joint global position.
void GPCMSupplementaryBVH::getJointGlobalHelper(
    MatrixXd &out,                          // Output matrix for storing global positions.
    MatrixXd &oldPos,                       // Temporary storage for previous position.
    MatrixXd &angles,                       // Temporary angles storage.
    VectorXd &ce,                           // Temporary cosine storage.
    VectorXd &se,                           // Temporary sine storage.
    VectorXd &norm,                         // Temporary normalization storage.
    const MatrixXd &data,                   // Input poses.
    int i,                                  // Desired joint index.
    bool bUseRoot,                          // Whether to use the root position and rotation.
    bool bUseRelative,                      // Root is relative.
    MatrixXd *dPdY,                         // Jacobian of joint with respect to Y.
    Matrix3d &rotParent,                    // Storage for parent's rotation matrix.
    Matrix3d &rot                           // Storage for this joint's rotation matrix.
    )
{
    // Compute position.
    const int *order = skeleton.joints[i].getOrder();
    const int *rotInd = skeleton.joints[i].getRotInd();
    if (rotInd)
    { // Rotate by the rotation of this joint.
        // Compute first index for this joint's rotation.
        int jntIdx = ((rotInd[order[0]]-3)/3)*4 + 4;

        // Rotate position by this joint's rotation and compute joint space Jacobian.
        rotateByJoint(out,oldPos,angles,ce,se,norm,data,i,jntIdx,dPdY);

        if (i == 0 && bUseRoot)
        { // For the root joint, apply its rotation about the vertical axis.
            //assert(!dPdY);
            oldPos = out;
            if (bUseRelative)
            {
                out.col(0) = oldPos.col(0).cwiseProduct((data.col(3)*(frameTime*PI/180.0)).array().cos().matrix()) +
                             oldPos.col(2).cwiseProduct((data.col(3)*(frameTime*PI/180.0)).array().sin().matrix());
                out.col(2) = -oldPos.col(0).cwiseProduct((data.col(3)*(frameTime*PI/180.0)).array().sin().matrix()) +
                              oldPos.col(2).cwiseProduct((data.col(3)*(frameTime*PI/180.0)).array().cos().matrix());
            }
            else
            {
                out.col(0) = oldPos.col(0).cwiseProduct(DEG_TO_RAD(data.col(3)).array().cos().matrix()) +
                             oldPos.col(2).cwiseProduct(DEG_TO_RAD(data.col(3)).array().sin().matrix());
                out.col(2) = -oldPos.col(0).cwiseProduct(DEG_TO_RAD(data.col(3)).array().sin().matrix()) +
                              oldPos.col(2).cwiseProduct(DEG_TO_RAD(data.col(3)).array().cos().matrix());
            }
        }

        // Add the offset of this joint.
        out += skeleton.joints[i].getOffset().transpose().replicate(out.rows(),1);
        
        
        // Check if this is the root and optionally add its position.
        if (i == 0)
        {
            
            oldPos = out;
            if (bUseRoot)
            {
                //assert(!dPdY);
                if (bUseRelative)
                {
                    out.col(1) += data.col(1);
                    out.col(0) += ((data.col(3)*(frameTime*PI/180.0)).array().cos().matrix().cwiseProduct(data.col(0)) +
                               (data.col(3)*(frameTime*PI/180.0)).array().sin().matrix().cwiseProduct(data.col(2)))*frameTime;
                    out.col(2) += ((data.col(3)*(frameTime*PI/180.0)).array().cos().matrix().cwiseProduct(data.col(2)) -
                               (data.col(3)*(frameTime*PI/180.0)).array().sin().matrix().cwiseProduct(data.col(0)))*frameTime;
                }
                else
                {
                    out += data.block(0,0,data.rows(),3);
                }
            }

            // Compute rotation matrix if necessary.
            if (dPdY)
            {
                // Compute rotation without yaw.
                rotationMatrixFromJoint(norm,data,i,jntIdx,rot);

                // Make sure we don't want yaw.
                //assert(!bUseRoot);
                
                if (bUseRoot)
                {
                    // Compute root rotation.
                    Matrix3d rotParent = Matrix3d::Identity();
                    rotParent(0,0) = cos(DEG_TO_RAD(frameTime*data(0,3)));
                    rotParent(0,2) = sin(DEG_TO_RAD(frameTime*data(0,3)));
                    rotParent(2,0) = -sin(DEG_TO_RAD(frameTime*data(0,3)));
                    rotParent(2,2) = cos(DEG_TO_RAD(frameTime*data(0,3)));
                    
                    // Compute global rotation.
                    rot = rotParent*rot;
                    
                    // Rotate the Jacobian by the parent.
                    dPdY->block(0,jntIdx,3,4) = rotParent*dPdY->block(0,jntIdx,3,4);
                    
                    // Compute gradient of root terms.
                    dPdY->col(0) = frameTime*rotParent.col(0);
                    dPdY->col(1) = rotParent.col(1);
                    dPdY->col(2) = frameTime*rotParent.col(2);
                    
                    (*dPdY)(0,3) = DEG_TO_RAD(-frameTime*frameTime*sin(DEG_TO_RAD(frameTime*data(0,3)))*data(0,0) 
                                              + frameTime*frameTime*cos(DEG_TO_RAD(frameTime*data(0,3)))*data(0,2)
                                              + frameTime*oldPos(0,2));
                    (*dPdY)(1,3) = 0.;
                    (*dPdY)(2,3) = DEG_TO_RAD(-frameTime*frameTime*cos(DEG_TO_RAD(frameTime*data(0,3)))*data(0,0) 
                                              -frameTime*frameTime*sin(DEG_TO_RAD(frameTime*data(0,3)))*data(0,2)
                                              - frameTime*oldPos(0,0));
                }
            }
        }
        else
        {
            // If this is not the root, recurse.
            getJointGlobalHelper(out,oldPos,angles,ce,se,norm,data,skeleton.joints[i].getParent(),
                bUseRoot,bUseRelative,dPdY,rotParent,rot);

            // Rotate Jacobian if necessary.
            if (dPdY)
            {
                // Store parent rotation.
                rotParent = rot;

                // Compute rotation of this joint.
                rotationMatrixFromJoint(norm,data,i,jntIdx,rot);

                // Compute global rotation.
                rot = rotParent*rot;

                // Rotate the Jacobian by the parent.
                dPdY->block(0,jntIdx,3,4) = rotParent*dPdY->block(0,jntIdx,3,4);
            }
        }
    }
    else
    { // If this joint has no rotation indices, recurse immediately.
        getJointGlobalHelper(out,oldPos,angles,ce,se,norm,data,skeleton.joints[i].getParent(),
            bUseRoot,bUseRelative,dPdY,rotParent,rot);
    }
}

// Get global position for a joint.
void GPCMSupplementaryBVH::getJointGlobal(
    MatrixXd &out,                          // Output matrix for storing positions.
    const MatrixXd &data,                   // Input poses.
    int joint,                              // Desired joint index.
    bool bUseRoot,                          // Whether to use the root position and rotation.
    bool bUseRelative,                      // Root is relative.
    MatrixXd *dPdY                          // Jacobian of joint with respect to Y.
    )
{
    // Initialize positions using transpose.
    MatrixXd oldPos(out.rows(),3);
    VectorXd norm(out.rows()); // Vector used for normalizing quaternions.
    out = skeleton.joints[joint].getOffset().transpose().replicate(out.rows(),1);

    // Resize Jacobian.
    if (dPdY)
    {
        assert(data.rows() == 1); // Jacobian mode only supports a single row for now.
        //assert(!bUseRoot);
        dPdY->resize(3,data.cols());
        dPdY->setZero(3,data.cols());
    }

    // Check if this is the root.
    if (joint == 0)
    {
        // Simply add the position.
        if (bUseRoot)
        {
            if (bUseRelative)
            {
                out.col(1) += data.col(1);
                out.col(0) += ((data.col(3)*(frameTime*PI/180.0)).array().cos().matrix().cwiseProduct(data.col(0)) +
                               (data.col(3)*(frameTime*PI/180.0)).array().sin().matrix().cwiseProduct(data.col(2)))*frameTime;
                out.col(2) += ((data.col(3)*(frameTime*PI/180.0)).array().cos().matrix().cwiseProduct(data.col(2)) -
                               (data.col(3)*(frameTime*PI/180.0)).array().sin().matrix().cwiseProduct(data.col(0)))*frameTime;
            }
            else
            {
                out += data.block(0,0,data.rows(),3);
            }
        }
    }
    else
    {
        // Allocate temporaries.
        MatrixXd angles(data.rows(),2);
        VectorXd ce(data.rows());
        VectorXd se(data.rows());
        Matrix3d rotParent;
        Matrix3d rot;

        getJointGlobalHelper(out,oldPos,angles,ce,se,norm,data,skeleton.joints[joint].getParent(),
            bUseRoot,bUseRelative,dPdY,rotParent,rot);
    }
}



// Transform the new pose to have the correct global root
void GPCMSupplementaryBVH::makeRootAbsolute(const MatrixXd &oldPose, MatrixXd *newPose)
{
    MatrixXd absRoot = newPose->block(0,0,oldPose.rows(),4);

    newPose->col(3) = oldPose.col(3) + frameTime * newPose->col(3);
    
    // Clamping angles
    for (int i = 0; i < newPose->rows(); i++)
    {   
        newPose->col(3)(i) =  modDeg(newPose->col(3)(i));
    }
    
    absRoot.col(0) = oldPose.col(0) + frameTime * ( newPose->col(0).cwiseProduct(DEG_TO_RAD(newPose->col(3)).array().cos().matrix()) 
                                                   + newPose->col(2).cwiseProduct(DEG_TO_RAD(newPose->col(3)).array().sin().matrix()) );
    absRoot.col(2) = oldPose.col(2) + frameTime * ( newPose->col(2).cwiseProduct(DEG_TO_RAD(newPose->col(3)).array().cos().matrix()) 
                                                   - newPose->col(0).cwiseProduct(DEG_TO_RAD(newPose->col(3)).array().sin().matrix()) );
    
    newPose->col(0) = absRoot.col(0);
    newPose->col(2) = absRoot.col(2);
}



// Write supplementary data.
void GPCMSupplementaryBVH::write(
    GPCMMatWriter *writer                   // Stream to write supplementary data to.
    )
{
    // Let the parent write first.
    GPCMSupplementaryData::write(writer);

    // Write type of rotation parameterization.
    writer->writeString(rotationParam,"rotation_type");

    // Write the number of configuration channels.
    writer->writeDouble(this->configChannels,"config_channels");

    // Write the frame length.
    writer->writeDouble(this->frameTime,"frameLength");

    // Create the skeleton struct.
    GPCMMatWriter *skelStruct = writer->writeStruct("skeleton",1,1);

    // Write constant terms.
    skelStruct->writeDouble(1,"mass");
    skelStruct->writeDouble(1,"length");
    skelStruct->writeString("bvh","type");
    skelStruct->writeString("deg","angle");
    skelStruct->writeString("","documentation");
    skelStruct->writeString("skeleton","name");

    // Gather the children of each joint.
    assert(skeleton.jointCount <= MAX_JOINTS);
    int childrenarr[MAX_JOINTS][MAX_JOINTS];
    int childcounts[MAX_JOINTS];
    memset(childcounts,0,sizeof(int)*skeleton.jointCount);
    for (int j = 0; j < skeleton.jointCount; j++)
    {
        int p = skeleton.joints[j].getParent();
        if (p >= 0)
        {
            childrenarr[p][childcounts[p]] = j+1;
            childcounts[p]++;
        }
    }

    // Create tree struct array.
    GPCMMatWriter *treeStruct = skelStruct->writeStruct("tree",1,skeleton.jointCount);
    for (int j = 0; j < skeleton.jointCount; j++)
    {
        // Create constant stuff.
        MatrixXd offset = skeleton.joints[j].getOffset().transpose();
        MatrixXd axis = MatrixXd::Zero(1,3);
        MatrixXd C = MatrixXd::Identity(3,3);
        MatrixXd Cinv = MatrixXd::Identity(3,3);

        // Create children.
        if (childcounts[j] > 0)
        {
            MatrixXd children(1,childcounts[j]);
            for (int k = 0; k < childcounts[j]; k++)
            {
                children(0,k) = childrenarr[j][k];
            }
            treeStruct->writeMatrix(children,"children");
        }
        else
        {
            treeStruct->writeBlank("children");
        }

        // Create rotation and position indices.
        MatrixXd rotIndMat = MatrixXd::Zero(1,1);
        const int *rotInd = skeleton.joints[j].getRotInd();
        if (rotInd)
        {
            rotIndMat.resize(1,3);
            rotIndMat << rotInd[0]+1,rotInd[1]+1,rotInd[2]+1;
        }
        MatrixXd posIndMat = MatrixXd::Zero(1,1);
        const int *posInd = skeleton.joints[j].getPosInd();
        if (posInd)
        {
            posIndMat.resize(1,3);
            posIndMat << posInd[0]+1,posInd[1]+1,posInd[2]+1;
        }

        // Write position channels.
        std::vector<std::string> channels;
        if (posInd)
        {
            channels.push_back("Xposition");
            channels.push_back("Yposition");
            channels.push_back("Zposition");
        }

        // Create order string and append rotation channels.
        std::string order("");
        const int *orderarr = skeleton.joints[j].getOrder();
        if (orderarr)
        {
            for (int k = 0; k < 3; k++)
            {
                switch (orderarr[k])
                {
                case 0:
                    order.append("x");
                    channels.push_back("Xrotation");
                    break;
                case 1:
                    order.append("y");
                    channels.push_back("Yrotation");
                    break;
                case 2:
                    order.append("z");
                    channels.push_back("Zrotation");
                    break;
                }
            }
        }

        // Write joint information.
        treeStruct->writeString(skeleton.joints[j].getName(),"name");
        treeStruct->writeDouble((double)j,"id");
        treeStruct->writeMatrix(offset,"offset");
        treeStruct->writeBlank("orientation");
        treeStruct->writeMatrix(axis,"axis");
        treeStruct->writeBlank("axisOrder");
        treeStruct->writeMatrix(C,"C");
        treeStruct->writeMatrix(Cinv,"Cinv");
        treeStruct->writeBlank("bodymass");
        treeStruct->writeBlank("confmass");
        treeStruct->writeDouble((double)(skeleton.joints[j].getParent()+1),"parent");
        treeStruct->writeString(order,"order");
        if (rotIndMat.cols() > 1)
            treeStruct->writeMatrix(rotIndMat,"rotInd");
        else
            treeStruct->writeBlank("rotInd");
        if (posIndMat.cols() > 1)
            treeStruct->writeMatrix(posIndMat,"posInd");
        else
            treeStruct->writeBlank("posInd");
        treeStruct->writeBlank("limits");

        // Write cell array of strings.
        GPCMMatWriter *channelCell = treeStruct->writeCell("channels",1,(int)channels.size());
        for (unsigned k = 0; k < channels.size(); k++)
        {
            channelCell->writeString(channels[k],"");
        }
        treeStruct->closeCell();

        // Advance to next element in struct array.
        treeStruct = skelStruct->closeStruct();
    }

    // Clean up.
    writer->closeStruct();
}

// Split apart the position and velocity portions of the data matrix.
void GPCMSupplementaryBVH::splitVelocity(
    MatrixXd &positions,                    // Output is position terms, input is data matrix.
    MatrixXd &velocities                    // Output is velocity terms.
    )
{
    // Determine where positions end and velocities start.
    // This excludes the root velocity terms.
    int firstVelocityIndex = 0;
    for (int j = 0; j < positions.cols(); j++)
    {
        if (variableIndices[j] >= configChannels)
        {
            firstVelocityIndex = j;
            break;
        }
    }

    // Now copy out the velocities.
    velocities.resize(positions.rows(),positions.cols()-firstVelocityIndex+3);
    // Copy over the root velocities.
    velocities.block(0,0,positions.rows(),1) = positions.block(0,0,positions.rows(),1);
    velocities.block(0,1,positions.rows(),1) = positions.block(0,2,positions.rows(),1);
    velocities.block(0,2,positions.rows(),1) = positions.block(0,3,positions.rows(),1);
    // Copy over remaining velocity terms.
    for (int j = firstVelocityIndex; j < positions.cols(); j++)
        velocities.block(0,j-firstVelocityIndex+3,positions.rows(),1) = positions.block(0,j,positions.rows(),1);

    // Create new positions matrix.
    MatrixXd newPositions(positions.rows(),firstVelocityIndex-3);
    // Copy over height.
    newPositions.block(0,0,positions.rows(),1) = positions.block(0,1,positions.rows(),1);
    // Copy over joint positions.
    for (int j = 4; j < firstVelocityIndex; j++)
        newPositions.block(0,j-3,positions.rows(),1) = positions.block(0,j,positions.rows(),1);

    // Swap out matrices to output positions.
    positions = newPositions;

    // Fill in reconstruction vectors.
    isVelocity[0] = 1;
    isVelocity[2] = 1;
    isVelocity[3] = 1;
    for (unsigned i = firstVelocityIndex; i < isVelocity.size(); i++) isVelocity[i] = 1;
    fullToSplit[1] = 0;
    for (int i = 4; i < firstVelocityIndex; i++) fullToSplit[i] = i-3;
    fullToSplit[0] = 0;
    fullToSplit[2] = 1;
    fullToSplit[3] = 2;
    for (unsigned i = firstVelocityIndex; i < isVelocity.size(); i++) fullToSplit[i] = i-firstVelocityIndex+3; 
    
    
    
    // Set position and velocity indices
    positionIndices = getPositionIndicesInFullPose();
    velocityIndices = getVelocityIndicesInFullPose();
}

// Destructor.
GPCMSupplementaryBVH::~GPCMSupplementaryBVH()
{
    delete[] skeleton.joints;
}

// Get frame length.
double GPCMSupplementaryBVH::getFrameTime()
{
    return this->frameTime;
}

// Get number of configuration channels.
int GPCMSupplementaryBVH::getConfigChannels()
{
    return this->configChannels;
}


// Split apart the position and velocity portions of the input matrix
void GPCMSupplementaryBVH::splitMatrix(
    const MatrixXd &matrixIn,               // Input Matrix
    MatrixXd *positions,                    // Output is position terms.
    MatrixXd *velocities                    // Output is velocity terms.
    )
{
    if(positions)
    {
        positions->resize(matrixIn.rows(),positionIndices.size());
        for (unsigned int i = 0; i < positionIndices.size(); i++)
        {
            positions->col(i) = matrixIn.col(positionIndices[i]);
        }
    }
    if (velocities)
    {
        velocities->resize(matrixIn.rows(),velocityIndices.size());
        for (unsigned int i = 0; i < velocityIndices.size(); i++)
        {
            velocities->col(i) = matrixIn.col(velocityIndices[i]);
        }
    }
}
