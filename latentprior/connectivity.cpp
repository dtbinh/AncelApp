// Term that encourages connectivity.

#include "connectivity.h"
#include "model.h"
#include "matwriter.h"
#include "mathutils.h"
#include "debugprint.h"

#include <Eigen/Eigen>

// Constructor.
GPCMConnectivity::GPCMConnectivity(
    GPCMParams &params,                     // Parameters of these dynamics.
    GPCMOptions &options,                   // Loaded options.
    GPCMOptimization *optimization,         // Optimization object to add new variables to.
    MatrixXd &X,                            // Pointer to latent positions matrix.
    MatrixXd &Xgrad,                        // Pointer to latent gradients matrix.
    std::vector<int> &sequence,             // Pointer to sequence indices.
    GPCMTask *task,                         // Pointer to the task.
    GPCMController *controller              // Pointer to the controller object.
    ) : GPCMLatentPrior(params,options,optimization,X,Xgrad,sequence,task,controller)
{
    // Get width of norm.
    if (params["width"].empty()) width = 1.0;
    else width = atof(params["width"][0].c_str());

    // Get weight.
    if (params["weight"].empty()) weight = 1.0;
    else weight = atof(params["weight"][0].c_str());

    // Adjust weight to compensate for number of points.
    if (!params["scale_weight"].empty() && !params["scale_weight"][0].compare("true"))
        weight *= 250000.0/pow((double)X.rows(),2);

    // Get power.
	double genPower;
    if (params["power"].empty()) genPower = 1.0;
    else genPower = atof(params["power"][0].c_str());

    // Get beta.
    if (params["diffusion_beta"].empty()) beta = 1.0;
    else beta = atof(params["diffusion_beta"][0].c_str());

    // Adjust beta to compensate for number of points.
    if (!params["scale_beta"].empty() && !params["scale_beta"][0].compare("true"))
        beta *= ((double)X.rows())/400.0;

    // Get cost type.
    if (params["cost_type"].empty())
        costType = CostTypeGaussian;
    else if (!params["cost_type"][0].compare("gaussian"))
        costType = CostTypeGaussian;
    else if (!params["cost_type"][0].compare("inverse"))
        costType = CostTypeInverse;

    // Get objective type.
    if (params["objective_type"].empty())
        objectiveType = ObjectiveLog;
    else if (!params["objective_type"][0].compare("log"))
        objectiveType = ObjectiveLog;
    else if (!params["objective_type"][0].compare("entropy"))
        objectiveType = ObjectiveEntropy;

    // Get temporal neighbor power.
    double neighborPower = genPower;
    if (!params["neighbor_power"].empty())
		neighborPower = atof(params["neighbor_power"][0].c_str());

    // Get temporal neighbor factor.
    double neighborFactor;
    if (params["neighbor_bonus"].empty()) neighborFactor = 1.0;
    else neighborFactor = atof(params["neighbor_bonus"][0].c_str());

	// Create temporal neighbor factor and power matrix.
    temporalNeighborFactor.resize(X.rows(),X.rows());
    temporalNeighborFactor.setOnes(X.rows(),X.rows());
    power.resize(X.rows(),X.rows());
    power.setConstant(X.rows(),X.rows(),genPower);
    int start = 0;
    for (unsigned i = 0; i < sequence.size(); i++)
    {
        int end = sequence[i];

        // Set weight on all temporal neighbors.
        for (int t = start+1; t < end; t++)
        {
            temporalNeighborFactor(t,t-1) = neighborFactor;
            temporalNeighborFactor(t-1,t) = neighborFactor;
            power(t,t-1) = neighborPower;
            power(t-1,t) = neighborPower;
        }

        // Set new start.
        start = end;
    }

    // Decide if we should print stats.
    if (params["print_stats"].empty() || params["print_stats"][0].compare("true"))
        bPrintKernelInfo = false;
    else
        bPrintKernelInfo = true;

    // Set type.
    type = ValueTypeConnectivity;
}

// Recompute closed-form MAP estimates when doing alternating optimization.
void GPCMConnectivity::recomputeClosedForm()
{
    // Nothing to do.
}

