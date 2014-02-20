#ifndef __BoneAccessor_h
#define __BoneAccessor_h

#include "Bone.h"
#include "IKChain.h"
#include "IKSolver.h"

namespace AncelIK
{
	class BoneAccessor
	{
	public:
		static void computeJacobian(const IKChain* chain,const AncelApp::Bone* bone, IKSolver::real_matrix_iter col_iter, IKSolver::real_vector_iter row_iter);
	};
}

#endif