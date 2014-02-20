// Responsible for reading MAT files.
#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <Eigen/Core>

using namespace Eigen;

class GPCMMatReader
{
protected:
    // Loaded variables.
    std::map<std::string, MatrixXd> variables;
    // Loaded structs.
    std::map<std::string, GPCMMatReader*> structs;
    // Ordered list of loaded variable names.
    std::vector<std::string> variableOrder;
    // Ordered list of loaded struct names.
    std::vector<std::string> structOrder;
    // Read a single variable.
    void readVariable(std::ifstream &file, std::string namestr, bool bNameProvided);
    // Read the contents of a file.
    void readFile(std::ifstream &file);
    // Read the contents of a struct or struct array.
    void readStruct(std::ifstream &file, int rows, int cols);
    // Read the contents of a cell array.
    void readCell(std::ifstream &file, int rows, int cols);
public:
    // Create a new reader from filename.
    GPCMMatReader(const std::string &filename);
    // Create a new reader from file stream.
    GPCMMatReader(std::ifstream &file, int mode, int rows = 0, int cols = 0);
    // Get number of variables in this structure/file.
    int variableCount();
    // Get variable by index.
    MatrixXd &getVariable(int index);
    // Get variable by name.
    MatrixXd &getVariable(const std::string &name);
    // Get number of structs in this structure/file.
    int structCount();
    // Get struct by index.
    GPCMMatReader *getStruct(int index);
    // Get struct by name.
    GPCMMatReader *getStruct(const std::string &name);
    // Destructor - release all variables.
    virtual ~GPCMMatReader();
};