// Recompute all stored temporaries when variables change.
double GPCMConnectivity::recompute(
    bool bNeedGradient                      // Whether we should recompute the gradients too.
    )
{

    double likelihood = 0.0;

    // Constants.
    int N = (int)X.rows();

    // Compute pairwise norms.
    dists = pairwiseDistance(X);

    // Compute costs.
    // Subtracting the identity ensures the diagonal has all 0s.
    switch (costType)
    {
    case CostTypeGaussian:
        costs = (-0.5*width*dists).array().exp().matrix() - MatrixXd::Identity(N,N);
        break;
    case CostTypeInverse:
		costs = dists + MatrixXd::Identity(N,N);
		for (int i = 0; i < costs.rows(); i++)
		{
			for (int j = 0; j < costs.cols(); j++)
			{
				costs(i,j) = 1.0/pow(costs(i,j),power(i,j));
			}
		}
        costs -= MatrixXd::Identity(N,N);
        break;
    }

    // Multiply costs by temporal neighbor factor.
    costs = costs.cwiseProduct(temporalNeighborFactor);

    // Build negative Laplacian.
    D = costs.rowwise().sum();
    H = costs;
    H -= D.asDiagonal();
    invD = D.array().inverse();
    invSqrtD = invD.array().sqrt();
    H = invSqrtD.asDiagonal()*H*invSqrtD.asDiagonal();

    // Use eigendecomposition to compute matrix exponential.
    SelfAdjointEigenSolver<MatrixXd> eigensolve(H);
    Lambda = eigensolve.eigenvalues().real();
    U = eigensolve.eigenvectors().real();

    // Exponentiate the eigenvalues.
    betaLambdaExp = (beta*Lambda).array().exp().matrix();

    // Compute kernel matrix.
    K = U*(betaLambdaExp.asDiagonal())*U.transpose();

    // Avoid degenerate entries in the kernel matrix.
    //K = K.cwiseMax(MatrixXd::Constant(N,N,1.0e-128));
    K = K.cwiseMax(MatrixXd::Constant(N,N,1.0e-16));

    // Compute objective as the entropy of the graph kernel.
    switch (objectiveType)
    {
    case ObjectiveLog:
        likelihood = weight*(K.array().log()).sum();
        break;
    case ObjectiveEntropy:
        likelihood = -weight*(K.array() * K.array().log()).sum();
        break;
    }

    // Compute derivatives.
    if (bNeedGradient)
    {
        // Compute derivative with respect to K.
        switch (objectiveType)
        {
        case ObjectiveLog:
            dLdK = weight*(K.array().inverse().matrix());
            break;
        case ObjectiveEntropy:
            dLdK = (-weight)*(K.array().log().matrix() + MatrixXd::Ones(N,N));
            break;
        }

        // Compute difference of eigenvalue exponentials, difference of eigenvalues denominator, and divide to get Q.
        Q = betaLambdaExp.replicate(1,N) - betaLambdaExp.transpose().replicate(N,1);
        temp = Lambda.replicate(1,N) - Lambda.transpose().replicate(N,1) + MatrixXd::Identity(N,N);
        clampAbsoluteValue(temp,1.0e-128); // This ensures that the absolute value of each denominator entry is not too low.
        Q = Q.cwiseQuotient(temp);
        Q += (beta*betaLambdaExp).asDiagonal(); // This places the correct values on the diagonal.
        
        // Compute intermediate matrix M.
        M = U*((U.transpose() * dLdK * U).cwiseProduct(Q))*U.transpose();

        // Now compute factor in front of dD/dX and dC/dX.
        MC = invSqrtD.asDiagonal()*M*invSqrtD.asDiagonal();
        MD = -MC;
        MD -= 0.5*(invD.asDiagonal()*(H*M));
        MD -= 0.5*(M*H)*invD.asDiagonal();

        // Compute gradient of costs (without the difference of Xs in front).
        switch (costType)
        {
        case CostTypeGaussian:
            dCdX = width*costs;
            break;
        case CostTypeInverse:
			dCdX = dists + MatrixXd::Identity(N,N);
			for (int i = 0; i < costs.rows(); i++)
			{
				for (int j = 0; j < costs.cols(); j++)
				{
					dCdX(i,j) = (2.0*power(i,j))/pow(dCdX(i,j),power(i,j)+1.0);
				}
			}
            dCdX -= (2.0*power(0,0))*MatrixXd::Identity(N,N);
            break;
        }

        // Multiply by MC and MD matrices.
        temp = (MC + MC.transpose() + MD.diagonal().replicate(1,N) + MD.diagonal().transpose().replicate(N,1)).cwiseProduct(dCdX);

        // Multiply by temporal neighbor factor.
        temp = temp.cwiseProduct(temporalNeighborFactor);

        for (int k = 0; k < X.cols(); k++)
        {
            // Now just compute the column-wise gradients.
            Xgrad.col(k) += temp.cwiseProduct(X.col(k).transpose().replicate(X.rows(),1) - X.col(k).replicate(1,X.rows())).rowwise().sum();
        }
    }

    // If necessary, print out statistics about the graph kernel.
    if (bPrintKernelInfo)
    {
        // Measure average probability of paths to temporal neighbors.
        double tmWt = 0.0;
        int tmCnt = 0;
        int start = 0;
        for (unsigned i = 0; i < sequence.size(); i++)
        {
            int end = sequence[i];
            for (int t = start; t < end-1; t++)
            {
                tmCnt++;
                tmWt += K(t,t+1);
            }
            start = end+1;
        }
        tmWt /= tmCnt;

        // Measure average probability of paths to self.
        double selfWt = K.diagonal().sum()/N;

        // Measure average probability to furthest points.
        double furthestWt = 0.0;
        for (int i = 0; i < N; i++)
        {
            int idx;
            int h;
            dists.col(i).maxCoeff(&idx,&h);
            assert(h == 0);
            furthestWt += K(i,idx)/N;
        }

        // Print results.
        DBPRINTLN("Kernel stats: neighbor = " << tmWt << "; self = " << selfWt << "; furthest = " << furthestWt << ";");
    }

    // Return likelihood.
    return likelihood;
}

// Write GP data to file.
void GPCMConnectivity::write(
    GPCMMatWriter *writer                   // Writing interface.
    )
{
    writer->writeString("connectivity","type");
}

// Destructor.
GPCMConnectivity::~GPCMConnectivity()
{
}
