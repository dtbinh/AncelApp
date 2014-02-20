#ifndef __IKSolver_h_
#define __IKSolver_h_

#include <list>
#include <vector>
#include "Skeleton.h"
#include "IKChain.h"

namespace AncelIK
{
	class Settings
	{
	public:
		typedef double real_type;
	
	public:
		Settings()
		: m_max_iterations(0u)
        , m_absolute_tolerance(0)
        , m_relative_tolerance(0)
        , m_stagnation_tolerance(0)
        , m_alpha(0)
        , m_beta(0)
        , m_resynchronization( false )
        {}

		Settings(
			size_t    const & max_iterations
		, real_type const & absolute_tolerance
		, real_type const & relative_tolerance
		, real_type const & stagnation_tolerance
		, real_type const & alpha
		, real_type const & beta
		)
		  :m_max_iterations(max_iterations)
		, m_absolute_tolerance( absolute_tolerance )
		, m_relative_tolerance( relative_tolerance )
		, m_stagnation_tolerance( stagnation_tolerance )
		, m_alpha( alpha )
		, m_beta( beta )
		{}

        Settings(Settings const & s){  *this = s; }

        Settings & operator=(Settings const & s)
        {
			if(this != &s)
			{
 				m_max_iterations       = s.m_max_iterations;
				m_absolute_tolerance   = s.m_absolute_tolerance;
				m_relative_tolerance   = s.m_relative_tolerance;
				m_stagnation_tolerance = s.m_stagnation_tolerance;
				m_alpha                = s.m_alpha;
				m_beta                 = s.m_beta;
			 }
			return *this;
        }
	public:
		size_t    m_max_iterations;        ///< This argument holds the value of the maximum allowed iterations.
		real_type m_absolute_tolerance;    ///< This argument holds the value used in the absolute stopping criteria. Setting the value to zero will make the test in-effective.
		real_type m_relative_tolerance;    ///< This argument holds the value used in the relative stopping criteria. Setting the value to zero will make the test in-effective.
		real_type m_stagnation_tolerance;  ///< This argument holds the value used in the stagnation test. It is an upper bound of the infinity-norm of the difference in the x-solution between two iterations.  Setting the value to zero will make the test in-effective.
		real_type m_alpha;                 ///< Armijo test paramter, should be in the range 0..1, this is the fraction of sufficient decrease that is needed by the line-search method. A good value is often 0.00001;
		real_type m_beta;                  ///< The step-length reduction parameter. Everytime the Armijo condition fails then the step length is reduced by this fraction. Usually alpha < beta < 1. A good value is often 0.5;
 		bool      m_resynchronization;

	};

			//-------------------------------------------------------------------------------------
	class Output
    {
	public:
		typedef double real_type;
		typedef std::vector<real_type>	 real_vector;
	public:
		Output()
        : m_value(0)
        , m_wall_time(0)
        , m_status(0)
        , m_error_message("")
        , m_iterations(0u)
        , m_gradient_norm(std::numeric_limits<real_type>::max())
        {}

        Output( Output const & o)
        {
			 *this = o;
        }

        Output & operator=( Output const & o)
        {
			if( this != &o)
			{
				m_value         = o.m_value;
				m_wall_time     = o.m_wall_time;
				m_status        = o.m_status;
				m_error_message = o.m_error_message;
				m_iterations    = o.m_iterations;
				m_gradient_norm = o.m_gradient_norm;
				m_profiling     = o.m_profiling;
			}
			return *this;
        }
	public:
		 static size_t const OK = 0;
		 static size_t const NON_DESCENT_DIRECTION = 1;
		 static size_t const BACKTRACKING_FAILED = 2;
		 static size_t const STAGNATION = 3;
		 static size_t const RELATIVE_CONVERGENCE = 4;
		 static size_t const ABSOLUTE_CONVERGENCE = 5;
		 static size_t const ITERATING  = 6;
		 static size_t const DESCEND_DIRECTION_IN_NORMAL_CONE  = 7;
    public:

          real_type   m_value;           ///< The value of the objective function.
          real_type   m_wall_time;       ///< The wall clock time it took for the numerical method to exit.
          size_t      m_status;          ///< The status of the numerical method when exiting.
          std::string m_error_message;   ///< A textual version of the status.
          size_t      m_iterations;      ///< The number of used iterations.
          real_type   m_gradient_norm;   ///< The value of the gradient norm.
          real_vector m_profiling;       ///< The values of the merit function at each iterates.
	};

	class IKSolver
	{
	public:

		typedef AncelApp::Skeleton			        skeleton_type;
		typedef skeleton_type::bone_type			bone_type;
		typedef double								real_type;
		typedef std::vector<real_type>				real_vector;
		typedef std::vector<real_vector>	        real_matrix;
		typedef real_vector::iterator				real_vector_iter;
		typedef real_matrix::iterator				real_matrix_iter;
 
  	public:
		IKSolver();
		~IKSolver();
		//void addChain(IKChain<bone_type> &chain);
		bool initSlover(const AncelApp::Skeleton *skeleton);

		void setEndEffectorGoal(std::size_t chainID, const Ogre::Vector3& pos, 
								const Ogre::Vector3& axisX = Ogre::Vector3(), 
								const Ogre::Vector3& axisY = Ogre::Vector3());

		void setEndEffectorGoal(std::string chainName, const Ogre::Vector3& pos, 
								const Ogre::Vector3& axisX = Ogre::Vector3(), 
								const Ogre::Vector3& axisY = Ogre::Vector3());

		void    solve(const Settings *s = NULL,Output *output = NULL);
 		static  Settings const & IKSolver::default_settings();
		IKChain* getChain(std::size_t index);
		IKChain* getChain(std::string chainName);
		
		void    addChain(const std::string& root, const std::string leaf);
		void    addChain(IKChain* chain);
		void    removeChain(IKChain* chain);
		void    removeChain(const std::size_t index);
		void    addAllChain();
		void    removeAllChain();
		std::size_t     getChianNum() const {return mChains.size();}
 	protected:
		
		real_type	 functionEvaluator(const real_vector& theta);
		real_vector& gradientEvaluator(const real_vector& theta);
		real_vector  projectionCalculator(const real_vector & theta);
		real_vector& computeDifference();
		real_vector  computeWeightedDifference();
 		
		real_type    armijoProjectedBacktracking(	
					 const real_vector & nabla_f, const real_vector & x, 
					 real_vector & x_tau,  const real_vector & dx,
					 real_type &f_tau,size_t &status, const Settings *s);

 		void		 computeJacobian();
 		bool relative_convergence(real_type f_old, real_type f, real_type tolerance);
	    bool stagnation(real_vector const & x_old,real_vector const & x, real_type const & tolerance);
	protected:
		skeleton_type*		   mSkeleton;

		real_vector			   mDelta;
		real_vector			   mTheta;
		real_matrix			   mJacobian;
		real_vector			   mDeltaFunc;
		std::vector<IKChain*>  mChains;
 	};


	
}
#endif