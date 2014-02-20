#define TIXML_USE_STL

#include "Utility.h"
#include <TinyXML\tinyxml.h>
#include <OGRE\OgreVector3.h>
#include <OGRE\OgreQuaternion.h>
#include <OGRE\OgreMatrix3.h>
#include <sstream>
#include <fstream>
#include <vector>


struct DOF
{
	bool x,y,z;
	DOF():x(false),y(false),z(false){};
};
struct LIM
{
	double low,up;
};
struct AnimBone
{
	int ID;
	DOF Dof;
	LIM X;
	LIM Y;
	LIM Z;
	std::string name;
	Ogre::Vector3 dir;
	float length;
	Ogre::Vector3 Axis;
	AnimBone* parent;
	std::vector<AnimBone*> child;
	AnimBone(): parent(NULL){}

	Ogre::Vector3 pos;
	Ogre::Quaternion LocalAxis;
	Ogre::Quaternion MeshOri;
	std::string meshName;
};

void LoadBone(std::ifstream & Loader,AnimBone* Bone)
{
	std::string strContent;
		
	Loader >> strContent;    //ID
	Loader >> Bone->ID;

	Loader >> strContent;   //Name
	Loader >> Bone->name;
 
	Loader >> strContent; // Direction
	Loader >> Bone->dir.x;
	Loader >> Bone->dir.y;
	Loader >> Bone->dir.z;

	Loader >> strContent; // Length
	Loader >> Bone->length;

	Loader >> strContent; // Axis
	Loader >> Bone->Axis.x;
	Loader >> Bone->Axis.y;
	Loader >> Bone->Axis.z;
	Loader >> strContent;

	Loader >> strContent;
	if(strContent == "dof"){
		Loader >> strContent;
		while(strContent.find("limits") == std::string::npos){
			if(strContent == "rx")
				Bone->Dof.x = 1;
			if(strContent == "ry")
				Bone->Dof.y = 1;
			if(strContent == "rz")
				Bone->Dof.z = 1;
			Loader >> strContent;
		}
	}
	bool fx = false,fy = false,fz = false;
	if(strContent.find("limits") != std::string::npos)
		Loader >> strContent;
	while(strContent.find("end") == std::string::npos) 
	{
 		char c;
		std::stringstream ssFilter(strContent);
		ssFilter >> c;
		if(Bone->Dof.x&& !fx)
		{
			ssFilter >> Bone->X.low;
			Loader >> strContent;
			ssFilter.str(strContent);
			ssFilter.clear();
			ssFilter >> Bone->X.up;
			fx = true;
		}
		else if(Bone->Dof.y&& !fy)
		{
			ssFilter >> Bone->Y.low;
			Loader >> strContent;
			ssFilter.str(strContent);
			ssFilter.clear();
 			ssFilter >> Bone->Y.up;
			fy = true;
		}
		else if(Bone->Dof.z&& !fz)
		{
			ssFilter >> Bone->Z.low;
			Loader >> strContent;
			ssFilter.str(strContent);
			ssFilter.clear();
 			ssFilter >> Bone->Z.up;
			fz= true;
		}

		Loader >> strContent;
	}
}
template<typename T>
std::string toString(T a)
{
	std::string ret;
	std::stringstream Filter;
	Filter << a;
 	Filter >> ret;
	return ret;
}

