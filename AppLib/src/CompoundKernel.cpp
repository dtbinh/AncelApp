#include "CompoundKernel.h"

using namespace ResCore;

CompoundKernel::CompoundKernel()
{
	initKernel();
}
CompoundKernel::~CompoundKernel()
{
	for(size_t i = 0; i < m_Components.size(); i++)
		delete m_Components[i];
}
CompoundKernel::CompoundKernel(const CompoundKernel& cmpnd)
{
	initKernel();
 	for(size_t i = 0; i < m_Components.size(); i++)
		addKernel(m_Components[i]->Clone());
}
CompoundKernel* CompoundKernel::Clone() const
{
	return new CompoundKernel(*this);
}
size_t CompoundKernel::addKernel(const Kernel * kern)
{
	m_Components.push_back(kern->Clone());
 	size_t oldNParams = m_ParamsNum;
 	m_ParamsNum += kern->getNumParams();

	for(size_t i=0; i< kern->getNumTransform(); i++)
	{
		addTransform(kern->getTransform(i), i + oldNParams);  
	}
	mhps.resize(m_ParamsNum);
	return  m_Components.size() - 1;
}
void CompoundKernel::setParam(double val, size_t index)
{
	size_t uStart = 0;
	size_t uEnd = 0;
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		uEnd = uStart + m_Components[i]->getNumParams() - 1;
		if(index <= uEnd)
		{
			m_Components[i]->setParam(val, index - uStart);
			return;
		}      
		uStart = uEnd + 1;
	}
}
double CompoundKernel::getParam(size_t index) const
{
	size_t uStart = 0;
	size_t uEnd = 0;
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		uEnd = uStart + m_Components[i]->getNumParams()-1;
		if(index <= uEnd)
 			return m_Components[i]->getParam(index - uStart);
 		uStart = uEnd + 1;
	}
	return -1;
}

std::string CompoundKernel::getParamName(size_t index) const
{
	size_t uStart = 0;
	size_t uEnd = 0;
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		uEnd = uStart + m_Components[i]->getNumParams()-1;
		if(index <= uEnd) {
			return m_Components[i]->getType() + m_Components[i]->getParamName(index - uStart);
		}
 		uStart = uEnd + 1;
	}
	return "";	
}
		
 
void CompoundKernel::AddPrior(Distribution* prior, size_t index)
{
	assert(false);
}
double CompoundKernel::PriorLogProb() const
{
	double L = 0.0;
	for(size_t t = 0; t < m_Components.size(); t++)
	{
		//L += m_Components[t]->PriorLogProb();
	}
	return L;
}

size_t CompoundKernel::GetKernsNum() const
{
	return m_Components.size();
}

void CompoundKernel::initKernel()
{
	m_ParamsNum = 0;
	mKernelType = "compound";
	setName("kernel_COMPOUND");
}
 	 
double CompoundKernel::getWhiteNoise() const
{
	double whiteNoise = 0.0;
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		whiteNoise  += m_Components[i]->getWhiteNoise();
	}
	return whiteNoise;
}
double CompoundKernel::computeDiagElement(const MMatrix & X,size_t index) const
{
	double dlgElement = 0.0;
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		dlgElement  += m_Components[i]->computeDiagElement(X,index);
	}
	return dlgElement;
}
void CompoundKernel::computeDiag(MMatrix &d, const MMatrix &X) const
{
	assert(d.sizeCol() == 1);
	assert(X.rowsMatch(d));
	d.zeroElements();
		  
	MMatrix dStore(d.sizeRow(), d.sizeCol(), 0.0);
	for(size_t i=0; i < m_Components.size(); i++)
	{
		m_Components[i]->computeDiag(dStore, X);
		d.axpy(dStore, 1.0);
	}
}

void   CompoundKernel::getGradientX(MMatrix &g, const MMatrix& X, size_t uRow, const MMatrix&X2,bool addG) const
{
	assert(g.sizeRow() == X2.sizeRow());
	assert(uRow < X.sizeRow());
	assert(X.sizeCol() == X2.sizeCol());
	if(!addG)
		g.zeroElements();
	
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->getGradientX(g, X, uRow, X2, true);
	}
}

void CompoundKernel::getDiagGradientX(MMatrix &g, const MMatrix &X,bool addG) const
{
	assert(g.dimensionsMatch(X));
	if(!addG)
		g.zeroElements();

	for(size_t i=0; i < m_Components.size(); i++)
	{
		m_Components[i]->getDiagGradientX(g, X, true);
	}
}

double CompoundKernel::computeElement(const MMatrix &X1,size_t uIndex1,const MMatrix &X2,size_t uIndex2) const
{
	double sumElement = 0.0;
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		sumElement += m_Components[i]->computeElement(X1,uIndex1,X2,uIndex2);
	}
	return sumElement;
}

void  CompoundKernel::getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &covGrad,bool regularise) const
{
	assert(g.sizeRow() == 1);
	assert(g.sizeCol() == getNumParams());

		 
	size_t uStart = 0;
	size_t uEnd   = 0;

	for(size_t i = 0; i < m_Components.size(); i++)
	{
		uEnd = uStart + m_Components[i]->getNumParams() - 1;
		MMatrix subg(1,m_Components[i]->getNumParams());
		m_Components[i]->getGradientParams(subg,X,covGrad,regularise);
		g.setMMatrix(0,uStart,subg);
		uStart = uEnd + 1;
	}
}

void  CompoundKernel::getGradientParams(MMatrix &g, const MMatrix &X, const MMatrix &X2,const MMatrix &covGrad,bool regularise) const
{
	assert(g.sizeRow() == 1);
	assert(g.sizeCol() == getNumParams());
	 
	size_t uStart = 0;
	size_t uEnd   = 0;

	for(size_t i = 0; i < m_Components.size(); i++)
	{
		uEnd = uStart + m_Components[i]->getNumParams() - 1;
		MMatrix subg(1,m_Components[i]->getNumParams());
		m_Components[i]->getGradientParams(subg,X,X2,covGrad,regularise);
		g.setMMatrix(0,uStart,subg);
		uStart = uEnd + 1;
	}
}

double CompoundKernel::getGradientParam(size_t index, const MMatrix &X, const MMatrix &X2, const MMatrix & covGrad) const 
{
	assert(index < m_ParamsNum);
	size_t uStart = 0;
	size_t uEnd = 0;
	for(size_t i = 0; i< m_Components.size(); i++)
	{
		uEnd = uStart + m_Components[i]->getNumParams()-1;
		if(index < uEnd)
			return m_Components[i]->getGradientParam(index - uStart, X, X2, covGrad);
		uStart = uEnd + 1;
	}
	return -1;
}

double CompoundKernel::getGradientParam(size_t index, const MMatrix& X, const MMatrix& covGrad) const
{
	assert(index < m_ParamsNum);
	size_t uStart = 0;
	size_t uEnd = 0;
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		uEnd = uStart + m_Components[i]->getNumParams()-1;
		if(index < uEnd)
			return m_Components[i]->getGradientParam(index - uStart, X, covGrad);
		uStart = uEnd + 1;
	}
	return -1;
}

void CompoundKernel::SetLog(std::ostream *pLog)
{
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->SetLog(pLog);
	}
}

void CompoundKernel::WriteParamsToLog()
{
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->WriteParamsToLog();
	}
}

void CompoundKernel::getGradientX(std::vector<MMatrix*>& gX, const MMatrix& X, bool addG) const
{
	for(size_t i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->getGradientX(gX, X, true);
	}
}
 