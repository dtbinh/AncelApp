// Embed with PPCA.

#include "debugprint.h"
#include "embedppca.h"

#include <vector>
#include <utility>

#include <Eigen/Eigen>

// Construct the embedding.
GPCMEmbedPPCA::GPCMEmbedPPCA(
    MatrixXd &X,                            // Embedded matrix.
    MatrixXd &Y                             // Design matrix.
    ) : GPCMEmbed(X,Y)
{
    // Make sure we have at least as many rows as columns.
    // If this is not the case, we need to implement the other version of PPCA.
    //assert(Y.rows() > Y.cols());

    // Run PCA.
    // Solve for eigenvalues.
    int q = X.cols();
    MatrixXd Ymean = Y.colwise().sum()/Y.rows();
    MatrixXd scale = ((Y - Ymean.replicate(Y.rows(),1)).colwise().squaredNorm()/Y.rows()).array().sqrt().inverse();
    MatrixXd Ycentered = (Y - Ymean.replicate(Y.rows(),1)).cwiseProduct(scale.replicate(Y.rows(),1));

    // If the desired dimensionality is equal to Y, just return Y.
    if (X.cols() == Y.cols())
    {
        X = Ycentered;
        return;
    }
    assert(X.cols() < Y.cols());

    // This version uses SVD.
    JacobiSVD<MatrixXd> svd(Ycentered.transpose(), ComputeThinU | ComputeThinV);
    VectorXd S = svd.singularValues();
    MatrixXd U = svd.matrixU();
    MatrixXd u(Y.cols(),q);
    MatrixXd D = MatrixXd::Zero(q,q);
    for (int i = 0; i < q; i++)
    {
        D(i,i) = 1/sqrt(pow(S(i),2)/Y.rows());
        u.block(0,i,Y.cols(),1) = U.block(0,i,Y.cols(),1);
    }

    /*
    // This version uses Eigendecomposition.
    // Otherwise, do PCA.
    MatrixXd covariance = Ycentered.transpose()*Ycentered/Y.rows();
    EigenSolver<MatrixXd> eigensolve(covariance);
    VectorXd v = eigensolve.eigenvalues().real();
    MatrixXd vectors = eigensolve.eigenvectors().real();

    // Sort the eigenvalues.
    std::vector<std::pair<double,int> > pairs;
    for (int i = 0 ; i < v.rows(); i++)
        pairs.push_back(std::make_pair(v(i),i));
    std::sort(pairs.begin(),pairs.end());

    // Construct corresponding sorted eigenvectors.
    MatrixXd u2(Y.cols(),q);
    MatrixXd D2 = MatrixXd::Zero(q,q);
    for (int i = 0; i < q; i++)
    {
        double val = pairs[pairs.size()-1-i].first;
        if (val <= 0.0)
            DBERROR("Negative eigenvalue in the desired dimensions!");
        D2(i,i) = 1/sqrt(val);
        u2.block(0,i,Y.cols(),1) = vectors.block(0,pairs[pairs.size()-1-i].second,Y.cols(),1);
    }
    */

    // Build the embedding.
    X = Ycentered*u*D;
}
