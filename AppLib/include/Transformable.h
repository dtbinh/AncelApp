#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H

#include "MMatrix.h"
#include "Transform.h"
#include <cassert>

namespace ResCore
{
	using namespace ResUtil;

	class Transformable
	{
	public:
		virtual ~Transformable();
						
		virtual void	getGradientParams(MMatrix& g) const = 0;		      //对参数的梯度
 
		virtual void	setParam(double val, size_t index) = 0;		      // 单个参数
		virtual double  getParam(size_t index)        const = 0;			

		virtual size_t getNumParams() const = 0;

		virtual void	setTransParam(double val,size_t index);           //单个变换参数
		virtual double	getTransParam(size_t index) const;

		virtual void	setParams(MMatrix& matParams);					  //所有参数
		virtual void	getParams(MMatrix& matParams) const;

 		virtual void	getTransParams(MMatrix& matTransParams) const;     //所有变换
	  	virtual void	setTransParams(const MMatrix& matTransParam);

		virtual void	getGradientTransParams(MMatrix& matGrad) const;	  //对变换参数的梯度
		
	    void			clearTransforms();
	 	size_t			getNumTransform() const;						  // 变换的个数

	 	TRANSTYPE		getTransformType(size_t index) const;    
	  	double			getGradientTransform(double val, size_t index) const;
	
		void			addTransform(CTransform* trans, size_t index);

		CTransform*     getTransform(size_t unIndex) const;
	private:
		std::vector<CTransform*> m_vTransform;							 // 存储变换类型
 	};
	//----------------------------------------------inline function ------------------------------
	inline Transformable::~Transformable()
	{
		clearTransforms();
	}
	inline void	Transformable::clearTransforms()
	{
		size_t uParamsNum = m_vTransform.size();
	 	for(size_t t = 0; t < uParamsNum; t++) 
			delete m_vTransform[t];

		m_vTransform.clear();
 	}
	inline size_t Transformable::getNumTransform() const
	{
		return m_vTransform.size();
	}
	inline TRANSTYPE Transformable::getTransformType(size_t index) const
	{
		return m_vTransform[index]->getType();
	}
	inline double Transformable::getGradientTransform(double val, size_t index) const
	{
		return m_vTransform[index]->Gradient(val);
	}
	inline void	Transformable::addTransform(CTransform* trans, size_t index)
	{
		assert(index < getNumParams());
  		m_vTransform.push_back(trans);
  	}
	inline CTransform* Transformable::getTransform(size_t index) const
	{
		return m_vTransform[index];
	}
};
#endif