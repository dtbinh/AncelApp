#include "GPLVM.h"
#include <fstream>
#include <time.h>

using namespace ResModel;

CGPLVM::CGPLVM(Kernel* pKernel,MMatrix *pYin,size_t LatDim)
	:m_pKernel(pKernel),m_pYIn(pYin),m_uLatDim(LatDim),Optimisable()
{
	m_bIsInputscaleLearnt  = false;
	m_bIsLatentRegularised = true;
	m_bIsBackConstrained   = false;
	m_uDataNum = m_pYIn->sizeRow();
	m_uDataDim = m_pYIn->sizeCol();
	m_bIsKtoUpdate = false;

	InitStoreage();
	InitVals();
}
CGPLVM::CGPLVM(Kernel* pKernel,MMatrix *pYin,Kernel *pBackKernel,size_t LatDim)
	:m_pKernel(pKernel),m_pYIn(pYin),m_pBackKernel(pBackKernel),m_uLatDim(LatDim),Optimisable()
{
	m_bIsInputscaleLearnt  = false;
	m_bIsLatentRegularised = true;
	m_bIsBackConstrained   = true;
	m_uDataNum = m_pYIn->sizeRow();
	m_uDataDim = m_pYIn->sizeCol();
	m_bIsKtoUpdate = false;

	InitStoreage();
	InitVals();
}
void CGPLVM::InitVals()
{
	m_MeanY = m_pYIn->meanCol();
	 
	m_scale.oneElements();
	m_CentredY = *m_pYIn;
 	for(size_t i = 0; i < m_uDataNum; i++)
	{
		m_CentredY.axpyRowRow(i,m_MeanY,0,-1.0);
	}
 	InitXByPCA();
}
void CGPLVM::LearnModel()
{

}
void CGPLVM::InitXByPCA()
{
	//if(!m_bIsBackConstrained)
	//{
 		MMatrix YMean = m_CentredY.meanCol();
	 
		MMatrix CovM(m_CentredY.sizeCol(),m_CentredY.sizeCol());
		CovM.setSymmetric(true);
		CovM.syrk(m_CentredY, 1.0 / (double)m_CentredY.sizeRow(), 0.0, "u", "t");
 
		YMean.transpose();
		CovM.syr(YMean, -1.0, "u");
 
		MMatrix EigVals(1,m_CentredY.sizeCol());
		CovM.syev(EigVals, "v", "u");
  	 
		MMatrix UMulLambda(m_CentredY.sizeCol(),m_uLatDim);

 		for(size_t t = 0; t < m_uLatDim; t++)
		{
			UMulLambda.copyColCol(t,CovM,m_CentredY.sizeCol()- 1 - t);
			UMulLambda.scaleCol(t,1/sqrt(EigVals.get(m_CentredY.sizeCol()- 1 - t)));
		}
 
		m_pX->gemm(m_CentredY,UMulLambda,1.0,0.0,"n","n");

 		MMatrix MeanX = m_pX->meanCol();

		for(size_t i = 0; i < m_pX->sizeRow(); i++)
			m_pX->axpyRowRow(i, MeanX, 0, -1.0);
	//}
	//else 
	//{
	//	MMatrix EigVectors(*m_pBackK);
	//	MMatrix EigVals(1,m_M.sizeRow()); 
	//	EigVectors.syev(EigVals,"V","U"); 
	// 
	//	// X initialized to first few eigenvectors of bK
	//	for(size_t i = 0; i < m_uLatDim; i++)
	//	{
	//	  m_pX->copyColCol(i, EigVectors, m_M.sizeRow() - 1 - i);
	//	}
	//	std::ofstream fout("TempG.txt");
	////	EigVectors.deepCopy(*m_pBackK);

	//	// A initialized to solution of bK * A = X
	////	EigVectors.setSymmetric(true); 

	//	m_ParamA.deepCopy(*m_pX);
	////	fout << m_ParamA << std::endl;
	////	fout << EigVectors << std::endl;
	////	fout << "--------------------------------------------" << std::endl;
	//	fout << *m_pBackK  << std::endl;
	//	m_ParamA.sysv(*m_pBackK,"U");
 	//}
	if(m_bIsBackConstrained)
	{
  		m_pBackKernel->computeKernel(m_BackK,m_CentredY);
		m_BackK.setSymmetric(true);
		MMatrix Mat;
		Mat = m_BackK;
 		m_ParamA = (*m_pX);
		m_ParamA.sysv(Mat,"U");
  	}
}
void CGPLVM::InitStoreage()
{
	m_pX = new MMatrix(m_uDataNum,m_uLatDim);
 	m_K.resize(m_uDataNum,m_uDataNum);
 	m_InvK.resize(m_uDataNum,m_uDataNum);
	 
	m_CovGradient.resize(m_uDataNum,m_uDataNum);
 	m_DiagX.resize(m_uDataNum,m_uLatDim);
	m_CentredY.resize(m_uDataNum,m_uDataDim);
	m_scale.resize(1,m_uDataDim);
	m_InvKY.resize(m_uDataNum,m_uDataDim);

	for(size_t t = 0; t < m_uDataNum; t++)
	{
		m_vGX.push_back(new MMatrix(m_uDataNum,m_uLatDim));
	}
//--------------------------------------------------------------
	if(m_bIsBackConstrained)
	{
		m_BackK.resize(m_uDataNum,m_uDataNum);
		m_ParamA.resize(m_uDataNum,m_uLatDim);
  	}
}
		
