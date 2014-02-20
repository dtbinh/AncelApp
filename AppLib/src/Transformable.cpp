#include "Transformable.h"
#include <algorithm>
#include <iostream>
using namespace ResCore;

void Transformable::getParams(MMatrix& matParams) const
{
	assert(matParams.sizeCol() == 1 && matParams.sizeRow() == getNumParams());
	for(size_t i = 0; i < matParams.sizeCol(); i++)
	{
		matParams.assign(getParam(i), i);
	}
}
void Transformable::getTransParams(MMatrix& matTransParams) const
{
	//assert(matTransParams.sizeCol() == 1 && matTransParams.sizeRow() == getNumParams());
 	for(size_t i = 0; i < matTransParams.sizeCol(); i++) 
	{
 		matTransParams.assign(m_vTransform[i]->XtoA(getParam(i)), i);
	}
}
void Transformable::setParams(MMatrix& matParams)
{
	assert(matParams.sizeCol() == 1 && matParams.sizeRow() == getNumParams());
	 
	for(size_t i = 0; i < matParams.sizeCol(); i++)
	{
		setParam(matParams.get(i),i);
	}
}
void Transformable::setTransParams(const MMatrix& matTransParam)
{
//	assert(matTransParam.sizeCol() == 1 && matTransParam.sizeRow() == getNumParams());
  	for(size_t i = 0; i < matTransParam.sizeCol(); i++) 
	{
		double tdVal = matTransParam.get(i);
		setParam(m_vTransform[i]->AtoX(tdVal),i);
	}
}

void Transformable::getGradientTransParams(MMatrix& matGrad) const
{
	assert(matGrad.sizeCol() == 1 && matGrad.sizeRow() == getNumParams());
	
	getGradientParams(matGrad);
	size_t unParamsNum = getNumTransform();
	for(size_t i = 0; i < unParamsNum; i++)  
	{
		double tdVal  = matGrad.get(i);
		double tdValX = getParam(i);
		matGrad.assign(tdVal * m_vTransform[i]->Gradient(tdValX),i);
	}
}
void Transformable::setTransParam(double val,size_t index)
{
	assert(index < getNumParams());
  	setParam(m_vTransform[index]->AtoX(val), index);	 
}
double Transformable::getTransParam(size_t index) const
{
	assert(index < getNumParams());
 	return m_vTransform[index]->XtoA(getParam(index));
}
