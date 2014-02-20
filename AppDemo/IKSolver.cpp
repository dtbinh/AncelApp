#include "IKSolver.h"
#include "BoneAccessor.h"
#include "AppUtility.h"
#include <limits>
#include <cassert>
#include <fstream>
using namespace AncelIK;

IKSolver::IKSolver()
{
}

IKSolver::~IKSolver()
{
	for(std::size_t i = 0; i < mChains.size(); i++)
	{
		delete mChains[i];
	}
}
//OK
bool IKSolver::initSlover(const AncelApp::Skeleton *skeleton)
{
 	mSkeleton = const_cast<AncelApp::Skeleton*>(skeleton);

	mTheta.resize(mSkeleton->getTotalFreedom());
  
 	return true;
}
void  IKSolver::addAllChain()
{
	std::vector<bone_type*>::const_iterator it = mSkeleton->begin();

	for (it; it != mSkeleton->end(); it++)
    {
		if(!(*it)->isLeaf())
          continue;

        AncelIK::IKChain* chain = new AncelIK::IKChain();
		chain->initChain(mSkeleton->getRoot(), *it); 
		mChains.push_back(chain);
    }
}
void  IKSolver::addChain(const std::string& root, const std::string leaf)
{
	 AncelIK::IKChain* chain = new AncelIK::IKChain();
	 chain->initChain(mSkeleton->getBone(root), mSkeleton->getBone(leaf)); 
	 mChains.push_back(chain);
}

void  IKSolver::removeChain(const std::size_t index)
{
	if(index < mChains.size() && index >= 0)
	{
		delete mChains[index];
		mChains.erase(mChains.begin() + index);
	}
}
//OK
void IKSolver::setEndEffectorGoal(std::size_t chainID, const Ogre::Vector3& pos, 
								const Ogre::Vector3& axisX, const Ogre::Vector3& axisY)
{
	assert(chainID < mChains.size());
	mChains[chainID]->goalPosition() = pos;

	if (!mChains[chainID]->onlyPosition())
	{
		mChains[chainID]->goalAxisX() = axisX;
		mChains[chainID]->goalAxisY() = axisY;
	}
}
//OK
void IKSolver::setEndEffectorGoal(std::string chainName, const Ogre::Vector3& pos, 
								const Ogre::Vector3& axisX,	const Ogre::Vector3& axisY)
{
 	for (std::size_t i = 0; i < mChains.size(); i++)
	{
		if (chainName == mChains[i]->name())
		{
			mChains[i]->goalPosition() = pos;
			if (!mChains[i]->onlyPosition())
			{
				mChains[i]->goalAxisX() = axisX;
				mChains[i]->goalAxisY() = axisY;
			}
			break;
		}
	}
}
//OK
IKSolver::real_vector& IKSolver::computeDifference() 
{
	std::size_t sizeDelta = 0;
	std::vector<IKChain*>::iterator it = mChains.begin();
  	for (it; it != mChains.end(); it++)
		sizeDelta += (*it)->getGoalDimension();
	
	if (mDelta.size() != sizeDelta)
		mDelta.resize(sizeDelta);

	// g - F(theta);
	std::size_t curIndex = 0;
	for (it = mChains.begin(); it != mChains.end(); it++)
	{ 
		Ogre::Vector3 delta_p = (*it)->goalPosition() - ((*it)->getEndEffector()->getGlobalOri() * (*it)->localPosition() + (*it)->getEndEffector()->getGlobalPos());
 
		mDelta[curIndex++] = delta_p.x;
		mDelta[curIndex++] = delta_p.y;
		mDelta[curIndex++] = delta_p.z;

		if ((*it)->getGoalDimension() == 9u)
		{
			Ogre::Vector3 delta_x = (*it)->goalPosition() - (*it)->getEndEffector()->getGlobalOri() * (*it)->localAxisX();
			mDelta[curIndex++] = delta_x.x;
			mDelta[curIndex++] = delta_x.y;
			mDelta[curIndex++] = delta_x.z;
 
			Ogre::Vector3 delta_y = (*it)->goalPosition() - (*it)->getEndEffector()->getGlobalOri() * (*it)->localAxisY();

			mDelta[curIndex++] = delta_y.x;
			mDelta[curIndex++] = delta_y.y;
			mDelta[curIndex++] = delta_y.z;
 		}
	}
	return mDelta;
}
//OK
IKSolver::real_vector IKSolver::computeWeightedDifference()  
{
	computeDifference();
	std::size_t curIndex = 0;
	IKSolver::real_vector delta = mDelta;

	std::vector<IKChain*>::iterator it = mChains.begin();
	for (it = mChains.begin(); it != mChains.end(); it++)
	{
 		delta[curIndex++] *= (*it)->weightPos();
		delta[curIndex++] *= (*it)->weightPos();
		delta[curIndex++] *= (*it)->weightPos();

		if ((*it)->getGoalDimension() == 9u)
		{
			delta[curIndex++] *= (*it)->weightAxisX();
			delta[curIndex++] *= (*it)->weightAxisX();
			delta[curIndex++] *= (*it)->weightAxisX();
  
			delta[curIndex++] *= (*it)->weightAxisY();
			delta[curIndex++] *= (*it)->weightAxisY();
			delta[curIndex++] *= (*it)->weightAxisY();
 		}
	}
	return delta;
}
 
