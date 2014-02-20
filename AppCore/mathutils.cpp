// General math utilities.

#include "mathutils.h"

// Clamp the absolute value of each entry.
void clampAbsoluteValue(
    MatrixXd &M,                            // Matrix to clamp.
    double val                                // Value to clamp to.
    )
{
    for (int i = 0; i < M.rows(); i++)
    {
        for (int j = 0; j < M.cols(); j++)
        {
            if (M(i,j) < val && M(i,j) > -val)
            {
                if (M(i,j) < 0.0) M(i,j) = val;
                else M(i,j) = -val;
            }
        }
    }
}

// Draw sample from multidimensional Gaussian with diagonal covariance.
void sampleGaussian(
    const VectorXd &cov,                    // Diagonal covariance matrix.
    MatrixXd &samples                       // Samples matrix to fill.
    )
{
    // Generate random normals for each dimension and scale by covariance.
    for (int i = 0; i < samples.cols(); i++)
    {
        double var = sqrt(cov(i));
        for (int j = 0; j < samples.rows(); j++)
        {
            samples(j,i) = randomNormal()*var;
        }
    }
}

// Return normal random double.
double randomNormal()
{
    // Use Box-Muller transform to generate random normally distributed number.
    return sqrt(-2.0*log(randomDouble()))*cos(2.0*PI*randomDouble());
}

// Return uniform random double in (0,1).
double randomDouble()
{
    return ((double)(rand()+1))/((double)(RAND_MAX+2));
}

// Convert exponential map to quaternion.
Quaterniond expToQuat(
    const Vector3d &exp                     // Exponential map.
    )
{
    double norm = exp.norm();
    Quaterniond quat;
    if (abs(norm) < 1.0e-16)
        quat = AngleAxisd(0.0, Vector3d(1,0,0));
    else
        quat = AngleAxisd(DEG_TO_RAD(norm), exp/norm);
    return quat;
}

// Convert Euler angles to exponential map.
Vector3d eulerToExp(
    const Vector3d &euler,                  // Input Euler angles.
    const int *order                        // Order of Euler angles.
    )
{
    AngleAxisd rot = AngleAxisd(eulerToQuat(euler,order));
    return rot.axis()*RAD_TO_DEG(modAngle(rot.angle()));
}

// Convert Euler angles to quaternion.
Quaterniond eulerToQuat(
    const Vector3d &euler,                  // Input Euler angles.
    const int *order                        // Order of Euler angles.
    )
{
    Quaterniond quat = AngleAxisd(DEG_TO_RAD(euler(0)), Vector3d::Unit(order[0]))
                     * AngleAxisd(DEG_TO_RAD(euler(1)), Vector3d::Unit(order[1]))
                     * AngleAxisd(DEG_TO_RAD(euler(2)), Vector3d::Unit(order[2]));
    return quat;
}

// Convert Euler angles between two different orders.
Vector3d convertEuler(
    const Vector3d &euler,                  // Input Euler angles.
    const int *oldorder,                    // Old order of Euler angles.
    const int *neworder                     // New order of Euler angles.
    )
{
    // Return resulting Euler angles in degrees.
    return RAD_TO_DEG(eulerToQuat(euler,oldorder).toRotationMatrix().
        eulerAngles(neworder[0],neworder[1],neworder[2]));
}

// Convert quaternion to exponential map.
Vector3d quatToExp(
    const Quaterniond &quat                 // Input quaternion.
    )
{
    AngleAxisd aa(quat);
    return RAD_TO_DEG(aa.axis()*aa.angle());
}

// Compute pairwise distances from a data matrix.
MatrixXd pairwiseDistance(
    const MatrixXd &X                       // Data matrix.
    )
{
    MatrixXd sq = X.rowwise().squaredNorm()*MatrixXd::Ones(1,X.rows());
    return sq + sq.transpose() - X*X.transpose()*2.0;
}

// Compute pairwise distances from a pair of data matrices.
MatrixXd pairwiseDistance(
    const MatrixXd &X1,                     // First data matrix.
    const MatrixXd &X2                      // Second data matrix.
    )
{
    int h1 = X1.rows();
    int h2 = X2.rows();
    return X1.rowwise().squaredNorm()*MatrixXd::Ones(1,h2) +
           MatrixXd::Ones(h1,1)*X2.rowwise().squaredNorm().transpose() -
           X1*X2.transpose()*2.0;
}

// Compute pairwise distances from a pair of data matrices.
void pairwiseDistance(
    const MatrixXd &X1,                     // First data matrix.
    const MatrixXd &X2,                     // Second data matrix.
    MatrixXd &out,                          // Where to write the result.
    int row,                                // Output row.
    int col                                 // Output col.
    )
{
    int h1 = X1.rows();
    int h2 = X2.rows();
    out.block(row,col,h1,h2) = X1.rowwise().squaredNorm()*MatrixXd::Ones(1,h2) +
           MatrixXd::Ones(h1,1)*X2.rowwise().squaredNorm().transpose() -
           X1*X2.transpose()*2.0;
}
