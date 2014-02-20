#ifndef PARAMPRIORS_H
#define PARAMPRIORS_H

#include <vector>
#include "Distribution.h"

namespace ResCore
{
	class ParamPriors
	{
	public:
		void		    addDist(Distribution* dist);
		void		    clearDist();
		std::string     getDistType(size_t index)  const;
 		size_t			getDistNum() const;
		Distribution*   getDist(size_t index) const;
	protected:
	 	std::vector<Distribution*> m_vDists;
 	};
	inline void	 ParamPriors::addDist(Distribution* dist)
	{
		m_vDists.push_back(dist);
  	}
	inline void	 ParamPriors::clearDist()
	{
		m_vDists.clear();
 	}
	inline std::string  ParamPriors::getDistType(size_t index) const
	{
		return m_vDists[index]->getType();
	}
 	inline size_t ParamPriors::getDistNum() const
	{
		return m_vDists.size();
	}
	inline Distribution* ParamPriors::getDist(size_t index) const
	{
		return m_vDists[index];
	}
};
#endif