#include <fstream>
#include <sstream>

#include "AMCParser.h"


using namespace ResUtil;

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
MMatrix* AMCParser::loadMocapData(std::string filePath,AMC_OPTIONS opts)
{
	std::ifstream loader(filePath);
	if(loader.fail())
		return false;
	std::string strContent;
	while(strContent != "1")
		std::getline(loader,strContent);

	std::vector<std::vector<float>> RawData;
 	while(!loader.eof())
	{
		int curDOF = 0;
		std::vector<float> pose(mTotalDOF);
		std::getline(loader,strContent);
 
		while(strContent.length() && (strContent[0] <= '0' || strContent[0] >'9'))
		{
			std::stringstream filter(strContent);
			filter >> strContent;
			while(!filter.eof())			
				filter >> pose[curDOF++];
			std::getline(loader,strContent);
		}
		RawData.push_back(pose);
	}

	MMatrix *data = new MMatrix(RawData.size(),RawData[0].size());
	for(size_t i = 0;i < data->sizeRow(); i++)
		for(size_t j = 0; j < data->sizeCol(); j++)
			data->assign(RawData[i][j],i,j);

	return data;
}
void AMCParser::toMMatrix(MMatrix* mat)
{
	
}