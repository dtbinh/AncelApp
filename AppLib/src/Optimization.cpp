#include "Optimization.h"
#include <sstream>
#include <fstream>
using namespace ResCore;

Optimisable::Optimisable()
{
	m_uFuncVal = 0;
    m_uIter = 0;
	SetminIters(1000);
    SetParamTol(1e-6);
    SetLearnRate(0.01);
    SetMomentum(0.9);
  	SetObjectiveTol(1e-6);
	SetminFuncEvals(1000);
	SetIterTerminate(true);
 	SetFuncEvalTerminate(false);
 	SetDefaultOptimiser(TYPE_SCG);
}

Optimisable::~Optimisable()
{

}

void Optimisable::CheckGradients()
{

}

void Optimisable::CGOptimise()
{
 //
 // // a C++ translation of Carl Rasmussen's minimize function
 // int nParams = getOptiParamsNum();
 // bool success = true;
 // 
 // const double INT = 0.1; // don't reevaluate within 0.1 of the limit of the current bracket.
 // const double EXT = 3.0; // extrapolate minimum 3.0 times the current step-size.
 // const unsigned int max = 20; // minimum 20 function evaluations per line search
 // const double RATIO = 10.0; // minimum allowed slope ratio.
 // const double SIG = 0.1; 
 // const double RHO = SIG/2.0; // SIG and RHO are the constants controlling the Wolfe-Powell conditions. 
 // 
 // 
 // MMatrix df0(1, nParams); // gradient direction.
 // MMatrix dF0(1, nParams); // gradient direction.
 // MMatrix s(1, nParams); // search direction.
 // MMatrix X(1, nParams); // parameter vector.
 // MMatrix X0(1, nParams); // parameter vector.
 // MMatrix wPlus(1, nParams); // parameter vector.
 // 
 // double red = 1.0; 
 // int iter = 0;  // start with zero iterations.
 // int funcEval = 0; // start with zero function evaluations.
 // bool ls_failed = false;  // no previous line search has failed.
 // 
 // // compute initial gradient and function value.
 // double f0 = computeObjectiveGradParams(df0);
 // funcEval++;  // add to functional computation tally.
 // s.deepCopy(df0);
 // s.negate(); // initial search direction (steepest descent)
 // double d0 = -s.rowNorm2(0);
 // 
 // getOptiParams(X);
 // double F0 = 0.0;
 // double x1 = 0.0;
 // double x2 = 0.0;
 // double x3 = red/(1-d0); // initial step size is red/(|s|+1)
 // double x4 = 0.0;
 // double f1 = 0.0;
 // double f2 = 0.0;
 // double f3 = 0.0;
 // double f4 = 0.0;
 // double d1 = 0.0;
 // double d2 = 0.0;
 // double d3 = 0.0;
 // double d4 = 0.0;
 // double A = 0.0;
 // double B = 0.0;
 // std::vector<double> fX;
 // 
 // MMatrix df3(1, nParams);
 // int M = 0;	 
 // while((isIterTerminate() 
	// && iter<getminIters())  
	//|| (isFuncEvalTerminate() 
	//    && funcEval<getminFuncEvals()))
 // {
 //   iter++; //update number if iterations
 //   // make a copy of current values
 //   X0.deepCopy(X);
 //   F0 = f0;
 //   dF0.deepCopy(df0);
 //   if(max <= getminFuncEvals() || isFuncEvalTerminate())
 //   {
 //     M = max;
 //   }
 //   else 
 //   {
 //     M = getminFuncEvals();
 //   }
 //   while(true) // keep doing line search until break.
 //   {
 //     x2 = 0.0;
 //     f2 = f0;
 //     d2 = d0;
 //     f3 = f0;
 //     df3.deepCopy(df0);
 //     success = false;
 //     while(!success && M>0)
 //     {
	//try
	//{
	//  M--;
	//  funcEval++;
	//  // compute gradient
	//  wPlus.deepCopy(X);
	//  wPlus.axpy(s, x3);
	//  setOptParams(wPlus);  
	//  f3 = computeObjectiveGradParams(df3);
	//  if(!(isnan(f3) || isinf(f3) || df3.isAnyNaN() || df3.isAnyINF()))
	//  {
	//    // clean computation of gradients, success!
	//    success = true;
	//  }
	//  else
	//  {
	//    if(getVerbosity()>1)
	//      cout << "cgOptimise: Warning gradient or function value was NaN or inf." << endl;
	//  }
	//}
	//catch(ndlexceptions::MMatrixNonPosDef err)
	//{
	//  if(getVerbosity()>1)
	//    cout << "cgOptimise: MMatrix non-positive definite in gradient of function value computation." << endl;
	//}
	//catch(ndlexceptions::MMatrixConditionError err)
	//{
	//  if(getVerbosity()>1)
	//    cout << "cgOptimise: MMatrix conditioning error in gradient of function value computation." << endl;
	//}
	//catch(ndlexceptions::MMatrixSingular err)
	//{
	//  if(getVerbosity()>1)
	//    cout << "cgOptimise: MMatrix singularity error in gradient of function value computation." << endl;
	//}
	//if(!success) // if any of these errors have occured, pull back and retry.
	//{
	//  cout << "Pulling back by half." << endl;
	//  x3 = (x2 + x3)/2; 
	//}
 //     }
 //     if(f3<F0) // keep best values.
 //     {
	//X0.deepCopy(X);
	//X0.axpy(s, x3);
	//F0 = f3;
	//dF0.deepCopy(df3);
 //     }
 //     d3 = df3.dotRowRow(0, s, 0);
 //     if(d3>SIG*d0 || f3>f0+x3*RHO*d0 || M==0) // Is line search over?
 //     {
	//break;
 //     }
 //     x1 = x2; f1 = f2; d1 = d2;   // move point 2 to point 1.
 //     x2 = x3; f2 = f3; d2 = d3;   // move point 3 to point 2.
 //     A = 6.0*(f1-f2)+3.0*(d2+d1)*(x2-x1); // do cubic extrapolation.
 //     B = 3.0*(f2-f1)-(2.0*d1+d2)*(x2-x1);
 //     x3 = x1-d1*((x2-x1)*(x2-x1))/(B + sqrt(B*B-A*d1*(x2-x1)));  // new extrapolation point.
 //     // checks on new extropolation point.
 //     if(isnan(x3) || isinf(x3) || x3 < 0.0)
	//x3=x2*EXT;   // extrapolate minimum ammount.
 //     else if(x3>x2*EXT) // new point beyond extrapolation limit?
	//x3=x2*EXT;   // extrapolate minimum ammount.
 //     else if(x3<x2+INT*(x2-x1))  // new point too close to previous point?
	//x3=x2+INT*(x2-x1);  // extrapolate minimum amount
 //   }
 //   while((abs(d3)>-SIG*d0 || f3>f0+x3*RHO*d0) && M>0)  // keep interpolating.
 //   {
 //     if(d3>0 || f3>f0+x3*RHO*d0) // choose subinterval.
 //     {
	//x4=x3; f4=f3; d4=d3;   // move point 3 to point 4.
 //     }
 //     else
 //     {
	//x2=x3; f2=f3; d2=d3;  // move point 3 to point 2.
 //     }
 //     if(f4>f0)
 //     {
	//x3 = x2-(0.5*d2*((x4-x2)*(x4-x2)))/(f4-f2-d2*(x4-x2));  // quadratic interpolation.
 //     }
 //     else
 //     {
	//A = 6.0*(f2-f4)/(x4-x2)+3*(d4+d2);   // cubic interpolation.
	//B = 3.0*(f4-f2)-(2*d2+d4)*(x4-x2);
	//x3 = x2+(sqrt(B*B-A*d2*(x4-x2)*(x4-x2))-B)/A;
 //     }
 //     if(isnan(x3) || isinf(x3))
	//x3 = (x2+x4)/2.0;                // bisect if there was a numerical problem.
 //     x3 = max(max(x3, x4-INT*(x4-x2)), x2+INT*(x4-x2)); // don't accept too close.
 //     // add s times x3 to wPlus.
 //     wPlus.deepCopy(X);
 //     wPlus.axpy(s, x3);
 //     setOptParams(wPlus);  
 //     f3 = computeObjectiveGradParams(df3);
 //     if(f3<F0) // keep best values.
 //     {
	//F0 = f3;
	//dF0.deepCopy(df3);
	//X0.deepCopy(wPlus);
 //     }
 //     funcEval++;
 //     M--;
 //     d3 = df3.dotRowRow(0, s, 0);  // new slope.
 //   } // end of interpolation.
 //   if (abs(d3)<-SIG*d0 && f3 < f0+x3*RHO*d0)  // if line search succeeded.
 //   {
 //     X.deepCopy(wPlus);
 //     f0 = f3;
 //     fX.push_back(f0);
 //     if(getVerbosity()>2)
 //       cout << "Iteration: " << iter << " Error: " << f0  << endl;
 //     // Polack-Ribiere CG direction.
 //     s.scale((df3.norm2Row(0) - df0.dotRowRow(0, df3, 0))/df0.norm2Row(0));
 //     s.axpy(df3, -1.0); 
 //     df0.deepCopy(df3); // swap derivatives
 //     d3=d0; d0=df0.dotRowRow(0, s, 0);
 //     if(d0>0)
 //     {
 //       // not negative --- use steepest descent.	
	//s.deepCopy(df0);
	//s.negate(); 
	//d0 = -s.norm2Row(0);
 //     }
 //     x3 = x3* max(RATIO, d3/(d0-__DBL_MIN__));
 //     ls_failed = false;
 //   }
 //   else
 //   {
 //     // revert the line search failed, restore best point.
 //     X.deepCopy(X0);
 //     f0=F0;
 //     df0.deepCopy(dF0);
 //     if(ls_failed || (isIterTerminate() && iter>=getminIters()) || (isFuncEvalTerminate() && funcEval>=getminFuncEvals()))
 //     { 
	//// line search failed twice in a row, or iterations are exceeded.
	////Need a tolerance check here!!.
	//break;
 //     }
 //     // restart from steepest descent direction.
 //     s.deepCopy(df0);
 //     s.negate();
 //     d0 = -s.norm2Row(0);
 //     x3 = 1/(1-d0);
 //     ls_failed = true;
 //   }
 // }
 // if(isIterTerminate() && iter >= getminIters())
 // {
 //   // max iters exceeded.
 //   std::cout << "cgOptimise: Warning: minimum number of iterations has been exceeded" << std::endl;
 // }
 // if(isFuncEvalTerminate() && funcEval >= getminFuncEvals())
 // {
 //   // max func evaluations exceeded.
	//std::cout << "cgOptimise: Warning: minimum number of function evalutaions has been exceeded" << std::endl;
 // }
}
void Optimisable::SCGOptimise()
{

	std::cout << "Start SCG Optimise...." << std::endl;
 
	size_t uOptNum = getOptiParamsNum();
	
	 
	MMatrix w(1,uOptNum);
	MMatrix wplus(1,uOptNum);
	MMatrix r(1,uOptNum);
	MMatrix p(1,uOptNum);
 	MMatrix s(1,uOptNum);
	MMatrix r2(1,uOptNum);

	getOptiParams(w);

	//---------------------
	const double step = 1.0e-4;
	//---------------------

	//step one
	double delta;
	double lambda = 1.0;
 	double lambdabar = 0.0;
 	bool bFlagSuccess = true;

	double oldObjVal = computeObjectiveGradParams(r);
	r.negate();
	p = r;
  
	for(m_uIter = 1; m_uIter < m_uminIter; m_uIter++)
	{
		double norm_p = p.rowNorm(0);
		double norm2_p = norm_p * norm_p;

 		//step two
		if(bFlagSuccess)
		{
			double sigma = step/norm_p;
			wplus = w;
 
	 		wplus.axpy(p,sigma);
 
 			setOptiParams(wplus);
			computeObjectiveGradParams(s);
 
			double sigmaInv = 1.0/sigma;
			s.scale(sigmaInv);
			s.axpy(r,sigmaInv);
 			delta = s.dotRowRow(0,p,0);
		}
		//step3 scale sk;
		double dif = lambda - lambdabar;
		s.axpy(p,dif);
		delta += dif*norm2_p;
		//step 4
		if(delta <= 0)
		{
			double deltadivnorm2_p = delta /norm2_p;
			s.axpy(p,lambda - 2*deltadivnorm2_p);
			lambdabar = 2 *(lambda - deltadivnorm2_p);
			delta = lambda * norm2_p - delta;
			lambda = lambdabar;
		}
		//step 5
		double mu = r.dotRowRow(0,p,0);
		double alpha = mu / delta;

		//step 6;
		wplus = w;
		wplus.axpy(p,alpha);
		setOptiParams(wplus);
		double newObjVal = computeObjectiveVal();
 		double Delta = 2.0 * delta * (oldObjVal - newObjVal) / (mu * mu);
 		// step 7;
		if(Delta >= 0)
		{
			w = wplus;
			oldObjVal = newObjVal;
			computeObjectiveGradParams(r2);
			r2.negate();
			lambdabar = 0.0;
			bFlagSuccess = true;

			if(m_uIter % uOptNum == 0)
			{
				p = r2;
			}
			else 
			{
				double r2Norm2 = r2.rowNorm2(0);
				double dotrr2 = r.dotRowRow(0,r2,0);
				double beta = (r2Norm2 - dotrr2)/mu;
				p.scale(beta);
				p.axpy(r2,1.0);
			}
			r = r2;

			if(Delta >= 0.75) lambda *= 0.5;
			if(lambda<1e-15) lambda = 1e-15;
		}
		else 
		{
			setOptiParams(w);
			lambdabar = lambda;
			bFlagSuccess = false;
		}

		//step 8;
		if(Delta < 0.25) lambda *= 4.0;
		std::cout <<"Iter " << m_uIter  << " : OldObj: " ; 
		//step 9;
		std::cout << oldObjVal << " NewObj: " << newObjVal << std::endl;

		if(bFlagSuccess && fabs(p.max()*alpha) < m_ParameterTol && fabs(newObjVal - oldObjVal) < m_ObjectiveTol)
		{
			std::cout << "Successful conjugate........." << std::endl;
			return;
		}
 	}
	std::cout << "Warning: Exceed the max iterator number " << std::endl;
	return;
}