bool IKSolver::stagnation(real_vector const & x_old,real_vector const & x, real_type const & tolerance)
{
    real_type max_dx = 0;

    for (size_t i = 0; i < x.size(); ++ i)
        max_dx = std::max( max_dx, std::fabs( x.at(i) - x_old.at(i) ) );

    assert(!_isnan( max_dx ));

    if(max_dx <= tolerance)
        return true;
    return false;
}

bool IKSolver::relative_convergence(real_type f_old, real_type f, real_type tolerance)
{
    if(fabs(f_old) <= 1e-6)
    {
        if(fabs(f) <= 1e-6)
			return true;
        return false;
    }
	real_type const relative_test = fabs(f - f_old) / fabs(f_old);
	assert(!_isnan(relative_test));
	
	if(relative_test <= tolerance)
       return true;

    return false;
}
//------------------------------OK----------------------------------------------------------
void IKSolver::computeJacobian()
{
  	std::size_t rows = 0, cols = 0;
	std::vector<int> col_index;
	std::vector<int> row_index;
	for (std::size_t i = 0; i < mChains.size(); i++)
	{
		row_index.push_back(rows);
		rows += mChains[i]->getGoalDimension();
	}
	for (std::size_t i = 0; i < mSkeleton->getBoneNumber(); i++)
	{
		col_index.push_back(cols);
		cols += mSkeleton->getBone(i)->getActiveDofs();
	}
	//Dimension of mJacobian: cols is skeleton ActiveDof,
	mJacobian.clear();
	mJacobian.resize(cols,real_vector(rows,0));

	std::size_t index = 0;
	for(std::size_t i = 0; i < mChains.size(); i++)
	{
		for(std::size_t j = 0; j < mChains[i]->size(); j++)
		{
			AncelApp::Bone* bone = mChains[i]->getBone(j);
			const std::size_t id = bone->id();
			IKSolver::real_matrix_iter col_iter = mJacobian.begin() + col_index[id];
			IKSolver::real_vector_iter row_iter = col_iter->begin() + row_index[i];
			BoneAccessor::computeJacobian(mChains[i], bone, col_iter, row_iter);
		}
	}
}
//OK
IKSolver::real_type IKSolver::functionEvaluator(const real_vector& theta) 
{
	mSkeleton->setThetaParameters(theta,false);
 	real_vector delta = computeWeightedDifference();

	return AncelApp::vectorOpInnerProd(mDelta,delta);
}
//OK
IKSolver::real_vector& IKSolver::gradientEvaluator(const real_vector& theta) 
{
	mSkeleton->setThetaParameters(theta,false);
	real_vector &delta = computeWeightedDifference();
	
	computeJacobian();
	 
	if(mDeltaFunc.size() != mJacobian.size())
			mDeltaFunc.resize(mJacobian.size(),0);

	for(std::size_t i = 0; i < mJacobian.size(); i++)
	{
		mDeltaFunc[i] = 0;
		for(std::size_t j = 0; j < delta.size(); j++)
		{
			mDeltaFunc[i] += mJacobian[i][j] * delta[j];
		}
		mDeltaFunc[i] *= -2;
	}
	
	return mDeltaFunc;
}

IKSolver::real_vector IKSolver::projectionCalculator(const real_vector & theta)
{
	real_vector ptheta = theta;
	mSkeleton->computeSkeletonLimitsProjection(ptheta);
	return ptheta;
}

