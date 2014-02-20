
#include <Windows.h>
#include "AppUtility.h"
#include "AppDemo.h"

using namespace AncelApp;


std::string AncelApp::loadFile(std::string filter,HWND hWnd)
{
	OPENFILENAMEA   ofn;               //   common   dialog   box   structure 
	char   szFile[MAX_PATH] = {0};               //   buffer   for   file   name 
  
	//   Initialize   OPENFILENAME 
	
	ZeroMemory(&ofn,   sizeof(ofn)); 
	ofn.lStructSize   =   sizeof(ofn); 
	ofn.hwndOwner   =   hWnd; 
	ofn.lpstrFile   =   szFile; 
	ofn.nMaxFile   =   MAX_PATH;
 
  	if(filter == "*")
		ofn.lpstrFilter = "all(*.*)\0*.*\0";
	else
	{
		char   szFilter[MAX_PATH] = {0};
		std::string sep = filter + "(*." + filter + ")";
		::strcpy_s(szFilter,sep.c_str());
		szFilter[sep.size()] = '\0';
		std::string format = "*." + filter;
		::strcpy_s((char*)(szFilter + sep.size() + 1),MAX_PATH - sep.size() - 1,format.c_str());
		szFilter[sep.size() + 1 + format.size()] = '\0';
		szFilter[sep.size() + 2 + format.size()] = '\0';
		ofn.lpstrFilter = szFilter;
   	}
  	ofn.nFilterIndex   =   1; 
	ofn.lpstrFileTitle   =   NULL; 
	ofn.nMaxFileTitle   =   0; 

	char initDir[MAX_PATH];
	::GetCurrentDirectoryA(MAX_PATH,initDir);
	ofn.lpstrInitialDir   =  initDir; 
	ofn.Flags   =   OFN_EXPLORER | OFN_ALLOWMULTISELECT; 

//   Display   the   Open   dialog   box.   

	if(GetOpenFileNameA(&ofn)==TRUE)   
	{
		std::string filename = ofn.lpstrFile;
		return filename;
	}
	return "";
}
std::string AncelApp::saveFile(std::string filter,HWND hWnd)
{
	OPENFILENAMEA   ofn;               //   common   dialog   box   structure 
	char   szFile[MAX_PATH] = {0};               //   buffer   for   file   name 
  
	//   Initialize   OPENFILENAME 
	
	ZeroMemory(&ofn,   sizeof(ofn)); 
	ofn.lStructSize   =   sizeof(ofn); 
	ofn.hwndOwner   =   hWnd; 
	ofn.lpstrFile   =   szFile; 
	ofn.nMaxFile   =   MAX_PATH;
 
  	if(filter == "*")
		ofn.lpstrFilter = "all(*.*)\0*.*\0";
	else
	{
		char   szFilter[MAX_PATH] = {0};
		std::string sep = filter + "(*." + filter + ")";
		::strcpy_s(szFilter,sep.c_str());
		szFilter[sep.size()] = '\0';
		std::string format = "*." + filter;
		::strcpy_s((char*)(szFilter + sep.size() + 1),MAX_PATH - sep.size() - 1,format.c_str());
		szFilter[sep.size() + 1 + format.size()] = '\0';
		szFilter[sep.size() + 2 + format.size()] = '\0';
		ofn.lpstrFilter = szFilter;
   	}
  	ofn.nFilterIndex   =   1; 
	ofn.lpstrFileTitle   =   NULL; 
	ofn.nMaxFileTitle   =   0; 

	char initDir[MAX_PATH];
	::GetCurrentDirectoryA(MAX_PATH,initDir);
	ofn.lpstrInitialDir   =  initDir; 
	ofn.Flags   =   OFN_EXPLORER | OFN_ALLOWMULTISELECT; 

//   Display   the   Open   dialog   box.   

	if(GetSaveFileNameA(&ofn)==TRUE)   
	{
		std::string filename = ofn.lpstrFile;
		return filename;
	}
	return "";
}

Eigen::MatrixXd AncelApp::loadData(std::string filename)
{
	std::ifstream reader(filename, std::ios::binary|std::ios::in);
	std::size_t rows,cols;
	reader.read((char*)&rows, sizeof(rows));
	reader.read((char*)&cols, sizeof(cols));

	Eigen::MatrixXd data(rows,cols);

	reader.read((char*)data.data(),sizeof(double)*rows*cols);
	return data;
}