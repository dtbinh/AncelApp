#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "l-bfgs-b.h"

extern "C" void lbfgs_(long int* numVariables, 
		       long int* numCorrections,
		       double* X,
		       double* funcVal,   // set by user to be func val.
		       double* gradVals,  // set by user to be grad vals.
		       long int* diagCo,
		       double* diag,
		       long int* iPrint,
		       double* prec,
		       double* xtol,
		       double* W, // work vector size N(2M+1) + 2M
		       long int* iFlag);

nlopt_result lbfgsb_minimize(int n, nlopt_func func, void *f_data,
                  const double *lb, const double *ub, /* bounds */
                  double *x, /* in: initial guess, out: minimizer */
                  double *minf,
		  nlopt_stopping *stop,
			 int mf)
{
  long int funcEval = 0;
  long int nParams = n;
  long int liflag = 0;
  long int memSize = 10;
  long int verbosity = 0;
  double* Xvals = new double[nParams];
  double* work = new double[nParams*(2*memSize+1) + 2*memSize];
  double* gvals = new double[nParams];
  double* diagVals = new double[nParams];
  double* X = new double[nParams];
  double* g = new double[nParams];
  nlopt_result ret = NLOPT_SUCCESS;

  long int iPrint[3] ={-1, 0, 0};
  if(verbosity>2)
  {
    iPrint[0] = 1;
  }
  double f = 0.0;
  memcpy(X,x,sizeof(double)*nParams);//getOptParams(X);
  while(funcEval<stop->maxeval)
  {
    f = func(n,x,g,f_data); //f = computeObjectiveGradParams(g);
    for (int i = 0; i < nParams; i++)
    {
        Xvals[i] = X[i];
        gvals[i] = g[i];
    }
      long int zero = 0;
    lbfgs_(&nParams, &memSize, Xvals, &f, gvals, &zero, diagVals, iPrint, &stop->ftol_rel, &stop->xtol_rel, work, &liflag);
    int iflag = (int)liflag;
    if(iflag<=0)
    {
      if(iflag==-1)
      {
        //printf("Warning: lbfgsOptimise: linesearch failed.");
          ret = NLOPT_FTOL_REACHED;
        break;
      }
      else if(iflag == -2)
      {
        //printf("An element of the inverse Hessian provided is not positive.");
        ret = NLOPT_FORCED_STOP;
      }
      else if(iflag == -3)
      {
        //printf("Inproper input to lbfgs_.");
        ret = NLOPT_INVALID_ARGS;
      }
    }
    else if(iflag==0)
    {
      break;
    }
    else if(iflag==1)
    {
      for (int i = 0; i < nParams; i++)
          X[i] = Xvals[i];
      for (int i = 0; i < nParams; i++)
          x[i] = X[i];
      funcEval++;
    }
    else
    {
      //printf("Unhandled iflag %d.",(int)iflag);
          ret = NLOPT_INVALID_ARGS;
    }
  }

  // Clean up.
  delete[] Xvals;
  delete[] work;
  delete[] gvals;
  delete[] diagVals;
  delete[] X;
  delete[] g;

  return ret;
}
