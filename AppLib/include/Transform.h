#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <string>

namespace ResCore
{
	const double limVal = 36;
	
	enum TRANSTYPE 
	{
		TRANS_EXP,
		TRANS_NEGLOGLOGIT,
		TRANS_LINEAR,
		TRANS_SIGMOID
	};
	//-----------------------------------base class---------------------------------------------
	class CTransform
	{
	public:
		CTransform(){};
		virtual ~CTransform() {};
		CTransform(const CTransform & rhs)
			:m_emType(rhs.m_emType) {};
		virtual CTransform* Clone()   const = 0;
		virtual double AtoX(double a) const = 0;
		virtual double XtoA(double x) const = 0;
		virtual double Gradient(double x) const = 0;

		virtual void SetType(const TRANSTYPE &emType){
			m_emType = emType;
		};
		virtual TRANSTYPE getType() const{
			return m_emType;
		};

	   static CTransform* DefaultPositive();
	   static CTransform* DefaultZeroOne();
	   static CTransform* GetNewTransformPointer(TRANSTYPE tsType);

	protected:
   	    const static double EPS;
 	private:
		TRANSTYPE m_emType;				//	±‰ªª¿‡–Õ
	};

	//--------------------------------------Exp Transform-------------------------------------
	class CExpTransform : public CTransform  
	{
	public:
		CExpTransform();
		CExpTransform(const CExpTransform & rhs)
			:CTransform(rhs)/*,m_bTransformed(rhs.m_bTransformed)*/{};
		 CTransform* Clone() const { return new CExpTransform(*this);}
		double AtoX(double a) const;
		double XtoA(double x) const;
		double Gradient(double x) const;
 	};

	//------------------------Negative logarithm logit Transform------------------------------
	class CNegLogLogitTransform : public CTransform  
	{
	public:
		CNegLogLogitTransform();
		CNegLogLogitTransform(const CNegLogLogitTransform & rhs)
			:CTransform(rhs)/*,m_bTransformed(rhs.m_bTransformed)*/{};
		CTransform* Clone() const { return new CNegLogLogitTransform(*this);}
		double AtoX(double a) const;
		double XtoA(double x) const;
		double Gradient(double x) const;
 	};

	//-----------------------Linear Transform-------------------------------------------------
	class CLinearTransform : public CTransform  
	{
	public:
		CLinearTransform();
		CLinearTransform(const CLinearTransform & rhs)
			:CTransform(rhs)/*,m_bTransformed(rhs.m_bTransformed)*/{};
		CTransform* Clone() const { return new CLinearTransform(*this);}

		double AtoX(double a) const;
		double XtoA(double x) const;
		double Gradient(double x) const;
		double getM() const;
		void SetValM(const double M);
		double getC() const;
		void SetValC(const double C);

	protected:
		double m_dvalM;
		double m_dvalC;
	};

	//---------------------Sigmoid Transform---------------------------------------------------
	class CSigmoidTransform : public CTransform  
	{
	public:
		CSigmoidTransform();
		CSigmoidTransform(const CSigmoidTransform & rhs)
			:CTransform(rhs)/*,m_bTransformed(rhs.m_bTransformed)*/{};
		CTransform* Clone() const { return new CSigmoidTransform(*this);}
		double AtoX(double a) const;
		double XtoA(double x) const;
		double Gradient(double x) const;
	};
	//----------------------------------------------------------------------------------------
}
#endif