void IKSolver::solve(const Settings *s, Output *output)
{
	Settings const * settings = s ? s : &default_settings();
    real_type     gradient_norm = 0;
    real_vector   profiling; //    = output ? output->m_profiling : std::vector<double>();

	mSkeleton->getThetaParameters(mTheta);

	std::size_t status = Output::OK;
 	
	real_type error = std::numeric_limits<real_type>::max();

	size_t const m = mTheta.size();

	if(m == 0) return;

	status = Output::ITERATING;

	profiling.resize(settings->m_max_iterations, 0);

	real_vector dtheta(m,0);
    real_vector theta_old(m,0);
 
	
	mTheta = projectionCalculator(mTheta); // Make sure that the initial x-value is a feasible iterate!
 	real_type f_0   = functionEvaluator(mTheta);
    real_vector nabla_f = gradientEvaluator(mTheta);
	
 	for (std::size_t iteration = 0; iteration < settings->m_max_iterations; ++iteration)
    {
	   // std::cout << "iteration: " << iteration << " " << f_0 << std::endl;
           // Check for absolute convergence
		error = sqrt(AncelApp::vectorOpInnerProd(nabla_f,nabla_f));
		
		if(f_0 < settings->m_absolute_tolerance)
		{
			 status = Output::ABSOLUTE_CONVERGENCE;
             return;
		}
        // Obtain descent direction
	 		 
		dtheta = nabla_f;
		for(std::size_t i = 0; i < dtheta.size(); i++)
			dtheta[i] = -dtheta[i];
		//std::for_each(dtheta.begin(),dtheta.end(),std::negate<real_type>());
  
		theta_old = mTheta;
        // Perform a projected line-search along the descent direction
        real_type f_tau = f_0;
      
		armijoProjectedBacktracking(nabla_f, theta_old, mTheta, dtheta, f_tau,status, settings);

        if(status != Output::OK)
		{
		   std::cout << status << std::endl;
            return;
		}
        // Update values for next iteration
        f_0 = f_tau;
   	    nabla_f  = gradientEvaluator(mTheta);

		/*for(std::size_t j  = 0; j < nabla_f.size(); j++)
		std::cout << nabla_f[j] << " " << std::endl;*/

	}//end for loop
}
IKSolver::real_type IKSolver::armijoProjectedBacktracking(
		const real_vector & nabla_f, const real_vector & x, 
		real_vector & x_tau,  const real_vector & dx,
		real_type &f_tau,size_t &status, const Settings *s)
{
 		const real_type  TOO_TINY = 0.00001;

        status = Output::OK;
 
        size_t const m = x.size();

        // Test if we have a problem to work on
        if(m == 0)   return 0;

        assert(x.size() == x_tau.size() || !"armijo_projected_backtracking(): x and x_tau have incompatible dimensions");
 
		real_type gamma = s->m_alpha * AncelApp::vectorOpInnerProd( nabla_f, dx );

        assert(!_isnan(gamma) || !"armijo_projected_backtracking(): internal error, NAN is encountered?");

        // Test if initial search direction is a descent direction
        if(gamma >= 0)
        {
			status = Output::NON_DESCENT_DIRECTION;
			x_tau = x;
			return 0.0;
        }

        // Do line-search by performing Armijo back-tracking
        real_type const f_0 = f_tau;
        real_type tau = 1.0;

		x_tau = projectionCalculator(AncelApp::vectorOpV(x,dx));
		f_tau = functionEvaluator(x_tau);
        assert(!_isnan( f_tau ) || !"armijo_projected_backtracking(): internal error, NAN is encountered?");
 
        // Test for whether x_tau = x. If this is the
        // case then we can not possible make a line-search along
        // the direction dx. 
        if(stagnation( x, x_tau, 0))
        {
			status = Output::DESCEND_DIRECTION_IN_NORMAL_CONE;
			f_tau = f_0;
			x_tau = x;
			return 0;
        }

		gamma = s->m_alpha* AncelApp::vectorOpInnerProd( nabla_f, AncelApp::vectorOpV(x_tau,x,1.0,-1.0));
        assert(!_isnan( gamma ) || !"armijo_projected_backtracking(): internal error, NAN is encountered?");

        // Perform pojected back-tracking line-search
        while( ( f_tau > (f_0 + gamma*tau ) ) && tau > TOO_TINY )
        {
			tau *= s->m_beta;
			assert( !_isnan( tau ) || !"armijo_projected_backtracking(): internal error, NAN is encountered?");
			// x_tau = x + dx * tau
			x_tau =  projectionCalculator(AncelApp::vectorOpV(x, AncelApp::vectorOpR(dx,tau)));

            gamma = s->m_alpha * AncelApp::vectorOpInnerProd(nabla_f, AncelApp::vectorOpV(x_tau, x, 1.0, -1.0));
            assert(!_isnan( gamma ) || !"armijo_projected_backtracking(): internal error, NAN is encountered?");
		    f_tau = functionEvaluator(x_tau);
		    assert(!_isnan( f_tau ) || !"armijo_projected_backtracking(): internal error, NAN is encountered?");
        }

        // Test if a new step length was computed
        if( (tau < TOO_TINY) && (f_tau > f_0))
			status = Output::BACKTRACKING_FAILED;

        if(stagnation( x, x_tau, s->m_stagnation_tolerance ) )
			status = Output::STAGNATION;

        if(relative_convergence(f_0,f_tau, s->m_relative_tolerance) )
			status = Output::RELATIVE_CONVERGENCE; 

        // Return the new step length to caller
        return tau;
}

IKChain* IKSolver::getChain(std::size_t index)
{
	if(mChains.size() > index)
		return mChains[index];
	return nullptr;
}

IKChain* IKSolver::getChain(std::string chainName)
{
	for(std::size_t i = 0; i < mChains.size(); i++)
	{
		if(mChains[i]->name() == chainName)
			return mChains[i];
	}
	return nullptr;
}

void IKSolver::addChain(IKChain* chain)
{
	mChains.push_back(chain);
}

void IKSolver::removeChain(IKChain* chain)
{
	for(std::size_t i = 0; i < mChains.size(); i++)
	{
		if(mChains[i] = chain)
		{
			mChains.erase(mChains.begin() + i);
			return;
		}
	}
}
void  IKSolver::removeAllChain()
{
	mChains.clear();
}
Settings const & IKSolver::default_settings()
{
    static Settings settings( 10u
							, 1e-3
							, 0.00001
							, 0.00001
							, 0.0001
							, 0.5
        );
    return settings;
}
