#include "Utility.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <ResUtility.h>
#include <MGPM.h>
#include <HMGPM.h>
#include <ctime>

//#define _DebugInfo

bool loadConfigs(const std::string &fileName,
				 std::vector<ResUtil::LoadOptions>&opts,
				 std::vector<std::vector<std::size_t>> &segmentLabel, 
				 std::vector<std::string> &acotrLs,
				 std::vector<std::string> &actionLs,
				 std::vector<std::string> &motionNameLs)
{
	std::fstream loader(fileName);
	if(loader.fail()) return false;
	
	std::string strContent;
	std::getline(loader,strContent);
	std::stringstream filter(strContent);

	std::size_t numFiles;
	filter >> numFiles;

	opts.resize(numFiles);
	segmentLabel.resize(numFiles);

	for(std::size_t i = 0; i < numFiles; i++)
	{
		std::getline(loader,opts[i].fileName);
 		
		opts[i].fileName = "data\\" + opts[i].fileName;
		
		std::getline(loader,strContent);
		std::stringstream filter;

		filter.str(strContent);
		filter >> opts[i].startFrames;
		filter >> opts[i].skipFrames;
		filter >> opts[i].endFrames;

		std::size_t numLabels;
		std::getline(loader,strContent);
		filter.clear();
		filter.str(strContent);
	 	 
		filter >> numLabels;
		segmentLabel[i].resize(numLabels);
		
		for(std::size_t t = 0;  t < numLabels; t++)
		{
			filter >> segmentLabel[i][t];
		}
		std::getline(loader,strContent);
		filter.clear();
		filter.str(strContent);
 	 	filter >> opts[i].numCycles;
	}
 	for (std::size_t i = 0; i < numFiles; i++)
	{
		std::getline(loader,strContent);
		motionNameLs.push_back(strContent);
	}

	std::getline(loader,strContent);
	filter.clear();
	filter.str(strContent);
	
	std::size_t numActor,numAction;

	//actior ls
 	filter >> numActor;
	for (std::size_t i = 0; i < numActor; i++)
	{
		std::getline(loader,strContent);
		acotrLs.push_back(strContent);
	}
	
	std::getline(loader,strContent);
	filter.clear();
	filter.str(strContent);

	//action ls
	filter >> numAction;
	for (std::size_t i = 0; i < numAction; i++)
	{
		std::getline(loader,strContent);
		actionLs.push_back(strContent);
	}

	return true;
}
bool Utility::ModelLearning()
{
	std::string modelFileName = "stylewalking_small.mm";//"test_Model_x.mm";

	std::vector<std::string> actorLs;
	std::vector<std::string> actionLs;
	std::vector<std::string> motionNameLs;
	std::vector<ResUtil::LoadOptions> opts;
	std::vector<std::vector<std::size_t>> segmentLabel;

	modelFileName = "data\\" + modelFileName;
	if(!loadConfigs(modelFileName,opts,segmentLabel,actorLs,actionLs,motionNameLs))
		return false;

	ResUtil::MocapData mocapData = ResUtil::loadMocapData(opts);
	
	mocapData.actorLs  = actorLs;
	mocapData.actionLs = actionLs;
	mocapData.segmentLabel = segmentLabel;
	mocapData.modelName = "stylewalking_small.mm";
	mocapData.mMotionNameLs = motionNameLs;

#ifdef _DebugInfo	
	std::ofstream fv("txt.txt");
	std::streambuf *oldbuf  = std::cout.rdbuf(fv.rdbuf());
//	std::cout << mocapData.Y << std::endl;
#endif

	using ResModel::MGPModel;
	 
	MGPModel model(mocapData);

	//for(std::size_t i = 0; i < 100; i++)
	//{
		model.optimize(500);

		time_t t;
  		tm ptm;
		time(&t);
		localtime_s(&ptm,&t);
		std::stringstream ss;
		ss << ptm.tm_year + 1900 << '_' << ptm.tm_mon + 1 << '_' << ptm.tm_mday << '_' 
		   << ptm.tm_hour << '_' << ptm.tm_min << '_' << ptm.tm_sec;

		std::string outputFileName = modelFileName.substr(0,modelFileName.find_last_of('.')) + ss.str() + ".MGPM";
		model.writeModelToFile(outputFileName);
	//}

#ifdef _DebugInfo
	std::cout.rdbuf(oldbuf);
#endif

	return true;
}

bool Utility::HMGPMLearning()
{
	std::string modelFileName = "stylewalking.mm";

	std::vector<std::string> actorLs;
	std::vector<std::string> actionLs;
	std::vector<ResUtil::LoadOptions> opts;
	std::vector<std::string> motionNameLs;
	std::vector<std::vector<std::size_t>> segmentLabel;

	modelFileName = "data\\trainingdata\\" + modelFileName;
	if(!loadConfigs(modelFileName,opts,segmentLabel,actorLs,actionLs,motionNameLs))
		return false;

	ResUtil::MocapData mocapData = ResUtil::loadMocapData(opts);
	
	mocapData.actorLs  = actorLs;
	mocapData.actionLs = actionLs;
	mocapData.segmentLabel = segmentLabel;

#ifdef _DebugInfo	
	std::ofstream fv("txt.txt");
	std::streambuf *oldbuf  = std::cout.rdbuf(fv.rdbuf());
	//std::cout << mocapData.Y << std::endl;
#endif

	using ResModel::HMGPModel;
	 
 	HMGPModel model(mocapData);

	//for(std::size_t i = 0; i < 100; i++)
	//{
		model.optimize(15550);

		time_t t;
  		tm ptm;
		time(&t);
		localtime_s(&ptm,&t);
		std::stringstream ss;
		ss << ptm.tm_year + 1900 << '_' << ptm.tm_mon + 1 << '_' << ptm.tm_mday << '_' 
		   << ptm.tm_hour << '_' << ptm.tm_min << '_' << ptm.tm_sec;

		std::string outputFileName = modelFileName.substr(0,modelFileName.find_last_of('.')) + ss.str() + ".HMGPM";
		model.writeModelToFile(outputFileName);
//	}

#ifdef _DebugInfo
	std::cout.rdbuf(oldbuf);
#endif

	return true;
}