bool Utility::WriteSkeletonToXML(std::string ASFFileName)
{
	std::ifstream Loader(ASFFileName);

	std::string strContent;
 	while(strContent != ":bonedata")
		std::getline(Loader,strContent);
	std::getline(Loader,strContent);
	
	std::vector<AnimBone*> vAllBones;
	
	
	AnimBone* Bone = new AnimBone();
	Bone->ID = 0;
	Bone->name = "root";
	Bone->dir = Ogre::Vector3(1.0f,0.0f,0.0f);
	Bone->length = 0.0;
	Bone->Axis = Ogre::Vector3(0.0f,0.0f,0.0f);
	
	Bone->Dof.x = true;
	Bone->Dof.y = true;
	Bone->Dof.z = true;
	
	Bone->X.low = -180;
	Bone->X.up =  180;
	Bone->Y.low = -180;
	Bone->Y.up =  180;
	Bone->Z.low = -180;
	Bone->Z.up =  180;
 
	vAllBones.push_back(Bone);

	
	while(strContent.find("begin") != std::string::npos)
	{
		AnimBone* Bone = new AnimBone();
		vAllBones.push_back(Bone);
	 	LoadBone(Loader,Bone);
		std::getline(Loader,strContent);
		std::getline(Loader,strContent);
	}
	
	std::string maskBone[] = {"rhand", "lhand", "rfingers", "lfingers", "rtoes", "ltoes", "rthumb","lthumb"};
	
	std::vector<AnimBone*>::iterator it = vAllBones.begin();
	
	double handlen = 0.0;
	for(it; it != vAllBones.end(); it++)
	{
		if(find(maskBone,maskBone + 8,(*it)->name) != maskBone + 8)
		{
			std::cout << (*it)->name << std::endl;
			if((*it)->name == "rthumb")
				handlen += (*it)->length;
			if((*it)->name == "rhand")
				handlen += (*it)->length;

			it = vAllBones.erase(it);
			if(it != vAllBones.begin())
				it = it - 1;
 		}
	}
	
	for(std::size_t i = 0; i < vAllBones.size(); i++)
	{
		vAllBones[i]->ID = i;
		if((vAllBones[i]->name == "rwrist") || (vAllBones[i]->name == "lwrist"))
			vAllBones[i]->length += handlen;
	}

	if(strContent == ":hierarchy")
	{ 
	 	std::getline(Loader,strContent);
		std::getline(Loader,strContent);
		while(strContent.find("end") == std::string::npos) {

			std::stringstream Filter(strContent);
			std::string strParent;
			Filter >> strParent;
			
			if(find(maskBone,maskBone + 8,strParent) != maskBone + 8)
			{
				getline(Loader,strContent);
				continue;
			}
			
			int parentID = 0;
			for(size_t i = 0; i < vAllBones.size(); i++)
				if(vAllBones[i]->name == strParent)
				{
					parentID = i;
					break;
				}
 			while(!Filter.eof()){
				Filter >> strContent;
				for(size_t i = 0; i < vAllBones.size(); i++)
					if(vAllBones[i]->name == strContent){
						vAllBones[i]->parent = vAllBones[parentID];
						vAllBones[parentID]->child.push_back(vAllBones[i]);
					}
 	  			}
			getline(Loader,strContent);
		}
  	}
	 
	 
	for(size_t i = 0; i < vAllBones.size(); i++)
	{
		Ogre::Vector3 axis;
		Ogre::Vector3 pos;
		Ogre::Degree deg;
		Ogre::Quaternion Q;
		Ogre::Quaternion meshOri;
		if(vAllBones[i]->name != "root")
		{
		 	
		 	Ogre::Matrix3 mat;
			pos = vAllBones[i]->parent->dir.normalisedCopy()*vAllBones[i]->parent->length;
			axis = vAllBones[i]->parent->Axis;
	 		 
			mat.FromEulerAnglesXYZ(Ogre::Radian(Ogre::Degree(-axis.x).valueRadians()),
								   Ogre::Radian(Ogre::Degree(-axis.y).valueRadians()),
								   Ogre::Radian(Ogre::Degree(-axis.z).valueRadians()));
			Ogre::Quaternion Q1;
			Q1.FromRotationMatrix(mat);
		 
			pos = mat * pos;

			Ogre::Matrix3 mat2;
			axis = vAllBones[i]->Axis;
			mat2.FromEulerAnglesZYX(Ogre::Radian(Ogre::Degree(axis.z).valueRadians()),
								    Ogre::Radian(Ogre::Degree(axis.y).valueRadians()),
								    Ogre::Radian(Ogre::Degree(axis.x).valueRadians()));
	 		 
			mat2 = mat * mat2;
			Q.FromRotationMatrix(mat2);

			Ogre::Matrix3 mat3;
			axis = vAllBones[i]->Axis;
			mat3.FromEulerAnglesXYZ(Ogre::Radian(Ogre::Degree(-axis.x).valueRadians()),
								    Ogre::Radian(Ogre::Degree(-axis.y).valueRadians()),
								    Ogre::Radian(Ogre::Degree(-axis.z).valueRadians()));
			
			Ogre::Vector3 Ori = mat3 * vAllBones[i]->dir;
			
			Ori.normalise();
			Ogre::Vector3 NegY = Ogre::Vector3(0.0f,-1.0f,0.0f);
			float _cosAngle = NegY.dotProduct(Ori);
			Ogre::Radian angle = Ogre::Math::ACos(_cosAngle);
			Ogre::Vector3 _axis = NegY.crossProduct(Ori);
			_axis.normalise();
			if(_axis.isZeroLength())
				_axis = Ogre::Vector3(1.0,0.0,0.0);
			meshOri.FromAngleAxis(angle,_axis);
	  		//Q.ToAngleAxis(deg,axis);

			if(vAllBones[i]->name == "head")
				vAllBones[i]->meshName = "head.mesh";
			else if(vAllBones[i]->name == "lfemur" || vAllBones[i]->name == "rfemur")
				vAllBones[i]->meshName = "femur.mesh";
			else if(vAllBones[i]->name == "ltibia" || vAllBones[i]->name == "rtibia")
				vAllBones[i]->meshName = "tibia.mesh";
			else if(vAllBones[i]->name == "lhumerus" || vAllBones[i]->name == "rhumerus")
				vAllBones[i]->meshName = "humerus.mesh";
			else 
				vAllBones[i]->meshName = "sphere.mesh";
		}
		else
 		{
			vAllBones[i]->meshName = "sphere.mesh";
			pos = Ogre::Vector3(0.0f,0.0f,0.0f);
			Q = Ogre::Quaternion(1,0,0,0);
			meshOri = Ogre::Quaternion();
	 
		}
		vAllBones[i]->MeshOri = meshOri;
		vAllBones[i]->LocalAxis = Q;
		vAllBones[i]->pos = pos;

	 }
	
	TiXmlDocument *myDocument = new TiXmlDocument();
	
	TiXmlElement *SkeletonElement = new TiXmlElement("skeleton");
	TiXmlElement *BonesElement = new TiXmlElement("bones");
	SkeletonElement->LinkEndChild(BonesElement);

	for(size_t i = 0; i < vAllBones.size(); i++)
	{
		TiXmlElement *BoneElement = new TiXmlElement("bone");
		BoneElement->SetAttribute("name",vAllBones[i]->name);
		if(vAllBones[i]->parent)
			 BoneElement->SetAttribute("parent",vAllBones[i]->parent->ID);
		else
			 BoneElement->SetAttribute("parent",0);
		BoneElement->SetAttribute("id",vAllBones[i]->ID);
		BoneElement->SetDoubleAttribute("length",vAllBones[i]->length);

 
		TiXmlElement *PositionElement = new TiXmlElement("position");
		PositionElement->SetDoubleAttribute("x", vAllBones[i]->pos.x);
		PositionElement->SetDoubleAttribute("y", vAllBones[i]->pos.y);
		PositionElement->SetDoubleAttribute("z", vAllBones[i]->pos.z);
		BoneElement->LinkEndChild(PositionElement);

	
		TiXmlElement *AxisElement = new TiXmlElement("localAxis");
		AxisElement->SetDoubleAttribute("w", vAllBones[i]->LocalAxis.w);
		AxisElement->SetDoubleAttribute("x", vAllBones[i]->LocalAxis.x);
		AxisElement->SetDoubleAttribute("y", vAllBones[i]->LocalAxis.y);
		AxisElement->SetDoubleAttribute("z", vAllBones[i]->LocalAxis.z);
		BoneElement->LinkEndChild(AxisElement);

		TiXmlElement *OriElement = new TiXmlElement("mesh");
		OriElement->SetAttribute      ("name", vAllBones[i]->meshName);
		OriElement->SetDoubleAttribute("w",    vAllBones[i]->MeshOri.w);
		OriElement->SetDoubleAttribute("x",    vAllBones[i]->MeshOri.x);
		OriElement->SetDoubleAttribute("y",    vAllBones[i]->MeshOri.y);
		OriElement->SetDoubleAttribute("z",    vAllBones[i]->MeshOri.z);
		BoneElement->LinkEndChild(OriElement);

		TiXmlElement *LimitsElement = new TiXmlElement("limits");
	
		int   jointtype = 0;
	 	if(vAllBones[i]->Dof.x) jointtype |= 1;
 		if(vAllBones[i]->Dof.y) jointtype |= 2;
		if(vAllBones[i]->Dof.z) jointtype |= 4;
		
		LimitsElement->SetAttribute("jointtype",jointtype);

 		TiXmlElement *DXElement = new TiXmlElement("dx");
		TiXmlElement *DYElement = new TiXmlElement("dy");
		TiXmlElement *DZElement = new TiXmlElement("dz");

	 	if(vAllBones[i]->Dof.x)
		{
			 DXElement->SetDoubleAttribute("low", vAllBones[i]->X.low);
			 DXElement->SetDoubleAttribute("up", vAllBones[i]->X.up);
			 LimitsElement->LinkEndChild(DXElement);
		}
 
		if(vAllBones[i]->Dof.y)
		{
			 DYElement->SetDoubleAttribute("low", vAllBones[i]->Y.low);
			 DYElement->SetDoubleAttribute("up", vAllBones[i]->Y.up);
			 LimitsElement->LinkEndChild(DYElement);
		}
 
		if(vAllBones[i]->Dof.z)
		{
			 DZElement->SetDoubleAttribute("low", vAllBones[i]->Z.low);
			 DZElement->SetDoubleAttribute("up", vAllBones[i]->Z.up);
			 LimitsElement->LinkEndChild(DZElement);
		}
 
 		BoneElement->LinkEndChild(LimitsElement);
 	 
		BonesElement->LinkEndChild(BoneElement);
 	}

	/*TiXmlElement *HierarchyElement = new TiXmlElement("boneshierarchy");
	SkeletonElement->LinkEndChild(HierarchyElement);

	for(size_t i = 0; i < vAllBones.size(); i++)
	{
		if(vAllBones[i]->parent != NULL)
		{
			TiXmlElement* boneparent = new TiXmlElement("boneparent");
 			boneparent->SetAttribute("parent", vAllBones[i]->parent->ID);
			boneparent->SetAttribute("id", vAllBones[i]->ID);
			boneparent->SetAttribute("name", vAllBones[i]->name);

			HierarchyElement->LinkEndChild(boneparent);
		}
	}*/

	myDocument->LinkEndChild(SkeletonElement);
	std::string filename = ASFFileName.substr(0,ASFFileName.find(".")) + ".xml";
 	myDocument->SaveFile(filename);

 //	//Writer << vAllBones.size() << std::endl;
 //	for(size_t i = 0; i < vAllBones.size(); i++) 
	//{
	//	Writer << vAllBones[i]->ID << " "
	//		<< vAllBones[i]->name << " "
	//		<< vAllBones[i]->length << " "
 //			<< vAllBones[i]->pos.x << " "
	//		<< vAllBones[i]->pos.y << " "
	//		<< vAllBones[i]->pos.z << " "
	//		<< vAllBones[i]->MeshOri.w << " "
	//		<< vAllBones[i]->MeshOri.x << " "
	//		<< vAllBones[i]->MeshOri.y << " "
	//		<< vAllBones[i]->MeshOri.z << " "
	//		<< vAllBones[i]->meshName << " "
	//		<< vAllBones[i]->LocalAxis.w << " "
	//		<< vAllBones[i]->LocalAxis.x << " "
	//		<< vAllBones[i]->LocalAxis.y << " "
	//		<< vAllBones[i]->LocalAxis.z << " "
	//		<< vAllBones[i]->child.size() << " ";
	//		for(size_t j = 0; j < vAllBones[i]->child.size(); j++)
	//		{
	//			Writer << vAllBones[i]->child[j]->ID << " ";
	//		}
	//		Writer << std::endl;
	//	delete vAllBones[i];
	//}
	return true;
}