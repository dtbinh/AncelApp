#ifndef __AppUtility_h_
#define __AppUtiltiy_h_

#include <string>
#include <vector>
#include <Windows.h>
#include <Eigen\Eigen>
namespace AncelApp
{
	std::string loadFile(std::string filter = "*",HWND hWnd = NULL);
	std::string saveFile(std::string filter = "*",HWND hWnd = NULL);
	
	Eigen::MatrixXd loadData(std::string filename);
	
	template<typename T> 
	static T vectorOpInnerProd(const std::vector<T>& v1, const std::vector<T>& v2)
	{
		assert(v1.size() == v2.size());

		T innerProd = 0.0;
		for(std::size_t i = 0; i < v1.size(); i++)
		{
			innerProd += v1.at(i)*v2.at(i);
		}
	 	return innerProd;
	}

	template<typename T> 
	static std::vector<T> vectorOpV(const std::vector<T>& v1,const std::vector<T>& v2, T alpha = 1, T beta = 1)
	{
		assert(v1.size() == v2.size());
		std::vector<T> ret;
		for (std::size_t i = 0; i < v1.size(); i++)
 			ret.push_back(alpha*v1.at(i) + beta*v2.at(i));
 
		return ret;
	}

	template<typename T> 
	static std::vector<T> vectorOpR(const std::vector<T>& v1, T alpha = 1)
	{
 		std::vector<T> ret;
		for (std::size_t i = 0; i < v1.size(); i++)
 			ret.push_back(alpha*v1.at(i));
		return ret;
	}
};


#endif