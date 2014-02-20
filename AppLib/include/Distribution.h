#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif 

#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include "Transformable.h"

namespace ResCore
{
	using namespace ResUtil;
  
	class Distribution : public Transformable
	{
	public:
		Distribution();
		~Distribution();
	 
		virtual size_t  getNumParams() const;
		virtual double	getParam(size_t index)  const;
		virtual void	setParam(double val, size_t index);
		virtual void	getGradientParams(MMatrix& g) const {};
	 
		virtual double  getGradInput(double x) const = 0;
		virtual void    getGradInput(MMatrix& g, const MMatrix& x);
 
		virtual double  logProb(double val) const = 0;
		virtual double  logProb(const MMatrix& x) const;
 	 
		virtual std::string getParamName(size_t index) const;
				
		std::string getType() const;
		std::string getName() const;
		std::string getBaseType() const;

		void setName(const std::string& strName);
 
	protected:
		virtual void initDist() = 0;
	protected:
		size_t mNumParams;
		
		std::string mDistName;
		std::string mDistType;

		typedef std::pair<std::string,double> PARAMS;
 		std::vector<PARAMS> mhps;  //hyper paramaters
 	};

	 
	inline size_t Distribution::getNumParams() const
	{
		return mhps.size();
	}
	inline std::string Distribution::getBaseType() const
	{
		return "BaseDist";
	}
 	inline std::string Distribution::getParamName(size_t index) const
	{
		assert(index < mhps.size());
		return mhps[index].first;
 	}
 	inline	std::string Distribution::getType() const
	{
		return mDistType;
	}
 	inline	void Distribution::setName(const std::string& strName)
	{
		mDistName = strName;
	}
	inline 	std::string Distribution::getName() const
	{
		return mDistName;
	}
	inline double Distribution::getParam(size_t index)  const
	{
		return mhps[index].second;
	}
	inline void	  Distribution::setParam(double val, size_t index)
	{
		mhps[index].second = val;
	}
};

#endif