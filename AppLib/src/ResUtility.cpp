#include "ResUtility.h"
#include <fstream>
#include <algorithm>
#include "Quaternion.h"

using namespace ResUtil;

//template <typename T>
//int len(T & ary)
//{
//	return sizeof(ary)/sizeof(ary[0]);
//}
//
//static int dims[] = {6,3,3,3,3,3,3,2,3,1,1,2,1,2,2,3,1,1,2,1,2,3,1,2,1,3,1,2,1};
//static int mask[] = {25,26,32,33,34,35,38,44,45,46,47,48,55,62};
//static int locations[] = {1,7,10,13,16,19,22,25,27,30,31,32,34,35,37,39,42,43,44,46,47,49,52,53,55,56,59,60,62};
//static string boneName[] = {"root","lowerback","upperback","thorax","lowerneck","upperneck","head","rclavicle","rhumerus"
//							"rradius","rwrist","rhand","rfingers","rthumb","lclavicle","lhumerus","lradius","lwrist",
//							"lhand","lfingers","lthumb","rfemur","rtibia","rfoot","rtoes","lfemur","ltibia","lfoot","ltoes"};
/**
* first make a solution for this problem,then consider the others. ok ?
*/
//mask = [25:26 32:33 34 35:38 44:45 46 47:48 55 62];
//static int mask[] = {24,25,31,32,33,34,35,36,37,43,44,45,46,47,54,61};
//static int locations[] = {1,7,10,13,16,19,22,25,28,29,30,32,33,35,37,40,41,42,44,45,47,50,51,53,54,57,58,60};

//dims =      [6 3 3  3  3  3  3  2  3  1  1  2  1  2  2  3  1  1  2  1  2  3  1  2  1  3  1  2 ];
//locations = [1 7 10 13 16 19 22    27 30 31             39 42 43          49 52 53    56 59 60];
//mask =      [                                                                                 ];
//								   -2                   -7                -5          -1
//       1 2 3 4 5 6 7 8 9 1011121314151617181920212223242526272829
//dims =[6 3 3 3 3 3 3   3 1 1         3 1 1       3 1 2   3 1 2  ];
//static int mask[] = {24,25,31,32,33,34,35,36,37,43,44,45,46,47,54,61}; 


MMatrix* ResUtil::loadData(const std::string &fileName,int start,int step ,int end)
{
	std::ifstream loader(fileName,std::ios::binary|std::ios::in);
 
	if(loader.fail()) return NULL;

	std::size_t row,col;
	
	loader.read((char*)(&row),sizeof(std::size_t));
	loader.read((char*)(&col),sizeof(std::size_t));
 
	ResUtil::MMatrix *rawData = new ResUtil::MMatrix(row,col);
	double *val = rawData->gets();
	loader.read((char*)(val),row * col * sizeof(double));
	return rawData;
}

void ResUtil::mocapToExpMap(ResUtil::MMatrix& data)
{
 	static const std::size_t NUMBONE = 21; 
   	static int DOF[NUMBONE] = {3,3,3,3,3,3,3,3,1,1,3,1,1,3,1,2,3,1,2};

 	std::size_t currentIndex = 3;

	for(std::size_t i = 0; i < NUMBONE; i++)
	{
		if(DOF[i] == 3)
		{
 			for(std::size_t j = 0; j < data.sizeRow(); j++)
			{
				EulerAngle er;
				er.x = data.get(j, currentIndex);
				er.y = data.get(j, currentIndex + 1);
				er.z = data.get(j, currentIndex + 2);

				CQuaternion Q = ResUtil::EAToQuaternion(er);
				ExpontialMap em = Q.ToExpontialMap();

				data.assign(em.x,j, currentIndex);
				data.assign(em.y,j, currentIndex + 1);
				data.assign(em.z,j, currentIndex + 2);
 			}
		}
		currentIndex += DOF[i];
  	}
}

void ResUtil::mocapToEulerAngle(ResUtil::MMatrix& data)
{
	static const std::size_t NUMBONE = 19; 
    //static std::size_t loc[] = {3,6,9,12,15,18,21,24,27,28,29,32,33,34,37,38,40,43,44,46};
	
	static int DOF[NUMBONE] = {3,3,3,3,3,3,3,3,1,1,3,1,1,3,1,2,3,1,2};
	
	std::size_t currentIndex = 3;
	
	for (std::size_t i = 0; i < NUMBONE; i++)
	{
		if(DOF[i] == 3)
		{
 			for(std::size_t j = 0; j < data.sizeRow(); j++)
			{
				ExpontialMap em;
				em.x = data.get(j, currentIndex);
				em.y = data.get(j, currentIndex + 1);
				em.z = data.get(j, currentIndex + 2);

				CQuaternion Q = ResUtil::EMToQuaternion(em);
				EulerAngle er = Q.ToEulerAngle();

				data.assign(er.x, j, currentIndex);
				data.assign(er.y, j, currentIndex + 1);
				data.assign(er.z, j, currentIndex + 2);
 			}
			
		}
		currentIndex += DOF[i];
 	}
}

