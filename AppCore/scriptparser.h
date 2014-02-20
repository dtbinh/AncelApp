// Object for parsing a script file and returning an options structure.
#pragma once

#include "options.h"

#include <Eigen/Core>

using namespace Eigen;

// Forward declarations.
class GPCMMatWriter;

class GPCMScriptParser
{
protected:
    // Stored script filename.
    std::string scriptName;
    // Stored script.
    std::string script;
    // The options.
    GPCMOptions options;
    // Read a script file.
    std::string readFile(std::string script, std::string dir);
public:
    // Helper function to parse a string representing a vector.
    static VectorXd parseVector(std::string &str);
    // Constructor.
    GPCMScriptParser(std::string filename, std::string dir);
    // Get options.
    GPCMOptions &getOptions();
    // Write debugging information.
    void write(GPCMMatWriter *writer);
    // Destructor.
    ~GPCMScriptParser();
};
