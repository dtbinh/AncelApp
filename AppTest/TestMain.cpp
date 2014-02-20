#include <iostream>
#include "Utility.h"
#include <vector>
#include <string>
#include <fstream>
#include "ResUtility.h"

int slove(const std::string &sub, const std::string &temp)
{
	std::vector<int> v(sub.length());
	v[0] = -1;
	int j = -1,cnt = 0;
	for(std::size_t i = 1; i < sub.length(); i++)
	{
		while(j != -1 && sub[j+1] != sub[i]) j = v[j];
		if(sub[j+1] == sub[i]) j = j + 1;
		v[i] = j;
	}
	j = -1;
	for(std::size_t i = 0; i < temp.length(); i++)
	{
		while(j != -1 && sub[j+1] != temp[i]) j = v[j];
		if(sub[j+1] == temp[i]) j = j + 1;
		if(j == sub.length()-1)
		{
			cnt ++;
			j = v[j];
		}

	}
	return cnt;
}
 

int vmain(int argc, char *argv[]) 
{ 
	ResUtil::MMatrix *mat = ResUtil::loadData("TransData.dat");

	//ResUtil::MMatrix splice(mat->sizeRow()/2-30,mat->sizeCol());

	//for(std::size_t i = 0; i < splice.sizeRow(); i++)
	//{
	//	splice.copyRowRow(i, *mat, 60+ i*2);
	//}
//	splice.submat(*mat,0,584,0,mat->sizeCol());

	std::size_t row = mat->sizeRow();
	std::size_t col = mat->sizeCol();

	for(std::size_t i = 0; i < row; i++)
	{
 		double val = mat->get(i,1) - (19.1769 - 1.01965)/row*i - 1.01965;
		mat->assign(val,i,1);
	}

	
	std::ofstream writer("TransData_xx.dat",std::ios::binary|std::ios::out);
			
	writer.write((char*)(&row), sizeof(std::size_t));
	writer.write((char*)(&col), sizeof(std::size_t));
	writer.write((char*)(mat->gets()), sizeof(double)*row*col);

	writer.close();

	//Utility::WriteSkeletonToXML("actor\\16.asf");
	/*Utility::WriteSkeletonToXML("actor\\HDM_bdd.asf");
	Utility::WriteSkeletonToXML("actor\\HDM_bkk.asf");
	Utility::WriteSkeletonToXML("actor\\HDM_trr.asf");
	Utility::WriteSkeletonToXML("actor\\HDM_mmm.asf");
	Utility::WriteSkeletonToXML("actor\\HDM_dgg.asf");
	Utility::WriteSkeletonToXML("actor\\113.asf");*/
	system("PAUSE"); 
	return EXIT_SUCCESS; 
}

int main()
{
	/*ResUtil::MMatrix *mat = ResUtil::loadData("HDM_bd_01-01_01_120.dat");
	for(std::size_t  i = 0; i < mat->sizeCol(); i++)
		std::cout << mat->get(0,i) << ", ";
	std::cout << std::endl;
	system("pause");*/
	Utility::WriteSkeletonToXML("actor\\16.asf");
	//Utility::ModelLearning();
}
//int main()
//{ 
//	int testcase;
//	 
//	std::string sub;
//	std::string tep;
//	
//	std::cin >> testcase;
//	while(testcase--)
//	{
//		std::cin >> sub >> tep;
//		std::cout << slove(sub,tep) << std::endl;
//	}
// 
//
//	 
//   	//	
////	Utility::ModelLearning();
////	Utility::HMGPMLearning();
//
// 	//system("pause");
//	return 0;
//}