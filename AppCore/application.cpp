#include "gpwithrankprior.h"
#include "scriptparser.h"
#include "debugprint.h"
#include "boost/filesystem.hpp"
#include "MGPM.h"

//#include "gm.h"

#include <Eigen\Eigen>

using namespace Eigen;
int main(int argc, char*argv[])
{
    std::string script = "scripts/embed.txt";

    // Pull out the directory portion of the script filename.
    boost::filesystem::path scriptPath(script);
    std::string scriptDir = scriptPath.parent_path().string();
    script = scriptPath.filename().string();
    scriptDir.append("/");

	DBPRINTLN("Running script " << script.c_str());

    // Load script.
    GPCMScriptParser *parser = new GPCMScriptParser(script,scriptDir);
 	GPMGPModel   *model = new GPMGPModel(parser->getOptions(),false, false);
 //	GPWithRankPrior   *model = new GPWithRankPrior(parser->getOptions(),false, true);
	//
	
	/*MatrixXd tX = model->getLatentVariable();

	std::ofstream tfout("scripts/tri_b.txt");
	for(std::size_t i = 0; i < tX.rows(); i++)
	{
		for(std::size_t j = 0; j < tX.cols(); j++)
		{
				tfout << tX(i,j) << " ";
		}
		tfout << std::endl;
	}*/

	model->optimize();
//	
//	MatrixXd X = model->getLatentVariable();

	///*std::ofstream fout("scripts/tri.ann",std::ios::binary|std::ios::out);

	//int m = X.rows(),n = X.cols();
	//fout.write((char*)&m,sizeof(m));
	//fout.write((char*)&n,sizeof(n));
	//fout.write((char*)X.data(),sizeof(double)*m*n);
	//std::cout << X << std::endl;*/

	//std::ofstream fout("D:/Ancel/mgp/tri.txt");
	//for(std::size_t i = 0; i < X.rows(); i++)
	//{
	//	for(std::size_t j = 0; j < X.cols(); j++)
	//	{
	//			fout << X(i,j) << " ";
	//	}
	//	fout << std::endl;
	//}
 //

	system("pause");
 	return 0;
}



	//std::ofstream fout("test_x.ann",std::ios::binary|std::ios::out);
	//int num = 30;
	//double delta = 1.5707963267948966192313216916398;
	//double r = 0.5;
	//double incr = 0.015;
	//int col = 2;
	//fout.write((char*)&num,sizeof(int));
	//fout.write((char*)&col,sizeof(int));
	//for (int i = 0; i < 30; i++)
	//{
	//	if(i == 0)
	//	{
	//		double temp = -r*sin(delta);
	//		fout.write((char*)&temp, sizeof(double));
	//		temp = -r*cos(delta);
	//		fout.write((char*)&temp, sizeof(double));
	//		//fout << -r*sin(delta) << " " << r*cos(delta) << std::endl;
	//	}
	//	else
	//	{
	//		double a = 0.17;
	//		double b = (r+i*incr);
	//		double c = (r+(i-1)*incr);
	//		double cosA = (b * b + c * c - a * a)/(2 * b * c);
	//		double theta_a = acos(cosA);
	//		delta += theta_a;

	//		double temp = -b*sin(delta);
	//		fout.write((char*)&temp, sizeof(double));
	//		temp = b*cos(delta);
	//		fout.write((char*)&temp, sizeof(double));
	//		//fout << -b*sin(delta) << " " << b*cos(delta) << std::endl;
	//	}
	//	
	//}
	//fout.close();


//
//	std::ofstream fout("test911.ann",std::ios::binary|std::ios::out);
//	int num = 30;
//	double delta = 1.5707963267948966192313216916398;
//	double r = 0.5;
//	double incr = 0.015;
//	int col = 2;
//	fout.write((char*)&num,sizeof(int));
//	fout.write((char*)&col,sizeof(int));
//
//	MatrixXd data(num,col);
//	for (int i = 0; i < 30; i++)
//	{
//		if(i == 0)
//		{
//			double temp = -r*sin(delta);
//			data(i,0) = temp;
////			fout.write((char*)&temp, sizeof(double));
//			temp = -r*cos(delta);
//			data(i,1) = temp;
////			fout.write((char*)&temp, sizeof(double));
//			//fout << -r*sin(delta) << " " << r*cos(delta) << std::endl;
//		}
//		else
//		{
//			double a = 0.17;
//			double b = (r+i*incr);
//			double c = (r+(i-1)*incr);
//			double cosA = (b * b + c * c - a * a)/(2 * b * c);
//			double theta_a = acos(cosA);
//			delta += theta_a;
//
//			double temp = -b*sin(delta);
//			data(i,0) = temp;
////			fout.write((char*)&temp, sizeof(double));
//			temp = b*cos(delta);
//			data(i,1) = temp;
////			fout.write((char*)&temp, sizeof(double));
//			//fout << -b*sin(delta) << " " << b*cos(delta) << std::endl;
//		}
//		
//	}
// 	fout.write((char*)data.data(),num*col*sizeof(double));
// 	fout.close();