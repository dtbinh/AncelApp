/****************************************************************************************
*
*
*/
#ifndef __ResUtility_h
#define __ResUtility_h

#include "MMatrix.h"
#include <string>
#include <fstream>
#include <algorithm>

namespace ResUtil
{
//	static vector<bool> mask;
//	static vector<int>  dims;
//	static vector<int>  locations;
	
	MMatrix* loadData(const std::string &fileName,int start = 0,int step = 1,int end = -1);
	 
	//const value PI
	static double PI = 3.141592653589;
	
	//load Options.............
	struct LoadOptions
	{
 		int startFrames;
		int endFrames;
		int skipFrames;

		double numCycles;
		std::string fileName;
	};

	struct MocapData
	{
		ResUtil::MMatrix Y;
		ResUtil::MMatrix initY;
		
		std::vector<size_t> segments;
		std::vector<std::vector<size_t>>segmentLabel;
 
		std::vector<double> numCycles;

		std::vector<string> actorLs;
		std::vector<string> actionLs;
		std::string         modelName;
		std::vector<string> mMotionNameLs;
	};
	
	void mocapToExpMap(ResUtil::MMatrix& data);
	void mocapToEulerAngle(ResUtil::MMatrix& data);
	MMatrix* loadRawData(const std::string &fileName,int start = 0,int step = 1,int end = -1);
	MocapData loadMocapData(std::vector<LoadOptions> options);
}
#endif
