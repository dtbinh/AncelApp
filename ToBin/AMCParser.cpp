#include <fstream>
#include <sstream>
 
#include "AMCParser.h"


const std::vector<DOF> AMCParser::mDefaultBoneDOF = AMCParser::DOFInitializer::Init();


std::vector<DOF> AMCParser::DOFInitializer::Init()
{
	std::vector<DOF> defaultDOF;

	defaultDOF.push_back(DOF(true,true,true,6,true,true,true));				  //root
	defaultDOF.push_back(DOF(false,false,false,0));	      //lhipjoint
	defaultDOF.push_back(DOF(true,true,true,3));		      //lfemur
	defaultDOF.push_back(DOF(true,false,false,1));		  //ltibia
	defaultDOF.push_back(DOF(true,false,true,2));		  //lfoot
	defaultDOF.push_back(DOF(true,false,false,1));		  //ltoes
	defaultDOF.push_back(DOF(false,false,false,0));	      //rhipjoint
	defaultDOF.push_back(DOF(true,true,true,3));		      //rfemur
	defaultDOF.push_back(DOF(true,false,false,1));		  //rtibia
	defaultDOF.push_back(DOF(true,false,true,2));		  //rfoot
	defaultDOF.push_back(DOF(true,false,false,1));		  //rtoes
	defaultDOF.push_back(DOF(true,true,true,3));			  //lowerback	
	defaultDOF.push_back(DOF(true,true,true,3));			  //upperback
	defaultDOF.push_back(DOF(true,true,true,3));			  //thorax
	defaultDOF.push_back(DOF(true,true,true,3));			  //lowerneck
	defaultDOF.push_back(DOF(true,true,true,3));			  //upperneck
	defaultDOF.push_back(DOF(true,true,true,3));			  //head
	defaultDOF.push_back(DOF(false,true,true,2));		  //lclavicle
	defaultDOF.push_back(DOF(true,true,true,3));			  //lhumerus
	defaultDOF.push_back(DOF(true,false,false,1));		  //lradius
	defaultDOF.push_back(DOF(false,true,false,1));		  //lwrist
	defaultDOF.push_back(DOF(true,false,true,2));		  //lhand
	defaultDOF.push_back(DOF(true,false,false,1));		  //lfingers
	defaultDOF.push_back(DOF(true,false,true,2));		  //lthumb
	defaultDOF.push_back(DOF(false,true,true,2));		  //rclavicle
	defaultDOF.push_back(DOF(true,true,true,3));			  //rhumerus
	defaultDOF.push_back(DOF(true,false,false,1));		  //rradius
	defaultDOF.push_back(DOF(false,true,false,1));		  //rwrist
	defaultDOF.push_back(DOF(true,false,true,2));		  //rhand
	defaultDOF.push_back(DOF(true,false,false,1));		  //rfingers
	defaultDOF.push_back(DOF(true,false,true,2));		  //rthumb
	
	return defaultDOF;
}

AMCParser::AMCParser()
	:mBoneDOF(AMCParser::mDefaultBoneDOF)
{
	mTotalDOF = 0;
	for(size_t i = 0; i < mBoneDOF.size();i++)
		mTotalDOF += mBoneDOF[i].mNumFreedom;
	 
}
AMCParser::AMCParser(std::vector<DOF> BoneDOF)
	:mBoneDOF(BoneDOF)
{
	mTotalDOF = 0;
	for(size_t i = 0; i < mBoneDOF.size();i++)
		mTotalDOF += mBoneDOF[i].mNumFreedom;
}
AMCParser::~AMCParser()
{

}
bool AMCParser::loadMocapData(std::string  file)
{
	std::ifstream loader(file);
	if(loader.fail())
		return false;
	std::string strContent;
	
	while(strContent != "1")
		std::getline(loader,strContent);
 
	while(!loader.eof())
	{
		int curDOF = 0;
		std::vector<double> pose(mTotalDOF);
		std::getline(loader,strContent);
 
		while(strContent.length() && (strContent[0] <= '0' || strContent[0] >'9'))
		{
			std::stringstream filter(strContent);
			filter >> strContent;
			while(!filter.eof())			
				filter >> pose[curDOF++];
			std::getline(loader,strContent);
		}
		mRawData.push_back(pose);
	}
	loader.close();
	return true;
}

void AMCParser::writeToFile(std::string file)
{
 	std::ofstream writer(file,std::ios::binary|std::ios::out);

	size_t row = mRawData.size();
	size_t col = mTotalDOF-16;
	writer.write((char*)(&row),sizeof(std::size_t));
	writer.write((char*)(&col),sizeof(std::size_t));

	static std::size_t mask[] = {24,25,31,32,33,34,35,36,37,43,44,45,46,47,54,61};
  
	std::size_t t = 0;
	for(size_t j = 0; j < mTotalDOF; j++)
	{
		if(j > mask[t]) t++;
		if(j == mask[t]) continue;
		for(size_t i = 0; i < mRawData.size(); i++)
		{
	 			writer.write((char*)(&mRawData[i][j]),sizeof(double));
		}
	}
 	writer.close();
}