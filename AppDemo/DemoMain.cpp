#include <iostream>
#include "AppDemo.h"
#include "AppUtility.h"

//#ifndef _DEBUG
//	#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
//#endif
#define NO_DEBUG 


#ifndef _DEBUG
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main(int argc, char **argv)
#endif
{
  
	AncelApp::AppDemo demo;
	try
	{
		demo.run();
	}
	catch(std::exception& e)
    {
         fprintf(stderr, "An exception has occurred: %s\n", e.what());
    }
    return 0;
}