void CGPLVM::UpdateX()
{
	if(m_bIsBackConstrained)
	{
		m_pX->symm(m_BackK,m_ParamA,1.0,0.0,"U","L");
 	}
	m_bIsKtoUpdate = false;
}
void CGPLVM::Update() const
{
 	if(!m_bIsKtoUpdate)
 	{
		UpdateK();
 		UpdateInvK();
  	}
    m_bIsKtoUpdate = true;
}
void CGPLVM::UpdateK() const
{	 
	for(size_t i = 0; i < m_uDataNum; i++)
	{
		m_K.assign(m_pKernel->computeDiagElement(*m_pX, i), i, i);
		for(size_t j = 0; j < i; j++)
		{
			double kVal = m_pKernel->computeElement(*m_pX, i, *m_pX, j);
			m_K.assign(kVal, i, j);
			m_K.assign(kVal, j, i);
		}
	}
	m_K.setSymmetric(true);
}
void CGPLVM::UpdateInvK() const
{
 //	m_LcholK = m_K;

//	m_LcholK.chol("U");				/// this will initially be upper triangular.
//	m_LogDetK = m_LcholK.LogDet();
//	m_InvK.setSymmetric(true);
//	m_InvK.pdinv(m_LcholK);
//	m_LcholK.transpose();
	m_InvK = m_K;
	m_LogDetK = MMatrix::invertMMatrix(m_InvK);
	m_InvKY.gemm(m_InvK,m_CentredY,1.0,0.0,"N","N");
}
	
void CGPLVM::UpdateCovGradient() const
{
 	MMatrix InvKYW;
	InvKYW = m_InvKY;

	if(m_bIsInputscaleLearnt)
	{
		for(size_t k = 0; k < m_uDataDim; k++)
		{
			InvKYW.scaleCol(k,m_scale.get(k));
		}
 	}
 	m_CovGradient = m_InvK;    // covgrad = invK   // dL/dK
	m_CovGradient.scale(m_uDataDim);
 	m_CovGradient.gemm(InvKYW,InvKYW,-1.0,1.0,"N","T");
	m_CovGradient.setSymmetric(true);
	m_CovGradient.scale(0.5);
}

