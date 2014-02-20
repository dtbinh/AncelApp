#pragma once

#include "nlopt.h"
#include "nlopt-util.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

nlopt_result lbfgsb_minimize(int n, nlopt_func f, void *f_data,
                  const double *lb, const double *ub, /* bounds */
                  double *x, /* in: initial guess, out: minimizer */
                  double *minf,
		  nlopt_stopping *stop,
			 int mf);

#ifdef __cplusplus
}
#endif /* __cplusplus */
