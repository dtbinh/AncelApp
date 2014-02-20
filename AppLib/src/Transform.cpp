#include "Transform.h"
#include <cassert>

using namespace ResCore;

const double CTransform::EPS = 1e-16;

CTransform* CTransform::DefaultPositive()
{
	return new CExpTransform();
}
CTransform* CTransform::DefaultZeroOne()
{
	return new CSigmoidTransform();
}
CTransform* CTransform::GetNewTransformPointer(TRANSTYPE tsType)
{
	if(tsType == TRANS_EXP)
		return new CExpTransform();
	else if(tsType == TRANS_NEGLOGLOGIT)
		return new CNegLogLogitTransform();
	else if(tsType == TRANS_LINEAR)
		return new CLinearTransform();
	else if(tsType == TRANS_SIGMOID)
		return new  CSigmoidTransform();
	return NULL;
}
 
//-------------------------Exp Transform---------------------------------------------
CExpTransform::CExpTransform()
{
 	SetType(TRANS_EXP);
}

double CExpTransform::AtoX(double a) const   // x = e^a;
{
	double x = 0;
	if(a < -limVal)
		x = exp(-limVal);
	else if(a < limVal)
		x = exp(a);
	else
		x = exp(limVal);
	
	assert(!_isnan(x));
    
	return x;
}

double CExpTransform::XtoA(double x) const // a = log x;
{
	assert(x > 0);
 	double a = log(x);
 	assert(!_isnan(a));
	return a;
}
double CExpTransform::Gradient(double x) const //x = e^a; x' = e^a;
{
	return x;
}
//-----------------------------Negative logarithm logit Transform---------------------------------------------
CNegLogLogitTransform::CNegLogLogitTransform()
{
 	SetType(TRANS_NEGLOGLOGIT);
}

double CNegLogLogitTransform::AtoX(double a) const //x = log(1 + e^a)
{
	double x = 0;
	if(a < -limVal)
		x = exp(-limVal);
	else if(a < limVal)
		x = log(1 + exp(a));
	assert(!_isnan(x));
	return x;
}

double CNegLogLogitTransform::XtoA(double x) const
{
	double a = x;
    assert(a >- limVal);
	if(a < limVal)
		a = log(exp(x) - 1);
	assert(!_isnan(a));
	return a;
}

double CNegLogLogitTransform::Gradient(double x) const
{
	double g;
	if(x < limVal)
		g = (exp(x) - 1) / exp(x);
	else
		g = 1.0;
	return g;
}
//----------------------------------Linear Transform-----------------------------------------------
CLinearTransform::CLinearTransform()
{
 	SetType(TRANS_LINEAR);
}
double CLinearTransform::AtoX(double a) const 
{
	return (a - m_dvalC) / m_dvalM;
}
double CLinearTransform::XtoA(double x) const
{
	return (m_dvalM * x) + m_dvalC;
}
double CLinearTransform::Gradient(double x) const
{
	return 1 / m_dvalM;
}
double CLinearTransform::getM() const
{
	return m_dvalM;
}
void CLinearTransform::SetValM(const double M)
{
	m_dvalM = M;
}
double CLinearTransform::getC() const
{
	return m_dvalC;
}
void CLinearTransform::SetValC(const double C)
{
	m_dvalC = C;
}
//----------------------------------Sigmoid Transform----------------------------------------------
CSigmoidTransform::CSigmoidTransform()
{
 	SetType(TRANS_SIGMOID);
}

double CSigmoidTransform::AtoX(double a) const // 1.0/(1.0+exp(-x));
{
	if(a < - limVal)
		return DBL_EPSILON;
	if(a < limVal)
		return 1.0 / (1.0 + exp(-a));
	else
		return 1 - DBL_EPSILON;
}
double CSigmoidTransform::XtoA(double x) const
{
	return log(x / (1.0 - x));
}
double CSigmoidTransform::Gradient(double x) const
{
	return x * (1 - x);
}