MMatrix* ResUtil::loadRawData(const std::string &fileName,int start,int step,int end)
{
 	std::ifstream loader(fileName,std::ios::binary|std::ios::in);
 
	if(loader.fail()) return NULL;

	std::size_t row,col;
	
	loader.read((char*)(&row),sizeof(std::size_t));
	loader.read((char*)(&col),sizeof(std::size_t));
 
	ResUtil::MMatrix *rawData = new ResUtil::MMatrix(row,col);
	double *val = rawData->gets();
	loader.read((char*)(val),row * col * sizeof(double));
	
	assert(end < int(row));
	assert(start >= 0 && start < int(row));

	std::size_t temp = 0;
	if(end == -1)
		temp = (row - start)/step + (((row - start)%step == 0) ? 0 : 1);
	else 
		temp = (end - start)/step + (((end - start)%step == 0) ? 0 : 1);

	ResUtil::MMatrix *data = new ResUtil::MMatrix(temp,col);

	for(size_t i = 0; i < temp; i++)
	{
		data->copyRowRow(i,*rawData,start+i*step);
	}

#ifdef _DebugInfo
	//--------------------------------------------------------------------------------

	for(int i = data->sizeRow() - 1; i >=0; i--)
	{
		data->assign(data->get(i,0) - data->get(0,0),i,0);
		data->assign(data->get(i,2) - data->get(0,2),i,2);
	}
	std::string newFileName =  fileName.substr(0,fileName.find_last_of('.')) + "_x.dat";

	std::ofstream fout(newFileName,ios::binary|ios::out);
	
	fout.write((char*)(&temp), sizeof(std::size_t));
	fout.write((char*)(&col),  sizeof(std::size_t));
	fout.write((char*)(data->gets()),sizeof(double) * data->getElemNum());
	////--------------------------------------------------------------------------------
#endif

	for(size_t i = 3; i < col; i++)
	{
		data->scaleCol(i, PI/180.0);
 	}
 	return data; 
}

MocapData ResUtil::loadMocapData(vector<LoadOptions> options)
{
	MocapData mocapData;
 
	std::vector<ResUtil::MMatrix*> velY(options.size());
 	std::vector<ResUtil::MMatrix*> rawData(options.size());

	for(size_t i = 0; i < options.size(); i++)
	{
		rawData[i] = loadRawData(options[i].fileName, options[i].startFrames, options[i].skipFrames, options[i].endFrames);
		assert(rawData[i] != NULL);
		mocapToExpMap(*rawData[i]);
			 
		if(i == 0)
			mocapData.segments.push_back(0);
		else 
			mocapData.segments.push_back(mocapData.segments[i-1] + rawData[i-1]->sizeRow());

		velY[i] = new ResUtil::MMatrix(rawData[i]->sizeRow(),rawData[i]->sizeCol());
		for(std::size_t j = 1; j < rawData[i]->sizeRow();j++)
		{
			velY[i]->copyRowRow(j - 1, *rawData[i], j);
 		}
		velY[i]->axpy(*rawData[i], -1.0);
		velY[i]->copyRowRow(rawData[i]->sizeRow() - 1, *velY[i], rawData[i]->sizeRow() - 2);
	}
	 
	mocapData.Y.resize(mocapData.segments[options.size()-1]+rawData[options.size()-1]->sizeRow(),rawData[0]->sizeCol() * 2 - 3);

	mocapData.initY.resize(options.size(),rawData[0]->sizeCol());
	for(size_t i = 0; i < options.size(); i++)
	{
		mocapData.numCycles.push_back(options[i].numCycles);
		mocapData.initY.copyRowRow(i, *rawData[i], 0);
		mocapData.initY.assign(rawData[i]->get(0,1), i, 1);
		mocapData.Y.copyMMatrix(mocapData.segments[i],0,*velY[i],0,velY[i]->sizeRow(), 0, 3);
		mocapData.Y.copyMMatrix(mocapData.segments[i],3,*rawData[i],0,rawData[i]->sizeRow(),3,rawData[i]->sizeCol());
		mocapData.Y.copyMMatrix(mocapData.segments[i],rawData[i]->sizeCol(),*velY[i],0,velY[i]->sizeRow(),3,velY[i]->sizeCol());
	}
	for(std::size_t i = 0;i < options.size(); i++)
	{
		delete rawData[i];
		delete velY[i];
	}
 	return mocapData;
}