double CGPLVM::LogLikelihood()  const
{
	Update();
  	
	double L = 0.0;
	 
 	L += (m_LogDetK*m_uDataDim);	// Dln|K|			
	
	for(size_t k = 0; k < m_uDataDim; k++)
	{
  		double wk2 = m_scale.get(k) * m_scale.get(k);
		L += wk2 * m_InvKY.dotColCol(k, m_CentredY, k);   // (yi')invK(yi)
 	}

 	if(m_bIsLatentRegularised)
	{
  		for(size_t t = 0; t < m_uLatDim; t++)
		{
			L += m_pX->colNorm2(t);
		}   // |xi|^2
  	}
	L *= 0.5;

  	//scale Learnt 
 	if(m_bIsInputscaleLearnt)
	{
		double DetW = 1.0;
		for(size_t k = 0; k < m_uDataDim; k++) 
		{
  			DetW *= m_scale.get(k);
		}
 		L -= (log(fabs(DetW)) * m_uDataNum);
	}
	
 	return L;
}
double CGPLVM::LogLikelihoodGradient(MMatrix& g) const
{
 	size_t uKernelNum = m_pKernel->getNumParams();
	size_t uParamNum = uKernelNum;
	size_t uXoffset = uParamNum;
 
	g.zeroElements();
	//---------------------
	//------------------
	Update();

	m_pKernel->getGradientX(m_vGX,*m_pX,*m_pX);
	m_pKernel->getDiagGradientX(m_DiagX,*m_pX);
	
 	for(size_t i = 0; i < m_uDataNum; i++)
	{
 		m_vGX[i]->scale(2.0);
		for(size_t j = 0; j < m_uLatDim; j++)
		{
			m_vGX[i]->assign(m_DiagX.get(i,j),i,j);
		}
	}
 	
	MMatrix InvKY(m_uDataNum, 1);
	MMatrix tempG(1,uKernelNum);
	MMatrix tempGX(m_uDataNum,m_uLatDim);
	
	tempGX.zeroElements();
	
	UpdateCovGradient();

	m_pKernel->getGradTransParams(tempG, *m_pX, m_CovGradient, true);
	
	for(size_t i = 0; i < uKernelNum; i++)
	{
		g.add(tempG.get(i), i);
	}
	// DL/DX
	for(size_t i = 0; i < m_uDataNum; i++)
	{
		for(size_t k = 0; k < m_uLatDim; k++)
		{
			int index = uXoffset + i + m_uDataNum * k;
			if(!m_bIsBackConstrained)
				g.assign(m_vGX[i]->dotColCol(k, m_CovGradient, i),index);
			else 
				tempGX.assign(m_vGX[i]->dotColCol(k,m_CovGradient,i),index - uXoffset);
		}
	}

	if(m_bIsLatentRegularised)
	{
		for(size_t i = 0; i < m_uDataNum; i++)
		{
			for(size_t k = 0; k < m_uLatDim; k++)
			{
	 			int index = uXoffset + i + m_uDataNum * k;
				if(!m_bIsBackConstrained)
					g.add(m_pX->get(i, k), index);
				else
					tempGX.add(m_pX->get(i, k),index - uXoffset);
	        } 
		}
	}
	if(m_bIsInputscaleLearnt)
	{
		for(size_t k = 0; k < m_uDataDim; k++)
		{
			int index = uParamNum + m_uDataNum * m_uLatDim + k;
			double tempw = m_scale.get(k)*m_InvKY.dotColCol(k,m_CentredY,k);
			tempw -= m_uDataNum / m_scale.get(k);
			g.assign(tempw,index);
		}
	}
	if(m_bIsBackConstrained)
	{
		MMatrix temp(m_uDataNum,m_uLatDim);
		temp.gemm(m_BackK,tempGX,1.0,0.0,"N","N");
		for(size_t i = 0; i < m_uDataNum; i++)
		{
			for(size_t k = 0; k < m_uLatDim; k++)
			{
				int index = uXoffset + i + m_uDataNum * k;
				g.assign(temp.get(i,k),index);
			}
		}
	}
	return  LogLikelihood();
}

