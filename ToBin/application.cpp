#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "AMCParser.h"
 
int main(int argc, char* argv[])
{
  
	if(argc < 2)
		return -1;
	std::string fullname(argv[1]);

	if(fullname.find(".amc") == std::string::npos)
		return -1;
	std::string outputname(fullname.substr(0,fullname.find('.')));

	outputname += ".ann";
	AMCParser parser;
	parser.loadMocapData(fullname);
	parser.writeToFile(outputname);

	if(argc > 2  && argc % 2 == 0)
	{
		std::vector<std::pair<std::size_t,std::size_t>> sequence;
		for(std::size_t i = 2; i < argc; i+=2)
		{
			std::stringstream ss;
			std::pair<std::size_t,std::size_t> seg;

			ss << argv[i];
			ss << " ";
			ss << argv[i+1];

			ss >> seg.first;
  			ss >> seg.second;
			sequence.push_back(seg);
		}

		std::ifstream reader(outputname,std::ios::binary|std::ios::in);
		std::size_t rows,cols;
		reader.read((char*)&rows,sizeof(rows));
		reader.read((char*)&cols,sizeof(cols));

		std::vector<std::vector<double>> data(rows,std::vector<double>(cols));

		for(std::size_t i = 0; i < cols; i++)
			for(std::size_t j = 0; j < rows; j++)
			{
				reader.read((char*)&data[j][i],sizeof(double));
			}
	
		reader.close();
		for(std::size_t i = 0; i < sequence.size(); i++)
		{
			std::stringstream ss;
			std::string filenameout;
			ss << sequence[i].first << "_" << sequence[i].second;
			ss >> filenameout;
			filenameout = outputname.substr(0,outputname.find('.')) + "_" + filenameout + ".ann";
			
			std::size_t subrows = ceil((sequence[i].second - sequence[i].first)/4.0);
			std::size_t subcols = cols;

			std::ofstream writer(filenameout ,std::ios::binary|std::ios::out);

			writer.write((char*)&subrows, sizeof(subrows));
			writer.write((char*)&subcols, sizeof(subcols));
		
			for(std::size_t j = 0; j < cols; j++)
			{
				for(std::size_t k = sequence[i].first; k < sequence[i].second; k+=4)
				{
 					writer.write((char*)&data[k][j],sizeof(double));
 				}
 			}
			writer.close();
		}
	}
	
 	return 0;
}


