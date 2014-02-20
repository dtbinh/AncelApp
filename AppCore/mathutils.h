// General math utilities.
#pragma once

#ifdef _WIN32
// This disables an annoying warning due to a compiler bug.
#pragma warning (disable: 4985)
#endif

#include <limits>

#include <float.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace Eigen;

// Large distance.
#define LARGE_DISTANCE  FLT_MAX

// Moderate angle second order variance.
#define SMALL_ANGLE_ACC_VAR 10.0

// Small angle (in degrees).
#define EPS_ANGLE   1e-6

// Small Z value.
#define EPS_Z       1.0e-10

// Definition of PI.
#define PI 3.14159265358979323846264338327

// Convert radians to degrees.
#define RAD_TO_DEG(x) ((x) * (180.0 / PI))

// Convert degrees to radians.
#define DEG_TO_RAD(x) ((x) * (PI / 180.0))

// Clamp the absolute value of each entry.
void clampAbsoluteValue(MatrixXd &M, double val);

// Draw sample from multidimensional Gaussian with diagonal covariance.
void sampleGaussian(const VectorXd &cov, MatrixXd &samples);

// Return normal random double.
double randomNormal();

// Return uniform random double in [0,1].
double randomDouble();

// Convert Euler angles to exponential map.
Vector3d eulerToExp(const Vector3d &euler, const int *order);

// Convert Euler angles to quaternion.
Quaterniond eulerToQuat(const Vector3d &euler, const int *order);

// Convert Euler angles between two different orders.
Vector3d convertEuler(const Vector3d &euler, const int *oldorder, const int *neworder);

// Convert quaternion to exponential map.
Vector3d quatToExp(const Quaterniond &quat);

// Convert exponential map to quaternion.
Quaterniond expToQuat(const Vector3d &exp);

// Compute pairwise distances from a data matrix.
MatrixXd pairwiseDistance(const MatrixXd &X);

// Compute pairwise distances from a pair of data matrices.
MatrixXd pairwiseDistance(const MatrixXd &X1, const MatrixXd &X2);

// Compute pairwise distances from a pair of data matrices and write to specified output.
void pairwiseDistance(const MatrixXd &X1, const MatrixXd &X2, MatrixXd &out, int row = 0, int col = 0);

// Double mod.
static inline double modDouble(
    double a,
    double base
    )
{
    return a < 0.0 ? fmod(fmod(a,base)+base,base) : fmod(a,base);
}

// Normalize real into [min,max] range.
static inline double modPositive(
    double val,                             // Value to normalize.
    double min,                             // Minimum.
    double max                              // Maximum.
    )
{
    if (val >= min && val < max)
        return val;
    else
        return modDouble(val-min,max-min)+min;
}

// Normalize angle into [-PI,PI] range.
static inline double modAngle(
    double angle                             // Angle to normalize.
    )
{
    return modPositive(angle,-PI,PI);
}

// Normalize angle into [-180,180] range.
static inline double modDeg(
    double angle                             // Angle to normalize.
    )
{
    return modPositive(angle,-180.0,180.0);
}