void CGPLVM::Optimize(size_t iter)
{
	SetminIters(iter);
	time_t t = clock();
	RunDefaultOptimiser(); 
	time_t e = clock();
	std::cout << "Iter Time : " << e - t << std::endl;
	m_pKernel->WriteParamsToLog();
	if(m_bIsBackConstrained)
		m_pBackKernel->WriteParamsToLog();
	std::ofstream  fout("Back.txt");
	fout << m_ParamA << std::endl;
	fout << m_BackK  << std::endl;
}
void CGPLVM::getOptiParams(MMatrix& param) const
{
	size_t uCn_t = 0;
	for(size_t i = 0; i < m_pKernel->getNumParams(); i++)
	{	  
		param.assign(m_pKernel->getTransParam(i),uCn_t);
 		uCn_t++;
	}
//----------------------------------------------------------------------
	for(size_t j = 0; j < m_uLatDim; j++)
	{
		for(size_t i = 0; i < m_uDataNum; i++)
		{
			if(!m_bIsBackConstrained)
 				param.assign(m_pX->get(i, j),uCn_t++);
			else 
				param.assign(m_ParamA.get(i, j),uCn_t++);
		}
	}
//-----------------------------------------------------------------------
	if(m_bIsInputscaleLearnt)
	{
		for(size_t j = 0; j <  m_uDataDim; j++)
		{
			param.assign(m_scale.get(j),uCn_t++);
 		}
	}
}
void CGPLVM::setOptiParams(const MMatrix& param)
{
 	m_bIsKtoUpdate = false;
	size_t uCn_t = 0;
	for(size_t i = 0; i < m_pKernel->getNumParams(); i++)
	{	  
		m_pKernel->setTransParam(param.get(uCn_t), i);
		uCn_t++;
	}
//--------------------------------------------------------------------------	
	for(size_t j = 0; j < m_uLatDim; j++)
	{
		for(size_t i = 0; i< m_uDataNum; i++)
		{
			if(!m_bIsBackConstrained)
				m_pX->assign(param.get(uCn_t++), i, j);
 			else
				m_ParamA.assign(param.get(uCn_t++), i, j);
		}
	}
//-------------------------------------------------------------------------
	if(m_bIsInputscaleLearnt)
	{
		for(size_t j = 0; j <  m_uDataDim; j++)
		{
			m_scale.assign(param.get(uCn_t++),j);
 		}
	}
	//if(m_bIsBackConstrained)
	//{
	//	size_t uParamsNum = m_pBackKernel->getNumParams();
	//	for(size_t i = 0; i < uParamsNum; i ++)
	//	{
	//		std::cout << "p:  " << param.get(uCn_t) << std::endl;
	//		m_pBackKernel->SetTransParam(param.get(uCn_t),i);
	//		uCn_t++;
	//	}
	//	m_pBackKernel->computeKernel(m_BackK,m_CentredY);
	//}
 	UpdateX();  
}
size_t CGPLVM::getOptiParamsNum()  const
{
	size_t uTotal = m_pKernel->getNumParams() + m_uDataNum * m_uLatDim;
  	if(m_bIsInputscaleLearnt)
		uTotal += m_uDataDim;
	return uTotal;
}
MMatrix* CGPLVM::GetDataSpaceDist() const
{
	MMatrix *pDistMat = new MMatrix(m_uDataNum,m_uDataNum);

	for(size_t i = 0; i < m_uDataNum; i++)
	{
		pDistMat->assign(0,i,i);
		for(size_t j = 0; j < i; j++)
		{
			pDistMat->assign(m_CentredY.rowDist2(i,m_CentredY,j),i,j);
			pDistMat->assign(m_CentredY.rowDist2(i,m_CentredY,j),j,i);
		}
	}
	return pDistMat;
}
double CGPLVM::computeObjectiveVal()   
{
	return  LogLikelihood();
}
double CGPLVM::computeObjectiveGradParams(MMatrix & g)  
{
	return  LogLikelihoodGradient(g);
}
void CGPLVM::SetInputscaleLearnt(bool val)
{
	m_bIsInputscaleLearnt = val;
}
void CGPLVM::SetLatentVals(MMatrix *pX)
{
	m_pX = pX;
}
MMatrix* CGPLVM::GetLatentVals() const
{
	return m_pX;
}
